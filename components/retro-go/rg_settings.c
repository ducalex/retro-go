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

// Global
static const char* Key_RomFilePath  = "RomFilePath";
static const char* Key_StartAction  = "StartAction";
static const char* Key_Backlight    = "Backlight";
static const char* Key_StartupApp   = "StartupApp";
static const char* Key_DiskActivity = "DiskActivity";
// static const char* Key_RetroGoVer   = "RetroGoVer";
// Per-app
static const char* Key_Region       = "Region";
static const char* Key_Palette      = "Palette";
static const char* Key_DispScaling  = "DispScaling";
static const char* Key_DispFilter   = "DispFilter";
static const char* Key_DispRotation = "DistRotation";
static const char* Key_DispOverscan = "DispOverscan";
static const char* Key_SpriteLimit  = "SpriteLimit";
// static const char* Key_AudioFilter  = "AudioFilter";

static int unsaved_changes = 0;
static bool initialized = false;
static cJSON *root = NULL;

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

    initialized = true;
}

bool rg_settings_save(void)
{
    if (unsaved_changes == 0)
        return true;

    char *buffer = cJSON_Print(root);
    bool success = false;

    if (buffer)
    {
    #if USE_CONFIG_FILE
        FILE *fp = fopen(CONFIG_FILE_PATH, "wb");
        if (fp)
        {
            success = (fputs(buffer, fp) > 0);
            fclose(fp);
        }
    #else
        nvs_set_str(my_handle, CONFIG_NVS_STORE, buffer);
        success = (nvs_commit(my_handle) == ESP_OK);
    #endif
        cJSON_free(buffer);
        unsaved_changes = 0;
    }

    return success;
}

void rg_settings_reset(void)
{
    cJSON_free(root);
    root = cJSON_CreateObject();
    unsaved_changes++;
    rg_settings_save();
}

char *rg_settings_string_get(const char *key, const char *default_value)
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

void rg_settings_string_set(const char *key, const char *value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to set key '%s' before rg_settings_init() was called!\n", key);
        return;
    }

    json_set(key, cJSON_CreateString(value));
    unsaved_changes++;
}

int32_t rg_settings_int32_get(const char *key, int32_t default_value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to get key '%s' before rg_settings_init() was called!\n", key);
        return default_value;
    }

    cJSON *obj = cJSON_GetObjectItem(root, key);

    return obj ? obj->valueint : default_value;
}

void rg_settings_int32_set(const char *key, int32_t value)
{
    if (!initialized)
    {
        RG_LOGW("Trying to set key '%s' before rg_settings_init() was called!\n", key);
        return;
    }
    else if (rg_settings_int32_get(key, ~value) == value)
    {
        return; // Do nothing, value identical
    }

    json_set(key, cJSON_CreateNumber(value));
    unsaved_changes++;
}


int32_t rg_settings_app_int32_get(const char *key, int32_t default_value)
{
    char app_key[16];
    sprintf(app_key, "%.12s.%u", key, rg_system_get_app()->id % 1000);
    return rg_settings_int32_get(app_key, default_value);
}

void rg_settings_app_int32_set(const char *key, int32_t value)
{
    char app_key[16];
    sprintf(app_key, "%.12s.%u", key, rg_system_get_app()->id % 1000);
    rg_settings_int32_set(app_key, value);
}


char* rg_settings_RomFilePath_get()
{
    return rg_settings_string_get(Key_RomFilePath, NULL);
}
void rg_settings_RomFilePath_set(const char* value)
{
    rg_settings_string_set(Key_RomFilePath, value);
}


int32_t rg_settings_Backlight_get()
{
    return rg_settings_int32_get(Key_Backlight, 2);
}
void rg_settings_Backlight_set(int32_t value)
{
    rg_settings_int32_set(Key_Backlight, value);
}


emu_start_action_t rg_settings_StartAction_get()
{
    return rg_settings_int32_get(Key_StartAction, 0);
}
void rg_settings_StartAction_set(emu_start_action_t value)
{
    rg_settings_int32_set(Key_StartAction, value);
}


int32_t rg_settings_StartupApp_get()
{
    return rg_settings_int32_get(Key_StartupApp, 1);
}
void rg_settings_StartupApp_set(int32_t value)
{
    rg_settings_int32_set(Key_StartupApp, value);
}


int32_t rg_settings_DiskActivity_get()
{
    return rg_settings_int32_get(Key_DiskActivity, 0);
}
void rg_settings_DiskActivity_set(int32_t value)
{
    rg_settings_int32_set(Key_DiskActivity, value);
}


int32_t rg_settings_Palette_get()
{
    return rg_settings_app_int32_get(Key_Palette, 0);
}
void rg_settings_Palette_set(int32_t value)
{
    rg_settings_app_int32_set(Key_Palette, value);
}


int32_t rg_settings_SpriteLimit_get()
{
    return rg_settings_app_int32_get(Key_SpriteLimit, 1);
}
void rg_settings_SpriteLimit_set(int32_t value)
{
    rg_settings_app_int32_set(Key_SpriteLimit, value);
}


emu_region_t rg_settings_Region_get()
{
    return rg_settings_app_int32_get(Key_Region, EMU_REGION_AUTO);
}
void rg_settings_Region_set(emu_region_t value)
{
    rg_settings_app_int32_set(Key_Region, value);
}


int32_t rg_settings_DisplayScaling_get()
{
    return rg_settings_app_int32_get(Key_DispScaling, RG_DISPLAY_SCALING_FILL);
}
void rg_settings_DisplayScaling_set(int32_t value)
{
    rg_settings_app_int32_set(Key_DispScaling, value);
}


int32_t rg_settings_DisplayFilter_get()
{
    return rg_settings_app_int32_get(Key_DispFilter, RG_DISPLAY_FILTER_OFF);
}
void rg_settings_DisplayFilter_set(int32_t value)
{
    rg_settings_app_int32_set(Key_DispFilter, value);
}


int32_t rg_settings_DisplayRotation_get()
{
    return rg_settings_app_int32_get(Key_DispRotation, RG_DISPLAY_ROTATION_AUTO);
}
void rg_settings_DisplayRotation_set(int32_t value)
{
    rg_settings_app_int32_set(Key_DispRotation, value);
}


int32_t rg_settings_DisplayOverscan_get()
{
    return rg_settings_app_int32_get(Key_DispOverscan, 1);
}
void rg_settings_DisplayOverscan_set(int32_t value)
{
    rg_settings_app_int32_set(Key_DispOverscan, value);
}
