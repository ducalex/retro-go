This document describes the localization protocol used in Retro-go


# C files
translation.h contains the original messages (in english) and the corresponding translations ex:
````c
{
    [RG_LANG_EN] = "Yes",
    [RG_LANG_FR] = "Oui",
    [RG_LANG_ES] = "Si",
},
````

If you want to add your own language :

You should update the enum accordingly (in rg_localization.h):
````c
typedef enum
{
    RG_LANG_EN,
    RG_LANG_FR,

    RG_LANG_ES, // <-- to add spanish translation

    RG_LANG_MAX
} rg_language_t;
````


# Python tool
`rg_locate_str.py` is a simple python tool that locate every string preceded by `_("` pattern in each file of Retro-go project
Then the tool compare theses strings to the ones in `translations.h` and put the missing ones in a .txt file called missing_translation.txt
