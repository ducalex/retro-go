#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RG_BASE_PATH           "/sd"
#define RG_BASE_PATH_ROMS      RG_BASE_PATH "/roms"
#define RG_BASE_PATH_SAVES     RG_BASE_PATH "/odroid/data"
#define RG_BASE_PATH_CACHE     RG_BASE_PATH "/odroid/cache"
#define RG_BASE_PATH_CONFIG    RG_BASE_PATH "/odroid"
#define RG_BASE_PATH_ROMART    RG_BASE_PATH "/romart"

typedef enum
{
    RG_SKIP_HIDDEN = (1 << 0),
    RG_RECURSIVE   = (1 << 1),
    RG_FILES_ONLY  = (1 << 2),
} rg_vfs_flags_t;

// FIX ME: This needs to be moved to a more appropriate place...
typedef struct
{
    size_t count;
    size_t length;
    char buffer[];
} rg_strings_t;

bool rg_sdcard_mount(void);
bool rg_sdcard_unmount(void);
bool rg_sdcard_format(void);
void rg_sdcard_set_enable_activity_led(bool enable);
bool rg_sdcard_get_enable_activity_led(void);

rg_strings_t *rg_readdir(const char* path, int flags);
bool rg_mkdir(const char *dir);

// These are similar to libgen's but they never modify the source
const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char* rg_extension(const char *path);
