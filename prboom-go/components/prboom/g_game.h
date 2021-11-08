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
 * DESCRIPTION: Main game control interface.
 *-----------------------------------------------------------------------------*/

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"

//
// GAME
//

// killough 5/2/98: number of bytes reserved for saving options
#define GAME_OPTION_SIZE 64

boolean G_Responder(event_t *ev);
boolean G_CheckDemoStatus(void);
void G_DeathMatchSpawnPlayer(int playernum);
void G_InitNew(skill_t skill, int episode, int map);
void G_DeferedInitNew(skill_t skill, int episode, int map);
void G_DeferedPlayDemo(const char *demo); // CPhipps - const
void G_LoadGame(int slot, boolean is_command); // killough 5/15/98
void G_ForcedLoadGame(void);           // killough 5/15/98: forced loadgames
void G_DoLoadGame(void);
void G_SaveGame(int slot, char *description); // Called by M_Responder.
void G_BeginRecording(void);
// CPhipps - const on these string params
void G_RecordDemo(const char *name);          // Only called by startup code.
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_EndGame(void); /* cph - make m_menu.c call a G_* function for this */
void G_Ticker(void);
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_SaveGameName(char *, size_t, int, boolean); /* killough 3/22/98: sets savegame filename */
void G_SetFastParms(int);        // killough 4/10/98: sets -fast parameters
void G_DoNewGame(void);
void G_DoReborn(int playernum);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_ReadDemoTiccmd(ticcmd_t *cmd);
void G_WriteDemoTiccmd(ticcmd_t *cmd);
void G_DoWorldDone(void);
void G_Compatibility(void);
const byte *G_ReadOptions(const byte *demo_p);   /* killough 3/1/98 - cph: const byte* */
byte *G_WriteOptions(byte *demo_p);        // killough 3/1/98
void G_PlayerReborn(int player);
void G_RestartLevel(void); // CPhipps - menu involked level restart
void G_DoVictory(void);
void G_BuildTiccmd (ticcmd_t* cmd); // CPhipps - move decl to header
void G_ChangedPlayerColour(int pn, int cl); // CPhipps - On-the-fly player colour changing
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */

// killough 1/18/98: Doom-style printf;   killough 4/25/98: add gcc attributes
// CPhipps - renames to doom_printf to avoid name collision with glibc
void doom_printf(const char *, ...) __attribute__((format(printf,1,2)));

// killough 5/2/98: moved from m_misc.c:

extern int  key_right;
extern int  key_left;
extern int  key_up;
extern int  key_down;
extern int  key_menu_right;                                  // phares 3/7/98
extern int  key_menu_left;                                   //     |
extern int  key_menu_up;                                     //     V
extern int  key_menu_down;
extern int  key_menu_backspace;                              //     ^
extern int  key_menu_escape;                                 //     |
extern int  key_menu_enter;                                  // phares 3/7/98
extern int  key_strafeleft;
extern int  key_straferight;

extern int  key_fire;
extern int  key_use;
extern int  key_strafe;
extern int  key_speed;
extern int  key_escape;                                             // phares
extern int  key_savegame;                                           //    |
extern int  key_loadgame;                                           //    V
extern int  key_autorun;
extern int  key_reverse;
extern int  key_zoomin;
extern int  key_zoomout;
extern int  key_chat;
extern int  key_backspace;
extern int  key_enter;
extern int  key_help;
extern int  key_soundvolume;
extern int  key_hud;
extern int  key_quicksave;
extern int  key_endgame;
extern int  key_messages;
extern int  key_quickload;
extern int  key_quit;
extern int  key_gamma;
extern int  key_spy;
extern int  key_pause;
extern int  key_setup;
extern int  key_forward;
extern int  key_leftturn;
extern int  key_rightturn;
extern int  key_backward;
extern int  key_weapontoggle;
extern int  key_weapon1;
extern int  key_weapon2;
extern int  key_weapon3;
extern int  key_weapon4;
extern int  key_weapon5;
extern int  key_weapon6;
extern int  key_weapon7;
extern int  key_weapon8;
extern int  key_weapon9;
extern int  destination_keys[MAXPLAYERS];
extern int  key_map_right;
extern int  key_map_left;
extern int  key_map_up;
extern int  key_map_down;
extern int  key_map_zoomin;
extern int  key_map_zoomout;
extern int  key_map;
extern int  key_map_gobig;
extern int  key_map_follow;
extern int  key_map_mark;                                           //    ^
extern int  key_map_clear;                                          //    |
extern int  key_map_grid;                                           // phares
extern int  key_map_rotate; // cph - map rotation
extern int  key_map_overlay;// cph - map overlay
extern int  key_screenshot;    // killough 2/22/98 -- add key for screenshot
extern int  autorun;           // always running?                   // phares

extern int  defaultskill;      //jff 3/24/98 default skill
extern boolean haswolflevels;  //jff 4/18/98 wolf levels present

extern int  bodyquesize;       // killough 2/8/98: adustable corpse limit

// killough 5/2/98: moved from d_deh.c:
// Par times (new item with BOOM) - from g_game.c
extern int pars[4][10];  // hardcoded array size
extern int cpars[32];    // hardcoded array size
// CPhipps - Make savedesciption visible in wider scope
#define SAVEDESCLEN 32
extern char savedescription[SAVEDESCLEN];  // Description to save in savegame

/* cph - compatibility level strings */
extern const char * comp_lev_str[];

#endif
