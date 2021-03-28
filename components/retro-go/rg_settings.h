#pragma once

#include <stdbool.h>
#include <stdint.h>

void rg_settings_init(const char *app_name);
void rg_settings_reset(void);
bool rg_settings_load(void);
bool rg_settings_save(void);
bool rg_settings_ready(void);
void rg_settings_set_app_name(const char *app_name);

void rg_settings_set_string(const char *key, const char *value);
char* rg_settings_get_string(const char *key, const char *default_value);

void rg_settings_set_app_string(const char *key, const char *value);
char *rg_settings_get_app_string(const char *key, const char *default_value);

int32_t rg_settings_get_int32(const char *key, int32_t value_default);
void rg_settings_set_int32(const char *key, int32_t value);

int32_t rg_settings_get_app_int32(const char *key, int32_t value_default);
void rg_settings_set_app_int32(const char *key, int32_t value);
