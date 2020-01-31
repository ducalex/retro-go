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
static const char* NvsKey_ScaleDisabled = "ScaleDisabled";
static const char* NvsKey_AudioSink = "AudioSink";
static const char* NvsKey_GBPalette = "GBPalette";

static nvs_handle my_handle;

void odroid_settings_init()
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
}

int32_t odroid_settings_VRef_get()
{
    int32_t result = 1100;

	// Read
	esp_err_t err = nvs_get_i32(my_handle, NvsKey_VRef, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_VRefGet: value=%d\n", result);
	}

    return result;
}
void odroid_settings_VRef_set(int32_t value)
{
    // Write key
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_VRef, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_commit(my_handle);
}


int32_t odroid_settings_Volume_get()
{
    int result = ODROID_VOLUME_LEVEL3;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_Volume, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_Volume_get: value=%d\n", result);
    }

    return result;
}
void odroid_settings_Volume_set(int32_t value)
{
    // Read
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_Volume, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
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

        printf("odroid_settings_RomFilePathGet: value='%s'\n", value);
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


int32_t odroid_settings_AppSlot_get()
{
    int32_t result = -1;

	// Read
	esp_err_t err = nvs_get_i32(my_handle, NvsKey_AppSlot, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_AppSlot_get: value=%d\n", result);
	}

    return result;
}
void odroid_settings_AppSlot_set(int32_t value)
{
    // Write key
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_AppSlot, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


int32_t odroid_settings_DataSlot_get()
{
    int32_t result = -1;

	// Read
	esp_err_t err = nvs_get_i32(my_handle, NvsKey_DataSlot, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_DataSlot_get: value=%d\n", result);
	}

    return result;
}
void odroid_settings_DataSlot_set(int32_t value)
{
    // Write key
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_DataSlot, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


int32_t odroid_settings_Backlight_get()
{
	// TODO: Move to header
    int result = 2;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_Backlight, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_Backlight_get: value=%d\n", result);
    }

    return result;
}
void odroid_settings_Backlight_set(int32_t value)
{
    // Read
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_Backlight, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


ODROID_START_ACTION odroid_settings_StartAction_get()
{
    int result = 0;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_StartAction, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    return result;
}
void odroid_settings_StartAction_set(ODROID_START_ACTION value)
{
    // Write
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_StartAction, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


uint8_t odroid_settings_ScaleDisabled_get(ODROID_SCALE_DISABLE system)
{
    int result = 0;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_ScaleDisabled, &result);
    if (err == ESP_OK)
    {
        printf("%s: result=%d\n", __func__, result);
    }

    return (result & system) ? 1 : 0;
}
void odroid_settings_ScaleDisabled_set(ODROID_SCALE_DISABLE system, uint8_t value)
{
	printf("%s: system=%#010x, value=%d\n", __func__, system, value);

	// Read
	int result = 0;
	esp_err_t err = nvs_get_i32(my_handle, NvsKey_ScaleDisabled, &result);
	if (err == ESP_OK)
	{
		printf("%s: result=%d\n", __func__, result);
	}

	// set system flag
	//result = 1;
	result &= ~system;
	//printf("%s: result=%d\n", __func__, result);
	result |= (system & (value ? 0xffffffff : 0));
	printf("%s: set result=%d\n", __func__, result);

    // Write
    err = nvs_set_i32(my_handle, NvsKey_ScaleDisabled, result);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}

ODROID_AUDIO_SINK odroid_settings_AudioSink_get()
{
    int result = ODROID_AUDIO_SINK_SPEAKER;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_AudioSink, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    return (ODROID_AUDIO_SINK)result;
}
void odroid_settings_AudioSink_set(ODROID_AUDIO_SINK value)
{
    // Write
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_AudioSink, (int)value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}


int32_t odroid_settings_GBPalette_get()
{
	// TODO: Move to header
    int result = 0;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, NvsKey_GBPalette, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_GBPalette_get: value=%d\n", result);
    }

    return result;
}
void odroid_settings_GBPalette_set(int32_t value)
{
    // Read
    esp_err_t err = nvs_set_i32(my_handle, NvsKey_GBPalette, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}

int32_t odroid_settings_int32_get(const char *key, int32_t value_default)
{
    // TODO: Move to header
    int result = value_default;

    // Read
    esp_err_t err = nvs_get_i32(my_handle, key, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    return result;
}
void odroid_settings_int32_set(const char *key, int32_t value)
{
    // Read
    esp_err_t err = nvs_set_i32(my_handle, key, value);
    if (err != ESP_OK) abort();

    nvs_commit(my_handle);
}
