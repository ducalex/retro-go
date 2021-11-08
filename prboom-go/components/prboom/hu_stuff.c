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
 * DESCRIPTION:  Heads-up displays
 *
 *-----------------------------------------------------------------------------
 */

// killough 5/3/98: remove unnecessary headers

#include "doomstat.h"
#include "hu_stuff.h"
#include "hu_lib.h"
#include "st_stuff.h" /* jff 2/16/98 need loc of status bar */
#include "w_wad.h"
#include "s_sound.h"
#include "dstrings.h"
#include "sounds.h"
#include "d_deh.h"   /* Ty 03/27/98 - externalization of mapnamesx arrays */
#include "g_game.h"
#include "r_main.h"

// global heads up display controls

int hud_active;       //jff 2/17/98 controls heads-up display mode
int hud_displayed;    //jff 2/23/98 turns heads-up display on/off
int hud_nosecrets;    //jff 2/18/98 allows secrets line to be disabled in HUD
int hud_distributed;  //jff 3/4/98 display HUD in different places on screen
int hud_graph_keys=1; //jff 3/7/98 display HUD keys as graphics

//
// Locally used constants, shortcuts.
//
// Ty 03/28/98 -
// These four shortcuts modifed to reflect char ** of mapnamesx[]
#define HU_TITLE  (*mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2 (*mapnames2[gamemap-1])
#define HU_TITLEP (*mapnamesp[gamemap-1])
#define HU_TITLET (*mapnamest[gamemap-1])
#define HU_TITLEHEIGHT  1
#define HU_TITLEX 0
//jff 2/16/98 change 167 to ST_Y-1
// CPhipps - changed to ST_TY
// proff - changed to 200-ST_HEIGHT for stretching
#define HU_TITLEY ((200-ST_HEIGHT) - 1 - hu_font[0].height)

//jff 2/16/98 add coord text widget coordinates
// proff - changed to SCREENWIDTH to 320 for stretching
#define HU_COORDX (320 - 13*hu_font2['A'-HU_FONTSTART].width)
//jff 3/3/98 split coord widget into three lines in upper right of screen
#define HU_COORDX_Y (1 + 0*hu_font['A'-HU_FONTSTART].height)
#define HU_COORDY_Y (2 + 1*hu_font['A'-HU_FONTSTART].height)
#define HU_COORDZ_Y (3 + 2*hu_font['A'-HU_FONTSTART].height)

//jff 2/16/98 add ammo, health, armor widgets, 2/22/98 less gap
#define HU_GAPY 8
#define HU_HUDHEIGHT (6*HU_GAPY)
#define HU_HUDX 2
#define HU_HUDY (200-HU_HUDHEIGHT-1)
#define HU_MONSECX (HU_HUDX)
#define HU_MONSECY (HU_HUDY+0*HU_GAPY)
#define HU_KEYSX   (HU_HUDX)
//jff 3/7/98 add offset for graphic key widget
#define HU_KEYSGX  (HU_HUDX+4*hu_font2['A'-HU_FONTSTART].width)
#define HU_KEYSY   (HU_HUDY+1*HU_GAPY)
#define HU_WEAPX   (HU_HUDX)
#define HU_WEAPY   (HU_HUDY+2*HU_GAPY)
#define HU_AMMOX   (HU_HUDX)
#define HU_AMMOY   (HU_HUDY+3*HU_GAPY)
#define HU_HEALTHX (HU_HUDX)
#define HU_HEALTHY (HU_HUDY+4*HU_GAPY)
#define HU_ARMORX  (HU_HUDX)
#define HU_ARMORY  (HU_HUDY+5*HU_GAPY)

//jff 3/4/98 distributed HUD positions
#define HU_HUDX_LL 2
#define HU_HUDY_LL (200-2*HU_GAPY-1)
// proff/nicolas 09/20/98: Changed for high-res
#define HU_HUDX_LR (320-120)
#define HU_HUDY_LR (200-2*HU_GAPY-1)
// proff/nicolas 09/20/98: Changed for high-res
#define HU_HUDX_UR (320-96)
#define HU_HUDY_UR 2
#define HU_MONSECX_D (HU_HUDX_LL)
#define HU_MONSECY_D (HU_HUDY_LL+0*HU_GAPY)
#define HU_KEYSX_D   (HU_HUDX_LL)
#define HU_KEYSGX_D  (HU_HUDX_LL+4*hu_font2['A'-HU_FONTSTART].width)
#define HU_KEYSY_D   (HU_HUDY_LL+1*HU_GAPY)
#define HU_WEAPX_D   (HU_HUDX_LR)
#define HU_WEAPY_D   (HU_HUDY_LR+0*HU_GAPY)
#define HU_AMMOX_D   (HU_HUDX_LR)
#define HU_AMMOY_D   (HU_HUDY_LR+1*HU_GAPY)
#define HU_HEALTHX_D (HU_HUDX_UR)
#define HU_HEALTHY_D (HU_HUDY_UR+0*HU_GAPY)
#define HU_ARMORX_D  (HU_HUDX_UR)
#define HU_ARMORY_D  (HU_HUDY_UR+1*HU_GAPY)

//#define HU_INPUTTOGGLE  't' // not used                           // phares
#define HU_INPUTX HU_MSGX
#define HU_INPUTY (HU_MSGY + HU_MSGHEIGHT*(hu_font[0].height) +1)
#define HU_INPUTWIDTH 64
#define HU_INPUTHEIGHT  1

#define key_alt KEYD_RALT
#define key_shift KEYD_RSHIFT

const char* chat_macros[] =
// Ty 03/27/98 - *not* externalized
// CPhipps - const char*
{
  HUSTR_CHATMACRO0,
  HUSTR_CHATMACRO1,
  HUSTR_CHATMACRO2,
  HUSTR_CHATMACRO3,
  HUSTR_CHATMACRO4,
  HUSTR_CHATMACRO5,
  HUSTR_CHATMACRO6,
  HUSTR_CHATMACRO7,
  HUSTR_CHATMACRO8,
  HUSTR_CHATMACRO9
};

const char* player_names[] =
// Ty 03/27/98 - *not* externalized
// CPhipps - const char*
{
  HUSTR_PLRGREEN,
  HUSTR_PLRINDIGO,
  HUSTR_PLRBROWN,
  HUSTR_PLRRED
};

//jff 3/17/98 translate player colmap to text color ranges
int plyrcoltran[MAXPLAYERS]={CR_GREEN,CR_GRAY,CR_BROWN,CR_RED};

char chat_char;                 // remove later.
static player_t*  plr;

// font sets
patchnum_t hu_font[HU_FONTSIZE];
patchnum_t hu_font2[HU_FONTSIZE];
patchnum_t hu_fontk[HU_FONTSIZE];//jff 3/7/98 added for graphic key indicators
patchnum_t hu_msgbg[9];          //jff 2/26/98 add patches for message background

// widgets
static hu_textline_t  w_title;
static hu_stext_t     w_message;
static hu_itext_t     w_chat;
static hu_itext_t     w_inputbuffer[MAXPLAYERS];
static hu_textline_t  w_coordx; //jff 2/16/98 new coord widget for automap
static hu_textline_t  w_coordy; //jff 3/3/98 split coord widgets automap
static hu_textline_t  w_coordz; //jff 3/3/98 split coord widgets automap
static hu_textline_t  w_ammo;   //jff 2/16/98 new ammo widget for hud
static hu_textline_t  w_health; //jff 2/16/98 new health widget for hud
static hu_textline_t  w_armor;  //jff 2/16/98 new armor widget for hud
static hu_textline_t  w_weapon; //jff 2/16/98 new weapon widget for hud
static hu_textline_t  w_keys;   //jff 2/16/98 new keys widget for hud
static hu_textline_t  w_gkeys;  //jff 3/7/98 graphic keys widget for hud
static hu_textline_t  w_monsec; //jff 2/16/98 new kill/secret widget for hud
static hu_mtext_t     w_rtext;  //jff 2/26/98 text message refresh widget

static boolean    always_off = false;
static char       chat_dest[MAXPLAYERS];
boolean           chat_on;
static boolean    message_on;
static boolean    message_list; //2/26/98 enable showing list of messages
boolean           message_dontfuckwithme;
static boolean    message_nottobefuckedwith;
static int        message_counter;
extern int        showMessages;
extern boolean    automapactive;
static boolean    headsupactive = false;

//jff 2/16/98 hud supported automap colors added
int hudcolor_titl;  // color range of automap level title
int hudcolor_xyco;  // color range of new coords on automap
//jff 2/16/98 hud text colors, controls added
int hudcolor_mesg;  // color range of scrolling messages
int hudcolor_chat;  // color range of chat lines
int hud_msg_lines;  // number of message lines in window
//jff 2/26/98 hud text colors, controls added
int hudcolor_list;  // list of messages color
int hud_list_bgon;  // enable for solid window background for message list

//jff 2/16/98 initialization strings for ammo, health, armor widgets
static char hud_coordstrx[32];
static char hud_coordstry[32];
static char hud_coordstrz[32];
static char hud_ammostr[80];
static char hud_healthstr[80];
static char hud_armorstr[80];
static char hud_weapstr[80];
static char hud_keysstr[80];
static char hud_gkeysstr[80]; //jff 3/7/98 add support for graphic key display
static char hud_monsecstr[80];

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//
// Ty 03/27/98 - externalized map name arrays - now in d_deh.c
// and converted to arrays of pointers to char *
// See modified HUTITLEx macros
extern char **mapnames[];
extern char **mapnames2[];
extern char **mapnamesp[];
extern char **mapnamest[];

extern int map_point_coordinates;

// key tables
// jff 5/10/98 french support removed,
// as it was not being used and couldn't be easily tested
//
const char* shiftxform;

const char english_shiftxform[] =
{
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31,
  ' ', '!', '"', '#', '$', '%', '&',
  '"', // shift-'
  '(', ')', '*', '+',
  '<', // shift-,
  '_', // shift--
  '>', // shift-.
  '?', // shift-/
  ')', // shift-0
  '!', // shift-1
  '@', // shift-2
  '#', // shift-3
  '$', // shift-4
  '%', // shift-5
  '^', // shift-6
  '&', // shift-7
  '*', // shift-8
  '(', // shift-9
  ':',
  ':', // shift-;
  '<',
  '+', // shift-=
  '>', '?', '@',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '[', // shift-[
  '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
  ']', // shift-]
  '"', '_',
  '\'', // shift-`
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '{', '|', '}', '~', 127
};

//
// HU_Init()
//
// Initialize the heads-up display, text that overwrites the primary display
//
// Passed nothing, returns nothing
//
void HU_Init(void)
{

  int   i;
  int   j;
  char  buffer[9];

  shiftxform = english_shiftxform;

  // load the heads-up font
  j = HU_FONTSTART;
  for (i=0;i<HU_FONTSIZE;i++,j++)
  {
    if ('0'<=j && j<='9')
    {
      sprintf(buffer, "DIG%.1d",j-48);
      R_SetPatchNum(&hu_font2[i], buffer);
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if ('A'<=j && j<='Z')
    {
      sprintf(buffer, "DIG%c",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if (j=='-')
    {
      R_SetPatchNum(&hu_font2[i], "DIG45");
      R_SetPatchNum(&hu_font[i], "STCFN045");
    }
    else if (j=='/')
    {
      R_SetPatchNum(&hu_font2[i], "DIG47");
      R_SetPatchNum(&hu_font[i], "STCFN047");
    }
    else if (j==':')
    {
      R_SetPatchNum(&hu_font2[i], "DIG58");
      R_SetPatchNum(&hu_font[i], "STCFN058");
    }
    else if (j=='[')
    {
      R_SetPatchNum(&hu_font2[i], "DIG91");
      R_SetPatchNum(&hu_font[i], "STCFN091");
    }
    else if (j==']')
    {
      R_SetPatchNum(&hu_font2[i], "DIG93");
      R_SetPatchNum(&hu_font[i], "STCFN093");
    }
    else if (j<97)
    {
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      R_SetPatchNum(&hu_font[i], buffer);
      //jff 2/23/98 make all font chars defined, useful or not
    }
    else if (j>122)
    {
      sprintf(buffer, "STBR%.3d",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else
      hu_font[i] = hu_font[0]; //jff 2/16/98 account for gap
  }

  // CPhipps - load patches for message background
  for (i=0; i<9; i++) {
    sprintf(buffer, "BOX%c%c", "UCL"[i/3], "LCR"[i%3]);
    R_SetPatchNum(&hu_msgbg[i], buffer);
  }

  // CPhipps - load patches for keys and double keys
  for (i=0; i<6; i++) {
    sprintf(buffer, "STKEYS%d", i);
    R_SetPatchNum(&hu_fontk[i], buffer);
  }
}

//
// HU_Stop()
//
// Make the heads-up displays inactive
//
// Passed nothing, returns nothing
//
static void HU_Stop(void)
{
  headsupactive = false;
}

//
// HU_Start(void)
//
// Create and initialize the heads-up widgets, software machines to
// maintain, update, and display information over the primary display
//
// This routine must be called after any change to the heads up configuration
// in order for the changes to take effect in the actual displays
//
// Passed nothing, returns nothing
//
void HU_Start(void)
{

  int   i;
  const char* s; /* cph - const */

  if (headsupactive)                    // stop before starting
    HU_Stop();

  plr = &players[displayplayer];        // killough 3/7/98
  message_on = false;
  message_dontfuckwithme = false;
  message_nottobefuckedwith = false;
  chat_on = false;

  // create the message widget
  // messages to player in upper-left of screen
  HUlib_initSText
  (
    &w_message,
    HU_MSGX,
    HU_MSGY,
    HU_MSGHEIGHT,
    hu_font,
    HU_FONTSTART,
    hudcolor_mesg,
    &message_on
  );

  //jff 2/16/98 added some HUD widgets
  // create the map title widget - map title display in lower left of automap
  HUlib_initTextLine
  (
    &w_title,
    HU_TITLEX,
    HU_TITLEY,
    hu_font,
    HU_FONTSTART,
    hudcolor_titl
  );

  // create the hud health widget
  // bargraph and number for amount of health,
  // lower left or upper right of screen
  HUlib_initTextLine
  (
    &w_health,
    hud_distributed? HU_HEALTHX_D : HU_HEALTHX,  //3/4/98 distribute
    hud_distributed? HU_HEALTHY_D : HU_HEALTHY,
    hu_font2,
    HU_FONTSTART,
    CR_GREEN
  );

  // create the hud armor widget
  // bargraph and number for amount of armor,
  // lower left or upper right of screen
  HUlib_initTextLine
  (
    &w_armor,
    hud_distributed? HU_ARMORX_D : HU_ARMORX,    //3/4/98 distribute
    hud_distributed? HU_ARMORY_D : HU_ARMORY,
    hu_font2,
    HU_FONTSTART,
    CR_GREEN
  );

  // create the hud ammo widget
  // bargraph and number for amount of ammo for current weapon,
  // lower left or lower right of screen
  HUlib_initTextLine
  (
    &w_ammo,
    hud_distributed? HU_AMMOX_D : HU_AMMOX,      //3/4/98 distribute
    hud_distributed? HU_AMMOY_D : HU_AMMOY,
    hu_font2,
    HU_FONTSTART,
    CR_GOLD
  );

  // create the hud weapons widget
  // list of numbers of weapons possessed
  // lower left or lower right of screen
  HUlib_initTextLine
  (
    &w_weapon,
    hud_distributed? HU_WEAPX_D : HU_WEAPX,      //3/4/98 distribute
    hud_distributed? HU_WEAPY_D : HU_WEAPY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  // create the hud keys widget
  // display of key letters possessed
  // lower left of screen
  HUlib_initTextLine
  (
    &w_keys,
    hud_distributed? HU_KEYSX_D : HU_KEYSX,      //3/4/98 distribute
    hud_distributed? HU_KEYSY_D : HU_KEYSY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  // create the hud graphic keys widget
  // display of key graphics possessed
  // lower left of screen
  HUlib_initTextLine
  (
    &w_gkeys,
    hud_distributed? HU_KEYSGX_D : HU_KEYSGX,    //3/4/98 distribute
    hud_distributed? HU_KEYSY_D : HU_KEYSY,
    hu_fontk,
    HU_FONTSTART,
    CR_RED
  );

  // create the hud monster/secret widget
  // totals and current values for kills, items, secrets
  // lower left of screen
  HUlib_initTextLine
  (
    &w_monsec,
    hud_distributed? HU_MONSECX_D : HU_MONSECX,  //3/4/98 distribute
    hud_distributed? HU_MONSECY_D : HU_MONSECY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  // create the hud text refresh widget
  // scrolling display of last hud_msg_lines messages received
  if (hud_msg_lines>HU_MAXMESSAGES)
    hud_msg_lines=HU_MAXMESSAGES;
  //jff 4/21/98 if setup has disabled message list while active, turn it off
  message_list = hud_msg_lines > 1; //jff 8/8/98 initialize both ways
  //jff 2/26/98 add the text refresh widget initialization
  HUlib_initMText
  (
    &w_rtext,
    0,
    0,
    320,
//    SCREENWIDTH,
    (hud_msg_lines+2)*HU_REFRESHSPACING,
    hu_font,
    HU_FONTSTART,
    hudcolor_list,
    hu_msgbg,
    &message_list
  );

  // initialize the automap's level title widget
  if (gamestate == GS_LEVEL) /* cph - stop SEGV here when not in level */
  switch (gamemode)
  {
    case shareware:
    case registered:
    case retail:
      s = HU_TITLE;
      break;

    case commercial:
    default:  // Ty 08/27/98 - modified to check mission for TNT/Plutonia
      s = (gamemission==pack_tnt)  ? HU_TITLET :
          (gamemission==pack_plut) ? HU_TITLEP : HU_TITLE2;
      break;
  } else s = "";
  while (*s)
    HUlib_addCharToTextLine(&w_title, *(s++));

  // create the automaps coordinate widget
  // jff 3/3/98 split coord widget into three lines: x,y,z
  // jff 2/16/98 added
  HUlib_initTextLine
  (
    &w_coordx,
    HU_COORDX,
    HU_COORDX_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );
  HUlib_initTextLine
  (
    &w_coordy,
    HU_COORDX,
    HU_COORDY_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );
  HUlib_initTextLine
  (
    &w_coordz,
    HU_COORDX,
    HU_COORDZ_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );

  // initialize the automaps coordinate widget
  //jff 3/3/98 split coordstr widget into 3 parts
  if (map_point_coordinates)
  {
    sprintf(hud_coordstrx,"X: %-5d",0); //jff 2/22/98 added z
    s = hud_coordstrx;
    while (*s)
      HUlib_addCharToTextLine(&w_coordx, *(s++));
    sprintf(hud_coordstry,"Y: %-5d",0); //jff 3/3/98 split x,y,z
    s = hud_coordstry;
    while (*s)
      HUlib_addCharToTextLine(&w_coordy, *(s++));
    sprintf(hud_coordstrz,"Z: %-5d",0); //jff 3/3/98 split x,y,z
    s = hud_coordstrz;
    while (*s)
      HUlib_addCharToTextLine(&w_coordz, *(s++));
  }

  //jff 2/16/98 initialize ammo widget
  strcpy(hud_ammostr,"AMM ");
  s = hud_ammostr;
  while (*s)
    HUlib_addCharToTextLine(&w_ammo, *(s++));

  //jff 2/16/98 initialize health widget
  strcpy(hud_healthstr,"HEL ");
  s = hud_healthstr;
  while (*s)
    HUlib_addCharToTextLine(&w_health, *(s++));

  //jff 2/16/98 initialize armor widget
  strcpy(hud_armorstr,"ARM ");
  s = hud_armorstr;
  while (*s)
    HUlib_addCharToTextLine(&w_armor, *(s++));

  //jff 2/17/98 initialize weapons widget
  strcpy(hud_weapstr,"WEA ");
  s = hud_weapstr;
  while (*s)
    HUlib_addCharToTextLine(&w_weapon, *(s++));

  //jff 2/17/98 initialize keys widget
  if (!deathmatch) //jff 3/17/98 show frags in deathmatch mode
    strcpy(hud_keysstr,"KEY ");
  else
    strcpy(hud_keysstr,"FRG ");
  s = hud_keysstr;
  while (*s)
    HUlib_addCharToTextLine(&w_keys, *(s++));

  //jff 2/17/98 initialize graphic keys widget
  strcpy(hud_gkeysstr," ");
  s = hud_gkeysstr;
  while (*s)
    HUlib_addCharToTextLine(&w_gkeys, *(s++));

  //jff 2/17/98 initialize kills/items/secret widget
  strcpy(hud_monsecstr,"STS ");
  s = hud_monsecstr;
  while (*s)
    HUlib_addCharToTextLine(&w_monsec, *(s++));

  // create the chat widget
  HUlib_initIText
  (
    &w_chat,
    HU_INPUTX,
    HU_INPUTY,
    hu_font,
    HU_FONTSTART,
    hudcolor_chat,
    &chat_on
  );

  // create the inputbuffer widgets, one per player
  for (i=0 ; i<MAXPLAYERS ; i++)
    HUlib_initIText
    (
      &w_inputbuffer[i],
      0,
      0,
      0,
      0,
      hudcolor_chat,
      &always_off
    );

  // now allow the heads-up display to run
  headsupactive = true;
}

//
// HU_MoveHud()
//
// Move the HUD display from distributed to compact mode or vice-versa
//
// Passed nothing, returns nothing
//
//jff 3/9/98 create this externally callable to avoid glitch
// when menu scatter's HUD due to delay in change of position
//
void HU_MoveHud(void)
{
  static int ohud_distributed=-1;

  //jff 3/4/98 move displays around on F5 changing hud_distributed
  if (hud_distributed!=ohud_distributed)
  {
    w_ammo.x =    hud_distributed? HU_AMMOX_D   : HU_AMMOX;
    w_ammo.y =    hud_distributed? HU_AMMOY_D   : HU_AMMOY;
    w_weapon.x =  hud_distributed? HU_WEAPX_D   : HU_WEAPX;
    w_weapon.y =  hud_distributed? HU_WEAPY_D   : HU_WEAPY;
    w_keys.x =    hud_distributed? HU_KEYSX_D   : HU_KEYSX;
    w_keys.y =    hud_distributed? HU_KEYSY_D   : HU_KEYSY;
    w_gkeys.x =   hud_distributed? HU_KEYSGX_D  : HU_KEYSGX;
    w_gkeys.y =   hud_distributed? HU_KEYSY_D   : HU_KEYSY;
    w_monsec.x =  hud_distributed? HU_MONSECX_D : HU_MONSECX;
    w_monsec.y =  hud_distributed? HU_MONSECY_D : HU_MONSECY;
    w_health.x =  hud_distributed? HU_HEALTHX_D : HU_HEALTHX;
    w_health.y =  hud_distributed? HU_HEALTHY_D : HU_HEALTHY;
    w_armor.x =   hud_distributed? HU_ARMORX_D  : HU_ARMORX;
    w_armor.y =   hud_distributed? HU_ARMORY_D  : HU_ARMORY;
  }
  ohud_distributed = hud_distributed;
}

//
// HU_Drawer()
//
// Draw all the pieces of the heads-up display
//
// Passed nothing, returns nothing
//
void HU_Drawer(void)
{
  char *s;
  player_t *plr;
  char ammostr[80];  //jff 3/8/98 allow plenty room for dehacked mods
  char healthstr[80];//jff
  char armorstr[80]; //jff
  int i,doit;

  plr = &players[displayplayer];         // killough 3/7/98
  // draw the automap widgets if automap is displayed
  if (automapmode & am_active)
  {
    // map title
    HUlib_drawTextLine(&w_title, false);

    //jff 2/16/98 output new coord display
    // x-coord
    if (map_point_coordinates)
    {
      sprintf(hud_coordstrx,"X: %-5d", (plr->mo->x)>>FRACBITS);
      HUlib_clearTextLine(&w_coordx);
      s = hud_coordstrx;
      while (*s)
        HUlib_addCharToTextLine(&w_coordx, *(s++));
      HUlib_drawTextLine(&w_coordx, false);

      //jff 3/3/98 split coord display into x,y,z lines
      // y-coord
      sprintf(hud_coordstry,"Y: %-5d", (plr->mo->y)>>FRACBITS);
      HUlib_clearTextLine(&w_coordy);
      s = hud_coordstry;
      while (*s)
        HUlib_addCharToTextLine(&w_coordy, *(s++));
      HUlib_drawTextLine(&w_coordy, false);

      //jff 3/3/98 split coord display into x,y,z lines
      //jff 2/22/98 added z
      // z-coord
      sprintf(hud_coordstrz,"Z: %-5d", (plr->mo->z)>>FRACBITS);
      HUlib_clearTextLine(&w_coordz);
      s = hud_coordstrz;
      while (*s)
        HUlib_addCharToTextLine(&w_coordz, *(s++));
      HUlib_drawTextLine(&w_coordz, false);
    }
  }

  // draw the weapon/health/ammo/armor/kills/keys displays if optioned
  //jff 2/17/98 allow new hud stuff to be turned off
  // killough 2/21/98: really allow new hud stuff to be turned off COMPLETELY
  if
  (
    hud_active>0 &&                  // hud optioned on
    hud_displayed &&                 // hud on from fullscreen key
    viewheight==SCREENHEIGHT &&      // fullscreen mode is active
    !(automapmode & am_active)       // automap is not active
  )
  {
    doit = !(gametic&1); //jff 3/4/98 speed update up for slow systems
    if (doit)            //jff 8/7/98 update every time, avoid lag in update
    {
      HU_MoveHud();                  // insure HUD display coords are correct

      // do the hud ammo display
      // clear the widgets internal line
      HUlib_clearTextLine(&w_ammo);
      strcpy(hud_ammostr,"AMM ");
      if (weaponinfo[plr->readyweapon].ammo == am_noammo)
      { // special case for weapon with no ammo selected - blank bargraph + N/A
        strcat(hud_ammostr,"\x7f\x7f\x7f\x7f\x7f\x7f\x7f N/A");
        w_ammo.cm = CR_GRAY;
      }
      else
      {
        int ammo = plr->ammo[weaponinfo[plr->readyweapon].ammo];
        int fullammo = plr->maxammo[weaponinfo[plr->readyweapon].ammo];
        int ammopct = (100*ammo)/fullammo;
        int ammobars = ammopct/4;

        // build the numeric amount init string
        sprintf(ammostr,"%d/%d",ammo,fullammo);
        // build the bargraph string
        // full bargraph chars
        for (i=4;i<4+ammobars/4;)
          hud_ammostr[i++] = 123;
        // plus one last character with 0,1,2,3 bars
        switch(ammobars%4)
        {
          case 0:
            break;
          case 1:
            hud_ammostr[i++] = 126;
            break;
          case 2:
            hud_ammostr[i++] = 125;
            break;
          case 3:
            hud_ammostr[i++] = 124;
            break;
        }
        // pad string with blank bar characters
        while(i<4+7)
          hud_ammostr[i++] = 127;
        hud_ammostr[i] = '\0';
        strcat(hud_ammostr,ammostr);

        // set the display color from the percentage of total ammo held
        if (ammopct<ammo_red)
          w_ammo.cm = CR_RED;
        else if (ammopct<ammo_yellow)
          w_ammo.cm = CR_GOLD;
        else
          w_ammo.cm = CR_GREEN;
      }
      // transfer the init string to the widget
      s = hud_ammostr;
      while (*s)
        HUlib_addCharToTextLine(&w_ammo, *(s++));
    }
    // display the ammo widget every frame
    HUlib_drawTextLine(&w_ammo, false);

    // do the hud health display
    if (doit)
    {
      int health = plr->health;
      int healthbars = health>100? 25 : health/4;

      // clear the widgets internal line
      HUlib_clearTextLine(&w_health);

      // build the numeric amount init string
      sprintf(healthstr,"%3d",health);
      // build the bargraph string
      // full bargraph chars
      for (i=4;i<4+healthbars/4;)
        hud_healthstr[i++] = 123;
      // plus one last character with 0,1,2,3 bars
      switch(healthbars%4)
      {
        case 0:
          break;
        case 1:
          hud_healthstr[i++] = 126;
          break;
        case 2:
          hud_healthstr[i++] = 125;
          break;
        case 3:
          hud_healthstr[i++] = 124;
          break;
      }
      // pad string with blank bar characters
      while(i<4+7)
        hud_healthstr[i++] = 127;
      hud_healthstr[i] = '\0';
      strcat(hud_healthstr,healthstr);

      // set the display color from the amount of health posessed
      if (health<health_red)
        w_health.cm = CR_RED;
      else if (health<health_yellow)
        w_health.cm = CR_GOLD;
      else if (health<=health_green)
        w_health.cm = CR_GREEN;
      else
        w_health.cm = CR_BLUE;

      // transfer the init string to the widget
      s = hud_healthstr;
      while (*s)
        HUlib_addCharToTextLine(&w_health, *(s++));
    }
    // display the health widget every frame
    HUlib_drawTextLine(&w_health, false);

    // do the hud armor display
    if (doit)
    {
      int armor = plr->armorpoints;
      int armorbars = armor>100? 25 : armor/4;

      // clear the widgets internal line
      HUlib_clearTextLine(&w_armor);
      // build the numeric amount init string
      sprintf(armorstr,"%3d",armor);
      // build the bargraph string
      // full bargraph chars
      for (i=4;i<4+armorbars/4;)
        hud_armorstr[i++] = 123;
      // plus one last character with 0,1,2,3 bars
      switch(armorbars%4)
      {
        case 0:
          break;
        case 1:
          hud_armorstr[i++] = 126;
          break;
        case 2:
          hud_armorstr[i++] = 125;
          break;
        case 3:
          hud_armorstr[i++] = 124;
          break;
      }
      // pad string with blank bar characters
      while(i<4+7)
        hud_armorstr[i++] = 127;
      hud_armorstr[i] = '\0';
      strcat(hud_armorstr,armorstr);

      // set the display color from the amount of armor posessed
      if (armor<armor_red)
        w_armor.cm = CR_RED;
      else if (armor<armor_yellow)
        w_armor.cm = CR_GOLD;
      else if (armor<=armor_green)
        w_armor.cm = CR_GREEN;
      else
        w_armor.cm = CR_BLUE;

      // transfer the init string to the widget
      s = hud_armorstr;
      while (*s)
        HUlib_addCharToTextLine(&w_armor, *(s++));
    }
    // display the armor widget every frame
    HUlib_drawTextLine(&w_armor, false);

    // do the hud weapon display
    if (doit)
    {
      int w;
      int ammo,fullammo,ammopct;

      // clear the widgets internal line
      HUlib_clearTextLine(&w_weapon);
      i=4; hud_weapstr[i] = '\0';      //jff 3/7/98 make sure ammo goes away

      // do each weapon that exists in current gamemode
      for (w=0;w<=wp_supershotgun;w++) //jff 3/4/98 show fists too, why not?
      {
        int ok=1;
        //jff avoid executing for weapons that do not exist
        switch (gamemode)
        {
          case shareware:
            if (w>=wp_plasma && w!=wp_chainsaw)
              ok=0;
            break;
          case retail:
          case registered:
            if (w>=wp_supershotgun)
              ok=0;
            break;
          default:
          case commercial:
            break;
        }
        if (!ok) continue;

        ammo = plr->ammo[weaponinfo[w].ammo];
        fullammo = plr->maxammo[weaponinfo[w].ammo];
        ammopct=0;

        // skip weapons not currently posessed
        if (!plr->weaponowned[w])
          continue;

        ammopct = fullammo? (100*ammo)/fullammo : 100;

        // display each weapon number in a color related to the ammo for it
        hud_weapstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
        if (weaponinfo[w].ammo==am_noammo) //jff 3/14/98 show berserk on HUD
          hud_weapstr[i++] = plr->powers[pw_strength]? '0'+CR_GREEN : '0'+CR_GRAY;
        else if (ammopct<ammo_red)
          hud_weapstr[i++] = '0'+CR_RED;
        else if (ammopct<ammo_yellow)
          hud_weapstr[i++] = '0'+CR_GOLD;
        else
          hud_weapstr[i++] = '0'+CR_GREEN;
        hud_weapstr[i++] = '0'+w+1;
        hud_weapstr[i++] = ' ';
        hud_weapstr[i] = '\0';
      }

      // transfer the init string to the widget
      s = hud_weapstr;
      while (*s)
        HUlib_addCharToTextLine(&w_weapon, *(s++));
    }
    // display the weapon widget every frame
    HUlib_drawTextLine(&w_weapon, false);

    if (doit && hud_active>1)
    {
      int k;

      hud_keysstr[4] = '\0';    //jff 3/7/98 make sure deleted keys go away
      //jff add case for graphic key display
      if (!deathmatch && hud_graph_keys)
      {
        i=0;
        hud_gkeysstr[i] = '\0'; //jff 3/7/98 init graphic keys widget string
        // build text string whose characters call out graphic keys from fontk
        for (k=0;k<6;k++)
        {
          // skip keys not possessed
          if (!plr->cards[k])
            continue;

          hud_gkeysstr[i++] = '!'+k;   // key number plus '!' is char for key
          hud_gkeysstr[i++] = ' ';     // spacing
          hud_gkeysstr[i++] = ' ';
        }
        hud_gkeysstr[i]='\0';
      }
      else // not possible in current code, unless deathmatching,
      {
        i=4;
        hud_keysstr[i] = '\0';  //jff 3/7/98 make sure deleted keys go away

        // if deathmatch, build string showing top four frag counts
        if (deathmatch) //jff 3/17/98 show frags, not keys, in deathmatch
        {
          int top1=-999,top2=-999,top3=-999,top4=-999;
          int idx1=-1,idx2=-1,idx3=-1,idx4=-1;
          int fragcount,m;
          char numbuf[32];

          // scan thru players
          for (k=0;k<MAXPLAYERS;k++)
          {
            // skip players not in game
            if (!playeringame[k])
              continue;

            fragcount = 0;
            // compute number of times they've fragged each player
            // minus number of times they've been fragged by them
            for (m=0;m<MAXPLAYERS;m++)
            {
              if (!playeringame[m]) continue;
              fragcount += (m!=k)?  players[k].frags[m] : -players[k].frags[m];
            }

            // very primitive sort of frags to find top four
            if (fragcount>top1)
            {
              top4=top3; top3=top2; top2 = top1; top1=fragcount;
              idx4=idx3; idx3=idx2; idx2 = idx1; idx1=k;
            }
            else if (fragcount>top2)
            {
              top4=top3; top3=top2; top2=fragcount;
              idx4=idx3; idx3=idx2; idx2=k;
            }
            else if (fragcount>top3)
            {
              top4=top3; top3=fragcount;
              idx4=idx3; idx3=k;
            }
            else if (fragcount>top4)
            {
              top4=fragcount;
              idx4=k;
            }
          }
          // if the biggest number exists, put it in the init string
          if (idx1>-1)
          {
            sprintf(numbuf,"%5d",top1);
            // make frag count in player's color via escape code
            hud_keysstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
            hud_keysstr[i++] = '0'+plyrcoltran[idx1&3];
            s = numbuf;
            while (*s)
              hud_keysstr[i++] = *(s++);
          }
          // if the second biggest number exists, put it in the init string
          if (idx2>-1)
          {
            sprintf(numbuf,"%5d",top2);
            // make frag count in player's color via escape code
            hud_keysstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
            hud_keysstr[i++] = '0'+plyrcoltran[idx2&3];
            s = numbuf;
            while (*s)
              hud_keysstr[i++] = *(s++);
          }
          // if the third biggest number exists, put it in the init string
          if (idx3>-1)
          {
            sprintf(numbuf,"%5d",top3);
            // make frag count in player's color via escape code
            hud_keysstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
            hud_keysstr[i++] = '0'+plyrcoltran[idx3&3];
            s = numbuf;
            while (*s)
              hud_keysstr[i++] = *(s++);
          }
          // if the fourth biggest number exists, put it in the init string
          if (idx4>-1)
          {
            sprintf(numbuf,"%5d",top4);
            // make frag count in player's color via escape code
            hud_keysstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
            hud_keysstr[i++] = '0'+plyrcoltran[idx4&3];
            s = numbuf;
            while (*s)
              hud_keysstr[i++] = *(s++);
          }
          hud_keysstr[i] = '\0';
        } //jff 3/17/98 end of deathmatch clause
        else // build alphabetical key display (not used currently)
        {
          // scan the keys
          for (k=0;k<6;k++)
          {
            // skip any not possessed by the displayed player's stats
            if (!plr->cards[k])
              continue;

            // use color escapes to make text in key's color
            hud_keysstr[i++] = '\x1b'; //jff 3/26/98 use ESC not '\' for paths
            switch(k)
            {
              case 0:
                hud_keysstr[i++] = '0'+CR_BLUE;
                hud_keysstr[i++] = 'B';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 1:
                hud_keysstr[i++] = '0'+CR_GOLD;
                hud_keysstr[i++] = 'Y';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 2:
                hud_keysstr[i++] = '0'+CR_RED;
                hud_keysstr[i++] = 'R';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 3:
                hud_keysstr[i++] = '0'+CR_BLUE;
                hud_keysstr[i++] = 'B';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
            case 4:
                hud_keysstr[i++] = '0'+CR_GOLD;
                hud_keysstr[i++] = 'Y';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
              case 5:
                hud_keysstr[i++] = '0'+CR_RED;
                hud_keysstr[i++] = 'R';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
            }
            hud_keysstr[i]='\0';
          }
        }
      }
    }
    // display the keys/frags line each frame
    if (hud_active>1)
    {
      HUlib_clearTextLine(&w_keys);      // clear the widget strings
      HUlib_clearTextLine(&w_gkeys);

      // transfer the built string (frags or key title) to the widget
      s = hud_keysstr; //jff 3/7/98 display key titles/key text or frags
      while (*s)
        HUlib_addCharToTextLine(&w_keys, *(s++));
      HUlib_drawTextLine(&w_keys, false);

      //jff 3/17/98 show graphic keys in non-DM only
      if (!deathmatch) //jff 3/7/98 display graphic keys
      {
        // transfer the graphic key text to the widget
        s = hud_gkeysstr;
        while (*s)
          HUlib_addCharToTextLine(&w_gkeys, *(s++));
        // display the widget
        HUlib_drawTextLine(&w_gkeys, false);
      }
    }

    // display the hud kills/items/secret display if optioned
    if (!hud_nosecrets)
    {
      if (hud_active>1 && doit)
      {
        // clear the internal widget text buffer
        HUlib_clearTextLine(&w_monsec);
        //jff 3/26/98 use ESC not '\' for paths
        // build the init string with fixed colors
        sprintf
        (
          hud_monsecstr,
          "STS \x1b\x36K \x1b\x33%d \x1b\x36M \x1b\x33%d \x1b\x37I \x1b\x33%d/%d \x1b\x35S \x1b\x33%d/%d",
          plr->killcount,totallive,
          plr->itemcount,totalitems,
          plr->secretcount,totalsecret
        );
        // transfer the init string to the widget
        s = hud_monsecstr;
        while (*s)
          HUlib_addCharToTextLine(&w_monsec, *(s++));
      }
      // display the kills/items/secrets each frame, if optioned
      if (hud_active>1)
        HUlib_drawTextLine(&w_monsec, false);
    }
  }

  //jff 3/4/98 display last to give priority
  HU_Erase(); // jff 4/24/98 Erase current lines before drawing current
              // needed when screen not fullsize

  //jff 4/21/98 if setup has disabled message list while active, turn it off
  if (hud_msg_lines<=1)
    message_list = false;

  // if the message review not enabled, show the standard message widget
  if (!message_list)
    HUlib_drawSText(&w_message);

  // if the message review is enabled show the scrolling message review
  if (hud_msg_lines>1 && message_list)
    HUlib_drawMText(&w_rtext);

  // display the interactive buffer for chat entry
  HUlib_drawIText(&w_chat);
}

//
// HU_Erase()
//
// Erase hud display lines that can be trashed by small screen display
//
// Passed nothing, returns nothing
//
void HU_Erase(void)
{
  // erase the message display or the message review display
  if (!message_list)
    HUlib_eraseSText(&w_message);
  else
    HUlib_eraseMText(&w_rtext);

  // erase the interactive text buffer for chat entry
  HUlib_eraseIText(&w_chat);

  // erase the automap title
  HUlib_eraseTextLine(&w_title);
}

//
// HU_Ticker()
//
// Update the hud displays once per frame
//
// Passed nothing, returns nothing
//
static boolean bsdown; // Is backspace down?
static int bscounter;

void HU_Ticker(void)
{
  int i, rc;
  char c;

  // tick down message counter if message is up
  if (message_counter && !--message_counter)
  {
    message_on = false;
    message_nottobefuckedwith = false;
  }
  if (bsdown && bscounter++ > 9) {
    HUlib_keyInIText(&w_chat, (unsigned char)key_backspace);
    bscounter = 8;
  }

  // if messages on, or "Messages Off" is being displayed
  // this allows the notification of turning messages off to be seen
  if (showMessages || message_dontfuckwithme)
  {
    // display message if necessary
    if ((plr->message && !message_nottobefuckedwith)
        || (plr->message && message_dontfuckwithme))
    {
      //post the message to the message widget
      HUlib_addMessageToSText(&w_message, 0, plr->message);
      //jff 2/26/98 add message to refresh text widget too
      HUlib_addMessageToMText(&w_rtext, 0, plr->message);

      // clear the message to avoid posting multiple times
      plr->message = 0;
      // note a message is displayed
      message_on = true;
      // start the message persistence counter
      message_counter = HU_MSGTIMEOUT;
      // transfer "Messages Off" exception to the "being displayed" variable
      message_nottobefuckedwith = message_dontfuckwithme;
      // clear the flag that "Messages Off" is being posted
      message_dontfuckwithme = 0;
    }
  }

  // check for incoming chat characters
  if (netgame)
  {
    for (i=0; i<MAXPLAYERS; i++)
    {
      if (!playeringame[i])
        continue;
      if (i != consoleplayer
          && (c = players[i].cmd.chatchar))
      {
        if (c <= HU_BROADCAST)
          chat_dest[i] = c;
        else
        {
          if (c >= 'a' && c <= 'z')
            c = (char) shiftxform[(unsigned char) c];
          rc = HUlib_keyInIText(&w_inputbuffer[i], c);
          if (rc && c == KEYD_ENTER)
          {
            if (w_inputbuffer[i].l.len
                && (chat_dest[i] == consoleplayer+1
                || chat_dest[i] == HU_BROADCAST))
            {
              HUlib_addMessageToSText(&w_message,
                                      player_names[i],
                                      w_inputbuffer[i].l.l);

              message_nottobefuckedwith = true;
              message_on = true;
              message_counter = HU_MSGTIMEOUT;
              if ( gamemode == commercial )
                S_StartSound(0, sfx_radio);
              else
                S_StartSound(0, sfx_tink);
            }
            HUlib_resetIText(&w_inputbuffer[i]);
          }
        }
        players[i].cmd.chatchar = 0;
      }
    }
  }
}

#define QUEUESIZE   128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;

//
// HU_queueChatChar()
//
// Add an incoming character to the circular chat queue
//
// Passed the character to queue, returns nothing
//
static void HU_queueChatChar(char c)
{
  if (((head + 1) & (QUEUESIZE-1)) == tail)
  {
    plr->message = HUSTR_MSGU;
  }
  else
  {
    chatchars[head] = c;
    head = (head + 1) & (QUEUESIZE-1);
  }
}

//
// HU_dequeueChatChar()
//
// Remove the earliest added character from the circular chat queue
//
// Passed nothing, returns the character dequeued
//
char HU_dequeueChatChar(void)
{
  char c;

  if (head != tail)
  {
    c = chatchars[tail];
    tail = (tail + 1) & (QUEUESIZE-1);
  }
  else
  {
    c = 0;
  }
  return c;
}

//
// HU_Responder()
//
// Responds to input events that affect the heads up displays
//
// Passed the event to respond to, returns true if the event was handled
//
boolean HU_Responder(event_t *ev)
{

  static char   lastmessage[HU_MAXLINELENGTH+1];
  const char*   macromessage; // CPhipps - const char*
  boolean   eatkey = false;
  static boolean  shiftdown = false;
  static boolean  altdown = false;
  unsigned char   c;
  int     i;
  int     numplayers;

  static int    num_nobrainers = 0;

  numplayers = 0;
  for (i=0 ; i<MAXPLAYERS ; i++)
    numplayers += playeringame[i];

  if (ev->data1 == key_shift)
  {
    shiftdown = ev->type == ev_keydown;
    return false;
  }
  else if (ev->data1 == key_alt)
  {
    altdown = ev->type == ev_keydown;
    return false;
  }
  else if (ev->data1 == key_backspace)
  {
    bsdown = ev->type == ev_keydown;
    bscounter = 0;
  }

  if (ev->type != ev_keydown)
    return false;

  if (!chat_on)
  {
    if (ev->data1 == key_enter)                                 // phares
    {
#ifndef INSTRUMENTED  // never turn on message review if INSTRUMENTED defined
      if (hud_msg_lines>1)  // it posts multi-line messages that will trash
      {
        if (message_list) HU_Erase(); //jff 4/28/98 erase behind messages
        message_list = !message_list; //jff 2/26/98 toggle list of messages
      }
#endif
      if (!message_list)              // if not message list, refresh message
      {
        message_on = true;
        message_counter = HU_MSGTIMEOUT;
      }
      eatkey = true;
    }//jff 2/26/98 no chat if message review is displayed
    // killough 10/02/98: no chat if demo playback
    // no chat in -solo-net mode
    else if (!demoplayback && !message_list && netgame && numplayers > 1)
    {
      if (ev->data1 == key_chat)
    {
      eatkey = chat_on = true;
      HUlib_resetIText(&w_chat);
      HU_queueChatChar(HU_BROADCAST);
    }
    else if (numplayers > 2)
    {
      for (i=0; i<MAXPLAYERS ; i++)
      {
        if (ev->data1 == destination_keys[i])
        {
          if (playeringame[i] && i!=consoleplayer)
          {
            eatkey = chat_on = true;
            HUlib_resetIText(&w_chat);
            HU_queueChatChar((char)(i+1));
            break;
          }
          else if (i == consoleplayer)
          {
            num_nobrainers++;
            if (num_nobrainers < 3)
                plr->message = HUSTR_TALKTOSELF1;
            else if (num_nobrainers < 6)
                plr->message = HUSTR_TALKTOSELF2;
            else if (num_nobrainers < 9)
                plr->message = HUSTR_TALKTOSELF3;
            else if (num_nobrainers < 32)
                plr->message = HUSTR_TALKTOSELF4;
            else
                plr->message = HUSTR_TALKTOSELF5;
          }
        }
      }
    }
    }
  }//jff 2/26/98 no chat functions if message review is displayed
  else if (!message_list)
  {
    c = ev->data1;
    // send a macro
    if (altdown)
    {
      c = c - '0';
      if (c > 9)
        return false;
      macromessage = chat_macros[c];

      // kill last message with a '\n'
        HU_queueChatChar((char)key_enter); // DEBUG!!!                // phares

      // send the macro message
      while (*macromessage)
        HU_queueChatChar(*macromessage++);
      HU_queueChatChar((char)key_enter);                            // phares

      // leave chat mode and notify that it was sent
      chat_on = false;
      strcpy(lastmessage, chat_macros[c]);
      plr->message = lastmessage;
      eatkey = true;
    }
    else
    {
      if (shiftdown || (c >= 'a' && c <= 'z'))
        c = shiftxform[c];
      eatkey = HUlib_keyInIText(&w_chat, c);
      if (eatkey)
        HU_queueChatChar(c);

      if (c == key_enter)                                     // phares
      {
        chat_on = false;
        if (w_chat.l.len)
        {
          strcpy(lastmessage, w_chat.l.l);
          plr->message = lastmessage;
        }
      }
      else if (c == key_escape)                               // phares
        chat_on = false;
    }
  }
  return eatkey;
}
