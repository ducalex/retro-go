#include "odroid_settings.h"

#include "nvs_flash.h"
#include "esp_heap_caps.h"

#include "string.h"

#include "odroid_audio.h"

static const char* NvsNamespace = "Odroid";

static const char* NvsKey_RomFilePath = "RomFilePath";
static const char* NvsKey_Volume = "Volume";
static const char* NvsKey_VRef = "VRef";
static const char* NvsKey_AppSlot = "AppSlot";
static const char* NvsKey_DataSlot = "DataSlot";
static const char* NvsKey_Backlight = "Backlight";
static const char* NvsKey_StartAction = "StartAction";
static const char* NvsKey_Scaling = "Scaling";
static const char* NvsKey_AudioSink = "AudioSink";
static const char* NvsKey_Palette = "Palette";

static nvs_handle my_handle;
static int app_id = 0;

void odroid_settings_init(int _app_id)
{
    // NVS
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        nvs_flash_erase();
        if (nvs_flash_init() != ESP_OK) {
            printf("odroid_system_init: Failed to init NVS");
            abort();
        }
    }

	err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) abort();

    app_id = _app_id;
}


char* odroid_settings_RomFilePath_get()
{
    char* result = NULL;

	// Read
    size_t required_size;
    esp_err_t err = nvs_get_str(my_handle, NvsKey_RomFilePath, NULL, &required_size);
    if (err == ESP_OK)
    {
        char* value = malloc(required_size);
        if (!value) abort();

        esp_err_t err = nvs_get_str(my_handle, NvsKey_RomFilePath, value, &required_size);
        if (err != ESP_OK) abort();

        result = value;

        printf("%s: value='%s'\n", __func__, result);
    }

    return result;
}
void odroid_settings_RomFilePath_set(char* value)
{
    // Write key
    esp_err_t err = nvs_set_str(my_handle, NvsKey_RomFilePath, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


int32_t odroid_settings_VRef_get()
{
	return odroid_settings_int32_get(NvsKey_VRef, 1100);
}
void odroid_settings_VRef_set(int32_t value)
{
    odroid_settings_int32_set(NvsKey_VRef, value);
}


int32_t odroid_settings_Volume_get()
{
    return odroid_settings_int32_get(NvsKey_Volume, ODROID_VOLUME_LEVEL3);
}
void odroid_settings_Volume_set(int32_t value)
{
    odroid_settings_int32_set(NvsKey_Volume, value);
}


ODROID_AUDIO_SINK odroid_settings_AudioSink_get()
{
    return odroid_settings_int32_get(NvsKey_AudioSink, ODROID_AUDIO_SINK_SPEAKER);
}
void odroid_settings_AudioSink_set(ODROID_AUDIO_SINK value)
{
    odroid_settings_int32_set(NvsKey_AudioSink, value);
}


int32_t odroid_settings_DataSlot_get()
{
    return odroid_settings_int32_get(NvsKey_DataSlot, -1);
}
void odroid_settings_DataSlot_set(int32_t value)
{
    odroid_settings_int32_set(NvsKey_DataSlot, value);
}


int32_t odroid_settings_Backlight_get()
{
    return odroid_settings_int32_get(NvsKey_Backlight, 2);
}
void odroid_settings_Backlight_set(int32_t value)
{
    odroid_settings_int32_set(NvsKey_Backlight, value);
}


ODROID_START_ACTION odroid_settings_StartAction_get()
{
    return odroid_settings_int32_get(NvsKey_StartAction, 0);
}
void odroid_settings_StartAction_set(ODROID_START_ACTION value)
{
    odroid_settings_int32_set(NvsKey_StartAction, value);
}


ODROID_SCALING odroid_settings_Scaling_get()
{
    return odroid_settings_app_int32_get(NvsKey_Scaling, ODROID_SCALING_FILL);
}
void odroid_settings_Scaling_set(ODROID_SCALING value)
{
    odroid_settings_app_int32_set(NvsKey_Scaling, value);
}


int32_t odroid_settings_Palette_get()
{
    return odroid_settings_app_int32_get(NvsKey_Palette, 0);
}
void odroid_settings_Palette_set(int32_t value)
{
    odroid_settings_app_int32_set(NvsKey_Palette, value);
}


int32_t odroid_settings_int32_get(const char *key, int32_t default_value)
{
    int result = default_value;

    esp_err_t err = nvs_get_i32(my_handle, key, &result);
    printf("odroid_settings_int32_get: key='%s' value=%d, err=%d\n", key, result, err);

    return result;
}
void odroid_settings_int32_set(const char *key, int32_t value)
{
    esp_err_t err = nvs_set_i32(my_handle, key, value);
    printf("odroid_settings_int32_set: key='%s' value=%d, err=%d\n", key, value, err);

    nvs_commit(my_handle);
}


int32_t odroid_settings_app_int32_get(const char *key, int32_t default_value)
{
    char app_key[16];
    sprintf(app_key, "%s.%d", key, app_id);
    return odroid_settings_int32_get(app_key, default_value);
}
void odroid_settings_app_int32_set(const char *key, int32_t value)
{
    char app_key[16];
    sprintf(app_key, "%s.%d", key, app_id);
    odroid_settings_int32_set(app_key, value);
}
