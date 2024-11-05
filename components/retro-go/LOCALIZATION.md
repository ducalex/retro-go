This document describes the localization protocol used in Retro-go


# C files
rg_localization.c contains the original messages and the corresponding translations ex :
````c
{
    .msg = "Yes",
    .fr = "Oui"
},
````

This is what the struct definition looks like
````c
typedef struct {
    char *msg;   // Original message in english
    char *fr;    // FR Translated message
//  char *de;    // for adding DE translation for example
} Translation;
````

# Python tool
`rg_locate_str.py` is a simple python tool that locate every string preceded by `_("` pattern in each file of Retro-go project
Then the tool compare theses strings to the ones in `rg_localization.h` and put the missing ones in a .txt file called missing_translation.txt