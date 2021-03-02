#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    EMU_START_ACTION_RESUME = 0,
    EMU_START_ACTION_NEWGAME,
    EMU_START_ACTION_NETPLAY
} emu_start_action_t;

typedef enum
{
    EMU_REGION_AUTO = 0,
    EMU_REGION_NTSC,
    EMU_REGION_PAL
} emu_region_t;

void rg_settings_init(void);
void rg_settings_reset(void);
bool rg_settings_load(void);
bool rg_settings_save(void);

char* rg_settings_RomFilePath_get();
void rg_settings_RomFilePath_set(const char* value);

int32_t rg_settings_Backlight_get();
void rg_settings_Backlight_set(int32_t value);

int32_t rg_settings_StartupApp_get();
void rg_settings_StartupApp_set(int32_t value);

emu_start_action_t rg_settings_StartAction_get();
void rg_settings_StartAction_set(emu_start_action_t value);

int32_t rg_settings_DiskActivity_get();
void rg_settings_DiskActivity_set(int32_t value);

emu_region_t rg_settings_Region_get();
void rg_settings_Region_set(emu_region_t value);

int32_t rg_settings_Palette_get();
void rg_settings_Palette_set(int32_t value);

int32_t rg_settings_SpriteLimit_get();
void rg_settings_SpriteLimit_set(int32_t value);

int32_t rg_settings_DisplayScaling_get();
void rg_settings_DisplayScaling_set(int32_t value);

int32_t rg_settings_DisplayFilter_get();
void rg_settings_DisplayFilter_set(int32_t value);

int32_t rg_settings_DisplayRotation_get();
void rg_settings_DisplayRotation_set(int32_t value);

int32_t rg_settings_DisplayOverscan_get();
void rg_settings_DisplayOverscan_set(int32_t value);

/*** Generic functions ***/

void rg_settings_string_set(const char *key, const char *value);
char* rg_settings_string_get(const char *key, const char *default_value);

int32_t rg_settings_int32_get(const char *key, int32_t value_default);
void rg_settings_int32_set(const char *key, int32_t value);

int32_t rg_settings_app_int32_get(const char *key, int32_t value_default);
void rg_settings_app_int32_set(const char *key, int32_t value);
