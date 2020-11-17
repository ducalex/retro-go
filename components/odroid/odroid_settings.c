#include <string.h>

#include "odroid_system.h"
#include "odroid_settings.h"

#define USE_CONFIG_FILE

// Global
static const char* Key_RomFilePath  = "RomFilePath";
static const char* Key_StartAction  = "StartAction";
static const char* Key_Backlight    = "Backlight";
static const char* Key_AudioSink    = "AudioSink";
static const char* Key_Volume       = "Volume";
static const char* Key_StartupApp   = "StartupApp";
static const char* Key_FontSize     = "FontSize";
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

#ifdef USE_CONFIG_FILE
    #include <cJSON.h>

    static const char* config_file  = "/sd/odroid/retro-go.json";
    static cJSON *root = NULL;

    static inline void json_set(const char *key, cJSON *value)
    {
        cJSON *obj = cJSON_GetObjectItem(root, key);
        if (obj == NULL)
            cJSON_AddItemToObject(root, key, value);
        else
            cJSON_ReplaceItemInObject(root, key, value);
    }
#else
    #include <nvs_flash.h>

    static const char* NvsNamespace = "Odroid";
    static nvs_handle my_handle = NULL;
#endif

void odroid_settings_init()
{
#ifdef USE_CONFIG_FILE
    int size = odroid_sdcard_get_filesize(config_file);
    if (size > 0)
    {
        void *buffer = rg_alloc(size, MEM_ANY);
        odroid_sdcard_read_file(config_file, buffer, size);
        root = cJSON_Parse(buffer);
        free(buffer);
    }
    if (!root)
    {
        printf("odroid_system_init: Failed to load JSON config file!\n");
        root = cJSON_CreateObject();
    }
#else
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        nvs_flash_erase();
        if (nvs_flash_init() != ESP_OK)
        {
            RG_PANIC("Failed to init NVS!");
        }
    }
	err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK)
    {
        RG_PANIC("Failed to open NVS!");
    }
#endif
}

void odroid_settings_commit()
{
    if (unsaved_changes > 0)
    {
#ifdef USE_CONFIG_FILE
        char *buffer = cJSON_Print(root);
        if (buffer)
        {
            odroid_sdcard_write_file(config_file, buffer, strlen(buffer));
            free(buffer);
        }
#else
        nvs_commit(my_handle);
#endif
        unsaved_changes = 0;
    }
}

void odroid_settings_reset()
{
#ifdef USE_CONFIG_FILE
    cJSON_free(root);
    root = cJSON_CreateObject();
    unsaved_changes++;
    odroid_settings_commit();
#else
    // nvs_flash_deinit()
    // nvs_flash_erase()
    // odroid_settings_init();
#endif
}

char* odroid_settings_string_get(const char *key, const char *default_value)
{
#ifdef USE_CONFIG_FILE
    cJSON *obj = cJSON_GetObjectItem(root, key);
    if (obj != NULL && obj->valuestring != NULL)
    {
        return strdup(obj->valuestring); // stdup to be compatible with old nvs impl. below
    }
#else
    size_t required_size;
    esp_err_t err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err == ESP_OK)
    {
        char* buffer = rg_alloc(required_size, MEM_ANY);

        esp_err_t err = nvs_get_str(my_handle, key, buffer, &required_size);
        if (err == ESP_OK)
        {
            return buffer;
        }
    }
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        printf("%s: key='%s' err=%d\n", __func__, key, err);
    }
#endif
    if (default_value != NULL)
        return strdup(default_value);

    return NULL;
}

void odroid_settings_string_set(const char *key, const char *value)
{
#ifdef USE_CONFIG_FILE
    json_set(key, cJSON_CreateString(value));
#else
    esp_err_t ret = nvs_set_str(my_handle, key, value);
    if (ret != ESP_OK)
        printf("%s: key='%s' err=%d\n", __func__, key, ret);
#endif
    unsaved_changes++;
}

int32_t odroid_settings_int32_get(const char *key, int32_t default_value)
{
#ifdef USE_CONFIG_FILE
    cJSON *obj = cJSON_GetObjectItem(root, key);
    return obj ? obj->valueint : default_value;
#else
    int value = default_value;
    esp_err_t ret = nvs_get_i32(my_handle, key, &value);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND)
        printf("%s: key='%s' err=%d\n", __func__, key, ret);
    return value;
#endif
}

void odroid_settings_int32_set(const char *key, int32_t value)
{
    if (odroid_settings_int32_get(key, ~value) == value)
        return; // Do nothing
#ifdef USE_CONFIG_FILE
    json_set(key, cJSON_CreateNumber(value));
#else
    esp_err_t ret = nvs_set_i32(my_handle, key, value);
    if (ret != ESP_OK)
        printf("%s: key='%s' err=%d\n", __func__, key, ret);
#endif
    unsaved_changes++;
}


int32_t odroid_settings_app_int32_get(const char *key, int32_t default_value)
{
    char app_key[16];
    sprintf(app_key, "%.12s.%d", key, odroid_system_get_app()->id);
    return odroid_settings_int32_get(app_key, default_value);
}

void odroid_settings_app_int32_set(const char *key, int32_t value)
{
    char app_key[16];
    sprintf(app_key, "%.12s.%d", key, odroid_system_get_app()->id);
    odroid_settings_int32_set(app_key, value);
}


int32_t odroid_settings_FontSize_get()
{
    return odroid_settings_int32_get(Key_FontSize, 8);
}
void odroid_settings_FontSize_set(int32_t value)
{
    odroid_settings_int32_set(Key_FontSize, value);
}


char* odroid_settings_RomFilePath_get()
{
    return odroid_settings_string_get(Key_RomFilePath, NULL);
}
void odroid_settings_RomFilePath_set(const char* value)
{
    odroid_settings_string_set(Key_RomFilePath, value);
}


int32_t odroid_settings_Volume_get()
{
    return odroid_settings_int32_get(Key_Volume, ODROID_AUDIO_VOLUME_DEFAULT);
}
void odroid_settings_Volume_set(int32_t value)
{
    odroid_settings_int32_set(Key_Volume, value);
}


int32_t odroid_settings_AudioSink_get()
{
    return odroid_settings_int32_get(Key_AudioSink, ODROID_AUDIO_SINK_SPEAKER);
}
void odroid_settings_AudioSink_set(int32_t value)
{
    odroid_settings_int32_set(Key_AudioSink, value);
}


int32_t odroid_settings_Backlight_get()
{
    return odroid_settings_int32_get(Key_Backlight, 2);
}
void odroid_settings_Backlight_set(int32_t value)
{
    odroid_settings_int32_set(Key_Backlight, value);
}


ODROID_START_ACTION odroid_settings_StartAction_get()
{
    return odroid_settings_int32_get(Key_StartAction, 0);
}
void odroid_settings_StartAction_set(ODROID_START_ACTION value)
{
    odroid_settings_int32_set(Key_StartAction, value);
}


int32_t odroid_settings_StartupApp_get()
{
    return odroid_settings_int32_get(Key_StartupApp, 1);
}
void odroid_settings_StartupApp_set(int32_t value)
{
    odroid_settings_int32_set(Key_StartupApp, value);
}


int32_t odroid_settings_Palette_get()
{
    return odroid_settings_app_int32_get(Key_Palette, 0);
}
void odroid_settings_Palette_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_Palette, value);
}


int32_t odroid_settings_SpriteLimit_get()
{
    return odroid_settings_app_int32_get(Key_SpriteLimit, 1);
}
void odroid_settings_SpriteLimit_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_SpriteLimit, value);
}


ODROID_REGION odroid_settings_Region_get()
{
    return odroid_settings_app_int32_get(Key_Region, ODROID_REGION_AUTO);
}
void odroid_settings_Region_set(ODROID_REGION value)
{
    odroid_settings_app_int32_set(Key_Region, value);
}


int32_t odroid_settings_DisplayScaling_get()
{
    return odroid_settings_app_int32_get(Key_DispScaling, ODROID_DISPLAY_SCALING_FILL);
}
void odroid_settings_DisplayScaling_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_DispScaling, value);
}


int32_t odroid_settings_DisplayFilter_get()
{
    return odroid_settings_app_int32_get(Key_DispFilter, ODROID_DISPLAY_FILTER_OFF);
}
void odroid_settings_DisplayFilter_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_DispFilter, value);
}


int32_t odroid_settings_DisplayRotation_get()
{
    return odroid_settings_app_int32_get(Key_DispRotation, ODROID_DISPLAY_ROTATION_AUTO);
}
void odroid_settings_DisplayRotation_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_DispRotation, value);
}


int32_t odroid_settings_DisplayOverscan_get()
{
    return odroid_settings_app_int32_get(Key_DispOverscan, 1);
}
void odroid_settings_DisplayOverscan_set(int32_t value)
{
    odroid_settings_app_int32_set(Key_DispOverscan, value);
}
