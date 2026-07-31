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
/**********************************************************************
   module: RT_BATTL.H

   author: James R. Dose
   phone:  (214)-271-1365 Ext #221
   date:   September 8, 1994

   Public header for RT_BATTL.C

   (c) Copyright 1994 Apogee Software.  All Rights Reserved.
**********************************************************************/

#ifndef __RT_BATTLE_public
#define __RT_BATTLE_public


#define BIT_MASK( bit_number )   ( 1 << ( bit_number ) )
#define MINUTES_TO_GAMECOUNT( minutes ) \
   ( ( minutes ) * 60 * VBLCOUNTER )

//
// Return codes
//

typedef enum
   {
   battle_no_event,
   battle_end_game,
   battle_end_round,
   battle_out_of_time
   } battle_status;

//
// Types of battle events
//

typedef enum
   {
   battle_refresh,
	battle_player_killed,
	battle_player_tagged,
	battle_kill_with_missile,
   battle_kill_with_bullet,
   battle_kill_with_missile_in_air,
   battle_kill_with_bullet_in_air,
   battle_kill_by_crushing,
   battle_get_collector_item,
   battle_caught_eluder,
	battle_shot_deluder,
   battle_captured_triad
   } battle_event;

//
// Battle modes
//

enum
   {
   battle_StandAloneGame,
   battle_Normal,
   battle_ScoreMore,
   battle_Collector,
   battle_Scavenger,
   battle_Hunter,
   battle_Tag,
   battle_Eluder,
   battle_Deluder,
	battle_CaptureTheTriad,
   battle_NumBattleModes
   };

//
// Battle mode option : Gravity
//

//enum
//   {
//   bo_low_gravity,
//   bo_normal_gravity,
//   bo_high_gravity
//   };

//
// Battle mode option : Speed
//

enum
   {
   bo_normal_speed,
   bo_fast_speed
   };

//
// Battle mode option : Ammo
//

enum
   {
   bo_one_shot,
   bo_normal_shots,
   bo_infinite_shots
   };

//
// Battle mode option : Hit points
//

#define bo_character_hitpoints 0
#define bo_default_hitpoints   250

//
// Battle mode option : Light levels
//

enum
   {
   bo_light_dark,
   bo_light_normal,
   bo_light_bright,
   bo_light_fog,
   bo_light_periodic,
   bo_light_lightning
   };

//
// Battle mode option : Number of kills
//

enum
   {
   bo_kills_random   = -2,
   bo_kills_blind    = -1,
   bo_kills_infinite = 0,
   bo_kills_default  = 21
   };

//
// Battle mode option : Environment danger damage
//

enum
   {
   bo_danger_normal = -1,
   bo_danger_low    = 1,
   bo_danger_kill   = 30000
   };

//
// Battle mode option : Time limit
//
#define bo_time_infinite 0

//
// Battle mode configuration
//
typedef struct
	{
	unsigned Gravity;
	unsigned Speed;
	unsigned Ammo;
	unsigned HitPoints;
	unsigned SpawnDangers;
	unsigned SpawnHealth;
	unsigned SpawnWeapons;
   unsigned SpawnMines;
   unsigned RespawnItems;
   unsigned WeaponPersistence;
   unsigned RandomWeapons;
   unsigned FriendlyFire;
	unsigned LightLevel;
	int      Kills;
	int      DangerDamage;
	unsigned TimeLimit;
   unsigned RespawnTime;
   } battle_type;

#define bo_normal_respawn_time 30

extern boolean  BATTLEMODE;
extern short    WhoKilledWho[ MAXPLAYERS ][ MAXPLAYERS ];
extern short    BATTLE_Points[ MAXPLAYERS ];
extern short    BATTLE_PlayerOrder[ MAXPLAYERS ];
extern int      BATTLE_NumCollectorItems;
extern int      PointGoal;
extern int      DisplayPoints;
extern int      BATTLE_It;
extern boolean  BATTLE_ShowKillCount;
extern short    BATTLE_Team[ MAXPLAYERS ];
extern short    BATTLE_TeamLeader[ MAXPLAYERS ];
extern int      BATTLE_NumberOfTeams;
extern boolean  UpdateKills;

// Located in RT_MENU.C
extern battle_type    BATTLE_Options[ battle_NumBattleModes ];

void          BATTLE_Init( int battlemode, int numplayers );
void          BATTLE_GetSpecials( void );
void          BATTLE_SetOptions( battle_type *options );
void          BATTLE_GetOptions( battle_type *options );
battle_status BATTLE_CheckGameStatus( battle_event reason, int player );
void          BATTLE_SortPlayerRanks( void );
battle_status BATTLE_PlayerKilledPlayer( battle_event reason, int killer, int victim );
void          BATTLE_Shutdown( void );

#endif
