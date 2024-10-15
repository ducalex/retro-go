#pragma once

#include <string.h>
#include <stdbool.h>

#define _(String) rg_gettext(String)
#define RG_LANGUAGE_MAX 2 // Update accordingly to Translation struct

// Define a struct for holding message translations
typedef struct {
    char *msg;   // Original message in english
    char *fr;    // FR Translated message
//  char *de;    // for adding DE translation for example
} Translation;

// Lookup function
const char* rg_gettext(const char *msg);
int rg_localization_get_language_id(void);
bool rg_localization_set_language_id(int language_id);