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

char* odroid_util_GetFileName(const char* path)
{
	int length = strlen(path);
	int fileNameStart = length;

	if (fileNameStart < 1) abort();

	while (fileNameStart > 0)
	{
		if (path[fileNameStart] == '/')
		{
			++fileNameStart;
			break;
		}

		--fileNameStart;
	}

	int size = length - fileNameStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = path[fileNameStart + i];
	}

	//printf("GetFileName: result='%s'\n", result);

	return result;
}

char* odroid_util_GetFileExtenstion(const char* path)
{
	// Note: includes '.'
	int length = strlen(path);
	int extensionStart = length;

	if (extensionStart < 1) abort();

	while (extensionStart > 0)
	{
		if (path[extensionStart] == '.')
		{
			break;
		}

		--extensionStart;
	}

	int size = length - extensionStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = path[extensionStart + i];
	}

	//printf("GetFileExtenstion: result='%s'\n", result);

	return result;
}

char* odroid_util_GetFileNameWithoutExtension(const char* path)
{
	char* fileName = odroid_util_GetFileName(path);

	int length = strlen(fileName);
	int extensionStart = length;

	if (extensionStart < 1) abort();

	while (extensionStart > 0)
	{
		if (fileName[extensionStart] == '.')
		{
			break;
		}

		--extensionStart;
	}

	int size = extensionStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = fileName[i];
	}

	free(fileName);

	//printf("GetFileNameWithoutExtension: result='%s'\n", result);

	return result;
}


int32_t odroid_settings_VRef_get()
{
    int32_t result = 1100;

	// Open
	nvs_handle my_handle;
	esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) abort();


	// Read
	err = nvs_get_i32(my_handle, NvsKey_VRef, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_VRefGet: value=%d\n", result);
	}


	// Close
	nvs_close(my_handle);

    return result;
}
void odroid_settings_VRef_set(int32_t value)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write key
    err = nvs_set_i32(my_handle, NvsKey_VRef, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


int32_t odroid_settings_Volume_get()
{
    int result = ODROID_VOLUME_LEVEL3;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_Volume, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_Volume_get: value=%d\n", result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}
void odroid_settings_Volume_set(int32_t value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_set_i32(my_handle, NvsKey_Volume, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


char* odroid_settings_RomFilePath_get()
{
    char* result = NULL;

	// Open
	nvs_handle my_handle;
	esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) abort();


	// Read
    size_t required_size;
    err = nvs_get_str(my_handle, NvsKey_RomFilePath, NULL, &required_size);
    if (err == ESP_OK)
    {
        char* value = malloc(required_size);
        if (!value) abort();

        err = nvs_get_str(my_handle, NvsKey_RomFilePath, value, &required_size);
        if (err != ESP_OK) abort();

        result = value;

        printf("odroid_settings_RomFilePathGet: value='%s'\n", value);
    }


	// Close
	nvs_close(my_handle);

    return result;
}
void odroid_settings_RomFilePath_set(char* value)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write key
    err = nvs_set_str(my_handle, NvsKey_RomFilePath, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


int32_t odroid_settings_AppSlot_get()
{
    int32_t result = -1;

	// Open
	nvs_handle my_handle;
	esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) abort();


	// Read
	err = nvs_get_i32(my_handle, NvsKey_AppSlot, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_AppSlot_get: value=%d\n", result);
	}


	// Close
	nvs_close(my_handle);

    return result;
}
void odroid_settings_AppSlot_set(int32_t value)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write key
    err = nvs_set_i32(my_handle, NvsKey_AppSlot, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


int32_t odroid_settings_DataSlot_get()
{
    int32_t result = -1;

	// Open
	nvs_handle my_handle;
	esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) abort();


	// Read
	err = nvs_get_i32(my_handle, NvsKey_DataSlot, &result);
	if (err == ESP_OK)
    {
        printf("odroid_settings_DataSlot_get: value=%d\n", result);
	}


	// Close
	nvs_close(my_handle);

    return result;
}
void odroid_settings_DataSlot_set(int32_t value)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write key
    err = nvs_set_i32(my_handle, NvsKey_DataSlot, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


int32_t odroid_settings_Backlight_get()
{
	// TODO: Move to header
    int result = 2;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_Backlight, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_Backlight_get: value=%d\n", result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}
void odroid_settings_Backlight_set(int32_t value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_set_i32(my_handle, NvsKey_Backlight, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


ODROID_START_ACTION odroid_settings_StartAction_get()
{
    int result = 0;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_StartAction, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}
void odroid_settings_StartAction_set(ODROID_START_ACTION value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write
    err = nvs_set_i32(my_handle, NvsKey_StartAction, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


uint8_t odroid_settings_ScaleDisabled_get(ODROID_SCALE_DISABLE system)
{
    int result = 0;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_ScaleDisabled, &result);
    if (err == ESP_OK)
    {
        printf("%s: result=%d\n", __func__, result);
    }

    // Close
    nvs_close(my_handle);

    return (result & system) ? 1 : 0;
}
void odroid_settings_ScaleDisabled_set(ODROID_SCALE_DISABLE system, uint8_t value)
{
	printf("%s: system=%#010x, value=%d\n", __func__, system, value);

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

	// Read
	int result = 0;
	err = nvs_get_i32(my_handle, NvsKey_ScaleDisabled, &result);
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

    // Close
    nvs_close(my_handle);
}

ODROID_AUDIO_SINK odroid_settings_AudioSink_get()
{
    int result = ODROID_AUDIO_SINK_SPEAKER;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_AudioSink, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    // Close
    nvs_close(my_handle);

    return (ODROID_AUDIO_SINK)result;
}
void odroid_settings_AudioSink_set(ODROID_AUDIO_SINK value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Write
    err = nvs_set_i32(my_handle, NvsKey_AudioSink, (int)value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}


int32_t odroid_settings_GBPalette_get()
{
	// TODO: Move to header
    int result = 0;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_GBPalette, &result);
    if (err == ESP_OK)
    {
        printf("odroid_settings_GBPalette_get: value=%d\n", result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}
void odroid_settings_GBPalette_set(int32_t value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_set_i32(my_handle, NvsKey_GBPalette, value);
    if (err != ESP_OK) abort();

    // Close
    nvs_close(my_handle);
}