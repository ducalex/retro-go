#include <esp_partition.h>
#include <esp_system.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <odroid_system.h>

#include "bitmaps/all.h"
#include "emulators.h"
#include "gui.h"

static retro_emulator_t emulators[16];
static int emulators_count = 0;

static void event_handler(gui_event_t event, tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_item_t *item = gui_get_selected_item(tab);;
    retro_emulator_file_t *file;

    if (event == TAB_INIT)
    {
        emulator_init(emu);

        sprintf(tab->status, "Games: %d", emu->roms.count);

        if (emu->roms.count > 0)
        {
            tab->listbox.length = emu->roms.count;
            tab->listbox.items = calloc(tab->listbox.length, sizeof(listbox_item_t));

            for (int i = 0; i < emu->roms.count; i++)
            {
                strcpy(tab->listbox.items[i].text, emu->roms.files[i].name);
                tab->listbox.items[i].arg = &emu->roms.files[i];
            }
        }
        else
        {
            tab->listbox.length = 8;
            tab->listbox.cursor = 2;
            tab->listbox.items = calloc(tab->listbox.length, sizeof(listbox_item_t));
            sprintf(tab->listbox.items[1].text, "No roms found!");
            sprintf(tab->listbox.items[4].text, "Place roms in folder: /roms/%s", emu->dirname);
            sprintf(tab->listbox.items[6].text, "With file extension: .%s", emu->ext);
        }
    }

    // File-specific events
    file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (file == NULL)
        return;

    // if (show_cover && idle_counter == (show_cover == 1 ? 8 : 1))

    if (event == TAB_REDRAW || gui.idle_counter == 8)
        gui_draw_cover(file);

    if (event == KEY_PRESS_A)
    {
        char *file_path = emulator_get_file_path(file);
        char *save_path = odroid_system_get_path(ODROID_PATH_SAVE_STATE, file_path);
        bool has_save = access(save_path, F_OK) != -1;

        odroid_dialog_choice_t choices[] = {
            {0, "Resume game", "", has_save, NULL},
            {1, "New game", "", 1, NULL},
            {2, "Delete save", "", has_save, NULL},
            ODROID_DIALOG_CHOICE_LAST
        };
        int sel = odroid_overlay_dialog(NULL, choices, has_save ? 0 : 1);

        if (sel == 0 || sel == 1) {
            gui_save_tab(tab);
            emulator_start(file, sel == 0);
        }
        else if (sel == 2) {
            if (odroid_overlay_confirm("Delete savestate?", false) == 1) {
                unlink(save_path);
            }
        }
        free(file_path);
        free(save_path);

        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        char *file_path = emulator_get_file_path(file);

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
        sprintf(choices[3].value, "%d KB", odroid_sdcard_get_filesize(file_path) / 1024);
        if (file->checksum > 1) {
            sprintf(choices[4].value, "%08X", file->checksum);
        }

        odroid_overlay_dialog("Properties", choices, -1);
        free(file_path);

        gui_redraw();
    }
    else if (event == TAB_IDLE)
    {
        if (file->checksum == 0)
            gui_crc32_file(file);
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

static int file_sort_comparator(const void *p, const void *q)
{
    return strcasecmp(((retro_emulator_file_t*)p)->name, ((retro_emulator_file_t*)q)->name);
}

void emulator_init(retro_emulator_t *emu)
{
    if (emu->initialized)
        return;

    emu->initialized = true;

    printf("Retro-Go: Initializing emulator '%s'\n", emu->system_name);

    char path[128];

    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    sprintf(path, ODROID_BASE_PATH_CRC_CACHE "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    sprintf(path, ODROID_BASE_PATH_SAVES "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    sprintf(path, ODROID_BASE_PATH_ROMS "/%s", emu->dirname);
    odroid_sdcard_mkdir(path);

    DIR* dir = opendir(path);
    if (dir)
    {
        char *selected_file = odroid_settings_RomFilePath_get();
        struct dirent* in_file;

        while ((in_file = readdir(dir)))
        {
            const char *ext = odroid_sdcard_get_extension(in_file->d_name);

            if (!ext) continue;

            if (strcasecmp(emu->ext, ext) != 0 && strcasecmp("zip", ext) != 0)
                continue;

            if (emu->roms.count % 100 == 0) {
                emu->roms.files = (retro_emulator_file_t *)realloc(emu->roms.files,
                    (emu->roms.count + 100) * sizeof(retro_emulator_file_t));
            }
            retro_emulator_file_t *file = &emu->roms.files[emu->roms.count++];
            strcpy(file->folder, path);
            strcpy(file->name, in_file->d_name);
            strcpy(file->ext, ext);
            file->name[strlen(file->name)-strlen(ext)-1] = 0;
            file->emulator = (void*)emu;
            file->checksum = 0;
        }

        if (emu->roms.count > 0)
        {
            qsort((void*)emu->roms.files, emu->roms.count, sizeof(retro_emulator_file_t), file_sort_comparator);
        }

        free(selected_file);
        closedir(dir);
    }

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);
}

char *emulator_get_file_path(retro_emulator_file_t *file)
{
    if (file == NULL) return NULL;
    char buffer[256];
    sprintf(buffer, "%s/%s.%s", file->folder, file->name, file->ext);
    return strdup(buffer);
}

void emulator_start(retro_emulator_file_t *file, bool load_state)
{
    char *path = emulator_get_file_path(file);
    assert(path != NULL);

    printf("Retro-Go: Starting game: %s\n", path);
    odroid_settings_StartAction_set(load_state ? ODROID_START_ACTION_RESUME : ODROID_START_ACTION_NEWGAME);
    odroid_settings_RomFilePath_set(path);
    odroid_system_switch_app(((retro_emulator_t *)file->emulator)->partition);
}

void emulators_init()
{
    add_emulator("Nintendo Entertainment System", "nes", "nes", "nofrendo", 16, logo_nes, header_nes);
    add_emulator("Nintendo Gameboy", "gb", "gb", "gnuboy", 0, logo_gb, header_gb);
    add_emulator("Nintendo Gameboy Color", "gbc", "gbc", "gnuboy", 0, logo_gbc, header_gbc);
    add_emulator("Sega Master System", "sms", "sms", "smsplusgx", 0, logo_sms, header_sms);
    add_emulator("Sega Game Gear", "gg", "gg", "smsplusgx", 0, logo_gg, header_gg);
    add_emulator("ColecoVision", "col", "col", "smsplusgx", 0, logo_col, header_col);
    add_emulator("PC Engine", "pce", "pce", "huexpress", 0, logo_pce, header_pce);
    add_emulator("Atari Lynx", "lnx", "lnx", "handy", 0, logo_lnx, header_lnx);
    // add_emulator("Atari 2600", "a26", "a26", "stella", 0, logo_a26, header_a26);
}
