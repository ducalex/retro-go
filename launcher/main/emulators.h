#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CRC_CACHE_MAGIC 0x21112222
#define CRC_CACHE_MAX_ENTRIES (0x10000 / sizeof(retro_crc_entry_t) - 1)
#define CRC_CACHE_PATH RG_BASE_PATH_CACHE "/crc32.bin"

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
    char name[128];
    char ext[8];
    char folder[32];
    uint32_t checksum;
    uint16_t missing_cover;
    uint8_t  is_valid;
    retro_emulator_t *emulator;
} retro_emulator_file_t;

typedef struct retro_emulator_s
{
    char system_name[64];
    char partition[16];
    char dirname[16];
    char extensions[32];
    size_t crc_offset;
    struct {
        retro_emulator_file_t *files;
        size_t count;
    } roms;
    bool crc_scan_done;
    bool initialized;
} retro_emulator_t;

typedef struct tab_s tab_t;

void emulators_init();
void emulator_init(retro_emulator_t *emu);
void emulator_start(retro_emulator_file_t *file, bool load_state);
void emulator_show_file_menu(retro_emulator_file_t *file);
void emulator_show_file_info(retro_emulator_file_t *file);
bool emulator_crc32_file(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);
const char *emu_get_file_path(retro_emulator_file_t *file);
size_t emu_get_file_crc_offset(retro_emulator_file_t *file);

void crc_cache_init(void);
void crc_cache_idle_task(tab_t *tab);
uint32_t crc_cache_lookup(retro_emulator_file_t *file);
void crc_cache_update(retro_emulator_file_t *file);
void crc_cache_save(void);
