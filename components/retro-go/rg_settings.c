#include <esp_err.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "rg_system.h"
#include "rg_settings.h"

#define USE_CONFIG_FILE (RG_DRIVER_SETTINGS == 1)

#define CONFIG_FILE_PATH RG_BASE_PATH_CONFIG "/retro-go.json"
#define CONFIG_NAMESPACE "retro-go"
#define CONFIG_NVS_STORE "config"
#define CONFIG_VERSION    0x01

#if !USE_CONFIG_FILE
    #include <nvs_flash.h>
    static nvs_handle my_handle = 0;
#endif

static int unsaved_changes = 0;
static cJSON *root = NULL;
static cJSON *app_root = NULL;


static cJSON *json_get(cJSON *root, const char *key)
{
    if (!root)
    {
        RG_LOGW("Trying to get key '%s' before rg_settings_init() was called!\n", key);
        return NULL;
    }

    return cJSON_GetObjectItem(root, key);
}

static void json_set(cJSON *root, const char *key, cJSON *value)
{
    if (!root)
    {
        RG_LOGW("Trying to set key '%s' before rg_settings_init() was called!\n", key);
        return;
    }

    cJSON *obj = cJSON_GetObjectItem(root, key);

    if (obj == NULL)
    {
        cJSON_AddItemToObject(root, key, value);
        unsaved_changes++;
    }
    else if (cJSON_Compare(obj, value, 0) == false)
    {
        // cJSON_ReplaceItemViaPointer(root, obj, value);
        cJSON_ReplaceItemInObject(root, key, value);
        unsaved_changes++;
    }
}


void rg_settings_init(const char *app_name)
{
    char *buffer = NULL;
    const char *source;
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
    source = "sdcard";
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
        RG_LOGI("Settings ready. source=%s\n", source);
    }
    else
    {
        RG_LOGE("Failed to initialize settings. source=%s!\n", source);
        root = cJSON_CreateObject();
    }

    rg_settings_set_app_name(app_name);

    json_set(root, "version", cJSON_CreateNumber(CONFIG_VERSION));
}

void rg_settings_set_app_name(const char *app_name)
{
    app_name = app_name ? app_name : "__app__";
    app_root = cJSON_GetObjectItem(root, app_name);

    if (!app_root)
    {
        app_root = cJSON_AddObjectToObject(root, app_name);
    }
    else if (!cJSON_IsObject(app_root))
    {
        app_root = cJSON_CreateObject();
        cJSON_ReplaceItemInObject(root, app_name, app_root);
    }
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
        unlink(CONFIG_FILE_PATH);
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
    cJSON_Delete(root);
    root = cJSON_CreateObject();
    unsaved_changes++;
    rg_settings_save();
}

bool rg_settings_ready(void)
{
    return root != NULL;
}

int32_t rg_settings_get_int32(const char *key, int32_t default_value)
{
    cJSON *obj = json_get(root, key);
    return obj ? obj->valueint : default_value;
}

void rg_settings_set_int32(const char *key, int32_t value)
{
    json_set(root, key, cJSON_CreateNumber(value));
}

char *rg_settings_get_string(const char *key, const char *default_value)
{
    cJSON *obj = json_get(root, key);

    if (obj && obj->valuestring)
        return strdup(obj->valuestring);

    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *key, const char *value)
{
    json_set(root, key, cJSON_CreateString(value));
}

int32_t rg_settings_get_app_int32(const char *key, int32_t default_value)
{
    cJSON *obj = json_get(app_root, key);
    return obj ? obj->valueint : default_value;
}

void rg_settings_set_app_int32(const char *key, int32_t value)
{
    json_set(app_root, key, cJSON_CreateNumber(value));
}

char *rg_settings_get_app_string(const char *key, const char *default_value)
{
    cJSON *obj = json_get(app_root, key);

    if (obj && obj->valuestring)
        return strdup(obj->valuestring);

    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_app_string(const char *key, const char *value)
{
    json_set(app_root, key, cJSON_CreateString(value));
}
