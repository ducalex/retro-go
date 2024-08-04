#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define RG_BASE_PATH        RG_STORAGE_ROOT "/retro-go"
#define RG_BASE_PATH_BIOS   RG_BASE_PATH "/bios"
#define RG_BASE_PATH_CACHE  RG_BASE_PATH "/cache"
#define RG_BASE_PATH_CONFIG RG_BASE_PATH "/config"
#define RG_BASE_PATH_COVERS RG_STORAGE_ROOT "/romart"
#define RG_BASE_PATH_MUSIC  RG_STORAGE_ROOT "/music"
#define RG_BASE_PATH_ROMS   RG_STORAGE_ROOT "/roms"
#define RG_BASE_PATH_SAVES  RG_BASE_PATH "/saves"
#define RG_BASE_PATH_THEMES RG_BASE_PATH "/themes"
#define RG_BASE_PATH_BORDERS RG_BASE_PATH "/borders"

typedef struct
{
    char path[RG_PATH_MAX + 1];
    const char *basename;
    const char *dirname;
    size_t size;
    time_t mtime;
    bool is_file;
    bool is_dir;
} rg_scandir_t;

typedef int (rg_scandir_cb_t)(const rg_scandir_t *file, void *arg);

enum
{
    RG_SCANDIR_FILES = (1 << 0),
    RG_SCANDIR_DIRS  = (1 << 1),
    RG_SCANDIR_STAT  = (1 << 8),
    RG_SCANDIR_SORT  = (1 << 9),
    RG_SCANDIR_RECURSIVE = (1 << 10),

    RG_SCANDIR_CONTINUE = 1,
    RG_SCANDIR_SKIP = 2,
    RG_SCANDIR_STOP = 0,
};

typedef struct
{
    const char *basename;
    const char *extension;
    size_t size;
    time_t mtime;
    bool is_dir;
    bool is_file;
    bool is_link;
    bool exists;
} rg_stat_t;

void rg_storage_init(void);
void rg_storage_deinit(void);
bool rg_storage_format(void);
bool rg_storage_ready(void);
void rg_storage_commit(void);
bool rg_storage_delete(const char *path);
bool rg_storage_exists(const char *path);
bool rg_storage_mkdir(const char *dir);
rg_stat_t rg_storage_stat(const char *path);
bool rg_storage_scandir(const char *path, rg_scandir_cb_t *callback, void *arg, uint32_t flags);
int64_t rg_storage_get_free_space(const char *path);

enum
{
    RG_FILE_ALIGN_8KB  = (1 << 0),      // Will align/pad data_out to 8KB  (not applicable if RG_FILE_USER_BUFFER)
    RG_FILE_ALIGN_16KB = (1 << 1),      // Will align/pad data_out to 16KB (not applicable if RG_FILE_USER_BUFFER)
    RG_FILE_ALIGN_32KB = (1 << 2),      // Will align/pad data_out to 32KB (not applicable if RG_FILE_USER_BUFFER)
    RG_FILE_ALIGN_64KB = (1 << 3),      // Will align/pad data_out to 64KB (not applicable if RG_FILE_USER_BUFFER)
    RG_FILE_USER_BUFFER = (1 << 4),     // Will use *data_out and *data_len provided by the user
    RG_FILE_ATOMIC_WRITE = (1 << 5),    // Will write to a temp file before replacing the target
};
bool rg_storage_read_file(const char *path, void **data_out, size_t *data_len, uint32_t flags);
bool rg_storage_write_file(const char *path, const void *data_ptr, size_t data_len, uint32_t flags);
bool rg_storage_unzip_file(const char *zip_path, const char *filter, void **data_out, size_t *data_len, uint32_t flags);
