#include "rg_system.h"
#include "rg_cheats.h"
#if 0
static rg_cheat_t **cheats;


void rg_cheats_enter_code_menu(void)
{
    // NES game genie:  AAAAAAAA        A-Z
    // GB game genie:   AAA-AAA-AAA     hex
    // GG game genie:   AAA-AAA-AAA     hex
    // GBC game shark:  AAAAAAAA        hex

    char code[] = {'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0};

    const dialog_option_t choices[] = {
        {10, "Type", NULL, 1, NULL}, // GG, GS
        {10, "1", code + 0, 1, NULL},
        {10, "2", code + 2, 1, NULL},
        {10, "3", code + 4, 1, NULL},
        {10, "4", code + 6, 1, NULL},
        {10, "5", code + 8, 1, NULL},
        {10, "6", code + 10, 1, NULL},
        {10, "7", code + 12, 1, NULL},
        {10, "8", code + 14, 1, NULL},
        {10, "9", code + 16, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {10, "Apply", NULL,  1, NULL},
        // here we list known cheats for the game
        RG_DIALOG_CHOICE_LAST
    };

    int r = rg_gui_dialog("Enter code", choices, 0);

    // ...
}
#endif
