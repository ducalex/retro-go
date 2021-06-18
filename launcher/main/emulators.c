#include <rg_system.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "emulators.h"
#include "bookmarks.h"
#include "images.h"
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
    if (!fp)
    {
        rg_vfs_mkdir(RG_BASE_PATH_CACHE);
        fp = fopen(CRC_CACHE_PATH, "wb");
        fwrite(crc_cache, sizeof(retro_crc_cache_t), 1, fp);
        fclose(fp);

        fp = fopen(CRC_CACHE_PATH, "rb");
    }

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
    return crc32_le(file->size, (void *)file->name, strlen(file->name));
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

    if (gui.idle_counter >= 2000 && crc_cache->count < CRC_CACHE_MAX_ENTRIES)
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
            gui_draw_status(tab);

            if (!emulator->initialized)
                emulator_init(emulator);

            for (int j = 0; j < emulator->roms.count && remaining > 0; j++)
            {
                retro_emulator_file_t *file = &emulator->roms.files[j];

                if (file->checksum == 0)
                    file->checksum = crc_cache_lookup(file);

                if (file->checksum == 0 && emulator_crc32_file(file))
                {
                    processed++;
                    remaining--;
                }

                if (gui.joystick & GAMEPAD_KEY_ANY)
                    remaining = -1;
            }

            if (processed == 0 && remaining != -1)
                emulator->crc_scan_done = true;

            gui_set_status(tab, "", "");
            gui_draw_status(tab);
        }
    }

    crc_cache_save();
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        emulator_init(emu);

        memset(&tab->status, 0, sizeof(tab->status));

        if (emu->roms.count > 0)
        {
            gui_resize_list(tab, emu->roms.count);

            for (int i = 0; i < emu->roms.count; i++)
            {
                strcpy(tab->listbox.items[i].text, emu->roms.files[i].name);
                tab->listbox.items[i].arg = &emu->roms.files[i];
            }

            tab->is_empty = false;
        }
        else
        {
            gui_resize_list(tab, 8);
            sprintf(tab->listbox.items[0].text, "Place roms in folder: /roms/%s", emu->dirname);
            sprintf(tab->listbox.items[2].text, "With file extension: %s", emu->extensions);
            sprintf(tab->listbox.items[4].text, "Use SELECT and START to navigate.");
            tab->listbox.cursor = 3;
            tab->is_empty = true;
        }
    }
    else if (event == TAB_ENTER)
    {
        //
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_SCROLL)
    {
        if (!tab->is_empty && tab->listbox.length)
            sprintf(tab->status[0].left, "%d / %d", (tab->listbox.cursor + 1) % 10000, tab->listbox.length % 10000);
        else
            strcpy(tab->status[0].left, "No Games");
        gui_set_status(tab, NULL, "");
    }
    else if (event == TAB_REDRAW)
    {
        // gui_draw_preview(tab, file);
    }
    else if (event == TAB_IDLE)
    {
        if (file && gui.show_preview && gui.idle_counter == (gui.show_preview_fast ? 1 : 8))
            gui_draw_preview(tab, file);
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == KEY_PRESS_A)
    {
        if (file)
            emulator_show_file_menu(file);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        if (file)
            emulator_show_file_info(file);
        gui_redraw();
    }
}

static void add_emulator(const char *system, const char *dirname, const char* extensions,
                         const char *part, uint16_t crc_offset, const binfile_t *logo,
                         const binfile_t *header)
{
    if (!rg_system_find_app(part))
    {
        RG_LOGI("Emulator '%s' (%s) not present, skipping\n", system, part);
        return;
    }

    retro_emulator_t *emulator = &emulators[emulators_count++];

    memset(emulator, 0, sizeof(retro_emulator_t));
    strcpy(emulator->system_name, system);
    strcpy(emulator->partition, part);
    strcpy(emulator->dirname, dirname);
    strcpy(emulator->extensions, extensions);
    emulator->crc_offset = crc_offset;

    gui_add_tab(
        dirname,
        logo ? rg_image_load_from_memory(logo->data, logo->size, 0) : NULL,
        header ? rg_image_load_from_memory(header->data, header->size, 0) : NULL,
        emulator,
        event_handler);
}

void emulator_init(retro_emulator_t *emu)
{
    if (emu->initialized)
        return;

    emu->initialized = true;

    RG_LOGI("Initializing emulator '%s'\n", emu->system_name);

    char path[256], buffer[32];

    sprintf(path, RG_BASE_PATH_SAVES "/%s", emu->dirname);
    rg_vfs_mkdir(path);

    sprintf(path, RG_BASE_PATH_ROMS "/%s", emu->dirname);
    rg_vfs_mkdir(path);

    rg_strings_t *files = rg_vfs_readdir(path, RG_SKIP_HIDDEN|RG_RECURSIVE|RG_FILES_ONLY);
    if (files && files->count > 0)
    {
        char *files_ptr = files->buffer;
        size_t count = files->count;

        emu->roms.files = calloc(count, sizeof(retro_emulator_file_t));;
        emu->roms.count = 0;

        // In case of low memory, try to fit at least *something*...
        while (!emu->roms.files && count > 20)
        {
            count /= 2;
            emu->roms.files = calloc(count, sizeof(retro_emulator_file_t));
            RG_LOGW("Had to reduce list to fit in memory %d => %d ...\n", files->count, count);
        }

        if (!emu->roms.files)
            RG_PANIC("Out of memory, unable to alloc ROMs list!\n");

        for (int i = 0; i < count; ++i)
        {
            const char *name = files_ptr;
            const char *ext = rg_vfs_extension(files_ptr);
            size_t name_len = strlen(name);
            bool ext_match = false;

            // Advance pointer to next entry
            files_ptr += name_len + 1;

            char *token = strtok(strcpy(buffer, emu->extensions), " ");
            while (token && ext && !ext_match)
            {
                ext_match = strcasecmp(token, ext) == 0;
                token = strtok(NULL, " ");
            }

            if (!ext_match)
                continue;

            retro_emulator_file_t *file = &emu->roms.files[emu->roms.count++];
            strcpy(file->folder, path);
            strcpy(file->name, name);
            strcpy(file->ext, ext);
            file->name[name_len-strlen(ext)-1] = 0;
            file->emulator = (void*)emu;
            file->checksum = 0;
            file->missing_cover = 0;
            file->is_valid = 1;
        }
    }
    free(files);
}

const char *emu_get_file_path(retro_emulator_file_t *file)
{
    static char buffer[192];
    if (file == NULL) return NULL;
    sprintf(buffer, "%s/%s.%s", file->folder, file->name, file->ext);
    return (const char*)&buffer;
}

size_t emu_get_file_crc_offset(retro_emulator_file_t *file)
{
    if (file && file->emulator)
        return file->emulator->crc_offset;
    return 0;
}

bool emulator_build_file_object(const char *path, retro_emulator_file_t *file)
{
    const char *name = rg_vfs_basename(path);
    const char *ext = rg_vfs_extension(path);

    if (!ext || !name || !file)
        return false;

    size_t folder_len = strlen(path) - strlen(name);
    size_t name_len = strlen(name) - strlen(ext);

    memset(file, 0, sizeof(retro_emulator_file_t));
    strncpy(file->folder, path, RG_MIN(folder_len, 32) - 1);
    strncpy(file->name, name, RG_MIN(name_len, 128) - 1);
    strncpy(file->ext, ext, 7);

    const char *dirname = rg_vfs_basename(file->folder);

    for (int i = 0; i < emulators_count; ++i)
    {
        if (strcmp(emulators[i].dirname, dirname) == 0)
        {
            file->emulator = &emulators[i];
            file->is_valid = true;
            return true;
        }
    }

    return false;
}

bool emulator_crc32_file(retro_emulator_file_t *file)
{
    uint8_t buffer[0x1000];
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
        gui_draw_status(tab);

        if ((fp = fopen(emu_get_file_path(file), "rb")))
        {
            fseek(fp, emu_get_file_crc_offset(file), SEEK_SET);

            while (count != 0)
            {
                gui.joystick = rg_input_read_gamepad();

                if (gui.joystick & GAMEPAD_KEY_ANY)
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
        gui_draw_status(tab);
    }

    return file->checksum > 0;
}

void emulator_show_file_info(retro_emulator_file_t *file)
{
    char filesize[16];
    char filecrc[16] = "Compute";

    dialog_option_t options[] = {
        {0, "File", file->name, 1, NULL},
        {0, "Type", file->ext, 1, NULL},
        {0, "Folder", file->folder, 1, NULL},
        {0, "Size", filesize, 1, NULL},
        {3, "CRC32", filecrc, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1, "Close", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    sprintf(filesize, "%ld KB", rg_vfs_filesize(emu_get_file_path(file)) / 1024);

    if (file->checksum)
    {
        sprintf(filecrc, "%08X (%d)", file->checksum, emu_get_file_crc_offset(file));
    }

    if (rg_gui_dialog("Properties", options, -1) == 3)
    {
        // CRC32 was selected, compute it and redraw dialog
        emulator_crc32_file(file);
        emulator_show_file_info(file);
    }
}

void emulator_show_file_menu(retro_emulator_file_t *file)
{
    char *save_path = rg_emu_get_path(RG_PATH_SAVE_STATE, emu_get_file_path(file));
    char *sram_path = rg_emu_get_path(RG_PATH_SAVE_SRAM, emu_get_file_path(file));
    char *scrn_path = rg_emu_get_path(RG_PATH_SCREENSHOT, emu_get_file_path(file));
    bool has_save = rg_vfs_filesize(save_path) > 0;
    bool has_sram = rg_vfs_filesize(sram_path) > 0;
    bool is_fav = bookmark_find(BOOK_TYPE_FAVORITE, file) != NULL;

    dialog_option_t choices[] = {
        {0, "Resume game", NULL, has_save, NULL},
        {1, "New game    ", NULL, 1, NULL},
        {0, "------------", NULL, -1, NULL},
        {3, is_fav ? "Del favorite" : "Add favorite", NULL, 1, NULL},
        {2, "Delete save", NULL, has_save || has_sram, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    int sel = rg_gui_dialog(NULL, choices, has_save ? 0 : 1);

    if (sel == 0 || sel == 1) {
        crc_cache_save();
        gui_save_position(0); // emulator_start will commit
        bookmark_add(BOOK_TYPE_RECENT, file);
        emulator_start(file, sel == 0);
    }
    else if (sel == 2) {
        if (has_save) {
            if (rg_gui_confirm("Delete save state?", 0, 0)) {
                rg_vfs_delete(save_path);
                rg_vfs_delete(scrn_path);
            }
        }
        if (has_sram) {
            if (rg_gui_confirm("Delete sram file?", 0, 0)) {
                rg_vfs_delete(sram_path);
            }
        }
    }
    else if (sel == 3) {
        if (is_fav)
            bookmark_remove(BOOK_TYPE_FAVORITE, file);
        else
            bookmark_add(BOOK_TYPE_FAVORITE, file);
    }

    free(save_path);
    free(sram_path);
    free(scrn_path);
}

void emulator_start(retro_emulator_file_t *file, bool load_state)
{
    if (file == NULL)
        RG_PANIC("Unable to find file...");

    rg_start_action_t action = load_state ? RG_START_ACTION_RESUME : RG_START_ACTION_NEWGAME;
    rg_emu_start_game(file->emulator->partition, emu_get_file_path(file), action);
}

void emulators_init()
{
    add_emulator("Nintendo Entertainment System", "nes",  "nes fds", "nofrendo-go", 16, &logo_nes, &header_nes);
    // add_emulator("Nintendo Famicom Disk System",  "fds",  "fds",     "nofrendo-go", 16, &logo_nes, &header_nes);
    add_emulator("Nintendo Gameboy",              "gb",   "gb gbc",  "gnuboy-go",    0, &logo_gb,  &header_gb);
    add_emulator("Nintendo Gameboy Color",        "gbc",  "gbc gb",  "gnuboy-go",    0, &logo_gbc, &header_gbc);
    add_emulator("Sega Master System",            "sms",  "sms",     "smsplusgx-go", 0, &logo_sms, &header_sms);
    add_emulator("Sega Game Gear",                "gg",   "gg",      "smsplusgx-go", 0, &logo_gg,  &header_gg);
    add_emulator("ColecoVision",                  "col",  "col",     "smsplusgx-go", 0, &logo_col, &header_col);
    add_emulator("PC Engine",                     "pce",  "pce",     "huexpress-go", 0, &logo_pce, &header_pce);
    add_emulator("Atari Lynx",                    "lnx",  "lnx",     "handy-go",    64, &logo_lnx, &header_lnx);
    add_emulator("Atari 2600",                    "a26",  "a26",     "stella-go",    0, NULL,      NULL);
    add_emulator("Super Nintendo",                "snes", "smc sfc", "snes9x-go",    0, &logo_snes, &header_snes);
    add_emulator("Neo Geo Pocket Color",          "ngp",  "ngp ngc", "ngpocket-go",  0, &logo_ngp,  &header_ngp);

    crc_cache_init();
}
