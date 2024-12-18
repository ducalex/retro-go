#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NS_GLOBAL ((char *)0)
#define NS_APP    ((char *)1)
#define NS_FILE   ((char *)2)
#define NS_WIFI   ((char *)3)
#define NS_BOOT   ((char *)4)

// Safe mode means no loading/saving config files
void rg_settings_init(bool safe_mode);
void rg_settings_commit(void);
void rg_settings_reset(void);
bool rg_settings_get_boolean(const char *section, const char *key, bool default_value);
void rg_settings_set_boolean(const char *section, const char *key, bool value);
double rg_settings_get_number(const char *section, const char *key, double default_value);
void rg_settings_set_number(const char *section, const char *key, double value);
char *rg_settings_get_string(const char *section, const char *key, const char *default_value);
void rg_settings_set_string(const char *section, const char *key, const char *value);
void rg_settings_delete(const char *section, const char *key);
bool rg_settings_exists(const char *section, const char *key);
