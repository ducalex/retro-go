#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RG_BASE_PATH        RG_STORAGE_ROOT "/retro-go"
#define RG_BASE_PATH_BIOS   RG_BASE_PATH "/bios"
#define RG_BASE_PATH_CACHE  RG_BASE_PATH "/cache"
#define RG_BASE_PATH_CONFIG RG_BASE_PATH "/config"
#define RG_BASE_PATH_COVERS RG_STORAGE_ROOT "/romart"
#define RG_BASE_PATH_ROMS   RG_STORAGE_ROOT "/roms"
#define RG_BASE_PATH_SAVES  RG_BASE_PATH "/saves"
#define RG_BASE_PATH_THEMES RG_BASE_PATH "/themes"

typedef struct __attribute__((packed))
{
    uint32_t is_valid : 1;
    uint32_t is_file  : 1;
    uint32_t is_dir   : 1;
    uint32_t unused   : 5;
    uint32_t size     : 24;
    char name[76];
} rg_scandir_t;

enum
{
    RG_SCANDIR_STAT = 1, // This will populate file size
    RG_SCANDIR_SORT = 2, // This will sort using natural order
};

void rg_storage_init(void);
void rg_storage_deinit(void);
bool rg_storage_format(void);
bool rg_storage_ready(void);
void rg_storage_commit(void);
void rg_storage_set_activity_led(bool enable);
bool rg_storage_get_activity_led(void);
bool rg_storage_read_file(const char *path, void **data_ptr, size_t *data_len);
bool rg_storage_write_file(const char *path, const void *data_ptr, const size_t data_len);
bool rg_storage_delete(const char *path);
bool rg_storage_mkdir(const char *dir);
rg_scandir_t *rg_storage_scandir(const char *path, bool (*validator)(const char *path), uint32_t flags);
