#include <rg_system.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "emulators.h"
#include "bookmarks.h"
#include "gui.h"

static retro_emulator_t emulators[32];
static int emulators_count = 0;
static retro_crc_cache_t *crc_cache;
static bool crc_cache_dirty = false;


void crc_cache_init(void)
{
    crc_cache = calloc(1, sizeof(retro_crc_cache_t));
    crc_cache_dirty = true;

    if (!crc_cache)
    {
        RG_LOGE("Failed to allocate crc_cache!\n");
        return;
    }

    FILE *fp = fopen(CRC_CACHE_PATH, "rb");
    if (fp)
    {
        fread(crc_cache, offsetof(retro_crc_cache_t, entries), 1, fp);

        if (crc_cache->magic == CRC_CACHE_MAGIC && crc_cache->count <= CRC_CACHE_MAX_ENTRIES)
        {
            RG_LOGI("Loaded CRC cache (entries: %d)\n", crc_cache->count);
            fread(crc_cache->entries, crc_cache->count, sizeof(crc_cache->entries), fp);
            crc_cache_dirty = false;
        }
        else
        {
            crc_cache->count = 0;
        }
        fclose(fp);
    }
}

static uint32_t crc_cache_calc_key(retro_emulator_file_t *file)
{
    // return ((uint64_t)crc32_le(0, (void *)file->name, strlen(file->name)) << 33 | file->size);
    // This should be reasonably unique
    return crc32_le(0, (void *)file->name, strlen(file->name));
}

uint32_t crc_cache_lookup(retro_emulator_file_t *file)
{
    uint32_t key = crc_cache_calc_key(file);

    if (!crc_cache)
        return 0;

    for (int i = 0; i < crc_cache->count; i++)
    {
        if (crc_cache->entries[i].key == key)
            return crc_cache->entries[i].crc;
    }

    return 0;
}

void crc_cache_save(void)
{
    if (!crc_cache || !crc_cache_dirty)
        return;

    RG_LOGI("Saving cache\n");

    FILE *fp = fopen(CRC_CACHE_PATH, "wb");
    if (!fp && rg_mkdir(rg_dirname(CRC_CACHE_PATH)))
    {
        fp = fopen(CRC_CACHE_PATH, "wb");
    }
    if (fp)
    {
        size_t unused = CRC_CACHE_MAX_ENTRIES - RG_MIN(crc_cache->count, CRC_CACHE_MAX_ENTRIES);
        size_t minsize = sizeof(retro_crc_cache_t) - (unused * sizeof(retro_crc_entry_t));

        fwrite(crc_cache, minsize, 1, fp);
        fclose(fp);
        crc_cache_dirty = false;
    }
}

void crc_cache_update(retro_emulator_file_t *file)
{
    uint32_t key = crc_cache_calc_key(file);
    size_t index = 0;

    if (!crc_cache)
        return;

    if (crc_cache->count < CRC_CACHE_MAX_ENTRIES)
        index = crc_cache->count++;
    else
        index = rand() % CRC_CACHE_MAX_ENTRIES;

    crc_cache->magic = CRC_CACHE_MAGIC;
    crc_cache->entries[index].key = key;
    crc_cache->entries[index].crc = file->checksum;
    crc_cache_dirty = true;

    RG_LOGI("Adding %08X => %08X to cache (new total: %d)\n",
        key, file->checksum, crc_cache->count);

    // crc_cache_save();
}

void crc_cache_idle_task(tab_t *tab)
{
    if (!crc_cache)
        return;

    if (crc_cache->count < CRC_CACHE_MAX_ENTRIES)
    {
        int start_offset = 0;
        int remaining = 20;

        // Find the currently focused emulator, if any
        for (int i = 0; i < emulators_count; i++)
        {
            if (tab && tab->arg == &emulators[i])
            {
                start_offset = i;
                break;
            }
        }

        for (int i = 0; i < emulators_count && remaining > 0; i++)
        {
            retro_emulator_t *emulator = &emulators[(start_offset + i) % emulators_count];
            int processed = 0;

            if (emulator->crc_scan_done)
                continue;

            gui_set_status(tab, "BUILDING CACHE...", "SCANNING");
            gui_redraw(); // gui_draw_status(tab);

            if (!emulator->initialized)
                emulator_init(emulator);

            for (int j = 0; j < emulator->roms.files_count && remaining > 0; j++)
            {
                retro_emulator_file_t *file = &emulator->roms.files[j];

                if (file->checksum == 0)
                    file->checksum = crc_cache_lookup(file);

                if (file->checksum == 0 && emulator_get_file_crc32(file))
                {
                    processed++;
                    remaining--;
                }

                if (gui.joystick & RG_KEY_ANY)
                    remaining = -1;
            }

            if (processed == 0 && remaining != -1)
                emulator->crc_scan_done = true;

            gui_set_status(tab, "", "");
            gui_redraw(); // gui_draw_status(tab);
        }
    }

    crc_cache_save();
}

static const char *roms_folder(retro_emulator_t *emu, const char* path)
{
    // To do : use hashmap or something faster
    for (int i = 0; i < emu->roms.folders_count; i++)
    {
        if (strcmp(emu->roms.folders[i], path) == 0)
        {
            return emu->roms.folders[i];
        }
    }

    const char *folder = strdup(path);

    emu->roms.folders = realloc(emu->roms.folders, (emu->roms.folders_count + 1) * sizeof(char*));
    RG_ASSERT(emu->roms.folders && folder, "alloc failed");

    emu->roms.folders[emu->roms.folders_count++] = folder;

    return folder;
}

bool emulator_build_file_object(const char *path, retro_emulator_file_t *file)
{
    RG_ASSERT(path && file, "Bad param");

    if (strstr(path, RG_BASE_PATH_ROMS "/") != path)
        return false;

    const char *ptr = path + strlen(RG_BASE_PATH_ROMS "/");
    ptrdiff_t namelen = strchr(ptr, '/') - ptr;
    char short_name[64] = {0};

    if (namelen < 1 || namelen > 32)
        return false;

    strncpy(short_name, ptr, namelen);

    for (int i = 0; i < emulators_count; ++i)
    {
        if (strcmp(short_name, emulators[i].short_name) == 0)
        {
            const char *name = rg_basename(path);
            const char *ext = rg_extension(name);

            if (!ext || !name)
                return false;

            *file = (retro_emulator_file_t) {
                .name = strdup(name),
                .folder = roms_folder(&emulators[i], rg_dirname(path)),
                .emulator = &emulators[i],
                .is_valid = true,
            };
            return true;
        }
    }

    return false;
}

int emulator_scan_folder(retro_emulator_t *emu, const char* path, int flags)
{
    RG_ASSERT(emu && path, "Bad param");

    RG_LOGI("Scanning directory %s\n", path);

    DIR* dir = opendir(path);
    if (!dir)
        return -1;

    const char *folder = roms_folder(emu, path);
    char pathbuf[PATH_MAX + 1];
    struct dirent* ent;

    while ((ent = readdir(dir)))
    {
        const char *name = ent->d_name;

        if (name[0] == '.')
            continue;

        if (ent->d_type == DT_REG)
        {
            const char *ext = strrchr(name, '.') + 1;
            const char **emu_ext = emu->extensions;
            bool ext_match = false;

            while (ext > name && *emu_ext && !ext_match)
            {
                ext_match = strcasecmp(ext, *emu_ext++) == 0;
            }

            if (!ext_match)
                continue;

            if ((emu->roms.files_count % 10) == 0)
            {
                size_t new_size = (emu->roms.files_count + 10 + 2) * sizeof(retro_emulator_file_t);
                retro_emulator_file_t *new_buf = realloc(emu->roms.files, new_size);
                if (!new_buf)
                {
                    RG_LOGW("Ran out of memory, file scanning stopped at %d entries ...\n", emu->roms.files_count);
                    break;
                }
                emu->roms.files = new_buf;
            }

            retro_emulator_file_t *file = &emu->roms.files[emu->roms.files_count++];
            *file = (retro_emulator_file_t) {
                .name = strdup(name), // TO DO: Use a memory pool instead?
                .folder = folder,
                .emulator = (void*)emu,
                .is_valid = true,
            };
        }
        else if (ent->d_type == DT_DIR)
        {
            strcpy(pathbuf, path);
            strcat(pathbuf, "/");
            strcat(pathbuf, name);
            emulator_scan_folder(emu, pathbuf, flags);
        }
    }

    closedir(dir);

    return 0;
}

static void tab_refresh(tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;

    memset(&tab->status, 0, sizeof(tab->status));

    size_t base_len = strlen(RG_BASE_PATH_ROMS) + 1 + strlen(emu->short_name);
    size_t items_count = 0;
    char *ext = NULL;

    if (emu->roms.files_count > 0)
    {
        gui_resize_list(tab, emu->roms.files_count);

        for (size_t i = 0; i < emu->roms.files_count; i++)
        {
            retro_emulator_file_t *file = &emu->roms.files[i];
            if (file->is_valid)
            {
                listbox_item_t *item = &tab->listbox.items[items_count++];

                if (strlen(file->folder) > base_len)
                    snprintf(item->text, 128, "%s/%s", file->folder + base_len, file->name);
                else
                    snprintf(item->text, 128, "%s", file->name);

                if ((ext = strrchr(item->text, '.')))
                    *ext = 0;

                item->arg = file;
            }
        }
    }
    tab->is_empty = items_count == 0;

    if (!tab->is_empty)
    {
        gui_resize_list(tab, items_count);
        gui_sort_list(tab);
    }
    else
    {
        char extensions[64] = "";
        for (const char **ext = emu->extensions; *ext; ext++)
        {
            strcat(extensions, *ext);
            strcat(extensions, " ");
        }

        gui_resize_list(tab, 6);
        sprintf(tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(tab->listbox.items[2].text, "Place roms in folder: /roms/%s", emu->short_name);
        sprintf(tab->listbox.items[3].text, "With file extension: %s", extensions);
        sprintf(tab->listbox.items[5].text, "You can hide this tab in the menu");
        tab->listbox.cursor = 4;
    }
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        emulator_init(emu);
        tab_refresh(tab);
    }
    else if (event == TAB_REFRESH)
    {
        tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        if (!tab->is_empty && tab->listbox.length)
            sprintf(tab->status[0].left, "%d / %d", (tab->listbox.cursor + 1) % 10000, tab->listbox.length % 10000);
        else
            strcpy(tab->status[0].left, "No Games");
        gui_set_status(tab, NULL, "");

        rg_image_free(tab->preview);
        tab->preview = NULL;
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        if (file && !tab->preview && gui.idle_counter == 1)
            gui_load_preview(tab);
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == KEY_PRESS_A)
    {
        if (file)
            emulator_show_file_menu(file, false);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        // This is now reserved for subfolder navigation (go back)
    }
}

static void add_emulator(const char *system_name, const char *short_name, const char* extensions,
                         const char *partition, uint16_t crc_offset)
{
    if (!rg_system_find_app(partition))
    {
        RG_LOGI("Emulator '%s' (%s) not present, skipping\n", system_name, partition);
        return;
    }

    retro_emulator_t *emulator = &emulators[emulators_count++];

    *emulator = (retro_emulator_t) {
        .system_name = strdup(system_name),
        .partition = strdup(partition),
        .short_name = strdup(short_name),
        .crc_offset = crc_offset,
        .roms.folders = calloc(1, sizeof(char *)),
        // .roms.files = calloc(10, sizeof(retro_emulator_file_t *)),
        .roms.files = calloc(10, sizeof(retro_emulator_file_t)),
        .initialized = false,
    };

    const char **ext = emulator->extensions;
    char *buffer = strdup(extensions);
    char *token = strtok(buffer, " ");
    while (token)
    {
        *ext++ = strdup(token);
        token = strtok(NULL, " ");
    }
    free(buffer);

    gui_add_tab(short_name, system_name, emulator, event_handler);
}

void emulator_init(retro_emulator_t *emu)
{
    char path[PATH_MAX + 1];

    if (emu->initialized)
        return;

    RG_LOGI("Initializing emulator '%s'\n", emu->system_name);

    emu->initialized = true;

    sprintf(path, RG_BASE_PATH_COVERS "/%s", emu->short_name);
    rg_mkdir(path);

    sprintf(path, RG_BASE_PATH_SAVES "/%s", emu->short_name);
    rg_mkdir(path);

    sprintf(path, RG_BASE_PATH_ROMS "/%s", emu->short_name);
    rg_mkdir(path);

    emulator_scan_folder(emu, path, 0);
}

const char *emulator_get_file_path(retro_emulator_file_t *file)
{
    static char buffer[PATH_MAX + 1];
    if (file == NULL) return NULL;
    strcpy(buffer, file->folder);
    strcat(buffer, "/");
    strcat(buffer, file->name);
    return (const char*)buffer;
}

bool emulator_get_file_crc32(retro_emulator_file_t *file)
{
    uint8_t buffer[0x800];
    uint32_t crc_tmp = 0;
    int count = -1;
    FILE *fp;

    if (file == NULL)
        return false;

    if (file->checksum > 0)
        return true;

    if ((crc_tmp = crc_cache_lookup(file)))
    {
        file->checksum = crc_tmp;
    }
    else
    {
        tab_t *tab = gui_get_current_tab();
        gui_set_status(tab, NULL, "CRC32...");
        gui_redraw(); // gui_draw_status(tab);

        if ((fp = fopen(emulator_get_file_path(file), "rb")))
        {
            fseek(fp, file->emulator->crc_offset, SEEK_SET);

            while (count != 0)
            {
                gui.joystick = rg_input_read_gamepad();

                if (gui.joystick & RG_KEY_ANY)
                    break;

                count = fread(buffer, 1, sizeof(buffer), fp);
                crc_tmp = crc32_le(crc_tmp, buffer, count);
            }

            if (feof(fp))
            {
                file->checksum = crc_tmp;
                crc_cache_update(file);
            }

            fclose(fp);
        }

        gui_set_status(tab, NULL, "");
        gui_redraw(); // gui_draw_status(tab);
    }

    return file->checksum > 0;
}

void emulator_show_file_info(retro_emulator_file_t *file)
{
    char filesize[16];
    char filecrc[16] = "Compute";
    struct stat st;

    if (stat(emulator_get_file_path(file), &st) != 0)
    {
        rg_gui_alert("File not found", file->name);
        return;
    }

    dialog_option_t options[] = {
        {0, "Name", (char *)file->name, 1, NULL},
        // {0, "Type", (char *)file->emulator->short_name, 1, NULL},
        {0, "Folder", (char *)file->folder, 1, NULL},
        {0, "Size", filesize, 1, NULL},
        {3, "CRC32", filecrc, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1, "Close", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    sprintf(filesize, "%ld KB", st.st_size / 1024);

    if (file->checksum)
    {
        sprintf(filecrc, "%08X (%d)", file->checksum, file->emulator->crc_offset);
    }

    if (rg_gui_dialog("File properties", options, -1) == 3)
    {
        // CRC32 was selected, compute it and redraw dialog
        emulator_get_file_crc32(file);
        emulator_show_file_info(file);
    }
}

void emulator_show_file_menu(retro_emulator_file_t *file, bool advanced)
{
    char *save_path = rg_emu_get_path(RG_PATH_SAVE_STATE, emulator_get_file_path(file));
    char *sram_path = rg_emu_get_path(RG_PATH_SAVE_SRAM, emulator_get_file_path(file));
    char *scrn_path = rg_emu_get_path(RG_PATH_SCREENSHOT, emulator_get_file_path(file));
    bool has_save = access(save_path, F_OK) == 0;
    bool has_sram = access(sram_path, F_OK) == 0;
    bool is_fav = bookmark_find(BOOK_TYPE_FAVORITE, file) != NULL;

    dialog_option_t choices[] = {
        {0, "Resume game", NULL, has_save, NULL},
        {1, "New game    ", NULL, 1, NULL},
        {0, "------------", NULL, -1, NULL},
        {3, is_fav ? "Del favorite" : "Add favorite", NULL, 1, NULL},
        {2, "Delete save", NULL, has_save || has_sram, NULL},
        {0, "------------", NULL, -1, NULL},
        {5, "Delete file", NULL, 1, NULL},
        {4, "Properties", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    int sel = rg_gui_dialog(NULL, choices, has_save ? 0 : 1);
    switch (sel)
    {
    case 0:
    case 1:
        crc_cache_save();
        gui_save_config();
        bookmark_add(BOOK_TYPE_RECENT, file);
        emulator_start(file, sel == 0);
        break;

    case 2:
        if (has_save && rg_gui_confirm("Delete save state?", 0, 0))
        {
            unlink(save_path);
            unlink(scrn_path);
        }
        if (has_sram && rg_gui_confirm("Delete sram file?", 0, 0))
        {
            unlink(sram_path);
        }
        break;

    case 3:
        if (is_fav)
            bookmark_remove(BOOK_TYPE_FAVORITE, file);
        else
            bookmark_add(BOOK_TYPE_FAVORITE, file);
        break;

    case 4:
        emulator_show_file_info(file);
        break;

    case 5:
        if (rg_gui_confirm("Delete selected file?", 0, 0))
        {
            if (is_fav)
            {
                bookmark_remove(BOOK_TYPE_FAVORITE, file);
            }
            if (unlink(emulator_get_file_path(file)) == 0)
            {
                file->is_valid = false;
                gui_event(TAB_REFRESH, gui_get_current_tab());
            }
        }
        break;

    default:
        break;
    }

    free(save_path);
    free(sram_path);
    free(scrn_path);
}

void emulator_start(retro_emulator_file_t *file, bool load_state)
{
    RG_ASSERT(file, "Unable to find file...");
    const char *path = emulator_get_file_path(file);
    const char *name = file->emulator->short_name;
    int flags = (load_state ? RG_BOOT_RESUME : 0) | (gui.startup ? RG_BOOT_ONCE : 0);
    rg_system_start_app(file->emulator->partition, name, path, flags);
}

void emulators_init(void)
{
    add_emulator("Nintendo Entertainment System", "nes", "nes fds nsf", "nofrendo-go", 16);
    // add_emulator("Nintendo Famicom Disk System",  "fds",  "fds",     "nofrendo-go", 16);
    add_emulator("Nintendo Gameboy", "gb", "gb gbc", "gnuboy-go", 0);
    add_emulator("Nintendo Gameboy Color", "gbc", "gbc gb", "gnuboy-go", 0);
    add_emulator("Sega Master System", "sms", "sms sg", "smsplusgx-go", 0);
    add_emulator("Sega Game Gear", "gg", "gg", "smsplusgx-go", 0);
    add_emulator("ColecoVision", "col", "col", "smsplusgx-go", 0);
    add_emulator("PC Engine", "pce", "pce", "pce-go", 0);
    add_emulator("Atari Lynx", "lnx", "lnx", "handy-go", 64);
    add_emulator("Atari 2600", "a26", "a26", "stella-go", 0);
    add_emulator("Super Nintendo", "snes", "smc sfc", "snes9x-go", 0);
    add_emulator("Neo Geo Pocket Color", "ngp", "ngp ngc", "ngpocket-go", 0);
    add_emulator("DOOM", "doom", "wad", "prboom-go", 0);

    crc_cache_init();
}
