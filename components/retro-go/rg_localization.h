#pragma once

#include <string.h>
#include <stdbool.h>

#define _(String) rg_gettext(String)

typedef enum
{
    RG_LANG_EN = 0,
    RG_LANG_FR,
    RG_LANG_CHS,
  //RG_LANG_ES,

    RG_LANG_MAX
} rg_language_t;

// Lookup function
const char *rg_gettext(const char *msg);
int rg_localization_get_language_id(void);
bool rg_localization_set_language_id(int language_id);
const char *rg_localization_get_language_name(int language_id);
