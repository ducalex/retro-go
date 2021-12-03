#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RG_ROOT_PATH            "/sd"
#define RG_BASE_PATH            RG_ROOT_PATH "/retro-go"
#define RG_BASE_PATH_CACHE      RG_BASE_PATH "/cache"
#define RG_BASE_PATH_CONFIG     RG_BASE_PATH "/config"
#define RG_BASE_PATH_COVERS     RG_ROOT_PATH "/romart"
#define RG_BASE_PATH_ROMS       RG_ROOT_PATH "/roms"
#define RG_BASE_PATH_SAVES      RG_BASE_PATH "/saves"
#define RG_BASE_PATH_SYSTEM     RG_BASE_PATH "/system"
#define RG_BASE_PATH_THEMES     RG_BASE_PATH "/themes"

#ifdef RG_TARGET_ODROID_GO
#undef RG_BASE_PATH_SAVES
#define RG_BASE_PATH_SAVES     RG_ROOT_PATH "/odroid/data"
#endif

bool rg_sdcard_mount(void);
bool rg_sdcard_unmount(void);
bool rg_sdcard_format(void);
void rg_sdcard_set_enable_activity_led(bool enable);
bool rg_sdcard_get_enable_activity_led(void);

bool rg_mkdir(const char *dir);

// These are similar to libgen's but they never modify the source
const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char *rg_extension(const char *path);
