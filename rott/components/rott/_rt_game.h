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
#ifndef _rt_game_private
#define _rt_game_private

//******************************************************************************
//
// Private header for RT_GAME.C
//
//******************************************************************************


//******************************************************************************
//
// TYPEDEFS
//
//******************************************************************************

typedef struct {
   char str[10];
   int length;
} STR;

//******************************************************************************
//
// DEFINES
//
//******************************************************************************

#define MENUSHADELEVEL  105

#define KILLS_X      0
#define KILLS_Y      176
#define KILLS_WIDTH  32
#define KILLS_HEIGHT 24
#define KILLS_OFFSET 14
#define KILLS_NAME_Y ( KILLS_Y + 16 )
#define MAXKILLBOXES 10
#define PLAYERS_Y      ( 107 + ( gamestate.teamplay ? 0 : 4 ) )
#define PLAYERS_NAME_Y ( PLAYERS_Y + 16 )
#define PLAYERS_TEAM_Y ( PLAYERS_Y + 24 )

#define LEADER_X      0
#define LEADER_Y      0
#define LEADER_NUM_X  61
#define LEADER_NUM_Y  ( LEADER_Y )
#define LEADER_NAME_X ( LEADER_X + 3 )
#define LEADER_NAME_Y ( LEADER_Y + 2 )
#define LEADER_WIDTH  88
#define LEADER_HEIGHT 16
/* bna++
#define HEALTH_X  20
#define HEALTH_Y  185

#define AMMO_X    300
#define AMMO_Y    184
*/
//--------------------
#define HEALTH_X  20*2
#define HEALTH_Y  (185*2)+16

#define AMMO_X    300*2
#define AMMO_Y    (184*2)+16
//--------------------

#define SCORE_X   4
#define SCORE_Y   0

/*
#define KEY1_X    152
#define KEY2_X    160
#define KEY3_X    168
#define KEY4_X    176
#define KEY_Y     0

#define POWER_X   184
#define POWER_Y   0

#define ARMOR_X   200
#define ARMOR_Y   0

#define MEN_X     216
#define MEN_Y     0
*/
//--------------------
#define KEY1_X    152
#define KEY2_X    160
#define KEY3_X    168
#define KEY4_X    176
#define KEY_Y     0

#define POWER_X   (184)
#define POWER_Y   0

#define ARMOR_X   200
#define ARMOR_Y   0

#define MEN_X     216
#define MEN_Y     0
//--------------------
#define HOUR_X    7
#define MIN_X     26
#define SEC_X     45
#define TIME_Y    0

#define GAMETIME_X 88
#define GAMETIME_Y 0

#define TALLYTIME_X 130
#define TALLYTIME_Y 8

#define LIVES_X   288
#define LIVES_Y   0

#define TRIAD_X   308
#define TRIAD_Y   6

#define POWERUP1X 184
#define POWERUP2X 200
#define POWERUPY  0

#define EXTRAPOINTS        50000
#define ADRENALINEBONUS    5

#define STR_SAVECHT1 "Your Save Game file is,"
#define STR_SAVECHT2 "shall we say, \"corrupted\"."
#define STR_SAVECHT3 "But I'll let you go on and"
#define STR_SAVECHT4 "play anyway...."

#define MAXSAVEDGAMESIZE 120000

#if (SHAREWARE == 0)
#define WEAPON_IS_MAGICAL(x) (((x) == wp_dog) || ((x) == wp_godhand))
#else
#define WEAPON_IS_MAGICAL(x) ((x) == wp_godhand)
#endif

//******************************************************************************
//
// PROTOTYPES
//
//******************************************************************************

void DrawMPPic (int xpos, int ypos, int width, int height, int heightmod, byte *src, boolean bufferofsonly);
void DrawHighScores (void);
void GM_MemToScreen (byte *source, int width, int height, int x, int y);

#endif
