#include <rg_system.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "emulators.h"
#include "favorites.h"
#include "images.h"
#include "gui.h"

static retro_emulator_t emulators[32];
static int emulators_count = 0;
static retro_crc_cache_t *crc_cache;


static void event_handler(gui_event_t event, tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        emulator_init(emu);

        if (emu->roms.count > 0)
        {
            sprintf(tab->status, "Games: %d", emu->roms.count);
            gui_resize_list(tab, emu->roms.count);

            for (int i = 0; i < emu->roms.count; i++)
            {
                strcpy(tab->listbox.items[i].text, emu->roms.files[i].name);
                tab->listbox.items[i].arg = &emu->roms.files[i];
            }

            gui_sort_list(tab, 0);
            tab->is_empty = false;
        }
        else
        {
            sprintf(tab->status, "No games");
            gui_resize_list(tab, 8);
            sprintf(tab->listbox.items[0].text, "Place roms in folder: /roms/%s", emu->dirname);
            sprintf(tab->listbox.items[2].text, "With file extension: %s", emu->extensions);
            sprintf(tab->listbox.items[4].text, "Use SELECT and START to navigate.");
            tab->listbox.cursor = 3;
            tab->is_empty = true;
        }
    }

    /* The rest of the events require a file to be selected */
    if (file == NULL)
        return;

    if (event == KEY_PRESS_A)
    {
        emulator_show_file_menu(file);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        emulator_show_file_info(file);
        gui_redraw();
    }
    else if (event == TAB_IDLE)
    {
        if (gui.show_preview && gui.idle_counter == (gui.show_preview_fast ? 1 : 8))
            gui_draw_preview(file);
    }
    else if (event == TAB_REDRAW)
    {
        if (gui.show_preview)
            gui_draw_preview(file);
    }
}

static void crc_cache_init(void)
{
    crc_cache = rg_alloc(sizeof(retro_crc_cache_t), MEM_ANY);

    FILE *fp = fopen(CRC_CACHE_PATH, "rb");
    if (fp)
    {
        fread(crc_cache, sizeof(retro_crc_cache_t), 1, fp);
        fclose(fp);

        if (crc_cache->magic == CRC_CACHE_MAGIC && crc_cache->count <= CRC_CACHE_MAX_ENTRIES)
        {
            RG_LOGI("Loaded CRC cache (entries: %d)\n", crc_cache->count);
        }
        else
        {
            crc_cache->count = 0;
            rg_fs_delete(CRC_CACHE_PATH);
        }
    }
}
static uint64_t crc_cache_calc_key(retro_emulator_file_t *file)
{
    // We could add the file type or emulator name for more entropy
    return ((uint64_t)crc32_le(0, (void *)file->name, strlen(file->name)) << 33 | file->size);
}

static uint32_t crc_cache_lookup(retro_emulator_file_t *file)
{
    uint64_t key = crc_cache_calc_key(file);

    for (int i = 0; i < crc_cache->count; i++)
    {
        if (crc_cache->entries[i].key == key)
            return crc_cache->entries[i].crc;
    }

    return 0;
}

static void crc_cache_update(retro_emulator_file_t *file)
{
    uint64_t key = crc_cache_calc_key(file);
    int index;
    FILE *fp;

    if (crc_cache->count < CRC_CACHE_MAX_ENTRIES)
        index = crc_cache->count++;
    else
        index = rand() % CRC_CACHE_MAX_ENTRIES;

    crc_cache->magic = CRC_CACHE_MAGIC;
    crc_cache->entries[index].key = crc_cache_calc_key(file);
    crc_cache->entries[index].crc = file->checksum;

    RG_LOGI("Adding %08X%08X %08X to cache (new total: %d)\n",
        (uint32_t)(key >> 32), (uint32_t)(key),
        file->checksum, crc_cache->count);

    if (!(fp = fopen(CRC_CACHE_PATH, "wb")))
    {
        rg_fs_mkdir(RG_BASE_PATH_CACHE);
        fp = fopen(CRC_CACHE_PATH, "wb");
    }

    if (fp)
    {
        fwrite(crc_cache, 32 + crc_cache->count * sizeof(retro_crc_entry_t), 1, fp);
        fclose(fp);
        // fwrite(cache, sizeof(retro_crc_cache_t), 1, fp);
        // fseek(fp, (cache->count - 1) * sizeof(retro_crc_entry_t), SEEK_CUR);
        // fwrite(entry, sizeof(retro_crc_entry_t), 1, fp);
    }
}

static void add_emulator(const char *system, const char *dirname, const char* extensions,
                         const char *part, uint16_t crc_offset, const binfile_t *logo,
                         const binfile_t *header)
{
    if (!rg_system_find_app(part))
    {
        RG_LOGW("add_emulator: Emulator '%s' (%s) not present, skipping\n", system, part);
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
        logo ? rg_gui_load_image(logo->data, logo->size) : NULL,
        header ? rg_gui_load_image(header->data, header->size) : NULL,
        emulator,
        event_handler);
}

void emulator_init(retro_emulator_t *emu)
{
    if (emu->initialized)
        return;

    emu->initialized = true;

    RG_LOGX("Retro-Go: Initializing emulator '%s'\n", emu->system_name);

    char path[256], buffer[32];
    char *files = NULL;
    size_t count = 0;

    sprintf(path, RG_BASE_PATH_SAVES "/%s", emu->dirname);
    rg_fs_mkdir(path);

    sprintf(path, RG_BASE_PATH_ROMS "/%s", emu->dirname);
    rg_fs_mkdir(path);

    if (rg_fs_readdir(path, &files, &count, true) && count > 0)
    {
        emu->roms.files = rg_alloc(count * sizeof(retro_emulator_file_t), MEM_ANY);
        emu->roms.count = 0;

        char *ptr = files;
        for (int i = 0; i < count; ++i)
        {
            const char *name = ptr;
            const char *ext = rg_fs_extension(ptr);
            size_t name_len = strlen(name);
            bool ext_match = false;

            // Advance pointer to next entry
            ptr += name_len + 1;

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
            file->crc_offset = emu->crc_offset;
            file->checksum = 0;
            file->missing_cover = 0;
        }
    }
    rg_free(files);
}

const char *emu_get_file_path(retro_emulator_file_t *file)
{
    static char buffer[192];
    if (file == NULL) return NULL;
    sprintf(buffer, "%s/%s.%s", file->folder, file->name, file->ext);
    return (const char*)&buffer;
}

bool emulator_build_file_object(const char *path, retro_emulator_file_t *file)
{
    const char *name = rg_fs_basename(path);
    const char *ext = rg_fs_extension(path);

    if (!ext || !name || !file)
        return false;

    size_t folder_len = strlen(path) - strlen(name);
    size_t name_len = strlen(name) - strlen(ext);

    memset(file, 0, sizeof(retro_emulator_file_t));
    strncpy(file->folder, path, RG_MIN(folder_len, 32) - 1);
    strncpy(file->name, name, RG_MIN(name_len, 128) - 1);
    strncpy(file->ext, ext, 7);

    const char *dirname = rg_fs_basename(file->folder);

    for (int i = 0; i < emulators_count; ++i)
    {
        if (strcmp(emulators[i].dirname, dirname) == 0)
        {
            file->crc_offset = emulators[i].crc_offset;
            file->emulator = &emulators[i];
            return true;
        }
    }

    return false;
}

bool emulator_crc32_file(retro_emulator_file_t *file)
{
    const size_t chunk_size = 0x4000;
    uint32_t crc_tmp = 0;
    FILE *fp;

    if (file == NULL)
        return false;

    if (file->checksum > 0)
        return true;

    if ((crc_tmp = crc_cache_lookup(file)))
    {
        file->checksum = crc_tmp;
    }
    else if ((fp = fopen(emu_get_file_path(file), "rb")) != NULL)
    {
        void *buffer = rg_alloc(chunk_size, MEM_ANY);
        size_t count = 0;
        bool done = false;

        gui_draw_notice("        CRC32", C_GREEN);

        fseek(fp, file->crc_offset, SEEK_SET);
        while (true)
        {
            gui.joystick = rg_input_read_gamepad();

            if (gui.joystick & GAMEPAD_KEY_ANY)
                break;

            count = fread(buffer, 1, chunk_size, fp);
            if (count == 0) break;

            crc_tmp = crc32_le(crc_tmp, buffer, count);
            if (count < chunk_size) break;
        }
        done = feof(fp);
        fclose(fp);

        rg_free(buffer);

        if (done)
        {
            file->checksum = crc_tmp;
            crc_cache_update(file);
        }
    }

    gui_draw_notice(" ", C_RED);

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

    sprintf(filesize, "%ld KB", rg_fs_filesize(emu_get_file_path(file)) / 1024);

    // if (emulator_crc32_file(file))
    if (file->checksum)
    {
        if (file->crc_offset)
            sprintf(filecrc, "%08X (%d)", file->checksum, file->crc_offset);
        else
            sprintf(filecrc, "%08X", file->checksum);
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
    char *save_path = rg_emu_get_path(EMU_PATH_SAVE_STATE, emu_get_file_path(file));
    char *sram_path = rg_emu_get_path(EMU_PATH_SAVE_SRAM, emu_get_file_path(file));
    char *scrn_path = rg_emu_get_path(EMU_PATH_SCREENSHOT, emu_get_file_path(file));
    bool has_save = rg_fs_filesize(save_path) > 0;
    bool has_sram = rg_fs_filesize(sram_path) > 0;
    bool is_fav = favorite_find(file) != NULL;

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
        gui_save_current_tab();
        emulator_start(file, sel == 0);
    }
    else if (sel == 2) {
        if (has_save) {
            if (rg_gui_confirm("Delete save state?", 0, 0)) {
                rg_fs_delete(save_path);
                rg_fs_delete(scrn_path);
            }
        }
        if (has_sram) {
            if (rg_gui_confirm("Delete sram file?", 0, 0)) {
                rg_fs_delete(sram_path);
            }
        }
    }
    else if (sel == 3) {
        if (is_fav)
            favorite_remove(file);
        else
            favorite_add(file);
    }

    rg_free(save_path);
    rg_free(sram_path);
    rg_free(scrn_path);
}

void emulator_start(retro_emulator_file_t *file, bool load_state)
{
    if (file == NULL)
        RG_PANIC("Unable to find file...");

    emu_start_action_t action = load_state ? EMU_START_ACTION_RESUME : EMU_START_ACTION_NEWGAME;
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
