#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CRC_CACHE_MAGIC 0x11112222
#define CRC_CACHE_MAX_ENTRIES 2500
#define CRC_CACHE_PATH RG_BASE_PATH_CACHE "/crc32.bin"

typedef struct __attribute__((__packed__))
{
    uint64_t key;
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
    size_t size;
    size_t crc_offset;
    uint32_t checksum;
    uint32_t missing_cover;
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
    bool initialized;
} retro_emulator_t;

void emulators_init();
void emulator_init(retro_emulator_t *emu);
void emulator_start(retro_emulator_file_t *file, bool load_state);
void emulator_show_file_menu(retro_emulator_file_t *file);
void emulator_show_file_info(retro_emulator_file_t *file);
bool emulator_crc32_file(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);
const char *emu_get_file_path(retro_emulator_file_t *file);
