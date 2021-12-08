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

#define NS_GLOBAL  "global"
#define NS_APP     rg_system_get_app()->name
#define NS_FILE    rg_system_get_app()->romPath

void rg_storage_init(void);
void rg_storage_deinit(void);
bool rg_storage_format(void);
bool rg_storage_ready(void);
void rg_storage_commit(void);
void rg_storage_set_activity_led(bool enable);
bool rg_storage_get_activity_led(void);

void rg_settings_reset(void);
double rg_settings_get_number(const char *section, const char *key, double default_value);
void rg_settings_set_number(const char *section, const char *key, double value);
void rg_settings_set_string(const char *section, const char *key, const char *value);
char*rg_settings_get_string(const char *section, const char *key, const char *default_value);
void rg_settings_delete(const char *section, const char *key);

bool rg_mkdir(const char *dir);
const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char *rg_extension(const char *path);
