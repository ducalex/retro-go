#include <rg_system.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "applications.h"
#include "bookmarks.h"
#include "utils.h"
#include "gui.h"

#define CRC_CACHE_MAGIC 0x21112222
#define CRC_CACHE_MAX_ENTRIES 8192
static struct __attribute__((__packed__))
{
    uint32_t magic;
    uint32_t count;
    struct {
        uint32_t key;
        uint32_t crc;
    } entries[CRC_CACHE_MAX_ENTRIES];
} *crc_cache;
static bool crc_cache_dirty = true;

static retro_app_t *apps[24];
static int apps_count = 0;


static const char *get_file_path(retro_file_t *file)
{
    static char buffer[RG_PATH_MAX + 1];
    if (file == NULL) return NULL;
    return strcat(strcat(strcpy(buffer, file->folder), "/"), file->name);
}

static void scan_folder(retro_app_t *app, const char* path, void *parent)
{
    RG_ASSERT(app && path, "Bad param");

    RG_LOGI("Scanning directory %s\n", path);

    const char *folder = const_string(path);
    rg_scandir_t *files = rg_storage_scandir(path, NULL, false);

    for (rg_scandir_t *entry = files; entry && entry->is_valid; ++entry)
    {
        uint8_t is_valid = false;
        uint8_t type = 0x00;

        if (entry->is_file)
        {
            char buffer[RG_PATH_MAX];
            snprintf(buffer, RG_PATH_MAX, " %s ", rg_extension(entry->name));
            is_valid = strstr(app->extensions, rg_strtolower(buffer)) != NULL;
            type = 0x00;
        }
        else if (entry->is_dir)
        {
            is_valid = true;
            type = 0xFF;
        }

        if (!is_valid)
            continue;

        app->files[app->files_count++] = (retro_file_t) {
            .name = strdup(entry->name),
            .folder = folder,
            .app = (void*)app,
            .type = type,
            .is_valid = true,
        };

        if ((app->files_count % 10) == 0)
        {
            size_t new_size = (app->files_count + 10 + 2) * sizeof(retro_file_t);
            retro_file_t *new_buf = realloc(app->files, new_size);
            if (!new_buf)
            {
                RG_LOGW("Ran out of memory, file scanning stopped at %d entries ...\n", app->files_count);
                break;
            }
            app->files = new_buf;
        }

        if (type == 0xFF)
        {
            retro_file_t *file = &app->files[app->files_count-1];
            scan_folder(app, get_file_path(file), file);
        }
    }

    free(files);
}

static void application_init(retro_app_t *app)
{
    RG_LOGI("Initializing application '%s' (%s)\n", app->description, app->partition);

    if (app->initialized)
        app->files_count = 0;

    // This checks if we have crc cover folders, the idea is to skip the crc later on if we don't!
    // It adds very little delay but it could become an issue if someone has thousands of named files...
    rg_scandir_t *files = rg_storage_scandir(app->paths.covers, NULL, false);
    if (!files)
        rg_storage_mkdir(app->paths.covers);
    else
    {
        for (rg_scandir_t *entry = files; entry->is_valid && !app->use_crc_covers; ++entry)
            app->use_crc_covers = entry->name[1] == 0 && isalnum(entry->name[0]);
        free(files);
    }

    rg_storage_mkdir(app->paths.saves);
    rg_storage_mkdir(app->paths.roms);
    scan_folder(app, app->paths.roms, 0);

    app->initialized = true;
}

static void application_start(retro_file_t *file, int load_state)
{
    RG_ASSERT(file, "Unable to find file...");
    char *part = strdup(file->app->partition);
    char *name = strdup(file->app->short_name);
    char *path = strdup(get_file_path(file));
    int flags = (gui.startup_mode ? RG_BOOT_ONCE : 0);
    if (load_state != -1)
    {
        flags |= RG_BOOT_RESUME;
        flags |= (load_state << 4) & RG_BOOT_SLOT_MASK;
    }
    bookmark_add(BOOK_TYPE_RECENT, file); // This could relocate *file, but we no longer need it
    rg_system_switch_app(part, name, path, flags);
}

static void crc_cache_init(void)
{
    crc_cache = calloc(1, sizeof(*crc_cache));
    if (!crc_cache)
    {
        RG_LOGE("Failed to allocate crc_cache!\n");
        return;
    }
    // File format: {magic:U32 count:U32} {{key:U32 crc:U32}, ...}
    FILE *fp = fopen(RG_BASE_PATH_CACHE"/crc32.bin", "rb");
    if (fp)
    {
        fread(crc_cache, 8, 1, fp);
        if (crc_cache->magic == CRC_CACHE_MAGIC && crc_cache->count <= CRC_CACHE_MAX_ENTRIES)
        {
            RG_LOGI("Loaded CRC cache (entries: %d)\n", crc_cache->count);
            fread(crc_cache->entries, crc_cache->count, 8, fp);
            crc_cache_dirty = false;
        }
        else
        {
            crc_cache->count = 0;
        }
        fclose(fp);
    }
}

static uint32_t crc_cache_calc_key(retro_file_t *file)
{
    // return ((uint64_t)rg_crc32(0, (void *)file->name, strlen(file->name)) << 33 | file->size);
    // This should be reasonably unique
    return rg_crc32(0, (void *)file->name, strlen(file->name));
}

static uint32_t crc_cache_lookup(retro_file_t *file)
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

static void crc_cache_save(void)
{
    if (!crc_cache || !crc_cache_dirty)
        return;

    RG_LOGI("Saving cache\n");

    FILE *fp = fopen(RG_BASE_PATH_CACHE"/crc32.bin", "wb");
    if (fp)
    {
        size_t minsize = RG_MIN(8 + (crc_cache->count * sizeof(crc_cache->entries[0])), sizeof(*crc_cache));
        fwrite(crc_cache, minsize, 1, fp);
        fclose(fp);
        crc_cache_dirty = false;
    }
}

static void crc_cache_update(retro_file_t *file)
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
    // FIXME: Disabled for now because it interferes with the webserver...
    return;

    if (!crc_cache)
        return;

    if (crc_cache->count < CRC_CACHE_MAX_ENTRIES)
    {
        int start_offset = 0;
        int remaining = 100;

        // Find the currently focused app, if any
        for (int i = 0; i < apps_count; i++)
        {
            if (tab && tab->arg == apps[i])
            {
                start_offset = i;
                break;
            }
        }

        for (int i = 0; i < apps_count && remaining > 0; i++)
        {
            retro_app_t *app = apps[(start_offset + i) % apps_count];
            int processed = 0;

            if (!app->available || app->crc_scan_done)
                continue;

            gui_set_status(tab, "BUILDING CACHE...", "SCANNING");
            gui_redraw(); // gui_draw_status(tab);

            if (!app->initialized)
                application_init(app);

            if ((gui.joystick |= rg_input_read_gamepad()))
                remaining = -1;

            for (int j = 0; j < app->files_count && remaining > 0; j++)
            {
                retro_file_t *file = &app->files[j];

                if (file->checksum == 0)
                    file->checksum = crc_cache_lookup(file);

                if (file->checksum == 0 && application_get_file_crc32(file))
                {
                    processed++;
                    remaining--;
                }

                // Give up on any button press to improve responsiveness
                if ((gui.joystick |= rg_input_read_gamepad()))
                    remaining = -1;
            }

            if (processed == 0 && remaining != -1)
                app->crc_scan_done = true;

            gui_set_status(tab, "", "");
            gui_redraw(); // gui_draw_status(tab);
        }
    }

    crc_cache_save();
}

static void tab_refresh(tab_t *tab)
{
    retro_app_t *app = (retro_app_t *)tab->arg;

    memset(&tab->status, 0, sizeof(tab->status));

    const char *basepath = const_string(app->paths.roms);
    const char *folder = tab->navpath ?: basepath;
    size_t items_count = 0;
    char *ext = NULL;

    if (folder == basepath)
        tab->navpath = NULL;

    if (app->files_count > 0)
    {
        gui_resize_list(tab, app->files_count);

        for (size_t i = 0; i < app->files_count; i++)
        {
            retro_file_t *file = &app->files[i];

            if (!file->is_valid)
                continue;

            if (file->folder != folder && strcmp(file->folder, folder) != 0)
                continue;

            listbox_item_t *item = &tab->listbox.items[items_count++];

            if (file->type == 0xFF)
            {
                snprintf(item->text, 128, "[%s]", file->name);
                // snprintf(item->text, 128, "/%s/", file->name);
            }
            else
            {
                snprintf(item->text, 128, "%s", file->name);
                if ((ext = strrchr(item->text, '.')))
                    *ext = 0;
            }

            item->arg = file;
        }
    }

    gui_resize_list(tab, items_count);
    gui_sort_list(tab);

    if (items_count == 0)
    {
        gui_resize_list(tab, 6);
        sprintf(tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(tab->listbox.items[1].text, " ");
        sprintf(tab->listbox.items[2].text, "Place roms in folder: %s", rg_relpath(app->paths.roms));
        sprintf(tab->listbox.items[3].text, "With file extension: %s", app->extensions);
        sprintf(tab->listbox.items[4].text, " ");
        sprintf(tab->listbox.items[5].text, "You can hide this tab in the menu");
        tab->listbox.cursor = 4;
    }

    gui_scroll_list(tab, SCROLL_SET, tab->listbox.cursor);
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_app_t *app = (retro_app_t *)tab->arg;
    retro_file_t *file = (retro_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        retro_file_t *selected = bookmark_find_first(BOOK_TYPE_RECENT, app);
        tab->navpath = selected ? selected->folder : NULL;

        application_init(app);
        tab_refresh(tab);

        if (selected)
        {
            for (int i = 0; i < tab->listbox.length; i++)
            {
                retro_file_t *file = tab->listbox.items[i].arg;
                if (strcmp(file->name, selected->name) == 0)
                {
                    gui_scroll_list(tab, SCROLL_SET, i);
                    break;
                }
            }
        }
    }
    else if (event == TAB_REFRESH)
    {
        tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        gui_set_status(tab, NULL, "");
        gui_set_preview(tab, NULL);
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        if (file && !tab->preview && gui.browse && gui.idle_counter == 1)
            gui_load_preview(tab);
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == TAB_ACTION)
    {
        if (file)
        {
            if (file->type == 0xFF)
            {
                tab->navpath = const_string(get_file_path(file));
                tab->listbox.cursor = 0;
                tab_refresh(tab);
            }
            else
            {
                application_show_file_menu(file, false);
            }
        }
    }
    else if (event == TAB_BACK)
    {
        if (tab->navpath)
        {
            const char *from = rg_basename(const_string(tab->navpath));

            tab->navpath = const_string(rg_dirname(tab->navpath));
            tab->listbox.cursor = 0;

            tab_refresh(tab);

            // This seems bad but keep in mind that folders are sorted to the top of items[]
            for (int i = 0; i < tab->listbox.length; ++i)
            {
                retro_file_t *item = tab->listbox.items[i].arg;
                if (strcmp(item->name, from) == 0)
                {
                    tab->listbox.cursor = i;
                    break;
                }
            }
        }
    }
}

bool application_path_to_file(const char *path, retro_file_t *file)
{
    RG_ASSERT(path && file, "Bad param");

    for (int i = 0; i < apps_count; ++i)
    {
        size_t baselen = strlen(apps[i]->paths.roms);
        if (strncmp(path, apps[i]->paths.roms, baselen) == 0 && path[baselen] == '/')
        {
            *file = (retro_file_t) {
                .name = strdup(rg_basename(path)),
                .folder = const_string(rg_dirname(path)),
                .app = apps[i],
                .is_valid = true,
            };
            return true;
        }
    }

    return false;
}

bool application_get_file_crc32(retro_file_t *file)
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

        if ((fp = fopen(get_file_path(file), "rb")))
        {
            fseek(fp, file->app->crc_offset, SEEK_SET);

            while (count != 0)
            {
                // Give up on any button press to improve responsiveness
                if ((gui.joystick = rg_input_read_gamepad()))
                    break;

                count = fread(buffer, 1, sizeof(buffer), fp);
                crc_tmp = rg_crc32(crc_tmp, buffer, count);
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

static void show_file_info(retro_file_t *file)
{
    char filesize[16];
    char filecrc[16] = "Compute";
    struct stat st;

    if (stat(get_file_path(file), &st) != 0)
    {
        rg_gui_alert("File not found", file->name);
        return;
    }

    rg_gui_option_t options[] = {
        {0, "Name", (char *)file->name, 1, NULL},
        {0, "Folder", (char *)file->folder, 1, NULL},
        {0, "Size", filesize, 1, NULL},
        {3, "CRC32", filecrc, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {5, "Delete file", NULL, 1, NULL},
        {1, "Close", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    sprintf(filesize, "%ld KB", st.st_size / 1024);

    while (true) // We loop in case we need to update the CRC
    {
        if (file->checksum)
            sprintf(filecrc, "%08X (%d)", file->checksum, file->app->crc_offset);

        switch (rg_gui_dialog("File properties", options, -1))
        {
        case 3:
            application_get_file_crc32(file);
            continue;
        case 5:
            if (rg_gui_confirm("Delete selected file?", 0, 0))
            {
                if (unlink(get_file_path(file)) == 0)
                {
                    bookmark_remove(BOOK_TYPE_FAVORITE, file);
                    bookmark_remove(BOOK_TYPE_RECENT, file);
                    file->is_valid = false;
                    gui_event(TAB_REFRESH, gui_get_current_tab());
                    return;
                }
            }
            continue;
        default:
            return;
        }
    }
}

void application_show_file_menu(retro_file_t *file, bool advanced)
{
    const char *rom_path = get_file_path(file);
    char *sram_path = rg_emu_get_path(RG_PATH_SAVE_SRAM, rom_path);
    rg_emu_state_t *savestate = rg_emu_get_states(rom_path, 4);
    bool has_save = savestate->used > 0;
    bool has_sram = access(sram_path, F_OK) == 0;
    bool is_fav = bookmark_exists(BOOK_TYPE_FAVORITE, file);
    int slot = -1;

    rg_gui_option_t choices[] = {
        {0, "Resume game", NULL, has_save, NULL},
        {1, "New game    ", NULL, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {3, is_fav ? "Del favorite" : "Add favorite", NULL, 1, NULL},
        {2, "Delete save", NULL, has_save || has_sram, NULL},
        RG_DIALOG_SEPARATOR,
        {4, "Properties", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    int sel = rg_gui_dialog(NULL, choices, has_save ? 0 : 1);
    switch (sel)
    {
    case 0:
        if ((slot = rg_gui_savestate_menu("Resume", rom_path, 1)) == -1)
            break;
        /* fallthrough */
    case 1:
        crc_cache_save();
        gui_save_config();
        application_start(file, slot);
        break;

    case 2:
        while ((slot = rg_gui_savestate_menu("Delete save?", rom_path, 0)) != -1)
        {
            unlink(savestate->slots[slot].preview);
            unlink(savestate->slots[slot].file);
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
            bookmark_add(BOOK_TYPE_FAVORITE, file); // This could relocate *file
        break;

    case 4:
        show_file_info(file);
        break;

    default:
        break;
    }

    free(sram_path);
    free(savestate);

    // gui_redraw();
}

static void application(const char *desc, const char *name, const char *exts, const char *part, uint16_t crc_offset)
{
    if (!rg_system_have_app(part))
    {
        RG_LOGI("Application '%s' (%s) not present, skipping\n", desc, part);
        return;
    }

    retro_app_t *app = calloc(1, sizeof(retro_app_t));
    apps[apps_count++] = app;

    snprintf(app->description, sizeof(app->description), "%s", desc);
    snprintf(app->short_name, sizeof(app->short_name), "%s", name);
    snprintf(app->partition, sizeof(app->partition), "%s", part);
    snprintf(app->extensions, sizeof(app->extensions), " %s ", exts);
    rg_strtolower(app->partition);
    rg_strtolower(app->short_name);
    rg_strtolower(app->extensions);
    snprintf(app->paths.covers, RG_PATH_MAX, RG_BASE_PATH_COVERS "/%s", app->short_name);
    snprintf(app->paths.saves, RG_PATH_MAX, RG_BASE_PATH_SAVES "/%s", app->short_name);
    snprintf(app->paths.roms, RG_PATH_MAX, RG_BASE_PATH_ROMS "/%s", app->short_name);
    app->available = rg_system_have_app(app->partition);
    app->files = calloc(10, sizeof(retro_file_t));
    app->crc_offset = crc_offset;

    gui_add_tab(app->short_name, app->description, app, event_handler);
}

void applications_init(void)
{
    application("Nintendo Entertainment System", "nes", "nes fc fds nsf", "retro-run", 16);
    application("Super Nintendo", "snes", "smc sfc", "snes9x-go", 0);
    application("Nintendo Gameboy", "gb", "gb gbc", "retro-run", 0);
    application("Nintendo Gameboy Color", "gbc", "gbc gb", "retro-run", 0);
    application("Nintendo Game & Watch", "gw", "gw", "retro-run", 0);
    application("Sega Master System", "sms", "sms sg", "smsplusgx-go", 0);
    application("Sega Game Gear", "gg", "gg", "smsplusgx-go", 0);
    application("Sega Mega Drive", "md", "md gen bin", "gwenesis", 0);
    application("Coleco ColecoVision", "col", "col", "smsplusgx-go", 0);
    application("NEC PC Engine", "pce", "pce", "retro-run", 0);
    application("Atari Lynx", "lnx", "lnx", "retro-run", 64);
    // application("Atari 2600", "a26", "a26", "stella-go", 0);
    // application("Neo Geo Pocket Color", "ngp", "ngp ngc", "ngpocket-go", 0);
    application("DOOM", "doom", "wad", "prboom-go", 0);

    // Special app to bootstrap native esp32 binaries from the SD card
    application("Bootstrap", "apps", "bin elf", "bootstrap", 0);

    crc_cache_init();
}
