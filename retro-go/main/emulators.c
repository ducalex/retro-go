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

    gui_add_tab(dirname, logo, header, p, NULL);
}

retro_emulator_file_t *emulator_get_file(retro_emulator_t *emu, int index)
{
    if (emu->roms.count == 0 || index > emu->roms.count) {
        return NULL;
    }
    return &emu->roms.files[index];
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
