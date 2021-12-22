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
 *  Main loop menu stuff.
 *  Default Config File.
 *  PCX Screenshots.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "doomstat.h"
#include "m_argv.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_joy.h"
#include "lprintf.h"
#include "d_main.h"
#include "r_draw.h"
#include "r_demo.h"
#include "r_fps.h"

/* cph - disk icon not implemented */
static inline void I_BeginRead(void) {}
static inline void I_EndRead(void) {}

/*
 * M_WriteFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

boolean M_WriteFile(char const *name, void *source, int length)
{
  FILE *fp;

  errno = 0;

  if (!(fp = fopen(name, "wb")))       // Try opening file
    return 0;                          // Could not open file for writing

  I_BeginRead();                       // Disk icon on
  length = fwrite(source, length, 1, fp);   // Write data
  fclose(fp);
  I_EndRead();                         // Disk icon off

  if (!length)                         // Remove partially written file
    remove(name);

  return length;
}

/*
 * M_ReadFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

int M_ReadFile(char const *name, byte **buffer)
{
  FILE *fp;

  if ((fp = fopen(name, "rb")))
    {
      size_t length;

      I_BeginRead();
      fseek(fp, 0, SEEK_END);
      length = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      *buffer = Z_Malloc(length, PU_STATIC, 0);
      if (fread(*buffer, 1, length, fp) == length)
        {
          fclose(fp);
          I_EndRead();
          return length;
        }
      fclose(fp);
    }

  /* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1. */
  return -1;
}

//
// DEFAULTS
//

int precache = true; /* if true, load all graphics at start */
int mus_pause_opt; // 0 = kill music, 1 = pause, 2 = continue
int endoom_mode;

extern int usemouse;
extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

extern int viewwidth;
extern int viewheight;

extern int realtic_clock_rate;         // killough 4/13/98: adjustable timer
extern int tran_filter_pct;            // killough 2/21/98

extern int screenblocks;
extern int showMessages;

extern const char* chat_macros[];

extern int map_point_coordinates;

static default_t defaults[] =
{
  {"Misc settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"default_compatibility_level",{(int*)&default_compatibility_level},
   {-1},-1,MAX_COMPATIBILITY_LEVEL-1,
   def_int,ss_none}, // compatibility level" - CPhipps
  {"realtic_clock_rate",{&realtic_clock_rate},{100},0,UL,
   def_int,ss_none}, // percentage of normal speed (35 fps) realtic clock runs at
  {"max_player_corpse", {&bodyquesize}, {32},-1,UL,   // killough 2/8/98
   def_int,ss_none}, // number of dead bodies in view supported (-1 = no limit)
  {"flashing_hom",{&flashing_hom},{0},0,1,
   def_bool,ss_none}, // killough 10/98 - enable flashing HOM indicator
  {"demo_insurance",{&default_demo_insurance},{2},0,2,  // killough 3/31/98
   def_int,ss_none}, // 1=take special steps ensuring demo sync, 2=only during recordings
  {"endoom_mode", {&endoom_mode},{5},0,7, // CPhipps - endoom flags
   def_hex, ss_none}, // 0, +1 for colours, +2 for non-ascii chars, +4 for skip-last-line
  {"level_precache",{(int*)&precache},{1},0,1,
   def_bool,ss_none}, // precache level data?
  {"demo_smoothturns", {&demo_smoothturns},  {0},0,1,
   def_bool,ss_stat},
  {"demo_smoothturnsfactor", {&demo_smoothturnsfactor},  {6},1,SMOOTH_PLAYING_MAXFACTOR,
   def_int,ss_stat},

  {"Game settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"default_skill",{&defaultskill},{3},1,5, // jff 3/24/98 allow default skill setting
   def_int,ss_none}, // selects default skill 1=TYTD 2=NTR 3=HMP 4=UV 5=NM
  {"weapon_recoil",{&default_weapon_recoil},{0},0,1,
   def_bool,ss_weap, &weapon_recoil},
  /* killough 10/98 - toggle between SG/SSG and Fist/Chainsaw */
  {"doom_weapon_toggles",{&doom_weapon_toggles}, {1}, 0, 1,
   def_bool, ss_weap },
  {"player_bobbing",{&default_player_bobbing},{1},0,1,         // phares 2/25/98
   def_bool,ss_weap, &player_bobbing},
  {"monsters_remember",{&default_monsters_remember},{1},0,1,   // killough 3/1/98
   def_bool,ss_enem, &monsters_remember},
   /* MBF AI enhancement options */
  {"monster_infighting",{&default_monster_infighting}, {1}, 0, 1,
   def_bool, ss_enem, &monster_infighting},
  {"monster_backing",{&default_monster_backing}, {0}, 0, 1,
   def_bool, ss_enem, &monster_backing},
  {"monster_avoid_hazards",{&default_monster_avoid_hazards}, {1}, 0, 1,
   def_bool, ss_enem, &monster_avoid_hazards},
  {"monkeys",{&default_monkeys}, {0}, 0, 1,
   def_bool, ss_enem, &monkeys},
  {"monster_friction",{&default_monster_friction}, {1}, 0, 1,
   def_bool, ss_enem, &monster_friction},
  {"help_friends",{&default_help_friends}, {1}, 0, 1,
   def_bool, ss_enem, &help_friends},
  {"allow_pushers",{&default_allow_pushers},{1},0,1,
   def_bool,ss_weap, &allow_pushers},
  {"variable_friction",{&default_variable_friction},{1},0,1,
   def_bool,ss_weap, &variable_friction},
#ifdef DOGS
  {"player_helpers",{&default_dogs}, {0}, 0, 3,
   def_bool, ss_enem },
  {"friend_distance",{&default_distfriend}, {128}, 0, 999,
   def_int, ss_enem, &distfriend},
  {"dog_jumping",{&default_dog_jumping}, {1}, 0, 1,
   def_bool, ss_enem, &dog_jumping},
#endif
   /* End of MBF AI extras */

  {"sts_always_red",{&sts_always_red},{1},0,1, // no color changes on status bar
   def_bool,ss_stat},
  {"sts_pct_always_gray",{&sts_pct_always_gray},{0},0,1, // 2/23/98 chg default
   def_bool,ss_stat}, // makes percent signs on status bar always gray
  {"sts_traditional_keys",{&sts_traditional_keys},{0},0,1,  // killough 2/28/98
   def_bool,ss_stat}, // disables doubled card and skull key display on status bar
  {"show_messages",{&showMessages},{1},0,1,
   def_bool,ss_none}, // enables message display
  {"autorun",{&autorun},{0},0,1,  // killough 3/6/98: preserve autorun across games
   def_bool,ss_none},

  {"Compatibility settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"comp_zombie",{&default_comp[comp_zombie]},{0},0,1,def_bool,ss_comp,&comp[comp_zombie]},
  {"comp_infcheat",{&default_comp[comp_infcheat]},{0},0,1,def_bool,ss_comp,&comp[comp_infcheat]},
  {"comp_stairs",{&default_comp[comp_stairs]},{0},0,1,def_bool,ss_comp,&comp[comp_stairs]},
  {"comp_telefrag",{&default_comp[comp_telefrag]},{0},0,1,def_bool,ss_comp,&comp[comp_telefrag]},
  {"comp_dropoff",{&default_comp[comp_dropoff]},{0},0,1,def_bool,ss_comp,&comp[comp_dropoff]},
  {"comp_falloff",{&default_comp[comp_falloff]},{0},0,1,def_bool,ss_comp,&comp[comp_falloff]},
  {"comp_staylift",{&default_comp[comp_staylift]},{0},0,1,def_bool,ss_comp,&comp[comp_staylift]},
  {"comp_doorstuck",{&default_comp[comp_doorstuck]},{0},0,1,def_bool,ss_comp,&comp[comp_doorstuck]},
  {"comp_pursuit",{&default_comp[comp_pursuit]},{0},0,1,def_bool,ss_comp,&comp[comp_pursuit]},
  {"comp_vile",{&default_comp[comp_vile]},{0},0,1,def_bool,ss_comp,&comp[comp_vile]},
  {"comp_pain",{&default_comp[comp_pain]},{0},0,1,def_bool,ss_comp,&comp[comp_pain]},
  {"comp_skull",{&default_comp[comp_skull]},{0},0,1,def_bool,ss_comp,&comp[comp_skull]},
  {"comp_blazing",{&default_comp[comp_blazing]},{0},0,1,def_bool,ss_comp,&comp[comp_blazing]},
  {"comp_doorlight",{&default_comp[comp_doorlight]},{0},0,1,def_bool,ss_comp,&comp[comp_doorlight]},
  {"comp_god",{&default_comp[comp_god]},{0},0,1,def_bool,ss_comp,&comp[comp_god]},
  {"comp_skymap",{&default_comp[comp_skymap]},{0},0,1,def_bool,ss_comp,&comp[comp_skymap]},
  {"comp_floors",{&default_comp[comp_floors]},{0},0,1,def_bool,ss_comp,&comp[comp_floors]},
  {"comp_model",{&default_comp[comp_model]},{0},0,1,def_bool,ss_comp,&comp[comp_model]},
  {"comp_zerotags",{&default_comp[comp_zerotags]},{0},0,1,def_bool,ss_comp,&comp[comp_zerotags]},
  {"comp_moveblock",{&default_comp[comp_moveblock]},{0},0,1,def_bool,ss_comp,&comp[comp_moveblock]},
  {"comp_sound",{&default_comp[comp_sound]},{0},0,1,def_bool,ss_comp,&comp[comp_sound]},
  {"comp_666",{&default_comp[comp_666]},{0},0,1,def_bool,ss_comp,&comp[comp_666]},
  {"comp_soul",{&default_comp[comp_soul]},{0},0,1,def_bool,ss_comp,&comp[comp_soul]},
  {"comp_maskedanim",{&default_comp[comp_maskedanim]},{0},0,1,def_bool,ss_comp,&comp[comp_maskedanim]},

  {"Sound settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"sound_card",{&snd_card},{-1},-1,7,       // jff 1/18/98 allow Allegro drivers
   def_int,ss_none}, // select sounds driver (DOS), -1 is autodetect, 0 is none; in Linux, non-zero enables sound
  {"music_card",{&mus_card},{-1},-1,9,       //  to be set,  -1 = autodetect
   def_int,ss_none}, // select music driver (DOS), -1 is autodetect, 0 is none"; in Linux, non-zero enables music
  {"pitched_sounds",{&pitched_sounds},{0},0,1, // killough 2/21/98
   def_bool,ss_none}, // enables variable pitch in sound effects (from id's original code)
  {"samplerate",{&snd_samplerate},{22050},11025,48000, def_int,ss_none},
  {"sfx_volume",{&snd_SfxVolume},{8},0,15, def_int,ss_none},
  {"music_volume",{&snd_MusicVolume},{8},0,15, def_int,ss_none},
  {"mus_pause_opt",{&mus_pause_opt},{2},0,2, // CPhipps - music pausing
   def_int, ss_none}, // 0 = kill music when paused, 1 = pause music, 2 = let music continue
  {"snd_channels",{&snd_channels},{8},1,32,
   def_int,ss_none}, // number of audio events simultaneously // killough

  {"Video settings",{NULL},{0},UL,UL,def_none,ss_none},
#if 0
  {"videomode",{NULL, &default_videomode},{0,"8"},UL,UL,def_str,ss_none},
  /* 640x480 default resolution */
  {"screen_width",{&desired_screenwidth},{320}, 320, MAX_SCREENWIDTH,
   def_int,ss_none},
  {"screen_height",{&desired_screenheight},{240},200,MAX_SCREENHEIGHT,
   def_int,ss_none},
  {"use_fullscreen",{&use_fullscreen},{1},0,1, /* proff 21/05/2000 */
   def_bool,ss_none},
#ifndef DISABLE_DOUBLEBUFFER
  {"use_doublebuffer",{&use_doublebuffer},{1},0,1,             // proff 2001-7-4
   def_bool,ss_none}, // enable doublebuffer to avoid display tearing (fullscreen)
#endif
#endif
  {"translucency",{&default_translucency},{1},0,1,   // phares
   def_bool,ss_none}, // enables translucency
  {"tran_filter_pct",{&tran_filter_pct},{66},0,100,         // killough 2/21/98
   def_int,ss_none}, // set percentage of foreground/background translucency mix
  {"screenblocks",{&screenblocks},{10},3,11,  // killough 2/21/98: default to 10
   def_int,ss_none},
  {"usegamma",{&usegamma},{3},0,4, //jff 3/6/98 fix erroneous upper limit in range
   def_int,ss_none}, // gamma correction level // killough 1/18/98
  {"uncapped_framerate", {&movement_smooth},  {0},0,1,
   def_bool,ss_stat},
  {"filter_wall",{(int*)&drawvars.filterwall},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_floor",{(int*)&drawvars.filterfloor},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_sprite",{(int*)&drawvars.filtersprite},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_z",{(int*)&drawvars.filterz},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_LINEAR, def_int,ss_none},
  {"filter_patch",{(int*)&drawvars.filterpatch},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_threshold",{(int*)&drawvars.mag_threshold},{49152},
   0, UL, def_int,ss_none},
  {"sprite_edges",{(int*)&drawvars.sprite_edges},{RDRAW_MASKEDCOLUMNEDGE_SQUARE},
   RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int,ss_none},
  {"patch_edges",{(int*)&drawvars.patch_edges},{RDRAW_MASKEDCOLUMNEDGE_SQUARE},
   RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int,ss_none},

#if 0
  {"Mouse settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_mouse",{&usemouse},{1},0,1,
   def_bool,ss_none}, // enables use of mouse with DOOM
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_horiz",{&mouseSensitivity_horiz},{10},0,UL,
   def_int,ss_none}, /* adjust horizontal (x) mouse sensitivity killough/mead */
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_vert",{&mouseSensitivity_vert},{10},0,UL,
   def_int,ss_none}, /* adjust vertical (y) mouse sensitivity killough/mead */
  //jff 3/8/98 allow -1 in mouse bindings to disable mouse function
  {"mouseb_fire",{&mousebfire},{0},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for fire
  {"mouseb_strafe",{&mousebstrafe},{1},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for strafing
  {"mouseb_forward",{&mousebforward},{2},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for forward motion
  //jff 3/8/98 end of lower range change for -1 allowed in mouse binding
#endif

// For key bindings, the values stored in the key_* variables       // phares
// are the internal Doom Codes. The values stored in the default.cfg
// file are the keyboard codes.
// CPhipps - now they're the doom codes, so default.cfg can be portable

  {"Key bindings",{NULL},{0},UL,UL,def_none,ss_none},
  {"key_right",       {&key_right},          {KEYD_RIGHTARROW},
   0,MAX_KEY,def_key,ss_keys}, // key to turn right
  {"key_left",        {&key_left},           {KEYD_LEFTARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to turn left
  {"key_up",          {&key_up},             {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to move forward
  {"key_down",        {&key_down},           {KEYD_DOWNARROW},
   0,MAX_KEY,def_key,ss_keys}, // key to move backward
  {"key_menu_right",  {&key_menu_right},     {KEYD_RIGHTARROW},// phares 3/7/98
   0,MAX_KEY,def_key,ss_keys}, // key to move right in a menu  //     |
  {"key_menu_left",   {&key_menu_left},      {KEYD_LEFTARROW} ,//     V
   0,MAX_KEY,def_key,ss_keys}, // key to move left in a menu
  {"key_menu_up",     {&key_menu_up},        {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to move up in a menu
  {"key_menu_down",   {&key_menu_down},      {KEYD_DOWNARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to move down in a menu
  {"key_strafeleft",  {&key_strafeleft},     {','}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to strafe left
  {"key_straferight", {&key_straferight},    {'.'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to strafe right

  {"key_fire",        {&key_fire},           {KEYD_RCTRL}     ,
   0,MAX_KEY,def_key,ss_keys}, // duh
  {"key_use",         {&key_use},            {' '}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to open a door, use a switch
  {"key_strafe",      {&key_strafe},         {KEYD_RALT}      ,
   0,MAX_KEY,def_key,ss_keys}, // key to use with arrows to strafe
  {"key_speed",       {&key_speed},          {KEYD_RSHIFT}    ,
   0,MAX_KEY,def_key,ss_keys}, // key to run

  {"key_savegame",    {&key_savegame},       {KEYD_F2}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to save current game
  {"key_loadgame",    {&key_loadgame},       {KEYD_F3}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to restore from saved games
  {"key_soundvolume", {&key_soundvolume},    {KEYD_F4}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to bring up sound controls
  {"key_hud",         {&key_hud},            {KEYD_F5}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to adjust HUD
  {"key_quicksave",   {&key_quicksave},      {KEYD_F6}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to to quicksave
  {"key_endgame",     {&key_endgame},        {KEYD_F7}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to end the game
  {"key_messages",    {&key_messages},       {KEYD_F8}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle message enable
  {"key_quickload",   {&key_quickload},      {KEYD_F9}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to load from quicksave
  {"key_quit",        {&key_quit},           {KEYD_F10}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to quit game
  {"key_gamma",       {&key_gamma},          {KEYD_F11}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to adjust gamma correction
  {"key_spy",         {&key_spy},            {KEYD_F12}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to view from another coop player's view
  {"key_pause",       {&key_pause},          {KEYD_PAUSE}     ,
   0,MAX_KEY,def_key,ss_keys}, // key to pause the game
  {"key_autorun",     {&key_autorun},        {KEYD_CAPSLOCK}  ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle always run mode
  {"key_chat",        {&key_chat},           {'t'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to enter a chat message
  {"key_backspace",   {&key_backspace},      {KEYD_BACKSPACE} ,
   0,MAX_KEY,def_key,ss_keys}, // backspace key
  {"key_enter",       {&key_enter},          {KEYD_ENTER}     ,
   0,MAX_KEY,def_key,ss_keys}, // key to select from menu or see last message
  {"key_map",         {&key_map},            {KEYD_TAB}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle automap display
  {"key_map_right",   {&key_map_right},      {KEYD_RIGHTARROW},// phares 3/7/98
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap right   //     |
  {"key_map_left",    {&key_map_left},       {KEYD_LEFTARROW} ,//     V
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap left
  {"key_map_up",      {&key_map_up},         {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap up
  {"key_map_down",    {&key_map_down},       {KEYD_DOWNARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap down
  {"key_map_zoomin",  {&key_map_zoomin},      {'='}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to enlarge automap
  {"key_map_zoomout", {&key_map_zoomout},     {'-'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to reduce automap
  {"key_map_gobig",   {&key_map_gobig},       {'0'}           ,
   0,MAX_KEY,def_key,ss_keys},  // key to get max zoom for automap
  {"key_map_follow",  {&key_map_follow},      {'f'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle follow mode
  {"key_map_mark",    {&key_map_mark},        {'m'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to drop a marker on automap
  {"key_map_clear",   {&key_map_clear},       {'c'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to clear all markers on automap
  {"key_map_grid",    {&key_map_grid},        {'g'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle grid display over automap
  {"key_map_rotate",  {&key_map_rotate},      {'r'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle rotating the automap to match the player's orientation
  {"key_map_overlay", {&key_map_overlay},     {'o'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle overlaying the automap on the rendered display
  {"key_reverse",     {&key_reverse},         {'/'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to spin 180 instantly
  {"key_zoomin",      {&key_zoomin},          {'='}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to enlarge display
  {"key_zoomout",     {&key_zoomout},         {'-'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to reduce display
  {"key_chatplayer1", {&destination_keys[0]}, {'g'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 1
  // killough 11/98: fix 'i'/'b' reversal
  {"key_chatplayer2", {&destination_keys[1]}, {'i'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 2
  {"key_chatplayer3", {&destination_keys[2]}, {'b'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 3
  {"key_chatplayer4", {&destination_keys[3]}, {'r'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 4
  {"key_weapontoggle",{&key_weapontoggle},    {'0'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle between two most preferred weapons with ammo
  {"key_weapon1",     {&key_weapon1},         {'1'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 1 (fist/chainsaw)
  {"key_weapon2",     {&key_weapon2},         {'2'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 2 (pistol)
  {"key_weapon3",     {&key_weapon3},         {'3'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 3 (supershotgun/shotgun)
  {"key_weapon4",     {&key_weapon4},         {'4'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 4 (chaingun)
  {"key_weapon5",     {&key_weapon5},         {'5'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 5 (rocket launcher)
  {"key_weapon6",     {&key_weapon6},         {'6'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 6 (plasma rifle)
  {"key_weapon7",     {&key_weapon7},         {'7'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 7 (bfg9000)         //    ^
  {"key_weapon8",     {&key_weapon8},         {'8'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 8 (chainsaw)        //    |
  {"key_weapon9",     {&key_weapon9},         {'9'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 9 (supershotgun)    // phares

#if 0
  {"Joystick settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_joystick",{&usejoystick},{0},0,2,
   def_int,ss_none}, // number of joystick to use (0 for none)
  {"joy_left",{&joyleft},{0},  UL,UL,def_int,ss_none},
  {"joy_right",{&joyright},{0},UL,UL,def_int,ss_none},
  {"joy_up",  {&joyup},  {0},  UL,UL,def_int,ss_none},
  {"joy_down",{&joydown},{0},  UL,UL,def_int,ss_none},
  {"joyb_fire",{&joybfire},{0},0,UL,
   def_int,ss_keys}, // joystick button number to use for fire
  {"joyb_strafe",{&joybstrafe},{1},0,UL,
   def_int,ss_keys}, // joystick button number to use for strafing
  {"joyb_speed",{&joybspeed},{2},0,UL,
   def_int,ss_keys}, // joystick button number to use for running
  {"joyb_use",{&joybuse},{3},0,UL,
   def_int,ss_keys}, // joystick button number to use for use/open

  {"Chat macros",{NULL},{0},UL,UL,def_none,ss_none},
  {"chatmacro0", {0,&chat_macros[0]}, {0,HUSTR_CHATMACRO0},UL,UL,
   def_str,ss_chat}, // chat string associated with 0 key
  {"chatmacro1", {0,&chat_macros[1]}, {0,HUSTR_CHATMACRO1},UL,UL,
   def_str,ss_chat}, // chat string associated with 1 key
  {"chatmacro2", {0,&chat_macros[2]}, {0,HUSTR_CHATMACRO2},UL,UL,
   def_str,ss_chat}, // chat string associated with 2 key
  {"chatmacro3", {0,&chat_macros[3]}, {0,HUSTR_CHATMACRO3},UL,UL,
   def_str,ss_chat}, // chat string associated with 3 key
  {"chatmacro4", {0,&chat_macros[4]}, {0,HUSTR_CHATMACRO4},UL,UL,
   def_str,ss_chat}, // chat string associated with 4 key
  {"chatmacro5", {0,&chat_macros[5]}, {0,HUSTR_CHATMACRO5},UL,UL,
   def_str,ss_chat}, // chat string associated with 5 key
  {"chatmacro6", {0,&chat_macros[6]}, {0,HUSTR_CHATMACRO6},UL,UL,
   def_str,ss_chat}, // chat string associated with 6 key
  {"chatmacro7", {0,&chat_macros[7]}, {0,HUSTR_CHATMACRO7},UL,UL,
   def_str,ss_chat}, // chat string associated with 7 key
  {"chatmacro8", {0,&chat_macros[8]}, {0,HUSTR_CHATMACRO8},UL,UL,
   def_str,ss_chat}, // chat string associated with 8 key
  {"chatmacro9", {0,&chat_macros[9]}, {0,HUSTR_CHATMACRO9},UL,UL,
   def_str,ss_chat}, // chat string associated with 9 key
#endif

  {"Automap settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 1/7/98 defaults for automap colors
  //jff 4/3/98 remove -1 in lower range, 0 now disables new map features
  {"mapcolor_back", {&mapcolor_back}, {247},0,255,  // black //jff 4/6/98 new black
   def_colour,ss_auto}, // color used as background for automap
  {"mapcolor_grid", {&mapcolor_grid}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for automap grid lines
  {"mapcolor_wall", {&mapcolor_wall}, {23},0,255,   // red-brown
   def_colour,ss_auto}, // color used for one side walls on automap
  {"mapcolor_fchg", {&mapcolor_fchg}, {55},0,255,   // lt brown
   def_colour,ss_auto}, // color used for lines floor height changes across
  {"mapcolor_cchg", {&mapcolor_cchg}, {215},0,255,  // orange
   def_colour,ss_auto}, // color used for lines ceiling height changes across
  {"mapcolor_clsd", {&mapcolor_clsd}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for lines denoting closed doors, objects
  {"mapcolor_rkey", {&mapcolor_rkey}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for red key sprites
  {"mapcolor_bkey", {&mapcolor_bkey}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for blue key sprites
  {"mapcolor_ykey", {&mapcolor_ykey}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for yellow key sprites
  {"mapcolor_rdor", {&mapcolor_rdor}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for closed red doors
  {"mapcolor_bdor", {&mapcolor_bdor}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for closed blue doors
  {"mapcolor_ydor", {&mapcolor_ydor}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for closed yellow doors
  {"mapcolor_tele", {&mapcolor_tele}, {119},0,255,  // dk green
   def_colour,ss_auto}, // color used for teleporter lines
  {"mapcolor_secr", {&mapcolor_secr}, {252},0,255,  // purple
   def_colour,ss_auto}, // color used for lines around secret sectors
  {"mapcolor_exit", {&mapcolor_exit}, {0},0,255,    // none
   def_colour,ss_auto}, // color used for exit lines
  {"mapcolor_unsn", {&mapcolor_unsn}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for lines not seen without computer map
  {"mapcolor_flat", {&mapcolor_flat}, {88},0,255,   // lt gray
   def_colour,ss_auto}, // color used for lines with no height changes
  {"mapcolor_sprt", {&mapcolor_sprt}, {112},0,255,  // green
   def_colour,ss_auto}, // color used as things
  {"mapcolor_item", {&mapcolor_item}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for counted items
  {"mapcolor_hair", {&mapcolor_hair}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for dot crosshair denoting center of map
  {"mapcolor_sngl", {&mapcolor_sngl}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for the single player arrow
  {"mapcolor_me",   {&mapcolor_me}, {112},0,255, // green
   def_colour,ss_auto}, // your (player) colour
  {"mapcolor_enemy",   {&mapcolor_enemy}, {177},0,255,
   def_colour,ss_auto},
  {"mapcolor_frnd",   {&mapcolor_frnd}, {112},0,255,
   def_colour,ss_auto},
  //jff 3/9/98 add option to not show secrets til after found
  {"map_secret_after", {&map_secret_after}, {0},0,1, // show secret after gotten
   def_bool,ss_auto}, // prevents showing secret sectors till after entered
  {"map_point_coord", {&map_point_coordinates}, {0},0,1,
   def_bool,ss_auto},
  //jff 1/7/98 end additions for automap
  {"automapmode", {(int*)&automapmode}, {0}, 0, 31, // CPhipps - remember automap mode
   def_hex,ss_none}, // automap mode

  {"Heads-up display settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 2/16/98 defaults for color ranges in hud and status
  {"hudcolor_titl", {&hudcolor_titl}, {5},0,9,  // gold range
   def_int,ss_auto}, // color range used for automap level title
  {"hudcolor_xyco", {&hudcolor_xyco}, {3},0,9,  // green range
   def_int,ss_auto}, // color range used for automap coordinates
  {"hudcolor_mesg", {&hudcolor_mesg}, {6},0,9,  // red range
   def_int,ss_mess}, // color range used for messages during play
  {"hudcolor_chat", {&hudcolor_chat}, {5},0,9,  // gold range
   def_int,ss_mess}, // color range used for chat messages and entry
  {"hudcolor_list", {&hudcolor_list}, {5},0,9,  // gold range  //jff 2/26/98
   def_int,ss_mess}, // color range used for message review
  {"hud_msg_lines", {&hud_msg_lines}, {1},1,16,  // 1 line scrolling window
   def_int,ss_mess}, // number of messages in review display (1=disable)
  {"hud_list_bgon", {&hud_list_bgon}, {0},0,1,  // solid window bg ena //jff 2/26/98
   def_bool,ss_mess}, // enables background window behind message review
  {"hud_distributed",{&hud_distributed},{0},0,1, // hud broken up into 3 displays //jff 3/4/98
   def_bool,ss_none}, // splits HUD into three 2 line displays

  {"health_red",    {&health_red}   , {25},0,200, // below is red
   def_int,ss_stat}, // amount of health for red to yellow transition
  {"health_yellow", {&health_yellow}, {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of health for yellow to green transition
  {"health_green",  {&health_green} , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of health for green to blue transition
  {"armor_red",     {&armor_red}    , {25},0,200, // below is red
   def_int,ss_stat}, // amount of armor for red to yellow transition
  {"armor_yellow",  {&armor_yellow} , {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of armor for yellow to green transition
  {"armor_green",   {&armor_green}  , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of armor for green to blue transition
  {"ammo_red",      {&ammo_red}     , {25},0,100, // below 25% is red
   def_int,ss_stat}, // percent of ammo for red to yellow transition
  {"ammo_yellow",   {&ammo_yellow}  , {50},0,100, // below 50% is yellow, above green
   def_int,ss_stat}, // percent of ammo for yellow to green transition

  //jff 2/16/98 HUD and status feature controls
  {"hud_active",    {&hud_active}, {2},0,2, // 0=off, 1=small, 2=full
   def_int,ss_none}, // 0 for HUD off, 1 for HUD small, 2 for full HUD
  //jff 2/23/98
  {"hud_displayed", {&hud_displayed},  {0},0,1, // whether hud is displayed
   def_bool,ss_none}, // enables display of HUD
  {"hud_nosecrets", {&hud_nosecrets},  {0},0,1, // no secrets/items/kills HUD line
   def_bool,ss_stat}, // disables display of kills/items/secrets on HUD

  {"Weapon preferences",{NULL},{0},UL,UL,def_none,ss_none},
  // killough 2/8/98: weapon preferences set by user:
  {"weapon_choice_1", {&weapon_preferences[0][0]}, {6}, 0,9,
   def_int,ss_weap}, // first choice for weapon (best)
  {"weapon_choice_2", {&weapon_preferences[0][1]}, {9}, 0,9,
   def_int,ss_weap}, // second choice for weapon
  {"weapon_choice_3", {&weapon_preferences[0][2]}, {4}, 0,9,
   def_int,ss_weap}, // third choice for weapon
  {"weapon_choice_4", {&weapon_preferences[0][3]}, {3}, 0,9,
   def_int,ss_weap}, // fourth choice for weapon
  {"weapon_choice_5", {&weapon_preferences[0][4]}, {2}, 0,9,
   def_int,ss_weap}, // fifth choice for weapon
  {"weapon_choice_6", {&weapon_preferences[0][5]}, {8}, 0,9,
   def_int,ss_weap}, // sixth choice for weapon
  {"weapon_choice_7", {&weapon_preferences[0][6]}, {5}, 0,9,
   def_int,ss_weap}, // seventh choice for weapon
  {"weapon_choice_8", {&weapon_preferences[0][7]}, {7}, 0,9,
   def_int,ss_weap}, // eighth choice for weapon
  {"weapon_choice_9", {&weapon_preferences[0][8]}, {1}, 0,9,
   def_int,ss_weap}, // ninth choice for weapon (worst)

};

static const int numdefaults = sizeof(defaults)/sizeof(defaults[0]);
static const char* configfile;

//
// M_SaveDefaults
//

void M_SaveDefaults(void)
{
  FILE *f = fopen(configfile, "w");
  if (!f)
    return; // can't write the file, but don't complain

  fprintf(f, "# Doom config file\n");
  fprintf(f, "# Format:\n");
  fprintf(f, "# variable   value\n");

  for (int i = 0; i < numdefaults; i++)
  {
    if (defaults[i].type == def_none)
    {
      fprintf(f, "\n# %s\n", defaults[i].name);
    }
    else if (!IS_STRING(defaults[i]))
    {
      if (defaults[i].type == def_hex)
        fprintf(f, "%-25s 0x%x\n", defaults[i].name, *(defaults[i].location.pi));
      else
        fprintf(f, "%-25s %5i\n", defaults[i].name, *(defaults[i].location.pi));
    }
    else
    {
      fprintf(f, "%-25s \"%s\"\n", defaults[i].name, *(defaults[i].location.ppsz));
    }
  }
  fclose (f);
}

/*
 * M_LookupDefault
 *
 * cph - mimic MBF function for now. Yes it's crap.
 */

struct default_s *M_LookupDefault(const char *name)
{
  for (int i = 0 ; i < numdefaults ; i++)
    if ((defaults[i].type != def_none) && !strcmp(name, defaults[i].name))
      return (default_t*)&defaults[i];
  I_Error("M_LookupDefault: %s not found",name);
  return NULL;
}

//
// M_LoadDefaults
//

void M_LoadDefaults(void)
{
  // check for a custom default file
  int i = M_CheckParm ("-config");
  if (i && i < myargc-1)
    configfile = myargv[i+1];
  else {
    const char *exedir = I_DoomExeDir();
    configfile = strcat(strcpy(malloc(strlen(exedir) + 32), exedir), "/prboom.cfg");
  }

  // set everything to base values
  for (int i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].type == def_str && defaults[i].location.ppsz)
      *defaults[i].location.ppsz = strdup(defaults[i].defaultvalue.psz);
    if (defaults[i].type != def_str && defaults[i].location.pi)
      *defaults[i].location.pi = defaults[i].defaultvalue.i;
  }

  // read the file in, overriding any set defaults
  FILE* f = fopen(configfile, "r");
  if (!f)
  {
    lprintf(LO_WARN, " failed to read config from: %s\n", configfile);
    return;
  }

  lprintf(LO_CONFIRM, " reading config from: %s\n", configfile);
  while (!feof(f))
  {
    char def[80], strparm[100];
    int parm = 0;

    if (fscanf (f, "%79s %[^\n]\n", def, strparm) != 2)
      continue;

    if (!isalnum(def[0]))
      continue;

    bool isstring = (strparm[0] == '"');
    if (isstring) {
      size_t len = strlen(strparm);
      memmove(strparm, strparm + 1, len - 2);
      strparm[len-2] = 0;
    } else if ((strparm[0] == '0') && (strparm[1] == 'x')) {
      sscanf(strparm+2, "%x", &parm);
    } else {
      sscanf(strparm, "%i", &parm);
    }

    for (i = 0 ; i < numdefaults ; i++)
      if ((defaults[i].type != def_none) && !strcmp(def, defaults[i].name))
      {
        if (isstring != IS_STRING(defaults[i])) {
          lprintf(LO_WARN, "M_LoadDefaults: Type mismatch reading %s\n", defaults[i].name);
          continue;
        }
        if (!isstring)
        {
        //jff 3/4/98 range check numeric parameters
        if ((defaults[i].minvalue==UL || defaults[i].minvalue<=parm) &&
            (defaults[i].maxvalue==UL || defaults[i].maxvalue>=parm))
          *(defaults[i].location.pi) = parm;
        }
        else
        {
        free((char*)*(defaults[i].location.ppsz));  /* phares 4/13/98 */
        *(defaults[i].location.ppsz) = strdup(strparm);
        }
        break;
      }
  }
  fclose(f);
}
