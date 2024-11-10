#pragma once

#include <string.h>
#include <stdbool.h>

#define _(String) rg_gettext(String)

enum languages
{
    RG_LANG_EN = 0,
    RG_LANG_FR,
  //RG_LANG_ES,

    RG_LANG_MAX
};

// Define a struct for holding message translations
typedef union
{
    struct {
      const char *msg;   // Original message in english
      const char *fr;    // FR Translated message
      //  const char *es;    // for adding ES translation for example
    };
    const char *msgs[RG_LANG_MAX];
} Translation;

// Lookup function
const char *rg_gettext(const char *msg);
int rg_localization_get_language_id(void);
bool rg_localization_set_language_id(int language_id);
const char *rg_localization_get_language_name(int language_id);
