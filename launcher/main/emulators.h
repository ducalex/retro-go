#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <rg_system.h>

#define CRC_CACHE_MAGIC 0x21112222
#define CRC_CACHE_MAX_ENTRIES (0x10000 / sizeof(retro_crc_entry_t) - 1)

typedef struct __attribute__((__packed__))
{
    // uint64_t key;
    uint32_t key;
    uint32_t crc;
} retro_crc_entry_t;

typedef struct __attribute__((__packed__))
{
    uint32_t magic;
    uint32_t count;
    retro_crc_entry_t entries[CRC_CACHE_MAX_ENTRIES];
} retro_crc_cache_t;

typedef struct retro_emulator_s retro_emulator_t;

typedef struct
{
    const char *name;
    const char *folder;
    uint32_t checksum;
    uint16_t missing_cover;
    uint8_t type;
    uint8_t is_valid;
    retro_emulator_t *emulator;
} retro_emulator_file_t;

typedef struct retro_emulator_s
{
    char system_name[60];
    char short_name[60];
    char partition[60];
    char extensions[60];
    struct {
        char covers[RG_PATH_MAX];
        char saves[RG_PATH_MAX];
        char roms[RG_PATH_MAX];
    } paths;
    size_t crc_offset;
    retro_emulator_file_t *files;
    size_t files_count;
    bool crc_scan_done;
    bool initialized;
} retro_emulator_t;

typedef struct tab_s tab_t;

void emulators_init(void);
void emulator_init(retro_emulator_t *emu);
void emulator_start(retro_emulator_file_t *file, bool load_state);
int  emulator_scan_folder(retro_emulator_t *emu, const char* path, void *parent);
void emulator_show_file_menu(retro_emulator_file_t *file, bool simplified);
void emulator_show_file_info(retro_emulator_file_t *file);
bool emulator_get_file_crc32(retro_emulator_file_t *file);
const char *emulator_get_file_path(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);

void crc_cache_init(void);
void crc_cache_idle_task(tab_t *tab);
uint32_t crc_cache_lookup(retro_emulator_file_t *file);
void crc_cache_update(retro_emulator_file_t *file);
void crc_cache_save(void);
