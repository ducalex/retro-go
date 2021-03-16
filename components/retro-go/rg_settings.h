#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    EMU_START_ACTION_RESUME = 0,
    EMU_START_ACTION_NEWGAME,
    EMU_START_ACTION_NETPLAY
} emu_start_action_t;

void rg_settings_init(void);
void rg_settings_reset(void);
bool rg_settings_load(void);
bool rg_settings_save(void);

void rg_settings_set_string(const char *key, const char *value);
char* rg_settings_get_string(const char *key, const char *default_value);

int32_t rg_settings_get_int32(const char *key, int32_t value_default);
void rg_settings_set_int32(const char *key, int32_t value);

int32_t rg_settings_get_app_int32(const char *key, int32_t value_default);
void rg_settings_set_app_int32(const char *key, int32_t value);


char* rg_settings_RomFilePath_get();
void rg_settings_RomFilePath_set(const char* value);

int32_t rg_settings_StartupApp_get();
void rg_settings_StartupApp_set(int32_t value);

emu_start_action_t rg_settings_StartAction_get();
void rg_settings_StartAction_set(emu_start_action_t value);

int32_t rg_settings_DiskActivity_get();
void rg_settings_DiskActivity_set(int32_t value);
