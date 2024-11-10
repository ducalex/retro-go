#include "rg_localization.h"
#include "translations.h"

static int rg_language = RG_LANG_EN;

int rg_localization_get_language_id(void)
{
    return rg_language;
}

bool rg_localization_set_language_id(int language_id)
{
    if (language_id < 0 || language_id > RG_LANG_MAX - 1)
        return false;

    rg_language = language_id;
    return true;
}

const char *rg_gettext(const char *text)
{
    if (text == NULL)
        return NULL;

    if (rg_language <= 0 || rg_language >= RG_LANG_MAX)
        return text; // If rg_language is english or invalid, we return the original string

    for (int i = 0; translations[i].msg != NULL; i++)
    {
        if (strcmp(translations[i].msg, text) == 0)
        {
            const char *msg = translations[i].msgs[rg_language];
            // If the translation is missing, we return the original string
            return msg ? msg : text;
        }
    }

    return text; // if no translation found
}

const char *rg_localization_get_language_name(int language_id)
{
    if (language_id < 0 || language_id > RG_LANG_MAX - 1)
        return NULL;
    return language_names[language_id];
}
