/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//******************************************************************************
//
// RT_MENU.C
//    Contains the menu stuff!
//
//******************************************************************************


#include "rt_def.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#if PLATFORM_DOS
#include <conio.h>
#include <dos.h>
#include <io.h>
#elif PLATFORM_UNIX
#include <unistd.h>
#include "SDL.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "_rt_menu.h"
#include "rt_menu.h"
#include "rt_sound.h"
#include "fx_man.h"
#include "rt_build.h"
#include "rt_in.h"
#include "isr.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_util.h"
#include "rt_main.h"
#include "rt_playr.h"
#include "rt_rand.h"
#include "rt_game.h"
#include "rt_floor.h"
#include "rt_draw.h"
#include "rt_view.h"
#include "rt_str.h"
#include "rt_vid.h"
#include "rt_ted.h"
#include "rt_com.h"
#include "lumpy.h"
#include "rt_cfg.h"
#include "version.h"
#include "modexlib.h"
#include "rt_msg.h"
#include "rt_net.h"
#include "rt_spbal.h"
#include "rt_scale.h"

#include "rt_battl.h"
#include "develop.h"
#include "rt_actor.h"
#include "rt_stat.h"
//MED
#include "memcheck.h"


//******************************************************************************
//
// GLOBALS
//
//******************************************************************************

#define DELAYAMT  2

#define SNDCARDS  12

#define CP_NO     0
#define CP_ESC    -1
#define CP_YES    1

int CP_Acknowledge;

boolean POK = false;
char    pword[ 13 ];

boolean ingame    = false;
boolean inmenu    = false;
boolean pickquick = false;
boolean NewGame   = false;

//
// Global window coords
//
int PrintX;
int PrintY;
int WindowX;
int WindowY;
int WindowH = 160;
int WindowW;

int px;
int py;
int bufferheight;
int bufferwidth;

cfont_t *IFont;
font_t  *CurrentFont;
font_t  *newfont1;
font_t  *smallfont;
font_t  *bigfont;
font_t  *tinyfont;

boolean loadedgame = false;

battle_type BATTLE_Options[ battle_NumBattleModes ];

int quicksaveslot=-1;

//******************************************************************************
//
// LOCALS
//
//******************************************************************************

char order[ 21 ] = {
   di_west, di_east, di_north, di_south, bt_run, bt_use, bt_attack,
   bt_strafe, bt_strafeleft, bt_straferight, bt_lookup, bt_lookdown,
   bt_aimbutton, bt_horizonup, bt_horizondown, bt_swapweapon, bt_dropweapon,
   bt_turnaround, bt_autorun, bt_message, bt_directmsg
   };

// bt_pistol, bt_dualpistol, bt_mp40, bt_missileweapon, bt_recordsound,

#define RETURNVAL    100

static boolean loadsavesound = false;
static int numdone;

static char *endStrings[ 7 ] =
   {
   "Press Y to reformat \nand install Windows.\0\0",
   "Press Y to activate \nguillotine.\0\0",
   "Press Y to release \nthe cyanide gas.\0\0",
   "Press Y to open \ntrap door.\0\0",
   "Press Y to drive your \ncar off the cliff.\0\0",
   "Press Y to pull \nyour plug.\0\0",
   "Press Y to activate \nelectric chair.\0\0"
   };

static char *BattleModeDescriptions[ battle_NumBattleModes - 1 ] =
   {
   "Kill your enemies!  Don't get killed!  Kill some more!",
   "Score more points for more difficult kills.",
   "Collect the most triads to win the game.  Whoopee!",
   "Collect triads to win the game--this time with weapons!",
   "Armed hunters vs. unarmed prey--then the tables are turned!",
   "Tag your enemies.  Run away.  Lowest points wins.",
   "Chase roving 'Eluders'--tag them for points.",
   "Use weapons to destroy roving Eluder triads for points.",
   "Capture the opposing team's triad while guarding your own."
   };

static char *BattleOptionDescriptions[ 9 ] =
   {
   "Adjust the Gravitational Constant of the game universe!",
   "Adjust the top speed for all players in the game",
   "Adjust the amount of ammo in all missile weapons",
   "Adjust the hit points of all players in the game",
   "Radically change the way the game plays",
   "Adjust the light characteristics of the game",
   "Adjust the point goal of the game",
   "Adjust the damage done by environment dangers",
   "Adjust the time limit for the game"
   };

static char *GravityOptionDescriptions[ 3 ] =
   {
   "Similar to gravity on the moon",
   "Normal Gravity (9.81 m/s^2 !)",
   "Similar to gravity on Jupiter"
   };

static char *SpeedOptionDescriptions[ 2 ] =
   {
   "Player speeds are determined by character",
   "All players can move at the fastest possible speed"
   };

static char *AmmoOptionDescriptions[ 3 ] =
   {
   "One piece of ammo per missile weapon",
   "Normal ammo for all missile weapons",
   "Infinite ammo for all missile weapons"
   };

static char *HitPointsOptionDescriptions[ 7 ] =
   {
   "One hit point for each player",
   "25 hit points for each player",
   "Hit points determined by character",
   "100 hit points for each player",
   "250 hit points for each player (default)",
   "500 hit points for each player",
   "4000 hit points for each player",
   };

static char *RadicalOptionDescriptions[ 8 ] =
   {
   "Control spawning of environment dangers",
   "Control spawning of health items",
   "Control spawning of missile weapons",
   "Spawn mines instead of health items",
   "Objects reappear a short time after being picked up",
   "Missile weapons remain when picked up",
   "Weapons are chosen randomly at the start of the game",
   "Killing yourself or a team member counts as a suicide"
   };

static char *LightLevelOptionDescriptions[ 6 ] =
   {
   "Very dark, cave-like",
   "Lighting determined by level design",
   "Full brightness",
   "Bright with fog",
   "Periodic lighting (voobing)",
   "Dark with lightning"
   };

static char *PointGoalOptionDescriptions[ 9 ] =
   {
   "One Point/Kill",
   "5 Points/Kills",
   "11 Points/Kills",
   "21 Points/Kills",
   "50 Points/Kills",
   "100 Points/Kills",
   "Random Points/Kills",
   "Random Points/Kills but goal is not revealed",
   "Infinite Points/Kills"
   };

static char *DangerDamageOptionDescriptions[ 3 ] =
   {
   "Environmental dangers' damage is relatively low",
   "Environmental dangers' damage normal",
   "One touch and you are dead!"
   };

static char *TimeLimitOptionDescriptions[ 8 ] =
   {
   "One Minute",
   "2 Minutes",
   "5 Minutes",
   "10 Minutes",
   "21 Minutes",
   "30 Minutes",
   "99 Minutes",
   "No Time Limit"
   };


static char *BattleModeNames[ battle_NumBattleModes - 1 ] =
   {
   "NORMAL COMM-BAT", "SCORE MORE", "COLLECTOR", "SCAVENGER",
   "HUNTER", "TAG", "ELUDER", "DELUDER", "CAPTURE THE TRIAD"
   };

static int OptionNums[ 12 ] =
   {
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
   };

static int HitPointNums[ 7 ] =
   {
   1, 25, bo_character_hitpoints, 100, 250, 500, 4000
   };

static int KillNums[ 9 ] =
   {
   1, 5, 11, 21, 50, 100, bo_kills_random,
   bo_kills_blind, bo_kills_infinite
   };

static int GravityNums[ 3 ] =
   {
   LOW_GRAVITY, NORMAL_GRAVITY, HIGH_GRAVITY
   };

static int TimeLimitNums[ 8 ] =
   {
   1, 2, 5, 10, 21, 30, 99, bo_time_infinite
   };

static int DangerNums[ 3 ] =
   {
   bo_danger_low, bo_danger_normal, bo_danger_kill
   };

static int MenuNum = 0;
static int handlewhich;
static int CSTactive = 0;
static boolean INFXSETUP = false;
static int MaxVoices;
static int MaxBits;
static int MaxChannels;

//
// MENU CURSOR SHAPES
//

#define MAXCURSORNUM 24

static int cursorwidth;
static int cursorheight;
static int yinc;

static char *FontNames[] = { "itnyfont", "ifnt", "sifont", "lifont" };
static int   FontSize[]  = { 6, 7, 9, 14 };
static char *SmallCursor = "smallc01";
static char *LargeCursor = "cursor01";
static char *CursorLump  = "cursor01";
static int CursorNum = 0;
static int CursorFrame[ MAXCURSORNUM ] =
   {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
   4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1
   };

typedef enum
   {
   MOUSEENABLE,
	JOYENABLE,
	USEPORT2,
	PADENABLE,
	SPACEBALLENABLE,
	CYBERMANENABLE,
	THRESSENS,
	MOUSESENS,
	CUSTOMIZE
   } controltypes;


static char *playerwadname[] =
   {
   "cass1", "bars1", "wens1", "lnis1", "ipfs1"
   };

char *colorname[] =
   {
   "Gray", "Brown", "Black", "Tan", "Red", "Olive",
   "Blue", "White", "Green", "Purple", "Orange"
   };

//
// MENU ITEMS
//
CP_MenuNames MainMenuNames[] =
   {
   "NEW GAME",
   "COMM-BAT� GAME",
   "RESTORE GAME",
   "SAVE GAME",
   "OPTIONS",
   "ORDERING INFO",
   "VIEW SCORES", //"END GAME"
   "BACK TO DEMO", //"BACK TO GAME"
   "QUIT"
   };

CP_iteminfo MainItems  = { MENU_X, MENU_Y + 1, 9, STARTITEM, 32, MainMenuNames, mn_largefont };
CP_itemtype MainMenu[] =
   {
      { CP_CursorLocation, "mm_opt1\0",  'N', (menuptr)CP_NewGame },
      { CP_Active,         "battle\0",   'C', (menuptr)CP_BattleModes },
      { CP_Active,         "mm_opt2\0",  'R', (menuptr)CP_LoadGame },
      { CP_Inactive,       "mm_opt3\0",  'S', (menuptr)CP_SaveGame },
      { CP_Active,         "mm_opt5\0",  'O', (menuptr)CP_ControlMenu },
      { CP_Active,         "ordrinfo\0", 'O', (menuptr)CP_OrderInfo },
      { CP_Active,         "mm_opt7\0",  'V', (menuptr)CP_ViewScores },
      { CP_Active,         "mm_opt8\0",  'B', (menuptr)NULL },
      { CP_Active,         "mm_opt9\0",  'Q', (menuptr)CP_Quit }
   };


CP_iteminfo LSItems  = { LSM_X, LSM_Y, NUMSAVEGAMES, 0, 10, NULL, mn_largefont };
CP_itemtype LSMenu[] =
   {
	   { CP_Active, "", 'a', NULL },
	   { CP_Active, "", 'b', NULL },
	   { CP_Active, "", 'c', NULL },
	   { CP_Active, "", 'd', NULL },
	   { CP_Active, "", 'e', NULL },
	   { CP_Active, "", 'f', NULL },
	   { CP_Active, "", 'g', NULL },
	   { CP_Active, "", 'h', NULL },
      { CP_Active, "", 'i', NULL },
      { CP_Active, "", 'j', NULL },
      { CP_Active, "", 'k', NULL },
      { CP_Active, "", 'l', NULL },
      { CP_Active, "", 'm', NULL },
      { CP_Active, "", 'n', NULL },
	   { CP_Active, "", 'o', NULL }
   };

CP_MenuNames CtlMenuNames[] =
   {
   "MOUSE ENABLED",
   "JOYSTICK ENABLED",
   "USE JOYSTICK PORT 2",
   "GAMEPAD ENABLED",
   "SPACEBALL ENABLED",
   "CYBERMAN ENABLED",
   "ADJUST THRESHOLD",
   "MOUSE SENSITIVITY",
   "CUSTOMIZE CONTROLS"
   };

CP_iteminfo CtlItems  = { CTL_X, MENU_Y, 9, -1, 36, CtlMenuNames, mn_largefont };
CP_itemtype CtlMenu[] =
   {
      { CP_Inactive, "ctl_mic\0", 'M', NULL },
      { CP_Inactive, "ctl_jen\0", 'J', NULL },
      { CP_Inactive, "ctl_jp2\0", 'U', NULL },
      { CP_Inactive, "ctl_gpd\0", 'G', NULL },
      { CP_Inactive, "spball\0",  'S', NULL },
      { CP_Inactive, "cyberman\0",'C', NULL },
      { CP_Inactive, "ctl_thr\0", 'A', (menuptr)DoThreshold },
      { CP_Inactive, "ctl_mse\0", 'M', (menuptr)MouseSensitivity },
      { CP_Active,   "ctl_cus\0", 'C', (menuptr)CP_Custom }
   };

CP_iteminfo CusItems  = { 32, CST_Y + 13 * 2, 9, -1, 0, NULL, mn_largefont };
CP_itemtype CusMenu[] =
   {
      { CP_Active,   "ctl_mic\0", 'a', NULL },
      { CP_Inactive, "ctl_mic\0", 'a', NULL },
      { CP_Inactive, "ctl_mic\0", 'a', NULL },
      { CP_Active,   "ctl_mic\0", 'a', NULL },
      { CP_Inactive, "ctl_mic\0", 'a', NULL },
      { CP_Inactive, "ctl_mic\0", 'a', NULL },
      { CP_Active,   "ctl_mic\0", 'a', NULL },
      { CP_Inactive, "ctl_mic\0", 'a', NULL },
      { CP_Active,   "ctl_mic\0", 'a', NULL }
   };


CP_iteminfo TufItems = { TUF_X, TUF_Y, 7, 0, 80, NULL, mn_largefont };
CP_itemtype TufMenu[ 4 ][ 7 ] =
   {
      {
         { 2, "new11\0", 'a', NULL },
         { 3, "new11\0", 'a', NULL },
         { 1, "new12\0", 'a', NULL },
         { 3, "new12\0", 'a', NULL },
         { 1, "new13\0", 'a', NULL },
         { 3, "new13\0", 'a', NULL },
         { 1, "new14\0", 'a', NULL },
      },

      {
         { 2, "new21\0", 'a', NULL },
         { 3, "new21\0", 'a', NULL },
         { 1, "new22\0", 'a', NULL },
         { 3, "new22\0", 'a', NULL },
         { 1, "new23\0", 'a', NULL },
         { 3, "new23\0", 'a', NULL },
         { 1, "new24\0", 'a', NULL },
      },

      {
         { 2, "new31\0", 'a', NULL },
         { 3, "new31\0", 'a', NULL },
         { 1, "new32\0", 'a', NULL },
         { 3, "new32\0", 'a', NULL },
         { 1, "new33\0", 'a', NULL },
         { 3, "new33\0", 'a', NULL },
         { 1, "new34\0", 'a', NULL },
      },

      {
         { 2, "stk_1\0", 'a', NULL },
         { 3, "stk_1\0", 'a', NULL },
         { 1, "stk_2\0", 'a', NULL },
         { 3, "stk_2\0", 'a', NULL },
         { 1, "stk_3\0", 'a', NULL },
         { 3, "stk_3\0", 'a', NULL },
         { 1, "stk_4\0", 'a', NULL },
      }
   };

CP_MenuNames CustomMenuNames[] =
   {
   "CUSTOMIZE KEYBOARD",
   "CUSTOMIZE MOUSE",
   "CUSTOMIZE JOYSTICK"
   };

CP_iteminfo CustomItems = {32, 64, 3, 0, 24, CustomMenuNames, mn_largefont };

CP_itemtype CustomMenu[] =
{
  {2, "custom1\0", 'C', (menuptr)CP_Keyboard},
  {1, "custom2\0", 'C', (menuptr)CP_Mouse},
  {1, "custom3\0", 'C', (menuptr)CP_Joystick}
};

#define KEYNAMEINDEX 21

CP_MenuNames NormalKeyNames[] =
   {
   "LEFT               \x9      ",
   "RIGHT              \x9      ",
   "FORWARD            \x9      ",
   "BACKWARD           \x9      ",
   "RUN                \x9      ",
   "OPEN               \x9      ",
   "FIRE               \x9      ",
   "STRAFE             \x9      ",
   "STRAFE LEFT        \x9      ",
   "STRAFE RIGHT       \x9      ",
   "LOOK/FLY UP        \x9      ",
   "LOOK/FLY DOWN      \x9      ",
   "AIM                \x9      ",
   "AIM UP             \x9      ",
   "AIM DOWN           \x9      ",
   "TOGGLE WEAPON      \x9      ",
   "DROP WEAPON        \x9      ",
   "VOLTE-FACE         \x9      ",
   "AUTORUN            \x9      ",
   "SEND MESSAGE       \x9      ",
   "DIRECT MESSAGE     \x9      "
   };

#define NORMALKEY_X  74
#define NORMALKEY_Y  16
CP_iteminfo NormalKeyItems = { NORMALKEY_X, 17, 21, 0, 16, NormalKeyNames, mn_tinyfont };

CP_itemtype NormalKeyMenu[] =
   {
      { 2, "\0", 'L', (menuptr)DefineKey },
      { 1, "\0", 'R', (menuptr)DefineKey },
      { 1, "\0", 'F', (menuptr)DefineKey },
      { 1, "\0", 'B', (menuptr)DefineKey },
      { 1, "\0", 'R', (menuptr)DefineKey },
      { 1, "\0", 'O', (menuptr)DefineKey },
      { 1, "\0", 'F', (menuptr)DefineKey },
      { 1, "\0", 'S', (menuptr)DefineKey },
      { 1, "\0", 'S', (menuptr)DefineKey },
      { 1, "\0", 'S', (menuptr)DefineKey },
      { 1, "\0", 'L', (menuptr)DefineKey },
      { 1, "\0", 'L', (menuptr)DefineKey },
      { 1, "\0", 'A', (menuptr)DefineKey },
      { 1, "\0", 'A', (menuptr)DefineKey },
      { 1, "\0", 'T', (menuptr)DefineKey },
      { 1, "\0", 'D', (menuptr)DefineKey },
      { 1, "\0", 'V', (menuptr)DefineKey },
      { 1, "\0", 'A', (menuptr)DefineKey },
      { 1, "\0", 'A', (menuptr)DefineKey },
      { 1, "\0", 'S', (menuptr)DefineKey },
      { 1, "\0", 'D', (menuptr)DefineKey }
   };

#define NUMCONTROLNAMES 21

CP_MenuNames ControlNames[] =
   {
   "NONE",
   "LEFT",
   "RIGHT",
   "FORWARD",
   "BACKWARD",
   "RUN",
   "OPEN",
   "FIRE",
   "STRAFE",
   "STRAFE LEFT",
   "STRAFE RIGHT",
   "LOOK/FLY UP",
   "LOOK/FLY DOWN",
   "AIM",
   "AIM UP",
   "AIM DOWN",
   "TOGGLE WEAPON",
   "DROP WEAPON",
   "VOLTE-FACE",
   "AUTORUN",
   "MAP"
   };

int controlorder[ NUMCONTROLNAMES ] = {
   bt_nobutton, di_west, di_east, di_north, di_south, bt_run, bt_use,
   bt_attack, bt_strafe, bt_strafeleft, bt_straferight, bt_lookup,
   bt_lookdown, bt_aimbutton, bt_horizonup, bt_horizondown,
   bt_swapweapon, bt_dropweapon, bt_turnaround, bt_autorun, bt_map
   };

#define CONTROLSELECT_X  106
CP_iteminfo ControlSelectItems = { CONTROLSELECT_X, 17, NUMCONTROLNAMES, 0, 16, ControlNames, mn_tinyfont };

CP_itemtype ControlSelectMenu[] =
   {
      { 2, "\0", 'N', NULL },
      { 1, "\0", 'L', NULL },
      { 1, "\0", 'R', NULL },
      { 1, "\0", 'F', NULL },
      { 1, "\0", 'B', NULL },
      { 1, "\0", 'R', NULL },
      { 1, "\0", 'O', NULL },
      { 1, "\0", 'F', NULL },
      { 1, "\0", 'S', NULL },
      { 1, "\0", 'S', NULL },
      { 1, "\0", 'S', NULL },
      { 1, "\0", 'L', NULL },
      { 1, "\0", 'L', NULL },
      { 1, "\0", 'A', NULL },
      { 1, "\0", 'A', NULL },
      { 1, "\0", 'A', NULL },
      { 1, "\0", 'T', NULL },
      { 1, "\0", 'D', NULL },
      { 1, "\0", 'V', NULL },
      { 1, "\0", 'A', NULL },
      { 1, "\0", 'M', NULL }
   };

#define MOUSEBTNINDEX 17

CP_MenuNames MouseBtnNames[] =
   {
   "             B0  \x9             ",
   "             B1  \x9             ",
   "             B2  \x9             ",
   "DOUBLE-CLICK B0  \x9             ",
   "DOUBLE-CLICK B1  \x9             ",
   "DOUBLE-CLICK B2  \x9             "
   };

CP_iteminfo MouseBtnItems = { 19, 52, 6, 0, 11, MouseBtnNames, mn_8x8font };

CP_itemtype MouseBtnMenu[] =
   {
      { 2, "\0", 'B', (menuptr)DefineMouseBtn },
      { 1, "\0", 'B', (menuptr)DefineMouseBtn },
      { 1, "\0", 'B', (menuptr)DefineMouseBtn },
      { 1, "\0", 'D', (menuptr)DefineMouseBtn },
      { 1, "\0", 'D', (menuptr)DefineMouseBtn },
      { 1, "\0", 'D', (menuptr)DefineMouseBtn }
   };


#define JOYBTNINDEX 17

CP_MenuNames JoyBtnNames[] =
   {
   "             B0  \x9             ",
   "             B1  \x9             ",
   "             B2  \x9             ",
   "             B3  \x9             ",
   "DOUBLE-CLICK B0  \x9             ",
   "DOUBLE-CLICK B1  \x9             ",
   "DOUBLE-CLICK B2  \x9             ",
   "DOUBLE-CLICK B3  \x9             "
   };

CP_iteminfo JoyBtnItems = { 19, 48, 8, 0, 11, JoyBtnNames, mn_8x8font };

CP_itemtype JoyBtnMenu[] =
   {
      { 2, "\0", 'B', (menuptr)DefineJoyBtn },
      { 1, "\0", 'B', (menuptr)DefineJoyBtn },
      { 1, "\0", 'B', (menuptr)DefineJoyBtn },
      { 1, "\0", 'B', (menuptr)DefineJoyBtn },
      { 1, "\0", 'D', (menuptr)DefineJoyBtn },
      { 1, "\0", 'D', (menuptr)DefineJoyBtn },
      { 1, "\0", 'D', (menuptr)DefineJoyBtn },
      { 1, "\0", 'D', (menuptr)DefineJoyBtn }
   };

CP_MenuNames PlayerMenuNames[] =
   {
   "TARADINO CASSATT",
   "THI BARRETT",
   "DOUG WENDT",
   "LORELEI NI",
   "IAN PAUL FREELEY"
   };

CP_iteminfo PlayerItems = {TUF_X, 48, 5, 0, 80, PlayerMenuNames, mn_largefont };

CP_itemtype PlayerMenu[] =
{
	{2, "name1\0", 'T', NULL},
   {1, "name2\0", 'T', NULL},
   {1, "name3\0", 'D', NULL},
   {1, "name4\0", 'L', NULL},
   {1, "name5\0", 'I', NULL},
};

CP_MenuNames ControlMMenuNames[] =
   {
   "CONTROLS",
   "USER OPTIONS",
   "EXT USER OPTIONS",//bna added
   "MUSIC VOLUME",
   "SOUND FX VOLUME"

   };
CP_iteminfo ControlMItems = {32, 48-8, 5, 0, 32, ControlMMenuNames, mn_largefont };//bna added
//CP_iteminfo ControlMItems = {32, 48, 4, 0, 32, ControlMMenuNames, mn_largefont };

CP_itemtype ControlMMenu[] =
{
   {2, "cntl\0",     'C', (menuptr)CP_Control},
   {1, "uopt\0",     'U', (menuptr)CP_OptionsMenu},
   {1, "euopt\0", 'E', (menuptr)CP_ExtOptionsMenu},//bna added
   {1, "muvolumn\0", 'M', (menuptr)MusicVolume},
   {1, "fxvolumn\0", 'S', (menuptr)FXVolume}

};

CP_MenuNames OptionsNames[] =
   {
   "AUTO DETAIL ADJUST",
   "LIGHT DIMINISHING",
   "BOBBIN'",
   "FLOOR AND CEILING",
   "DOUBLE-CLICK SPEED",
   "MENU FLIP SPEED",
   "DETAIL LEVELS",
   "VIOLENCE LEVEL",
   "SCREEN SIZE"
   };
//bna added
CP_MenuNames ExtOptionsNames[] =
   {
   "MOUSELOOK",
   "INVERSE MOUSE",
   "CROSS HAIR",
   "JUMPING",
   "FULLSCREEN"
   };
CP_iteminfo ExtOptionsItems = { 20, MENU_Y, 5, 0, 43, ExtOptionsNames, mn_largefont };

CP_itemtype ExtOptionsMenu[] =
{
   {1, "", 'M', NULL},
   {1, "", 'I', NULL},
   {1, "", 'C', NULL},
   {1, "", 'J', NULL},
   {1, "", 'F', NULL}
};
   
//bna added end

CP_iteminfo OptionsItems = { 20, MENU_Y, 9, 0, 43, OptionsNames, mn_largefont };

CP_itemtype OptionsMenu[] =
{
   {2, "autoadj\0", 'A', NULL},
   {1, "lightdim\0",'L', NULL},
   {1, "bobbin\0",  'B', NULL},
   {1, "fandc\0",   'F', NULL},
   {1, "double\0",  'D', (menuptr)CP_DoubleClickSpeed},
   {1, "menuspd\0", 'M', (menuptr)MenuFlipSpeed},
   {1, "detail\0",  'D', (menuptr)CP_DetailMenu},
   {1, "vlevel\0",  'V', (menuptr)CP_ViolenceMenu},
   {1, "\0",        'S', (menuptr)CP_ScreenSize}
};

CP_MenuNames DetailMenuNames[] =
   {
   "LOW DETAIL",
   "MEDIUM DETAIL",
   "HIGH DETAIL"
   };

CP_iteminfo DetailItems = { 32, 64, 3, 0, 43, DetailMenuNames, mn_largefont };

CP_itemtype DetailMenu[] =
{
   {2, "lowdtl\0", 'L', NULL},
   {1, "meddtl\0", 'M', NULL},
   {1, "hidtl\0",  'H', NULL}
};

CP_MenuNames BattleMenuNames[] =
   {
   "PLAY GAME",
   "PLAY TEAM GAME",
   "COMM-BAT OPTIONS"
   };

CP_iteminfo BattleItems = { 32, 19, 3, 0, 24, BattleMenuNames, mn_largefont };

CP_itemtype BattleMenu[] =
{
   {2, "bplay\0",    'P', (menuptr)BattleNoTeams},
   {1, "playteam\0", 'P', (menuptr)BattleTeams},
   {1, "comopt\0",   'C', (menuptr)CP_BattleOptions}
};

CP_MenuNames ViolenceMenuNames[] =
   {
   "NONE",
   "SOME",
   "A LOT",
   "EXCESSIVE"
   };

CP_iteminfo ViolenceItems = { 32, 64, 4, 0, 45, ViolenceMenuNames, mn_largefont };

CP_itemtype ViolenceMenu[] =
{
   {2, "vnone\0",   'N', NULL},
   {1, "vsome\0",   'S', NULL},
   {1, "valot\0",   'A', NULL},
   {1, "vexcess\0", 'E', NULL}
};

CP_MenuNames VMenuNames[] =
   {
   "SET VIOLENCE LEVEL",
   "" // "ENTER PASSWORD" // "CHANGE PASSWORD"
   };

CP_iteminfo VItems = { 32, MP_Y, 2, 0, 24, VMenuNames, mn_largefont };

CP_itemtype VMenu[] =
{
   {2, "msetv\0",  'S',  (menuptr)CP_ViolenceLevel},
   {1, "mepass\0", 'E', (menuptr)CP_PWMenu}
};

CP_MenuNames ModeMenuNames[] =
   {
   "NORMAL",
   "SCORE MORE",
   "COLLECTOR",
   "SCAVENGER",
   "HUNTER",
   "TAG",
   "ELUDER",
   "DELUDER",
   "CAPTURE THE TRIAD"
   };

CP_iteminfo ModeItems = { MENU_X, MENU_Y + 1, 9, 0, 24, ModeMenuNames, mn_largefont };

CP_itemtype ModeMenu[] =
{
   {CP_CursorLocation, "normal\0",   'N', (menuptr)CP_BattleMenu},
   {CP_Active,         "scorem\0",   'S', (menuptr)CP_BattleMenu},
   {CP_Active,         "collect\0",  'C', (menuptr)CP_BattleMenu},
   {CP_Active,         "scaven\0",   'S', (menuptr)CP_BattleMenu},
   {CP_Active,         "hunter\0",   'H', (menuptr)CP_BattleMenu},
   {CP_Active,         "tag\0",      'T', (menuptr)CP_BattleMenu},
   {CP_Active,         "eluder\0",   'E', (menuptr)CP_BattleMenu},
   {CP_Active,         "deluder\0",  'D', (menuptr)CP_BattleMenu},
   {CP_Active,         "captriad\0", 'C', (menuptr)CP_BattleMenu}
};

CP_MenuNames BOptNames[] =
   {
   "GRAVITY",
   "SPEED",
   "AMMO PER WEAPON",
   "HIT POINTS",
   "RADICAL OPTIONS",
   "LIGHT LEVELS",
   "POINT GOAL",
   "DANGER DAMAGE",
   "TIME LIMIT"
   };

CP_iteminfo BOptItems = { MENU_X, MENU_Y + 1, 9, 0, 24, BOptNames, mn_largefont };

CP_itemtype BOptMenu[] =
{
   {2, "gravity\0",  'G', (menuptr)CP_GravityOptions},
   {1, "speed\0",    'S', (menuptr)CP_SpeedOptions},
   {1, "ammoper\0",  'A', (menuptr)CP_AmmoPerWeaponOptions},
   {1, "hitp\0",     'H', (menuptr)CP_HitPointsOptions},
   {1, "radical\0",  'R', (menuptr)CP_SpawnControlOptions},
   {1, "lightl\0",   'L', (menuptr)CP_LightLevelOptions},
   {1, "pntgoal\0",  'P', (menuptr)CP_PointGoalOptions},
   {1, "danger\0",   'D', (menuptr)CP_DangerOptions},
   {1, "timel\0",    'T', (menuptr)CP_TimeLimitOptions}
};

CP_MenuNames GravityMenuNames[] =
   {
   "LOW",
   "NORMAL",
   "HIGH"
   };

CP_iteminfo GravityItems = { 32, 26, 3, 0, 45, GravityMenuNames, mn_largefont };

CP_itemtype GravityMenu[] =
{
	{2, "b_low\0",    'L', NULL},
	{1, "b_normal\0", 'N', NULL},
	{1, "b_high\0",   'H', NULL}
};

CP_MenuNames SpeedMenuNames[] =
   {
   "NORMAL",
   "FAST"
   };

CP_iteminfo SpeedItems = { 32, MP_Y, 2, 0, 45, SpeedMenuNames, mn_largefont };

CP_itemtype SpeedMenu[] =
{
   {2, "b_normal\0", 'N', NULL},
   {1, "b_fast\0",   'F', NULL}
};

CP_MenuNames AmmoPerWeaponMenuNames[] =
   {
   "ONE",
   "NORMAL",
   "GUNFINITY"
   };

CP_iteminfo AmmoPerWeaponItems = { 32, 26, 3, 0, 45, AmmoPerWeaponMenuNames, mn_largefont };

CP_itemtype AmmoPerWeaponMenu[] =
{
   {2, "b_one\0",    'O', NULL},
   {1, "b_normal\0", 'N', NULL},
   {1, "b_gunf\0",   'G', NULL}
};

CP_MenuNames HitPointMenuNames[] =
   {
   "ONE",
   "25",
   "BY CHARACTER",
   "100",
   "250",
   "500",
   "4000"
   };

CP_iteminfo HitPointItems = { 32, 32, 7, 0, 45, HitPointMenuNames, mn_largefont };

CP_itemtype HitPointMenu[] =
{
   {2, "b_one\0",  'O', NULL},
   {1, "b_25\0",   'a', NULL},
   {1, "b_char\0", 'C', NULL},
   {1, "b_100\0",  'a', NULL},
   {1, "b_250\0",  'a', NULL},
	{1, "b_500\0",  'a', NULL},
   {1, "b_4000\0", 'a', NULL}
};

CP_MenuNames SpawnMenuNames[] =
   {
   "SPAWN DANGERS",
   "SPAWN HEALTH",
   "SPAWN WEAPONS",
   "SPAWN MINES",
   "RESPAWN ITEMS",
   "WEAPON PERSISTENCE",
   "RANDOM WEAPONS",
   "FRIENDLY FIRE"
   };

CP_iteminfo SpawnItems = { 20, 24, 8, 0, 35, SpawnMenuNames, mn_largefont };

CP_itemtype SpawnMenu[] =
{
   {2, "b_danger\0", 'S', NULL},
   {1, "b_health\0", 'S', NULL},
   {1, "b_weap\0",   'S', NULL},
   {1, "b_mines\0",  'S', NULL},
   {1, "b_rpawn\0",  'R', NULL},
   {1, "b_persis\0", 'W', NULL},
   {1, "b_rndwpn\0", 'R', NULL},
   {1, "b_friend\0", 'F', NULL}
};

CP_MenuNames LightLevelMenuNames[] =
   {
   "DARK",
   "NORMAL",
   "BRIGHT",
   "FOG",
   "PERIODIC",
   "LIGHTNING"
   };

CP_iteminfo LightLevelItems = { 32, 40, 6, 0, 45, LightLevelMenuNames, mn_largefont };

CP_itemtype LightLevelMenu[] =
{
   {2, "b_dark\0",   'D', NULL},
   {1, "b_normal\0", 'N', NULL},
   {1, "b_bright\0", 'B', NULL},
   {1, "b_fog\0",    'F', NULL},
   {1, "b_period\0", 'P', NULL},
   {1, "b_light\0",  'L', NULL}
};

CP_MenuNames PointGoalMenuNames[] =
   {
   "1",
   "5",
   "11",
   "21",
   "50",
   "100",
   "RANDOM",
   "RANDOM BLIND",
   "INFINITE"
   };

CP_iteminfo PointGoalItems = { 32, 16, 9, 0, 45, PointGoalMenuNames, mn_largefont };

CP_itemtype PointGoalMenu[] =
{
	{2, "b_1\0",      'a', NULL},
   {1, "b_5\0",      'a', NULL},
   {1, "b_11\0",     'a', NULL},
   {1, "b_21\0",     'a', NULL},
   {1, "b_50\0",     'a', NULL},
   {1, "b_100\0",    'a', NULL},
   {1, "b_random\0", 'R', NULL},
	{1, "b_randb\0",  'R', NULL},
	{1, "b_inf\0",    'I', NULL}
};

CP_MenuNames DangerMenuNames[] =
   {
   "LOW",
   "NORMAL",
   "KILL"
   };

CP_iteminfo DangerItems = { 32, 56, 3, 0, 45, DangerMenuNames, mn_largefont };

CP_itemtype DangerMenu[] =
{
   {2, "b_low\0",    'L', NULL},
   {1, "b_normal\0", 'N', NULL},
	{1, "b_kill\0",   'K', NULL}
};

CP_MenuNames TimeLimitMenuNames[] =
   {
   "1",
   "2",
   "5",
   "10",
   "21",
   "30",
   "99",
   "NONE"
   };

CP_iteminfo TimeLimitItems = { 32, 24, 8, 0, 45, TimeLimitMenuNames, mn_largefont };

CP_itemtype TimeLimitMenu[] =
{
	{2, "b_1\0",    'a', NULL},
	{1, "b_2\0",    'a', NULL},
	{1, "b_5\0",    'a', NULL},
	{1, "b_10\0",   'a', NULL},
   {1, "b_21\0",   'a', NULL},
	{1, "b_30\0",   'a', NULL},
	{1, "b_99\0",   'a', NULL},
   {1, "vnone\0",  'N', NULL}
};

CP_MenuNames MultiPageCustomNames[ MAXCUSTOM + 2 ] =
   {
   "NEXT PAGE",
   "PREVIOUS PAGE"
   };

CP_iteminfo MultiPageCustomItems = { 18, 17, 0, 0, 12, MultiPageCustomNames, mn_smallfont };
CP_itemtype MultiPageCustomMenu[] =
{
	{1, "", 'N', NULL},
   {1, "", 'P', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
   {1, "", 'a', NULL},
};

#define COLORX 113
#define COLORY 43
#define COLORW 60
#define COLORH 96

// Custom menu stuff
static int CUSTOM_y[ 7 ] = { 31, 0, 63, 0, 94, 0, 126 };

//
// Save globals
//
static boolean StartGame = false;

static int  SaveGamesAvail[ NUMSAVEGAMES ];
static char SaveGameNames[ NUMSAVEGAMES ][ 32 ];
static char SaveName[ 13 ] = "rottgam?.rot\0";

static byte *savedscreen;
static mapfileinfo_t * mapinfo;

static void HideCursor
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int x,
   int y,
   int which
   );
static void ShowCursor
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int x,
   int *y,
   int which,
   int basey
   );
void CP_DrawSelectedGame (int w);
int HandleMenu (CP_iteminfo *item_i, CP_itemtype *items, void (*routine)(int w));
void DrawStoredGame ( byte * pic, int episode, int area );
void DrawCustomKeyboard (void);
void DrawBattleModeName( int which );
void DrawBattleModeDescription( int w );
void DrawSoundSetupMainMenu( void );
int ColorMenu(void);

//******************************************************************************
//
// MN_DrawButtons
//
//******************************************************************************

void MN_DrawButtons
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int check,
   int *nums
   )

   {
   int i;
   int button_on;
   int button_off;

   button_on  = W_GetNumForName( "snd_on" );
   button_off = W_GetNumForName( "snd_off" );

   for( i = 0; i < item_i->amount; i++ )
      {
      if ( items[ i ].active != CP_Active3 )
         {
         if ( nums[ i ] == check )
            {
            DrawMenuBufItem( item_i->x + 27, item_i->y + i *
               FontSize[ item_i->fontsize ] - 1, button_on);
            }
         else
            {
            DrawMenuBufItem( item_i->x + 27, item_i->y + i *
               FontSize[ item_i->fontsize ] - 1, button_off);
            }
         }
      }
   }


//****************************************************************************
//
// MN_GetCursorLocation()
//
//****************************************************************************

void MN_GetCursorLocation
   (
   CP_iteminfo *item_i,
   CP_itemtype *items
   )

   {
   int i;
   int position;

   position = -1;
   for( i = 0; i < item_i->amount; i++ )
      {
      if ( items[ i ].active == CP_CursorLocation )
         {
         position = i;
         break;
         }

      if ( ( items[ i ].active == CP_Active ) && ( position == -1 ) )
         {
         position = i;
         }
      }

   if ( position != -1 )
      {
      item_i->curpos = position;
      items[ position ].active = CP_CursorLocation;
      }
   }


//****************************************************************************
//
// MN_GetActive ()
//
//****************************************************************************

int MN_GetActive
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int check,
   int *nums
   )

   {
   int i;
   int returnval;

   returnval = 0;
   for( i = 0; i < item_i->amount; i++ )
      {
      items[ i ].active = CP_Active;
      if ( nums[ i ] == check )
         {
         item_i->curpos    = i;
         items[ i ].active = CP_CursorLocation;
         returnval = i;
         }
      }

   return( returnval );
   }


//****************************************************************************
//
// MN_MakeActive ()
//
//****************************************************************************

void MN_MakeActive
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int which
   )

   {
   int i;

   for( i = 0; i < item_i->amount; i++ )
      if (i == which)
      {
         items[i].active = CP_CursorLocation;
         item_i->curpos    = i;
      }
      else
         items[i].active = CP_Active;
}


//******************************************************************************
//
// DrawMenu ()
//
// Purpose - Draws a menu
//
//******************************************************************************
void DrawMenu
   (
   CP_iteminfo *item_i,
   CP_itemtype *items
   )

   {
   int i;
   int active;
   int color;
   int posx;
   int posy;

   posx = item_i->x + item_i->indent;
   posy = item_i->y;
   WindowX = posx;
   WindowY = posy;
   WindowW = 320;
   WindowH = 200;

   for ( i = 0; i < item_i->amount; i++ )
      {
      posy   = item_i->y + i * FontSize[ item_i->fontsize ];
      active = items[ i ].active;

      color = -1;
      switch( active )
         {
         case CP_CursorLocation :
            color = ACTIVECOLOR;
            break;

         case CP_Inactive :
            color = NOTAVAILABLECOLOR;
            break;

         case CP_Active :
            color = NORMALCOLOR;
            break;

         case CP_SemiActive :
            color = DIMMEDCOLOR;
            break;

         case CP_Highlight :
            color = HIGHLIGHTCOLOR;
            break;
         }

      if ( color != -1 )
         {
         if ( item_i->names == NULL )
            {
            DrawIMenuBufItem( posx, posy, W_GetNumForName( items[ i ].texture ),
               color );
            }
         else
            {
            IFont = ( cfont_t * )W_CacheLumpName( FontNames[ item_i->fontsize ],
               PU_CACHE, Cvt_cfont_t, 1 );
            if ( item_i->fontsize == mn_tinyfont )
               {
               DrawMenuBufIString( posx + 1, posy, item_i->names[ i ], 0 );
               }
            DrawMenuBufIString( posx, posy - 1, item_i->names[ i ], color );
            }
         }
      }
   }


//******************************************************************************
//
// getASCII () - Gets info from Keyboard.
//
//******************************************************************************

int getASCII ( void )
{
   int i;
   int LS;
   int RS;
   int returnvalue = 0;
   int scancode = 0;

#ifdef DOS
   _disable ();      // must disable for SHIFT purposes
#endif

   IN_UpdateKeyboard ();

   LS = Keyboard[sc_LShift];
   RS = Keyboard[sc_RShift];

   Keyboard[sc_LShift] = Keyboard[sc_RShift] = 0;

   scancode = 0;

   for (i = 0; i < 127; i++)
      if (Keyboard[i])
      {
         scancode = i;
         break;
      }

	if (scancode)
   {
      if (LS || RS)
         returnvalue = ShiftNames[scancode];
      else
         returnvalue = ASCIINames[scancode];
   }

   Keyboard[sc_LShift] = LS;
   Keyboard[sc_RShift] = RS;

#ifdef DOS
   _enable ();
#endif

   return (returnvalue);
}


//******************************************************************************
//
// ScanForSavedGames ()
//
//******************************************************************************

void ScanForSavedGames ()
{
   struct find_t f;
   char filename[256];
   char str[45];
   int which;
   boolean found = false;
#ifndef DOS
   char *pathsave;
#endif

   //
   // SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
   //
   memset (&SaveGamesAvail[0], 0, sizeof (SaveGamesAvail));
#if PLATFORM_DOS || PLATFORM_WIN32 || defined(RETRO_GO)
   GetPathFromEnvironment( filename, ApogeePath, SaveName );
#else
   strncpy (filename, SaveName, 256);
   pathsave = getcwd (NULL, 0);
   chdir (ApogeePath);
#endif

   printf("ScanForSavedGames: searching for %s\n", filename);

   if (!_dos_findfirst (filename, 0, &f))
   {
      do
      {
         printf("ScanForSavedGames: found %s\n", f.name);
         strcpy(str,&f.name[7]);
         sscanf((const char *)&str[0],"%x",&which);

         if (which < NUMSAVEGAMES)
         {
            found = true;
            SaveGamesAvail[which] = 1;
            GetSavedMessage (which, &SaveGameNames[which][0]);
         }

      } while (!_dos_findnext (&f));
   }

   if (found)
   {
      if (MainMenu[loadgame].active == CP_Inactive)
         MainMenu[loadgame].active = CP_Active;
   }
   else
      MainMenu[loadgame].active = CP_Inactive;

#if ((!PLATFORM_DOS) && (!PLATFORM_WIN32) && (!defined(RETRO_GO)))
   chdir (pathsave);
   free (pathsave);
#endif
}


//******************************************************************************
//
// SetUpControlPanel ()
//
//******************************************************************************

void SetUpControlPanel (void)
{
   int i;
   int j;
   byte * b;
   byte * s;

//   int Xres = 320;//org
//   int Yres = 200;//org
   int Xres = 640;
   int Yres = 400;

   //dont work in 800x600 until we get a better screen schrinker
  //  int Xres = iGLOBAL_SCREENWIDTH;//640;
 //  int Yres = iGLOBAL_SCREENHEIGHT;//400;

  Xres = 640;
   Yres = 400;



   // Save the current game screen

   //bna--savedscreen = SafeMalloc (16000);
   savedscreen = SafeMalloc (16000);

   // Copy the current save game screen (� size) to this buffer

   if (RefreshPause==false)
      {
      GamePaused=false;
      ThreeDRefresh();
      FlipPage();
      FlipPage();
      GamePaused=true;
      }


   s=savedscreen;


    
	  if (iGLOBAL_SCREENWIDTH == 320) { 
	  for (i=0;i<320;i+=2)	{	  
	  b=(byte *)bufferofs+i;
	  for (j=0;j<100;j++,s++,b+=(iGLOBAL_SCREENWIDTH<<1))
	  *s=*b;
	  }
	  }	  
	  else if (iGLOBAL_SCREENWIDTH >= 640) { 
	  for (i=0;i<640;i+=4)	{		  
	  b=(byte *)bufferofs+i;
	  for (j=0;j<100;j++,s++,b+=(iGLOBAL_SCREENWIDTH<<2))
	  *s=*b;
	  }
	  }
/*
      if (iGLOBAL_SCREENWIDTH == 800) { 	 
		  for (i=0;i<Xres;i+=8)		{	  
			  b=(byte *)bufferofs+i;//schrink screen to 1/3 size
			  for (j=0;j<(Yres/8);j++,s++,b+=(iGLOBAL_SCREENWIDTH<<1)*3)
				 *s=*b;
		  }

      }*/

   ScanForSavedGames ();



   if (modemgame == true)
      {
      // Make battle mode active
      //
      MainMenu[battlemode].active = CP_Active;

      // No save or load game in modem game
      //
      MainMenu[newgame].active    = CP_Inactive;
      MainMenu[backtodemo].active = CP_Inactive;
      MainMenu[loadgame].active   = CP_Inactive;
      MainMenu[savegame].active   = CP_Inactive;

      if ( MainMenu[ MainItems.curpos ].active == CP_Inactive )
         {
         MainItems.curpos = battlemode;
         }

      MainMenu[MainItems.curpos].active = CP_CursorLocation;

      if ( consoleplayer != 0 )
         {
         MainMenu[battlemode].routine = ( void (*)(int) )BattleGamePlayerSetup;
         }
      }
   }

//******************************************************************************
//
// GetMenuInfo ()
//
// Gets the user's password
//
//******************************************************************************

void GetMenuInfo (void)
{
   ConvertPasswordStringToPassword ();

   POK=true;
   if (pword[0]==0)
      POK=false;
}


//******************************************************************************
//
// WriteMenuInfo ()
//
// Writes out password
//
//******************************************************************************

void WriteMenuInfo (void)
{
   ConvertPasswordToPasswordString ();
}

//******************************************************************************
//
// AllocateSavedScreenPtr ()
//
//******************************************************************************

void AllocateSavedScreenPtr (void)
{
   // Save the current game screen

   savedscreen = SafeMalloc(16000);
   inmenu  = true;
   numdone = 0;
}


//******************************************************************************
//
// FreeSavedScreenPtr ()
//
//******************************************************************************

void FreeSavedScreenPtr (void)
{
   SafeFree (savedscreen);
   inmenu  = false;
}


//******************************************************************************
//
// CleanUpControlPanel ()
//
//******************************************************************************

void CleanUpControlPanel (void)
{
	int   joyx, joyy;

   if ((playstate==ex_resetgame) || (loadedgame==true))
      ShutdownClientControls();

   // Free up saved screen image

   FreeSavedScreenPtr ();

   WriteConfig ();

   INL_GetJoyDelta (joystickport, &joyx, &joyy);

   if (mouseenabled)
      PollMouseMove ();    // Trying to kill movement

   if (cybermanenabled)
      PollCyberman ();

   RefreshPause = true;
}


//******************************************************************************
//
// CP_CheckQuick ()
//
//******************************************************************************
boolean CP_CheckQuick
   (
   byte scancode
   )

   {
   if (demoplayback==true)
      {
      switch ( scancode )
         {
         case sc_Escape:
            inmenu = true;
            return( true );
            break;
         }
      }
   else
      {
      switch ( scancode )
         {
         case sc_Escape:
         case sc_F1:
         case sc_F2:
         case sc_F3:
         case sc_F4:
         case sc_F8:
         case sc_F9:
         case sc_F10:
            inmenu = true;
            return( true );
            break;
         }
      }

   return( false );
   }


//******************************************************************************
//
// ControlPanel
//
//    ROTT Control Panel!
//
//******************************************************************************
void ControlPanel
   (
   byte scancode
   )

   {
   if ( scancode == sc_Escape )
      {
      CP_MainMenu();
      if ( ( playstate == ex_stillplaying ) && ( loadedgame == false ) )
         {
         fizzlein = true;
         }
      return;
      }

   SetupMenuBuf();

   numdone = 0;
   StartGame = false;
 
   SetUpControlPanel();
   EnableScreenStretch();
   //
   // F-KEYS FROM WITHIN GAME
   //
	switch( scancode )
      {
      case sc_F1:
         CP_F1Help();
         break;

      case sc_F2:
         CP_SaveGame();
         break;

      case sc_F3:
         CP_LoadGame( 0, 0 );
         break;

      case sc_F4:
         CP_ControlMenu();
         break;

      case sc_F8:
         LastScan          = 0;
         Keyboard[ sc_F8 ] = 0;
         CP_EndGame();
         break;

      case sc_F9:
         LastScan          = 0;
         Keyboard[ sc_F9 ] = 0;

         loadsavesound = true;
         CP_LoadGame( 1, 0 );
         break;

      case sc_F10:
         SetMenuTitle ("Quit");

         LastScan           = 0;
         Keyboard[ sc_F10 ] = 0;
         CP_Quit( -1 );
         break;

      }

   CleanUpControlPanel();
   ShutdownMenuBuf();

   if ( loadedgame == false )
      {
      SetupScreen( false );
      fizzlein = true;
      inmenu = false;
      }

   loadsavesound = false;
   }


//******************************************************************************
//
// CP_MainMenu
//
//******************************************************************************
menuitems CP_MainMenu
   (
   void
   )

   {
   int which;

   SetupMenuBuf();

   numdone = 0;

   SetUpControlPanel();

   DrawMainMenu();

   //
   // Main menu loop.  "Exit options" or "New game" exits
   //
   StartGame = false;
   EnableScreenStretch();

   while( !StartGame )
      {
      StartGame = false;

      IN_ClearKeysDown();

      which = HandleMenu( &MainItems, &MainMenu[ 0 ], NULL );

      switch( which )
         {
         case backtodemo:
            if ( !ingame )
               {
               playstate = ex_titles;
               }

            StartGame = true;
			DisableScreenStretch();//bna++ shut off streech mode
            break;

         case -1:
            CP_Quit( 0 );
            break;

         default:
            if ( !StartGame )
               {
               DoMainMenu();
               }
         }
      }

   // Deallocate everything
   CleanUpControlPanel();
   ShutdownMenuBuf();

   return( which );
   }


//******************************************************************************
//
// DrawMainMenu ()
//
//******************************************************************************

void DrawMainMenu(void)
{    

   MenuNum = 1;
		EnableScreenStretch();//bna++ shut off streech mode
   //
   // CHANGE "GAME" AND "DEMO"
   //
   if ( ingame )
      {
      MainMenu[ backtodemo ].texture[ 6 ] = '1';
      MainMenu[ backtodemo ].texture[ 7 ] = '1';
      MainMenu[ backtodemo ].texture[ 8 ] = '\0';
      strcpy (MainMenuNames[ backtodemo ] , "BACK TO GAME");
      }
   else
      {
      MainMenu[ backtodemo ].texture[ 6 ] = '8';
      MainMenu[ backtodemo ].texture[ 7 ] = '\0';
      strcpy (MainMenuNames[ backtodemo ] , "BACK TO DEMO");
      }

   MN_GetCursorLocation( &MainItems, &MainMenu[ 0 ] );
   SetMenuTitle ("Main Menu");
   DrawMenu (&MainItems, &MainMenu[0]);

	numdone ++;
   DisplayInfo (0);
}


//******************************************************************************
//
// Handle moving triad around a menu
//
//******************************************************************************

int HandleMenu (CP_iteminfo *item_i, CP_itemtype *items, void (*routine)(int w))
{

   char        key;
   int         i,
               x,
               y,
               basey,
               exit,
               numactive,
               count;
   int         newpos;
   volatile int timer;
   ControlInfo ci;
   boolean     playsnd = false;

   handlewhich = item_i->curpos;
   x     = item_i->x;
   if ((MenuNum == 4) || (MenuNum == 6) ||
      ( item_i->fontsize == mn_smallfont ) )
      {
      basey = item_i->y;

      CursorLump = SmallCursor;
      yinc = 9;
      cursorwidth = cursorheight = 8;
      }
   else if ( item_i->fontsize == mn_8x8font )
      {
      basey = item_i->y - 1;

      CursorLump = SmallCursor;
		yinc = 7;
      cursorwidth = cursorheight = 8;
      }
   else if ( item_i->fontsize == mn_tinyfont )
      {
      basey = item_i->y - 2;

      CursorLump = SmallCursor;
		yinc = 6;
      cursorwidth = cursorheight = 8;
      }
   else
      {
      basey = item_i->y-2;

      CursorLump = LargeCursor;
		yinc = 14;
      cursorwidth = cursorheight = 16;
      }


   if (MenuNum)
      y = basey + handlewhich*yinc;
   else
      y = CUSTOM_y[handlewhich];


   if (MenuNum != 5)
      DrawMenuBufItem (x, y, W_GetNumForName( CursorLump ) +
         CursorFrame[ CursorNum ] );

   if (routine)
      routine (handlewhich);

   count    = 2;
   exit     = 0;
   timer    = GetTicCount();
   IN_ClearKeysDown ();

   numactive = GetNumActive (item_i, items);

   do
   {
      ReadAnyControl (&ci);
      RefreshMenuBuf (0);
     // Change Cursor Shape
      if ((GetTicCount() > (timer+count)) && (MenuNum != 5))
      {
         timer = GetTicCount();

         CursorNum++;
         if (CursorNum > (MAXCURSORNUM-1))
            CursorNum = 0;

         EraseMenuBufRegion(x, y, cursorwidth, cursorheight);
         DrawMenuBufItem (x, y, W_GetNumForName( CursorLump ) +
            CursorFrame[ CursorNum ] );

      }

     // Initial char - pass 1
      key = getASCII ();
      if (key)
      {
         int ok = 0;

         key = toupper (key);

         for (i = (handlewhich + 1); i < item_i->amount; i++)
            if ((items+i)->active && (items+i)->letter == key)
            {
               HideCursor (item_i, items, x, y, handlewhich);
               MN_PlayMenuSnd (SD_MOVECURSORSND);
               handlewhich = i;


               if (routine)
                  routine (handlewhich);

               ShowCursor (item_i, items, x, &y, handlewhich, basey);
               ok = 1;
               IN_ClearKeysDown();
               break;
            }

        // Initial char - pass 2
         if (!ok)
         {
            for (i = 0; i < handlewhich; i++)
               if ((items+i)->active && (items+i)->letter == key)
               {
                  HideCursor (item_i, items, x, y, handlewhich);
                  MN_PlayMenuSnd (SD_MOVECURSORSND);
                  handlewhich = i;


                  if (routine)
                     routine (handlewhich);

                  ShowCursor (item_i, items, x,& y, handlewhich, basey);
                  IN_ClearKeysDown ();
                  break;
               }
         }
      }

      ReadAnyControl (&ci);

      if (numactive > 1)
		{
         switch (ci.dir)
         {
            case dir_North:
               HideCursor (item_i, items, x, y, handlewhich);


               CursorNum++;
               if (CursorNum > (MAXCURSORNUM-1))
                  CursorNum = 0;


              // Do a half step if possible
               if ((handlewhich) &&
                  (((items+handlewhich-1)->active == CP_CursorLocation) ||
                  ((items+handlewhich-1)->active == CP_Active)))
               {
                  y -= 6;
                  DrawHalfStep (x, y);
                  playsnd = false;

                  RefreshMenuBuf (0);

                  CursorNum++;
                  if (CursorNum > (MAXCURSORNUM-1))
                     CursorNum = 0;
               }
               else
               {
                  playsnd = true;
                  RefreshMenuBuf (0);
               }

               do
               {
                  if (!handlewhich)
                     handlewhich = item_i->amount-1;
                  else
                     handlewhich--;
               } while (((items+handlewhich)->active == CP_Inactive) || ((items+handlewhich)->active == CP_Active3));

               if (playsnd)
                  MN_PlayMenuSnd (SD_MOVECURSORSND);
					ShowCursor (item_i, items, x, &y, handlewhich, basey);

               if (routine)
                  routine (handlewhich);

               RefreshMenuBuf(0);
            break;

            case dir_South:
               HideCursor (item_i, items, x, y, handlewhich);

               CursorNum++;
               if (CursorNum > (MAXCURSORNUM-1))
                  CursorNum = 0;

               // Do a half step if possible
               if ((handlewhich != item_i->amount-1) &&
                  (((items+handlewhich+1)->active == CP_CursorLocation) ||
                  ((items+handlewhich+1)->active == CP_Active)))
               {
                  y += 6;
                  DrawHalfStep(x,y);
                  playsnd = false;

                  RefreshMenuBuf (0);

                  CursorNum++;
                  if (CursorNum > (MAXCURSORNUM-1))
                     CursorNum = 0;
               }
               else
               {
                  playsnd = true;
                  RefreshMenuBuf (0);
               }

               do
               {
                  if (handlewhich==item_i->amount-1)
                     handlewhich=0;
                  else
                     handlewhich++;
               } while (((items+handlewhich)->active == CP_Inactive) || ((items+handlewhich)->active == CP_Active3));

               if (playsnd)
                  MN_PlayMenuSnd (SD_MOVECURSORSND);
               ShowCursor(item_i,items,x,&y,handlewhich,basey);

               if (routine)
                  routine (handlewhich);

               RefreshMenuBuf (0);
            break;
	 default:
	     ;
         }
      }

      ReadAnyControl (&ci);
      if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
      {
         exit = 1;
         WaitKeyUp ();
         MN_PlayMenuSnd (SD_SELECTSND);
      }

      if (ci.button1 || Keyboard[sc_Escape])
      {
         WaitKeyUp ();
         exit = 2;
      }

      if ( ( Keyboard[ sc_Home ] ) && ( numactive > 1 ) )
         {
         newpos = 0;
         while( ( items[ newpos ].active == CP_Inactive ) ||
            ( items[ newpos ].active == CP_Active3 ) )
            {
            newpos++;
            }

         if ( newpos != handlewhich )
            {
            HideCursor( item_i, items, x, y, handlewhich );

            CursorNum++;
            if ( CursorNum > ( MAXCURSORNUM - 1 ) )
               {
               CursorNum = 0;
               }

            RefreshMenuBuf( 0 );

            handlewhich = newpos;

            MN_PlayMenuSnd( SD_MOVECURSORSND );

            ShowCursor( item_i, items, x, &y, handlewhich, basey );

            if ( routine )
               {
               routine( handlewhich );
               }

            RefreshMenuBuf( 0 );
            }
         }
      else if ( ( Keyboard[ sc_End ] ) && ( numactive > 1 ) )
         {
         newpos = item_i->amount - 1;
         while( ( items[ newpos ].active == CP_Inactive ) ||
            ( items[ newpos ].active == CP_Active3 ) )
            {
            newpos--;
            }

         if ( newpos != handlewhich )
            {
            HideCursor( item_i, items, x, y, handlewhich );

            CursorNum++;
            if ( CursorNum > ( MAXCURSORNUM - 1 ) )
               {
               CursorNum = 0;
               }

            RefreshMenuBuf( 0 );

            handlewhich = newpos;

            MN_PlayMenuSnd( SD_MOVECURSORSND );

            ShowCursor( item_i, items, x, &y, handlewhich, basey );

            if ( routine )
               {
               routine( handlewhich );
               }

            RefreshMenuBuf( 0 );
            }
         }

      // Page Up/Down
      if ( MenuNum == 11 )
         {
         if ( ( Keyboard[ sc_PgUp ] ) &&
            ( ( items + 1 )->active != CP_Inactive ) )
            {
            item_i->curpos = handlewhich;
            handlewhich = PAGEUP;
            exit = 3;
            MN_PlayMenuSnd( SD_SELECTSND );
            }
         else if ( ( Keyboard[ sc_PgDn ] ) &&
            ( ( items + 0 )->active != CP_Inactive ) )
            {
            item_i->curpos = handlewhich;
            handlewhich = PAGEDOWN;
            exit = 3;
            MN_PlayMenuSnd( SD_SELECTSND );
            }
         }

      // Delete save games
      if ((MenuNum == 4) || (MenuNum == 6))
      {
         if (Keyboard[sc_Delete] && SaveGamesAvail[handlewhich])
         {
            if (CP_DisplayMsg ("Delete saved game?\nAre you sure?", 12) == true)
            {
               char loadname[45] = "rottgam0.rot";
               char filename[128];

               // Create the proper file name
               itoa (handlewhich, &loadname[7], 16);
               loadname[8]='.';

               GetPathFromEnvironment( filename, ApogeePath, loadname );

               // Delete the file

					unlink (filename);

               memset (&SaveGameNames[handlewhich][0], 0, 32);
               SaveGamesAvail[handlewhich] = 0;
               if (handlewhich==quicksaveslot)
                  quicksaveslot=-1;

               PrintX = LSM_X+LSItems.indent+2;
               PrintY = LSM_Y+handlewhich*9+2;
            }
            ScanForSavedGames ();

            LSItems.curpos = handlewhich;
            if (MenuNum == 4)
               DrawLoadSaveScreenAlt (1);
            else
               DrawLoadSaveScreenAlt (0);
            CP_DrawSelectedGame (handlewhich);
         }
      }


#if SAVE_SCREEN
      if (Keyboard[sc_CapsLock] && Keyboard[sc_C])
      {
         inhmenu=true;
         SaveScreen (true);
         inhmenu=false;
      }
      else
         if (Keyboard[sc_CapsLock] && Keyboard[sc_X])
         {
            inhmenu=true;
            SaveScreen (false);
            inhmenu=false;
         }
         else
            if (Keyboard[sc_CapsLock] && Keyboard[sc_Q])
               Error ("Insta-Menu Quit!\n");
#endif

   } while (!exit);


   IN_ClearKeysDown();

   if (routine)
      routine (handlewhich);

   if ( exit != 3 )
      {
      item_i->curpos = handlewhich;
      }

   if (MenuNum == 3)
   {
      if (exit != 2)
         CSTactive = handlewhich;
      else
         CSTactive = -1;
   }

   switch (exit)
   {
      case 1:
         if ((items+handlewhich)->routine!=NULL)
            (items+handlewhich)->routine(0);
         return (handlewhich);

      case 2:
         MN_PlayMenuSnd (SD_ESCPRESSEDSND);
         return (-1);

      case 3:
         return( handlewhich );
   }


   return (0);
}

//******************************************************************************
//
// HideCursor
//
//******************************************************************************
void HideCursor
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int x,
   int y,
   int which
   )

   {
   int time = GetTicCount();
   int color;
   int delay;
   int posx;
   int posy;

   if ( MenuNum != 5 )
      {
		EraseMenuBufRegion( x, y, cursorwidth, cursorheight );
      }

   if ( MenuNum && ( MenuNum != 4 ) && ( MenuNum != 6 ) )
      {
      posx = item_i->x + item_i->indent;
      posy = item_i->y + ( which * yinc );

      color = -1;
      switch( items[ which ].active )
         {
         case CP_Inactive :
            color = NOTAVAILABLECOLOR;
            break;

         case CP_CursorLocation :
         case CP_Active :
            color = NORMALCOLOR;
            break;

         case CP_SemiActive :
            color = DIMMEDCOLOR;
            break;

         case CP_Highlight :
            color = HIGHLIGHTCOLOR;
            break;
         }

      if ( color != -1 )
         {
         if ( item_i->names == NULL )
            {
            DrawIMenuBufItem( posx, posy,
               W_GetNumForName( items[ which ].texture ), color );
            }
         else
            {
            IFont = ( cfont_t * )W_CacheLumpName( FontNames[ item_i->fontsize ],
               PU_CACHE, Cvt_cfont_t, 1 );
            if ( item_i->fontsize == mn_tinyfont )
               {
               DrawMenuBufIString( posx + 1, posy, item_i->names[ which ], 0 );
               }
            DrawMenuBufIString( posx, posy - 1, item_i->names[ which ],
               color );
            }
         }
      }

   if ( ( items[ which ].active != CP_Inactive ) &&
      ( items[ which ].active != CP_SemiActive ) )
      {
      items[ which ].active = CP_Active;
      }

   delay = DELAYAMT - tics;
   while( ( time + delay ) > GetTicCount() )
      {
      RefreshMenuBuf (0);
      }
   }


//******************************************************************************
//
// DrawHalfStep
//
//******************************************************************************

void DrawHalfStep (int x, int y)
{
   MN_PlayMenuSnd (SD_MOVECURSORSND);
   if (MenuNum == 5)
      return;

   DrawMenuBufItem (x, y, W_GetNumForName( CursorLump ) +
      CursorFrame[ CursorNum ] );
}


//******************************************************************************
//
// GetNumActive ()
//
//******************************************************************************

int GetNumActive (CP_iteminfo *item_i, CP_itemtype *items)
{
   int cnt;
   int num = 0;

   for (cnt = 0; cnt < item_i->amount; cnt ++)
   {
      if ((items+cnt)->active != CP_Inactive)
         num++;
   }

   return (num);
}


//******************************************************************************
//
// ShowCursor
//    Draw triad at new position.
//
//******************************************************************************
void ShowCursor
   (
   CP_iteminfo *item_i,
   CP_itemtype *items,
   int x,
   int *y,
   int which,
   int basey
   )

   {
   int time = GetTicCount();
   int delay;
   int posx;
   int posy;

   if ( MenuNum )
      {
      EraseMenuBufRegion( x, *y, cursorwidth, cursorheight );
      *y = basey + which * yinc;
      }
   else
      {
      *y = CUSTOM_y[ which ];
      }

   if ( MenuNum != 5 )
      {
      DrawMenuBufItem( x, *y, W_GetNumForName( CursorLump ) +
         CursorFrame[ CursorNum ] );
      }

   if ( items[ which ].active != CP_SemiActive )
      {
      if ( MenuNum && ( MenuNum != 4 ) && ( MenuNum != 6 ) )
         {
         posx = item_i->x + item_i->indent;
         posy = item_i->y + which * yinc;

         if ( item_i->names == NULL )
            {
            DrawIMenuBufItem( posx, posy,
               W_GetNumForName( items[ which ].texture ), ACTIVECOLOR);
            }
         else
            {
            IFont = ( cfont_t * )W_CacheLumpName( FontNames[ item_i->fontsize ],
               PU_CACHE, Cvt_cfont_t, 1 );
            if ( item_i->fontsize == mn_tinyfont )
               {
               DrawMenuBufIString( posx + 1, posy, item_i->names[ which ], 0 );
               }
            DrawMenuBufIString( posx, posy - 1, item_i->names[ which ],
               ACTIVECOLOR );
            }
         }

      items[ which ].active = CP_CursorLocation;
      }

   delay = DELAYAMT - tics;
   while( ( time + delay ) > GetTicCount() )
      {
      RefreshMenuBuf( 0 );
      }
   }

//******************************************************************************
//
// DrawOrderInfo()
//
//******************************************************************************

void DrawOrderInfo
   (
   int which
   )

   {
   int start;
   char *lumpname;

   start = W_GetNumForName( "ORDRSTRT" ) + 1;

   lumpname = W_GetNameForNum( start + which );

   // Screen shots are grabbed as pics
   if ( lumpname[ 0 ] == 'S' )
      {
      VWB_DrawPic( 0, 0, ( pic_t * )W_CacheLumpNum( start + which, PU_CACHE, Cvt_pic_t, 1 ) );
      }
   else
      {
      VL_DrawPostPic( W_GetNumForName( "trilogo" ) );
      DrawNormalSprite( 0, 0, start );
      DrawNormalSprite( 0, 0, start + which );
      }

   VW_UpdateScreen();
   }


//******************************************************************************
//
// CP_OrderInfo()
//
//******************************************************************************

void CP_OrderInfo
   (
   void
   )

   {
   int maxpage;
   int page;
   int key;
   boolean newpage;



   maxpage = W_GetNumForName( "ORDRSTOP" ) - W_GetNumForName( "ORDRSTRT" ) - 2;
   newpage = false;
   page = 1;

   do
      {
	  EnableScreenStretch();//bna++
      DrawOrderInfo( page );
	  DisableScreenStretch();//bna++ turn off or screen will be strected every time it passes VW_UpdateScreen
      if ( newpage )
         {
         while( Keyboard[ key ] )
            {
            VW_UpdateScreen();
            IN_UpdateKeyboard ();
            }
         }

      LastScan=0;
      while( LastScan == 0 )
         {
         VW_UpdateScreen();
         IN_UpdateKeyboard ();
         }

      key = LastScan;
      switch( key )
         {
         case sc_Home :
            if ( page != 1 )
               {
               page = 1;
               newpage = true;
               MN_PlayMenuSnd( SD_MOVECURSORSND );
               }
            break;

         case sc_End :
            if ( page != maxpage )
               {
               page = maxpage;
               newpage = true;
               MN_PlayMenuSnd( SD_MOVECURSORSND );
               }
            break;

         case sc_PgUp :
         case sc_UpArrow :
         case sc_LeftArrow :
            if ( page > 1 )
               {
               page--;
               newpage = true;
               MN_PlayMenuSnd( SD_MOVECURSORSND );
               }
            break;

         case sc_PgDn :
         case sc_DownArrow :
         case sc_RightArrow :
            if ( page < maxpage )
               {
               page++;
               newpage = true;
               MN_PlayMenuSnd( SD_MOVECURSORSND );
               }
            break;
         }
      }
   while( key != sc_Escape );

   Keyboard[ key ] = 0;
   LastScan = 0;
 EnableScreenStretch();//bna++
   MN_PlayMenuSnd( SD_ESCPRESSEDSND );
   }


//******************************************************************************
//
// CP_ViewScores ()
//
//******************************************************************************

void CP_ViewScores (void)
{
   CheckHighScore (0, 0, true);
}


//******************************************************************************
//
// CP_Quit () - QUIT THIS INFERNAL GAME!
//
//******************************************************************************
void CP_Quit ( int which )
{
   int num = 100;
   static int oldnum;

   while ((num >= 7) || (oldnum == num))
      num = (RandomNumber ("CP_QUIT", 0) & 7);

   oldnum = num;

   if (CP_DisplayMsg (endStrings[num], num))
   {
      int handle;

      MU_FadeOut(310);
      handle=SD_Play(SD_QUIT1SND+num);
      VL_FadeOut (0, 255, 0, 0, 0, 10);
      CleanUpControlPanel();
      SD_WaitSound (handle);
      QuitGame ();
   }

   if ( which != -1 )
      {
      ClearMenuBuf();
      DrawMainMenu();
      DrawMenuBufItem (MainItems.x,  ((MainItems.curpos*14)+(MainItems.y-2)),
         W_GetNumForName ( LargeCursor ) + CursorFrame[ CursorNum ] );
      RefreshMenuBuf (0);
      }
}

//******************************************************************************
//
// CP_DisplayMsg ()
//
//******************************************************************************

boolean CP_DisplayMsg
   (
   char *s,
   int number
   )

   {
#define Q_W    184
#define Q_H    72
#define Q_X    ((320-Q_W)/2)-18
#define Q_Y    ((200-Q_H)/2)-33

#define Q_b1X  (Q_X+85)
#define Q_b2X  (Q_X+135)
#define Q_bY   (Q_Y+45)
#define Q_bW   33
#define Q_bH   10

#define W_X    72
#define W_Y    11
#define W_W    102

#define YES    "q_yes\0"
#define NO     "q_no\0"

   ControlInfo ci;
   boolean retval;
   boolean done;
   boolean YESON;
   boolean redraw;
   boolean blowout;
   char   *temp;
   char   *active;
   char   *inactive;
   int     activex;
   int     inactivex;
   int     W_H = 0;
   int     L_Y = 0;
   int     tri;
   int     QUITPIC;
   int     t;

   W_H = 1;
   retval  = false;
   done    = false;
   YESON   = true;
   redraw  = false;
   blowout = false;

   IN_ClearKeysDown();
   IN_IgnoreMouseButtons();


   QUITPIC = W_GetNumForName( "quitpic" );

   if ( number < 11 )
      {
      tri = W_GetNumForName( "QUIT01" ) + number;
      MN_PlayMenuSnd( SD_WARNINGBOXSND );
      }
   else
      {
      if ( number == 11 )
         {
         tri = W_GetNumForName( "tri1pic" );
         MN_PlayMenuSnd( SD_INFOBOXSND );
         }
      else
         {
         if ( number == 12 )
            {
            tri = W_GetNumForName( "tri2pic" );
            MN_PlayMenuSnd( SD_QUESTIONBOXSND );
            }
         if ( number == 13 )
            {
            tri = W_GetNumForName( "tri1pic" );
            MN_PlayMenuSnd( SD_WARNINGBOXSND );
            }
         }
      }

   DrawMenuBufPic( Q_X, Q_Y, QUITPIC );
   DrawMenuBufPic( Q_X + 12, Q_Y + 11, tri );

   temp = s;
   while( *temp )
      {
      if ( *temp == '\n' )
         {
         W_H++;
         }
      temp++;
      }

   CurrentFont = tinyfont;
   W_H = ( W_H * CurrentFont->height ) + 3;

   WindowX = Q_X + W_X;
   WindowY = L_Y + 2;
   L_Y     = Q_Y + W_Y;
   PrintX  = WindowX;
   PrintY  = WindowY;

   WindowW = W_W;
   WindowH = W_H;

   redraw = true;

   IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_smallfont ],
      PU_CACHE, Cvt_cfont_t, 1 );
/*
   DrawSTMenuBuf( WindowX, L_Y, W_W, W_H, false );
   MenuBufCPrint( s );

   DrawSTMenuBuf( Q_b1X, Q_bY, Q_bW, Q_bH, false );
   DrawMenuBufIString( Q_b1X + 3, Q_Y + 46, "YES", NORMALCOLOR );
//   DrawIMenuBufItem (PrintX, PrintY, W_GetNumForName (YES), NORMALCOLOR);

   DrawSTMenuBuf( Q_b2X, Q_bY, Q_bW, Q_bH, true );
   DrawMenuBufIString( Q_b2X + 2, Q_Y + 45, "NO", ACTIVECOLOR );
//   DrawIMenuBufItem (PrintX, PrintY, W_GetNumForName (NO), ACTIVECOLOR);
*/
   if (number != 13)
   {
   while ( !done )
      {
      RefreshMenuBuf( 0 );

      ReadAnyControl( &ci );

      if ( ( ci.dir == dir_West ) && ( !YESON ) )
         {
         MN_PlayMenuSnd( SD_MOVECURSORSND );
         YESON = 1;
         redraw = true;
         }
      else if ( ( ci.dir == dir_East ) && ( YESON ) )
         {
         MN_PlayMenuSnd( SD_MOVECURSORSND );
         YESON = 0;
         redraw = true;
         }

      if ( Keyboard[ sc_Y ] )
         {
         YESON  = 1;
         redraw = true;
         Keyboard[ sc_Enter ] = true;
         blowout = true;
         }
      else if ( Keyboard[ sc_N ] )
         {
         YESON  = 0;
         redraw = true;
         Keyboard[ sc_Enter ] = true;
         blowout = true;
         }

      if ( redraw )
         {
         redraw = false;

         DrawMenuBufPic( Q_X, Q_Y, QUITPIC );
         DrawMenuBufPic( Q_X + 12, Q_Y + 11, tri );

         PrintX = Q_X + W_X;
         PrintY = Q_Y + W_Y + 2;
         DrawSTMenuBuf( WindowX, L_Y, W_W, W_H, false );
         CurrentFont = tinyfont;
         MenuBufCPrint( s );

         if ( YESON )
            {
            active    = "YES";
            inactive  = "NO";
            activex   = Q_b1X;
            inactivex = Q_b2X;
            }
         else
            {
            active    = "NO";
            inactive  = "YES";
            activex   = Q_b2X;
            inactivex = Q_b1X;
            }

         DrawSTMenuBuf( activex, Q_bY, Q_bW, Q_bH, false );
         DrawMenuBufIString( activex + 3, Q_Y + 46, active, ACTIVECOLOR );
//         DrawIMenuBufItem (PrintX, PrintY, W_GetNumForName (YES), NORMALCOLOR);

         DrawSTMenuBuf( inactivex, Q_bY, Q_bW, Q_bH, true );
         DrawMenuBufIString( inactivex + 2, Q_Y + 45, inactive, NORMALCOLOR );
//         DrawIMenuBufItem (PrintX, PrintY, W_GetNumForName (NO), ACTIVECOLOR);

         for( t = 0; t < 5; t++ )
            {
            RefreshMenuBuf( 0 );
            }
         }

      if ( ( Keyboard[ sc_Space ] || Keyboard[ sc_Enter ] ||
         ci.button0 ) && YESON )
         {
         done   = true;
         retval = true;
         MN_PlayMenuSnd( SD_SELECTSND );
         CP_Acknowledge = CP_YES;
         }
      else if ( Keyboard[ sc_Escape ] || ci.button1 )
         {
         done   = true;
         retval = false;
         CP_Acknowledge = CP_ESC;
         MN_PlayMenuSnd( SD_ESCPRESSEDSND );
         }
      else if ( ( Keyboard[ sc_Space ] || Keyboard[ sc_Enter ] ||
         ci.button0 ) && !YESON )
         {
         done   = true;
         retval = false;
         CP_Acknowledge = CP_NO;

         if ( Keyboard[ sc_N ] )
            {
            MN_PlayMenuSnd( SD_SELECTSND );
            }
         else
            {
            MN_PlayMenuSnd( SD_ESCPRESSEDSND );
            }
         }
      }

   while( ( Keyboard[ sc_Enter ] || Keyboard[ sc_Space ] ||
      Keyboard[ sc_Escape ] ) && !blowout )
      {
      IN_UpdateKeyboard();
      RefreshMenuBuf( 0 );
      }
   }
   else
   {
      PrintX = Q_X + W_X;
      PrintY = Q_Y + W_Y + 2;
      DrawSTMenuBuf( WindowX, L_Y, W_W, W_H, false );
      CurrentFont = tinyfont;
      MenuBufCPrint( s );
      LastScan=0;
      while (LastScan == 0)
         {
         IN_UpdateKeyboard();
         RefreshMenuBuf( 0 );
         }
      LastScan = 0;
   }
   IN_ClearKeysDown();
   return( retval );
   }


//******************************************************************************
//
// EndGameStuff ()
//
//******************************************************************************

void EndGameStuff (void)
{
   Z_FreeTags( PU_LEVELSTRUCT, 255 );

   InitStaticList ();
   InitActorList();

	pickquick = false;
   CheckHighScore (gamestate.score, gamestate.mapon+1, true);
   locplayerstate->lives = 0;
   playstate = ex_died;
   damagecount = 0;
   SetBorderColor (0);

   AdjustMenuStruct ();
   ingame = false;

   GamePaused  = false;
}


//******************************************************************************
//
// START A NEW GAME
//
//******************************************************************************

#define CURGAME   "You are currently in\n"\
      "a game. Continuing will\n"\
      "erase old game. Ok?\0"

int ToughMenuNum;

void CP_NewGame
   (
   void
   )

   {
   int which;

#if ( SHAREWARE == 1 )
   ToughMenuNum = 0;
#else
   int temp;

   temp = ToughMenuNum;

   while( ToughMenuNum == temp )
      {
		temp = ( ( RandomNumber( "TOUGH MENU", 0 ) ) & 3 );
      if ( temp == 3 )
         {
         temp = 1;
         }
      }

   ToughMenuNum = temp;
#endif

   //
   // ALREADY IN A GAME?
   //
   if ( ingame )
      {
      if ( !CP_DisplayMsg( CURGAME, 12 ) )
         {
         return;
         }
      else
         {
         EndGameStuff();
         }
      }
   else
      {
      handlewhich = 100;
      }

	if ( CP_PlayerSelection() == 0 )
      {
		return;
      }

   TufMenu[ ToughMenuNum ][ 0 ].active = CP_Active;
   TufMenu[ ToughMenuNum ][ 2 ].active = CP_Active;
   TufMenu[ ToughMenuNum ][ 4 ].active = CP_Active;
   TufMenu[ ToughMenuNum ][ 6 ].active = CP_Active;

   switch( DefaultDifficulty )
      {
      case gd_baby :
         TufItems.curpos = 0;
         break;

      case gd_easy :
         TufItems.curpos = 2;
         break;

      case gd_medium :
         TufItems.curpos = 4;
         break;

      case gd_hard :
         TufItems.curpos = 6;
         break;

      default :
         TufItems.curpos = 0;
         break;
      }

   TufMenu[ ToughMenuNum ][ TufItems.curpos ].active = CP_CursorLocation;

   DrawNewGame();

   which = HandleMenu( &TufItems, &TufMenu[ ToughMenuNum ][ 0 ],
      DrawNewGameDiff );

   if ( which < 0 )
      {
      handlewhich = 1;
      return;
      }

   handlewhich = 0;

   switch( which )
      {
      case 0 :
         DefaultDifficulty = gd_baby;
         break;

      case 2 :
         DefaultDifficulty = gd_easy;
         break;

      case 4 :
         DefaultDifficulty = gd_medium;
         break;

      case 6 :
         DefaultDifficulty = gd_hard;
         break;
      }

   MainMenu[ savegame ].active = CP_Active;

   gamestate.battlemode = battle_StandAloneGame;
   StartGame = true;
   DisableScreenStretch();
   playstate = ex_resetgame;


   }

//******************************************************************************
//
// CP_EndGame ()
//
//******************************************************************************

#define ENDGAMESTR   "Are you sure you want\n"\
               "to end the game you\n"\
               "are playing? (Y or N):"

void CP_EndGame
   (
   void
   )

   {
   boolean action;

   SetMenuTitle( "End Game" );
   action = CP_DisplayMsg( ENDGAMESTR, 12 );

   StartGame = false;
   EnableScreenStretch();
   if ( action )
      {
      EndGameStuff ();
      pickquick = false;
      }
   }

//******************************************************************************
//
// AdjustMenuStruct ()
//
//******************************************************************************
void AdjustMenuStruct
   (
   void
   )

   {
   MainMenu[ savegame ].active         = CP_Inactive;
   MainMenu[ viewscores ].routine      = ( void * )CP_ViewScores;
   MainMenu[ viewscores ].texture[ 6 ] = '7';
   MainMenu[ viewscores ].texture[ 7 ] = '\0';
   MainMenu[ viewscores ].letter       = 'V';
   strcpy (MainMenuNames[ viewscores ] , "VIEW SCORES");
   }

//******************************************************************************
//
// CP_DrawSelectedGame
//
//******************************************************************************

void CP_DrawSelectedGame (int w)
{
   extern gamestorage_t *menu_game_storage;
   gamestorage_t *game = menu_game_storage;

   if (SaveGamesAvail[w])
      {
      GetSavedHeader(w,game);

      DrawStoredGame(&game->picture[0],game->episode,game->area);
      }
   else
      EraseMenuBufRegion(SaveGamePicX,SaveGamePicY,160,124);
}

//******************************************************************************
//
// DrawStoredGame
//
//******************************************************************************

void DrawStoredGame ( byte * pic, int episode, int area )
{
   char str[3];
   int level;
   byte *shape;

	shape = W_CacheLumpNum (W_GetNumForName ("newfnt1"), PU_CACHE, Cvt_font_t, 1);
	newfont1 = (font_t *)shape;
   CurrentFont = newfont1;
   EraseMenuBufRegion (74, 128, 85, 14);

	DrawMenuBufPropString (74, 128, "E");

   itoa (episode, str, 10);
   DrawMenuBufPropString (87, 128, str);

   DrawMenuBufPropString (103, 128, "A");

   if (episode > 1)
      level = (area+1) - ((episode-1) << 3);
   else
      level = area+1;

   ltoa (level, str, 10);
   DrawMenuBufPropString (117, 128, str);
   CurrentFont = tinyfont;

   DrawMenuBufPicture(SaveGamePicX,SaveGamePicY,pic,160,100);
}



//******************************************************************************
//
// DoLoad ()
//
//******************************************************************************

int DoLoad (int which)
{
   extern gamestorage_t *menu_game_storage;
   gamestorage_t *game = menu_game_storage;
   int exit = 0;

   if ((which >= 0) && SaveGamesAvail[which])
   {
      loadedgame = true;

      if (loadsavesound)
         MN_PlayMenuSnd (SD_SELECTSND);

      if (LoadTheGame (which, game) == true)
      {
         MenuFixup ();
		 DisableScreenStretch();
         StartGame = true;
			exit      = 1;
      }
      else
      {
         if (CP_DisplayMsg ("Saved Game is\n old or incompatible\nDelete it?", 12)==true)
         {
            char loadname[45] = "rottgam0.rot";
            char filename[128];

            // Create the proper file name
            itoa (which, &loadname[7], 16);
            loadname[8]='.';

            GetPathFromEnvironment( filename, ApogeePath, loadname );

            // Delete the file

            unlink (filename);

            memset (&SaveGameNames[which][0], 0, 32);
            SaveGamesAvail[which] = 0;
         }

         loadedgame = false;
         DrawLoadSaveScreenAlt (0);
      }
   }

   return (exit);
}


//******************************************************************************
//
// LOAD SAVED GAMES
//
//******************************************************************************

int CP_LoadGame (int quick, int dieload)
{
   int which,
       exit = 0;


	MenuNum = 6;

   SaveTime = GetTicCount();

   //
   // QUICKLOAD?
   //
   if (quick)
   {
      which = LSItems.curpos;

      if (SaveGamesAvail[which])
      {
         if (dieload)
         {
            DrawLoadSaveScreenAlt (0);
            CP_DrawSelectedGame (which);
            RefreshMenuBuf (0);
            DoLoad (which);

            return (1);
         }
         else
         {
            DrawLoadSaveScreen (0);
            if (CP_DisplayMsg ("Quick load saved game?\nAre you sure?", 12) == true)
            {
               DrawLoadSaveScreen (0);
               CP_DrawSelectedGame (which);
               RefreshMenuBuf (0);
               DoLoad (which);

               return (1);
            }
            else
            {
					return (0);
            }
         }
      }
   }

   DrawLoadSaveScreen (0);
   do
   {
      which = HandleMenu (&LSItems, &LSMenu[0], CP_DrawSelectedGame);

      if ((exit = DoLoad (which)))
         break;

   } while (which >= 0);

   handlewhich = OUTOFRANGE;

   if (MainMenu[loadgame].active == CP_Inactive)    // If all the saved games have been
   {
      MainItems.curpos = 0;               //  deleted dehighlight LOADGAME
      MainMenu[newgame].active = CP_CursorLocation;
   }

   return exit;
}

//******************************************************************************
//
// QuickSaveGame ()
//
//******************************************************************************

void QuickSaveGame (void)
{
   int i;
   int j;
   byte * b;
   byte * s;
   int which;
   extern gamestorage_t *menu_game_storage;
   gamestorage_t *game = menu_game_storage;
   byte * buf;
   int length;

   char   loadname[45]="rottgam0.rot";
   char   filename[128];

   // Create the proper file name

   itoa(quicksaveslot,&loadname[7],16);
   loadname[8]='.';

   GetPathFromEnvironment( filename, ApogeePath, loadname );
   length=LoadFile(filename,(void **)&buf);
   GetPathFromEnvironment( filename, ApogeePath, QUICKSAVEBACKUP );
   SaveFile(filename,buf,length);
   SafeFree(buf);

   s=&game->picture[0];
   for (i=0;i<320;i+=2)
      {
#ifdef DOS
      VGAREADMAP(i&3);
      b=(byte *)bufferofs+(i>>2);
      for (j=0;j<100;j++,s++,b+=iGLOBAL_SCREENBWIDE<<1)
         *s=*b;
#else
      b=(byte *)bufferofs+i;
      for (j=0;j<100;j++,s++,b+=(iGLOBAL_SCREENWIDTH<<1))
         *s=*b;
#endif
      }

   ScanForSavedGames ();
   which = quicksaveslot;

   if (SaveGamesAvail[which])
      {
      game->episode = gamestate.episode;
      game->area    = gamestate.mapon;
      game->version = ROTTVERSION;
      strcpy (game->message, &SaveGameNames[which][0]);

      if (SaveTheGame (which, game) == true)
         {
         char str[50];

         strcpy (str, "Game Saved: ");
         strcat (str, &SaveGameNames[which][0]);
         AddMessage(str,MSG_SYSTEM);
         }
      else
         {
         AddMessage("Game Not Saved.",MSG_SYSTEM);
         }
      }
   else
      {
      AddMessage("No Quick Save Slot.",MSG_SYSTEM);
      }

   }

//******************************************************************************
//
// UndoQuickSaveGame ()
//
//******************************************************************************

void UndoQuickSaveGame (void)
{
   byte * buf;
   char   loadname[45]="rottgam0.rot";
   char   filename[128];
   int length;

   if (quicksaveslot!=-1)
      {
      // Create the proper file name

      itoa(quicksaveslot,&loadname[7],16);
      loadname[8]='.';
      GetPathFromEnvironment( filename, ApogeePath, QUICKSAVEBACKUP );
      length=LoadFile(filename,(void **)&buf);
      GetPathFromEnvironment( filename, ApogeePath, loadname );
      SaveFile(filename,buf,length);
      SafeFree(buf);
      AddMessage("Previous Quicksave Game Restored.",MSG_SYSTEM);
      }
   else
      {
      AddMessage("No Quick Save Slot Selected.",MSG_SYSTEM);
      }
}


//******************************************************************************
//
// SAVE CURRENT GAME
//
//******************************************************************************
int CP_SaveGame ( void )
{
   int  which,
        exit=0;

   char input[32];
   extern gamestorage_t *menu_game_storage;
   gamestorage_t *game = menu_game_storage;


        MenuNum = 4;


   DrawLoadSaveScreen (1);

   do
   {
      which = HandleMenu (&LSItems, &LSMenu[0], CP_DrawSelectedGame);
      if (which >= 0)
      {
         //
         // OVERWRITE EXISTING SAVEGAME?
         //
          if (SaveGamesAvail[which]) {
            if (!CP_DisplayMsg (GAMESVD, 12))
            {
               DrawLoadSaveScreenAlt (1);
               continue;
            }
            else
            {
               DrawLoadSaveScreenAlt (1);
               EraseMenuBufRegion (LSM_X+LSItems.indent, LSM_Y+1+which*9, 80, 8);
               PrintLSEntry (which);
            }
          }
         quicksaveslot=which;

         DrawStoredGame (savedscreen, gamestate.episode, gamestate.mapon);

         strcpy (input, &SaveGameNames[which][0]);

         if (!SaveGamesAvail[which])
            EraseMenuBufRegion (LSM_X+LSItems.indent+1, LSM_Y+which*9+2,
                                77, 6);

         if (US_LineInput (LSM_X+LSItems.indent+2, LSM_Y+which*9+2,
                           input, input, true, 22, 75, 0))
         {
            SaveGamesAvail[which] = 1;
            memcpy(&game->picture[0],savedscreen,16000);
            game->episode=gamestate.episode;
            game->area=gamestate.mapon;
            game->version=ROTTVERSION;
            strcpy (game->message, input);
            strcpy (&SaveGameNames[which][0], input);

            if (SaveTheGame(which,game)==true)
               {

					MainMenu[loadgame].active=CP_Active;

//               MN_PlayMenuSnd (SD_SELECTSND);
               exit = 1;
               }
            WaitKeyUp ();
         }
         else
         {
            EraseMenuBufRegion (LSM_X+LSItems.indent+1, LSM_Y+which*9+2,
                                77, 6);

            PrintX = LSM_X+LSItems.indent+2;
            PrintY = LSM_Y+which*9+2;

            if (SaveGamesAvail[which])
               DrawMenuBufPropString (PrintX, PrintY, SaveGameNames[which]);
            else
               DrawMenuBufPropString (PrintX, PrintY, "     - � -");

//            MN_PlayMenuSnd (SD_ESCPRESSEDSND);
            continue;
         }
         break;
      }

   } while (which >= 0);

   handlewhich = OUTOFRANGE;

   return (exit);
}



//******************************************************************************
//
// DEFINE CONTROLS
//
//******************************************************************************

void CP_Control (void)
{
   #define CTL_SPC   70

   int which;

   DrawCtlScreen ();
   WaitKeyUp ();

   do
   {
      which = HandleMenu (&CtlItems, &CtlMenu[0], NULL);

      switch (CSTactive)
      {
         case MOUSEENABLE:
            if (MousePresent)
            {
               mouseenabled^=1;
               DrawCtlButtons ();
               CusItems.curpos=-1;
            }
            else
               mouseenabled = 0;
         break;

			case JOYENABLE:
            joystickenabled^=1;
            if ( joystickenabled )
               {
               if ( !CalibrateJoystick() )
                  {
                  joystickenabled = 0;
                  joypadenabled = 0;
                  }
               }
            else
               {
               joypadenabled = 0;
               }
            DrawCtlScreen();
            break;

         case USEPORT2:
            joystickport^=1;
            if ( joystickport )
               {
               joypadenabled = 0;
               }

            joystickenabled = 1;
            if ( !CalibrateJoystick() )
               {
               joystickenabled = 0;
               joystickport = 0;
               }

            if ( joystickport )
               {
               CtlMenu[ 3 ].active = CP_Inactive;
               joypadenabled = 0;
               }
            else
               {
               CtlMenu[ 3 ].active = CP_Active;
               }

            DrawCtlScreen();
         break;

         case PADENABLE:
            joypadenabled^=1;
            if ( ( joypadenabled ) && ( !joystickenabled ) )
               {
               joystickenabled=1;
               if ( !CalibrateJoystick() )
                  {
                  joystickenabled = 0;
                  joypadenabled = 0;
                  }

               DrawCtlScreen();
               }
            else
               {
               DrawCtlButtons ();
               }
         break;

         case SPACEBALLENABLE:
            spaceballenabled ^= 1;
            DrawCtlButtons ();
         break;

         case CYBERMANENABLE:
            cybermanenabled ^= 1;
            DrawCtlButtons ();
         break;

			case THRESSENS:
         case MOUSESENS:
         case CUSTOMIZE:
            DrawCtlScreen ();
         break;
      }

   } while (which >= 0);

   DrawControlMenu ();

   if (which < 0)
   {
      handlewhich = 1;
      return;
   }
}


//****************************************************************************
//
// CP_Custom ()
//
//****************************************************************************

void CP_Custom (void)
{
   int which;

   DrawCustomMenu();

   do
   {
      which = HandleMenu (&CustomItems, &CustomMenu[0], NULL);
   } while (which >= 0);

   DrawCtlScreen ();
}


//******************************************************************************
//
// CUSTOMIZE CONTROLS
//
//******************************************************************************

//****************************************************************************
//
// CP_Keyboard ()
//
//****************************************************************************
void CP_Keyboard
   (
   void
   )

   {
   int which;

   MenuNum = 1;

   DrawCustomKeyboard ();

   do
      {
      which = HandleMenu( &NormalKeyItems, &NormalKeyMenu[ 0 ], NULL );
      }
   while( which >= 0 );

   DrawCustomMenu();
   }


//******************************************************************************
//
// DEFINE THE KEYBOARD BUTTONS
//
//******************************************************************************

void DefineKey
   (
   void
   )

   {
   boolean tick;
   boolean picked;
   int     timer;
   int     x;
   int     y;

   tick   = false;
   picked = false;
   timer  = GetTicCount();

   x = NORMALKEY_X + 97;
   y = NORMALKEY_Y + ( handlewhich * FontSize[ NormalKeyItems.fontsize ] );

   strcpy( &NormalKeyNames[ handlewhich ][ KEYNAMEINDEX ],
      "     " );

//   SetMenuTitle ( "Select which key to use" );
   ClearMenuBuf();
   DrawMenu( &NormalKeyItems, &NormalKeyMenu[ 0 ] );
   DisplayInfo( 0 );

   DrawMenuBufIString( x + 3, y, "?", 0 );
   DrawMenuBufIString( x + 2, y - 1, "?", HIGHLIGHTCOLOR );

   RefreshMenuBuf( 0 );

   do
      {

      IN_PumpEvents();

      //
      // FLASH CURSOR
      //
      if ( ( GetTicCount() - timer ) > 10 )
         {
         int color;

         if ( tick )
            {
            color = HIGHLIGHTCOLOR;
            }
         else
            {
            color = DIMMEDCOLOR;
            }

         DrawMenuBufIString( x + 3, y, "?", 0 );
         DrawMenuBufIString( x + 2, y - 1, "?", color );

         tick  = !tick;
         timer = GetTicCount();
         }

      RefreshMenuBuf( 0 );


      if ( LastScan )
         {
         int key;

         key = LastScan;
         LastScan = 0;

         buttonscan[ (unsigned int)order[ handlewhich ] ] = key;

         strcpy( &NormalKeyNames[ handlewhich ][ KEYNAMEINDEX ],
            IN_GetScanName( key ) );

         picked = true;

         WaitKeyUp();
         Keyboard[ key ] = 0;

         IN_ClearKeysDown();
         }
      }
   while( !picked );

   ClearMenuBuf();
   SetMenuTitle( "Customize Keyboard" );

   DrawMenu( &NormalKeyItems, &NormalKeyMenu[ 0 ] );
   DisplayInfo( 0 );
   RefreshMenuBuf (0);
   }

//****************************************************************************
//
// DrawControlSelect ()
//
//****************************************************************************

void DrawControlSelect
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Select Button Function");

   MN_GetCursorLocation( &ControlSelectItems, &ControlSelectMenu[ 0 ] );
   DrawMenu( &ControlSelectItems, &ControlSelectMenu[ 0 ] );

   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// DefineMouseBtn()
//
//****************************************************************************

void DefineMouseBtn
   (
   void
   )

   {
   int button;
   int which;

   button = handlewhich;

   MN_GetActive( &ControlSelectItems, &ControlSelectMenu[ 0 ],
      buttonmouse[ button ], controlorder );

   DrawControlSelect();

   which = HandleMenu( &ControlSelectItems, &ControlSelectMenu[ 0 ], NULL );
   if ( which != -1 )
      {
      buttonmouse[ button ] = controlorder[ which ];
      }

   handlewhich = OUTOFRANGE;
   }

//****************************************************************************
//
// CP_Mouse ()
//
//****************************************************************************
void CP_Mouse
   (
   void
   )

   {
   int which;

   MenuNum = 1;

   do
      {
      DrawCustomMouse();
      which = HandleMenu( &MouseBtnItems, &MouseBtnMenu[ 0 ], NULL );
      }
   while( which >= 0 );

   handlewhich = OUTOFRANGE;

   DrawCustomMenu();
   }


//****************************************************************************
//
// DrawCustomMouse ()
//
//****************************************************************************

void DrawCustomMouse
   (
   void
   )

   {
   int i;
   int j;
   int num;
   int button;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Customize Mouse");

   for( i = 0; i < 6; i++ )
      {
      num = 0;
      button = buttonmouse[ i ];
      buttonmouse[ i ] = bt_nobutton;
      MouseBtnNames[ i ][ MOUSEBTNINDEX ] = 0;

      for( j = 0; j < NUMCONTROLNAMES; j++ )
         {
         if ( button == controlorder[ j ] )
            {
            buttonmouse[ i ] = button;
            num = j;
            break;
            }
         }

      strcpy( &MouseBtnNames[ i ][ MOUSEBTNINDEX ],
         ControlNames[ num ] );
      }

   MN_GetCursorLocation( &MouseBtnItems, &MouseBtnMenu[ 0 ] );
   DrawMenu( &MouseBtnItems, &MouseBtnMenu[ 0 ] );

   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// DefineJoyBtn()
//
//****************************************************************************

void DefineJoyBtn
   (
   void
   )

   {
   int button;
   int which;

   button = handlewhich;

   MN_GetActive( &ControlSelectItems, &ControlSelectMenu[ 0 ],
      buttonjoy[ button ], controlorder );

   DrawControlSelect();

   which = HandleMenu( &ControlSelectItems, &ControlSelectMenu[ 0 ], NULL );
   if ( which != -1 )
      {
      buttonjoy[ button ] = controlorder[ which ];
      }

   handlewhich = OUTOFRANGE;
   }


//****************************************************************************
//
// DrawCustomJoystick ()
//
//****************************************************************************

void DrawCustomJoystick
   (
   void
   )

   {
   int i;
   int j;
   int num;
   int button;
   int active;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Customize Joystick");

   for( i = 0; i < 8; i++ )
      {
      num = 0;
      button = buttonjoy[ i ];
      buttonjoy[ i ] = bt_nobutton;
      JoyBtnNames[ i ][ JOYBTNINDEX ] = 0;

      for( j = 0; j < NUMCONTROLNAMES; j++ )
         {
         if ( button == controlorder[ j ] )
            {
            buttonjoy[ i ] = button;
            num = j;
            break;
            }
         }

      strcpy( &JoyBtnNames[ i ][ JOYBTNINDEX ], ControlNames[ num ] );
      }

   JoyBtnMenu[ 0 ].active = CP_Active;
   JoyBtnMenu[ 1 ].active = CP_Active;
   JoyBtnMenu[ 4 ].active = CP_Active;
   JoyBtnMenu[ 5 ].active = CP_Active;

   if ( joypadenabled )
      {
      active = CP_Active;
      }
   else
      {
      active = CP_Inactive;
      }

   JoyBtnMenu[ 2 ].active = active;
   JoyBtnMenu[ 3 ].active = active;
   JoyBtnMenu[ 6 ].active = active;
   JoyBtnMenu[ 7 ].active = active;

   if ( JoyBtnMenu[ JoyBtnItems.curpos ].active == CP_Inactive )
      {
      MN_GetCursorLocation( &JoyBtnItems, &JoyBtnMenu[ 0 ] );
      }
   else
      {
      JoyBtnMenu[ JoyBtnItems.curpos ].active = CP_CursorLocation;
      }

   DrawMenu( &JoyBtnItems, &JoyBtnMenu[ 0 ] );

   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_Joystick ()
//
//****************************************************************************

void CP_Joystick
   (
   void
   )

   {
   int which;

   MenuNum = 1;

   do
      {
      DrawCustomJoystick();
      which = HandleMenu( &JoyBtnItems, &JoyBtnMenu[ 0 ], NULL );
      }
   while( which >= 0 );

   handlewhich = OUTOFRANGE;

   DrawCustomMenu();
   }


//******************************************************************************
//
// PRINT A MESSAGE IN A WINDOW
//
//******************************************************************************

void Message (char *string)
{
   int   h = 0,
         w = 0,
         mw = 0,
         i;
   byte *shape;

	shape = W_CacheLumpNum (W_GetNumForName ("newfnt1"), PU_CACHE, Cvt_font_t, 1);
	newfont1 = (font_t *)shape;
   CurrentFont = newfont1;
   h = CurrentFont->height;

   for (i = 0; i < (int)strlen (string); i++)
      if (string[i] == '\n')
      {
         if (w > mw)
            mw = w;
         w = 0;
         h += CurrentFont->height;
      }
      else
         w += CurrentFont->width[string[i]-31];

   if ((w + 10) > mw)
      mw = w+10;

   PrintY = 78 - (h / 2);
   PrintX = WindowX = 143 - (mw / 2);
   WindowW = mw;

   EraseMenuBufRegion (WindowX-5, PrintY-5, (mw+14)&0xFFFC, h+10);
	DrawSTMenuBuf (WindowX-5, PrintY-5, (mw+14)&0xFFFC, h+10, true);

   MenuBufCPrint (string);
   RefreshMenuBuf (0);
}



//******************************************************************************
//
// DRAW NEW GAME MENU
//
//******************************************************************************

void DrawNewGame (void)
{
   MenuNum = 5;

   SetAlternateMenuBuf ();
   ClearMenuBuf ();
   SetMenuTitle ("Choose Difficulty");
   DrawMenu (&TufItems, &TufMenu[ToughMenuNum][0]);
   DrawNewGameDiff (TufItems.curpos);
   DisplayInfo (0);
   FlipMenuBuf();

}

//******************************************************************************
//
// DRAW NEW GAME GRAPHIC
//
//******************************************************************************

int newgameY[7] = {19, 0, 42, 0, 74, 0, 93};
char *DifficultyStrings[ 4 ] =
   {
   "Easy", "Medium", "Hard", "Crezzy Man"
   };

void DrawNewGameDiff
   (
   int w
   )

   {
   int x;

   switch ( w )
      {
      case 0:
         x = 0;
         break;

      case 2:
         x = 1;
         break;

      case 4:
         x = 2;
         break;

      case 6:
         x = 3;
         break;
      }

   EraseMenuBufRegion( 25, 18, 52, 125 );
   DrawMenuBufPic( 25, newgameY[ w ], W_GetNumForName( "NEWG1" ) +
      ( ToughMenuNum * 4 ) + x );
   EraseMenuBufRegion( 25, 149, 64, 8 );
//   DrawMenuBufPic (25, 149, W_GetNumForName( "O_EASY" ) + x );

   CurrentFont = tinyfont;
   DrawMenuBufPropString( 25, 149, DifficultyStrings[ x ] );
   }


//******************************************************************************
//
// DRAW THE LOAD/SAVE SCREEN
//
//******************************************************************************

void DrawLoadSaveScreen (int loadsave)
{
   int i;
   byte *shape;

	shape = W_CacheLumpNum (W_GetNumForName ("newfnt1"), PU_CACHE, Cvt_font_t, 1);
	newfont1 = (font_t *)shape;
   CurrentFont = newfont1;

   if (numdone || (!ingame) || (!inmenu))
      SetAlternateMenuBuf();

   ClearMenuBuf();
   if (loadsave)
      {
      SetMenuTitle ("Save Game");
      }
   else
      {
      SetMenuTitle ("Load Game");
      }

   for (i = 0; i < NUMSAVEGAMES; i++)
      PrintLSEntry (i);
   DrawMenuBufItem (LSItems.x, ((LSItems.curpos*9)+(LSItems.y)),
      W_GetNumForName( SmallCursor ) + CursorFrame[ CursorNum ] );
   DisplayInfo (7);
   if ((!numdone) && ingame && inmenu)
      RefreshMenuBuf (0);
   else
		FlipMenuBuf();

   WaitKeyUp ();

   numdone++;
}

//******************************************************************************
//
// DRAW THE LOAD/SAVE SCREEN - no flip
//
//******************************************************************************

void DrawLoadSaveScreenAlt (int loadsave)
{
   int i;
   byte *shape;

	shape = W_CacheLumpNum (W_GetNumForName ("newfnt1"), PU_CACHE, Cvt_font_t, 1);
	newfont1 = (font_t *)shape;
   CurrentFont = newfont1;

   ClearMenuBuf();
   if (loadsave)
      {
      SetMenuTitle ("Save Game");
      }
   else
      {
      SetMenuTitle ("Load Game");
      }

   for (i = 0; i < NUMSAVEGAMES; i++)
      PrintLSEntry (i);
   DrawMenuBufItem (LSItems.x, ((LSItems.curpos*9)+(LSItems.y)),
      W_GetNumForName( SmallCursor ) + CursorFrame[ CursorNum ] );
   DisplayInfo (7);
   RefreshMenuBuf (0);

   WaitKeyUp ();

   numdone++;
}


//******************************************************************************
//
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
//
//******************************************************************************

void PrintLSEntry (int w)
{

   DrawSTMenuBuf (LSM_X+LSItems.indent, LSM_Y+1+w*9, 80, 7, false);

   PrintX = LSM_X+LSItems.indent+2;
   PrintY = LSM_Y+(w*9)+2;

   CurrentFont = tinyfont;

   if (SaveGamesAvail[w])
      DrawMenuBufPropString (PrintX, PrintY, SaveGameNames[w]);
   else
      DrawMenuBufPropString (PrintX, PrintY, "     - � -");
}



//******************************************************************************
//
// CALIBRATE JOYSTICK
//
//******************************************************************************
int CalibrateJoystick
   (
   void
   )

   {
   #define CALX   45
   #define CALY   22

   word xmax, ymax, xmin, ymin, jb;
   int  checkbits;
   int  status;
   boolean done;

   if ( joypadenabled )
      {
      // Gravis GamePad : Check all buttons
      checkbits = ( 1 << 0 ) + ( 1 << 1 ) + ( 1 << 2 ) + ( 1 << 3 );
      }
   else if ( joystickport )
      {
      // Joystick port 2 : check only buttons 2 and 3
      checkbits = ( 1 << 2 ) + ( 1 << 3 );
      }
   else
      {
      // Joystick port 1 : check only buttons 0 and 1
      checkbits = ( 1 << 0 ) + ( 1 << 1 );
      }

   status = 0;
   done = false;
   while( !done )
      {
      SetAlternateMenuBuf();
      ClearMenuBuf();
      SetMenuTitle ("Calibrate Joystick");
      //DrawMenuBufItem( CALX, CALY, W_GetNumForName( "joystk2" ) );
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 50;

      newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1 );
      CurrentFont = newfont1;
      MenuBufCPrint( "MOVE JOYSTICK TO\nUPPER LEFT AND\nPRESS A BUTTON." );

      DisplayInfo( 2 );
      FlipMenuBuf();

      do
         {
         RefreshMenuBuf( 0 );
         jb = IN_JoyButtons();
         IN_UpdateKeyboard();

         if ( Keyboard[ sc_Escape ] )
            {
            return( 0 );
            }
         }
      while( !( jb & checkbits ) );

      IN_GetJoyAbs( joystickport, &xmin, &ymin );
      MN_PlayMenuSnd( SD_SELECTSND );

      while( IN_JoyButtons() & checkbits )
         {
         IN_UpdateKeyboard();

         if ( Keyboard[ sc_Escape ] )
            {
            return( 0 );
            }
         }

      ClearMenuBuf();
//      DrawMenuBufItem( CALX, CALY, W_GetNumForName( "joystk1" ) );
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 50;

      newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1 );
      CurrentFont = newfont1;
      MenuBufCPrint( "MOVE JOYSTICK TO\nLOWER RIGHT AND\nPRESS A BUTTON." );

      DisplayInfo( 2 );

      do
         {
         RefreshMenuBuf( 0 );
         jb = IN_JoyButtons();
         IN_UpdateKeyboard();

         if ( Keyboard[ sc_Escape ] )
            {
            return( 0 );
            }
         }
      while( !( jb & checkbits ) );

      IN_GetJoyAbs( joystickport, &xmax, &ymax );
      MN_PlayMenuSnd( SD_SELECTSND );

      while( IN_JoyButtons() & checkbits )
         {
         IN_UpdateKeyboard();

         if ( Keyboard[ sc_Escape ] )
            {
            return( 0 );
            }
         }

      //
      // ASSIGN ACTUAL VALUES HERE
      //
      if ( ( xmin < xmax ) && ( ymin < ymax ) )
         {
         IN_SetupJoy( joystickport, xmin, xmax, ymin, ymax );
         joyxmin = xmin;
         joyxmax = xmax;
         joyymin = ymin;
         joyymax = ymax;

         status = 1;
         done = true;
         }
      else
         {
         CP_ErrorMsg( "Joystick Error",
            "Calibration failed.  The joystick must be moved "
            "to the upper-left first and then the lower-right.",
            mn_smallfont );
         }
      }

   return( status );
   }


//******************************************************************************
//
// ADJUST MOUSE SENSITIVITY
//
//******************************************************************************

void MouseSensitivity
   (
   void
   )

   {
   SliderMenu( &mouseadjustment, 11, 0, 21, 81, 240, 1, "block1", NULL,
      "Mouse Sensitivity", "Slow", "Fast" );
   }

//******************************************************************************
//
// ADJUST MOUSE AND JOYSTICK THRESHOLD
//
//******************************************************************************

void DoThreshold
   (
   void
   )

   {
   SliderMenu (&threshold, 15, 1, 44, 81, 194, 1, "block2", NULL,
      "Adjust Threshold", "Small", "Large" );
   }

//******************************************************************************
//
// DRAW CONTROL MENU SCREEN
//
//******************************************************************************

void DrawCtlScreen (void)
{
   MenuNum = 3;

   if (numdone || (!ingame) || (!inmenu))
      SetAlternateMenuBuf();

   ClearMenuBuf();
   SetMenuTitle ("Options");


   DrawCtlButtons ();

   DisplayInfo (0);
   DrawMenu (&CtlItems, &CtlMenu[0]);
   DrawMenuBufItem (CtlItems.x, ((CtlItems.curpos*14)+(CtlItems.y-2)),
                  W_GetNumForName( LargeCursor ) + CursorFrame[ CursorNum ] );

   if (ingame && inmenu && (!numdone))
      RefreshMenuBuf (0);
   else
      FlipMenuBuf();

	numdone++;
}


//******************************************************************************
//
// DrawCtlButtons ()
//
//******************************************************************************

void DrawCtlButtons (void)
{
   int i,
       x,
       y;
   static boolean first = true;
   int button_on;
   int button_off;

   button_on  = W_GetNumForName ("snd_on");
   button_off = W_GetNumForName ("snd_off");

   WindowX = 0;
   WindowW = 320;

   if (first)
   {
      if (JoysPresent[0] || JoysPresent[1])
         {
         CtlMenu[JOYENABLE].active = CP_Active;
         CtlMenu[USEPORT2].active  = CP_Active;
         CtlMenu[PADENABLE].active = CP_Active;
         CtlMenu[THRESSENS].active = CP_Active;
         }
      else
      {
         joystickenabled = 0;
         joypadenabled   = 0;
         joystickport    = 0;
      }

      if (MousePresent)
      {
			CtlMenu[THRESSENS].active = CP_Active;
         CtlMenu[MOUSESENS].active = CP_Active;
         CtlMenu[MOUSEENABLE].active = CP_Active;
      }
      else
      {
         CtlMenu[0].active = CP_Inactive;
         mouseenabled = 0;
      }

      if (SpaceBallPresent)
         CtlMenu[SPACEBALLENABLE].active = CP_Active;

      if (CybermanPresent)
         CtlMenu[CYBERMANENABLE].active = CP_Active;

      for (x = 0; x < CtlItems.amount; x++)
      {
         if (CtlMenu[x].active)
         {
            CtlMenu[x].active = CP_CursorLocation;
            break;
         }
      }
      first = false;
   }

   x = CTL_X+CtlItems.indent-18;
   y = MENU_Y-1;

   if (mouseenabled)
      DrawMenuBufItem (x, y, button_on);
   else
   {
      EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem  (x, y, button_off);
   }

   y += 14;
   if (joystickenabled)
      DrawMenuBufItem (x, y, button_on);
   else
   {
		EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem (x, y, button_off);
   }

   y += 14;
   if (joystickport)
      DrawMenuBufItem (x, y, button_on);
   else
   {
      EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem (x, y, button_off);
   }

   y += 14;
   if (joypadenabled)
      DrawMenuBufItem (x, y, button_on);
   else
   {
      EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem (x, y, button_off);
   }

   y += 14;
   if (spaceballenabled)
      DrawMenuBufItem (x, y, button_on);
   else
   {
      EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem (x, y, button_off);
   }

   y += 14;
   if (cybermanenabled)
      DrawMenuBufItem (x, y, button_on);
   else
   {
      EraseMenuBufRegion (x, y, 16, 16);
      DrawMenuBufItem (x, y, button_off);
   }


   if ((CtlItems.curpos < 0) || (!CtlMenu[CtlItems.curpos].active))
      for (i = 0; i < CtlItems.amount; i++)
			if (CtlMenu[i].active)
         {
            CtlItems.curpos = i;
            break;
         }

}

//******************************************************************************
//
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
//
//******************************************************************************

void WaitKeyUp (void)
{
   ControlInfo ci;

   IN_IgnoreMouseButtons();
   ReadAnyControl (&ci);

   while (ci.button0 || ci.button1 || ci.button2 || ci.button3 ||
          Keyboard[sc_Space] || Keyboard[sc_Enter] || Keyboard[sc_Escape])
   {
      ReadAnyControl (&ci);
      RefreshMenuBuf (0);
      if (Keystate[sc_CapsLock] && Keystate[sc_Q])
         Error("Stuck in WaitKeyUp\n");
   }
}

#define PMOUSE    3
#define SMOUSE    4

//******************************************************************************
//
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//
//******************************************************************************

void ReadAnyControl (ControlInfo *ci)
{

#if PLATFORM_DOS
   union REGS inregs;
   union REGS outregs;
#endif

   int mouseactive = 0;
   word buttons = 0;
//   struct Spw_IntPacket packet;


   IN_UpdateKeyboard ();  /* implies IN_PumpEvents() ... */
   IN_ReadControl (0, ci);

   if (MousePresent && mouseenabled)
   {
      int mousey,
          mousex;

#if USE_SDL
      INL_GetMouseDelta(&mousex, &mousey);
      if (mousex >= SENSITIVE)
      {
         ci->dir = dir_East;
         mouseactive = 1;
      }
      else if (mousex <= -SENSITIVE)
      {
         ci->dir = dir_West;
         mouseactive = 1;
      }

      if (mousey >= SENSITIVE)
      {
         ci->dir = dir_South;
         mouseactive = 1;
      }
      else if (mousey <= -SENSITIVE)
      {
         ci->dir = dir_North;
         mouseactive = 1;
      }

#elif PLATFORM_DOS
		// READ MOUSE MOTION COUNTERS
      // RETURN DIRECTION
      // HOME MOUSE
      // CHECK MOUSE BUTTONS

      inregs.w.ax = PMOUSE;
      int386 (MouseInt, &inregs, &outregs);

      mousex = outregs.w.cx;
      mousey = outregs.w.dx;


      if (mousey < (CENTER-SENSITIVE))
      {
         ci->dir = dir_North;

         inregs.w.cx = CENTER;
         inregs.w.dx = CENTER;

         inregs.w.ax = SMOUSE;
         int386 (MouseInt, &inregs, &outregs);

         mouseactive = 1;

      }
      else
      if (mousey > (CENTER+SENSITIVE))
      {
         ci->dir = dir_South;

         inregs.w.cx = CENTER;
         inregs.w.dx = CENTER;

         inregs.w.ax = SMOUSE;
         int386 (MouseInt, &inregs, &outregs);

         mouseactive = 1;
      }

      if (mousex < (CENTER-SENSITIVE))
      {
         ci->dir = dir_West;

			inregs.w.cx = CENTER;
         inregs.w.dx = CENTER;

         inregs.w.ax = SMOUSE;
         int386 (MouseInt, &inregs, &outregs);


         mouseactive = 1;
      }
      else
      if (mousex > (CENTER+SENSITIVE))
      {
         ci->dir = dir_East;

         inregs.w.cx = CENTER;
         inregs.w.dx = CENTER;

         inregs.w.ax = SMOUSE;
         int386 (MouseInt, &inregs, &outregs);

         mouseactive = 1;
      }
#else
#error please define your platform.  /* or maybe just nuke the DOS section? */
#endif

      buttons = IN_GetMouseButtons();
      if ( buttons )
         {
         ci->button0 = buttons & 1;
         ci->button1 = buttons & 2;
         ci->button2 = buttons & 4;
         ci->button3 = false;
         mouseactive = 1;
         }
      }

   if (joystickenabled && !mouseactive)
   {
      int jx,jy,jb;


      INL_GetJoyDelta (joystickport, &jx, &jy);

      if (jy<-SENSITIVE)
         ci->dir=dir_North;
      else
		if (jy>SENSITIVE)
         ci->dir=dir_South;

      if (jx<-SENSITIVE)
         ci->dir=dir_West;
      else
      if (jx>SENSITIVE)
         ci->dir=dir_East;

      jb = IN_JoyButtons();
      if (jb)
      {
         ci->button0=jb&1;
         ci->button1=jb&2;
         if (joypadenabled)
         {
            ci->button2=jb&4;
            ci->button3=jb&8;
         }
         else
            ci->button2=ci->button3=false;
      }
   }


#if 0
   if (SpaceBallPresent && spaceballenabled)
   {
		SP_Get(&packet);

      if (packet.button)
      {
         if (packet.button & SP_BTN_1)
            ci->button0 = true;

         if (packet.button & SP_BTN_2)
            ci->button1 = true;
      }

		if (packet.ty >  MENU_AMT)
         ci->dir = dir_North;
      else
         if (packet.ty < -MENU_AMT)
            ci->dir = dir_South;

		if (packet.tx < (-MENU_AMT* 6))
         ci->dir = dir_West;
      else
         if (packet.tx > (MENU_AMT * 6))
            ci->dir = dir_East;
   }
#endif
}


//******************************************************************************
//
// IN_GetScanName () - Returns a string containing the name of the
//                     specified scan code
//
//******************************************************************************

byte * IN_GetScanName (ScanCode scan)
{
   byte     **p;
   ScanCode *s;

   for (s = ExtScanCodes, p = ExtScanNames; *s; p++, s++)
      if (*s == scan)
         return (*p);

   return(ScanNames[scan]);
}


//******************************************************************************
//
// DisplayInfo ()
//
//******************************************************************************

void DisplayInfo (int which)
{
   patch_t *p;
   int x;
   int num;

   num = W_GetNumForName ( "info1" ) + which;
   p = (patch_t *) W_CacheLumpNum (num, PU_CACHE, Cvt_patch_t, 1);

   x = (288 - p->width) >> 1;

   DrawMenuBufItem (x, 149, num);
}


//******************************************************************************
//
// DrawSTMenuBuf()
//
//******************************************************************************

void DrawSTMenuBuf (int x, int y, int w, int h, boolean up)
{
   if (!up)
   {
      DrawTMenuBufHLine (x,   y,   w+1, false);
      DrawTMenuBufVLine (x,   y+1, h-1, false);
      DrawTMenuBufHLine (x,   y+h, w+1, true);
      DrawTMenuBufVLine (x+w, y+1, h-1, true);
   }
   else
   {
      DrawTMenuBufHLine (x,   y,   w+1, true);
      DrawTMenuBufVLine (x,   y+1, h-1, true);
      DrawTMenuBufHLine (x,   y+h, w+1, false);
      DrawTMenuBufVLine (x+w, y+1, h-1, false);
   }
}


//****************************************************************************
//
// DoMainMenu ()
//
//****************************************************************************

void DoMainMenu (void)
{
		EnableScreenStretch();//bna++ shut on streech mode
   SetAlternateMenuBuf();
   ClearMenuBuf();
   DrawMainMenu();
	DrawMenuBufItem (MainItems.x,  ((MainItems.curpos*14)+(MainItems.y-2)),
                  W_GetNumForName( LargeCursor ) + CursorFrame[ CursorNum ] );
   FlipMenuBuf();
}


//****************************************************************************
//
// DrawCustomMenu ()
//
//****************************************************************************

void DrawCustomMenu (void)
{
   MenuNum = 3;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Customize Menu");
   MN_GetCursorLocation( &CustomItems, &CustomMenu[ 0 ] );
   DrawMenu (&CustomItems, &CustomMenu[0]);
   DrawMenuBufItem (CustomItems.x, ((CustomItems.curpos*14)+(CustomItems.y-2)),
                  W_GetNumForName( LargeCursor ) + CursorFrame[ CursorNum ] );
   DisplayInfo (0);
   FlipMenuBuf();
}



//****************************************************************************
//
// DrawCustomKeyboard ()
//
//****************************************************************************
void DrawCustomKeyboard (void)
{
   int i;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Customize Keyboard");

   for( i = 0; i < NormalKeyItems.amount; i++ )
      {
      strcpy( &NormalKeyNames[ i ][ KEYNAMEINDEX ],
         IN_GetScanName( buttonscan[ (unsigned int)order[ i ] ] ) );
      }

   MN_GetCursorLocation( &NormalKeyItems, &NormalKeyMenu[ 0 ] );
   DrawMenu( &NormalKeyItems, &NormalKeyMenu[ 0 ] );

   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// MusicVolume ()
//
//****************************************************************************

void MusicVolume
   (
   void
   )

   {
#ifdef DOS
   extern boolean SOUNDSETUP;
#endif

   SliderMenu( &MUvolume, 254, 0, 33, 81, 225, 8, "block3", MUSIC_SetVolume,
      "Music Volume", "Low", "High" );

#ifdef DOS
   if ( SOUNDSETUP )
      {
      DrawSoundSetupMainMenu();
      }
   else
      {
#endif
      DrawControlMenu();
#ifdef DOS
      }
#endif
   }


//****************************************************************************
//
// FXVolume ()
//
//****************************************************************************

void FXVolume
   (
   void
   )

   {
#ifdef DOS
   extern boolean SOUNDSETUP;
#endif
   int oldvolume;

   oldvolume = FXvolume;

   SliderMenu( &FXvolume, 254, 0, 33, 81, 225, 8, "block3", FX_SetVolume,
      "Sound Volume", "Low", "High" );

#ifdef DOS
   if ( SOUNDSETUP )
      {
      DrawSoundSetupMainMenu();
      }
   else
      {
#endif
      DrawControlMenu();
#ifdef DOS
      }
#endif
   }



//****************************************************************************
//
// DrawPlayerMenu ()
//
//****************************************************************************

void DrawPlayerMenu (void)
{
   MenuNum = 5;


   MN_MakeActive( &PlayerItems, &PlayerMenu[0], DefaultPlayerCharacter );

#if ( SHAREWARE == 1 )
   PlayerMenu[ 1 ].active = CP_SemiActive; // Thi Barrett
   PlayerMenu[ 2 ].active = CP_SemiActive; // Doug Wendt
   PlayerMenu[ 3 ].active = CP_SemiActive; // Lorelei Ni
   PlayerMenu[ 4 ].active = CP_SemiActive; // Ian Paul Freeley
#endif

   if (numdone || (!ingame) || (!inmenu))
      SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Choose Player");

   DrawMenu (&PlayerItems, &PlayerMenu[0]);
   DrawNewPlayerDiff (PlayerItems.curpos);

   DisplayInfo (0);


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   if (ingame && inmenu && (!numdone))
      RefreshMenuBuf (0);
   else
      FlipMenuBuf();
}

//******************************************************************************
//
// DRAW NEW PLAYER GRAPHIC
//
//******************************************************************************

int newplayerY[5] = {28, 42, 56, 70, 84};

void DrawNewPlayerDiff (int w)
{
   EraseMenuBufRegion (25, 18, 52, 125);
   DrawMenuBufPic (25, newplayerY[w], W_GetNumForName( "PLAYER1" ) + w );
}


//******************************************************************************
//
// MenuFixup ()
//
//******************************************************************************
void MenuFixup
   (
   void
   )

   {
   MainMenu[ viewscores ].texture[ 6 ] = '1';
   MainMenu[ viewscores ].texture[ 7 ] = '0';
   MainMenu[ viewscores ].texture[ 8 ] = '\0';
   MainMenu[ viewscores ].routine      = ( void * )CP_EndGame;
   MainMenu[ viewscores ].letter       = 'E';
   strcpy (MainMenuNames[ viewscores ] , "END GAME");
   MainMenu[ savegame ].active         = CP_Active;
   ingame = true;
   }

//******************************************************************************
//
// GetEpisode ()
//
//******************************************************************************

void GetEpisode (int level)
{
   if (level < 8)
      gamestate.episode = 1;
   else
      if (level < 16)
         gamestate.episode = 2;
      else
         if (level < 24)
            gamestate.episode = 3;
         else
            gamestate.episode = 4;
}


//****************************************************************************
//
// DrawControlMenu ()
//
//****************************************************************************

void DrawControlMenu (void)
{
   MenuNum = 1;

   if (numdone || (!ingame) || (!inmenu))
      SetAlternateMenuBuf();

   ClearMenuBuf();
   SetMenuTitle ("Options");

   MN_GetCursorLocation( &ControlMItems, &ControlMMenu[ 0 ] );
   DrawMenu (&ControlMItems, &ControlMMenu[0]);
   DisplayInfo (0);

   if (ingame && inmenu && (!numdone))
      RefreshMenuBuf (0);
   else
      FlipMenuBuf();

   numdone ++;
}

//****************************************************************************
//
//  CP_ControlMenu ()
//
//****************************************************************************

void CP_ControlMenu (void)
{
   int which;

   DrawControlMenu();

   do
   {
      which = HandleMenu (&ControlMItems, &ControlMMenu[0], NULL);

   } while (which >= 0);

   handlewhich = OUTOFRANGE;
}



//****************************************************************************
//
// DrawOptionsMenu ()
//
//****************************************************************************

void DrawOptionsMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("User Options");

   MN_GetCursorLocation( &OptionsItems, &OptionsMenu[ 0 ] );
   DrawMenu (&OptionsItems, &OptionsMenu[0]);
   DrawOptionsButtons ();
   DisplayInfo (0);
   FlipMenuBuf();
}
//****************************************************************************
//
// DrawExtOptionsMenu ()  () bna added
//
//****************************************************************************

void DrawExtOptionsMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Extended User Options");

   MN_GetCursorLocation( &ExtOptionsItems, &ExtOptionsMenu[ 0 ] );
   DrawMenu (&ExtOptionsItems, &ExtOptionsMenu[0]);
   DrawExtOptionsButtons ();
   DisplayInfo (0);

   FlipMenuBuf();
}
extern long inverse_mouse;
extern boolean usemouselook;
extern int iG_aimCross;
extern boolean usejump;
extern boolean sdl_fullscreen;

void CP_ExtOptionsMenu (void)
{
   int which;

   DrawExtOptionsMenu();

   do
   {
      which = HandleMenu (&ExtOptionsItems, &ExtOptionsMenu[0], NULL);

      switch (which)
      {
         case 0: usemouselook  ^= 1; DrawExtOptionsButtons (); break;
         case 1: if (inverse_mouse == 1){
					inverse_mouse = -1;
				 }else{
					inverse_mouse = 1;
				 }
				 DrawExtOptionsButtons (); 
				 break;
         case 2: iG_aimCross   ^= 1; DrawExtOptionsButtons (); break;
         case 3: usejump       ^= 1; DrawExtOptionsButtons (); break;
         case 4:
            if (SDL_WM_ToggleFullScreen(SDL_GetVideoSurface()))
            {
               sdl_fullscreen ^= 1;
               DrawExtOptionsButtons ();
            }
            break;
      }

	} while (which >= 0);

   DrawControlMenu();
}
void DrawExtOptionsButtons (void)
{
   int i,
       on;
   int button_on;
   int button_off;

   button_on  = W_GetNumForName ("snd_on");
   button_off = W_GetNumForName ("snd_off");

   for (i = 0; i < ExtOptionsItems.amount; i++)
      if (ExtOptionsMenu[i].active != CP_Active3)
      {
         //
         // DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
         //

         on = 0;

         switch (i)
         {
            case 0: if (usemouselook  == 1) on = 1; break;
            case 1: if (inverse_mouse == -1)on = 1; break;
            case 2: if (iG_aimCross   == 1) on = 1; break;
            case 3: if (usejump       == 1) on = 1; break;
            case 4: if (sdl_fullscreen== 1) on = 1; break;
         }

         if (on)
            DrawMenuBufItem (20+22, ExtOptionsItems.y+i*14-1, button_on);
         else
            DrawMenuBufItem (20+22, ExtOptionsItems.y+i*14-1, button_off);
		}
}

//****************************************************************************
//
// CP_OptionsMenu ()
//
//****************************************************************************

void CP_OptionsMenu (void)
{
   int which;

   DrawOptionsMenu();

   do
   {
      which = HandleMenu (&OptionsItems, &OptionsMenu[0], NULL);

      switch (which)
      {
         case 0: AutoDetailOn  ^= 1; DrawOptionsButtons (); break;
         case 1: fulllight     ^= 1; DrawOptionsButtons (); break;
         case 2: BobbinOn      ^= 1; DrawOptionsButtons (); break;
         case 3: fandc         ^= 1; DrawOptionsButtons (); break;
      }

	} while (which >= 0);

   DrawControlMenu();
}

//****************************************************************************
//
// DrawOptionsButtons
//
//****************************************************************************

void DrawOptionsButtons (void)
{
   int i,
       on;
   int button_on;
   int button_off;

   button_on  = W_GetNumForName ("snd_on");
   button_off = W_GetNumForName ("snd_off");

   for (i = 0; i < OptionsItems.amount-5; i++)
      if (OptionsMenu[i].active != CP_Active3)
      {
         //
         // DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
         //

         on = 0;

         switch (i)
         {
            case 0: if (AutoDetailOn  == 1) on = 1; break;
            case 1: if (fulllight     == 0) on = 1; break;
            case 2: if (BobbinOn      == 1) on = 1; break;
            case 3: if (fandc         == 1) on = 1; break;
         }

         if (on)
            DrawMenuBufItem (20+22, OptionsItems.y+i*14-1, button_on);
         else
            DrawMenuBufItem (20+22, OptionsItems.y+i*14-1, button_off);
		}
}


//****************************************************************************
//
// CP_DoubleClickSpeed()
//
//****************************************************************************

void CP_DoubleClickSpeed
   (
   void
   )

   {
   int temp;

   temp = 50 - ( DoubleClickSpeed - 5 );
   SliderMenu( &temp, 50, 5, 31, 81, 225, 3, "block1", NULL,
      "Double-Click Speed", "Slow", "Fast" );
   DoubleClickSpeed = 50 - ( temp - 5 );

   handlewhich = 100;
   DrawOptionsMenu();
   }

//****************************************************************************
//
// MenuFlipSpeed ()
//
//****************************************************************************

void MenuFlipSpeed
   (
   void
   )

   {
   int temp;

   temp = 50 - ( Menuflipspeed - 5 );

   SliderMenu( &temp, 50, 5, 31, 81, 225, 3, "block1", NULL,
      "Menu Flip Speed", "Slow", "Fast" );

   Menuflipspeed = 50 - ( temp - 5 );

   DrawOptionsMenu ();
   handlewhich = 10;
   }


//****************************************************************************
//
// DrawDetailMenu ()
//
//****************************************************************************

void DrawDetailMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Detail Menu");

   MN_DrawButtons (&DetailItems, &DetailMenu[0], DetailLevel, OptionNums);
   MN_GetCursorLocation( &DetailItems, &DetailMenu[ 0 ] );
   DrawMenu (&DetailItems, &DetailMenu[0]);
   DisplayInfo (0);
   FlipMenuBuf();
}

//****************************************************************************
//
// CP_DetailMenu ()
//
//****************************************************************************

void CP_DetailMenu (void)
{
   int which;

   DrawDetailMenu();

   do
   {
      which = HandleMenu (&DetailItems, &DetailMenu[0], NULL);

      switch (which)
      {
         case 0:
            DetailLevel = 0;
            MN_DrawButtons (&DetailItems, &DetailMenu[0], DetailLevel, OptionNums);
         break;

         case 1:
            DetailLevel = 1;
            MN_DrawButtons (&DetailItems, &DetailMenu[0], DetailLevel, OptionNums);
         break;

         case 2:
            DetailLevel = 2;
            MN_DrawButtons (&DetailItems, &DetailMenu[0], DetailLevel, OptionNums);
         break;
      }

   } while (which >= 0);

   handlewhich = 10;
   DrawOptionsMenu();
}


//****************************************************************************
//
// DrawBattleMenu ()
//
//****************************************************************************

void DrawBattleMenu (void)
{
   MenuNum = 1;

   if (numdone || (!ingame) || (!inmenu))
      SetAlternateMenuBuf();

   ClearMenuBuf();
   SetMenuTitle ("Battle Menu");

   DrawBattleModeName( gamestate.battlemode );

   MN_GetCursorLocation( &BattleItems, &BattleMenu[ 0 ] );
   DrawMenu (&BattleItems, &BattleMenu[0]);
   DisplayInfo (0);

   BATTLE_SetOptions( &BATTLE_Options[ gamestate.battlemode ] );
   ShowBattleOptions( true, MENU_X, MENU_Y + 49 );

   if (ingame && inmenu && (!numdone))
      RefreshMenuBuf (0);
   else
      FlipMenuBuf();
}


//****************************************************************************
//
// BattleGamePlayerSetup()
//
//****************************************************************************

void BattleGamePlayerSetup( void )
   {
   int status;
   int pos;

   pos = 1;
   if ( consoleplayer == 0 )
      {
      pos = 0;
      }

   while( 1 )
      {
      switch( pos )
         {
         case 0 :
            // Select level to play on
            status = CP_LevelSelectionMenu ();
            if ( status >= 0  )
               {
               gamestate.mapon=status;
               pos = 1;
               }
            else
               {
               DrawBattleMenu();
               return;
               }
            break;

         case 1 :
            // Select CodeName
            status = CP_EnterCodeNameMenu();
            pos = 2;
            if ( !status )
               {
               if ( consoleplayer == 0 )
                  {
                  pos = 0;
                  }
               else
                  {
                  return;
                  }
               }
            break;

         case 2 :
            // Select character
            status = CP_PlayerSelection ();
            pos = 1;
            if ( status )
               {
               pos = 3;
               }
            break;

         case 3 :
            // Select color/team
            status = CP_ColorSelection();
            pos = 2;
            if ( status )
               {
               StartGame   = true;
			      DisableScreenStretch();
               handlewhich = -2;
               playstate   = ex_resetgame;
               BATTLEMODE  = true;
               // Show please wait
               CP_ModemGameMessage( consoleplayer );
               return;
               }
            break;
         }
      }
   }


//****************************************************************************
//
// BattleNoTeams()
//
//****************************************************************************

void BattleNoTeams( void )
   {
   BattleGamePlayerSetup();
   if ( StartGame )
      {
      gamestate.teamplay = false;
      }
   }


//****************************************************************************
//
// BattleTeams()
//
//****************************************************************************

void BattleTeams( void )
   {
   BattleGamePlayerSetup();
   if ( StartGame )
      {
      gamestate.teamplay = true;
      }
   }


//****************************************************************************
//
// CP_BattleMenu ()
//
//****************************************************************************

void CP_BattleMenu (void)
{
   int which;

   gamestate.battlemode = handlewhich + battle_Normal;

   BattleMenu[0].active = CP_Active;
   BattleMenu[1].active = CP_Active;


   // Tag can't be played in team mode
   // Also, can't play teams if only 1 person is playing
   if ( ( gamestate.battlemode == battle_Tag ) ||
      ( numplayers < 2 ) )
      {
      BattleMenu[1].active = CP_Inactive;
      if ( BattleItems.curpos == 1 )
         {
         BattleItems.curpos = 0;
         }
      }

   // Capture the Triad can only be played in team mode
   if ( gamestate.battlemode == battle_CaptureTheTriad )
      {
      BattleMenu[0].active = CP_Inactive;
      if ( BattleItems.curpos == 0 )
         {
         BattleItems.curpos = 1;
         }
      }

   BattleMenu[ BattleItems.curpos ].active = CP_CursorLocation;
   DrawBattleMenu();

   do
      {
		which = HandleMenu (&BattleItems, &BattleMenu[0], NULL);
      }
   while (which >= 0);

   if ( which == -1 )
      {
      DrawBattleModes ();
      handlewhich = OUTOFRANGE;
      }
   }


//****************************************************************************
//
// MN_PlayMenuSnd ()
//
//****************************************************************************

extern boolean dopefish;
void MN_PlayMenuSnd (int which)
{
   if (INFXSETUP || (SD_Started == false))
      return;
#if (SHAREWARE==0)
   if (dopefish==true)
      {
      switch (which)
         {
         case SD_ESCPRESSEDSND:
            which = SD_SOUNDESCSND;
         break;
         case SD_MOVECURSORSND:
            which = SD_SILLYMOVESND;
         break;
         case SD_SELECTSND:
            which = SD_SOUNDSELECTSND;
         break;
         }
      }
#endif
   SD_Play (which);
}


//******************************************************************************
//
// SliderMenu ()
//
//******************************************************************************

boolean SliderMenu
   (
   int *number,
   int upperbound,
   int lowerbound,
   int erasex,
   int erasey,
   int erasew,
   int numadjust,
   char *blockname,
   void ( *routine )( int w ),
   char *title,
   char *left,
   char *right
   )

   {
   ControlInfo ci;
   Direction   lastdir;
   patch_t    *shape;
   boolean     returnval;
   boolean     moved;
   unsigned long scale;
   int         exit;
   int         range;
   int         timer;
   int         width;
   int         height;
   int         blkx;
   int         eraseh;
   int         block;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( title );

   newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1);
   CurrentFont = newfont1;
   PrintX = 25;
   PrintY = 62;
   DrawMenuBufPropString( PrintX, PrintY, left );

   VW_MeasurePropString( right, &width, &height );
   DrawMenuBufPropString( 263 - width, PrintY, right );

   block = W_GetNumForName( blockname );
   shape = ( patch_t * )W_CacheLumpNum( block, PU_CACHE, Cvt_patch_t, 1 );
   blkx  = erasex - shape->leftoffset;
   eraseh =  shape->height;
   scale = ( erasew + shape->leftoffset - shape->width ) << 16;
   range = upperbound - lowerbound;

   DrawSTMenuBuf( erasex - 1, erasey - 1, erasew + 1, eraseh + 1, false );

   DrawMenuBufItem( blkx + ( ( ( ( *number - lowerbound ) *
      scale ) / range ) >> 16 ), erasey, block );

   DisplayInfo( 1 );
   FlipMenuBuf();

   exit  = 0;
   moved = false;
   timer = GetTicCount();
   lastdir = dir_None;

   do
      {
      RefreshMenuBuf( 0 );

      ReadAnyControl( &ci );
      if ( ( ( GetTicCount() - timer ) > 5 ) || ( ci.dir != lastdir ) )
         {
         timer = GetTicCount();

         switch( ci.dir )
            {
            case dir_North:
            case dir_West:
               if ( *number > lowerbound )
                  {
                  *number = *number - numadjust;

                  if ( *number < lowerbound )
                     {
                     *number = lowerbound;
                     }

                  moved = true;
                  }
               break;

            case dir_South:
            case dir_East:
               if ( *number < upperbound )
                  {
                  *number = *number + numadjust;

                  if ( *number > upperbound )
                     {
                     *number = upperbound;
                     }

                  moved = true;
                  }
               break;
	    default:
		;
            }

         lastdir = ci.dir;
         }

      if ( moved )
         {
         moved = false;

         EraseMenuBufRegion( erasex, erasey, erasew, eraseh );

         DrawMenuBufItem( blkx + ( ( ( ( *number - lowerbound ) *
            scale ) / range ) >> 16 ), erasey, block );

         if ( routine )
            {
            routine( *number );
            }

         MN_PlayMenuSnd( SD_MOVECURSORSND );
         }

      if ( ci.button0 || Keyboard[ sc_Space ] || Keyboard[ sc_Enter ] )
         {
         exit = 1;
         }
      else if ( ci.button1 || Keyboard[ sc_Escape ] )
         {
         exit = 2;
         }
      }
   while( !exit );

   if ( exit == 2 )
      {
      MN_PlayMenuSnd( SD_ESCPRESSEDSND );
      returnval = false;
      }
   else
      {
      MN_PlayMenuSnd( SD_SELECTSND );
      returnval = true;
      }

   WaitKeyUp ();
   return( returnval );
   }


//******************************************************************************
//
// DrawF1Help ()
//
//******************************************************************************

void DrawF1Help (void)
{

   VL_DrawPostPic (W_GetNumForName("trilogo"));

   DrawNormalSprite (0, 0, W_GetNumForName("help"));

   VW_UpdateScreen ();
}

//******************************************************************************
//
// CP_F1Help ()
//
//******************************************************************************

void CP_F1Help (void)
{
   LastScan=0;

   DrawF1Help ();

   while (LastScan == 0)
   {
      IN_UpdateKeyboard ();
   }

   LastScan=0;
#if (SHAREWARE==1)
   {
   DrawOrderInfo( 2 );
   while (LastScan == 0)
   {
      IN_UpdateKeyboard ();
   }

   LastScan=0;
   }
#endif
}


//****************************************************************************
//
// CP_ScreenSize()
//
//****************************************************************************

void CP_ScreenSize
   (
   void
   )

   {
   SliderMenu( &viewsize, MAXVIEWSIZES - 1, 0, 33, 81, 225, 1, "block1",
      NULL, "Screen Size", "Small", "Large" );

   handlewhich = 100;
   DrawOptionsMenu();
   }


//****************************************************************************
//
// DrawViolenceMenu ()
//
//****************************************************************************

void DrawViolenceMenu (void)
{
   MenuNum = 1;
   if ( POK )
      {
      memcpy( &VMenu[ 1 ].texture, "mcpass\0", 7 );
      VMenu[ 1 ].letter = 'C';
      strcpy (VMenuNames[ 1 ] , "CHANGE PASSWORD");
      }
   else
      {
      memcpy( &VMenu[ 1 ].texture, "mepass\0", 7 );
      VMenu[ 1 ].letter = 'E';
      strcpy (VMenuNames[ 1 ] , "ENTER PASSWORD");
      }

   if (VMenu[0].active != CP_CursorLocation)
      VMenu[0].active = CP_Active;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Violence Level");

   MN_GetCursorLocation( &VItems, &VMenu[ 0 ] );
   DrawMenu (&VItems, &VMenu[0]);
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_ViolenceMenu ()
//
//****************************************************************************

void CP_ViolenceMenu (void)
{
   int which;

   CurrentFont = smallfont;
   DrawViolenceMenu ();

   do
   {
      which = HandleMenu (&VItems, &VMenu[0], NULL);

   } while (which >= 0);

   handlewhich = 100;
   DrawOptionsMenu();
}


//****************************************************************************
//
// DrawViolenceLevel ()
//
//****************************************************************************

void DrawViolenceLevel (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Change Violence Level");

   MN_DrawButtons (&ViolenceItems, &ViolenceMenu[0], gamestate.violence, OptionNums);
   MN_GetActive (&ViolenceItems, &ViolenceMenu[0], gamestate.violence, OptionNums);

//   DrawMenuBufItem (58, 24, W_GetNumForName ("blood"));
   IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_largefont ],
      PU_CACHE, Cvt_cfont_t, 1 );
   DrawMenuBufIString( 58, 24, "HOW MUCH ", NORMALCOLOR );
   DrawMenuBufIString( PrintX, PrintY, "BLOOD", 51 );
   DrawMenuBufIString( 71, 37, "DO YOU WANT?", NORMALCOLOR );

   DrawMenu (&ViolenceItems, &ViolenceMenu[0]);
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_ViolenceLevel ()
//
//****************************************************************************

void CP_ViolenceLevel (void)
{
   int which;
   char p1[13];
   boolean passok=false;

   if (ingame)
   	{
      CP_ErrorMsg( "Change Violence Level",
         "The current game must be ended to change the Violence Level.",
         mn_largefont );
      }
   else if ( POK )
      {
      memset (p1, 0, 13);

      CurrentFont = smallfont;
      DrawViolenceLevelPWord ();

      if (US_lineinput (PBOXX+2, PBOXY+1, p1, NULL, true, 12, 110, 0))
         {
         //compare user entered to password
         if (StringsNotEqual (p1, pword, StringLength (p1)) == false)
            passok=true;
         else
            {
            CP_ErrorMsg( "Violence Password", "Incorrect Password.",
               mn_largefont );
            }
         }
      }
   else
      passok=true;
   if (passok==true)
      {
      DrawViolenceLevel ();
      do
         {
         which = HandleMenu (&ViolenceItems, &ViolenceMenu[0], NULL);

         if (which >= 0)
            gamestate.violence = which;

         MN_DrawButtons (&ViolenceItems, &ViolenceMenu[0], gamestate.violence, OptionNums);

         } while (which >= 0);

      WriteMenuInfo ();
      }

   handlewhich = 100;
   DrawViolenceMenu();
}



//****************************************************************************
//
// DrawViolenceLevelPWord ();
//
//****************************************************************************

void DrawViolenceLevelPWord
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Violence Password" );

//   CurrentFont = newfont1;
//   DrawMenuBufPropString( PWORDX, PWORDY, "ENTER PASSWORD" );

   IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_largefont ],
      PU_CACHE, Cvt_cfont_t, 1 );
   DrawMenuBufIString( PWORDX, PWORDY, "ENTER PASSWORD", NORMALCOLOR );

   DrawSTMenuBuf( PBOXX, PBOXY, PBOXW, PBOXH, false );
   FlipMenuBuf();
   }



//****************************************************************************
//
// DrawPWMenu ()
//
//****************************************************************************

void DrawPWMenu
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Violence Password" );

//   CurrentFont = newfont1;
   IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_largefont ],
      PU_CACHE, Cvt_cfont_t, 1 );

   if ( POK )
      {
//      DrawMenuBufPropString( PWORDX - 24, PWORDY, "ENTER OLD PASSWORD" );
      DrawMenuBufIString( PWORDX - 24, PWORDY, "ENTER OLD PASSWORD", NORMALCOLOR );
      }
   else
      {
//      DrawMenuBufPropString( PWORDX - 24, PWORDY, "ENTER PASSWORD" );
      DrawMenuBufIString( PWORDX - 24, PWORDY, "ENTER PASSWORD", NORMALCOLOR );
      }

   DrawSTMenuBuf( PBOXX, PBOXY, PBOXW, PBOXH, false );
   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_PWMenu ()
//
//****************************************************************************

void CP_PWMenu (void)
   {
   char p1[13];
   char p2[13];
   boolean EnterNewPassword;
   boolean AskForNew;
   boolean RetypePassword;

   memset (p1, 0, 13);
   memset (p2, 0, 13);

   CurrentFont = smallfont;

   EnterNewPassword = true;
   if ( POK )
      {
      DrawPWMenu ();

      // get old password
      //
      EnterNewPassword = false;
      if (US_lineinput (PBOXX+2, PBOXY+1, p1, NULL, true, 12, PSTRW, 0))
         {
         //compare user entered to old
         //
         if (StringsNotEqual (p1, pword, StringLength (p1))==false)
            {

            // Password was correct so they may change it.
            EnterNewPassword = true;
            }
         else
            {
            CP_ErrorMsg( "Violence Password", "Incorrect Password.",
               mn_largefont );
            }
         }
      }

   if ( EnterNewPassword )
      {
      MenuNum = 1;

      // get new password
      //
		AskForNew = true;
      RetypePassword = false;
      while( AskForNew )
         {
         CurrentFont = smallfont;

         SetAlternateMenuBuf ();
         ClearMenuBuf();
         SetMenuTitle ("Violence Password");

         IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_largefont ],
            PU_CACHE, Cvt_cfont_t, 1 );
         DrawMenuBufIString( PWORDX - 24, PWORDY, "ENTER NEW PASSWORD", NORMALCOLOR );

//         CurrentFont = newfont1;
//         DrawMenuBufPropString( PWORDX - 24, PWORDY, "ENTER NEW PASSWORD" );
//         DrawMenuBufItem (PWORDX-24, PWORDY, W_GetNumForName ("mnewpass"));

         DrawSTMenuBuf (PBOXX, PBOXY, PBOXW, PBOXH, false);
         FlipMenuBuf();

         memset (p1, 0, 13);

         AskForNew = false;
         if (US_lineinput (PBOXX+2, PBOXY+1, p1, NULL, true, 12, PSTRW, 0))
            {
            // Check for blank password
            if ( p1[ 0 ] == 0 )
               {
               if ( CP_DisplayMsg ( "Clear Password?\nAre you sure?", 12 ) )
                  {
                  AskForNew = false;
                  memset (pword, 0, 13);
                  WriteMenuInfo ();
                  POK = false;
                  }
               else
                  {
                  AskForNew = true;
                  }
               }
            else
               {
               RetypePassword = true;
               }
            }
         }

      if ( RetypePassword )
         {
			SetAlternateMenuBuf();
         ClearMenuBuf();
         SetMenuTitle ("Violence Password");

//         CurrentFont = newfont1;
//         DrawMenuBufPropString( PWORDX, PWORDY, "RETYPE PASSWORD" );

         IFont = ( cfont_t * )W_CacheLumpName( FontNames[ mn_largefont ],
            PU_CACHE, Cvt_cfont_t, 1 );
         DrawMenuBufIString( PWORDX, PWORDY, "RETYPE PASSWORD", NORMALCOLOR );

         DrawSTMenuBuf (PBOXX, PBOXY, PBOXW, PBOXH, false);

         FlipMenuBuf();

         // reenter password
         //
         if ( US_lineinput (PBOXX+2, PBOXY+1, p2, NULL, true, 12, PSTRW, 0) )
            {
            // compare password and retyped password
            //
            if (stricmp (p1, p2) == 0)
               {
               memset (pword, 0, 13);
               strcpy (pword, p1);
               WriteMenuInfo ();

               // If we have a null password, then we don't need to
               // ask for one.
               POK = true;
               if ( pword[ 0 ] == 0 )
                  {
                  POK = false;
                  }
               }
            else
               {
               CP_ErrorMsg( "Violence Password", "Passwords did not match.",
                  mn_largefont );
               }
            }
         }
      }

   DrawViolenceMenu ();
   }

//****************************************************************************
//
// DrawOptionDescription()
//
//****************************************************************************

void DrawOptionDescription( char ** options, int w )
   {
   int     width;
   int     height;
   char   *string;
   font_t *temp;

   EraseMenuBufRegion (25, 4, 287 - 25, 10 );

   temp = CurrentFont;
   CurrentFont = tinyfont;

   string = options[ w ];

	VW_MeasurePropString ( string, &width, &height );
   DrawMenuBufPropString ( ( 288 - width) / 2, 4, string );

   CurrentFont = temp;
   }

//****************************************************************************
//
// DrawBattleOptionDescription()
//
//****************************************************************************

void DrawBattleOptionDescription( int w )
   {
   DrawOptionDescription( BattleOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawGravityOptionDescription()
//
//****************************************************************************

void DrawGravityOptionDescription( int w )
   {
   DrawOptionDescription( GravityOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawSpeedOptionDescription()
//
//****************************************************************************

void DrawSpeedOptionDescription( int w )
   {
   DrawOptionDescription( SpeedOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawAmmoOptionDescription()
//
//****************************************************************************

void DrawAmmoOptionDescription( int w )
   {
   DrawOptionDescription( AmmoOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawHitPointsOptionDescription()
//
//****************************************************************************

void DrawHitPointsOptionDescription( int w )
   {
   DrawOptionDescription( HitPointsOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawRadicalOptionDescription()
//
//****************************************************************************

void DrawRadicalOptionDescription( int w )
   {
   DrawOptionDescription( RadicalOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawLightLevelOptionDescription()
//
//****************************************************************************

void DrawLightLevelOptionDescription( int w )
   {
   DrawOptionDescription( LightLevelOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawPointGoalOptionDescription()
//
//****************************************************************************

void DrawPointGoalOptionDescription( int w )
   {
   DrawOptionDescription( PointGoalOptionDescriptions, w );
   }
//****************************************************************************
//
// DrawDangerDamageOptionDescription()
//
//****************************************************************************

void DrawDangerDamageOptionDescription( int w )
   {
   DrawOptionDescription( DangerDamageOptionDescriptions, w );
   }

//****************************************************************************
//
// DrawTimeLimitOptionDescription()
//
//****************************************************************************

void DrawTimeLimitOptionDescription( int w )
   {
   DrawOptionDescription( TimeLimitOptionDescriptions, w );
   }



#define TURN_OFF_BATTLE_MODE( x ) \
   ModeMenu[ ( x ) - 1 ].active  = CP_SemiActive; \
   ModeMenu[ ( x ) - 1 ].routine = NULL;


//****************************************************************************
//
// DrawBattleModes ()
//
//****************************************************************************
void DrawBattleModes
   (
   void
   )

   {
   int i;

   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Battle Modes");


   MN_GetActive( &ModeItems, &ModeMenu[ 0 ], gamestate.battlemode -
      battle_Normal, OptionNums );

   #if ( SHAREWARE == 1 )
      TURN_OFF_BATTLE_MODE( battle_ScoreMore );
      TURN_OFF_BATTLE_MODE( battle_Scavenger );
      TURN_OFF_BATTLE_MODE( battle_Tag );
      TURN_OFF_BATTLE_MODE( battle_Eluder );
      TURN_OFF_BATTLE_MODE( battle_Deluder );
      TURN_OFF_BATTLE_MODE( battle_CaptureTheTriad );
   #endif

   // Capture the Triad, Tag, ScoreMore, and Hunter can only be
   // played with 2 or more players
   if ( numplayers < 2 )
      {
      TURN_OFF_BATTLE_MODE( battle_ScoreMore );
      TURN_OFF_BATTLE_MODE( battle_Tag );
      TURN_OFF_BATTLE_MODE( battle_Hunter );
      TURN_OFF_BATTLE_MODE( battle_CaptureTheTriad );
      }

   if ( ModeMenu[ ModeItems.curpos ].active != CP_CursorLocation )
      {
      for( i = 0; i < ModeItems.amount; i++ )
         {
         if ( ModeMenu[ i ].active == CP_Active )
            {
            ModeItems.curpos = i;
            ModeMenu[ i ].active = CP_CursorLocation;
            break;
            }
         }
      }

   DrawMenu( &ModeItems, &ModeMenu[ 0 ] );
   DisplayInfo( 0 );
   DrawBattleModeDescription( ModeItems.curpos );

   FlipMenuBuf();
   }



//****************************************************************************
//
// DrawBattleModeName()
//
//****************************************************************************

void DrawBattleModeName( int which )
   {
   int     width;
   int     height;
   char   *string;
   font_t *temp;


   if ( ( which < battle_Normal ) || ( which > battle_CaptureTheTriad ) )
      {
      return;
      }

   string = BattleModeNames[ which - battle_Normal ];

   temp = CurrentFont;
   CurrentFont = tinyfont;

	VW_MeasurePropString ( string, &width, &height );
   DrawMenuBufPropString ( ( 288 - width ) / 2, 4, string );
//   DrawMenuBufPropString ( 270-width, 4, string );

   CurrentFont = temp;
   }


//****************************************************************************
//
// DrawBattleModeDescription()
//
//****************************************************************************

void DrawBattleModeDescription( int w )
   {
   int     width;
   int     height;
   char   *string;
   font_t *temp;

   EraseMenuBufRegion (25, 4, 287 - 25, 10 );

   temp = CurrentFont;
   CurrentFont = tinyfont;

   string = BattleModeDescriptions[ w ];

   // Capture the Triad, Tag, ScoreMore, and Hunter can only be
   // played with 2 or more players
   if ( numplayers < 2 )
      {
      switch( w + 1 )
         {
         case battle_ScoreMore :
         case battle_Tag :
         case battle_Hunter :
         case battle_CaptureTheTriad :
            string = "This mode can only be played with 2 or more players.";
            break;
         }
      }

   #if ( SHAREWARE == 1 )
      switch( w + 1 )
         {
         case battle_ScoreMore :
         case battle_Scavenger :
         case battle_Tag :
         case battle_Eluder :
         case battle_Deluder :
         case battle_CaptureTheTriad :
            string = "See Ordering Info to find out how to get this game.";
            break;
         }
   #endif

	VW_MeasurePropString ( string, &width, &height );
   DrawMenuBufPropString ( ( 288 - width ) / 2, 4, string );

   CurrentFont = temp;
   }


//****************************************************************************
//
// CP_BattleModes ()
//
//****************************************************************************

void CP_BattleModes ( void )
   {
   int which;
   static char Warning = 0;

   //
   // ALREADY IN A GAME?
   //
   if ( ingame )
      {
      if ( !CP_DisplayMsg( CURGAME, 12 ) )
         {
         return;
         }
      else
         {
         EndGameStuff();
         }
      }

   if ( ( numplayers==1 ) && ( Warning == 0 ) )
      {
      Warning = 1;
      CP_OnePlayerWarningMessage();
      }

   DrawBattleModes ();

   damagecount = 0;
   BATTLEMODE  = true;

   do
      {
      which = HandleMenu( &ModeItems, &ModeMenu[ 0 ],
         DrawBattleModeDescription );
      }
   while ( which >= 0 );

   handlewhich = 100;

   if ( !StartGame )
      {
      BATTLEMODE = false;
      gamestate.battlemode = battle_StandAloneGame;
      }
   }

//****************************************************************************
//
// DrawBattleOptions ()
//
//****************************************************************************

void DrawBattleOptions (void)
{
   int i;

	MenuNum = 1;

	SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Battle Mode Options");

   MN_MakeActive ( &BOptItems, &BOptMenu[0], BOptItems.curpos );

   switch( gamestate.battlemode )
      {
      case battle_Collector :
         BOptMenu[2].active = CP_Inactive; // Ammo
         BOptMenu[6].active = CP_Inactive; // Point Goal
         break;

      case battle_Scavenger :
         BOptMenu[6].active = CP_Inactive; // Point Goal
         break;

      case battle_Tag :
         BOptMenu[2].active = CP_Inactive; // Ammo
         break;

      case battle_Eluder :
         BOptMenu[2].active = CP_Inactive; // Ammo
         BOptMenu[3].active = CP_Inactive; // Hit points
         BOptMenu[7].active = CP_Inactive; // Danger damage
         break;
      }

   if ( BOptMenu[ BOptItems.curpos ].active == CP_Inactive )
      {
      // Find an available cursor position
      for( i = 0; i < BOptItems.amount; i++ )
         {
         if ( BOptMenu[ i ].active == CP_Active )
            {
            BOptMenu[ i ].active = CP_CursorLocation;
            BOptItems.curpos = i;
            break;
            }
         }
      }

   DrawBattleModeName( gamestate.battlemode );
   DrawBattleOptionDescription( BOptItems.curpos );
   DrawMenu (&BOptItems, &BOptMenu[0]);
	DisplayInfo (0);
	FlipMenuBuf();
}


//****************************************************************************
//
// CP_BattleOptions ()
//
//****************************************************************************

void CP_BattleOptions (void)
{
	int which;

	DrawBattleOptions ();

	do
	{
      which = HandleMenu (&BOptItems, &BOptMenu[0], DrawBattleOptionDescription);

	} while (which >= 0);

	handlewhich = 100;
//   WriteBattleConfig();
	CalcTics();

	DrawBattleMenu ();
}



//****************************************************************************
//
// DrawColorMenu ()
//
//****************************************************************************


void DrawColorMenu( void )
   {
   int   width;
   int   height;
   char *text;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Uniform Color");

   CurrentFont = smallfont;

   text = colorname[ locplayerstate->uniformcolor ];
   VW_MeasurePropString ( text, &width, &height );
   DrawMenuBufPropString ( ( 320 - width ) / 2 - 16, MENU_Y + 5, text );
   DisplayInfo( 8 );
   EraseMenuBufRegion( COLORX, COLORY, COLORW, COLORH );
   DrawTMenuBufBox( COLORX, COLORY, COLORW, COLORH );
   DrawColoredMenuBufItem( COLORX - 36, COLORY - 33,
      W_GetNumForName( playerwadname[ locplayerstate->player ] ),
      locplayerstate->uniformcolor);


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_ColorSelection ()
//
//****************************************************************************

int CP_ColorSelection (void)
{
   int status;

   locplayerstate->uniformcolor = DefaultPlayerColor;

	DrawColorMenu ();
	status = ColorMenu();
   return( status );
}

int ColorMenu
   (
   void
   )

   {
   ControlInfo ci;
   int colorindex;
   char *text;
   int width;
   int height;
   int timer;
   int baseshape;
   int status;
   boolean update;
   boolean done;

   colorindex = DefaultPlayerColor;
   timer      = GetTicCount();
   baseshape  = W_GetNumForName( playerwadname[ locplayerstate->player ] );

   update = false;
   done   = false;
   while( !done )
      {
      ReadAnyControl( &ci );
      if ( ( ci.dir == dir_East ) && ( ( GetTicCount() - timer ) > 5 ) )
         {
         update = true;
         timer = GetTicCount();

         colorindex++;
         if ( colorindex >= MAXPLAYERCOLORS )
            {
            colorindex = 0;
            }

         MN_PlayMenuSnd( SD_MOVECURSORSND );
         }

      if ( ( ci.dir == dir_West ) && ( ( GetTicCount() - timer ) > 5 ) )
         {
         update = true;
         timer = GetTicCount();

         colorindex--;
         if ( colorindex < 0 )
            {
            colorindex = MAXPLAYERCOLORS - 1;
            }
         MN_PlayMenuSnd( SD_MOVECURSORSND );
         }

      if ( update )
         {
         update = false;
         DefaultPlayerColor = colorindex;
         locplayerstate->uniformcolor = colorindex;
         text = colorname[ locplayerstate->uniformcolor ];

         EraseMenuBufRegion( 0, MENU_Y + 5, 200, 10 );
         EraseMenuBufRegion( COLORX, COLORY, COLORW, COLORH );
         VW_MeasurePropString( text, &width, &height );
         DrawMenuBufPropString( ( 320 - width ) / 2 - 16, MENU_Y + 5, text );
         DrawTMenuBufBox( COLORX, COLORY, COLORW, COLORH );
         DrawColoredMenuBufItem( COLORX - 36, COLORY - 33,
            baseshape, locplayerstate->uniformcolor );
         }

      if ( ci.button0 || Keyboard[ sc_Space ] || Keyboard[sc_Enter ] )
         {
         Keyboard[ sc_Space ] = 0;
         Keyboard[ sc_Enter ] = 0;
         MN_PlayMenuSnd( SD_SELECTSND );
         status = 1;
         done = true;
         }
      else if ( ci.button1 || Keyboard[ sc_Escape ] )
         {
         MN_PlayMenuSnd( SD_ESCPRESSEDSND );
         status = 0;
         done = true;
         }

      RefreshMenuBuf( 0 );
      }

   IN_ClearKeysDown();
   return( status );
   }


//****************************************************************************
//
// CP_PlayerSelection ()
//
//****************************************************************************
int CP_PlayerSelection
   (
   void
   )

   {
	int which;

   // Do Pick-A-Player menu
	DrawPlayerMenu();

   do
      {
      which = HandleMenu( &PlayerItems, &PlayerMenu[ 0 ], DrawNewPlayerDiff );
      if ( which < 0 )
         {
         handlewhich = 1;
         return( 0 );
         }

      #if ( SHAREWARE == 1 )
         if ( PlayerMenu[ which ].active == CP_SemiActive )
            {
            CP_ErrorMsg( "Choose Player",
               "Read the Ordering Info section from the Main Menu to "
               "find out how to get the other characters.",
               mn_smallfont );

            DrawPlayerMenu();
            }
      #endif
      }
   while( PlayerMenu[ which ].active == CP_SemiActive );


#if ( SHAREWARE == 1 )
   DefaultPlayerCharacter = 0;
   locplayerstate->player = 0;
#else
   DefaultPlayerCharacter = which;
   locplayerstate->player = which;
#endif

	return (1);
}

//****************************************************************************
//
// CP_OnePlayerWarningMessage ()
//
//****************************************************************************

void CP_OnePlayerWarningMessage
   (
   void
   )

   {
   CP_ErrorMsg( "Comm-bat Warning",
      "Comm-bat is designed for modem and network play.  "
      "One player mode is provided for exploration.  The "
      "Collector battle mode is still fun on your own.",
      mn_smallfont );
   }


//****************************************************************************
//
// CP_CaptureTheTriadError()
//
//****************************************************************************

void CP_CaptureTheTriadError
   (
   void
   )

   {
   SetupMenuBuf();
   SetUpControlPanel();

   CP_ErrorMsg( "Comm-bat Warning",
      "Capture the Traid can only be played with 2 teams."
      "  All players must return to the menu to choose their "
      "team colors.", mn_largefont );

   CleanUpControlPanel();
   ShutdownMenuBuf();
   }


//****************************************************************************
//
// CP_TeamPlayErrorMessage()
//
//****************************************************************************

void CP_TeamPlayErrorMessage
   (
   void
   )

   {
   SetupMenuBuf();
   SetUpControlPanel();

   CP_ErrorMsg( "Comm-bat Warning",
      "Team play can only be played with 2 or more teams."
      "  All players must return to the menu to choose their "
      "team colors.", mn_largefont );

   CleanUpControlPanel();
   ShutdownMenuBuf();
   }


//****************************************************************************
//
// CP_ModemGameMessage ()
//
//****************************************************************************

#if (SITELICENSE == 0)

#define SITELINES 8

char *sitemessage[] =
   {
   "HEY! ARE YOU PLAYING ON AN ILLEGAL COPY?",
   "Network players: it's easy to play legally!",
   "Just get a site license.  For a little over",
   "the price of two games, you get 10 more battle",
   "levels, 11 command cards, and a signed Site",
   "License.  We don't charge you for 11 copies--",
   "just for two!  Call 1-800-APOGEE1 to order.",
   "For more on site licenses, see ORDERING INFO."
   };

#endif

void CP_ModemGameMessage (int player  )

   {
   int i;
    EnableScreenStretch();
       // SetTextMode (  );

	SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Game Message");

   newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1);
   CurrentFont = newfont1;
   if ( modemgame == false )
      {
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 60;
      MenuBufCPrint ("Please wait.\nLoading game.");
      }
   else
      {
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 50;

      if (networkgame==true)
         {
         PrintY = WindowY = 28;
         }

      if ( player == 0 )
         {
         MenuBufCPrint ("Please wait for\nplayers to choose\ntheir characters.");
         }
      else
         {
         MenuBufCPrint ("Please wait while\nMaster selects\nCOMM-BAT options.");
         }

#if (SITELICENSE == 0)
      if (networkgame==true)
         {
         for( i = 0; i < SITELINES; i++ )
            {
            PrintBattleOption( true, 68, 77 + i * 8,
               sitemessage[ i ] );
            }
         }
#endif
      }

   FlipMenuBuf();
	RefreshMenuBuf (0);

	 DisableScreenStretch();
   }


//****************************************************************************
//
// DrawGravityMenu ()
//
//****************************************************************************

void DrawGravityMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Gravity");


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons (&GravityItems, &GravityMenu[0],
		BATTLE_Options[gamestate.battlemode].Gravity, GravityNums);
   MN_GetActive (&GravityItems, &GravityMenu[0],
		BATTLE_Options[gamestate.battlemode].Gravity, GravityNums);

	DrawMenu (&GravityItems, &GravityMenu[0]);
   DrawGravityOptionDescription( GravityItems.curpos );
   PrintBattleOption( true, 32, 79,
      "WARNING: High gravity has an unfortunate side effect in" );
   PrintBattleOption( true, 32, 87,
      "some levels.  It is possible to jump into an area that is" );
   PrintBattleOption( true, 32, 95,
      "impossible, or at least extremely difficult to get out" );
   PrintBattleOption( true, 32, 103,
      "of.  In these situations, the only thing you can do is" );
   PrintBattleOption( true, 32, 111,
      "kill your character, or find some kindly soul to do it" );
   PrintBattleOption( true, 32, 119,
      "for you.  If this fails, you'll just have to end your game." );

   DisplayInfo( 0 );

   FlipMenuBuf();
}


//****************************************************************************
//
// CP_GravityOptions ()
//
//****************************************************************************

void CP_GravityOptions (void)
{
   int which;

   DrawGravityMenu ();

   do
   {
      which = HandleMenu (&GravityItems, &GravityMenu[0], DrawGravityOptionDescription);

      if (which >= 0)
		  BATTLE_Options[gamestate.battlemode].Gravity = GravityNums[ which ];

      MN_DrawButtons (&GravityItems, &GravityMenu[0],
			BATTLE_Options[gamestate.battlemode].Gravity, GravityNums);

   } while (which >= 0);

   handlewhich = 100;
   DrawBattleOptions ();
}


//****************************************************************************
//
// DrawSpeedMenu ()
//
//****************************************************************************

void DrawSpeedMenu (void)
{
	MenuNum = 1;

   SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Speed");


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons (&SpeedItems, &SpeedMenu[0],
						 BATTLE_Options[gamestate.battlemode].Speed, OptionNums );
   MN_GetActive (&SpeedItems, &SpeedMenu[0],
					  BATTLE_Options[gamestate.battlemode].Speed, OptionNums );

   DrawMenu (&SpeedItems, &SpeedMenu[0]);
   DrawSpeedOptionDescription( SpeedItems.curpos );
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_SpeedOptions ()
//
//****************************************************************************

void CP_SpeedOptions (void)
{
   int which;

   DrawSpeedMenu ();

   do
   {
      which = HandleMenu (&SpeedItems, &SpeedMenu[0], DrawSpeedOptionDescription);

      if (which >= 0)
		  BATTLE_Options[gamestate.battlemode].Speed = which;

      MN_DrawButtons (&SpeedItems, &SpeedMenu[0],
         BATTLE_Options[gamestate.battlemode].Speed, OptionNums );
   } while (which >= 0);

   handlewhich = 100;
   DrawBattleOptions ();
}


//****************************************************************************
//
// DrawAmmoPerWeaponMenu ()
//
//****************************************************************************
void DrawAmmoPerWeaponMenu
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Ammo Per Weapon");

   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons( &AmmoPerWeaponItems, &AmmoPerWeaponMenu[ 0 ],
      BATTLE_Options[ gamestate.battlemode ].Ammo, OptionNums );

   MN_GetActive( &AmmoPerWeaponItems, &AmmoPerWeaponMenu[ 0 ],
      BATTLE_Options[ gamestate.battlemode ].Ammo, OptionNums );

   DrawMenu( &AmmoPerWeaponItems, &AmmoPerWeaponMenu[ 0 ] );

   PrintBattleOption( true, 32, 79,
      "WARNING: Infinite ammo can seriously alter the balance of" );
   PrintBattleOption( true, 32, 87,
      "the game.  We recommend that you only use it occasionally." );
   PrintBattleOption( true, 32, 95,
      "It tends to only work well on small levels with lots of" );
   PrintBattleOption( true, 32, 103,
      "weapons, where the action is far more intense.  On large" );
   PrintBattleOption( true, 32, 111,
      "levels, you may find it causes people to wait in easily" );
   PrintBattleOption( true, 32, 119,
      "guardable areas and pick off anyone that comes in the room" );
   PrintBattleOption( true, 32, 127,
      "(creating an unfair advantage)." );

   if ( AmmoPerWeaponItems.curpos == 2 )
      {
      PrintBattleOption( true, 102, 136, "You have been warned." );
      }

   DrawAmmoOptionDescription( AmmoPerWeaponItems.curpos );
   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_AmmoPerWeaponOptions ()
//
//****************************************************************************
void CP_AmmoPerWeaponOptions
   (
   void
   )

   {
   int which;

   DrawAmmoPerWeaponMenu();

   do
      {
      which = HandleMenu( &AmmoPerWeaponItems, &AmmoPerWeaponMenu[ 0 ], DrawAmmoOptionDescription);

      if (which >= 0)
         {
         if ( AmmoPerWeaponItems.curpos == 2 )
            {
            MN_PlayMenuSnd( SD_LIGHTNINGSND );
            PrintBattleOption( true, 102, 136, "You have been warned." );
            VL_FillPalette(255,255,255);
            VL_FadeIn(0,255,origpal,10);
            }
         else if ( BATTLE_Options[ gamestate.battlemode ].Ammo == 2 )
            {
            EraseMenuBufRegion( 102, 136, 84, 8 );
            MN_PlayMenuSnd( SD_PLAYERTCSND );
            }

         BATTLE_Options[ gamestate.battlemode ].Ammo = which;
         }

      MN_DrawButtons( &AmmoPerWeaponItems, &AmmoPerWeaponMenu[ 0 ],
         BATTLE_Options[ gamestate.battlemode ].Ammo, OptionNums );
      }
   while( which >= 0 );

   handlewhich = 100;
   DrawBattleOptions();
   }


//****************************************************************************
//
// DrawHitPointsMenu ()
//
//****************************************************************************
void DrawHitPointsMenu
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Player Hitpoints");

   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons( &HitPointItems, &HitPointMenu[ 0 ],
      BATTLE_Options[ gamestate.battlemode ].HitPoints, HitPointNums );

   MN_GetActive( &HitPointItems, &HitPointMenu[ 0 ],
      BATTLE_Options[ gamestate.battlemode ].HitPoints, HitPointNums );

   DrawMenu( &HitPointItems, &HitPointMenu[ 0 ] );
   DrawHitPointsOptionDescription( HitPointItems.curpos );
   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_HitPointsOptions ()
//
//****************************************************************************

void CP_HitPointsOptions (void)
{
   int which;

   DrawHitPointsMenu ();

   do
   {
      which = HandleMenu (&HitPointItems, &HitPointMenu[0], DrawHitPointsOptionDescription);

      if (which >= 0)
         BATTLE_Options[gamestate.battlemode].HitPoints = HitPointNums[ which ];

      MN_DrawButtons (&HitPointItems, &HitPointMenu[0],
							 BATTLE_Options[gamestate.battlemode].HitPoints, HitPointNums);

   } while (which >= 0);

   handlewhich = 100;
   DrawBattleOptions ();
}



//****************************************************************************
//
// DrawSpawnControlMenu ()
//
//****************************************************************************
void DrawSpawnControlMenu
   (
   void
   )

   {
   int i;

   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Radical Options");

   MN_MakeActive( &SpawnItems, &SpawnMenu[ 0 ], SpawnItems.curpos );

#if ( SHAREWARE == 1 )
   BATTLE_Options[ gamestate.battlemode ].SpawnMines = false;
   SpawnMenu[ 3 ].active = CP_Inactive; // Mines
#endif

   switch( gamestate.battlemode )
      {
      case battle_Normal :
         break;

      case battle_ScoreMore :
         break;

      case battle_Collector :
         SpawnMenu[ 2 ].active = CP_Inactive; // Weapons
         SpawnMenu[ 5 ].active = CP_Inactive; // Persistence
         SpawnMenu[ 6 ].active = CP_Inactive; // Random Weapons
         SpawnMenu[ 7 ].active = CP_Inactive; // Friendly Fire
         break;

      case battle_Scavenger :
         SpawnMenu[ 7 ].active = CP_Inactive; // Friendly Fire
         break;

      case battle_Hunter :
         break;

      case battle_Tag :
         SpawnMenu[ 2 ].active = CP_Inactive; // Weapons
         SpawnMenu[ 5 ].active = CP_Inactive; // Persistence
         SpawnMenu[ 6 ].active = CP_Inactive; // Random Weapons
         break;

      case battle_Eluder :
         SpawnMenu[ 1 ].active = CP_Inactive; // Health
         SpawnMenu[ 2 ].active = CP_Inactive; // Weapons
         SpawnMenu[ 4 ].active = CP_Inactive; // Respawn
         SpawnMenu[ 5 ].active = CP_Inactive; // Persistence
         SpawnMenu[ 6 ].active = CP_Inactive; // Random Weapons
         SpawnMenu[ 7 ].active = CP_Inactive; // Friendly Fire
         break;

      case battle_Deluder :
         SpawnMenu[ 7 ].active = CP_Inactive; // Friendly Fire
         break;

      case battle_CaptureTheTriad :
         SpawnMenu[ 7 ].active = CP_Inactive; // Friendly Fire
         break;
      }

   if ( SpawnMenu[ SpawnItems.curpos ].active == CP_Inactive )
      {
      // Find an available cursor position
      for( i = 0; i < SpawnItems.amount; i++ )
         {
         if ( SpawnMenu[ i ].active == CP_Active )
            {
            SpawnMenu[ i ].active = CP_CursorLocation;
            SpawnItems.curpos = i;
            break;
            }
         }
      }

   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   DrawSpawnControlButtons();

   MN_GetCursorLocation( &SpawnItems, &SpawnMenu[ 0 ] );

   DrawMenu( &SpawnItems, &SpawnMenu[ 0 ] );
   DrawRadicalOptionDescription( SpawnItems.curpos );
   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// DrawSpawnControlButtons ()
//
//****************************************************************************
void DrawSpawnControlButtons
   (
   void
   )

   {
   int x;
   int y;
   int button_on;
   int button_off;

   button_on  = W_GetNumForName( "snd_on" );
   button_off = W_GetNumForName( "snd_off" );

   x = SpawnItems.x + 18;
   y = SpawnItems.y - 1;

	if ( BATTLE_Options[ gamestate.battlemode ].SpawnDangers )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].SpawnHealth )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].SpawnWeapons )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].SpawnMines )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].RespawnItems )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].WeaponPersistence )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].RandomWeapons )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}

	y += 14;
	if ( BATTLE_Options[ gamestate.battlemode ].FriendlyFire )
		{
		DrawMenuBufItem( x, y, button_on );
		}
	else
		{
		EraseMenuBufRegion( x, y, 16, 16 );
		DrawMenuBufItem( x, y, button_off );
		}
	}


//****************************************************************************
//
// CP_SpawnControlOptions ()
//
//****************************************************************************
void CP_SpawnControlOptions
   (
   void
   )

   {
	int which;

	DrawSpawnControlMenu();

   do
      {
      which = HandleMenu( &SpawnItems, &SpawnMenu[ 0 ], DrawRadicalOptionDescription );
      switch( which )
         {
			case 0 :
				BATTLE_Options[ gamestate.battlemode ].SpawnDangers =
               !BATTLE_Options[ gamestate.battlemode ].SpawnDangers;
            break;

         case 1 :
				BATTLE_Options[ gamestate.battlemode ].SpawnHealth =
               !BATTLE_Options[ gamestate.battlemode ].SpawnHealth;
            break;

         case 2 :
				BATTLE_Options[ gamestate.battlemode ].SpawnWeapons =
               !BATTLE_Options[ gamestate.battlemode ].SpawnWeapons;
            break;

         case 3 :
				BATTLE_Options[ gamestate.battlemode ].SpawnMines =
               !BATTLE_Options[ gamestate.battlemode ].SpawnMines;
            break;

         case 4 :
				BATTLE_Options[ gamestate.battlemode ].RespawnItems =
               !BATTLE_Options[ gamestate.battlemode ].RespawnItems;
            break;

         case 5 :
				BATTLE_Options[ gamestate.battlemode ].WeaponPersistence =
               !BATTLE_Options[ gamestate.battlemode ].WeaponPersistence;
            break;

         case 6 :
				BATTLE_Options[ gamestate.battlemode ].RandomWeapons =
               !BATTLE_Options[ gamestate.battlemode ].RandomWeapons;
            break;

         case 7 :
				BATTLE_Options[ gamestate.battlemode ].FriendlyFire =
               !BATTLE_Options[ gamestate.battlemode ].FriendlyFire;
            break;
         }

      DrawSpawnControlButtons();
      }
   while( which >= 0 );

   handlewhich = 100;
   DrawBattleOptions();
   }



//****************************************************************************
//
// DrawLightLevelMenu ()
//
//****************************************************************************

void DrawLightLevelMenu (void)
{
   MenuNum = 1;

	SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Light Levels");


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons (&LightLevelItems, &LightLevelMenu[0],
      BATTLE_Options[gamestate.battlemode].LightLevel, OptionNums );
   MN_GetActive (&LightLevelItems, &LightLevelMenu[0],
      BATTLE_Options[gamestate.battlemode].LightLevel, OptionNums );

   DrawMenu (&LightLevelItems, &LightLevelMenu[0]);
   DrawLightLevelOptionDescription( LightLevelItems.curpos );
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_LightLevelOptions ()
//
//****************************************************************************

void CP_LightLevelOptions (void)
{
   int which;

   DrawLightLevelMenu ();

   do
   {
      which = HandleMenu (&LightLevelItems, &LightLevelMenu[0], DrawLightLevelOptionDescription);

      if (which >= 0)
		  BATTLE_Options[gamestate.battlemode].LightLevel = which;

      MN_DrawButtons (&LightLevelItems, &LightLevelMenu[0],
         BATTLE_Options[gamestate.battlemode].LightLevel, OptionNums );

   } while (which >= 0);

   handlewhich = 100;
   DrawBattleOptions ();
}



//****************************************************************************
//
// DrawPointGoalMenu()
//
//****************************************************************************

void DrawPointGoalMenu
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Point Goal");

   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   MN_DrawButtons( &PointGoalItems, &PointGoalMenu[ 0 ],
      BATTLE_Options[ gamestate.battlemode ].Kills, KillNums );

   MN_GetActive( &PointGoalItems, &PointGoalMenu[ 0 ],
	   BATTLE_Options[ gamestate.battlemode ].Kills, KillNums );

   DrawMenu( &PointGoalItems, &PointGoalMenu[ 0 ] );
   DrawPointGoalOptionDescription( PointGoalItems.curpos );
   DisplayInfo( 0 );
   FlipMenuBuf();
   }


//****************************************************************************
//
// CP_PointGoalOptions()
//
//****************************************************************************

void CP_PointGoalOptions
   (
   void
   )

   {
   int which;

   DrawPointGoalMenu();

   do
      {
      which = HandleMenu( &PointGoalItems, &PointGoalMenu[ 0 ], DrawPointGoalOptionDescription );

      if ( which >= 0 )
         {
         BATTLE_Options[ gamestate.battlemode ].Kills = KillNums[ which ];

         MN_DrawButtons( &PointGoalItems, &PointGoalMenu[ 0 ],
            BATTLE_Options[ gamestate.battlemode ].Kills, KillNums );
         }
      }
   while( which >= 0 );

   handlewhich = 100;
   DrawBattleOptions();
   }


//****************************************************************************
//
// DrawDangerMenu ()
//
//****************************************************************************

void DrawDangerMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Danger Damage");


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }
   MN_DrawButtons (&DangerItems, &DangerMenu[0],
						 BATTLE_Options[gamestate.battlemode].DangerDamage, DangerNums);
   MN_GetActive (&DangerItems, &DangerMenu[0],
						 BATTLE_Options[gamestate.battlemode].DangerDamage, DangerNums);

   DrawMenu (&DangerItems, &DangerMenu[0]);
   DrawDangerDamageOptionDescription( DangerItems.curpos );
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_DangerOptions ()
//
//****************************************************************************

void CP_DangerOptions (void)
{
   int which;

   DrawDangerMenu ();

   do
   {
      which = HandleMenu (&DangerItems, &DangerMenu[0], DrawDangerDamageOptionDescription);

      if (which >= 0)
		  BATTLE_Options[gamestate.battlemode].DangerDamage = DangerNums[which];

      MN_DrawButtons (&DangerItems, &DangerMenu[0],
			BATTLE_Options[gamestate.battlemode].DangerDamage, DangerNums);

   } while (which >= 0);

   handlewhich = 100;
   DrawBattleOptions ();
}


//****************************************************************************
//
// DrawTimeLimitMenu ()
//
//****************************************************************************

void DrawTimeLimitMenu (void)
{
   MenuNum = 1;

   SetAlternateMenuBuf ();
   ClearMenuBuf();
   SetMenuTitle ("Time Limit");


   if ( ( gamestate.battlemode != battle_StandAloneGame ) &&
      ( consoleplayer == 0 ) )
      {
      DrawBattleModeName( gamestate.battlemode );
      }
   MN_DrawButtons (&TimeLimitItems, &TimeLimitMenu[0],
	   BATTLE_Options[gamestate.battlemode].TimeLimit, TimeLimitNums);
   MN_GetActive (&TimeLimitItems, &TimeLimitMenu[0],
      BATTLE_Options[gamestate.battlemode].TimeLimit, TimeLimitNums);

   if ( gamestate.battlemode == battle_Hunter )
      {
      TimeLimitMenu[7].active = CP_Inactive; // No time limit
      if ( TimeLimitItems.curpos == 7 )
         {
         TimeLimitMenu[0].active = CP_CursorLocation;
         TimeLimitItems.curpos = 0;
         }
      }

	DrawMenu (&TimeLimitItems, &TimeLimitMenu[0]);
   DrawTimeLimitOptionDescription( TimeLimitItems.curpos );
   DisplayInfo (0);
   FlipMenuBuf();
}


//****************************************************************************
//
// CP_TimeLimitOptions ()
//
//****************************************************************************

void CP_TimeLimitOptions (void)
{
   int which;

   DrawTimeLimitMenu ();

   do
   {
      which = HandleMenu (&TimeLimitItems, &TimeLimitMenu[0], DrawTimeLimitOptionDescription);

      if (which >= 0)
		  BATTLE_Options[gamestate.battlemode].TimeLimit = TimeLimitNums[which];

      MN_DrawButtons (&TimeLimitItems, &TimeLimitMenu[0],
			BATTLE_Options[gamestate.battlemode].TimeLimit, TimeLimitNums);

   } while (which >= 0);

	handlewhich = 100;
	DrawBattleOptions ();
}

void PrintBattleOption
   (
   boolean inmenu,
   int x,
   int y,
   char *text
   )

   {
   if ( inmenu )
      {
      IFont = ( cfont_t * )W_CacheLumpName( "itnyfont", PU_CACHE, Cvt_cfont_t, 1 );
      DrawMenuBufIString( x + 1, y + 1, text, 0 );
      DrawMenuBufIString( x, y, text, ACTIVECOLOR );
      }
   else
      {
      PrintX = x;
      PrintY = y;
      US_BufPrint( text );
      }
   }

void ShowBattleOption
   (
   boolean inmenu,
   int PosX,
   int PosY,
   int column,
   int Line,
   char *text1,
   char *text2
   )

   {
   char text[ 80 ];
   int x;
   int y;

   y = PosY + ( Line * 7 );
   x = PosX + column * 120;

   PrintBattleOption( inmenu, x, y, text1 );

   strcpy( text, ": " );
   strcat( text, text2 );

   PrintBattleOption( inmenu, x + 60, y, text );
   }

void ShowBattleOptions
	(
   boolean inmenu,
   int PosX,
   int PosY
	)

	{
   battle_type *options;
	battle_type BatOps;
   char *string;
   char text[ 80 ];
   int  width;
   int  height;
   int  temp;

	CurrentFont = tinyfont;

   strcpy( text, "CURRENT OPTIONS FOR " );
   strcat( text, BattleModeNames[ gamestate.battlemode - battle_Normal ] );
	VW_MeasurePropString ( text, &width, &height );
   if ( inmenu )
      {
      temp = 288;
      }
   else
      {
      temp = 320;
      }
   PrintBattleOption( inmenu, (temp - width)/2, PosY, text );

   PosY++;

   BATTLE_GetOptions( &BatOps );
	options = &BatOps;

   ShowBattleOption( inmenu, PosX, PosY, 0, 1, "Friendly Fire",
      ( options->FriendlyFire ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 2, "Weapon Persist",
      ( options->WeaponPersistence ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 3, "Random Weapons",
      ( options->RandomWeapons ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 4, "Respawn Items",
      ( options->RespawnItems ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 5, "Spawn Health",
      ( options->SpawnHealth ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 6, "Spawn Weapons",
      ( options->SpawnWeapons ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 7, "Spawn Mines",
      ( options->SpawnMines ) ? "On" : "Off" );

   ShowBattleOption( inmenu, PosX, PosY, 0, 8, "Spawn Dangers",
      ( options->SpawnDangers ) ? "On" : "Off" );

   switch( options->DangerDamage )
      {
      case bo_danger_low :
         string = "Low";
         break;

      case bo_danger_normal :
         string = "Normal";
         break;

      case bo_danger_kill :
         string = "Kill";
         break;

      default :
         itoa( options->DangerDamage, text, 10 );
         string = text;
      }
   ShowBattleOption( inmenu, PosX, PosY, 0, 9, "Danger Damage", string );

   GetMapFileName ( text );
   ShowBattleOption( inmenu, PosX, PosY, 0, 10, "Filename", text );

   itoa( numplayers, text, 10 );
   ShowBattleOption( inmenu, PosX, PosY, 1, 1, "Players", text );

   if ( options->TimeLimit == bo_time_infinite )
      {
      string = "None";
      }
   else
      {
      itoa( options->TimeLimit, text, 10 );
      string = text;
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 2, "Time Limit", string );

   if ( ( gamestate.battlemode == battle_Collector ) ||
      ( gamestate.battlemode == battle_Scavenger ) )
      {
      string = "?";
      }
   else
      {
      switch( options->Kills )
         {
         case bo_kills_random :
            string = "Random";
            break;

         case bo_kills_blind :
            string = "Blind";
            break;

         case bo_kills_infinite :
            string = "Infinite";
            break;

         default :
            itoa( options->Kills, text, 10 );
            string = text;
         }
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 3, "Point Goal", string );

   switch( options->Ammo )
      {
      case bo_one_shot :
         string = "One shot";
         break;

      case bo_normal_shots :
         string = "Normal";
         break;

      case bo_infinite_shots :
         string = "Gunfinity";
         break;
		}
   ShowBattleOption( inmenu, PosX, PosY, 1, 4, "Ammo", string );

   if ( options->HitPoints == bo_character_hitpoints )
      {
      string = "Character";
      }
   else
      {
      itoa( options->HitPoints, text, 10 );
      string = text;
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 5, "Hit Points", string );

   itoa( options->RespawnTime, text, 10 );
   ShowBattleOption( inmenu, PosX, PosY, 1, 6, "Respawn Time", text );

   switch( options->Speed )
      {
      case bo_normal_speed :
         string = "Normal";
         break;

      case bo_fast_speed :
         string = "Fast";
         break;
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 7, "Speed", string );

   switch( options->LightLevel )
      {
      case bo_light_dark :
         string = "Dark";
         break;

      case bo_light_normal :
         string = "Normal";
         break;

      case bo_light_bright :
         string = "Bright";
         break;

      case bo_light_fog :
         string = "Fog";
         break;

      case bo_light_periodic :
         string = "Periodic";
			break;

      case bo_light_lightning :
         string = "Lightning";
         break;
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 8, "Light", string );

   if ( options->Gravity == NORMAL_GRAVITY )
		{
      string = "Normal";
      temp = NORMAL_GRAVITY;
      }
   else if ( options->Gravity < NORMAL_GRAVITY )
      {
      string = "Low";
      temp = LOW_GRAVITY;
      }
   else
      {
      string = "High";
      temp = HIGH_GRAVITY;
      }

   strcpy( text, string );
   if ( options->Gravity < (unsigned int)temp )
      {
      strcat( text, "-" );
      }

   if ( options->Gravity > (unsigned int)temp )
      {
      strcat( text, "+" );
      }
   ShowBattleOption( inmenu, PosX, PosY, 1, 9, "Gravity", text );
   }


//****************************************************************************
//
// SetMenuHeader()
//
//****************************************************************************

void SetMenuHeader
   (
   char *header
   )

   {
   int width;
   int height;

   EraseMenuBufRegion( 16, 0, 256, 16 );

   CurrentFont = tinyfont;
   VW_MeasurePropString ( header, &width, &height );
   DrawMenuBufPropString ( ( 288 - width ) / 2, 4, header );

   RefreshMenuBuf( 0 );
   }


//****************************************************************************
//
// DrawMultiPageCustomMenu()
//
//****************************************************************************

void DrawMultiPageCustomMenu
   (
   char *title,
   void ( *redrawfunc )( void )
   )

   {
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( title );

   DrawMenu( &MultiPageCustomItems, &MultiPageCustomMenu[ 0 ] );

   DisplayInfo( 0 );

   if ( redrawfunc != NULL )
      {
      redrawfunc();
      }

   CalcTics();
   FlipMenuBuf();
	RefreshMenuBuf( 0 );
   }


//****************************************************************************
//
// HandleMultiPageCustomMenu()
//
//****************************************************************************

int HandleMultiPageCustomMenu
   (
   char **names,
   int   amount,
   int   curpos,
   char *title,
   void  ( *routine )( int w ),
   void ( *redrawfunc )( void ),
   boolean exitonselect
   )

   {
   boolean redraw;
   int  page;
   int  cursorpos;
   int  maxpos;
   int  numpos;
   int  which;
   int  selection;
   int  i;
   char letter;

   MenuNum = 11;

   cursorpos = curpos % MAXCUSTOM;
   page      = curpos - cursorpos;
   MultiPageCustomItems.curpos = cursorpos + 2;

   redraw = true;

   do
      {
      if ( redraw )
         {
         redraw = false;
         MultiPageCustomMenu[ 0 ].active = CP_Active;
         MultiPageCustomMenu[ 1 ].active = CP_Active;
         if ( page == 0 )
            {
            MultiPageCustomMenu[ 1 ].active = CP_Inactive;
            }

         maxpos = page + MAXCUSTOM;
         if ( maxpos >= amount )
            {
            MultiPageCustomMenu[ 0 ].active = CP_Inactive;
            maxpos = amount;
            }

         numpos = maxpos - page + 2;
         MultiPageCustomItems.amount = numpos;

         for( i = 2; i < numpos; i++ )
            {
            MultiPageCustomMenu[ i ].active = CP_Active;

            // Set the name of the level
            strcpy (MultiPageCustomNames[ i ] , names[ page + i - 2 ]);

            // Set the quick key
            letter = *names[ page + i - 2 ];

            // Force it to upper case
            if ( ( letter >= 'a' ) && ( letter <= 'z' ) )
               {
               letter = letter - 'a' + 'A';
               }

            // Only use letters
            if ( ( letter < 'A' ) || ( letter > 'Z' ) )
               {
               letter = 'a';
               }
            MultiPageCustomMenu[ i ].letter = letter;
            }

         // If the cursor is at an invalid position, find a valid one
         cursorpos = MultiPageCustomItems.curpos;
         if ( cursorpos >= numpos )
            {
            cursorpos = numpos - 1;
            }
         else
            {
            while( MultiPageCustomMenu[ cursorpos ].active == CP_Inactive )
               {
               cursorpos++;
               }
            }
         MultiPageCustomItems.curpos = cursorpos;
         MultiPageCustomMenu[ cursorpos ].active = CP_CursorLocation;

         DrawMultiPageCustomMenu( title, redrawfunc );
         }

      which = HandleMenu( &MultiPageCustomItems, &MultiPageCustomMenu[ 0 ],
         NULL );

      switch( which )
         {
         case ESCPRESSED :
            selection = -1;
            break;

         case PAGEDOWN :
         case 0 :
            page += MAXCUSTOM;
            which = 0;
            redraw = true;
            break;

         case PAGEUP :
         case 1 :
            page -= MAXCUSTOM;
            which = 0;
            redraw = true;
            break;

         default :
            selection = page + which - 2;
            if ( routine )
               {
               routine( selection );
               }

            if ( exitonselect )
               {
               which = -1;
               }
            break;
         }
      }
   while( which >= 0 );

   return( selection );
   }


//****************************************************************************
//
// CP_LevelSelectionRedraw()
//
//****************************************************************************

void CP_LevelSelectionRedraw
   (
   void
   )

   {
   if ( gamestate.battlemode >= battle_Normal )
      {
      SetMenuHeader( BattleModeNames[ gamestate.battlemode - battle_Normal ] );
      }
   }


//****************************************************************************
//
// CP_LevelSelectionMenu ()
//
//****************************************************************************
int CP_LevelSelectionMenu
   (
   void
   )

   {
   static char levelcursorpos[ 2 ] = { 0 };

   char *LevelNames[ 100 ];
   int   whichlevels;
   int   numlevels;
   int   level;
   int   i;

   whichlevels = 0;
   if ( BATTLEMODE )
      {
      whichlevels = 1;
      }

   mapinfo = ( mapfileinfo_t * )SafeMalloc( sizeof( mapfileinfo_t ) );
   GetMapInfo( mapinfo );

   numlevels = mapinfo->nummaps;
   if ( numlevels <= 0 )
      {
      Error( "CP_LevelSelectionMenu : No maps found in RTL/RTC file." );
      }

   for( i = 0; i < numlevels; i++ )
      {
      LevelNames[ i ] = mapinfo->maps[ i ].mapname;
      }

   level = HandleMultiPageCustomMenu( LevelNames, numlevels,
      levelcursorpos[ whichlevels ], "Level Selection", NULL,
      CP_LevelSelectionRedraw, true );

   SafeFree( mapinfo );

   if ( level >= 0 )
      {
      levelcursorpos[ whichlevels ] = level;
      }

   return( level );
   }


//****************************************************************************
//
// DrawEnterCodeNameMenu ()
//
//****************************************************************************
void DrawEnterCodeNameMenu
   (
   void
   )

   {
	SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Enter CodeName");

   if ( consoleplayer == 0 )
      {
      DrawBattleModeName( gamestate.battlemode );
      }

   WindowW = 288;
   WindowH = 158;
   WindowX = 0;
   WindowY = 50;
   PrintX  = 0;
   PrintY  = 50;

   CurrentFont = smallfont;

   MenuBufCPrint( "Enter CodeName\n" );
   MenuBufCPrint( "maximum 8 letters\n" );

   DrawSTMenuBuf( ( 288 - 92 ) / 2 - 2, 80 - 2, 92 + 4, 10 + 4, false );

   DrawMenuBufPropString( ( 288 - 92 ) / 2, 80, CodeName );

   WindowX = 144 - ( 9 * 4 );
   PrintX  = WindowX;

   FlipMenuBuf();
	RefreshMenuBuf( 0 );
   }

//****************************************************************************
//
// CP_EnterCodeNameMenu ()
//
//****************************************************************************
int CP_EnterCodeNameMenu
   (
   void
   )

   {
   char input[10];

   DrawEnterCodeNameMenu();

   memset(input,0,sizeof(input));
   strcpy(input,CodeName);


   if (US_LineInput ((288-92)/2, 80, input, input, true, 8, 92, 0))
      {
      strcpy (&locplayerstate->codename[0], input);
      strcpy (CodeName, input);
      WaitKeyUp();
      return 1;
      }
   else
      {
      WaitKeyUp();
      return 0;
      }
   }

#ifdef DOS
void SS_DrawSBTypeMenu( void );
void SS_SBTypeMenu( void );
void SS_DrawPortMenu( void );
void SS_PortMenu( void );
void SS_Draw8BitDMAMenu( void );
void SS_8BitDMAMenu( void );
void SS_Draw16BitDMAMenu( void );
void SS_16BitDMAMenu( void );
void SS_DrawIrqMenu( void );
void SS_IrqMenu( void );
void SS_Quit( void );
void DrawSoundSetupMainMenu( void );
void CP_SoundSetup( void );
void SS_MusicMenu( void );
void SS_DrawMusicMenu( void );
void SS_SoundMenu( void );
void SS_DrawSoundMenu( void );
void SS_SetupMusicCardMenu( void );
void DrawMusicCardMenu( void );
void SS_SetupSoundBlaster( int sbmenu );
void SS_SetupSoundCardMenu( void );
void SS_VoiceMenu( int sbmenu );
void SS_DrawVoiceMenu( void );
void SS_ChannelMenu( void );
void SS_DrawChannelMenu( void );
void SS_BitMenu( void );
void SS_DrawBitMenu( void );

extern int musicnums[ 11 ];
extern int fxnums[ 11 ];

static int midinums[ 12 ]  = {
   0x220, 0x230, 0x240, 0x250, 0x300, 0x320,
   0x330, 0x332, 0x334, 0x336, 0x340, 0x360
   };
static int voicenums[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static int resnums[2]   = {8, 16};
static int smnums[2]    = {1, 2};

//
// MENU ITEMS
//
CP_MenuNames SoundSetupMenuNames[] =
   {
   "SETUP SOUND FX",
   "SETUP MUSIC",
   "SOUND FX VOLUME",
   "MUSIC VOLUME",
   "QUIT"
   };

CP_iteminfo SoundSetupMenuItems  = { MENU_X, 48, 5, STARTITEM, 32, SoundSetupMenuNames, mn_largefont };
CP_itemtype SoundSetupMenu[] =
   {
      { CP_CursorLocation, "\0",  'S', (menuptr)SS_SoundMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_MusicMenu },
      { CP_Active,         "\0",  'S', (menuptr)FXVolume },
      { CP_Active,         "\0",  'M', (menuptr)MusicVolume },
      { CP_Active,         "\0",  'Q', (menuptr)SS_Quit }
   };

CP_MenuNames SoundSetupMusicNames[] =
   {
   "None",
   "Ultrasound",
   "Sound Blaster",
   "Sound Man 16",
   "Pro Audio Spectrum",
   "AWE 32",
   "Soundscape",
   "Wave Blaster",
   "General Midi",
   "Sound Canvas",
   "Adlib"
   };

CP_iteminfo SoundSetupMusicItems  = { 44, MENU_Y + 15, 11, STARTITEM, 16, SoundSetupMusicNames, mn_smallfont };
CP_itemtype SoundSetupMusic[] =
   {
      { CP_CursorLocation, "\0",  'N', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'U', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'P', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'A', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'W', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'G', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupMusicCardMenu },
      { CP_Active,         "\0",  'A', (menuptr)SS_SetupMusicCardMenu }
   };


CP_MenuNames SoundSetupSoundNames[] =
   {
   "None",
   "Ultrasound",
   "Sound Blaster",
   "Sound Man 16",
   "Pro Audio Spectrum",
   "AWE 32",
   "Soundscape",
   "Adlib",
   "Disney Sound Source",
   "Tandy Sound Source",
   "PC Speaker"
   };

CP_iteminfo SoundSetupSoundItems  = { MENU_X, MENU_Y + 15, 11, STARTITEM, 16, SoundSetupSoundNames, mn_smallfont };
CP_itemtype SoundSetupSound[] =
   {
      { CP_CursorLocation, "\0",  'N', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'U', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'P', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'A', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'A', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'S', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'T', (menuptr)SS_SetupSoundCardMenu },
      { CP_Active,         "\0",  'P', (menuptr)SS_SetupSoundCardMenu }
   };

CP_MenuNames SoundSetupMidiPortNames[] =
   {
   "220", "230", "240", "250", "300", "320",
   "330", "332", "334", "336", "340", "360"
   };

CP_iteminfo SoundSetupMidiPortItems = { 108, MENU_Y + 10, 12, STARTITEM, 16, SoundSetupMidiPortNames, mn_smallfont };
CP_itemtype SoundSetupMidiPort[] =
   {
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_CursorLocation, "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL }
   };

CP_MenuNames SoundSetupVoiceNames[] =
   {
   "1 voice",
   "2 voices",
   "3 voices",
   "4 voices",
   "5 voices",
   "6 voices",
   "7 voices",
   "8 voices"
   };

CP_iteminfo SoundSetupVoiceItems  = { MENU_X + 36, 24, 8, STARTITEM, 32, SoundSetupVoiceNames, mn_largefont };
CP_itemtype SoundSetupVoice[] =
   {
      { CP_CursorLocation, "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL }
   };

CP_MenuNames SoundSetupChannelNames[] =
   {
   "  Mono Sound FX",
   "Stereo Sound FX"
   };

CP_iteminfo SoundSetupChannelItems  = { MENU_X, MP_Y, 2, STARTITEM, 32, SoundSetupChannelNames, mn_largefont };
CP_itemtype SoundSetupChannel[] =
   {
      { CP_CursorLocation, "\0",  'M', NULL },
      { CP_Active,         "\0",  'S', NULL }
   };

CP_MenuNames SoundSetupResolutionNames[] =
   {
   "8 Bit Mixing",
   "16 Bit Mixing"
   };

CP_iteminfo SoundSetupResolutionItems  = { MENU_X, MP_Y, 2, STARTITEM, 32, SoundSetupResolutionNames, mn_largefont };
CP_itemtype SoundSetupResolution[] =
   {
      { CP_CursorLocation, "\0",  'a', NULL },
      { CP_Active,         "\0",  'b', NULL }
   };

static int typenums[ 5 ] =
   {
   fx_SB, fx_SB20, fx_SBPro, fx_SBPro2, fx_SB16
   };

static char typetostring[ 6 ] =
   {
   5, 0, 2, 1, 3, 4
   };

#define UNDEFINED -1

CP_MenuNames TypeNames[] =
   {
   "Sound Blaster or compatible",
   "Sound Blaster 2.0",
   "Sound Blaster Pro (old)",
   "Sound Blaster Pro 2.0 (new)",
   "Sound Blaster 16 or AWE32",
   "Undefined"
   };

CP_iteminfo TypeItems  = { MENU_X - 13, MENU_Y + 36, 5, STARTITEM, 10, TypeNames, mn_smallfont };
CP_itemtype TypeMenu[] =
   {
      { CP_CursorLocation, "\0",  'S', NULL },
      { CP_Active,         "\0",  'S', NULL },
      { CP_Active,         "\0",  'S', NULL },
      { CP_Active,         "\0",  'S', NULL },
      { CP_Active,         "\0",  'S', NULL }
   };

static int portnums[ 7 ] =
   {
   0x210, 0x220, 0x230, 0x240, 0x250, 0x260, 0x280
   };

CP_MenuNames PortNames[] =
   {
   "210", "220", "230", "240", "250", "260", "280"
   };

CP_iteminfo PortItems = { 92, 32, 7, STARTITEM, 32, PortNames, mn_largefont };
CP_itemtype PortMenu [] =
   {
      { CP_CursorLocation, "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL },
      { CP_Active,         "\0",  '2', NULL }
   };

static int _8BitDMAnums[ 3 ] =
   {
   0, 1, 3
   };

CP_MenuNames _8BitDMANames[] =
   {
   "DMA channel 0",
   "DMA channel 1",
   "DMA channel 3"
   };

CP_iteminfo _8BitDMAItems = { 32, 60, 3, STARTITEM, 32, _8BitDMANames, mn_largefont };
CP_itemtype _8BitDMAMenu [] =
   {
      { CP_CursorLocation, "\0",  '0', NULL },
      { CP_Active,         "\0",  '1', NULL },
      { CP_Active,         "\0",  '3', NULL }
   };

static int _16BitDMAnums[ 3 ] =
   {
   5, 6, 7
   };

CP_MenuNames _16BitDMANames[] =
   {
   "DMA channel 5",
   "DMA channel 6",
   "DMA channel 7"
   };

CP_iteminfo _16BitDMAItems = { 32, 60, 3, STARTITEM, 32, _16BitDMANames, mn_largefont };
CP_itemtype _16BitDMAMenu [] =
   {
      { CP_CursorLocation, "\0",  '5', NULL },
      { CP_Active,         "\0",  '6', NULL },
      { CP_Active,         "\0",  '7', NULL }
   };

static int irqnums[ 8 ] =
   {
   2, 3, 5, 7, 10, 11, 12, 15
   };

CP_MenuNames IrqNames[] =
   {
   "IRQ 2",
   "IRQ 3",
   "IRQ 5",
   "IRQ 7",
   "IRQ 10",
   "IRQ 11",
   "IRQ 12",
   "IRQ 15"
   };

CP_iteminfo IrqItems = { 82, 24, 8, STARTITEM, 32, IrqNames, mn_largefont };
CP_itemtype IrqMenu [] =
   {
      { CP_CursorLocation, "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL },
      { CP_Active,         "\0",  'a', NULL }
   };

CP_MenuNames SBSetupNames[] =
   {
   "Use Current Settings",
   "Custom Setup"
   };

CP_iteminfo SBSetupItems  = { MENU_X - 11, MENU_Y + 76, 2, STARTITEM, 19, SBSetupNames, mn_largefont };
CP_itemtype SBSetupMenu[] =
   {
      { CP_CursorLocation, "\0",  'U', SS_SetupSoundBlaster },
      { CP_Active,         "\0",  'C', (menuptr)SS_SBTypeMenu },
   };

extern fx_blaster_config SBSettings;

//******************************************************************************
//
// SS_DrawSBSetupMenu()
//
//******************************************************************************

void SS_DrawSBSetupMenu
   (
   void
   )

   {
   char text[ 80 ];
   char num[ 10 ];
   char *undefined;

   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Sound Blaster Setup" );

   WindowW = 288;
   WindowH = 158;
   WindowX = 96;
   WindowY = 32;
   PrintX  = 0;
   PrintY  = 32;

   CurrentFont = tinyfont;

   undefined = "Undefined";

   SBSetupMenu[ 0 ].active = CP_Active;

   PrintBattleOption( true, WindowX + 16, PrintY, "Current Settings:" );
   PrintY += 9;

   strcpy( text, "Card Type : " );

   if ( ( SBSettings.Type < fx_SB ) || ( SBSettings.Type > fx_SB16 ) )
      {
      SBSettings.Type = UNDEFINED;
      strcat( text, undefined );
      SBSetupMenu[ 0 ].active = CP_Inactive;
      }
   else
      {
	  strcat( text, TypeNames[ (unsigned int)typetostring[ SBSettings.Type ] ] );
      }

   PrintBattleOption( true, WindowX, PrintY, text );
   PrintY += 6;

   strcpy( text, "Port : " );
   if ( SBSettings.Address != (unsigned long)UNDEFINED )
      {
      itoa( SBSettings.Address, num, 16 );
      strcat( text, num );
      }
   else
      {
      strcat( text, undefined );
      SBSetupMenu[ 0 ].active = CP_Inactive;
      }

   PrintBattleOption( true, WindowX + 19, PrintY, text );
   PrintY += 6;

   strcpy( text, "IRQ : " );
   if ( SBSettings.Interrupt != (unsigned long)UNDEFINED )
      {
      itoa( SBSettings.Interrupt, num, 10 );
      strcat( text, num );
      }
   else
      {
      strcat( text, undefined );
      SBSetupMenu[ 0 ].active = CP_Inactive;
      }
   PrintBattleOption( true, WindowX + 23, PrintY, text );
   PrintY += 6;

   strcpy( text, "DMA : " );
   if ( SBSettings.Dma8 != (unsigned long)UNDEFINED )
      {
      itoa( SBSettings.Dma8, num, 10 );
      strcat( text, num );
      }
   else
      {
      strcat( text, undefined );
      SBSetupMenu[ 0 ].active = CP_Inactive;
      }
   PrintBattleOption( true, WindowX + 21, PrintY, text );
   PrintY += 6;

   strcpy( text, "16-Bit DMA : " );
   if ( SBSettings.Dma16 != (unsigned long)UNDEFINED )
      {
      itoa( SBSettings.Dma16, num, 10 );
      strcat( text, num );
      }
   else
      {
      strcat( text, undefined );
      }
   PrintBattleOption( true, WindowX - 4, PrintY, text );
   PrintY += 6;

   MN_GetCursorLocation( &SBSetupItems, &SBSetupMenu[ 0 ] );
   DrawMenu( &SBSetupItems, &SBSetupMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   RefreshMenuBuf( 0 );
   }


//******************************************************************************
//
// SS_DrawSBTypeMenu()
//
//******************************************************************************

void SS_DrawSBTypeMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select Sound Blaster Type" );
   MN_GetActive( &TypeItems, &TypeMenu[ 0 ], SBSettings.Type, typenums );
   DrawMenu( &TypeItems, &TypeMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_SBTypeMenu()
//
//******************************************************************************

void SS_SBTypeMenu
   (
   void
   )

   {
   int which;

   do
      {
      SS_DrawSBTypeMenu();
      which = HandleMenu( &TypeItems, &TypeMenu[0], NULL );
      if ( which >= 0 )
         {
         SBSettings.Type = typenums[ which ];
         SS_PortMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   handlewhich = -2;
   }

//******************************************************************************
//
// SS_DrawPortMenu()
//
//******************************************************************************

void SS_DrawPortMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select Sound Blaster Port" );
   MN_GetActive( &PortItems, &PortMenu[ 0 ], SBSettings.Address, portnums );
   DrawMenu( &PortItems, &PortMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_PortMenu()
//
//******************************************************************************

void SS_PortMenu
   (
   void
   )

   {
   int which;

   do
      {
      SS_DrawPortMenu();
      which = HandleMenu( &PortItems, &PortMenu[0], NULL );
      if ( which >= 0 )
         {
         SBSettings.Address = portnums[ which ];
         SBSettings.Emu = SBSettings.Address;
         SS_8BitDMAMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   handlewhich = -1;
   }

//******************************************************************************
//
// SS_Draw8BitDMAMenu()
//
//******************************************************************************

void SS_Draw8BitDMAMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select 8-bit DMA Channel" );
   MN_GetActive( &_8BitDMAItems, &_8BitDMAMenu[ 0 ], SBSettings.Dma8, _8BitDMAnums );
   DrawMenu( &_8BitDMAItems, &_8BitDMAMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_8BitDMAMenu()
//
//******************************************************************************

void SS_8BitDMAMenu
   (
   void
   )

   {
   int which;

   do
      {
      SS_Draw8BitDMAMenu();
      which = HandleMenu( &_8BitDMAItems, &_8BitDMAMenu[0], NULL );
      if ( which >= 0 )
         {
         SBSettings.Dma8 = _8BitDMAnums[ which ];
         SS_16BitDMAMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   handlewhich = -1;
   }

//******************************************************************************
//
// SS_Draw16BitDMAMenu()
//
//******************************************************************************

void SS_Draw16BitDMAMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select 16-bit DMA Channel" );
   MN_GetActive( &_16BitDMAItems, &_16BitDMAMenu[ 0 ], SBSettings.Dma16, _16BitDMAnums );
   DrawMenu( &_16BitDMAItems, &_16BitDMAMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_16BitDMAMenu()
//
//******************************************************************************

void SS_16BitDMAMenu
   (
   void
   )

   {
   int which;

   if ( SBSettings.Type != fx_SB16 )
      {
      SS_IrqMenu();
      return;
      }

   do
      {
      SS_Draw16BitDMAMenu();
      which = HandleMenu( &_16BitDMAItems, &_16BitDMAMenu[0], NULL );
      if ( which >= 0 )
         {
         SBSettings.Dma16 = _16BitDMAnums[ which ];
         SS_IrqMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   handlewhich = -1;
   }

//******************************************************************************
//
// SS_DrawIrqMenu()
//
//******************************************************************************

void SS_DrawIrqMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select Sound Blaster IRQ" );
   MN_GetActive( &IrqItems, &IrqMenu[ 0 ], SBSettings.Interrupt, irqnums );
   DrawMenu( &IrqItems, &IrqMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_IrqMenu()
//
//******************************************************************************

void SS_IrqMenu
   (
   void
   )

   {
   int which;

   do
      {
      SS_DrawIrqMenu();
      which = HandleMenu( &IrqItems, &IrqMenu[0], NULL );

      if ( which >= 0 )
         {
         SBSettings.Interrupt = irqnums[ which ];
         SS_SetupSoundBlaster( true );

         if ( handlewhich == -1 )
            {
            continue;
            }

         return;
         }
      }
   while( which >= 0 );

   handlewhich = -1;
   }
#endif


//******************************************************************************
//
// GetNextWord()
//
//******************************************************************************
char *GetNextWord
   (
   char *dest,
   char *source,
   int  length
   )

   {
   while( ( *source != 0 ) && ( isspace( *source ) ) && ( length > 0 ) )
      {
      *dest = *source;
      length--;
      dest++;
      source++;
      }

   while( ( *source != 0 ) && ( !isspace( *source ) ) && ( length > 0 ) )
      {
      *dest = *source;
      length--;
      dest++;
      source++;
      }

   *dest = 0;
   return( source );
   }

//******************************************************************************
//
// CP_ErrorMsg()
//
//******************************************************************************

void CP_ErrorMsg
   (
   char *title,
   char *error,
   int font
   )

   {
   char wordtext[ 80 ];
   char text[ 10 ][ 80 ];
   int  pos;
   int  length;
   int  line;
   int  w;
   int  h;
   int  y;
   extern void VWL_MeasureIntensityString (char *s, int *width, int *height, cfont_t *font);

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( title );

   WindowW = 288;
   WindowH = 158;
   WindowX = 0;
   WindowY = 40;

   IFont = ( cfont_t * )W_CacheLumpName( FontNames[ font ], PU_CACHE, Cvt_cfont_t, 1 );

   pos = 0;
   line = 0;

   text[ 0 ][ 0 ] = 0;

   while( *error != 0 )
      {
      error = GetNextWord( wordtext, error, 79 );

      pos = 0;
      while( ( wordtext[ pos ] != 0 ) && ( isspace( wordtext[ pos ] ) ) )
         {
         pos++;
         }

      length = strlen( text[ line ] );
      if ( length == 0 )
         {
         strcat( text[ line ], &wordtext[ pos ] );
         }
      else
         {
         strcat( text[ line ], wordtext );
         }

      VWL_MeasureIntensityString( text[ line ], &w, &h, IFont );

      if ( w > WindowW - 32 )
         {
         text[ line ][ length ] = 0;
         if ( line >= 10 )
            {
            break;
            }
         line++;
         strcpy( text[ line ], &wordtext[ pos ] );
         }
      }

   if ( strlen( text[ line ] ) == 0 )
      {
      line--;
      }

   VWL_MeasureIntensityString( text[ 0 ], &w, &h, IFont );

   y = ( WindowH - ( line + 1 ) * h ) / 2;

   for( pos = 0; pos <= line; pos++ )
      {
      VWL_MeasureIntensityString( text[ pos ], &w, &h, IFont );
      DrawMenuBufIString( ( 288 - w ) / 2, y, text[ pos ], ACTIVECOLOR );
      y += h;
      }

   DisplayInfo( 5 );
   FlipMenuBuf();
   RefreshMenuBuf( 0 );

   IN_Ack();
   WaitKeyUp();

   MN_PlayMenuSnd( SD_ESCPRESSEDSND );
   }

#ifdef DOS

//****************************************************************************
//
// DrawSoundSetupMainMenu ()
//
//****************************************************************************
void DrawSoundSetupMainMenu
   (
   void
   )

   {
   MenuNum = 1;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Sound Setup" );
   if ( FXMode == 0 )
      {
      SoundSetupMenu[ 2 ].active = CP_Inactive;
      }
   else if ( SoundSetupMenu[ 2 ].active == CP_Inactive )
      {
      SoundSetupMenu[ 2 ].active = CP_Active;
      }

   if ( MusicMode == 0 )
      {
      SoundSetupMenu[ 3 ].active = CP_Inactive;
      }
   else if ( SoundSetupMenu[ 3 ].active == CP_Inactive )
      {
      SoundSetupMenu[ 3 ].active = CP_Active;
      }

   MN_GetCursorLocation( &SoundSetupMenuItems, &SoundSetupMenu[ 0 ] );
   DrawMenu( &SoundSetupMenuItems, &SoundSetupMenu[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }


//******************************************************************************
//
// Sound Setup Quit
//
//******************************************************************************

void SS_Quit
   (
   void
   )

   {
   extern boolean WriteSoundFile;

   CP_DisplayMsg( "Do you wish to\nsave your changes?\n"
      "Press ESC to return\nto sound setup.", 12 );

   if ( CP_Acknowledge != CP_ESC )
      {
      WriteSoundFile = true;
      if ( CP_Acknowledge == CP_NO )
         {
         WriteSoundFile = false;
         }

      MU_FadeOut(310);
      VL_FadeOut (0, 255, 0, 0, 0, 10);
      ShutdownMenuBuf();
      QuitGame ();
      }

   DrawSoundSetupMainMenu();
   }

//******************************************************************************
//
// Sound Setup
//
//******************************************************************************

void CP_SoundSetup ( void )
   {
   int which;
   extern boolean WriteSoundFile;

   WriteSoundFile = false;

   SetupMenuBuf();
   DrawSoundSetupMainMenu();
   IN_ClearKeysDown();

   if ( MusicMode != 0 )
      {
      MU_StartSong( song_title );
      }

   while( 1 )
      {
      which = HandleMenu( &SoundSetupMenuItems, SoundSetupMenu, NULL );
      if ( which == -1 )
         {
         SS_Quit();
         }
      }
   }

//******************************************************************************
//
// SS_MusicMenu ()
//
//******************************************************************************

void SS_MusicMenu
   (
   void
   )

   {
   int which;

   MN_MakeActive( &SoundSetupMusicItems, &SoundSetupMusic[ 0 ], MusicMode );

   SS_DrawMusicMenu ();
   IN_ClearKeysDown();

   do
      {
      which = HandleMenu( &SoundSetupMusicItems, &SoundSetupMusic[ 0 ], NULL );
      }
   while( which > 0 );

   DrawSoundSetupMainMenu();
   handlewhich = RETURNVAL;
   }


//******************************************************************************
//
// SS_DrawMusicMenu ()
//
//******************************************************************************

void SS_DrawMusicMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select Music Card" );

   MN_GetCursorLocation( &SoundSetupMusicItems, &SoundSetupMusic[ 0 ] );
   DrawMenu( &SoundSetupMusicItems, &SoundSetupMusic[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }


//******************************************************************************
//
// SS_SetupMusicCardMenu()
//
//******************************************************************************

void SS_SetupMusicCardMenu
   (
   void
   )

   {
   int status;
   int select;
   if ( ( ( musicnums[ handlewhich ] == SoundBlaster ) ||
      ( musicnums[ handlewhich ] == WaveBlaster ) ||
      ( musicnums[ handlewhich ] == Awe32 ) ) &&
      ( fxnums[ FXMode ] != SoundBlaster ) &&
      ( fxnums[ FXMode ] != Awe32 ) )
      {
      CP_ErrorMsg( "Music Error",
         "Please set your Sound FX card to Sound Blaster before "
         "selecting this card for music.", mn_smallfont );

      handlewhich = -1;
      return;
      }

   MusicMode = handlewhich;

   MU_Shutdown();

   if ( ( musicnums[ MusicMode ] == GenMidi ) ||
      ( musicnums[ MusicMode ] == SoundCanvas ) ||
      ( musicnums[ MusicMode ] == WaveBlaster ) ||
      ( musicnums[ MusicMode ] == Awe32 ) )
      {
      DrawMusicCardMenu();

      IN_ClearKeysDown ();

      select = HandleMenu( &SoundSetupMidiPortItems,
         &SoundSetupMidiPort[0], NULL );

      if ( select > -1 )
         {
         MidiAddress = midinums[ select ];
         }
      else
         {
         SS_DrawMusicMenu ();
         handlewhich = RETURNVAL;
         MusicMode = 0;
         return;
         }
      }

   if ( MusicMode != 0 )
      {
      if ( ( musicnums[ handlewhich ] == WaveBlaster ) ||
         ( musicnums[ handlewhich ] == Awe32 ) )
         {
         SD_Shutdown();
         }

      SetAlternateMenuBuf();
      ClearMenuBuf();
      SetMenuTitle( "Music Card Initialization" );
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 65;
      newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1 );
      CurrentFont = newfont1;
      MenuBufCPrint( "Initializing card.\nPlease wait." );
      FlipMenuBuf();
      RefreshMenuBuf( 0 );

      status = MU_Startup( false );
      if ( status )
         {
         MusicMode = 0;

         CP_ErrorMsg( "Music Error", MUSIC_ErrorString( MUSIC_Error ),
            mn_smallfont );
         SS_DrawMusicMenu ();
         handlewhich = RETURNVAL;
         }
      else
         {
         handlewhich = -1;
         MU_StartSong( song_title );
         }

      if ( ( musicnums[ MusicMode ] == WaveBlaster ) ||
         ( musicnums[ MusicMode ] == Awe32 ) )
         {
         status = SD_Startup( false );
            if ( status != FX_Ok )
            {
            CP_ErrorMsg( "Sound FX Error", FX_ErrorString( FX_Error ),
               mn_smallfont );

            handlewhich = -2;
            }
         }
      }
   else
      {
      handlewhich = -1;
      MU_Shutdown();
      }
   }


//******************************************************************************
//
// DrawMusicCardMenu()
//
//******************************************************************************

void DrawMusicCardMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;
   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle( "Select MIDI Port Address" );
   MN_GetActive( &SoundSetupMidiPortItems, &SoundSetupMidiPort[ 0 ],
      MidiAddress, midinums );
   DrawMenu( &SoundSetupMidiPortItems, &SoundSetupMidiPort[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_SoundMenu ()
//
//******************************************************************************

void SS_SoundMenu
   (
   void
   )

   {
   int which;

   MN_MakeActive( &SoundSetupSoundItems, &SoundSetupSound[0], FXMode );

   SS_DrawSoundMenu ();
   IN_ClearKeysDown();

   do {
      which = HandleMenu( &SoundSetupSoundItems, &SoundSetupSound[ 0 ], NULL );
      }
   while( which >= 0 );

   DrawSoundSetupMainMenu();
   handlewhich = RETURNVAL;
   }


//******************************************************************************
//
// SS_DrawSoundMenu ()
//
//******************************************************************************

void SS_DrawSoundMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Select Sound FX Card");

   MN_GetCursorLocation( &SoundSetupSoundItems, &SoundSetupSound[0] );

   DrawMenu( &SoundSetupSoundItems, &SoundSetupSound[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }


//******************************************************************************
//
// SS_SetupSoundBlaster()
//
//******************************************************************************

void SS_SetupSoundBlaster
   (
   int sbmenu
   )

   {
   int status;

   status = FX_SetupSoundBlaster( SBSettings, &MaxVoices,
      &MaxBits, &MaxChannels );

   if ( status == FX_Ok )
      {
      SS_VoiceMenu( sbmenu );

      if ( handlewhich < 0 )
         {
         return;
         }
      SetAlternateMenuBuf();
      ClearMenuBuf();
      SetMenuTitle( "Sound Card Initialization" );
      WindowW = 288;
      WindowH = 158;
      PrintX = WindowX = 0;
      PrintY = WindowY = 65;
      newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1 );
      CurrentFont = newfont1;
      MenuBufCPrint( "Initializing card.\nPlease wait." );
      FlipMenuBuf();
      RefreshMenuBuf( 0 );

      status = SD_Startup( false );

      RefreshMenuBuf( 0 );
      }

   if ( status != FX_Ok )
      {
      CP_ErrorMsg( "Sound FX Error", FX_ErrorString( FX_Error ),
         mn_smallfont );

      handlewhich = -2;
      }
   }


//******************************************************************************
//
// SS_SetupSoundCardMenu()
//
//******************************************************************************

void SS_SetupSoundCardMenu
   (
   void
   )

   {
   int which;
   int FXstatus;
   extern int fxnums[];

   INFXSETUP = true;

   FXMode = handlewhich;

   SD_Shutdown();

   if ( FXMode == 0 )
      {
      handlewhich = -1;
      }
   else
      {
      if ( ( fxnums[ FXMode ] == SoundBlaster ) ||
         ( fxnums[ FXMode ] == Awe32 ) )
         {
         do
            {
            SS_DrawSBSetupMenu();
            which = HandleMenu( &SBSetupItems, &SBSetupMenu[ 0 ], NULL );
            }
         while( which == -2 );

         if ( which == -1 )
            {
            FXMode = 0;
            handlewhich = 0;
            SS_DrawSoundMenu();
            }
         else
            {
            handlewhich = -1;
            }

         INFXSETUP = false;
         return;
         }

      FXstatus = SD_SetupFXCard( &MaxVoices, &MaxBits, &MaxChannels );
      if ( FXstatus == FX_Ok )
         {
         SS_VoiceMenu( true );

         if ( handlewhich == -1 )
            {
            SS_DrawSoundMenu ();
            handlewhich = RETURNVAL;
            FXMode = 0;
            SD_Shutdown ();
            INFXSETUP = false;
            return;
            }

         SetAlternateMenuBuf();
         ClearMenuBuf();
         SetMenuTitle( "Sound Card Initialization" );
         WindowW = 288;
         WindowH = 158;
         PrintX = WindowX = 0;
         PrintY = WindowY = 65;
         newfont1 = (font_t *)W_CacheLumpName( "newfnt1", PU_CACHE, Cvt_font_t, 1 );
         CurrentFont = newfont1;
         MenuBufCPrint( "Initializing card.\nPlease wait." );
         FlipMenuBuf();
         RefreshMenuBuf( 0 );

         FXstatus = SD_Startup( false );

         RefreshMenuBuf( 0 );
         }

      if ( FXstatus != FX_Ok )
         {
         FXMode = 0;
         handlewhich = RETURNVAL;

         CP_ErrorMsg( "Sound FX Error", FX_ErrorString( FX_Error ),
            mn_smallfont );
         SS_DrawSoundMenu();
         }
      else
         {
         handlewhich = -1;
         }
      }

   INFXSETUP = false;
   }


//******************************************************************************
//
// SS_VoiceMenu ()
//
//******************************************************************************

void SS_VoiceMenu
   (
   int sbmenu
   )

   {
   int which;

   if ( MaxVoices < 2 )
      {
      NumVoices = 1;
      SS_ChannelMenu();
      return;
      }

   do
      {
      SS_DrawVoiceMenu ();

      which = HandleMenu( &SoundSetupVoiceItems, &SoundSetupVoice[0],
         NULL );
      if ( which >= 0 )
         {
         NumVoices = voicenums[ which ];
         SS_ChannelMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   if ( sbmenu )
      {
      handlewhich = -1;
      }
   else
      {
      handlewhich = -2;
      }
   }

//******************************************************************************
//
// SS_DrawVoiceMenu()
//
//******************************************************************************

void SS_DrawVoiceMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Number of Voices");

   MN_GetActive( &SoundSetupVoiceItems, &SoundSetupVoice[0],
      NumVoices, voicenums );

   DrawMenu( &SoundSetupVoiceItems, &SoundSetupVoice[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_ChannelMenu ()
//
//******************************************************************************

void SS_ChannelMenu
   (
   void
   )

   {
   int which;

   if ( MaxChannels < 2 )
      {
      NumChannels = 1;
      SS_BitMenu();
      return;
      }

   do
      {
      SS_DrawChannelMenu ();
      which = HandleMenu( &SoundSetupChannelItems, &SoundSetupChannel[0],
         NULL );
      if ( which >= 0 )
         {
         NumChannels = smnums[ which ];
         SS_BitMenu();
         if ( handlewhich == -1 )
            {
            continue;
            }
         return;
         }
      }
   while( which >= 0 );

   handlewhich = -1;
   }

//******************************************************************************
//
// SS_DrawChannelMenu()
//
//******************************************************************************

void SS_DrawChannelMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Number of Channels");

   MN_GetActive( &SoundSetupChannelItems, &SoundSetupChannel[0],
      NumChannels, smnums );

   DrawMenu( &SoundSetupChannelItems, &SoundSetupChannel[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }

//******************************************************************************
//
// SS_BitMenu ()
//
//******************************************************************************

void SS_BitMenu
   (
   void
   )

   {
   int which;

   if ( MaxBits < 16 )
      {
      NumBits = 8;
      return;
      }

   SS_DrawBitMenu ();

   which = HandleMenu( &SoundSetupResolutionItems,
      &SoundSetupResolution[0], NULL );

   if ( which >= 0 )
      {
      NumBits = resnums[ which ];
      return;
      }

   handlewhich = -1;
   }


//******************************************************************************
//
// SS_DrawBitMenu()
//
//******************************************************************************

void SS_DrawBitMenu
   (
   void
   )

   {
   MenuNum = SNDCARDS;

   SetAlternateMenuBuf();
   ClearMenuBuf();
   SetMenuTitle ("Playback Resolution");

   MN_GetActive( &SoundSetupResolutionItems, &SoundSetupResolution[0],
      NumBits, resnums );

   DrawMenu( &SoundSetupResolutionItems, &SoundSetupResolution[ 0 ] );
   DisplayInfo( 0 );

   FlipMenuBuf();
   }
#endif
