#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <rg_system.h>

typedef struct retro_app_s retro_app_t;

typedef struct
{
    const char *name;
    const char *folder;
    uint32_t checksum;
    uint16_t missing_cover;
    uint8_t type;
    uint8_t is_valid;
    retro_app_t *app;
} retro_file_t;

typedef struct retro_app_s
{
    char description[64];
    char short_name[24];
    char partition[24];
    char extensions[32];
    struct {
        char covers[RG_PATH_MAX];
        char saves[RG_PATH_MAX];
        char roms[RG_PATH_MAX];
    } paths;
    size_t crc_offset;
    retro_file_t *files;
    size_t files_capacity;
    size_t files_count;
    bool use_crc_covers;
    bool crc_scan_done;
    bool initialized;
    bool available;
} retro_app_t;

typedef struct tab_s tab_t;

void applications_init(void);
void application_show_file_menu(retro_file_t *file, bool simplified);
bool application_get_file_crc32(retro_file_t *file);
bool application_path_to_file(const char *path, retro_file_t *out_file);
void crc_cache_idle_task(tab_t *tab);
