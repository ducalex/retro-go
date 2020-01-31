#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ROM_PATH  "/sd/roms"
#define SAVE_PATH "/sd/odroid/data"
#define CACHE_PATH "/sd/odroid/cache"
#define ROMART_PATH "/sd/romart"

typedef struct {
    char name[64];
    char path[96];
    uint32_t checksum;
} retro_emulator_file_t;

typedef struct {
    char system_name[64];
    char dirname[32];
    char ext[8];
    uint16_t crc_offset;
    uint16_t partition;
    uint16_t* image_logo;
    uint16_t* image_header;
    struct {
        retro_emulator_file_t *files;
        int selected;
        int count;
    } roms;
    bool initialized;
} retro_emulator_t;

typedef struct {
    retro_emulator_t entries[16];
    uint8_t count;
} retro_emulators_t;

void emulators_init();
void emulators_init_emu(retro_emulator_t *emu);
void emulators_start_emu(retro_emulator_t *emu);

extern retro_emulators_t *emulators;
