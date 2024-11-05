#include "rg_localization.h"
#include "translations.h"

enum languages rg_language = RG_LANG_EN;

int rg_localization_get_language_id(void)
{
    return rg_language;
}

bool rg_localization_set_language_id(int language_id)
{
    if (language_id < 0 || language_id > RG_LANGUAGE_MAX - 1)
        return false;

    rg_language = language_id;
    return true;
}

const char* rg_gettext(const char *text)
{
    if (rg_language == RG_LANG_EN)
        return text; // in case language == 0 == english -> return the original string

    for (int i = 0; translations[i].msg != NULL; i++)
    {
        if (strcmp(translations[i].msg, text) == 0)
        {
            switch (rg_language)
            {
            case RG_LANG_FR:
                return translations[i].fr;  // Return the french string
                break;
            }
        }
    }
    return text; // if no translation found
}