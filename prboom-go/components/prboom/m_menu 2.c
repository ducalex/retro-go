/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  DOOM selection menu, options, episode etc. (aka Big Font menus)
 *  Sliders and icons. Kinda widget stuff.
 *  Setup Menus.
 *  Extended HELP screens.
 *  Dynamic HELP screen.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "d_deh.h"
#include "m_misc.h"
#include "lprintf.h"
#include "am_map.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "r_demo.h"
#include "r_fps.h"

extern patchnum_t hu_font[HU_FONTSIZE];

int showMessages;           // Show messages has default, 0 = off, 1 = on
bool inhelpscreens;      // indicates we are in or just left a help screen
int menuactive;             // The menus are up
int screenSize;             // temp for screenblocks (0-9))
int screenblocks;           // has default
int mouseSensitivity_horiz; // has default
int mouseSensitivity_vert;  // has default

static bool setup_active      = false; // in one of the setup screens
static bool setup_select      = false; // changing an item
static bool default_verify    = false; // verify reset defaults decision
static int epi;
static int set_menu_itemon; // which setup item is selected?   // phares 3/98
static int messageToPrint;         // 1 = message to be printed////
static const char* messageString; // ...and here is the message string!
static int messageLastMenuActive;
static int messageNeedsInput; // timed message = no input from user
static void (*messageRoutine)(int response);
static int itemOn;           // menu item skull is on (for Big Font menus)
static int skullAnimCounter; // skull animation counter
static int whichSkull;       // which skull to draw (he blinks
static int warning_about_changes, print_warning_about_changes;
static int quickSaveSlot;          // -1 = no quicksave slot picked!
static int saveSlot;        // which slot to save in
#define SAVESTRINGSIZE  24
static char savegamestrings[10][SAVESTRINGSIZE];
static char menu_buffer[160];

#define warn_about_changes(x) (warning_about_changes=(x), \
             print_warning_about_changes = 2)

//
// MENU TYPEDEFS
//
typedef struct
{
  short status; // 0 = no cursor here, 1 = ok, 2 = arrows ok
  char  name[16];
  void  (*routine)(int choice);
  char  alphaKey; // hotkey in menu (unused)
} menuitem_t;

typedef struct menu_s
{
  short           numitems;     // # of menu items
  struct menu_s*  prevMenu;     // previous menu
  menuitem_t*     menuitems;    // menu items
  void            (*routine)(); // draw routine
  short           x;
  short           y;            // x,y of menu
  short           lastOn;       // last item user was on in menu
} menu_t;

static int current_setup_page; // points to current setup menu table
static menu_t* currentMenu; // current menudef

#define SKULLXOFF  -32
#define LINEHEIGHT  16
#define SPACEWIDTH 4
#define LOADGRAPHIC_Y 8
#define CR_TITLE  CR_GOLD
#define CR_SET    CR_GREEN
#define CR_ITEM   CR_RED
#define CR_HILITE CR_ORANGE
#define CR_SELECT CR_GRAY
#define VERIFYBOXXORG 66
#define VERIFYBOXYORG 88

#define G_X    270
#define G_Y     31
#define G_H      9
#define G_PREV  57
#define G_NEXT 310
#define G_PREVNEXT 185
#define G_RESET_X 301
#define G_RESET_Y   3

#define KT_X1 283
#define KT_X2 172
#define KT_X3  87
#define KT_Y1   2
#define KT_Y2 118
#define KT_Y3 102

#define CR_S 9
#define CR_X 20
#define CR_X2 50
#define CR_Y 32
#define CR_SH 9

static const struct {
  const char *code;
  const char *desc;
} cheats_list[] = {
  {"idkfa",      "All Weapons + Keys"},
  {"iddqd",      "Invulnerability"},
  {"idbeholdh",  "200% Health"},
  {"idchoppers", "Get Chainsaw"},
  {"idbeholdl",  "Toggle Light Ampli."},
  {"idbeholds",  "Toggle Berserk"},
  {"idbeholdi",  "Toggle Invisibility"},
  {"idclip",     "Toggle clipping"},
  {"idrate",     "Toggle FPS"},
  //{"          ", "Skip Level"},
};

static const char *renderfilters[] = {"none", "point", "linear", "rounded"};
static const char *edgetypes[] = {"jagged", "sloped"};

static void M_NewGame(int choice);
static void M_Options(int choice);
static void M_LoadGame(int choice);
static void M_SaveGame(int choice);
static void M_ReadThis(int choice);
static void M_QuitDOOM(int choice);
static void M_ReadThis2(int choice);
static void M_FinishReadThis(int choice);
static void M_FinishHelp(int choice);
static void M_Episode(int choice);
static void M_ChooseSkill(int choice);
static void M_LoadSelect(int choice);
static void M_SaveSelect(int choice);
static void M_Setup(int choice);
static void M_SizeDisplay(int choice);
static void M_Sound(int choice);
static void M_Cheats(int choice);
static void M_ToggleMap(int choice);
static void M_LevelSelect(int choice);
static void M_LevelSelectSelect(int choice);
static void M_CheatSelect(int choice);
static void M_EndGame(int choice);
static void M_DoNothing(int choice);
static void M_MusicVol(int choice);
static void M_SfxVol(int choice);
static void M_DrawReadThis1(void);
static void M_DrawReadThis2(void);
static void M_DrawHelp(void);
static void M_DrawEpisode(void);
static void M_DrawNewGame(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);
static void M_DrawOptions(void);
static void M_DrawCheats(void);
static void M_DrawSound(void);
static void M_DrawSetup(void);
static void M_DrawMainMenu(void);
static void M_DrawLevelSelect(void);
static void M_ChangeDemoSmoothTurns(void);
static void M_ClearMenus(void);
static void M_Trans(void);


static menuitem_t MainMenu[] =
{
  {1,"M_NGAME", M_NewGame, 'n'},
  {1,"M_OPTION",M_Options, 'o'},
  {1,"M_LOADG", M_LoadGame,'l'},
  {1,"M_SAVEG", M_SaveGame,'s'},
  {1,"M_RDTHIS",M_ReadThis,'r'},
  {1,"M_QUITG", M_QuitDOOM,'q'},
};

static menu_t MainDef =
{
  sizeof(MainMenu) / sizeof(menuitem_t),              // number of menu items
  NULL,           // previous menu screen
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  97, 72,         // initial cursor position
  0               // last menu item the user was on
};

static menuitem_t ReadMenu1[] =
{
  {1,"",M_ReadThis2,0},
};

static menuitem_t ReadMenu2[]=
{
  {1,"",M_FinishReadThis,0},
};

static menu_t ReadDef1 =
{
  1,
  &MainDef,
  ReadMenu1,
  M_DrawReadThis1,
  330,175,
  0
};

static menu_t ReadDef2 =
{
  1,
  &ReadDef1,
  ReadMenu2,
  M_DrawReadThis2,
  330,175,
  0
};

static menuitem_t HelpMenu[]=    // killough 10/98
{
  {1,"",M_FinishHelp,0},
};

static menu_t HelpDef =           // killough 10/98
{
  1,
  &HelpDef,
  HelpMenu,
  M_DrawHelp,
  330,175,
  0
};

static menuitem_t EpisodeMenu[]=
{
  {1,"M_EPI1", M_Episode,'k'},
  {1,"M_EPI2", M_Episode,'t'},
  {1,"M_EPI3", M_Episode,'i'},
  {1,"M_EPI4", M_Episode,'t'},
  {1,"M_EPI5", M_Episode,'t'},
  {1,"M_EPI6", M_Episode,'t'}
};

static menu_t EpiDef =
{
  sizeof(EpisodeMenu) / sizeof(menuitem_t),             // # of menu items
  &MainDef,      // previous menu
  EpisodeMenu,   // menuitem_t ->
  M_DrawEpisode, // drawing routine ->
  48,63,         // x,y
  0              // lastOn
};

static menuitem_t NewGameMenu[]=
{
  {1,"M_JKILL", M_ChooseSkill, 'i'},
  {1,"M_ROUGH", M_ChooseSkill, 'h'},
  {1,"M_HURT",  M_ChooseSkill, 'h'},
  {1,"M_ULTRA", M_ChooseSkill, 'u'},
  {1,"M_NMARE", M_ChooseSkill, 'n'},
};

static menu_t NewDef =
{
  sizeof(NewGameMenu) / sizeof(menuitem_t),       // # of menu items
  &EpiDef,        // previous menu
  NewGameMenu,    // menuitem_t ->
  M_DrawNewGame,  // drawing routine ->
  48,63,          // x,y
  2               // lastOn
};

static menuitem_t LoadMenue[]=
{
  {1,"", M_LoadSelect,'1'},
  {1,"", M_LoadSelect,'2'},
  {1,"", M_LoadSelect,'3'},
  {1,"", M_LoadSelect,'4'},
  {1,"", M_LoadSelect,'5'},
  {1,"", M_LoadSelect,'6'},
  {1,"", M_LoadSelect,'7'},
  {1,"", M_LoadSelect,'8'},
};

static menu_t LoadDef =
{
  sizeof(LoadMenue) / sizeof(menuitem_t),
  &MainDef,
  LoadMenue,
  M_DrawLoad,
  80,34,
  0
};

static menuitem_t SaveMenu[]=
{
  {1,"", M_SaveSelect,'1'},
  {1,"", M_SaveSelect,'2'},
  {1,"", M_SaveSelect,'3'},
  {1,"", M_SaveSelect,'4'},
  {1,"", M_SaveSelect,'5'},
  {1,"", M_SaveSelect,'6'},
  {1,"", M_SaveSelect,'7'},
  {1,"", M_SaveSelect,'8'},
};

static menu_t SaveDef =
{
  sizeof(SaveMenu) / sizeof(menuitem_t),
  &MainDef,
  SaveMenu,
  M_DrawSave,
  80,34,
  0
};

static menuitem_t OptionsMenu[]=
{
  {1,"M_SETUP",  M_Setup,   's'},
  {2,"M_SCRNSZ", M_SizeDisplay,'s'},
  {-1,"",0}, // Display size placeholder
  {1,"M_SVOL",   M_Sound,'s'},
  {1,":Cheats", M_Cheats, 'e'},
  {1,":Level Select", M_LevelSelect,'s'},
  {1,":Toggle Map", M_ToggleMap, 's'},
  {1,"M_ENDGAM", M_EndGame,'e'},
};

static menu_t OptionsDef =
{
  sizeof(OptionsMenu) / sizeof(menuitem_t),
  &MainDef,
  OptionsMenu,
  M_DrawOptions,
  60,37,
  0
};

static menuitem_t LevelSelectMenu[]=
{
  {1, ":Level 1", M_LevelSelectSelect, '1'},
  {1, ":Level 2", M_LevelSelectSelect, '2'},
  {1, ":Level 3", M_LevelSelectSelect, '3'},
  {1, ":Level 4", M_LevelSelectSelect, '4'},
  {1, ":Level 5", M_LevelSelectSelect, '5'},
  {1, ":Level 6", M_LevelSelectSelect, '6'},
  {1, ":Level 7", M_LevelSelectSelect, '7'},
  {1, ":Level 8", M_LevelSelectSelect, '8'},
  {1, ":Level 9", M_LevelSelectSelect, '9'}
};

static menu_t LevelSelectDef =
{
  sizeof(LevelSelectMenu) / sizeof(menuitem_t),
  &OptionsDef,
  LevelSelectMenu,
  M_DrawLevelSelect,
  80, 24,
  0
};

static menuitem_t CheatsMenu[]=
{
  {1, "", M_CheatSelect, '1'},
  {1, "", M_CheatSelect, '2'},
  {1, "", M_CheatSelect, '3'},
  {1, "", M_CheatSelect, '4'},
  {1, "", M_CheatSelect, '5'},
  {1, "", M_CheatSelect, '6'},
  {1, "", M_CheatSelect, '7'},
  {1, "", M_CheatSelect, '8'},
  {1, "", M_CheatSelect, '9'},
};

static menu_t CheatsDef =
{
  sizeof(CheatsMenu) / sizeof(menuitem_t),
  &OptionsDef,
  CheatsMenu,
  M_DrawCheats,
  80, 24,
  0
};

static menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",M_SfxVol,'s'},
  {-1,"",0},
  {2,"M_MUSVOL",M_MusicVol,'m'},
  {-1,"",0}
};

static menu_t SoundDef =
{
  sizeof(SoundMenu) / sizeof(menuitem_t),
  &OptionsDef,
  SoundMenu,
  M_DrawSound,
  80,48,
  0
};

static menuitem_t SetupMenu[] =
{
  {1,"",M_DoNothing,0}
};

static menu_t SetupDef =
{
  1,
  &OptionsDef,
  SetupMenu,
  M_DrawSetup,
  36,8,
  0
};

static setup_menu_t gen_settings3[] =  // General Settings
{
  {"GENERAL",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"Enable Translucency", S_YESNO, m_null, G_X, G_Y + 1*G_H, {"translucency"}, M_Trans},
  {"Translucency filter percentage", S_NUM, m_null, G_X, G_Y + 2*G_H, {"tran_filter_pct"}, M_Trans},
  {"Filter for walls", S_CHOICE, m_null, G_X, G_Y + 3*G_H, {"filter_wall"}, NULL, renderfilters},
  {"Filter for floors/ceilings", S_CHOICE, m_null, G_X, G_Y + 4*G_H, {"filter_floor"}, NULL, renderfilters},
  {"Filter for sprites", S_CHOICE, m_null, G_X, G_Y + 5*G_H, {"filter_sprite"}, NULL, renderfilters},
  {"Filter for patches", S_CHOICE, m_null, G_X, G_Y + 6*G_H, {"filter_patch"}, NULL, renderfilters},
  {"Filter for lighting", S_CHOICE, m_null, G_X, G_Y + 7*G_H, {"filter_z"}, NULL, renderfilters},
  {"Drawing of sprite edges", S_CHOICE, m_null, G_X, G_Y + 8*G_H, {"sprite_edges"}, NULL, edgetypes},
  {"Drawing of patch edges", S_CHOICE, m_null, G_X, G_Y + 9*G_H, {"patch_edges"}, NULL, edgetypes},
  {"Flashing HOM indicator", S_YESNO, m_null, G_X, G_Y + 10*G_H, {"flashing_hom"}},
  {"Maximum number of player corpses", S_NUM|S_PRGWARN, m_null, G_X, G_Y + 11*G_H, {"max_player_corpse"}},
  {"Game speed, percentage of normal", S_NUM|S_PRGWARN, m_null, G_X, G_Y + 12*G_H, {"realtic_clock_rate"}},
  {"Smooth Demo", S_YESNO, m_null, G_X, G_Y + 13*G_H, {"demo_smoothturns"}, M_ChangeDemoSmoothTurns},
  {"Smooth Demo Factor", S_NUM, m_null, G_X, G_Y + 14*G_H, {"demo_smoothturnsfactor"}, M_ChangeDemoSmoothTurns},
  {"Show map secrets only after entering",S_YESNO,m_null,G_X,G_Y+15*G_H, {"map_secret_after"}},
  {"Show map pointer coordinates",S_YESNO,m_null,G_X,G_Y+16*G_H, {"map_point_coord"}},  // killough 10/98

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t weap_settings1[] =  // Weapons Settings screen
{
  {"WEAPONS",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"ENABLE RECOIL", S_YESNO,m_null,G_X, G_Y+1*G_H, {"weapon_recoil"}},
  {"ENABLE BOBBING",S_YESNO,m_null,G_X, G_Y+2*G_H, {"player_bobbing"}},

  {"1ST CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+4*G_H, {"weapon_choice_1"}},
  {"2nd CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+5*G_H, {"weapon_choice_2"}},
  {"3rd CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+6*G_H, {"weapon_choice_3"}},
  {"4th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+7*G_H, {"weapon_choice_4"}},
  {"5th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+8*G_H, {"weapon_choice_5"}},
  {"6th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+9*G_H, {"weapon_choice_6"}},
  {"7th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+10*G_H, {"weapon_choice_7"}},
  {"8th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+11*G_H, {"weapon_choice_8"}},
  {"9th CHOICE WEAPON",S_WEAP,m_null,G_X,G_Y+12*G_H, {"weapon_choice_9"}},

  {"Enable Fist/Chainsaw\n& SG/SSG toggle", S_YESNO, m_null, G_X, G_Y+ 14*G_H, {"doom_weapon_toggles"}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t stat_settings1[] =  // Status Bar and HUD Settings screen
{
  {"STATUS BAR"        ,S_SKIP|S_TITLE,m_null,200,G_Y},
  {"Use Red Numbers"   ,S_YESNO, m_null,G_X,G_Y+ 1*G_H, {"sts_always_red"}},
  {"Gray %"            ,S_YESNO, m_null,G_X,G_Y+ 2*G_H, {"sts_pct_always_gray"}},
  {"Single Key Display",S_YESNO, m_null,G_X,G_Y+ 3*G_H, {"sts_traditional_keys"}},

  {"HEADS-UP DISPLAY"  ,S_SKIP|S_TITLE,m_null,200,G_Y+ 5*G_H},
  {"Hide secrets"      ,S_YESNO     ,m_null,G_X,G_Y+ 6*G_H, {"hud_nosecrets"}},
  {"Message Color During Play", S_CRITEM, m_null, G_X, G_Y + 7*G_H, {"hudcolor_mesg"}},
  {"Chat Message Color", S_CRITEM, m_null, G_X, G_Y + 8*G_H, {"hudcolor_chat"}},
  {"Message Review Color", S_CRITEM, m_null, G_X, G_Y + 9*G_H, {"hudcolor_list"}},
  {"Number of Review Message Lines", S_NUM, m_null,  G_X, G_Y + 10*G_H, {"hud_msg_lines"}},
  {"Message Background",  S_YESNO,  m_null,  G_X, G_Y + 11*G_H, {"hud_list_bgon"}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t enem_settings1[] =  // Enemy Settings screen
{
  {"ENEMIES" ,S_SKIP|S_TITLE,m_null,200,G_Y},
  {"Monster Infighting When Provoked",S_YESNO,m_null,G_X,G_Y+ 1*G_H, {"monster_infighting"}},
  {"Remember Previous Enemy",S_YESNO,m_null,G_X,G_Y+ 2*G_H, {"monsters_remember"}},
  {"Monster Backing Out",S_YESNO,m_null,G_X,G_Y+ 3*G_H, {"monster_backing"}},
  {"Climb Steep Stairs", S_YESNO,m_null,G_X,G_Y+4*G_H, {"monkeys"}},
  {"Intelligently Avoid Hazards",S_YESNO,m_null,G_X,G_Y+ 5*G_H, {"monster_avoid_hazards"}},
  {"Affected by Friction",S_YESNO,m_null,G_X,G_Y+ 6*G_H, {"monster_friction"}},
  {"Rescue Dying Friends",S_YESNO,m_null,G_X,G_Y+ 7*G_H, {"help_friends"}},
#ifdef DOGS
  {"Number Of Single-Player Helper Dogs",S_NUM|S_LEVWARN,m_null,G_X,G_Y+ 8*G_H, {"player_helpers"}},
  {"Distance Friends Stay Away",S_NUM,m_null,G_X,G_Y+ 9*G_H, {"friend_distance"}},
  {"Allow dogs to jump down",S_YESNO,m_null,G_X,G_Y+ 10*G_H, {"dog_jumping"}},
#endif

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t comp_settings1[] =  // Compatibility Settings screen #1
{
  {"COMPAT 1/1",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"Any monster can telefrag on MAP30", S_YESNO, m_null, G_X, G_Y + 1*G_H, {"comp_telefrag"}},
  {"Some objects never hang over tall ledges", S_YESNO, m_null, G_X, G_Y + 2*G_H, {"comp_dropoff"}},
  {"Objects don't fall under their own weight", S_YESNO, m_null, G_X, G_Y + 3*G_H, {"comp_falloff"}},
  {"Monsters randomly walk off of moving lifts", S_YESNO, m_null, G_X, G_Y + 4*G_H, {"comp_staylift"}},
  {"Monsters get stuck on doortracks", S_YESNO, m_null, G_X, G_Y + 5*G_H, {"comp_doorstuck"}},
  {"Monsters don't give up pursuit of targets", S_YESNO, m_null, G_X, G_Y + 6*G_H, {"comp_pursuit"}},
  {"Arch-Vile resurrects invincible ghosts", S_YESNO, m_null, G_X, G_Y + 7*G_H, {"comp_vile"}},
  {"Pain Elementals limited to 21 lost souls", S_YESNO, m_null, G_X, G_Y + 8*G_H, {"comp_pain"}},
  {"Lost souls get stuck behind walls", S_YESNO, m_null, G_X, G_Y + 9*G_H, {"comp_skull"}},
  {"Blazing doors make double closing sounds", S_YESNO, m_null, G_X, G_Y + 10*G_H, {"comp_blazing"}},
  {"Tagged doors don't trigger special lighting", S_YESNO, m_null, G_X, G_Y + 11*G_H, {"comp_doorlight"}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t comp_settings2[] =  // Compatibility Settings screen #2
{
  {"COMPAT 2/2",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"Lost souls don't bounce off flat surfaces", S_YESNO, m_null, G_X, G_Y + 1*G_H, {"comp_soul"}},
  {"God mode isn't absolute", S_YESNO, m_null, G_X, G_Y + 2*G_H, {"comp_god"}},
  {"Zombie players can exit levels", S_YESNO, m_null, G_X, G_Y + 4*G_H, {"comp_zombie"}},
  {"Sky is unaffected by invulnerability", S_YESNO, m_null, G_X, G_Y + 5*G_H, {"comp_skymap"}},
  {"Use Doom's stairbuilding method", S_YESNO, m_null, G_X, G_Y + 6*G_H, {"comp_stairs"}},
  {"Use Doom's floor motion behavior", S_YESNO, m_null, G_X, G_Y + 7*G_H, {"comp_floors"}},
  {"Use Doom's movement clipping code", S_YESNO, m_null, G_X, G_Y + 8*G_H, {"comp_moveblock"}},
  {"Use Doom's linedef trigger model", S_YESNO, m_null, G_X, G_Y + 9*G_H, {"comp_model"}},
  {"Linedef effects work with sector tag = 0", S_YESNO, m_null, G_X, G_Y + 10*G_H, {"comp_zerotags"}},
  {"All boss types can trigger tag 666 at ExM8", S_YESNO, m_null, G_X, G_Y + 11*G_H, {"comp_666"}},
  {"2S middle textures do not animate", S_YESNO, m_null, G_X, G_Y + 12*G_H, {"comp_maskedanim"}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t keys_settings1[] =  // Key Binding screen strings
{
  {"KEYBOARD 1/2",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,G_X,G_Y+1*G_H},
  {"FORWARD"     ,S_KEY       ,m_scrn,G_X,G_Y+2*G_H,{&key_up}},
  {"BACKWARD"    ,S_KEY       ,m_scrn,G_X,G_Y+3*G_H,{&key_down}},
  {"TURN LEFT"   ,S_KEY       ,m_scrn,G_X,G_Y+4*G_H,{&key_left}},
  {"TURN RIGHT"  ,S_KEY       ,m_scrn,G_X,G_Y+5*G_H,{&key_right}},
  {"RUN"         ,S_KEY       ,m_scrn,G_X,G_Y+6*G_H,{&key_speed}},
  {"STRAFE LEFT" ,S_KEY       ,m_scrn,G_X,G_Y+7*G_H,{&key_strafeleft}},
  {"STRAFE RIGHT",S_KEY       ,m_scrn,G_X,G_Y+8*G_H,{&key_straferight}},
  {"STRAFE"      ,S_KEY       ,m_scrn,G_X,G_Y+9*G_H,{&key_strafe}},
  {"AUTORUN"     ,S_KEY       ,m_scrn,G_X,G_Y+10*G_H,{&key_autorun}},
  {"180 TURN"    ,S_KEY       ,m_scrn,G_X,G_Y+11*G_H,{&key_reverse}},
  {"USE"         ,S_KEY       ,m_scrn,G_X,G_Y+12*G_H,{&key_use}},

  {"WEAPONS" ,S_SKIP|S_TITLE,m_null,G_X,G_Y+13*G_H},
  {"TOGGLE"  ,S_KEY       ,m_scrn,G_X,G_Y+14*G_H,{&key_weapontoggle}},
  {"FIRE"    ,S_KEY       ,m_scrn,G_X,G_Y+15*G_H,{&key_fire}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t keys_settings2[] =  // Key Binding screen strings
{
  {"KEYBOARD 2/2",S_SKIP|S_TITLE,m_null,200,G_Y},
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,G_X,G_Y+1*G_H},
  {"HELP"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_escape}},
  {"PAUSE"       ,S_KEY       ,m_scrn,G_X,G_Y+ 2*G_H,{&key_pause}},
  {"AUTOMAP"     ,S_KEY       ,m_scrn,G_X,G_Y+ 3*G_H,{&key_map}},
  {"HUD"         ,S_KEY       ,m_scrn,G_X,G_Y+ 4*G_H,{&key_hud}},
  {"MESSAGES"    ,S_KEY       ,m_scrn,G_X,G_Y+ 5*G_H,{&key_messages}},
  {"GAMMA FIX"   ,S_KEY       ,m_scrn,G_X,G_Y+ 6*G_H,{&key_gamma}},
  {"SPY"         ,S_KEY       ,m_scrn,G_X,G_Y+ 7*G_H,{&key_spy}},

  {"GAME"        ,S_SKIP|S_TITLE,m_null,G_X,G_Y+9*G_H},
  {"SAVE"        ,S_KEY       ,m_scrn,G_X,G_Y+10*G_H,{&key_savegame}},
  {"LOAD"        ,S_KEY       ,m_scrn,G_X,G_Y+11*G_H,{&key_loadgame}},
  {"QUICKSAVE"   ,S_KEY       ,m_scrn,G_X,G_Y+12*G_H,{&key_quicksave}},
  {"QUICKLOAD"   ,S_KEY       ,m_scrn,G_X,G_Y+13*G_H,{&key_quickload}},
  {"END GAME"    ,S_KEY       ,m_scrn,G_X,G_Y+14*G_H,{&key_endgame}},
  {"QUIT"        ,S_KEY       ,m_scrn,G_X,G_Y+15*G_H,{&key_quit}},

  {0,S_SKIP|S_END,m_null}
};

static setup_menu_t helpstrings[] =  // HELP screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y1},
  {"HELP"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 1*8,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 2*8,{&key_escape}},
  {"PAUSE"       ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 4*8,{&key_pause}},
  {"AUTOMAP"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 5*8,{&key_map}},
  {"SOUND VOLUME",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 6*8,{&key_soundvolume}},
  {"HUD"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 7*8,{&key_hud}},
  {"MESSAGES"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 8*8,{&key_messages}},
  {"GAMMA FIX"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 9*8,{&key_gamma}},
  {"SPY"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+10*8,{&key_spy}},
  {"LARGER VIEW" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+11*8,{&key_zoomin}},
  {"SMALLER VIEW",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+12*8,{&key_zoomout}},

  {"AUTOMAP"     ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y2},
  {"FOLLOW MODE" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 1*8,{&key_map_follow}},
  {"ZOOM IN"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 2*8,{&key_map_zoomin}},
  {"ZOOM OUT"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 3*8,{&key_map_zoomout}},
  {"MARK PLACE"  ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 4*8,{&key_map_mark}},
  {"CLEAR MARKS" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 5*8,{&key_map_clear}},
  {"FULL/ZOOM"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 6*8,{&key_map_gobig}},
  {"GRID"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 7*8,{&key_map_grid}},

  {"WEAPONS"     ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y1},
  {"FIST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 1*8,{&key_weapon1}},
  {"PISTOL"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 2*8,{&key_weapon2}},
  {"SHOTGUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 3*8,{&key_weapon3}},
  {"CHAINGUN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 4*8,{&key_weapon4}},
  {"ROCKET"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 5*8,{&key_weapon5}},
  {"PLASMA"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 6*8,{&key_weapon6}},
  {"BFG 9000"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 7*8,{&key_weapon7}},
  {"CHAINSAW"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 8*8,{&key_weapon8}},
  {"SSG"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 9*8,{&key_weapon9}},
  {"BEST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+10*8,{&key_weapontoggle}},
  {"FIRE"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+11*8,{&key_fire}},

  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y3},
  {"FORWARD"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 1*8,{&key_up}},
  {"BACKWARD"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 2*8,{&key_down}},
  {"TURN LEFT"   ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 3*8,{&key_left}},
  {"TURN RIGHT"  ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 4*8,{&key_right}},
  {"RUN"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 5*8,{&key_speed}},
  {"STRAFE LEFT" ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 6*8,{&key_strafeleft}},
  {"STRAFE RIGHT",S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 7*8,{&key_straferight}},
  {"STRAFE"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 8*8,{&key_strafe}},
  {"AUTORUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 9*8,{&key_autorun}},
  {"180 TURN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+10*8,{&key_reverse}},
  {"USE"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+11*8,{&key_use}},

  {"GAME"        ,S_SKIP|S_TITLE,m_null,KT_X2,KT_Y1},
  {"SAVE"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 1*8,{&key_savegame}},
  {"LOAD"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 2*8,{&key_loadgame}},
  {"QUICKSAVE"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 3*8,{&key_quicksave}},
  {"END GAME"    ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 4*8,{&key_endgame}},
  {"QUICKLOAD"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 5*8,{&key_quickload}},
  {"QUIT"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 6*8,{&key_quit}},

  // Final entry
  {0,S_SKIP|S_END,m_null}
};

enum {prog, prog_stub, prog_stub1, prog_stub2, adcr};
enum {cr_prog, cr_stub, cr_adcr};

static setup_menu_t cred_settings[] =
{
  {"Programmers",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*prog + CR_SH*cr_prog},
  {"Florian 'Proff' Schulze",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+1) + CR_SH*cr_prog},
  {"Colin Phipps",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+2) + CR_SH*cr_prog},
  {"Neil Stevens",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+3) + CR_SH*cr_prog},
  {"Andrey Budko",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+4) + CR_SH*cr_prog},

  {"Additional Credit To",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*adcr + CR_SH*cr_adcr},
  {"id Software for DOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+1)+CR_SH*cr_adcr},
  {"TeamTNT for BOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+2)+CR_SH*cr_adcr},
  {"Lee Killough for MBF",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+3)+CR_SH*cr_adcr},
  {"The DOSDoom-Team for DOSDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+4)+CR_SH*cr_adcr},
  {"Randy Heit for ZDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+5)+CR_SH*cr_adcr},
  {"Michael 'Kodak' Ryssen for DOOMGL",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+6)+CR_SH*cr_adcr},
  {"Jess Haas for lSDLDoom",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+7) + CR_SH*cr_adcr},
  {"all others who helped (see AUTHORS file)",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+8)+CR_SH*cr_adcr},

  // Final entry
  {0,S_SKIP|S_END,m_null},
};

static setup_menu_t *setup_screens[] =
{
  gen_settings3,
  weap_settings1,
  stat_settings1,
  enem_settings1,
  comp_settings1,
  comp_settings2,
  keys_settings1,
  keys_settings2,
  NULL
};


static void M_SetupNextMenu(menu_t *menudef)
{
  currentMenu = menudef;
  itemOn = currentMenu->lastOn;
}

static void M_DrawThermo(int x,int y,int thermWidth,int thermDot )
{
  /*
  *Modification By Barry Mead to allow the Thermometer to have vastly
  *larger ranges. (the thermWidth parameter can now have a value as
  *large as 200.      Modified 1-9-2000  Originally I used it to make
  *the sensitivity range for the mouse better. It could however also
  *be used to improve the dynamic range of music and sound affect
  *volume controls for example.
   */
  int horizScaler; //Used to allow more thermo range for mouse sensitivity.
  int xx = x;
  thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
  horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
  V_DrawNamePatch(xx, y, 0, "M_THERML", CR_DEFAULT, VPT_STRETCH);
  xx += 8;
  for (int i=0;i<thermWidth;i++, xx += horizScaler)
    V_DrawNamePatch(xx, y, 0, "M_THERMM", CR_DEFAULT, VPT_STRETCH);
  xx += (8 - horizScaler);  /* make the right end look even */
  V_DrawNamePatch(xx, y, 0, "M_THERMR", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch((x+8)+thermDot*horizScaler,y,0,"M_THERMO",CR_DEFAULT,VPT_STRETCH);
}

static int M_StringWidth(const char* string)
{
  int c, w = 0;
  while ((c = *string++))
    w += (c = toupper(c) - HU_FONTSTART) < 0 || c >= HU_FONTSIZE ? SPACEWIDTH : hu_font[c].width;
  return w;
}

static int M_StringHeight(const char* string)
{
  int c, l = 1;
  while ((c = *string++))
    if (c == '\n') l++;
  return l * hu_font[0].height;
}

static void M_WriteText(int x,int y,const char* string)
{
  const char *ch = string;
  int cx = x;
  int cy = y;
  int c;

  while ((c = *ch++))
  {
    if (c == '\n') {
      cx = x;
      cy += 12;
      continue;
    }

    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c>= HU_FONTSIZE) {
      cx += 4;
      continue;
    }

    int w = hu_font[c].width;
    if (cx+w > SCREENWIDTH)
      break;
    V_DrawNumPatch(cx, cy, 0, hu_font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);
    cx+=w;
  }
}

static int M_IndexInChoices(const char *str, const char **choices)
{
  int i = 0;

  while (*choices != NULL) {
    if (!strcmp(str, *choices))
      return i;
    i++;
    choices++;
  }
  return 0;
}

static void M_UpdateCurrent(default_t* def)
{
  if (def->current) {
    if (!(demoplayback || demorecording || netgame))  /* killough 8/15/98 */
      *def->current = *def->location.pi;
    else if (*def->current != *def->location.pi)
      warn_about_changes(S_LEVWARN); /* killough 8/15/98 */
  }
}

static void M_DrawString(int cx, int cy, int color, const char* ch)
{
  int  c, w;

  while ((c = *ch++)) {
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
    {
      cx += SPACEWIDTH;    // space
      continue;
    }
    w = hu_font[c].width;
    if (cx + w > 320)
      break;

    V_DrawNumPatch(cx, cy, 0, hu_font[c].lumpnum, color, VPT_STRETCH | VPT_TRANS);
    // The screen is cramped, so trim one unit from each
    // character so they butt up against each other.
    cx += w - 1;
  }
}

static void M_DrawMenuString(int cx, int cy, int color)
{
    M_DrawString(cx, cy, color, menu_buffer);
}

static int M_GetKeyString(int c,int offset)
{
  const char* s;

  if (c >= 33 && c <= 126)
  {
    // The '=', ',', and '.' keys originally meant the shifted
    // versions of those keys, but w/o having to shift them in
    // the game. Any actions that are mapped to these keys will
    // still mean their shifted versions. Could be changed later
    // if someone can come up with a better way to deal with them.

    if (c == '=')      // probably means the '+' key?
      c = '+';
    else if (c == ',') // probably means the '<' key?
      c = '<';
    else if (c == '.') // probably means the '>' key?
      c = '>';
    menu_buffer[offset++] = c; // Just insert the ascii key
    menu_buffer[offset] = 0;
  }
  else
  {
    // Retrieve 4-letter (max) string representing the key

    // cph - Keypad keys, general code reorganisation to
    //  make this smaller and neater.
    if ((0x100 <= c) && (c < 0x200)) {
      if (c == KEYD_KEYPADENTER)
        s = "PADE";
      else {
        strcpy(&menu_buffer[offset], "PAD");
        offset+=4;
        menu_buffer[offset-1] = c & 0xff;
        menu_buffer[offset] = 0;
      }
    } else if ((KEYD_F1 <= c) && (c < KEYD_F10)) {
      menu_buffer[offset++] = 'F';
      menu_buffer[offset++] = '1' + c - KEYD_F1;
      menu_buffer[offset]   = 0;
    } else {
      switch(c) {
      case KEYD_TAB:      s = "TAB";  break;
      case KEYD_ENTER:      s = "ENTR"; break;
      case KEYD_ESCAPE:     s = "ESC";  break;
      case KEYD_SPACEBAR:   s = "SPAC"; break;
      case KEYD_BACKSPACE:  s = "BACK"; break;
      case KEYD_RCTRL:      s = "CTRL"; break;
      case KEYD_LEFTARROW:  s = "LARR"; break;
      case KEYD_UPARROW:    s = "UARR"; break;
      case KEYD_RIGHTARROW: s = "RARR"; break;
      case KEYD_DOWNARROW:  s = "DARR"; break;
      case KEYD_RSHIFT:     s = "SHFT"; break;
      case KEYD_RALT:       s = "ALT";  break;
      case KEYD_CAPSLOCK:   s = "CAPS"; break;
      case KEYD_SCROLLLOCK: s = "SCRL"; break;
      case KEYD_HOME:       s = "HOME"; break;
      case KEYD_PAGEUP:     s = "PGUP"; break;
      case KEYD_END:        s = "END";  break;
      case KEYD_PAGEDOWN:   s = "PGDN"; break;
      case KEYD_INSERT:     s = "INST"; break;
      case KEYD_DEL:        s = "DEL"; break;
      case KEYD_F10:        s = "F10";  break;
      case KEYD_F11:        s = "F11";  break;
      case KEYD_F12:        s = "F12";  break;
      case KEYD_PAUSE:      s = "PAUS"; break;
      default:              s = "JUNK"; break;
      }

      if (s) { // cph - Slight code change
        strcpy(&menu_buffer[offset],s); // string to display
        offset += strlen(s);
      }
    }
  }
  return offset;
}

static void M_DrawStringCentered(int cx, int cy, int color, const char* ch)
{
    M_DrawString(cx - M_StringWidth(ch)/2, cy, color, ch);
}

static void M_DrawItem(const setup_menu_t* s)
{
  const char ResetButtonName[2][8] = {"M_BUTT1","M_BUTT2"};
  int x = s->m_x;
  int y = s->m_y;
  int flags = s->m_flags;
  if (flags & S_RESET)
  {
    // This item is the reset button
    // Draw the 'off' version if this isn't the current menu item
    // Draw the blinking version in tune with the blinking skull otherwise
    V_DrawNamePatch(x, y, 0, ResetButtonName[(flags & (S_HILITE|S_SELECT)) ? whichSkull : 0],
        CR_DEFAULT, VPT_STRETCH);
  }
  else
  { // Draw the item string
    char temp[strlen(s->m_text) + 1];
    int w = 0;
    int color =
      flags & S_SELECT ? CR_SELECT :
      flags & S_HILITE ? CR_HILITE :
      flags & (S_TITLE|S_NEXT|S_PREV) ? CR_TITLE : CR_ITEM; // killough 10/98

    /* killough 10/98:
     * Enhance to support multiline text separated by newlines.
     * This supports multiline items on horizontally-crowded menus.
     */

    memcpy(temp, s->m_text, sizeof(temp));
    for (char *p = temp; (p = strtok(p, "\n")); y += 8, p = NULL)
    {      /* killough 10/98: support left-justification: */
      strcpy(menu_buffer, p);
      if (!(flags & S_LEFTJUST))
        w = M_StringWidth(menu_buffer) + 4;
      M_DrawMenuString(x - w, y ,color);
    }
  }
}

static void M_DrawSetting(const setup_menu_t* s)
{
  int x = s->m_x, y = s->m_y, flags = s->m_flags, color;

  // Determine color of the text. This may or may not be used later,
  // depending on whether the item is a text string or not.

  color = flags & S_SELECT ? CR_SELECT : flags & S_HILITE ? CR_HILITE : CR_SET;

  // Is the item a YES/NO item?

  if (flags & S_YESNO) {
    strcpy(menu_buffer,*s->var.def->location.pi ? "YES" : "NO");
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a simple number?

  if (flags & S_NUM) {
    sprintf(menu_buffer,"%d",*s->var.def->location.pi);
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a key binding?

  if (flags & S_KEY) { // Key Binding
    // Draw the key bound to the action
    if (s->var.m_key) {
      M_GetKeyString(*(s->var.m_key), 0);
      M_DrawMenuString(x,y,color);
    }
    return;
  }

  // Is the item a weapon number?
  // OR, Is the item a colored text string from the Automap?
  //
  // killough 10/98: removed special code, since the rest of the engine
  // already takes care of it, and this code prevented the user from setting
  // their overall weapons preferences while playing Doom 1.
  //
  // killough 11/98: consolidated weapons code with color range code

  if (flags & (S_WEAP|S_CRITEM)) // weapon number or color range
  {
    sprintf(menu_buffer,"%d", *s->var.def->location.pi);
    M_DrawMenuString(x,y, flags & S_CRITEM ? *s->var.def->location.pi : color);
    return;
  }

  // Is the item a paint chip?

  if (flags & S_COLOR) // Automap paint chip
  {
    int ch = *s->var.def->location.pi;
    // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
    // draw the paint chip
    V_FillRect(0, x*SCREENWIDTH/320, (y-1)*SCREENHEIGHT/200,
                  8*SCREENWIDTH/320, 8*SCREENHEIGHT/200,
                0);
    V_FillRect(0, (x+1)*SCREENWIDTH/320, y*SCREENHEIGHT/200,
                      6*SCREENWIDTH/320, 6*SCREENHEIGHT/200,
                (byte)ch);

    if (!ch) // don't show this item in automap mode
      V_DrawNamePatch(x+1,y,0,"M_PALNO", CR_DEFAULT, VPT_STRETCH);
    return;
  }

  // Is the item a chat string?
  // killough 10/98: or a filename?

  if (flags & S_STRING) {
    strcpy(menu_buffer, (char*)*s->var.def->location.ppsz);
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a selection of choices?

  if (flags & S_CHOICE) {
    if (s->var.def->type == def_int) {
      if (s->selectstrings == NULL) {
        sprintf(menu_buffer,"%d",*s->var.def->location.pi);
      } else {
        strcpy(menu_buffer,s->selectstrings[*s->var.def->location.pi]);
      }
    }

    if (s->var.def->type == def_str) {
      sprintf(menu_buffer,"%s", *s->var.def->location.ppsz);
    }

    M_DrawMenuString(x,y,color);
    return;
  }
}

static void M_DrawScreenItems(const setup_menu_t* src, int flags)
{
  if (print_warning_about_changes > 0) { /* killough 8/15/98: print warning */
    if (warning_about_changes & S_BADVAL) {
      strcpy(menu_buffer, "Value out of Range");
      M_DrawMenuString(100,176,CR_RED);
    } else if (warning_about_changes & S_PRGWARN) {
      strcpy(menu_buffer, "Warning: Program must be restarted to see changes");
      M_DrawMenuString(3, 176, CR_RED);
    } else if (warning_about_changes & S_BADVID) {
      strcpy(menu_buffer, "Video mode not supported");
      M_DrawMenuString(80,176,CR_RED);
    } else {
      strcpy(menu_buffer, "Warning: Changes are pending until next game");
      M_DrawMenuString(18,184,CR_RED);
    }
  }

  while (!(src->m_flags & S_END))
  {
    // See if we're to draw the item description (left-hand part)
    if (src->m_flags & S_SHOWDESC)
      M_DrawItem(src);
    // See if we're to draw the setting (right-hand part)
    if (src->m_flags & S_SHOWSET)
      M_DrawSetting(src);
    src++;
  }

  if (flags & S_PREV)
    M_DrawItem(&(setup_menu_t){"<- PREV", S_SKIP|S_PREV, m_null, G_PREV, G_PREVNEXT});
  if (flags & S_NEXT)
    M_DrawItem(&(setup_menu_t){"NEXT ->", S_SKIP|S_NEXT, m_null, G_NEXT, G_PREVNEXT});
  if (flags & S_RESET)
    M_DrawItem(&(setup_menu_t){NULL, S_RESET, m_null, G_RESET_X, G_RESET_Y});
}

static void M_DrawDefVerify(void)
{
  // proff 12/6/98: Drawing of verify box changed for hi-res, it now uses a patch
  V_DrawNamePatch(VERIFYBOXXORG,VERIFYBOXYORG,0,"M_VBOX",CR_DEFAULT,VPT_STRETCH);
  // The blinking messages is keyed off of the blinking of the
  // cursor skull.

  if (whichSkull) { // blink the text
    strcpy(menu_buffer,"Reset to defaults? (Y or N)");
    M_DrawMenuString(VERIFYBOXXORG+8,VERIFYBOXYORG+8,CR_RED);
  }
}

static void M_DrawInstructions(void)
{
  int flags = setup_screens[current_setup_page][set_menu_itemon].m_flags;

  // There are different instruction messages depending on whether you
  // are changing an item or just sitting on it.

  if (setup_select) {
    switch (flags & (S_KEY | S_YESNO | S_WEAP | S_NUM | S_COLOR | S_CRITEM | S_CHAT | S_RESET | S_FILE | S_CHOICE)) {
      case S_KEY:
        M_DrawStringCentered(160, 20, CR_SELECT, "Press key for this action");
        break;
      case S_COLOR:
        M_DrawStringCentered(160, 20, CR_SELECT, "Select color and press ENTER");
        break;
      case S_CHAT:
        M_DrawStringCentered(160, 20, CR_SELECT, "Type/edit chat string and Press ENTER");
        break;
      case S_FILE:
        M_DrawStringCentered(160, 20, CR_SELECT, "Type/edit filename and Press ENTER");
        break;
      case S_CRITEM:
      case S_WEAP:
      case S_NUM:
      case S_YESNO:
      case S_CHOICE:
        M_DrawStringCentered(160, 20, CR_SELECT, "Press left or right to choose");
        break;
      case S_RESET:
        break;
      default:
        lprintf(LO_WARN, "Unrecognised menu item type %d", flags);
    }
  } else {
    if (flags & S_RESET)
      M_DrawStringCentered(160, 20, CR_HILITE, "Press ENTER key to reset to defaults");
    else
      M_DrawStringCentered(160, 20, CR_HILITE, "Press ENTER to Change");
  }
}

static void M_Trans(void) // To reset translucency after setting it in menu
{
  general_translucency = default_translucency;
  if (general_translucency)
    R_InitTranMap(0);
}

static void M_ChangeDemoSmoothTurns(void)
{
  R_SmoothPlaying_Reset(NULL);
}

static void M_StartMessage(const char* string, void* routine, bool input)
{
  messageLastMenuActive = menuactive;
  messageToPrint = 1;
  messageString = string;
  messageRoutine = routine;
  messageNeedsInput = input;
  menuactive = true;
}

static void M_SelectDone(setup_menu_t* ptr)
{
  ptr->m_flags &= ~S_SELECT;
  ptr->m_flags |= S_HILITE;
  S_StartSound(NULL, sfx_itemup);
  setup_select = false;
  if (print_warning_about_changes)     // killough 8/15/98
    print_warning_about_changes--;
}

static void M_DrawSetup(void)
{
  inhelpscreens = true;    // killough 4/6/98: Force status bar redraw

  V_DrawBackground("FLOOR4_6", 0); // Draw background
  V_DrawNamePatch(124, 2, 0, "M_SETUP", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();

  int flags = 0; // S_RESET
  if (current_setup_page > 0) flags |= S_PREV;
  if (setup_screens[current_setup_page + 1]) flags |= S_NEXT;
  M_DrawScreenItems(setup_screens[current_setup_page], flags);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.
  if (default_verify)
    M_DrawDefVerify();
}

static void M_Setup(int choice)
{
  M_SetupNextMenu(&SetupDef);
  setup_active = true;
  setup_select = false;
  default_verify = false;
  current_setup_page = 0;
  set_menu_itemon = 0;
  while (setup_screens[current_setup_page][set_menu_itemon++].m_flags & S_SKIP);
  setup_screens[current_setup_page][--set_menu_itemon].m_flags |= S_HILITE;
}

static void M_ResetDefaults(void)
{
  //
}

static void M_InitDefaults(void)
{
  setup_menu_t **p, *t;
  default_t *dp;

  p = setup_screens;
  while ((t = *(p++)))
    for (; !(t->m_flags & S_END); t++)
      if (t->m_flags & S_HASDEFPTR) {
        if (!(dp = M_LookupDefault(t->var.name)))
          I_Error("M_InitDefaults: Couldn't find config variable %s", t->var.name);
        else
          t->var.def = dp;
      }
}

static void M_DrawMainMenu(void)
{
  V_DrawNamePatch(94, 2, 0, "M_DOOM", CR_DEFAULT, VPT_STRETCH);
}

static void M_ReadThis(int choice)
{
  M_SetupNextMenu(&ReadDef1);
}

static void M_ReadThis2(int choice)
{
  M_SetupNextMenu(&ReadDef2);
}

static void M_FinishReadThis(int choice)
{
  M_SetupNextMenu(&MainDef);
}

static void M_FinishHelp(int choice)        // killough 10/98
{
  M_SetupNextMenu(&MainDef);
}

static void M_DrawReadThis1(void)
{
  inhelpscreens = true;
  if (gamemode == shareware)
    V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);
  else
    M_DrawCredits();
}

static void M_DrawReadThis2(void)
{
  inhelpscreens = true;
  if (gamemode == shareware)
    M_DrawCredits();
  else
    V_DrawNamePatch(0, 0, 0, "CREDIT", CR_DEFAULT, VPT_STRETCH);
}

static void M_DrawEpisode(void)
{
  V_DrawNamePatch(54, 38, 0, "M_EPISOD", CR_DEFAULT, VPT_STRETCH);
}

static void M_Episode(int choice)
{
  if (gamemode == shareware && choice)
  {
    M_StartMessage(s_SWSTRING,NULL,false);
    M_SetupNextMenu(&ReadDef1);
    return;
  }
  epi = choice;
  M_SetupNextMenu(&NewDef);
}

static void M_DrawNewGame(void)
{
  V_DrawNamePatch(96, 14, 0, "M_NEWG", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(54, 38, 0, "M_SKILL",CR_DEFAULT, VPT_STRETCH);
}

static void M_RestartLevelResponse(int ch)
{
  if (ch != key_enter)
    return;

  if (demorecording)
    exit(0);

  currentMenu->lastOn = itemOn;
  M_ClearMenus();
  G_RestartLevel();
}

static void M_NewGame(int choice)
{
  if (netgame && !demoplayback)
  {
    if (compatibility_level < lxdoom_1_compatibility)
      M_StartMessage(s_NEWGAME,NULL,false);
    else // CPhipps - query restarting the level
      M_StartMessage(s_RESTARTLEVEL,M_RestartLevelResponse,true);
  }
  else if (demorecording)
    M_StartMessage("you can't start a new game\nwhile recording a demo!\n\n"PRESSKEY, NULL, false);
  else if (gamemode == commercial)
    M_SetupNextMenu(&NewDef);
  else
    M_SetupNextMenu(&EpiDef);
}

static void M_VerifyNightmare(int ch)
{
  if (ch != key_enter)
    return;

  G_DeferedInitNew(sk_nightmare,epi+1,1);
  M_ClearMenus();
}

static void M_ChooseSkill(int choice)
{
  if (choice == sk_nightmare)
  {
    M_StartMessage(s_NIGHTMARE, M_VerifyNightmare, true);
    return;
  }
  G_DeferedInitNew(choice, epi+1, 1);
  M_ClearMenus();
}

static void M_DrawSaveLoadBorder(int x,int y)
{
  V_DrawNamePatch(x-8, y+7, 0, "M_LSLEFT", CR_DEFAULT, VPT_STRETCH);
  for (int i = 0 ; i < 24 ; i++, x += 8)
    V_DrawNamePatch(x, y+7, 0, "M_LSCNTR", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(x, y+7, 0, "M_LSRGHT", CR_DEFAULT, VPT_STRETCH);
}

static void M_ReadSaveStrings(void)
{
  for (int i = 0 ; i < 8 ; i++) {
    char name[PATH_MAX+1];
    FILE *fp;

    G_SaveGameName(name,sizeof(name),i,false);

    if ((fp = fopen(name, "rb"))) {
      fread(&savegamestrings[i], SAVESTRINGSIZE, 1, fp);
      //sprintf(&savegamestrings[i], "Slot %d", i);
      fclose(fp);
      LoadMenue[i].status = 1;
    } else {
      sprintf(savegamestrings[i], "Slot %d (empty)", i);
      LoadMenue[i].status = 0;
    }
  }
}

static void M_DrawLoad(void)
{
  V_DrawNamePatch(72 ,LOADGRAPHIC_Y, 0, "M_LOADG", CR_DEFAULT, VPT_STRETCH);
  for (int i = 0 ; i < 8 ; i++) {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
  }
}

static void M_LoadSelect(int choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.
  G_LoadGame(choice, false);
  M_ClearMenus();
}

static void M_VerifyForcedLoadGame(int ch)
{
  if (ch == key_enter)
    G_ForcedLoadGame();
  M_ClearMenus();
}

void M_ForcedLoadGame(const char *msg)
{
  M_StartMessage(msg, M_VerifyForcedLoadGame, true); // free()'d above
}

static void M_LoadGame(int choice)
{
  /* killough 5/26/98: exclude during demo recordings
  *cph - unless a new demo */
  if (demorecording && (compatibility_level < prboom_2_compatibility))
  {
    M_StartMessage("you can't load a game\nwhile recording an old demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  M_SetupNextMenu(&LoadDef);
  M_ReadSaveStrings();
}

static void M_DrawSave(void)
{
  V_DrawNamePatch(72, LOADGRAPHIC_Y, 0, "M_SAVEG", CR_DEFAULT, VPT_STRETCH);
  for (int i = 0 ; i < 8 ; i++)
  {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
  }
}

static void M_DoSave(int slot)
{
  sprintf(savegamestrings[slot], "Slot %d:Episode %d Map %d", slot, gameepisode, gamemap);
  G_SaveGame(slot, savegamestrings[slot]);
  M_ClearMenus();

  // PICK QUICKSAVE SLOT YET?
  if (quickSaveSlot == -2)
    quickSaveSlot = slot;
}

static void M_SaveSelect(int choice)
{
  saveSlot = choice;
  M_DoSave(saveSlot);
}

static void M_SaveGame(int choice)
{
  // killough 10/6/98: allow savegames during single-player demo playback
  if (!usergame && (!demoplayback || netgame))
  {
    M_StartMessage(s_SAVEDEAD,NULL,false);
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  M_SetupNextMenu(&SaveDef);
  M_ReadSaveStrings();
}

static void M_DrawOptions(void)
{
  V_DrawNamePatch(108, 15, 0, "M_OPTTTL", CR_DEFAULT, VPT_STRETCH);
  M_DrawThermo(OptionsDef.x, OptionsDef.y+LINEHEIGHT*2, 9, screenSize);
}

static void M_Options(int choice)
{
  M_SetupNextMenu(&OptionsDef);
}

static void M_QuitResponse(int ch)
{
  byte quitsounds[2][8] = {
    {sfx_pldeth, sfx_dmpain, sfx_popain, sfx_slop, sfx_telept, sfx_posit1, sfx_posit3, sfx_sgtatk},
    {sfx_vilact, sfx_getpow, sfx_boscub, sfx_slop,sfx_skeswg, sfx_kntdth, sfx_bspact, sfx_sgtatk},
  };

  if (ch != key_enter)
    return;

  if ((!netgame || demoplayback) && !nosfxparm && snd_card) // avoid delay if no sound card
  {
    S_StartSound(NULL,quitsounds[gamemode == commercial][(gametic>>2)&7]);

    // wait till all sounds stopped or 3 seconds are over
    int i = 30;
    while (i>0) {
      I_uSleep(100000); // CPhipps - don't thrash cpu in this loop
      if (!I_AnySoundStillPlaying())
        break;
      i--;
    }
  }
  exit(0); // killough
}

static void M_QuitDOOM(int choice)
{
  sprintf(menu_buffer,"%s\n\n%s", endmsg[gametic%(NUM_QUITMESSAGES-1)+1], s_DOSY);
  M_StartMessage(menu_buffer,M_QuitResponse,true);
}

static void M_LevelSelect(int choice)
{
  M_SetupNextMenu(&LevelSelectDef);
}

static void M_ToggleMap(int choice)
{
  AM_Responder(&(event_t){ev_keydown, key_map});
}

static void M_DrawLevelSelect(void)
{
  M_WriteText(72, 12, "Level Select");
}

static void M_LevelSelectSelect(int choice)
{
  M_ClearMenus();
  G_DeferedInitNew(gameskill, gameepisode, choice);
}

static void M_Cheats(int choice)
{
  M_SetupNextMenu(&CheatsDef);
}

static void M_DrawCheats(void)
{
  M_WriteText(72, 12, "Cheats");
  for (int i = 0; i < CheatsDef.numitems; i++) {
    M_DrawSaveLoadBorder(CheatsDef.x, CheatsDef.y + LINEHEIGHT*i);
    M_WriteText(CheatsDef.x, CheatsDef.y + LINEHEIGHT*i, cheats_list[i].desc);
  }
}

static void M_CheatSelect(int choice)
{
  const char *code = cheats_list[choice].code;

  for (int i = 0; i < strlen(code); i++)
    M_FindCheats(code[i]);
}

static void M_DrawSound(void)
{
  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT*1, 16, snd_SfxVolume);
  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT*3, 16, snd_MusicVolume);
}

static void M_Sound(int choice)
{
  M_SetupNextMenu(&SoundDef);
}

static void M_SfxVol(int choice)
{
  if (choice == 1 && snd_SfxVolume < 15)
      snd_SfxVolume++;
  if (choice == 0 && snd_SfxVolume > 0)
    snd_SfxVolume--;

  S_SetSfxVolume(snd_SfxVolume);
}

static void M_MusicVol(int choice)
{
  if (choice == 1 && snd_MusicVolume < 15)
      snd_MusicVolume++;
  if (choice == 0 && snd_MusicVolume > 0)
    snd_MusicVolume--;

  S_SetMusicVolume(snd_MusicVolume);
}

static void M_QuickSaveResponse(int ch)
{
  if (ch == key_enter)  {
    M_DoSave(quickSaveSlot);
    S_StartSound(NULL,sfx_swtchx);
  }
}

static void M_QuickSave(void)
{
  if (!usergame && (!demoplayback || netgame)) { /* killough 10/98 */
    S_StartSound(NULL,sfx_oof);
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  if (quickSaveSlot < 0) {
    M_StartControlPanel();
    M_ReadSaveStrings();
    M_SetupNextMenu(&SaveDef);
    quickSaveSlot = -2; // means to pick a slot now
    return;
  }
  sprintf(menu_buffer,s_QSPROMPT,savegamestrings[quickSaveSlot]);
  M_StartMessage(menu_buffer,M_QuickSaveResponse,true);
}

static void M_QuickLoadResponse(int ch)
{
  if (ch == key_enter) {
    M_LoadSelect(quickSaveSlot);
    S_StartSound(NULL,sfx_swtchx);
  }
}

static void M_QuickLoad(void)
{
  // cph - removed restriction against quickload in a netgame
  if (demorecording) {  // killough 5/26/98: exclude during demo recordings
    M_StartMessage("you can't quickload\nwhile recording a demo!\n\n"PRESSKEY, NULL, false);
    return;
  }

  if (quickSaveSlot < 0) {
    M_StartMessage(s_QSAVESPOT,NULL,false);
    return;
  }
  sprintf(menu_buffer,s_QLPROMPT,savegamestrings[quickSaveSlot]);
  M_StartMessage(menu_buffer,M_QuickLoadResponse,true);
}

static void M_EndGameResponse(int ch)
{
  if (ch != key_enter)
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (demorecording || singledemo)
    G_CheckDemoStatus();

  currentMenu->lastOn = itemOn;
  M_ClearMenus ();
  D_StartTitle ();
}

static void M_EndGame(int choice)
{
  if (netgame)
    M_StartMessage(s_NETEND,NULL,false);
  else
    M_StartMessage(s_ENDGAME,M_EndGameResponse,true);
}

static void M_SizeDisplay(int choice)
{
  switch(choice) {
  case 0:
    if (screenSize > 0) {
      screenblocks--;
      screenSize--;
      hud_displayed = 0;
    }
    break;
  case 1:
    if (screenSize < 8) {
      screenblocks++;
      screenSize++;
    }
    else
      hud_displayed = !hud_displayed;
    break;
  }
  R_SetViewSize (screenblocks /*, detailLevel obsolete -- killough */);
}

static void M_DrawHelp (void)
{
  inhelpscreens = true;                        // killough 10/98
  V_DrawBackground("FLOOR4_6", 0);
  M_DrawScreenItems(helpstrings, 0);
}

static void M_DoNothing(int choice)
{
  //
}

static void M_ClearMenus(void)
{
  menuactive = 0;
  print_warning_about_changes = 0;     // killough 8/15/98
  default_verify = 0;                  // killough 10/98

  // if (!netgame && usergame && paused)
  //     sendpause = true;
}

bool M_Responder(event_t* ev)
{
  int ch = ev->data1;

  // We only care about keydown
  if (ev->type != ev_keydown)
    return false;

  // Take care of any messages that need input
  if (messageToPrint)
  {
    lprintf(LO_INFO, "M_Responder: message needs input %d\n", messageNeedsInput);
    if (messageNeedsInput && !(ch == ' ' || ch == key_enter || ch == key_backspace || ch == key_escape))
      return false;
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    if (messageRoutine)
      messageRoutine(ch);
    menuactive = false;
    S_StartSound(NULL,sfx_swtchx);
    return true;
  }

  // If there is no active menu displayed...

  if (!menuactive)
  {
    if (ch == key_autorun)      // Autorun
    {
      autorun = !autorun;
      return true;
    }

    if (ch == key_help)      // Help key
    {
      M_StartControlPanel();
      currentMenu = &HelpDef;
      itemOn = 0;
      S_StartSound(NULL,sfx_swtchn);
      return true;
    }

    if (ch == key_savegame)     // Save Game
    {
      M_StartControlPanel();
      S_StartSound(NULL,sfx_swtchn);
      M_SaveGame(0);
      return true;
    }

    if (ch == key_loadgame)     // Load Game
    {
      M_StartControlPanel();
      S_StartSound(NULL,sfx_swtchn);
      M_LoadGame(0);
      return true;
    }

    if (ch == key_quicksave)      // Quicksave
    {
      S_StartSound(NULL,sfx_swtchn);
      M_QuickSave();
      return true;
    }

    if (ch == key_endgame)      // End game
    {
      S_StartSound(NULL,sfx_swtchn);
      M_EndGame(0);
      return true;
    }

    if (ch == key_quickload)      // Quickload
    {
      S_StartSound(NULL,sfx_swtchn);
      M_QuickLoad();
      return true;
    }

    if (ch == key_quit)       // Quit DOOM
    {
      S_StartSound(NULL,sfx_swtchn);
      M_QuitDOOM(0);
      return true;
    }

    if (ch == key_gamma)       // gamma toggle
    {
      usegamma = (usegamma + 1) % 5;
      players[consoleplayer].message =
        usegamma == 0 ? s_GAMMALVL0 :
        usegamma == 1 ? s_GAMMALVL1 :
        usegamma == 2 ? s_GAMMALVL2 :
        usegamma == 3 ? s_GAMMALVL3 :
        s_GAMMALVL4;
      V_SetPalette(0);
      return true;
    }

    if (ch == key_hud)   // heads-up mode
    {
      if (automapmode & am_active)    // jff 2/22/98
        return false;                  // HUD mode control
      if (screenSize<8)                // function on default F5
        while (screenSize<8 || !hud_displayed) // make hud visible
          M_SizeDisplay(1);            // when configuring it
      else
      {
        hud_displayed = 1;               //jff 3/3/98 turn hud on
        hud_active = (hud_active+1)%3;   // cycle hud_active
        if (!hud_active)                 //jff 3/4/98 add distributed
        {
          hud_distributed = !hud_distributed; // to cycle
          HU_MoveHud(); //jff 3/9/98 move it now to avoid glitch
        }
      }
      return true;
    }
  }
  // Pop-up Main menu?

  if (!menuactive)
  {
    if (ch == key_escape)
    {
      M_StartControlPanel();
      S_StartSound(NULL,sfx_swtchn);
      return true;
    }
    return false;
  }


  if (setup_active)
  {
    setup_menu_t *ptr1= &setup_screens[current_setup_page][set_menu_itemon];

    // Catch the response to the 'reset to default?' verification
    if (default_verify)
    {
      if (ch == key_enter) {
        M_ResetDefaults();
        default_verify = false;
        M_SelectDone(ptr1);
      }
      else if (ch == key_escape || ch == key_backspace) {
        default_verify = false;
        M_SelectDone(ptr1);
      }
      return true;
    }

    // Common processing for some items

    if (setup_select)  // changing an entry
    {
      if (ch == key_escape) // Exit key = no change
      {
        M_SelectDone(ptr1);                           // phares 4/17/98
        return true;
      }

      if (ptr1->m_flags & S_YESNO) // yes or no setting?
      {
        if (ch == key_menu_left || ch == key_menu_right)
        {
          *ptr1->var.def->location.pi = !*ptr1->var.def->location.pi;
          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
            warn_about_changes(ptr1->m_flags & (S_LEVWARN | S_PRGWARN));
          else
            M_UpdateCurrent(ptr1->var.def);
        }
        else if (ch == key_enter)
        {
          if (ptr1->action)
            ptr1->action();
          M_SelectDone(ptr1);
        }
        return true;
      }

      if (ptr1->m_flags & (S_CRITEM|S_WEAP))
      {
        int value = *ptr1->var.def->location.pi;

        if (ch == key_menu_left) {
          value--;
        }
        if (ch == key_menu_right) {
          value++;
        }

        if (value < 0 || value > 9)
          return true; // ignore

        *ptr1->var.def->location.pi = value;

        if (ch == key_enter)
        {
          if (ptr1->action)
            ptr1->action();
          M_SelectDone(ptr1);
        }
        return true;
      }

      if (ptr1->m_flags & S_NUM) // number?
      {
        int value = *ptr1->var.def->location.pi;

        if (ch == key_menu_left) {
          value--;
        }
        if (ch == key_menu_right) {
          value++;
        }
        if (value != *ptr1->var.def->location.pi)
        {
          if ((ptr1->var.def->minvalue != UL && value < ptr1->var.def->minvalue) ||
              (ptr1->var.def->maxvalue != UL && value > ptr1->var.def->maxvalue))
            warn_about_changes(S_BADVAL);
          else {
            *ptr1->var.def->location.pi = value;
          }
        }

        if (ch == key_enter)
        {
          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
            warn_about_changes(ptr1->m_flags &
            (S_LEVWARN | S_PRGWARN));
          else
            M_UpdateCurrent(ptr1->var.def);

          if (ptr1->action)      // killough 10/98
            ptr1->action();

          M_SelectDone(ptr1);
        }
        return true;
      }

      if (ptr1->m_flags & S_CHOICE) // selection of choices?
      {
        if (ch == key_menu_left)
        {
          if (ptr1->var.def->type == def_int)
          {
            int value = *ptr1->var.def->location.pi;
            value = value - 1;
            if ((ptr1->var.def->minvalue != UL && value < ptr1->var.def->minvalue))
              value = ptr1->var.def->minvalue;
            if ((ptr1->var.def->maxvalue != UL && value > ptr1->var.def->maxvalue))
              value = ptr1->var.def->maxvalue;
            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL,sfx_pstop);
            *ptr1->var.def->location.pi = value;
          }

          if (ptr1->var.def->type == def_str)
          {
            int old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz, ptr1->selectstrings);
            int value = old_value - 1;
            if (value < 0)
              value = 0;
            if (old_value != value)
              S_StartSound(NULL,sfx_pstop);
            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }
        else if (ch == key_menu_right)
        {
          if (ptr1->var.def->type == def_int)
          {
            int value = *ptr1->var.def->location.pi;
            value = value + 1;
            if ((ptr1->var.def->minvalue != UL && value < ptr1->var.def->minvalue))
              value = ptr1->var.def->minvalue;
            if ((ptr1->var.def->maxvalue != UL && value > ptr1->var.def->maxvalue))
              value = ptr1->var.def->maxvalue;
            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL,sfx_pstop);
            *ptr1->var.def->location.pi = value;
          }
          if (ptr1->var.def->type == def_str)
          {
            int old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz, ptr1->selectstrings);
            int value = old_value + 1;
            if (ptr1->selectstrings[value] == NULL)
              value = old_value;
            if (old_value != value)
              S_StartSound(NULL,sfx_pstop);
            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }
        else if (ch == key_enter)
        {
          // phares 4/14/98:
          // If not in demoplayback, demorecording, or netgame,
          // and there's a second variable in var2, set that
          // as well

          // killough 8/15/98: add warning messages

          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
            warn_about_changes(ptr1->m_flags & (S_LEVWARN | S_PRGWARN));
          else
            M_UpdateCurrent(ptr1->var.def);

          if (ptr1->action)      // killough 10/98
            ptr1->action();
          M_SelectDone(ptr1);                           // phares 4/17/98
        }
        return true;
      }
    }

    // Not changing any items on the Setup screens. See if we're
    // navigating the Setup menus or selecting an item to change.

    if (ch == key_menu_down)
    {
      ptr1->m_flags &= ~S_HILITE;
      do {
        if (ptr1->m_flags & S_END)
          set_menu_itemon = 0;
        else
          set_menu_itemon++;
        ptr1 = &setup_screens[current_setup_page][set_menu_itemon];
      } while (ptr1->m_flags & S_SKIP);
      M_SelectDone(ptr1);
      return true;
    }
    else if (ch == key_menu_up)
    {
      ptr1->m_flags &= ~S_HILITE;
      do {
        if (set_menu_itemon == 0) {
          do {
            set_menu_itemon++;
          }
          while(!(setup_screens[current_setup_page][set_menu_itemon].m_flags & S_END));
        }
        set_menu_itemon--;
      }
      while(setup_screens[current_setup_page][set_menu_itemon].m_flags & S_SKIP);
      M_SelectDone(&setup_screens[current_setup_page][set_menu_itemon]);
      return true;
    }
    else if (ch == key_enter)
    {
      if (ptr1->m_flags & S_RESET)
        default_verify = true;
      ptr1->m_flags |= S_SELECT;
      setup_select = true;
      S_StartSound(NULL, sfx_itemup);
      return true;
    }
    else if ((ch == key_escape) || (ch == key_backspace))
    {
      if (ch == key_escape) // Clear all menus
        M_ClearMenus();
      else if (currentMenu->prevMenu) // key_backspace = return to Setup Menu
      {
        currentMenu = currentMenu->prevMenu;
        itemOn = currentMenu->lastOn;
        S_StartSound(NULL,sfx_swtchn);
      }
      ptr1->m_flags &= ~(S_HILITE|S_SELECT);// phares 4/19/98
      setup_active = false;
      default_verify = false;       // phares 4/19/98
      HU_Start();    // catch any message changes // phares 4/19/98
      S_StartSound(NULL,sfx_swtchx);
      return true;
    }
    /* Prev/Next screen navigation */
    else if (ch == key_menu_left)
    {
      if (current_setup_page > 0)
      {
        ptr1->m_flags &= ~S_HILITE;
        current_setup_page--;
        set_menu_itemon = 0;
        print_warning_about_changes = false; // killough 10/98
        while (setup_screens[current_setup_page][set_menu_itemon++].m_flags&S_SKIP);
        setup_screens[current_setup_page][--set_menu_itemon].m_flags |= S_HILITE;
        S_StartSound(NULL, sfx_pstop);  // killough 10/98
        return true;
      }
    }
    else if (ch == key_menu_right)
    {
      if (setup_screens[current_setup_page+1])
      {
        ptr1->m_flags &= ~S_HILITE;
        current_setup_page++;
        set_menu_itemon = 0;
        print_warning_about_changes = false; // killough 10/98
        while (setup_screens[current_setup_page][set_menu_itemon++].m_flags&S_SKIP);
        setup_screens[current_setup_page][--set_menu_itemon].m_flags |= S_HILITE;
        S_StartSound(NULL, sfx_pstop);  // killough 10/98
        return true;
      }
    }
  } // End of Setup Screen processing



  // From here on, these navigation keys are used on the BIG FONT menus
  // like the Main Menu.

  if (ch == key_menu_down)
  {
    do {
      if (itemOn+1 > currentMenu->numitems-1) itemOn = 0;
      else itemOn++;
      S_StartSound(NULL,sfx_pstop);
    } while(currentMenu->menuitems[itemOn].status==-1);
    return true;
  }
  else if (ch == key_menu_up)
  {
    do {
      if (!itemOn) itemOn = currentMenu->numitems-1;
      else itemOn--;
      S_StartSound(NULL,sfx_pstop);
    } while(currentMenu->menuitems[itemOn].status==-1);
    return true;
  }
  else if (ch == key_menu_left)
  {
    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartSound(NULL,sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
  }
  else if (ch == key_menu_right)
  {
    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartSound(NULL,sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(1);
    }
    return true;
  }
  else if (ch == key_enter)
  {
    if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status)
    {
      currentMenu->lastOn = itemOn;
      if (currentMenu->menuitems[itemOn].status == 2)
      {
        currentMenu->menuitems[itemOn].routine(1);   // right arrow
        S_StartSound(NULL,sfx_stnmov);
      }
      else
      {
        currentMenu->menuitems[itemOn].routine(itemOn);
        S_StartSound(NULL,sfx_pistol);
      }
    }
    return true;
  }
  else if (ch == key_escape)
  {
    currentMenu->lastOn = itemOn;
    M_ClearMenus();
    S_StartSound(NULL,sfx_swtchx);
    return true;
  }
  else if (ch == key_backspace)
  {
    currentMenu->lastOn = itemOn;
    if (currentMenu->prevMenu)
    {
      currentMenu = currentMenu->prevMenu;
      itemOn = currentMenu->lastOn;
      S_StartSound(NULL,sfx_swtchn);
    }
    return true;
  }

  return false;
}

void M_DrawCredits(void)     // killough 10/98: credit screen
{
  inhelpscreens = true;
  V_DrawBackground(gamemode==shareware ? "CEIL5_1" : "MFLR8_4", 0);
  V_DrawNamePatch(115,9,0, "PRBOOM",CR_GOLD, VPT_TRANS | VPT_STRETCH);
  M_DrawScreenItems(cred_settings, 0);
}

void M_StartControlPanel(void)
{
  // intro might call this repeatedly
  if (menuactive)
    return;

  //jff 3/24/98 make default skill menu choice follow -skill or defaultskill
  //from command line or config file
  NewDef.lastOn = defaultskill - 1;
  default_verify = 0;                  // killough 10/98
  menuactive = 1;
  currentMenu = &MainDef;         // JDC
  itemOn = currentMenu->lastOn;   // JDC
  print_warning_about_changes = false;   // killough 11/98
}

void M_Drawer(void)
{
  inhelpscreens = false;

  // Horiz. & Vertically center string and print it.
  // killough 9/29/98: simplified code, removed 40-character width limit
  if (messageToPrint)
  {
    char temp[strlen(messageString) + 1];
    memcpy(temp, messageString, sizeof(temp));

    int y = 100 - M_StringHeight(messageString)/2;
    for (char *p = temp; *p;)
    {
      char *string = p, c;
      while ((c = *p) && *p != '\n')
        p++;
      *p = 0;
      M_WriteText(160 - M_StringWidth(string)/2, y, string);
      y += hu_font[0].height;
      if ((*p = c))
        p++;
    }
  }
  else if (menuactive)
  {
    if (currentMenu->routine)
      currentMenu->routine();     // call Draw routine

    int x = currentMenu->x;
    int y = currentMenu->y;
    int max = currentMenu->numitems;

    // DRAW MENU
    for (int i=0;i<max;i++)
    {
      if (currentMenu->menuitems[i].name[0] == ':')
        M_WriteText(x, y + 4, currentMenu->menuitems[i].name + 1);
      else if (currentMenu->menuitems[i].name[0])
        V_DrawNamePatch(x, y, 0, currentMenu->menuitems[i].name, CR_DEFAULT, VPT_STRETCH);
      y += LINEHEIGHT;
    }

    // DRAW SKULL
    V_DrawNamePatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,0,
        whichSkull ? "M_SKULL2" : "M_SKULL1", CR_DEFAULT, VPT_STRETCH);
  }
}

void M_Ticker(void)
{
  if (--skullAnimCounter <= 0)
  {
    whichSkull ^= 1;
    skullAnimCounter = 8;
  }
}

void M_Init(void)
{
  M_InitDefaults();                // killough 11/98
  currentMenu = &MainDef;
  menuactive = 0;
  itemOn = currentMenu->lastOn;
  whichSkull = 0;
  skullAnimCounter = 10;
  screenSize = screenblocks - 3;
  messageToPrint = 0;
  messageString = NULL;
  messageLastMenuActive = menuactive;
  quickSaveSlot = -1;

  // Detect the number of available episodes
  for (int i = 0; i < 6; i++) {
    if (W_CheckNumForName(EpisodeMenu[i].name) == -1) {
      EpiDef.numitems = i;
      break;
    }
  }

  // DOOM 2 has no episodes
  if (!EpiDef.numitems || gamemode == commercial)
  {
    NewDef.prevMenu = &MainDef;
  }

  setup_menu_t *src = helpstrings;
  while (!(src->m_flags & S_END))
  {
    if ((strncmp(src->m_text,"PLASMA",6) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"BFG",3) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"SSG",3) == 0) && (gamemode != commercial))
      src->m_flags = S_SKIP; // Don't show setting or item
    src++;
  }

  M_ChangeDemoSmoothTurns();
}
