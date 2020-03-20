#pragma once

#include <stdint.h>

typedef enum
{
    ODROID_START_ACTION_RESUME = 0,
    ODROID_START_ACTION_RESTART,
    ODROID_START_ACTION_NETPLAY
} ODROID_START_ACTION;

typedef enum
{
    ODROID_AUDIO_SINK_SPEAKER = 0,
    ODROID_AUDIO_SINK_DAC
} ODROID_AUDIO_SINK;

typedef enum
{
    ODROID_REGION_AUTO = 0,
    ODROID_REGION_NTSC,
    ODROID_REGION_PAL
} ODROID_REGION;

void odroid_settings_init(int app_id);

int32_t odroid_settings_VRef_get();
void odroid_settings_VRef_set(int32_t value);

int32_t odroid_settings_Volume_get();
void odroid_settings_Volume_set(int32_t value);

char* odroid_settings_RomFilePath_get();
void odroid_settings_RomFilePath_set(char* value);

int32_t odroid_settings_DataSlot_get();
void odroid_settings_DataSlot_set(int32_t value);

int32_t odroid_settings_Backlight_get();
void odroid_settings_Backlight_set(int32_t value);

ODROID_START_ACTION odroid_settings_StartAction_get();
void odroid_settings_StartAction_set(ODROID_START_ACTION value);

ODROID_AUDIO_SINK odroid_settings_AudioSink_get();
void odroid_settings_AudioSink_set(ODROID_AUDIO_SINK value);

ODROID_REGION odroid_settings_Region_get();
void odroid_settings_Region_set(ODROID_REGION value);

int32_t odroid_settings_Palette_get();
void odroid_settings_Palette_set(int32_t value);

int32_t odroid_settings_DisplayScaling_get();
void odroid_settings_DisplayScaling_set(int32_t value);

int32_t odroid_settings_DisplayFilter_get();
void odroid_settings_DisplayFilter_set(int32_t value);

void odroid_settings_string_set(const char *key, char *value);
char* odroid_settings_string_get(const char *key, char *default_value);

int32_t odroid_settings_int32_get(const char *key, int32_t value_default);
void odroid_settings_int32_set(const char *key, int32_t value);

int32_t odroid_settings_app_int32_get(const char *key, int32_t value_default);
void odroid_settings_app_int32_set(const char *key, int32_t value);
