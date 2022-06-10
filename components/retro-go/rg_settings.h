#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// TO DO: This should be an enum, not strings and even less a function call...
#define NS_GLOBAL  "global"
#define NS_APP     rg_system_get_app()->configNs
#define NS_FILE    rg_system_get_app()->romPath

void rg_settings_init(void);
void rg_settings_commit(void);
void rg_settings_reset(void);
double rg_settings_get_number(const char *section, const char *key, double default_value);
void rg_settings_set_number(const char *section, const char *key, double value);
void rg_settings_set_string(const char *section, const char *key, const char *value);
char*rg_settings_get_string(const char *section, const char *key, const char *default_value);
void rg_settings_delete(const char *section, const char *key);
