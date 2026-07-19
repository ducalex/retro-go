// =====================================================================================================
// Stella DS/DSi Pheonix Edition - Improved Version by Dave Bernazzani (wavemotion)
//
// Copyright (c) 2020-2026 by Dave Bernazzani
//
// Copying and distribution of this emulator, it's source code and associated 
// readme files, with or without modification, are permitted in any medium without 
// royalty provided this copyright notice is used and wavemotion-dave (Phoenix-Edition),
// Alekmaul (original port) are thanked profusely along with the entire Stella Team.
//
// The StellaDS emulator is offered as-is, without any warranty.
// =====================================================================================================

#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <dirent.h>
#include <unistd.h>
#include "StellaDS.h"
#include "printf.h"
#include "Console.hxx"
#include "Cart.hxx"
#include "stella_config.h"
#include "bgBottom.h"
#include "bgInstructions.h"

// allConfigs handled by pointer in StellaDS.cpp

extern int bg0, bg0b,bg1b;

static int display_options_list(bool bFullDisplay);

#define CONFIG_INSTRUCTION_STR "B=EXIT STA=SAVE SEL=DEF L/R=MORE"

uInt8 OptionPage = 0;

// ---------------------------------------------------------------------------
// Write out the StellaDS.DAT configuration file to capture the settings for
// each game.  We can store more than 1500 game settings!
// ---------------------------------------------------------------------------
void SaveConfig(bool bShow)
{
    FILE *fp;
    int slot = 0;
    
    if (bShow) dsPrintValue(0,23,0, (char*)"     SAVING CONFIGURATION       ");

    // Set the global configuration version number...
    allConfigs.config_ver = CONFIG_VER;

    // Find the slot we should save into...
    for (slot=0; slot<MAX_CONFIGS; slot++)
    {
        if (strcmp(allConfigs.cart[slot].md5, myCartInfo.md5) == 0)  // Got a match?!
        {
            break;                           
        }
        if (strcmp(allConfigs.cart[slot].md5, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") == 0) // Didn't find it... use a blank slot...
        {
            break;                           
        }
    }

    allConfigs.cart[slot] = myCartInfo;
    
    // Always copy back the Global Info
    allConfigs.global = myGlobalCartInfo;
    
    // -------------------------------------------------------------------------------------
    // Compute the CRC32 of everything and we can check this as integrity in the future...
    // -------------------------------------------------------------------------------------
    uInt8 *ptr=(uInt8*)&allConfigs;
    allConfigs.crc32 = 0x00000000;
    for (uInt32 i=0; i < sizeof(allConfigs) - 4; i++)
    {
        allConfigs.crc32 += *ptr++;
    }    

    DIR* dir = opendir("/data");
    if (dir)
    {
        closedir(dir);  // Directory exists.
    }
    else
    {
        mkdir("/data", 0777);   // Doesn't exist - make it...
    }
    fp = fopen("/data/StellaDS.DAT", "wb+");
    if (fp != NULL)
    {
        fwrite(&allConfigs, sizeof(allConfigs), 1, fp);
        fclose(fp);
    } else dsPrintValue(2,20,0, (char*)"  ERROR SAVING CONFIG FILE  ");

    if (bShow) 
    {
        WAITVBL;WAITVBL;WAITVBL;WAITVBL;WAITVBL;
        dsPrintValue(0,23, 0, (char *)CONFIG_INSTRUCTION_STR);
    }
}


// -------------------------------------------------------------------------------------------------
// After settings hae changed, we call this to apply the new options to the game being played.
// This is also called when loading a game and after the configuration if read from StelaDS.DAT
// -------------------------------------------------------------------------------------------------
static void ApplyOptions(bool bFull)
{
    extern u8 bScreenRefresh;
    bScreenRefresh = 1;
    if (bFull)
    {
        dsInitPalette();
    }
}


static void WipeGameConfigsBackToDefault(void)
{
    memset(&allConfigs, 0x00, sizeof(allConfigs));
    // Init the entire database
    for (int slot=0; slot<MAX_CONFIGS; slot++)
    {
        strcpy(allConfigs.cart[slot].md5, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    }
    
    // Set the global stuff...
    allConfigs.global.palette = 0;
    allConfigs.global.sound   = (isDSiMode() ? SOUND_20KHZ : SOUND_10KHZ);
    allConfigs.global.global1 = 0;
    allConfigs.global.global2 = 0;
    allConfigs.global.global3 = 0;
    allConfigs.global.global4 = 0;
    allConfigs.global.global5 = 0;
    allConfigs.global.global6 = 1;
    allConfigs.global.global7 = 1;
    allConfigs.global.global8 = 2;
    
    allConfigs.config_ver = CONFIG_VER;
}

// -------------------------------------------------------------------------
// Find the StellaDS.DAT file and load it... if it doesn't exist, then
// default values will be used for the entire configuration database...
// -------------------------------------------------------------------------
void LoadConfig(void)
{
    bool bInitDatabase = true;
    FILE *fp;

    fp = fopen("/data/StellaDS.DAT", "rb");
    if (fp != NULL)
    {
        fread(&allConfigs, sizeof(allConfigs), 1, fp);
        fclose(fp);
        
        if (allConfigs.config_ver == CONFIG_VER)
        {
            bInitDatabase = false;
        }
    }
    
    if (bInitDatabase)
    {
        WipeGameConfigsBackToDefault();
        allConfigs.config_ver = CONFIG_VER;
        myGlobalCartInfo = allConfigs.global;
        SaveConfig(FALSE);        
    }
    else
    {
        // Always grab the global config on a successful load
        myGlobalCartInfo = allConfigs.global;
    }
    
    ApplyOptions(false);
}


// ------------------------------------------------------------------------------
// Options are handled here... we have a number of things the user can tweak
// and these options are applied immediately. The user can also save off 
// their option choices for the currently running game into the StellaDS.DAT
// configuration database. When games are loaded back up, StellaDS.DAT is read
// to see if we have a match and the user settings can be restored for the game.
// ------------------------------------------------------------------------------
struct options_t
{
    const char  *label;
    uInt8 isNumeric;            // 0=String List, 1=Positive 8-bit, 2=Negative 8-bit
    const char  *option[40];
    uInt8 *option_val;
    uInt8 option_max;
};

const struct options_t Game_Option_Table[2][20] =
{
    {
        {"CONTROLLER",  0, {"LEFTJOY+SAVEKEY", "RIGHT JOYSTICK", "LEFT PADDLE 0", "LEFT PADDLE 1", "RIGHT PADDLE 2", "RIGHT PADDLE 3", "DRIVING", "LEFT KEYBOARD", "BOTH KEYBOARDS", 
                            "BOOSTER", "GENESIS+SAVEKEY", "QUAD+SAVE", "LOST ARK", "STAR RAIDERS", "STARGATE", "SOLARIS", "MC ARCADE", "BUMP BASH", "TWIN STICK"},                        &myCartInfo.controllerType,      19},
        {"BANKSWITCH",  0, {"2K","4K","F4","F4SC","F6","F6SC","F8","F8SC","AR","DPC","DPC+","3E","3F","E0","E7","FASC","FE","CDFJ/CDFJ+","F0/MB","CV","UA","WD","EF","EFSC","BF",
                            "BFSC","DF","DFSC","SB","FA2","TVBOY", "UASW", "0840", "X07", "CTY", "3E+", "WF8", "JANE", "03E0", "0FA0"},                                                   &myCartInfo.banking,             40},
        {"FRAME BLEND", 0, {"NORMAL", "FLICKER FREE", "FF BACKGROUND", "FF BLACK ONLY"},                                                                                                  &myCartInfo.frame_mode,          4},
        {"TV TYPE",     0, {"NTSC", "PAL"},                                                                                                                                               &myCartInfo.tv_type,             2},
        {"PALETTE",     0, {"DS OPTIMIZED", "STELLA", "Z26"},                                                                                                                             &myCartInfo.palette_type,        3},
        {"SOUND",       0, {"OFF (MUTE)", "10 kHZ", "15 kHZ", "20 kHZ", "30 kHZ", "WAVE DIRECT"},                                                                                         &myCartInfo.soundQuality,        6},
        {"A BUTTON",    0, {"FIRE", "JOY UP", "JOY DOWN", "JOY LEFT", "JOY RIGHT", "AUTOFIRE", "SCREEN PAN UP", "SCREEN PAN DOWN"},                                                       &myCartInfo.aButton,             8},
        {"B BUTTON",    0, {"FIRE", "JOY UP", "JOY DOWN", "JOY LEFT", "JOY RIGHT", "AUTOFIRE", "SCREEN PAN UP", "SCREEN PAN DOWN"},                                                       &myCartInfo.bButton,             8},
        {"X BUTTON",    0, {"FIRE", "JOY UP", "JOY DOWN", "JOY LEFT", "JOY RIGHT", "AUTOFIRE", "SCREEN PAN UP", "SCREEN PAN DOWN"},                                                       &myCartInfo.xButton,             8},
        {"Y BUTTON",    0, {"FIRE", "JOY UP", "JOY DOWN", "JOY LEFT", "JOY RIGHT", "AUTOFIRE", "SCREEN PAN UP", "SCREEN PAN DOWN"},                                                       &myCartInfo.yButton,             8},
        {"HBLANK ZERO", 0, {"NO (FASTER)", "YES (ACCURATE)"},                                                                                                                             &myCartInfo.hBlankZero,          2},
        {"VBLANK ZERO", 0, {"NO (FASTER)", "YES (ACCURATE)"},                                                                                                                             &myCartInfo.vblankZero,          2},
        {"ANALOG SENS", 1, {"5",   "25"},                                                                                                                                                 &myCartInfo.analogSensitivity,   1},
        {"START SCANL", 1, {"25",  "75"},                                                                                                                                                 &myCartInfo.displayStartScanline,1},
        {"NUM   SCANL", 1, {"190", "255"},                                                                                                                                                &myCartInfo.displayNumScalines,  1},
        {"Y SCALE",     1, {"50",  "100"},                                                                                                                                                &myCartInfo.yScale,              1},
        {"X OFFSET",    2, {"-50", "50"},                                                                                                                                         (uInt8*)&myCartInfo.xOffset,             1},
        {"Y OFFSET",    2, {"-50", "50"},                                                                                                                                         (uInt8*)&myCartInfo.yOffset,             1},
        {"X STRETCH",   2, {"0",   "32"},                                                                                                                                         (uInt8*)&myCartInfo.xStretch,            1},

        {NULL,          0, {"",      ""},                                                                                                                                                 NULL,                            1},
    },
    {
        {"ARM THUMB",  0, {"SAFE", "OPTIMIZED", "OPT-NO-COLL", "MAX-FRAMESKIP"},                                                                                                          &myCartInfo.thumbOptimize,       4},
        {"BUS MODE",   0, {"OPTIMIZED", "ACCURATE"},                                                                                                                                      &myCartInfo.bus_driver,          2},
        {"6532 RAM",   0, {"RANDOM", "CLEAR (ZEROS)"},                                                                                                                                    &myCartInfo.clearRAM,            2},
        
        {"GLOB PALET", 0, {"DS OPTIMIZED", "STELLA", "Z26"},                                                                                                                              &myGlobalCartInfo.palette,       3},
        {"GLOB SOUND", 0, {"OFF (MUTE)", "10 kHZ", "15 kHZ", "20 kHZ", "30 kHZ", "WAVE DIRECT"},                                                                                          &myGlobalCartInfo.sound,         6},
        {NULL,         0, {"",      ""},                                                                                                                                                  NULL,                            1},
    }    
};

void display_line(uInt8 idx, uInt8 highlight)
{
    static char strBuf[35];
    if (Game_Option_Table[OptionPage][idx].isNumeric == 1)  // Unsigned 8 bit
    {
        sprintf(strBuf, " %-11s : %-15d", Game_Option_Table[OptionPage][idx].label, *(Game_Option_Table[OptionPage][idx].option_val));
    }
    else if (Game_Option_Table[OptionPage][idx].isNumeric == 2) // Signed 8 bit
    {
        sprintf(strBuf, " %-11s : %-15d", Game_Option_Table[OptionPage][idx].label, *((Int8*)Game_Option_Table[OptionPage][idx].option_val));
    }
    else    // Array of strings
    {
        sprintf(strBuf, " %-11s : %-15s", Game_Option_Table[OptionPage][idx].label, Game_Option_Table[OptionPage][idx].option[*(Game_Option_Table[OptionPage][idx].option_val)]);
    }
    dsPrintValue(1,3+idx, highlight, strBuf);
}

// ------------------------------------------------------------------
// Display the current list of options...
// ------------------------------------------------------------------
static int display_options_list(bool bFullDisplay)
{
    short int len=0;
    
    if (bFullDisplay)
    {
        while (true)
        {
            display_line(len, (len==0 ? 1:0));
            len++;
            if (Game_Option_Table[OptionPage][len].label == NULL) break;
        }

        // Blank out rest of the screen... option menus are of different lengths...
        for (int i=len; i<22; i++) 
        {
            dsPrintValue(1,3+i, 0, (char *)"                               ");
        }
    }

    dsPrintValue(0,23, 0, (char *)CONFIG_INSTRUCTION_STR);
    return len;    
}


// -----------------------------------------------------------------------------
// Allows the user to move the cursor up and down through the various table 
// enties  above to select options for the game they wish to play. 
// -----------------------------------------------------------------------------
void ShowConfig(void)
{
    short int optionHighlighted;
    short int idx;
    u8 bDone=false;
    short int keys_pressed;
    short int last_keys_pressed = 999;

    // Show the Options background...
    decompress(bgInstructionsTiles, bgGetGfxPtr(bg0b), LZ77Vram);
    decompress(bgInstructionsMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
    dmaCopy((void *) bgInstructionsPal,(u16*) BG_PALETTE_SUB,256*2);
    unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
    dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
    swiWaitForVBlank();    

    idx=display_options_list(true);
    optionHighlighted = 0;
    while (!bDone)
    {
        keys_pressed = keysCurrent();
        if (keys_pressed != last_keys_pressed)
        {
            last_keys_pressed = keys_pressed;
            if (keysCurrent() & KEY_UP) // Previous option
            {
                display_line(optionHighlighted, 0);
                if (optionHighlighted > 0) optionHighlighted--; else optionHighlighted=(idx-1);
                display_line(optionHighlighted, 1);
            }
            if (keysCurrent() & KEY_DOWN) // Next option
            {
                display_line(optionHighlighted, 0);
                if (optionHighlighted < (idx-1)) optionHighlighted++;  else optionHighlighted=0;
                display_line(optionHighlighted, 1);
            }
            if (keysCurrent() & (KEY_L|KEY_R))
            {
                OptionPage = 1-OptionPage;
                optionHighlighted=0;
                idx = display_options_list(true);
            }
            if (keysCurrent() & KEY_RIGHT)  // Toggle option clockwise
            {
                if (Game_Option_Table[OptionPage][optionHighlighted].isNumeric == 1)
                {
                    if (*(Game_Option_Table[OptionPage][optionHighlighted].option_val) < atoi(Game_Option_Table[OptionPage][optionHighlighted].option[1]))
                    {
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) += 1;
                    }
                }
                else if (Game_Option_Table[OptionPage][optionHighlighted].isNumeric == 2)
                {
                    if (*((Int8*)Game_Option_Table[OptionPage][optionHighlighted].option_val) < atoi(Game_Option_Table[OptionPage][optionHighlighted].option[1]))
                    {
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) += 1;
                    }
                }
                else
                {
                    *(Game_Option_Table[OptionPage][optionHighlighted].option_val) = (*(Game_Option_Table[OptionPage][optionHighlighted].option_val) + 1) % Game_Option_Table[OptionPage][optionHighlighted].option_max;
                }
                display_line(optionHighlighted, 1);
                ApplyOptions(true);
            }
            if (keysCurrent() & KEY_LEFT)  // Toggle option counterclockwise
            {
                if (Game_Option_Table[OptionPage][optionHighlighted].isNumeric == 1)
                {
                    if (*(Game_Option_Table[OptionPage][optionHighlighted].option_val) > atoi(Game_Option_Table[OptionPage][optionHighlighted].option[0]))
                    {
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) -= 1;
                    }
                }
                else if (Game_Option_Table[OptionPage][optionHighlighted].isNumeric == 2)
                {
                    if (*((Int8*)Game_Option_Table[OptionPage][optionHighlighted].option_val) > atoi(Game_Option_Table[OptionPage][optionHighlighted].option[0]))
                    {
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) -= 1;
                    }
                }
                else
                {
                    if ((*(Game_Option_Table[OptionPage][optionHighlighted].option_val)) == 0)
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) = Game_Option_Table[OptionPage][optionHighlighted].option_max -1;
                    else
                        *(Game_Option_Table[OptionPage][optionHighlighted].option_val) = (*(Game_Option_Table[OptionPage][optionHighlighted].option_val) - 1) % Game_Option_Table[OptionPage][optionHighlighted].option_max;
                }
                display_line(optionHighlighted, 1);
                ApplyOptions(true);
            }
            if (keysCurrent() & KEY_START)  // Save Options
            {
                SaveConfig(TRUE);
            }
            if (keysCurrent() & KEY_SELECT)  // Restore Defaults
            {
                extern void CartSetDefaultFromInternalDatabase(void);
                CartSetDefaultFromInternalDatabase();
                display_options_list(true);
                optionHighlighted = 0;
            }
            if ((keysCurrent() & KEY_B) || (keysCurrent() & KEY_A))  // Exit options
            {
                break;
            }
        }
        swiWaitForVBlank();
    }

    // Restore original bottom graphic
    dsShowScreenMain(false);
    
    // Give a third of a second time delay...
    for (int i=0; i<20; i++)
    {
        swiWaitForVBlank();
    }

    return;
}


// End of Line

