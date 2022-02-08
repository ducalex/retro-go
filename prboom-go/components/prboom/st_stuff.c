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
 *      Status bar code.
 *      Does the face/direction indicator animatin.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "i_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_main.h"
#include "am_map.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "sounds.h"
#include "dstrings.h"
#include "r_draw.h"

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// Location of status bar
#define ST_X                    0
#define ST_X2                   104

// proff 08/18/98: Changed for high-res
#define ST_FX                   (ST_X+143)
#define ST_FY                   (ST_Y+1)
//#define ST_FX                   143
//#define ST_FY                   169

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

// proff 08/18/98: Changed for high-res
#define ST_FACESX               (ST_X+143)
#define ST_FACESY               (ST_Y)
//#define ST_FACESX               143
//#define ST_FACESY               168

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?
// I dunno, why don't you go and find out!!!  killough

// AMMO number pos.
#define ST_AMMOWIDTH            3
// proff 08/18/98: Changed for high-res
#define ST_AMMOX                (ST_X+44)
#define ST_AMMOY                (ST_Y+3)
//#define ST_AMMOX                44
//#define ST_AMMOY                171

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
// proff 08/18/98: Changed for high-res
#define ST_HEALTHX              (ST_X+90)
#define ST_HEALTHY              (ST_Y+3)
//#define ST_HEALTHX              90
//#define ST_HEALTHY              171

// Weapon pos.
// proff 08/18/98: Changed for high-res
#define ST_ARMSX                (ST_X+111)
#define ST_ARMSY                (ST_Y+4)
#define ST_ARMSBGX              (ST_X+104)
#define ST_ARMSBGY              (ST_Y)
//#define ST_ARMSX                111
//#define ST_ARMSY                172
//#define ST_ARMSBGX              104
//#define ST_ARMSBGY              168
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
// proff 08/18/98: Changed for high-res
#define ST_FRAGSX               (ST_X+138)
#define ST_FRAGSY               (ST_Y+3)
//#define ST_FRAGSX               138
//#define ST_FRAGSY               171
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
// proff 08/18/98: Changed for high-res
#define ST_ARMORX               (ST_X+221)
#define ST_ARMORY               (ST_Y+3)
//#define ST_ARMORX               221
//#define ST_ARMORY               171

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
// proff 08/18/98: Changed for high-res
#define ST_KEY0X                (ST_X+239)
#define ST_KEY0Y                (ST_Y+3)
//#define ST_KEY0X                239
//#define ST_KEY0Y                171
#define ST_KEY1WIDTH            ST_KEY0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_KEY1X                (ST_X+239)
#define ST_KEY1Y                (ST_Y+13)
//#define ST_KEY1X                239
//#define ST_KEY1Y                181
#define ST_KEY2WIDTH            ST_KEY0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_KEY2X                (ST_X+239)
#define ST_KEY2Y                (ST_Y+23)
//#define ST_KEY2X                239
//#define ST_KEY2Y                191

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
// proff 08/18/98: Changed for high-res
#define ST_AMMO0X               (ST_X+288)
#define ST_AMMO0Y               (ST_Y+5)
//#define ST_AMMO0X               288
//#define ST_AMMO0Y               173
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO1X               (ST_X+288)
#define ST_AMMO1Y               (ST_Y+11)
//#define ST_AMMO1X               288
//#define ST_AMMO1Y               179
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO2X               (ST_X+288)
#define ST_AMMO2Y               (ST_Y+23)
//#define ST_AMMO2X               288
//#define ST_AMMO2Y               191
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO3X               (ST_X+288)
#define ST_AMMO3Y               (ST_Y+17)
//#define ST_AMMO3X               288
//#define ST_AMMO3Y               185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO0X            (ST_X+314)
#define ST_MAXAMMO0Y            (ST_Y+5)
//#define ST_MAXAMMO0X            314
//#define ST_MAXAMMO0Y            173
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO1X            (ST_X+314)
#define ST_MAXAMMO1Y            (ST_Y+11)
//#define ST_MAXAMMO1X            314
//#define ST_MAXAMMO1Y            179
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO2X            (ST_X+314)
#define ST_MAXAMMO2Y            (ST_Y+23)
//#define ST_MAXAMMO2X            314
//#define ST_MAXAMMO2Y            191
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO3X            (ST_X+314)
#define ST_MAXAMMO3Y            (ST_Y+17)
//#define ST_MAXAMMO3X            314
//#define ST_MAXAMMO3Y            185

// killough 2/8/98: weapon info position macros UNUSED, removed here

// main player in game
static player_t *plyr;

// ST_Start() has just been called
static boolean st_firsttime;

// used to execute ST_Init() only once
static int veryfirsttime = 1;

// CPhipps - no longer do direct PLAYPAL handling here

// used for timing
static unsigned int st_clock;

// used for making messages go away
static int st_msgcounter=0;

// used when in chat
static st_chatstateenum_t st_chatstate;

// whether in automap or first-person
static st_stateenum_t st_gamestate;

// whether left-side main status bar is active
static boolean st_statusbaron;

// whether status bar chat is active
static boolean st_chat;

// value of st_chat before message popped up
static boolean st_oldchat;

// whether chat window has the cursor on
static boolean st_cursoron;

// !deathmatch
static boolean st_notdeathmatch;

// !deathmatch && st_statusbaron
static boolean st_armson;

// !deathmatch
static boolean st_fragson;

// 0-9, tall numbers
static patchnum_t tallnum[10];

// tall % sign
static patchnum_t tallpercent;

// 0-9, short, yellow (,different!) numbers
static patchnum_t shortnum[10];

// 3 key-cards, 3 skulls, 3 card/skull combos
// jff 2/24/98 extend number of patches by three skull/card combos
static patchnum_t keys[NUMCARDS+3];

// face status patches
static patchnum_t faces[ST_NUMFACES];

// face background
static patchnum_t faceback; // CPhipps - single background, translated for different players

//e6y: status bar background
static patchnum_t stbarbg;

// main bar right
static patchnum_t armsbg;

// weapon ownership patches
static patchnum_t arms[6][2];

// ready-weapon widget
static st_number_t w_ready;

//jff 2/16/98 status color change levels
int ammo_red;      // ammo percent less than which status is red
int ammo_yellow;   // ammo percent less is yellow more green
int health_red;    // health amount less than which status is red
int health_yellow; // health amount less than which status is yellow
int health_green;  // health amount above is blue, below is green
int armor_red;     // armor amount less than which status is red
int armor_yellow;  // armor amount less than which status is yellow
int armor_green;   // armor amount above is blue, below is green

 // in deathmatch only, summary of frags stats
static st_number_t w_frags;

// health widget
static st_percent_t w_health;

// arms background
static st_binicon_t  w_armsbg;

// weapon ownership widgets
static st_multicon_t w_arms[6];

// face status widget
static st_multicon_t w_faces;

// keycard widgets
static st_multicon_t w_keyboxes[3];

// armor widget
static st_percent_t  w_armor;

// ammo widgets
static st_number_t   w_ammo[4];

// max ammo widgets
static st_number_t   w_maxammo[4];

 // number of frags so far in deathmatch
static int      st_fragscount;

// used to use appopriately pained face
static int      st_oldhealth = -1;

// used for evil grin
static boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int      st_facecount = 0;

// current face index, used by w_faces
static int      st_faceindex = 0;

// holds key-type for each key box on bar
static int      keyboxes[3];

// a random number per tick
static int      st_randomnumber;

//
// STATUS BAR CODE
//

static void ST_Stop(void);

static void ST_refreshBackground(void)
{
  int y=0;

  if (st_statusbaron)
    {
      V_DrawNumPatch(ST_X, y, BG, stbarbg.lumpnum, CR_DEFAULT, VPT_STRETCH);
      if (st_armson)
        V_DrawNumPatch(ST_ARMSBGX, y, BG, armsbg.lumpnum, CR_DEFAULT, VPT_STRETCH);

      // killough 3/7/98: make face background change with displayplayer
      if (netgame)
      {
        V_DrawNumPatch(ST_FX, y, BG, faceback.lumpnum,
           displayplayer ? CR_LIMIT+displayplayer : CR_DEFAULT,
           displayplayer ? (VPT_TRANS | VPT_STRETCH) : VPT_STRETCH);
      }
      V_CopyRect(ST_X, y, BG, ST_SCALED_WIDTH, ST_SCALED_HEIGHT, ST_X, ST_SCALED_Y, FG, VPT_NONE);
    }
}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder(event_t *ev)
{
  // Filter automap on/off.
  if (ev->type == ev_keyup && (ev->data1 & 0xffff0000) == AM_MSGHEADER)
    {
      switch(ev->data1)
        {
        case AM_MSGENTERED:
          st_gamestate = AutomapState;
          st_firsttime = true;
          break;

        case AM_MSGEXITED:
          st_gamestate = FirstPersonState;
          break;
        }
    }
  else  // if a user keypress...
    if (ev->type == ev_keydown)       // Try cheat responder in m_cheat.c
      return M_FindCheats(ev->data1); // killough 4/17/98, 5/2/98
  return false;
}

static int ST_calcPainOffset(void)
{
  static int lastcalc;
  static int oldhealth = -1;
  int health = plyr->health > 100 ? 100 : plyr->health;

  if (health != oldhealth)
    {
      lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
      oldhealth = health;
    }
  return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//

static void ST_updateFaceWidget(void)
{
  int         i;
  angle_t     badguyangle;
  angle_t     diffang;
  static int  lastattackdown = -1;
  static int  priority = 0;
  boolean     doevilgrin;

  if (priority < 10)
    {
      // dead
      if (!plyr->health)
        {
          priority = 9;
          st_faceindex = ST_DEADFACE;
          st_facecount = 1;
        }
    }

  if (priority < 9)
    {
      if (plyr->bonuscount)
        {
          // picking up bonus
          doevilgrin = false;

          for (i=0;i<NUMWEAPONS;i++)
            {
              if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                  doevilgrin = true;
                  oldweaponsowned[i] = plyr->weaponowned[i];
                }
            }
          if (doevilgrin)
            {
              // evil grin if just picked up weapon
              priority = 8;
              st_facecount = ST_EVILGRINCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }

  if (priority < 8)
    {
      if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
          // being attacked
          priority = 7;

          // haleyjd 10/12/03: classic DOOM problem of missing OUCH face
          // was due to inversion of this test:
          // if(plyr->health - st_oldhealth > ST_MUCHPAIN)
          if(st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
          else
            {
              badguyangle = R_PointToAngle2(plyr->mo->x,
                                            plyr->mo->y,
                                            plyr->attacker->x,
                                            plyr->attacker->y);

              if (badguyangle > plyr->mo->angle)
                {
                  // whether right or left
                  diffang = badguyangle - plyr->mo->angle;
                  i = diffang > ANG180;
                }
              else
                {
                  // whether left or right
                  diffang = plyr->mo->angle - badguyangle;
                  i = diffang <= ANG180;
                } // confusing, aint it?


              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset();

              if (diffang < ANG45)
                {
                  // head-on
                  st_faceindex += ST_RAMPAGEOFFSET;
                }
              else if (i)
                {
                  // turn face right
                  st_faceindex += ST_TURNOFFSET;
                }
              else
                {
                  // turn face left
                  st_faceindex += ST_TURNOFFSET+1;
                }
            }
        }
    }

  if (priority < 7)
    {
      // getting hurt because of your own damn stupidity
      if (plyr->damagecount)
        {
          // haleyjd 10/12/03: classic DOOM problem of missing OUCH face
          // was due to inversion of this test:
          // if(plyr->health - st_oldhealth > ST_MUCHPAIN)
          if(st_oldhealth - plyr->health > ST_MUCHPAIN)
            {
              priority = 7;
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
          else
            {
              priority = 6;
              st_facecount = ST_TURNCOUNT;
              st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }

    }

  if (priority < 6)
    {
      // rapid firing
      if (plyr->attackdown)
        {
          if (lastattackdown==-1)
            lastattackdown = ST_RAMPAGEDELAY;
          else if (!--lastattackdown)
            {
              priority = 5;
              st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
              st_facecount = 1;
              lastattackdown = 1;
            }
        }
      else
        lastattackdown = -1;

    }

  if (priority < 5)
    {
      // invulnerability
      if ((plyr->cheats & CF_GODMODE)
          || plyr->powers[pw_invulnerability])
        {
          priority = 4;

          st_faceindex = ST_GODFACE;
          st_facecount = 1;

        }

    }

  // look left or look right if the facecount has timed out
  if (!st_facecount)
    {
      st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
      st_facecount = ST_STRAIGHTFACECOUNT;
      priority = 0;
    }

  st_facecount--;

}

int sts_traditional_keys; // killough 2/28/98: traditional status bar keys

static void ST_updateWidgets(void)
{
  static int  largeammo = 1994; // means "n/a"
  int         i;

  // must redirect the pointer if the ready weapon has changed.
  //  if (w_ready.data != plyr->readyweapon)
  //  {
  if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
    w_ready.num = &largeammo;
  else
    w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
  //{
  // static int tic=0;
  // static int dir=-1;
  // if (!(tic&15))
  //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
  // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
  //   dir = 1;
  // tic++;
  // }
  w_ready.data = plyr->readyweapon;

  // if (*w_ready.on)
  //  STlib_updateNum(&w_ready, true);
  // refresh weapon change
  //  }

  // update keycard multiple widgets
  for (i=0;i<3;i++)
    {
      keyboxes[i] = plyr->cards[i] ? i : -1;

      //jff 2/24/98 select double key
      //killough 2/28/98: preserve traditional keys by config option

      if (plyr->cards[i+3])
        keyboxes[i] = keyboxes[i]==-1 || sts_traditional_keys ? i+3 : i+6;
    }

  // refresh everything if this is him coming back to life
  ST_updateFaceWidget();

  // used by the w_armsbg widget
  st_notdeathmatch = !deathmatch;

  // used by w_arms[] widgets
  st_armson = st_statusbaron && !deathmatch;

  // used by w_frags widget
  st_fragson = deathmatch && st_statusbaron;
  st_fragscount = 0;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      if (i != displayplayer)            // killough 3/7/98
        st_fragscount += plyr->frags[i];
      else
        st_fragscount -= plyr->frags[i];
    }

  // get rid of chat window if up because of message
  if (!--st_msgcounter)
    st_chat = st_oldchat;

}

void ST_Ticker(void)
{
  st_clock++;
  st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = plyr->health;
}

int st_palette = 0;

static void ST_doPaletteStuff(void)
{
  int         palette;
  int cnt = plyr->damagecount;

  if (plyr->powers[pw_strength])
    {
      // slowly fade the berzerk out
      int bzc = 12 - (plyr->powers[pw_strength]>>6);
      if (bzc > cnt)
        cnt = bzc;
    }

  if (cnt)
    {
      palette = (cnt+7)>>3;
      if (palette >= NUMREDPALS)
        palette = NUMREDPALS-1;

      /* cph 2006/08/06 - if in the menu, reduce the red tint - navigating to
       * load a game can be tricky if the screen is all red */
      if (menuactive) palette >>=1;

      palette += STARTREDPALS;
    }
  else
    if (plyr->bonuscount)
      {
        palette = (plyr->bonuscount+7)>>3;
        if (palette >= NUMBONUSPALS)
          palette = NUMBONUSPALS-1;
        palette += STARTBONUSPALS;
      }
    else
      if (plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet] & 8)
        palette = RADIATIONPAL;
      else
        palette = 0;

  if (palette != st_palette) {
    V_SetPalette(st_palette = palette); // CPhipps - use new palette function

    // have to redraw the entire status bar when the palette changes
    // in truecolor modes - POPE
    if (V_GetMode() != VID_MODE8)
      st_firsttime = true;
  }
}

static void ST_drawWidgets(boolean refresh)
{
  int i;

  // used by w_arms[] widgets
  st_armson = st_statusbaron && !deathmatch;

  // used by w_frags widget
  st_fragson = deathmatch && st_statusbaron;

  //jff 2/16/98 make color of ammo depend on amount
  if (*w_ready.num*100 < ammo_red*plyr->maxammo[weaponinfo[w_ready.data].ammo])
    STlib_updateNum(&w_ready, CR_RED, refresh);
  else
    if (*w_ready.num*100 <
        ammo_yellow*plyr->maxammo[weaponinfo[w_ready.data].ammo])
      STlib_updateNum(&w_ready, CR_GOLD, refresh);
    else
      STlib_updateNum(&w_ready, CR_GREEN, refresh);

  for (i=0;i<4;i++)
    {
      STlib_updateNum(&w_ammo[i], CR_DEFAULT, refresh);   //jff 2/16/98 no xlation
      STlib_updateNum(&w_maxammo[i], CR_DEFAULT, refresh);
    }

  //jff 2/16/98 make color of health depend on amount
  if (*w_health.n.num<health_red)
    STlib_updatePercent(&w_health, CR_RED, refresh);
  else if (*w_health.n.num<health_yellow)
    STlib_updatePercent(&w_health, CR_GOLD, refresh);
  else if (*w_health.n.num<=health_green)
    STlib_updatePercent(&w_health, CR_GREEN, refresh);
  else
    STlib_updatePercent(&w_health, CR_BLUE2, refresh); //killough 2/28/98

  //jff 2/16/98 make color of armor depend on amount
  if (*w_armor.n.num<armor_red)
    STlib_updatePercent(&w_armor, CR_RED, refresh);
  else if (*w_armor.n.num<armor_yellow)
    STlib_updatePercent(&w_armor, CR_GOLD, refresh);
  else if (*w_armor.n.num<=armor_green)
    STlib_updatePercent(&w_armor, CR_GREEN, refresh);
  else
    STlib_updatePercent(&w_armor, CR_BLUE2, refresh); //killough 2/28/98

  //e6y: moved to ST_refreshBackground() for correct single-pass stretching
  //STlib_updateBinIcon(&w_armsbg, refresh);

  for (i=0;i<6;i++)
    STlib_updateMultIcon(&w_arms[i], refresh);

  STlib_updateMultIcon(&w_faces, refresh);

  for (i=0;i<3;i++)
    STlib_updateMultIcon(&w_keyboxes[i], refresh);

  STlib_updateNum(&w_frags, CR_DEFAULT, refresh);

}

static void ST_doRefresh(void)
{

  st_firsttime = false;

  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets(true);

}

static void ST_diffDraw(void)
{
  // update all widgets
  ST_drawWidgets(false);
}

void ST_Drawer(boolean statusbaron, boolean refresh)
{
  /* cph - let status bar on be controlled
   * completely by the call from D_Display
   * proff - really do it
   */
  st_firsttime = st_firsttime || refresh;

  ST_doPaletteStuff();  // Do red-/gold-shifts from damage/items

  if (statusbaron) {
    if (st_firsttime)
      ST_doRefresh();     /* If just after ST_Start(), refresh all */
    else
      ST_diffDraw();      /* Otherwise, update as little as possible */
  }
}



//
// ST_loadGraphics
//
// CPhipps - Loads graphics needed for status bar if doload is true,
//  unloads them otherwise
//
static void ST_loadGraphics(boolean doload)
{
  int  i, facenum;
  char namebuf[16];
  // cph - macro that either acquires a pointer and lock for a lump, or
  // unlocks it. var is referenced exactly once in either case, so ++ in arg works

  // Load the numbers, tall and short
  for (i=0;i<10;i++)
    {
      sprintf(namebuf, "STTNUM%d", i);
      R_SetPatchNum(&tallnum[i],namebuf);
      sprintf(namebuf, "STYSNUM%d", i);
      R_SetPatchNum(&shortnum[i],namebuf);
    }

  // Load percent key.
  R_SetPatchNum(&tallpercent,"STTPRCNT");

  // key cards
  for (i=0;i<NUMCARDS+3;i++)  //jff 2/23/98 show both keys too
    {
      sprintf(namebuf, "STKEYS%d", i);
      R_SetPatchNum(&keys[i], namebuf);
    }

  //e6y: status bar background
  R_SetPatchNum(&stbarbg, "STBAR");

  // arms background
  R_SetPatchNum(&armsbg, "STARMS");

  // arms ownership widgets
  for (i=0;i<6;i++)
    {
      sprintf(namebuf, "STGNUM%d", i+2);

      // gray #
      R_SetPatchNum(&arms[i][0], namebuf);

      // yellow #
      arms[i][1] = shortnum[i+2];
    }

  // face backgrounds for different color players
  // killough 3/7/98: add better support for spy mode by loading all
  // player face backgrounds and using displayplayer to choose them:
  R_SetPatchNum(&faceback, "STFB0");

  // face states
  facenum = 0;
  for (i=0;i<ST_NUMPAINFACES;i++)
    {
      int j;
      for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
          sprintf(namebuf, "STFST%d%d", i, j);
          R_SetPatchNum(&faces[facenum++], namebuf);
        }
      sprintf(namebuf, "STFTR%d0", i);        // turn right
      R_SetPatchNum(&faces[facenum++], namebuf);
      sprintf(namebuf, "STFTL%d0", i);        // turn left
      R_SetPatchNum(&faces[facenum++], namebuf);
      sprintf(namebuf, "STFOUCH%d", i);       // ouch!
      R_SetPatchNum(&faces[facenum++], namebuf);
      sprintf(namebuf, "STFEVL%d", i);        // evil grin ;)
      R_SetPatchNum(&faces[facenum++], namebuf);
      sprintf(namebuf, "STFKILL%d", i);       // pissed off
      R_SetPatchNum(&faces[facenum++], namebuf);
    }
  R_SetPatchNum(&faces[facenum++], "STFGOD0");
  R_SetPatchNum(&faces[facenum++], "STFDEAD0");
}

static void ST_initData(void)
{
  int i;

  st_firsttime = true;
  plyr = &players[displayplayer];            // killough 3/7/98

  st_clock = 0;
  st_chatstate = StartChatState;
  st_gamestate = FirstPersonState;

  st_statusbaron = true;
  st_oldchat = st_chat = false;
  st_cursoron = false;

  st_faceindex = 0;
  st_palette = -1;

  st_oldhealth = -1;

  for (i=0;i<NUMWEAPONS;i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

  for (i=0;i<3;i++)
    keyboxes[i] = -1;

  STlib_init();
}

static void ST_createWidgets(void)
{
  int i;

  // ready weapon ammo
  STlib_initNum(&w_ready,
                ST_AMMOX,
                ST_AMMOY,
                tallnum,
                &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
                &st_statusbaron,
                ST_AMMOWIDTH );

  // the last weapon type
  w_ready.data = plyr->readyweapon;

  // health percentage
  STlib_initPercent(&w_health,
                    ST_HEALTHX,
                    ST_HEALTHY,
                    tallnum,
                    &plyr->health,
                    &st_statusbaron,
                    &tallpercent);

  // arms background
  STlib_initBinIcon(&w_armsbg,
                    ST_ARMSBGX,
                    ST_ARMSBGY,
                    &armsbg,
                    &st_notdeathmatch,
                    &st_statusbaron);

  // weapons owned
  for(i=0;i<6;i++)
    {
      STlib_initMultIcon(&w_arms[i],
                         ST_ARMSX+(i%3)*ST_ARMSXSPACE,
                         ST_ARMSY+(i/3)*ST_ARMSYSPACE,
                         arms[i], (int *) &plyr->weaponowned[i+1],
                         &st_armson);
    }

  // frags sum
  STlib_initNum(&w_frags,
                ST_FRAGSX,
                ST_FRAGSY,
                tallnum,
                &st_fragscount,
                &st_fragson,
                ST_FRAGSWIDTH);

  // faces
  STlib_initMultIcon(&w_faces,
                     ST_FACESX,
                     ST_FACESY,
                     faces,
                     &st_faceindex,
                     &st_statusbaron);

  // armor percentage - should be colored later
  STlib_initPercent(&w_armor,
                    ST_ARMORX,
                    ST_ARMORY,
                    tallnum,
                    &plyr->armorpoints,
                    &st_statusbaron, &tallpercent);

  // keyboxes 0-2
  STlib_initMultIcon(&w_keyboxes[0],
                     ST_KEY0X,
                     ST_KEY0Y,
                     keys,
                     &keyboxes[0],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[1],
                     ST_KEY1X,
                     ST_KEY1Y,
                     keys,
                     &keyboxes[1],
                     &st_statusbaron);

  STlib_initMultIcon(&w_keyboxes[2],
                     ST_KEY2X,
                     ST_KEY2Y,
                     keys,
                     &keyboxes[2],
                     &st_statusbaron);

  // ammo count (all four kinds)
  STlib_initNum(&w_ammo[0],
                ST_AMMO0X,
                ST_AMMO0Y,
                shortnum,
                &plyr->ammo[0],
                &st_statusbaron,
                ST_AMMO0WIDTH);

  STlib_initNum(&w_ammo[1],
                ST_AMMO1X,
                ST_AMMO1Y,
                shortnum,
                &plyr->ammo[1],
                &st_statusbaron,
                ST_AMMO1WIDTH);

  STlib_initNum(&w_ammo[2],
                ST_AMMO2X,
                ST_AMMO2Y,
                shortnum,
                &plyr->ammo[2],
                &st_statusbaron,
                ST_AMMO2WIDTH);

  STlib_initNum(&w_ammo[3],
                ST_AMMO3X,
                ST_AMMO3Y,
                shortnum,
                &plyr->ammo[3],
                &st_statusbaron,
                ST_AMMO3WIDTH);

  // max ammo count (all four kinds)
  STlib_initNum(&w_maxammo[0],
                ST_MAXAMMO0X,
                ST_MAXAMMO0Y,
                shortnum,
                &plyr->maxammo[0],
                &st_statusbaron,
                ST_MAXAMMO0WIDTH);

  STlib_initNum(&w_maxammo[1],
                ST_MAXAMMO1X,
                ST_MAXAMMO1Y,
                shortnum,
                &plyr->maxammo[1],
                &st_statusbaron,
                ST_MAXAMMO1WIDTH);

  STlib_initNum(&w_maxammo[2],
                ST_MAXAMMO2X,
                ST_MAXAMMO2Y,
                shortnum,
                &plyr->maxammo[2],
                &st_statusbaron,
                ST_MAXAMMO2WIDTH);

  STlib_initNum(&w_maxammo[3],
                ST_MAXAMMO3X,
                ST_MAXAMMO3Y,
                shortnum,
                &plyr->maxammo[3],
                &st_statusbaron,
                ST_MAXAMMO3WIDTH);
}

static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initData();
  ST_createWidgets();
  st_stopped = false;
}

static void ST_Stop(void)
{
  if (st_stopped)
    return;
  V_SetPalette(0);
  st_stopped = true;
}

void ST_Init(void)
{
  veryfirsttime = 0;
  ST_loadGraphics(true);
}
