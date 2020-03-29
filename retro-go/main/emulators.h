#pragma once

#include <odroid_sdcard.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char name[96];
    char ext[8];
    char path[128];
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
