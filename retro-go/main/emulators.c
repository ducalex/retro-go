#include <esp_partition.h>
#include <odroid_system.h>
#include <string.h>

#include "emulators.h"
#include "favorites.h"
#include "bitmaps.h"
#include "gui.h"

static retro_emulator_t emulators[16];
static int emulators_count = 0;

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
            sprintf(tab->listbox.items[2].text, "With file extension: .%s", emu->ext);
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
        if (file->checksum == 0)
            emulator_crc32_file(file);

        if (gui.show_cover && gui.idle_counter == (gui.show_cover == 1 ? 8 : 1))
            gui_draw_cover(file);
    }
    else if (event == TAB_REDRAW)
    {
        if (gui.show_cover)
            gui_draw_cover(file);
    }
}

static void add_emulator(const char *system, const char *dirname, const char* ext, const char *part,
                          uint16_t crc_offset, const void *logo, const void *header)
{
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, part);

    if (partition == NULL)
    {
        return;
    }

    retro_emulator_t *p = &emulators[emulators_count++];
    strcpy(p->system_name, system);
    strcpy(p->dirname, dirname);
    strcpy(p->ext, ext);
    p->partition = partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
    p->roms.count = 0;
    p->roms.files = NULL;
    p->initialized = false;
    p->crc_offset = crc_offset;

    gui_add_tab(dirname, logo, header, p, event_handler);
}

void emulator_init(retro_emulator_t *emu)
{
    if (emu->initialized)
        return;

    emu->initialized = true;

    printf("Retro-Go: Initializing emulator '%s'\n", emu->system_name);

    char path[128];
    char *files = NULL;
    size_t count = 0;

    sprintf(path, ODROID_BASE_PATH_CRC_CACHE "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    sprintf(path, ODROID_BASE_PATH_SAVES "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    sprintf(path, ODROID_BASE_PATH_ROMS "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    if (odroid_sdcard_list(path, &files, &count) == 0 && count > 0)
    {
        emu->roms.files = rg_alloc(count * sizeof(retro_emulator_file_t), MEM_ANY);
        emu->roms.count = 0;

        char *ptr = files;
        for (int i = 0; i < count; ++i)
        {
            const char *name = ptr;
            const char *ext = odroid_sdcard_get_extension(ptr);
            size_t name_len = strlen(name);

            // Advance pointer to next entry
            ptr += name_len + 1;

            if (!ext || strcasecmp(emu->ext, ext) != 0) //  && strcasecmp("zip", ext) != 0
                continue;

            retro_emulator_file_t *file = &emu->roms.files[emu->roms.count++];
            strcpy(file->folder, path);
            strcpy(file->name, name);
            strcpy(file->ext, ext);
            file->name[name_len-strlen(ext)-1] = 0;
            file->emulator = (void*)emu;
            file->crc_offset = emu->crc_offset;
            file->checksum = 0;
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

bool emulator_build_file_object(const char *path, retro_emulator_file_t *file)
{
    const char *name = odroid_sdcard_get_filename(path);
    const char *ext = odroid_sdcard_get_extension(path);

    if (ext == NULL || name == NULL)
        return false;

    memset(file, 0, sizeof(retro_emulator_file_t));
    strncpy(file->folder, path, strlen(path)-strlen(name)-1);
    strncpy(file->name, name, strlen(name)-strlen(ext)-1);
    strcpy(file->ext, ext);

    const char *dirname = odroid_sdcard_get_filename(file->folder);

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

void emulator_crc32_file(retro_emulator_file_t *file)
{
    if (file == NULL || file->checksum > 0)
        return;

    const int chunk_size = 32768;
    const char *file_path = emu_get_file_path(file);
    char *cache_path = odroid_system_get_path(ODROID_PATH_CRC_CACHE, file_path);
    FILE *fp, *fp2;

    file->missing_cover = 0;

    if ((fp = fopen(cache_path, "rb")) != NULL)
    {
        fread(&file->checksum, 4, 1, fp);
        fclose(fp);
    }
    else if ((fp = fopen(file_path, "rb")) != NULL)
    {
        void *buffer = malloc(chunk_size);
        uint32_t crc_tmp = 0;
        uint32_t count = 0;

        gui_draw_notice("        CRC32", C_GREEN);

        fseek(fp, file->crc_offset, SEEK_SET);
        while (true)
        {
            odroid_input_read_gamepad(&gui.joystick);
            if (gui.joystick.bitmask > 0) break;

            count = fread(buffer, 1, chunk_size, fp);
            if (count == 0) break;

            crc_tmp = crc32_le(crc_tmp, buffer, count);
            if (count < chunk_size) break;
        }

        free(buffer);

        if (feof(fp))
        {
            file->checksum = crc_tmp;
            if ((fp2 = fopen(cache_path, "wb")) != NULL)
            {
                fwrite(&file->checksum, 4, 1, fp2);
                fclose(fp2);
            }
        }
        fclose(fp);
    }
    else
    {
        file->checksum = 1;
    }

    free(cache_path);

    gui_draw_notice(" ", C_RED);
}

void emulator_show_file_info(retro_emulator_file_t *file)
{
    odroid_dialog_choice_t choices[] = {
        {0, "File", "...", 1, NULL},
        {0, "Type", "N/A", 1, NULL},
        {0, "Folder", "...", 1, NULL},
        {0, "Size", "0", 1, NULL},
        {0, "CRC32", "N/A", 1, NULL},
        {0, "---", "", -1, NULL},
        {1, "Close", "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    sprintf(choices[0].value, "%.127s", file->name);
    sprintf(choices[1].value, "%s", file->ext);
    sprintf(choices[2].value, "%s", file->folder);
    sprintf(choices[3].value, "%d KB", odroid_sdcard_get_filesize(emu_get_file_path(file)) / 1024);

    if (file->checksum > 1)
    {
        if (file->crc_offset)
            sprintf(choices[4].value, "%08X (%d)", file->checksum, file->crc_offset);
        else
            sprintf(choices[4].value, "%08X", file->checksum);
    }

    odroid_overlay_dialog("Properties", choices, -1);
}

void emulator_show_file_menu(retro_emulator_file_t *file)
{
    char *save_path = odroid_system_get_path(ODROID_PATH_SAVE_STATE, emu_get_file_path(file));
    char *sram_path = odroid_system_get_path(ODROID_PATH_SAVE_SRAM, emu_get_file_path(file));
    bool has_save = odroid_sdcard_get_filesize(save_path) > 0;
    bool has_sram = odroid_sdcard_get_filesize(sram_path) > 0;
    bool is_fav = favorite_find(file) != NULL;

    odroid_dialog_choice_t choices[] = {
        {0, "Resume game ", "", has_save, NULL},
        {1, "New game    ", "", 1, NULL},
        {0, "------------", "", -1, NULL},
        {3, is_fav ? "Del favorite" : "Add favorite", "", 1, NULL},
        {2, "Delete save ", "", has_save || has_sram, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };
    int sel = odroid_overlay_dialog(NULL, choices, has_save ? 0 : 1);

    if (sel == 0 || sel == 1) {
        gui_save_current_tab();
        emulator_start(file, sel == 0);
    }
    else if (sel == 2) {
        if (odroid_overlay_confirm("Delete save file?", false) == 1) {
            if (has_save) {
                odroid_sdcard_unlink(save_path);
            }
            if (has_sram) {
                odroid_sdcard_unlink(sram_path);
            }
        }
    }
    else if (sel == 3) {
        if (is_fav)
            favorite_remove(file);
        else
            favorite_add(file);
    }

    free(save_path);
    free(sram_path);
}

void emulator_start(retro_emulator_file_t *file, bool load_state)
{
    const char *path = emu_get_file_path(file);
    assert(path != NULL);

    printf("Retro-Go: Starting game: %s\n", path);

    odroid_settings_StartAction_set(load_state ? ODROID_START_ACTION_RESUME : ODROID_START_ACTION_NEWGAME);
    odroid_settings_RomFilePath_set(path);
    odroid_settings_commit();

    odroid_system_switch_app(((retro_emulator_t *)file->emulator)->partition);
}

void emulators_init()
{
    add_emulator("Nintendo Entertainment System", "nes", "nes", "nofrendo-go", 16, logo_nes, header_nes);
    add_emulator("Nintendo Gameboy", "gb", "gb", "gnuboy-go", 0, logo_gb, header_gb);
    add_emulator("Nintendo Gameboy Color", "gbc", "gbc", "gnuboy-go", 0, logo_gbc, header_gbc);
    add_emulator("Sega Master System", "sms", "sms", "smsplusgx-go", 0, logo_sms, header_sms);
    add_emulator("Sega Game Gear", "gg", "gg", "smsplusgx-go", 0, logo_gg, header_gg);
    add_emulator("ColecoVision", "col", "col", "smsplusgx-go", 0, logo_col, header_col);
    add_emulator("PC Engine", "pce", "pce", "huexpress-go", 0, logo_pce, header_pce);
    add_emulator("Atari Lynx", "lnx", "lnx", "handy-go", 64, logo_lnx, header_lnx);
    // add_emulator("Atari 2600", "a26", "a26", "stella-go", 0, logo_a26, header_a26);
}
