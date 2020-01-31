#pragma once

#include <stdint.h>

char* odroid_util_GetFileName(const char* path);
char* odroid_util_GetFileExtenstion(const char* path);
char* odroid_util_GetFileNameWithoutExtension(const char* path);


typedef enum
{
    ODROID_START_ACTION_NORMAL = 0,
    ODROID_START_ACTION_RESTART
} ODROID_START_ACTION;

typedef enum
{
    ODROID_SCALE_DISABLE_NES = (1 << 0),
    ODROID_SCALE_DISABLE_GB = (1 << 1),
    ODROID_SCALE_DISABLE_SMS = (1 << 2)
} ODROID_SCALE_DISABLE;

typedef enum
{
    ODROID_AUDIO_SINK_SPEAKER = 0,
    ODROID_AUDIO_SINK_DAC
} ODROID_AUDIO_SINK;


int32_t odroid_settings_VRef_get();
void odroid_settings_VRef_set(int32_t value);

int32_t odroid_settings_Volume_get();
void odroid_settings_Volume_set(int32_t value);

char* odroid_settings_RomFilePath_get();
void odroid_settings_RomFilePath_set(char* value);

int32_t odroid_settings_AppSlot_get();
void odroid_settings_AppSlot_set(int32_t value);

int32_t odroid_settings_DataSlot_get();
void odroid_settings_DataSlot_set(int32_t value);

int32_t odroid_settings_Backlight_get();
void odroid_settings_Backlight_set(int32_t value);

ODROID_START_ACTION odroid_settings_StartAction_get();
void odroid_settings_StartAction_set(ODROID_START_ACTION value);

uint8_t odroid_settings_ScaleDisabled_get(ODROID_SCALE_DISABLE system);
void odroid_settings_ScaleDisabled_set(ODROID_SCALE_DISABLE system, uint8_t value);

ODROID_AUDIO_SINK odroid_settings_AudioSink_get();
void odroid_settings_AudioSink_set(ODROID_AUDIO_SINK value);

int32_t odroid_settings_GBPalette_get();
void odroid_settings_GBPalette_set(int32_t value);