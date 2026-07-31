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
   module: RT_BATTL.C

   author: James R. Dose
   phone:  (214)-271-1365 Ext #221
   date:   September 8, 1994

   Battle mode support routines for Rise of the Triad.

   (c) Copyright 1994 Apogee Software.  All Rights Reserved.
**********************************************************************/

#include "rt_def.h"
#include <string.h>
#include "rottnet.h"
#include "isr.h"
#include "rt_battl.h"
#include "rt_actor.h"
#include "rt_rand.h"
#include "rt_playr.h"
#include "rt_game.h"
#include "rt_sound.h"
#include "rt_com.h"
#include "rt_msg.h"
#include "rt_view.h"

#include "rt_util.h"
#include "rt_main.h"
//MED
#include "memcheck.h"

#define INFINITE -1

static battle_status BATTLE_StartRound( void );

static int Timer;
static int TimeLimit;
static int NumberOfPlayers;
static int BattleRound;
static int BattleMode;

static boolean RoundOver;
static boolean KillsEndGame;
static boolean KeepTrackOfKills;
boolean UpdateKills;

static boolean SwapFlag;

static battle_type BattleOptions;

specials BattleSpecialsTimes =
      {
      60, // god
      60, // dog
      20, // shrooms
      20, // elasto
      60, // asbestos vest
      60, // bullet proof vest
      GASTICS / VBLCOUNTER, // gas mask
      60, // mercury mode

      300, // god respawn
      60, // dog respawn
      60, // shrooms respawn
      60, // elasto respawn
      60, // asbestos vest respawn
      60, // bullet proof vest respawn
      60, // gas mask respawn
      60  // mercury mode respawn
      };

short WhoKilledWho[ MAXPLAYERS ][ MAXPLAYERS ];
short BATTLE_Points[ MAXPLAYERS ];
short BATTLE_PlayerOrder[ MAXPLAYERS ];
short BATTLE_Team[ MAXPLAYERS ];
short BATTLE_TeamLeader[ MAXPLAYERS ];
int   BATTLE_NumberOfTeams;
int   BATTLE_NumCollectorItems;
int   PointGoal;
int   DisplayPoints;
int   BATTLE_It;

boolean BATTLE_ShowKillCount;

boolean BATTLEMODE = false;

/*---------------------------------------------------------------------
	Function Prototypes:
---------------------------------------------------------------------*/

static int BATTLE_CheckKills
	(
	battle_event reason,
	int player
	);

/*---------------------------------------------------------------------
	Function: BATTLE_Init

	Initializes the battle information.
---------------------------------------------------------------------*/

void BATTLE_Init
	(
	int battlemode,
	int numplayers
	)

	{
	int index;
   int index2;
   int team;
   int TeamNumber[ MAXPLAYERCOLORS ];

   #if (BATTLECHECK == 1)
      if ( ( gamestate.teamplay ) && ( BattleMode == battle_Tag ) )
         {
         Error( "BATTLE_Init : Cannot play Tag in team mode.\n" );
         }

      if ( ( gamestate.teamplay ) && ( BattleMode == battle_CaptureTheTriad ) )
         {
         Error( "BATTLE_Init : Can only play Capture the Triad in team mode.\n" );
         }
   #endif

	Timer   = 0;
	RoundOver = false;

	BattleRound = -1;
   BATTLE_It = 0;

	BattleMode = battlemode;

	BATTLEMODE = false;

   UpdateKills = true;

	gamestate.BattleOptions.Gravity       = NORMAL_GRAVITY;
	gamestate.BattleOptions.Speed         = bo_normal_speed;
	gamestate.BattleOptions.Ammo          = bo_normal_shots;
	gamestate.BattleOptions.HitPoints     = bo_character_hitpoints;
	gamestate.BattleOptions.LightLevel    = bo_light_normal;
   gamestate.BattleOptions.Kills         = bo_kills_default;
	gamestate.BattleOptions.DangerDamage  = bo_danger_normal;
	gamestate.BattleOptions.TimeLimit     = bo_time_infinite;
   gamestate.BattleOptions.RespawnTime   = bo_normal_respawn_time;
   gamestate.BattleOptions.RandomWeapons = false;
   gamestate.BattleOptions.FriendlyFire  = true;
   gamestate.BattleOptions.WeaponPersistence = false;
	gamestate.BattleOptions.SpawnMines    = false;

	if ( BattleMode != battle_StandAloneGame )
		{
		BATTLEMODE = true;

      if ( gamestate.Product == ROTT_SHAREWARE )
         {
         switch( battlemode )
            {
            case battle_Normal :
            case battle_Collector :
            case battle_Hunter :
               break;

            default :
               Error( "Shareware version can only play Normal, Collector, "
                  "or Hunter in Comm-bat game." );
            }
         }

		gamestate.BattleOptions.Gravity       = BattleOptions.Gravity;
		gamestate.BattleOptions.Speed         = BattleOptions.Speed;
		gamestate.BattleOptions.Ammo          = BattleOptions.Ammo;
		gamestate.BattleOptions.HitPoints     = BattleOptions.HitPoints;
		gamestate.BattleOptions.LightLevel    = BattleOptions.LightLevel;
		gamestate.BattleOptions.Kills         = BattleOptions.Kills;
		gamestate.BattleOptions.DangerDamage  = BattleOptions.DangerDamage;
		gamestate.BattleOptions.TimeLimit     = BattleOptions.TimeLimit;
      gamestate.BattleOptions.RespawnTime   = BattleOptions.RespawnTime;
      gamestate.BattleOptions.RandomWeapons = BattleOptions.RandomWeapons;
      gamestate.BattleOptions.FriendlyFire  = BattleOptions.FriendlyFire;
      gamestate.BattleOptions.SpawnMines    = BattleOptions.SpawnMines;
      gamestate.BattleOptions.WeaponPersistence = BattleOptions.WeaponPersistence;
      }

	gamestate.ShowScores                 = true;
	gamestate.BattleOptions.SpawnHealth  = true;
	gamestate.BattleOptions.SpawnWeapons = true;
   gamestate.BattleOptions.SpawnDangers = true;
	gamestate.SpawnCollectItems          = false;
	gamestate.SpawnEluder                = false;
	gamestate.SpawnDeluder               = false;
	gamestate.BattleOptions.RespawnItems = false;

	NumberOfPlayers = numplayers;


   BATTLE_NumberOfTeams = numplayers;
	for( index = 0; index < MAXPLAYERS; index++ )
		{
      BATTLE_PlayerOrder[ index ] = index;
		BATTLE_Points[ index ] = 0;
      for( index2 = 0; index2 < MAXPLAYERS; index2++ )
         {
         WhoKilledWho[ index ][ index2 ] = 0;
         }

      BATTLE_Team[ index ] = index;
      BATTLE_TeamLeader[ index ] = index;
      }


   if ( gamestate.teamplay )
      {
      for( index = 0; index < MAXPLAYERCOLORS; index++ )
         {
         TeamNumber[ index ] = -1;
         }

      BATTLE_NumberOfTeams = 0;

      for( index = 0; index < numplayers; index++ )
         {
         team = PLAYERSTATE[ index ].uniformcolor;
         if ( TeamNumber[ team ] == -1 )
            {
            TeamNumber[ team ] = BATTLE_NumberOfTeams;
            BATTLE_TeamLeader[ BATTLE_NumberOfTeams ] = index;
            BATTLE_NumberOfTeams++;
            }
         BATTLE_Team[ index ] = TeamNumber[ team ];
         }
      }

	PointGoal = gamestate.BattleOptions.Kills;
	if ( ( gamestate.BattleOptions.Kills == bo_kills_random ) ||
		( gamestate.BattleOptions.Kills == bo_kills_blind ) )
		{
		// Possibility of playing from 5 to 50 kills
		PointGoal = ( GameRandomNumber( "BATTLE_Init", 0 ) % 46 ) + 5;
		}

   DisplayPoints = PointGoal;

	for( index = 0; index < MAXPLAYERS; index++ )
		{
		gamestate.PlayerHasGun[ index ] = true;
		}

	KillsEndGame = true;
	KeepTrackOfKills = true;

	switch( BattleMode )
		{
		case battle_StandAloneGame :
			KillsEndGame      = false;
			KeepTrackOfKills  = false;
			break;

		case battle_Normal :
			break;

		case battle_ScoreMore :
			break;

		case battle_Collector :
			for( index = 0; index < MAXPLAYERS; index++ )
				{
				gamestate.PlayerHasGun[ index ] = false;
				}
			KillsEndGame     = false;
			KeepTrackOfKills = false;
			gamestate.BattleOptions.SpawnHealth  = false;
			gamestate.BattleOptions.SpawnWeapons = false;
			gamestate.SpawnCollectItems          = true;
			break;

		case battle_Scavenger :
			KillsEndGame     = false;
			KeepTrackOfKills = false;
			gamestate.BattleOptions.SpawnWeapons = true;
			gamestate.BattleOptions.SpawnHealth  = true;
			gamestate.SpawnCollectItems          = true;
			break;

		case battle_Hunter :
         PointGoal *= BATTLE_NumberOfTeams;
			KillsEndGame      = false;
			KeepTrackOfKills  = true;
         BATTLE_It = 0;
         for( index = 0; index < NumberOfPlayers; index++ )
            {
            if ( BATTLE_Team[ index ] == 0 )
               {
               gamestate.PlayerHasGun[ index ] = false;
               }
            }
			break;

		case battle_Tag :
			for( index = 0; index < MAXPLAYERS; index++ )
				{
				gamestate.PlayerHasGun[ index ] = false;
				}

			gamestate.BattleOptions.SpawnHealth  = false;
			gamestate.BattleOptions.SpawnWeapons = false;
			gamestate.BattleOptions.SpawnDangers = true;
			KeepTrackOfKills = true;
			KillsEndGame     = true;
			break;

		case battle_Eluder :
			KeepTrackOfKills   = false;
			KillsEndGame       = false;

			for( index = 0; index < MAXPLAYERS; index++ )
				{
				gamestate.PlayerHasGun[ index ] = false;
				}

			gamestate.BattleOptions.SpawnWeapons = false;
			gamestate.SpawnEluder                = true;
			break;

		case battle_Deluder :
			KeepTrackOfKills    = false;
			KillsEndGame        = false;
			gamestate.SpawnDeluder = true;
			break;

		case battle_CaptureTheTriad :
         KillsEndGame     = false;
         KeepTrackOfKills = false;
			break;
		}

	if ( BattleMode != battle_StandAloneGame )
		{
		if ( BattleOptions.RespawnItems )
			{
			gamestate.BattleOptions.RespawnItems = true;
			}

		if ( !BattleOptions.SpawnDangers )
			{
			gamestate.BattleOptions.SpawnDangers = false;
			}

		if ( !BattleOptions.SpawnHealth )
			{
			gamestate.BattleOptions.SpawnHealth = false;
			}

		if ( !BattleOptions.SpawnWeapons )
			{
			gamestate.BattleOptions.SpawnWeapons = false;
			}

		if ( gamestate.BattleOptions.Kills == bo_kills_blind )
			{
			gamestate.ShowScores = false;
			}

      GRAVITY = gamestate.BattleOptions.Gravity;

      if ( gamestate.BattleOptions.Kills == bo_kills_infinite )
         {
         KillsEndGame = false;
         }
		}

	BATTLE_StartRound();

   #if (BATTLEINFO == 1)
      SoftError( "GRAVITY      = %d\n", GRAVITY );
      SoftError( "BO_Gravity   = %d\n", BattleOptions.Gravity );
      SoftError( "BO_Speed     = %d\n", BattleOptions.Speed );
      SoftError( "BO_Ammo      = %d\n", BattleOptions.Ammo );
      SoftError( "BO_HitPoints = %d\n", BattleOptions.HitPoints );
      SoftError( "BO_Dangers   = %d\n", BattleOptions.SpawnDangers );
      SoftError( "BO_Health    = %d\n", BattleOptions.SpawnHealth );
      SoftError( "BO_Weapons   = %d\n", BattleOptions.SpawnWeapons );
      SoftError( "BO_Respawn   = %d\n", BattleOptions.RespawnItems );
      SoftError( "BO_Light     = %d\n", BattleOptions.LightLevel );
      SoftError( "BO_Kills     = %d\n", BattleOptions.Kills );
      SoftError( "BO_DangerDam = %d\n", BattleOptions.DangerDamage );
      SoftError( "BO_TimeLimit = %d\n", BattleOptions.TimeLimit );
   #endif
	}


/*---------------------------------------------------------------------
   Function: BATTLE_GetSpecials

   Set the battle special times.
---------------------------------------------------------------------*/

void BATTLE_GetSpecials
	(
   void
	)

   {
   int *src;
   int *dest;

   src  = ( int * )&BattleSpecialsTimes;
   dest = ( int * )&gamestate.SpecialsTimes;

   while( src < ( int * )( &BattleSpecialsTimes + 1 ) )
      {
      *dest = *src * VBLCOUNTER;
      dest++;
      src++;
      }
   }


/*---------------------------------------------------------------------
	Function: BATTLE_SetOptions

	Set the battle options.
---------------------------------------------------------------------*/

void BATTLE_SetOptions
	(
	battle_type *options
	)

	{
	memcpy( &BattleOptions, options, sizeof( battle_type ) );
	}


/*---------------------------------------------------------------------
   Function: BATTLE_GetOptions

   Returns the battle options.
---------------------------------------------------------------------*/

void BATTLE_GetOptions
   (
   battle_type *options
   )

   {
	memcpy( options, &BattleOptions, sizeof( battle_type ) );
   }


/*---------------------------------------------------------------------
   Function: BATTLE_Shutdown

   Shutsdown the battle information.
---------------------------------------------------------------------*/

void BATTLE_Shutdown
   (
   void
   )

   {
   int index;
   int index2;

   Timer             = 0;
   RoundOver         = false;
   BattleRound       = 0;
   BattleMode        = battle_StandAloneGame;
   BATTLEMODE        = false;
   NumberOfPlayers   = 1;
   BATTLE_NumberOfTeams = 1;
   PointGoal         = 0;
   KillsEndGame      = false;
   KeepTrackOfKills  = false;

   for( index = 0; index < MAXPLAYERS; index++ )
      {
      BATTLE_Points[ index ] = 0;
      for( index2 = 0; index2 < MAXPLAYERS; index2++ )
         {
         WhoKilledWho[ index ][ index2 ] = 0;
         }
		gamestate.PlayerHasGun[ index ] = true;
		}
	gamestate.BattleOptions.SpawnHealth  = true;
	gamestate.BattleOptions.SpawnWeapons = true;
	gamestate.BattleOptions.SpawnDangers = true;
   gamestate.BattleOptions.RandomWeapons = false;
   gamestate.BattleOptions.FriendlyFire  = true;
   gamestate.BattleOptions.WeaponPersistence = false;
	gamestate.BattleOptions.SpawnMines    = false;

	gamestate.ShowScores        = true;
	gamestate.SpawnCollectItems = false;
	gamestate.SpawnEluder       = false;
	gamestate.SpawnDeluder      = false;
	}



/*---------------------------------------------------------------------
	Function: BATTLE_StartRound

	Begins a round of battle.
---------------------------------------------------------------------*/

static battle_status BATTLE_StartRound
	(
	void
	)

	{
	int index;

	Timer     = 0;
	TimeLimit = INFINITE;
	RoundOver = false;

	if ( !BATTLEMODE )
		{
		return( battle_no_event );
		}

	BattleRound++;

	if ( gamestate.BattleOptions.TimeLimit == bo_time_infinite )
      {
		if ( BattleMode == battle_Hunter )
			{
	      TimeLimit = MINUTES_TO_GAMECOUNT( 99 );
         }
      else
         {
         TimeLimit = INFINITE;
         }
      }
   else
      {
      TimeLimit = MINUTES_TO_GAMECOUNT( gamestate.BattleOptions.TimeLimit );
      }

	if ( BattleMode == battle_Hunter )
		{
		for( index = 0; index < MAXPLAYERS; index++ )
			{
			gamestate.PlayerHasGun[ index ] = true;
			}

		if ( ( gamestate.BattleOptions.Kills != bo_kills_infinite ) &&
         ( BattleRound >= PointGoal ) )
			{
			return( battle_end_game );
			}

      BATTLE_It = BattleRound % BATTLE_NumberOfTeams;
      for( index = 0; index < NumberOfPlayers; index++ )
         {
         if ( BATTLE_Team[ index ] == BATTLE_It )
            {
            gamestate.PlayerHasGun[ index ] = false;
            }
         }
		}

	return( battle_no_event );
	}

/*---------------------------------------------------------------------
	Function: BATTLE_CheckGameStatus

	Checks if certain battle mode conditions have been met and
	determines the appropriate response.
---------------------------------------------------------------------*/

battle_status BATTLE_CheckGameStatus
   (
   battle_event reason,
   int player
   )

   {
   battle_status status;
   int team;

   if ( ( player < 0 ) || ( player >= MAXPLAYERS ) )
      {
      #if (BATTLECHECK == 1)
         Error( "BATTLE_CheckGameStatus - reason %d : Player out of range!\n",
            reason );
      #else
         return( battle_no_event );
      #endif
      }

   if ( !BATTLEMODE )
      {
      return( battle_no_event );
      }

   team = BATTLE_Team[ player ];

   status = battle_no_event;

   switch( reason )
      {
      case battle_refresh :
         Timer++;
         if ( ( TimeLimit != INFINITE ) &&
            ( Timer > TimeLimit ) )
            {
            RoundOver = true;

            if ( BattleMode == battle_Hunter )
               {
               status = BATTLE_StartRound();
               if ( status == battle_no_event )
                  {
                  status = battle_end_round;
                  }
               }
            else
               {
               status = battle_out_of_time;
               }

            UpdateKills = true;
            }

         if ( UpdateKills )
            {
            BATTLE_SortPlayerRanks();
            if ( gamestate.ShowScores )
               {
               DrawKills (false);
               }
            UpdateKills = false;
            }

         if ( RoundOver )
            {
            return( battle_end_game );
            }
         break;

      case battle_player_killed :
         #if (BATTLEINFO == 1)
            SoftError( "BATTLE_CheckGameStatus: Player %d Died", player );
            SoftError( "---ticks = %d\n", Timer );
         #endif

         switch( BattleMode )
            {
            case battle_Normal :
            case battle_ScoreMore :
            case battle_Hunter :
               if ( BattleOptions.FriendlyFire )
                  {
                  BATTLE_Points[ team ]--;
                  UpdateKills = true;
                  }
               break;

            case battle_Tag :
               // Same as being tagged
               if ( BattleOptions.FriendlyFire )
                  {
                  BATTLE_Points[ team ]++;
                  UpdateKills = true;
                  }
               break;
            }

         WhoKilledWho[ player ][ player ]++;
         break;

      case battle_get_collector_item :

         if ( ( BattleMode != battle_Collector ) &&
            ( BattleMode != battle_Scavenger ) )
            {
            #if (BATTLECHECK == 1)
               Error( "BATTLE_CheckGameStatus : Got collector item on wrong battle mode!" );
            #else
               return( battle_no_event );
            #endif
            }
         BATTLE_Points[ team ]++;
         UpdateKills = true;

         BATTLE_NumCollectorItems--;
         if ( BATTLE_NumCollectorItems <= 0 )
            {
            RoundOver = true;
            return( battle_end_game );
            }
         break;

      case battle_caught_eluder :
         if ( BattleMode == battle_Deluder )
            {
            return( battle_no_event );
            }

         if ( BattleMode != battle_Eluder )
            {
            #if (BATTLECHECK == 1)
               Error( "BATTLE_CheckGameStatus : Caught Eluder on non-Eluder battle mode!" );
            #else
               return( battle_no_event );
            #endif
            }

         BATTLE_Points[ team ]++;
         UpdateKills = true;

         if ( ( gamestate.BattleOptions.Kills != bo_kills_infinite ) &&
            ( BATTLE_Points[ team ] >= PointGoal ) )
            {
            RoundOver = true;
            return( battle_end_game );
            }
         RespawnEluder();
         break;

      case battle_shot_deluder :
         if ( BattleMode == battle_Eluder )
            {
            return( battle_no_event );
            }

         if ( BattleMode != battle_Deluder )
            {
            #if (BATTLECHECK == 1)
               Error( "BATTLE_CheckGameStatus : Shot Eluder on non-Eluder battle mode!" );
            #else
               return( battle_no_event );
            #endif
            }

         BATTLE_Points[ team ]++;
         UpdateKills = true;

         if ( ( gamestate.BattleOptions.Kills != bo_kills_infinite ) &&
            ( BATTLE_Points[ team ] >= PointGoal ) )
            {
            RoundOver = true;
            return( battle_end_game );
            }
         RespawnEluder();
         break;

      case battle_captured_triad :
         if ( BattleMode != battle_CaptureTheTriad )
            {
            #if (BATTLECHECK == 1)
               Error( "BATTLE_CheckGameStatus : Triad Captured on invalid battle mode!" );
            #else
               return( battle_no_event );
            #endif
            }

         if ( consoleplayer == player )
            {
            AddMessage( "You captured a triad!  You rule!", MSG_GAME );
            }

         BATTLE_Points[ team ]++;
         UpdateKills = true;

         if ( ( gamestate.BattleOptions.Kills != bo_kills_infinite ) &&
            ( BATTLE_Points[ team ] >= PointGoal ) )
            {
            RoundOver = true;
            return( battle_end_game );
            }
         break;

      default :
         #if (BATTLECHECK == 1)
            Error( "BATTLE_CheckGameStatus called with a reason of %d.",
               reason );
         #else
            return( battle_no_event );
         #endif
         break;
      }

   return( status );
   }

/*---------------------------------------------------------------------
   Function: BATTLE_SortPlayerRanks

   Sorts the players in order of score.
---------------------------------------------------------------------*/

void BATTLE_SortPlayerRanks
   (
   void
   )

   {
   int i;
   int j;
   int temp;

   SwapFlag = false;

   if ( BattleMode == battle_Tag )
      {
      for( i = 0; i < BATTLE_NumberOfTeams - 1; i++ )
         {
         for( j = i + 1; j < BATTLE_NumberOfTeams; j++ )
            {
            if ( BATTLE_Points[ BATTLE_PlayerOrder[ i ] ] >
               BATTLE_Points[ BATTLE_PlayerOrder[ j ] ] )
               {
               SwapFlag = true;
               temp = BATTLE_PlayerOrder[ i ];
               BATTLE_PlayerOrder[ i ] = BATTLE_PlayerOrder[ j ];
               BATTLE_PlayerOrder[ j ] = temp;
               }
            }
         }
      }
   else
      {
      for( i = 0; i < BATTLE_NumberOfTeams - 1; i++ )
         {
         for( j = i + 1; j < BATTLE_NumberOfTeams; j++ )
            {
            if ( BATTLE_Points[ BATTLE_PlayerOrder[ i ] ] <
               BATTLE_Points[ BATTLE_PlayerOrder[ j ] ] )
               {
               SwapFlag = true;
               temp = BATTLE_PlayerOrder[ i ];
               BATTLE_PlayerOrder[ i ] = BATTLE_PlayerOrder[ j ];
               BATTLE_PlayerOrder[ j ] = temp;
               }
            }

         if ( BattleMode != battle_Hunter )
            {
            BATTLE_It = BATTLE_PlayerOrder[ 0 ];
            }
         }
      }

   #if (BATTLEINFO == 1)
      for( i = 0; i < BATTLE_NumberOfTeams; i++ )
         {
         SoftError( "Sorted rank %d = player %d : Score = %d\n", i,
            BATTLE_PlayerOrder[ i ], BATTLE_Points[ BATTLE_PlayerOrder[ i ] ] );
         }
   #endif

   if ( ( SwapFlag == true ) && ( gamestate.ShowScores ) &&
      ( SHOW_TOP_STATUS_BAR() || SHOW_KILLS() ) )
      {
      SD_Play ( SD_ENDBONUS1SND );
      }
   }


/*---------------------------------------------------------------------
   Function: BATTLE_PlayerKilledPlayer

   Increases the number of kills a player has.
---------------------------------------------------------------------*/

battle_status BATTLE_PlayerKilledPlayer
   (
   battle_event reason,
   int killer,
   int victim
   )

   {
   int points;
   int status;
   int killerteam;
   int victimteam;

   #if (BATTLEINFO == 1)
      SoftError( "PlayerKilledPlayer:\nMode = %d\n", BattleMode );
      SoftError( "Reason = %d\n", reason );
      SoftError( "killer = %d, team = %d\n", killer, killerteam );
      SoftError( "victim = %d, team = %d\n", victim, victimteam );
      SoftError( "---ticks = %d\n", Timer );
   #endif

   if ( ( killer < 0 ) || ( killer >= MAXPLAYERS ) )
      {
      #if (BATTLECHECK == 1)
         Error( "BATTLE_PlayerKilledPlayer - reason %d : Killer out of range!\n",
            reason );
      #else
         return( battle_no_event );
      #endif
      }
   if ( ( victim < 0 ) || ( victim >= MAXPLAYERS ) )
      {
      #if (BATTLECHECK == 1)
         Error( "BATTLE_PlayerKilledPlayer - reason %d : Victim out of range!\n",
            reason );
      #else
         return( battle_no_event );
      #endif
      }

   if ( ( killer == victim ) && ( reason != battle_kill_with_missile ) &&
      ( reason != battle_kill_with_missile_in_air ) )
      {
      #if (BATTLECHECK == 1)
         Error( "BATTLE_PlayerKilledPlayer : Player "
         "killed self with illegal reason of %d.", reason );
      #else
         return( battle_no_event );
      #endif
      }

   killerteam = BATTLE_Team[ killer ];
   victimteam = BATTLE_Team[ victim ];

   if ( ( killerteam < 0 ) || ( killerteam >= BATTLE_NumberOfTeams ) ||
      ( victimteam < 0 ) || ( victimteam >= BATTLE_NumberOfTeams ) )
      {
      #if (BATTLECHECK == 1)
         Error( "BATTLE_PlayerKilledPlayer - reason %d : Team out of range!\n",
            reason );
      #else
         return( battle_no_event );
      #endif
      }

   if ( !BATTLEMODE )
      {
      return( battle_no_event );
      }

   if ( ( consoleplayer == victim ) &&
      ( reason == battle_kill_by_crushing ) )
      {
      AddMessage( "Oh yeah.  You've been crushed.", MSG_GAME );
      }

   status = battle_no_event;
   if ( BattleMode == battle_ScoreMore )
      {
      points = 0;
      switch( reason )
         {
         case battle_kill_with_missile :
            points = 1;
            break;

         case battle_kill_with_bullet :
            points = 2;
            break;

         case battle_kill_with_missile_in_air :
            points = 2;
            break;

         case battle_kill_with_bullet_in_air :
            points = 3;
            break;

         case battle_kill_by_crushing :
            points = 4;
            break;

         default :
            #if (BATTLECHECK == 1)
               Error( "BATTLE_PlayerKilledPlayer called with a reason of %d.",
                  reason );
            #else
               return( battle_no_event );
            #endif
         }

      if ( killerteam == victimteam )
         {
         if ( BattleOptions.FriendlyFire )
            {
            BATTLE_Points[ killerteam ]--;
            WhoKilledWho[ killer ][ victim ]++;
            }
         }
      else
         {
         BATTLE_Points[ killerteam ]      += points;
         WhoKilledWho[ killer ][ victim ] += points;
         }
      UpdateKills = true;
      }
   else if ( BattleMode == battle_Tag )
      {
      if ( reason == battle_player_tagged )
         {
         WhoKilledWho[ killer ][ victim ]++;
         BATTLE_Points[ victimteam ]++;
         UpdateKills = true;
         BATTLE_It   = victimteam;

         if ( ( gamestate.BattleOptions.Kills != bo_kills_infinite ) &&
            ( BATTLE_Points[ victimteam ] >= PointGoal ) )
            {
            RoundOver = true;
            status = battle_end_game;
            }
         }
      #if (BATTLECHECK == 1)
      else if ( reason != battle_kill_by_crushing )
         {
         Error( "BATTLE_PlayerKilledPlayer - reason %d : "
            "Illegal reason in Tag!\n", reason );
         }
      #endif

      return( status );
      }
   else if ( BattleMode == battle_Hunter )
      {
      switch( reason )
         {
         case battle_kill_with_missile :
         case battle_kill_with_bullet :
         case battle_kill_with_missile_in_air :
         case battle_kill_with_bullet_in_air :
         case battle_kill_by_crushing :
            if ( victimteam == BATTLE_It )
               {
               WhoKilledWho[ killer ][ victim ]++;
               if ( killerteam == victimteam )
                  {
                  if ( BattleOptions.FriendlyFire )
                     {
                     BATTLE_Points[ killerteam ]--;
                     UpdateKills = true;
                     }
                  }
               else
                  {
                  BATTLE_Points[ killerteam ]++;
                  UpdateKills = true;
                  }
               }
            break;
         default :
         #if (BATTLECHECK == 1)
            Error( "BATTLE_PlayerKilledPlayer called with a "
               "reason of %d in Hunter.", reason );
         #else
	    ;
         #endif
         }
      }
   else
      {
      switch( reason )
         {
         case battle_kill_with_missile :
         case battle_kill_with_bullet :
         case battle_kill_with_missile_in_air :
         case battle_kill_with_bullet_in_air :
         case battle_kill_by_crushing :
            WhoKilledWho[ killer ][ victim ]++;
            if ( KeepTrackOfKills )
               {
               if ( killerteam == victimteam )
                  {
                  if ( BattleMode == battle_Normal )
                     {
                     if ( BattleOptions.FriendlyFire )
                        {
                        BATTLE_Points[ killerteam ]--;
                        UpdateKills = true;
                        }
                     }
                  }
               else
                  {
                  BATTLE_Points[ killerteam ]++;
                  UpdateKills = true;
                  }
               }
            break;

         default :
            #if (BATTLECHECK == 1)
               Error( "BATTLE_PlayerKilledPlayer called with a reason of %d.",
                  reason );
            #else
               return( battle_no_event );
            #endif
         }
      }

   if ( ( KillsEndGame ) && ( BATTLE_Points[ killerteam ] >= PointGoal ) )
      {
      RoundOver = true;
      status = battle_end_game;
      }

   return( status );
   }
