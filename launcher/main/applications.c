#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "applications.h"
#include "bookmarks.h"
#include "gui.h"

#define CRC_CACHE_MAGIC 0x21112223
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

static int scan_folder_cb(const rg_scandir_t *entry, void *arg)
{
    retro_app_t *app = (retro_app_t *)arg;
    uint8_t type = RETRO_TYPE_INVALID;

    // Skip hidden files
    if (entry->basename[0] == '.')
        return RG_SCANDIR_SKIP;

    if (entry->is_file)
    {
        if (rg_extension_match(entry->basename, app->extensions))
            type = RETRO_TYPE_FILE;
    }
    else if (entry->is_dir)
    {
        RG_LOGI("Found subdirectory '%s'", entry->path);
        type = RETRO_TYPE_FOLDER;
    }

    if (type == RETRO_TYPE_INVALID)
        return RG_SCANDIR_CONTINUE;

    if (app->files_count + 1 > app->files_capacity)
    {
        size_t new_capacity = app->files_capacity * 1.5;
        retro_file_t *new_buf = realloc(app->files, new_capacity * sizeof(retro_file_t));
        if (!new_buf)
        {
            RG_LOGW("Ran out of memory, file scanning stopped at %d entries ...", app->files_count);
            return RG_SCANDIR_STOP;
        }
        app->files = new_buf;
        app->files_capacity = new_capacity;
    }

    app->files[app->files_count++] = (retro_file_t) {
        .name = strdup(entry->basename),
        .folder = rg_unique_string(entry->dirname),
        .checksum = 0,
        .missing_cover = 0,
        .saves = 0,
        .type = type,
        .app = (void*)app,
    };

    return RG_SCANDIR_CONTINUE;
}

static int scan_saves_cb(const rg_scandir_t *entry, void *arg)
{
    if (entry->is_file && rg_extension_match(entry->basename, "sav"))
    {
        retro_app_t *app = (retro_app_t *)arg;
        for (size_t i = 0; i < app->files_count; i++)
        {
            retro_file_t *file = &app->files[i];
            // Saves are the rom name with possibly `.sav` or `-0.sav` appended.
            if (strncmp(entry->basename, file->name, strlen(file->name)) == 0)
            {
                // FIXME: Check if folder matches first
                file->saves++;
                break;
            }
        }
    }
    return RG_SCANDIR_CONTINUE;
}

static void application_init(retro_app_t *app)
{
    RG_LOGI("Initializing application '%s' (%s)", app->description, app->partition);

    if (app->initialized)
        return;

    rg_storage_mkdir(app->paths.covers);
    rg_storage_mkdir(app->paths.saves);
    rg_storage_mkdir(app->paths.roms);

    rg_storage_scandir(app->paths.roms, scan_folder_cb, app, RG_SCANDIR_RECURSIVE);
    rg_storage_scandir(app->paths.saves, scan_saves_cb, app, RG_SCANDIR_RECURSIVE);
    // rg_storage_scandir(app->paths.covers, scan_folder_cb3, app, RG_SCANDIR_RECURSIVE);

    app->use_crc_covers = rg_storage_exists(strcat(app->paths.covers, "/0"));
    app->paths.covers[strlen(app->paths.covers) - 2] = 0;

    app->initialized = true;
}

static const char *get_file_path(retro_file_t *file)
{
    static char buffer[RG_PATH_MAX + 1];
    RG_ASSERT_ARG(file);
    snprintf(buffer, RG_PATH_MAX, "%s/%s", file->folder, file->name);
    return buffer;
}

static void application_start(retro_file_t *file, int load_state)
{
    RG_ASSERT_ARG(file);
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

static uint32_t crc_read_file(retro_file_t *file, bool interactive)
{
    uint8_t buffer[0x800];
    uint32_t crc_tmp = 0;
    bool done = false;
    int count = -1;
    FILE *fp;

    if (file == NULL)
        return 0;

    if ((fp = fopen(get_file_path(file), "rb")))
    {
        fseek(fp, file->app->crc_offset, SEEK_SET);

        while (count != 0)
        {
            // Give up on any button press to improve responsiveness
            if (interactive && (gui.joystick = rg_input_read_gamepad()))
                break;

            count = fread(buffer, 1, sizeof(buffer), fp);
            crc_tmp = rg_crc32(crc_tmp, buffer, count);
        }

        done = feof(fp);

        fclose(fp);
    }

    return done ? crc_tmp : 0;
}

static void crc_cache_init(void)
{
    crc_cache = calloc(1, sizeof(*crc_cache));
    if (!crc_cache)
    {
        RG_LOGE("Failed to allocate crc_cache!");
        return;
    }

    void *data_ptr = crc_cache;
    size_t data_len = sizeof(*crc_cache);
    rg_storage_read_file(RG_BASE_PATH_CACHE "/crc32.bin", &data_ptr, &data_len, RG_FILE_USER_BUFFER);
    if (crc_cache->magic == CRC_CACHE_MAGIC && crc_cache->count <= CRC_CACHE_MAX_ENTRIES)
    {
        RG_LOGI("Loaded CRC cache (entries: %d)", (int)crc_cache->count);
        crc_cache_dirty = false;
    }
    else
    {
        crc_cache->count = 0;
    }
}

static void crc_cache_save(void)
{
    if (!crc_cache || !crc_cache_dirty)
        return;

    RG_LOGI("Saving CRC cache...");
    size_t data_len = RG_MIN(8 + (crc_cache->count * sizeof(crc_cache->entries[0])), sizeof(*crc_cache));
    crc_cache_dirty = !rg_storage_write_file(RG_BASE_PATH_CACHE"/crc32.bin", crc_cache, data_len, 0);
}

static uint32_t crc_cache_calc_key(retro_file_t *file)
{
    // return ((uint64_t)rg_crc32(0, (void *)file->name, strlen(file->name)) << 33 | file->size);
    // This should be reasonably unique
    const char *path = get_file_path(file);
    return rg_crc32(0, (const uint8_t *)path, strlen(path));
}

static int crc_cache_find(retro_file_t *file)
{
    if (!crc_cache)
        return -1;

    uint32_t key = crc_cache_calc_key(file);

    for (int i = 0; i < crc_cache->count; i++)
    {
        if (crc_cache->entries[i].key == key)
            return i;
    }

    return -1;
}

static uint32_t crc_cache_lookup(retro_file_t *file)
{
    int index = crc_cache_find(file);
    if (index != -1)
        return crc_cache->entries[index].crc;
    return 0;
}

static void crc_cache_update(retro_file_t *file)
{
    if (!crc_cache)
        return;

    uint32_t key = crc_cache_calc_key(file);
    int index = crc_cache_find(file);
    if (index == -1)
    {
        if (crc_cache->count < CRC_CACHE_MAX_ENTRIES)
            index = crc_cache->count++;
        else
            index = rand() % CRC_CACHE_MAX_ENTRIES;

        RG_LOGI("Adding %08X => %08X to cache (new total: %d)",
            (int)key, (int)file->checksum, (int)crc_cache->count);
    }
    else
    {
        RG_LOGI("Updating %08X => %08X to cache (total: %d)",
            (int)key, (int)file->checksum, (int)crc_cache->count);
    }

    crc_cache->magic = CRC_CACHE_MAGIC;
    crc_cache->entries[index].key = key;
    crc_cache->entries[index].crc = file->checksum;
    crc_cache_dirty = true;

    // crc_cache_save();
}

void crc_cache_prebuild(void)
{
    if (!crc_cache)
        return;

    for (int i = 0; i < apps_count; i++)
    {
        retro_app_t *app = apps[i];

        if (!app->available)
            continue;

        if (!app->initialized)
            application_init(app);

        for (int j = 0; j < app->files_count; j++)
        {
            retro_file_t *file = &app->files[j];

            rg_gui_draw_message("Scanning %s %d/%d", app->short_name, j, app->files_count);

            // Give up on any button press to improve responsiveness
            if (rg_input_read_gamepad())
                break;

            if (file->checksum)
                continue;

            if ((file->checksum = crc_cache_lookup(file)))
                continue;

            if ((file->checksum = crc_read_file(file, true)))
                crc_cache_update(file);
        }

        if (rg_input_read_gamepad())
            break;

        crc_cache_save();
        gui_redraw();
    }

    crc_cache_save();
}

static void tab_refresh(tab_t *tab, const char *selected)
{
    retro_app_t *app = (retro_app_t *)tab->arg;

    memset(&tab->status, 0, sizeof(tab->status));

    const char *basepath = rg_unique_string(app->paths.roms);
    const char *folder = rg_unique_string(tab->navpath ?: basepath);
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

            if (file->type == RETRO_TYPE_INVALID || !file->name)
                continue;

            if (file->folder != folder && strcmp(file->folder, folder) != 0)
                continue;

            if (file->type == RETRO_TYPE_FOLDER)
            {
                listbox_item_t *item = &tab->listbox.items[items_count++];
                snprintf(item->text, sizeof(item->text), "[%.40s]", file->name);
                item->group = 1;
                item->arg = file;
            }
            else if (file->type == RETRO_TYPE_FILE)
            {
                listbox_item_t *item = &tab->listbox.items[items_count++];
                snprintf(item->text, sizeof(item->text), "%s", file->name);
                if ((ext = strrchr(item->text, '.')))
                    *ext = 0;
                item->group = 2;
                item->arg = file;
            }
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
    else if (selected)
    {
        for (int i = 0; i < tab->listbox.length; i++)
        {
            retro_file_t *file = tab->listbox.items[i].arg;
            if (file && strcmp(file->name, selected) == 0) // file->folder == selected->folder
            {
                tab->listbox.cursor = i;
                break;
            }
        }
    }

    gui_scroll_list(tab, SCROLL_SET, tab->listbox.cursor);
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_app_t *app = (retro_app_t *)tab->arg;
    retro_file_t *file = (retro_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT || event == TAB_RESCAN)
    {
        if (event == TAB_RESCAN && app->initialized)
        {
            for (size_t i = 0; i < app->files_count; ++i)
                free((char *)app->files[i].name);
            app->files_count = 0;
            app->initialized = false;
        }

        application_init(app);
        tab->navpath = NULL;

        retro_file_t *selected = bookmark_find_by_app(BOOK_TYPE_RECENT, app);
        if (selected) // && !rg_storage_exists(get_file_path(selected)))
        {
            // rg_storage_exists can take a long time on large folders (200ms), this is much faster
            for (size_t i = 0; i < app->files_count; ++i)
            {
                retro_file_t *file = &app->files[i];
                if (selected->folder == file->folder && strcmp(selected->name, file->name) == 0)
                {
                    tab->navpath = file->folder;
                    break;
                }
            }
        }

        tab_refresh(tab, selected ? selected->name : NULL);
    }
    else if (event == TAB_REFRESH)
    {
        tab_refresh(tab, NULL);
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
    }
    else if (event == TAB_ACTION)
    {
        if (file)
        {
            if (file->type == RETRO_TYPE_FOLDER)
            {
                tab->navpath = rg_unique_string(get_file_path(file));
                tab->listbox.cursor = 0;
                tab_refresh(tab, NULL);
            }
            else if (file->type == RETRO_TYPE_FILE)
            {
                application_show_file_menu(file, false);
            }
        }
    }
    else if (event == TAB_BACK)
    {
        if (tab->navpath)
        {
            const char *from = rg_basename(rg_unique_string(tab->navpath));

            tab->navpath = rg_unique_string(rg_dirname(tab->navpath));
            tab->listbox.cursor = 0;

            tab_refresh(tab, from);
        }
    }
}

bool application_path_to_file(const char *path, retro_file_t *file)
{
    RG_ASSERT_ARG(path && file);

    for (int i = 0; i < apps_count; ++i)
    {
        size_t baselen = strlen(apps[i]->paths.roms);
        if (strncmp(path, apps[i]->paths.roms, baselen) == 0 && path[baselen] == '/')
        {
            *file = (retro_file_t) {
                .name = strdup(rg_basename(path)),
                .folder = rg_unique_string(rg_dirname(path)),
                .saves = 0xFF, // We don't know, but we want gui_load_preview to check if needed
                .type = RETRO_TYPE_FILE,
                .app = apps[i],
            };
            return true;
        }
    }

    return false;
}

bool application_get_file_crc32(retro_file_t *file)
{
    uint32_t crc_tmp = 0;

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

        if ((crc_tmp = crc_read_file(file, true)))
        {
            file->checksum = crc_tmp;
            crc_cache_update(file);
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
    rg_stat_t info = rg_storage_stat(get_file_path(file));

    if (!info.exists)
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
        RG_DIALOG_END,
    };

    sprintf(filesize, "%d KB", (int)info.size / 1024);

    while (true) // We loop in case we need to update the CRC
    {
        if (file->checksum)
            sprintf(filecrc, "%08X (%d)", (int)file->checksum, file->app->crc_offset);

        switch (rg_gui_dialog("File properties", options, -1))
        {
        case 3:
            application_get_file_crc32(file);
            continue;
        case 5:
            if (rg_gui_confirm("Delete selected file?", 0, 0))
            {
                if (remove(get_file_path(file)) == 0)
                {
                    bookmark_remove(BOOK_TYPE_FAVORITE, file);
                    bookmark_remove(BOOK_TYPE_RECENT, file);
                    file->type = RETRO_TYPE_INVALID;
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
    char *rom_path = strdup(get_file_path(file));
    char *sram_path = rg_emu_get_path(RG_PATH_SAVE_SRAM, rom_path);
    rg_emu_states_t *savestates = rg_emu_get_states(rom_path, 4);
    bool has_save = savestates->used > 0; // Don't rely on file->saves just yet
    bool has_sram = rg_storage_exists(sram_path);
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
        RG_DIALOG_END,
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
            remove(savestates->slots[slot].preview);
            remove(savestates->slots[slot].file);
            // FIXME: We should update the last slot used here
        }
        if (has_sram && rg_gui_confirm("Delete sram file?", 0, 0))
        {
            remove(sram_path);
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

    free(rom_path);
    free(sram_path);
    free(savestates);

    // gui_redraw();
}

static void application(const char *desc, const char *name, const char *exts, const char *part, uint16_t crc_offset)
{
    RG_ASSERT_ARG(desc && name && exts && part);

    if (!rg_system_have_app(part))
    {
        RG_LOGI("Application '%s' (%s) not present, skipping", desc, part);
        return;
    }

    retro_app_t *app = calloc(1, sizeof(retro_app_t));
    apps[apps_count++] = app;

    snprintf(app->description, sizeof(app->description), "%s", desc);
    snprintf(app->short_name, sizeof(app->short_name), "%s", name);
    snprintf(app->partition, sizeof(app->partition), "%s", part);
    snprintf(app->extensions, sizeof(app->extensions), " %s ", exts);
    snprintf(app->paths.covers, RG_PATH_MAX, RG_BASE_PATH_COVERS "/%s", app->short_name);
    snprintf(app->paths.saves, RG_PATH_MAX, RG_BASE_PATH_SAVES "/%s", app->short_name);
    snprintf(app->paths.roms, RG_PATH_MAX, RG_BASE_PATH_ROMS "/%s", app->short_name);
    app->available = rg_system_have_app(app->partition);
    app->files = calloc(100, sizeof(retro_file_t));
    app->files_capacity = 100;
    app->crc_offset = crc_offset;

    gui_add_tab(app->short_name, app->description, app, event_handler);
}

void applications_init(void)
{
    bool big_memory = rg_system_get_app()->availableMemory >= 0x600000;
    application("Nintendo Entertainment System", "nes", "nes fc fds nsf zip", "retro-core", 16);
    application("Super Nintendo", "snes", "smc sfc zip", "retro-core", 0);
    application("Nintendo Gameboy", "gb", "gb gbc zip", "retro-core", 0);
    application("Nintendo Gameboy Color", "gbc", "gbc gb zip", "retro-core", 0);
    // application("Nintendo Gameboy Advance", "gba", "gba zip", "gbsp", 0);
    application("Nintendo Game & Watch", "gw", "gw", "retro-core", 0);
    // application("Sega SG-1000", "sg1", "sms sg sg1", "retro-core", 0);
    application("Sega Master System", "sms", "sms sg zip", "retro-core", 0);
    application("Sega Game Gear", "gg", "gg zip", "retro-core", 0);
    application("Sega Mega Drive", "md", "md gen bin zip", "gwenesis", 0);
    application("Coleco ColecoVision", "col", "col rom zip", "retro-core", 0);
    application("NEC PC Engine", "pce", "pce zip", "retro-core", 0);
    application("Atari Lynx", "lnx", "lnx zip", "retro-core", 64);
    // application("Atari 2600", "a26", "a26", "stella-go", 0);
    // application("Neo Geo Pocket Color", "ngp", "ngp ngc", "ngpocket-go", 0);
    application("DOOM", "doom", big_memory ? "wad zip" : "wad", "prboom-go", 0);
    application("MSX", "msx", "rom mx1 mx2 dsk", "fmsx", 0);

    // Special app to bootstrap native esp32 binaries from the SD card
    // application("Bootstrap", "apps", "bin elf", "bootstrap", 0);

    crc_cache_init();
}
