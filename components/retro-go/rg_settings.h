#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NS_GLOBAL ((char *)0)
#define NS_APP    ((char *)1)
#define NS_FILE   ((char *)2)
#define NS_WIFI   ((char *)3)
#define NS_BOOT   ((char *)4)

void rg_settings_init(void);
void rg_settings_commit(void);
void rg_settings_reset(void);
double rg_settings_get_number(const char *section, const char *key, double default_value);
void rg_settings_set_number(const char *section, const char *key, double value);
void rg_settings_set_string(const char *section, const char *key, const char *value);
char *rg_settings_get_string(const char *section, const char *key, const char *default_value);
void rg_settings_delete(const char *section, const char *key);
