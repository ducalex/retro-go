#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char name[128];
    char ext[8];
    char folder[32];
    size_t size;
    size_t crc_offset;
    uint32_t checksum;
    bool missing_cover;
    void *emulator;
} retro_emulator_file_t;

typedef struct {
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
void emulator_crc32_file(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);
const char *emu_get_file_path(retro_emulator_file_t *file);
