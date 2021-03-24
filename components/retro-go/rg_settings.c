#include <esp_err.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "rg_system.h"
#include "rg_settings.h"

#define USE_CONFIG_FILE 1

#define CONFIG_FILE_PATH "/sd/odroid/retro-go.json"
#define CONFIG_NAMESPACE "retro-go"
#define CONFIG_NVS_STORE "config"

// static const char *Key_RetroGoVer   = "RetroGoVer";

static int unsaved_changes = 0;
static bool initialized = false;
static cJSON *root = NULL;
static cJSON *app_root = NULL;

#if !USE_CONFIG_FILE
    #include <nvs_flash.h>
    static nvs_handle my_handle = 0;
#endif


static inline void json_set(const char *key, cJSON *value)
{
    cJSON *obj = cJSON_GetObjectItem(root, key);
    if (obj == NULL)
        cJSON_AddItemToObject(root, key, value);
    else
        cJSON_ReplaceItemInObject(root, key, value);
}


void rg_settings_init()
{
    char *buffer = NULL;
    char *source;
    size_t length = 0;

#if USE_CONFIG_FILE
    FILE *fp = fopen(CONFIG_FILE_PATH, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = calloc(1, length + 1);
        fread(buffer, 1, length, fp);
        fclose(fp);
    }
    source = "SD Card";
#else
    if (nvs_flash_init() != ESP_OK)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    if (nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &my_handle) == ESP_OK)
    {
        if (nvs_get_str(my_handle, CONFIG_NVS_STORE, NULL, &length) == ESP_OK)
        {
            buffer = calloc(1, length + 1);
            nvs_get_str(my_handle, CONFIG_NVS_STORE, buffer, &length);
        }
    }
    source = "NVS";
#endif

    if (buffer)
    {
        root = cJSON_Parse(buffer);
        free(buffer);
    }

    if (root)
    {
        RG_LOGI("Config loaded from %s!\n", source);
    }
    else
    {
        RG_LOGE("Config failed to load from %s!\n", source);
        root = cJSON_CreateObject();
    }

    app_root = NULL;

    initialized = true;
}

bool rg_settings_save(void)
{
    if (unsaved_changes == 0)
        return true;

    char *buffer = cJSON_Print(root);
    if (!buffer)
    {
        RG_LOGE("cJSON_Print() failed.\n");
        return false;
    }

#if USE_CONFIG_FILE
    FILE *fp = fopen(CONFIG_FILE_PATH, "wb");
    if (!fp)
    {
        // Sometimes the FAT is left in an inconsistent state and this might help
        rg_vfs_delete(CONFIG_FILE_PATH);
        fp = fopen(CONFIG_FILE_PATH, "wb");
    }
    if (fp)
    {
        if (fputs(buffer, fp) > 0)
            unsaved_changes = 0;
        fclose(fp);
    }
#else
    if (nvs_set_str(my_handle, CONFIG_NVS_STORE, buffer) == ESP_OK
        && nvs_commit(my_handle) == ESP_OK) unsaved_changes = 0;
#endif

    cJSON_free(buffer);

    return (unsaved_changes == 0);
}

void rg_settings_reset(void)
{
    cJSON_free(root);
    root = cJSON_CreateObject();
    unsaved_changes++;
    rg_settings_save();
}

char *rg_settings_get_string(const char *key, const char *default_value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to get key '%s' before rg_settings_init() was called!\n", key);
    }
    else
    {
        cJSON *obj = cJSON_GetObjectItem(root, key);
        if (obj != NULL && obj->valuestring != NULL)
        {
            return strdup(obj->valuestring);
        }
    }

    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *key, const char *value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to set key '%s' before rg_settings_init() was called!\n", key);
        return;
    }

    json_set(key, cJSON_CreateString(value));
    unsaved_changes++;
}

int32_t rg_settings_get_int32(const char *key, int32_t default_value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to get key '%s' before rg_settings_init() was called!\n", key);
        return default_value;
    }

    cJSON *obj = cJSON_GetObjectItem(root, key);

    return obj ? obj->valueint : default_value;
}

void rg_settings_set_int32(const char *key, int32_t value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to set key '%s' before rg_settings_init() was called!\n", key);
        return;
    }
    else if (rg_settings_get_int32(key, ~value) == value)
    {
        return; // Do nothing, value identical
    }

    json_set(key, cJSON_CreateNumber(value));
    unsaved_changes++;
}


int32_t rg_settings_get_app_int32(const char *key, int32_t default_value)
{
    char app_key[32];
    snprintf(app_key, 32, "%.16s.%u", key, rg_system_get_app()->id);
    return rg_settings_get_int32(app_key, default_value);
}

void rg_settings_set_app_int32(const char *key, int32_t value)
{
    char app_key[32];
    snprintf(app_key, 32, "%.16s.%u", key, rg_system_get_app()->id);
    rg_settings_set_int32(app_key, value);
}


char *rg_settings_get_app_string(const char *key, const char *default_value)
{
    char app_key[32];
    snprintf(app_key, 32, "%.16s.%u", key, rg_system_get_app()->id);
    return rg_settings_get_string(app_key, default_value);
}

void rg_settings_set_app_string(const char *key, const char *value)
{
    char app_key[32];
    snprintf(app_key, 32, "%.16s.%u", key, rg_system_get_app()->id);
    rg_settings_set_string(app_key, value);
}
