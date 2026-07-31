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
#include "rt_def.h"
#include "rt_debug.h"
#include "isr.h"
#include "rt_game.h"
#include "rt_menu.h"
#include "rt_build.h"
#include "rt_str.h"
#include "rt_vid.h"
#include "rt_playr.h"
#include "rt_main.h"
#include "rt_util.h"
#include "rt_draw.h"
#include "rt_in.h"
#include "z_zone.h"
#include "rt_ted.h"
#include "rt_view.h"
#include "develop.h"
#include "rt_msg.h"
#include "rt_net.h"
#include "rt_sound.h"
#include "rt_stat.h"
#include "rt_map.h"

#include <stdlib.h>
#include <ctype.h>
//MED
#include "memcheck.h"
#include "w_wad.h"

extern int		iDemoNames;
extern boolean iG_aimCross;


extern void DisplayMessage   (int num,int position);


typedef struct {
   char code[15];
   byte length;
} CodeStruct;

enum
{
   ENABLECHEAT,               // enable cheats
   ENABLECHEATALT,            // enable cheats
   SOMEITEMS,                 // three keys, more health
   SOMEITEMSALT,              // three keys, more health
   INVULNERABLE,              // normal god mode
   INVULNERABLEALT,           // normal god mode
   WARP,                      // warp
   WARPALT,                   // warp
   ITEMS,                     // all keys, armor, 100% health
   ITEMSALT,                  // all keys, armor, 100% health
   GODMODEPWUP,               // god mode powerup
	GODMODEPWUPALT,            // god mode powerup
#if (SHAREWARE == 0)
   DOGMODEPWUP,               // dog mode powerup
   DOGMODEPWUPALT,            // dog mode powerup
#endif
	MERCURYPWUP,               // mercury mode powerup
   MERCURYPWUPALT,            // mercury mode powerup
   SHROOMSPWUP,               // shrooms mode powerup
   SHROOMSPWUPALT,            // shrooms mode powerup
   ELASTOPWUP,                // elasto mode powerup
   ELASTOPWUPALT,             // elasto mode powerup
   RESTARTGAME,               // warp to level 1, start with pistol
   RESTARTGAMEALT,            // warp to level 1, start with pistol
   HURTPLAYER,                // hurt player 10%
   HURTPLAYERALT,             // hurt player 10%
   TOMHALLMODE,               // run fast all the time
	TOMHALLMODEALT,            // run fast all the time
   NORMAL,                    // back to normal
   NORMALALT,                 // back to normal
   LIGHTDIMON,                // light diminishing on
   LIGHTDIMONALT,             // light diminishing on
   LIGHTDIMOFF,               // light diminishing off
   LIGHTDIMOFFALT,            // light diminishing off
   FOGON,                     // fog on (0x00 - 0x80 minmax)
   FOGONALT,                  // fog on (0x00 - 0x80 minmax)
   FOGOFF,                    // fog off (0x80 - 0xFF minmax)
   FOGOFFALT,                 // fog off (0x80 - 0xFF minmax)
   QUITGAME,                  // blow out of game
   QUITGAMEALT,               // blow out of game
   ENDLEVEL,                  // end the current level
   ENDLEVELALT,               // end the current level
   FANDCOFF,                  // floor and ceiling off
   FANDCOFFALT,               // floor and ceiling off
   FANDCON,                   // floor and ceiling on
   FANDCONALT,                // floor and ceiling on
   BULLETARMOR,               // bullet proof armor
   BULLETARMORALT,            // bullet proof armor
   FIREARMOR,                 // fire proof armor
   FIREARMORALT,              // fire proof armor
   GASMASK,                   // gas mask
   GASMASKALT,                // gas mask
   OUTFIT,                    // all keys, armor, 100% health, MP40, heatseek
   OUTFITALT,                 // all keys, armor, 100% health, MP40, heatseek
   KILLPLAYER,                // kill player
	KILLPLAYERALT,             // kill player
	RESTARTLEVEL,              // re-enter level
	RESTARTLEVELALT,           // re-enter level
   WEAPONTWOPISTOL,           // give double pistol
   WEAPONTWOPISTOLALT,        // give double pistol
   WEAPONMP40,                // give mp40
   WEAPONMP40ALT,             // give mp40
   WEAPONBAZOOKA,             // give bazooka
   WEAPONBAZOOKAALT,          // give bazooka
   WEAPONHEAT,                // give heatseeker
   WEAPONHEATALT,             // give heatseeker
   WEAPONDRUNK,               // give drunk missile
   WEAPONDRUNKALT,            // give drunk missile
   WEAPONFIREBOMB,            // give firebomb
   WEAPONFIREBOMBALT,         // give firebomb
   WEAPONFIREWALL,            // give firewall
   WEAPONFIREWALLALT,         // give firewall
	WEAPONGOD,                 // give godhand
	WEAPONGODALT,              // give godhand
	AIMCROSS,                 // bna++
	AIMCROSSALT,              // give bna++

#if (SHAREWARE == 0)

   WEAPONSPLIT,               // give split missile
   WEAPONSPLITALT,            // give split missile
   WEAPONKES,                 // give kes
   WEAPONKESALT,              // give kes
   WEAPONBAT,                 // give bat
   WEAPONBATALT,              // give bat
   WEAPONDOG,                 // give dogmode
   WEAPONDOGALT,              // give dogmode
#endif

   MISSILECAMTOGGLE,          // Turn missile cam on/off
   MISSILECAMTOGGLEALT,       // Turn missile cam on/off
   HUDTOGGLE,                 // Turn HUD on/off
   HUDTOGGLEALT,              // Turn HUD on/off
   ROTATIONFUN,               // Rotation fun
   DEMORECORD,                // Start recording demo
   DEMOEND,                   // End recording demo
   DEMOPLAYBACK,              // Playback demo
   CRAZYGIBS,                 // Engine Killing Gibs
   JUKEBOX,                   // JukeBox
   JUKEBOXALT,                // JukeBox
   MAPCHEAT,                  // Map Cheat
   MAPCHEATALT,               // Map Cheat Alt
   MAXCODES
};

CodeStruct Codes[MAXCODES + 6] =
{
	{"KCITSPID",    8},        // enable cheats
	{"CCE\\",       4},        // enable cheats
   {"REKCALS",     7},        // three keys, more health
   {"MUB\\",       4},        // three keys, more health
   {"NIJOHC",      6},        // normal god mode
	{"WWW\\",       4},        // normal god mode
	{"OTOG",        4},        // warp
	{"LTG\\",       4},        // warp
	{"SYOTXIS",     7},        // all keys, armor, 100% health
	{"IAG\\",       4},        // all keys, armor, 100% health
   {"DASOOT",      6},        // god mode powerup
   {"DOG\\",       4},        // god mode powerup
#if (SHAREWARE == 0)
   {"FOOW",        4},        // dog mode powerup
	{"GOD\\",       4},        // dog mode powerup
#endif
	{"YOBYLF",      6},        // mercury mode powerup
	{"REM\\",       4},        // mercury mode powerup
	{"PIRTDAB",     7},        // shrooms mode powerup
	{"RHS\\",       4},        // shrooms mode powerup
	{"GNIOB",       5},        // elasto mode powerup
	{"ALE\\",       4},        // elasto mode powerup
	{"SREBOOG",     7},        // warp to level 1, start with pistol
	{"OOG\\",       4},        // warp to level 1, start with pistol
	{"KCAHW",       5},        // hurt player 10%
	{"FOO\\",       4},        // hurt player 10%
	{"DEEPS",       5},        // run fast all the time
	{"AFR\\",       4},        // run fast all the time
	{"CINAP",       5},        // back to normal
	{"NAP\\",       4},        // back to normal
	{"NOMID",       5},        // light diminishing on
	{"NOD\\",       4},        // light diminishing on
	{"FFOMID",      6},        // light diminishing off
	{"FOD\\",       4},        // light diminishing off
	{"NODNOL",      6},        // fog on (0x00 - 0x80 minmax)
	{"NOF\\",       4},        // fog on (0x00 - 0x80 minmax)
	{"LONDON",      6},        // fog off (0x80 - 0xFF minmax)
	{"FOF\\",       4},        // fog off (0x80 - 0xFF minmax)
	{"SETAGOG",     7},        // blow out of game
	{"R8L\\",       4},        // blow out of game
	{"HCRAOG",      6},        // end the current level
	{"LCE\\",       4},        // end the current level
	{"683ATOG",     7},        // floor and ceiling off
	{"NOC\\",       4},        // floor and ceiling off
	{"684ATOG",     7},        // floor and ceiling on
	{"FOC\\",       4},        // floor and ceiling on
	{"EMTOOHS",     7},        // bullet proof armor
	{"RAB\\",       4},        // bullet proof armor
	{"EMNRUB",      6},        // fire proof armor
	{"RAF\\",       4},        // fire proof armor
	{"GNUDGNUL",    8},        // gas mask
	{"RAG\\",       4},        // gas mask
	{"KCAPTNUH",    8},        // all keys, armor, 100% health, MP40, heatseek
	{"PFO\\",       4},        // all keys, armor, 100% health, MP40, heatseek
	{"EM68",        4},        // kill player
	{"EID\\",       4},        // kill player
	{"REEN",        4},        // re-enter level
	{"LER\\",       4},        // re-enter level
   {"OOWNHOJ",     7},        // give double pistol
	{"2WG\\",       4},        // give double pistol
	{"EMGULP",      6},        // give mp40
	{"3WG\\",       4},        // give mp40
	{"ALLINAV",     7},        // give bazooka
	{"4WG\\",       4},        // give bazooka
	{"SEMITTOH",    8},        // give heatseeker
   {"5WG\\",       4},        // give heatseeker
   {"EZOOB",       5},        // give drunk missile
   {"6WG\\",       4},        // give drunk missile
   {"BMOBERIF",    8},        // give firebomb
   {"7WG\\",       4},        // give firebomb
	{"SENOB",       5},        // give firewall
	{"8WG\\",       4},        // give firewall
	{"AYEES",       5},        // give god hand
   {"9WG\\",       4},        // give god hand
	{"MIA",       3},        // give aim bna++
   {"MIA\\",       4},        // give aim bna++



#if (SHAREWARE == 0)

   {"TILPS",       5},        // give split missile
   {"AWG\\",       4},        // give split missile
	{"HTAEDFOSEK", 10},        // give kes
   {"BWG\\",       4},        // give kes
	{"NUREMOH",     8},        // give bat
   {"CWG\\",       4},        // give bat
   {"OJUC",        4},        // give dog weapon
   {"DWG\\",       4},        // give dog weapon
#endif
   {"EDIR",        4},        // give MISSILE CAM
   {"MAC\\",       4},        // give Missile Cam
   {"EREHW",       5},        // turn where am i on/off
   {"DUH\\",       4},        // give hud
   {"NUF\\",       4},        // Rotation fun
   {"DROCER",      6},        // Demo RECORD
   {"POTS",        4},        // Demo stop recording
   {"YALP",        4},        // Demo Playback
   {"GKE\\",       4},        // Engine Killing Gibs
   {"ORTSEAM",     7},        // JukeBox
   {"EEL\\",       4},        // JukeBox
   {"REITRAC",     7},        // Map Cheat
   {"PAM\\",       4},        // Map Cheat
   {"UOY\\",       4},        // Secret Message
   {"EVAH",        4},        // Secret Message
   {"ON\\",        3},        // Secret Message
   {"EFIL",        4},        // Secret Message

};




/*
================
=
= CheatSpawnItem
=
================
*/

void CheatSpawnItem (int item)
   {
   SpawnStatic(player->tilex, player->tiley, item,-1);
   LASTSTAT->z = player->z;
	MakeStatActive(LASTSTAT);
	LASTSTAT->flags|=FL_ABP;
   }


/*
================
=
= FixupPowerupsY
=
================
*/

void FixupPowerupsY (void)
{
	player->z   = nominalheight;
}


/*
================
=
= EnableCheatCodes
=
================
*/

void EnableCheatCodes (void)
{
	DebugOk ^= 1;

	if (DebugOk)
      AddMessage ("Cheat Codes \\cENABLED!", MSG_CHEAT);
	else
      AddMessage ("Cheat Codes \\cDISABLED!", MSG_CHEAT);
}


/*
================
=
= ResetCheatCodes
=
================
*/

void ResetCheatCodes (void)
{
//	godmode = false;
}

/*
================
=
= DoMapCheat
=
================
*/

void DoMapCheat (void)
{
   AddMessage ("Entire Map Revealed!", MSG_CHEAT);
   CheatMap ();
}

/*
================
=
= DoGodMode
=
================
*/

void DoGodMode (void)
{
	if (godmode)
      AddMessage ("Woundless With Weapons \\cOFF", MSG_CHEAT);
	else
      AddMessage ("Woundless With Weapons \\cON", MSG_CHEAT);

	godmode ^= 1;
}


/*
================
=
= DoWarp
=
================
*/
#include "byteordr.h"//bna++
void DoWarp (void)
{
/*
	char str[10];
	boolean esc;
	int level;

   CurrentFont = smallfont;

   US_CenterWindow(26,3);
	PrintY+=6;

   US_Print(" Warp to level(1-99):");

   VW_UpdateScreen();

   ShutdownClientControls();

   esc = !US_LineInput (px, py, str, NULL, true, 2, 25, 13);

   if (!esc)
   {
      level = ParseNum (str);
      if (level>0 && level<=99)
      {
         gamestate.mapon = level-1;
         playstate = ex_warped;
         gamestate.episode = GetEpisode (gamestate.mapon);
      }
   }

   while (Keyboard[sc_Escape])
		IN_UpdateKeyboard ();
   IN_ClearKeyboardQueue ();

*/

   int level;


	EnableScreenStretch();//bna++ shut on streech mode


   MU_StoreSongPosition();
   MU_StartSong( song_secretmenu);
   StopWind();
   ShutdownClientControls();

   SetupMenuBuf();
   SetUpControlPanel ();

   level = CP_LevelSelectionMenu();

   CleanUpControlPanel();
   ShutdownMenuBuf();

   	//bna++ section
  if (( playstate == ex_stillplaying )&&(iGLOBAL_SCREENWIDTH > 320)){
		pic_t *shape;
		shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );
		DrawTiledRegion( 0, 16, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT - 32, 0, 16, shape );
		DisableScreenStretch();//dont strech when we go BACK TO GAME
		DrawPlayScreen(true);//repaint ammo and life stat
		VW_UpdateScreen ();//update screen
  }
   //bna section end

	EnableScreenStretch();//bna++ shut on streech mode
   while( Keyboard[ sc_Escape ] )
      {
		IN_UpdateKeyboard();
      }
   IN_ClearKeyboardQueue();

   if ((level == -1) || (level == gamestate.mapon))
      {
      MU_StartSong(song_level);
      MU_RestoreSongPosition();
      }

   if ( level >= 0 )
      {
      playstate = ex_warped;
      gamestate.mapon   = level;
      
      GetEpisode( gamestate.mapon );

      VL_FadeOut (0, 255, 0, 0, 0, 20);
      }
   else
      {
	   DisableScreenStretch();//dont strech when we go BACK TO GAME
      SetupScreen(true);
      }

   StartupClientControls();
   }

/*
================
=
= DoJukeBox
=
================
*/

void DoJukeBox  (void)

   {
	if (iGLOBAL_SCREENWIDTH > 320) {
		EnableScreenStretch();//bna++ shut on streech mode
	}
   StopWind();
   ShutdownClientControls();

   SetupMenuBuf();
   SetUpControlPanel ();

   MU_JukeBoxMenu();

   CleanUpControlPanel();
   ShutdownMenuBuf();

   	//bna++ section
  if (( playstate == ex_stillplaying )&&(iGLOBAL_SCREENWIDTH > 320)){
		pic_t *shape;
		shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );
		DrawTiledRegion( 0, 16, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT - 32, 0, 16, shape );
		DisableScreenStretch();//dont strech when we go BACK TO GAME
		DrawPlayScreen(true);//repaint ammo and life stat
		VW_UpdateScreen ();//update screen
  }
   //bna section end

   SetupScreen(true);

   while( Keyboard[ sc_Escape ] )
      {
		IN_UpdateKeyboard();
      }
   IN_ClearKeyboardQueue();

   StartupClientControls();
   }



/*
================
=
= DoNormalThing
=
================
*/

void DoNormalThing (void)
{
   AddMessage ("BACK TO NORMAL.  AH.", MSG_CHEAT);

	player->flags &= ~(FL_BPV|FL_AV|FL_GASMASK);
	locplayerstate->protectiontime = 0;

   if (player->flags & FL_ELASTO)
      player->flags &= ~FL_NOFRICTION;
   player->flags &= ~(FL_SHROOMS|FL_ELASTO|FL_FLEET);
	locplayerstate->poweruptime = 0;

	InitializeWeapons(locplayerstate);
	locplayerstate->keys = 0;

   DrawPlayScreen (false);
}



/*
================
=
= DoItemCheat
=
================
*/

void DoItemCheat (void)
{
   AddMessage ("Items Aplenty!", MSG_CHEAT);

	GivePoints (100000);
	HealPlayer (99, player);

	locplayerstate->keys = 0xF;
	DrawKeys (false);

   player->flags &= ~(FL_GASMASK|FL_BPV|FL_AV);
   CheatSpawnItem(stat_bulletproof);

   /*
   player->flags |= FL_BPV;
	player->flags &= ~(FL_GASMASK|FL_AV);
	locplayerstate->protectiontime = POWERUPTICS;
   GM_DrawBonus (stat_bulletproof);
   */
}

/*
================
=
= DoSomeItemCheat
=
================
*/

void DoSomeItemCheat (void)
{
   AddMessage ("Slacker pack!", MSG_CHEAT);

   HealPlayer (40, player);

   locplayerstate->keys = 0x7;
	DrawKeys (false);
}


/*
================
=
= DoGodModePowerup
=
================
*/

void DoGodModePowerup (void)
   {

   if (PLAYER[0]->flags & FL_GODMODE)
	  return;

   CheatSpawnItem(stat_godmode);
   }


/*
================
=
= DoDogModePowerup
=
================
*/

void DoDogModePowerup (void)
   {

   if (PLAYER[0]->flags & FL_DOGMODE)
	  return;

   CheatSpawnItem(stat_dogmode);
   }


/*
================
=
= DoMercuryModePowerup
=
================
*/

void DoMercuryModePowerup (void)
   {

   if (PLAYER[0]->flags & FL_FLEET)
	  return;

   CheatSpawnItem(stat_fleetfeet);
   }


/*
================
=
= DoElastoModePowerup
=
================
*/

void DoElastoModePowerup (void)
   {
   if (PLAYER[0]->flags & FL_ELASTO)
	  return;

   CheatSpawnItem(stat_elastic);
   }


/*
================
=
= DoShroomsModePowerup
=
================
*/

void DoShroomsModePowerup (void)
   {
   if (PLAYER[0]->flags & FL_SHROOMS)
	  return;

   AddMessage ("SHROOMS MODE POWERDOWN!", MSG_CHEAT);

   CheatSpawnItem(stat_mushroom);
   }


/*
================
=
= RestartNormal
=
================
*/

void RestartNormal (void)
{
	EnableScreenStretch();//bna
	DoNormalThing ();

   AddMessage ("Restart to level 1", MSG_CHEAT);
	gamestate.mapon   = 0;
	playstate         = ex_warped;
	
	GetEpisode (gamestate.mapon);
}


/*
================
=
= HurtPlayer
=
================
*/

void HurtPlayer (void)
{
   int oldhitpoints;

   oldhitpoints = player->hitpoints;
	DamageThing (player, MaxHitpointsForCharacter(locplayerstate) / 10);
   if (player->hitpoints < oldhitpoints)
      AddMessage ("OUCH!!!!", MSG_CHEAT);

   Collision(player,player,0,0);
}


/*
================
=
= SetLightDiminish
=
================
*/

void SetLightDiminish (boolean off)
{
	if (off)
	{
      AddMessage ("Light Diminishing \\cOff", MSG_CHEAT);
		fulllight = 1;
	}
	else
	{
      AddMessage ("Light Diminishing \\cOn", MSG_CHEAT);
		fulllight = 0;
	}
}


/*
================
=
= SetFog
=
================
*/

void SetFog (boolean on)
{
	if (on)
	{
      AddMessage ("Fog \\cOn", MSG_CHEAT);
		MAPSPOT(2,0,1)=105;
		MAPSPOT(3,0,1)=0;
		SetupLightLevels ();

	}
	else
	{
      AddMessage ("Fog \\cOff", MSG_CHEAT);
		MAPSPOT(2,0,1)=104;
		SetupLightLevels ();
	}
}



/*
================
=
= ToggleMissileCam
=
================
*/

void ToggleMissileCam (void)
{
   if (missilecam==false)
      {
      missilecam=true;
      AddMessage ("Missile Cam \\cOn", MSG_CHEAT);
      }
   else
      {
      missilecam=false;
      AddMessage ("Missile Cam \\cOff", MSG_CHEAT);
      }
}

/*
================
=
= ToggleHUD
=
================
*/

void ToggleHUD (void)
{
   if (HUD==false)
      {
      HUD=true;
      AddMessage ("HUD \\cOn", MSG_CHEAT);
      }
   else
      {
      HUD=false;
      AddMessage ("HUD \\cOff", MSG_CHEAT);
      }
}



/*
================
=
= EndLevel
=
================
*/

void EndLevel (void)
{
   AddMessage ("End Level", MSG_CHEAT);
   playstate = ex_skiplevel;
}


/*
================
=
= FloorandCeiling
=
================
*/

void FloorandCeiling (boolean off)
{
	if (off)
	{
      AddMessage ("Floor and Ceiling \\cON", MSG_CHEAT);
		fandc = 1;
	}
	else
	{
      AddMessage ("Floor and Ceiling \\cOFF", MSG_CHEAT);
		fandc = 0;
	}
}


/*
================
=
= GiveGasMask
=
================
*/

void GiveGasMask ()
   {

	if (PLAYER[0]->flags & FL_GASMASK)
     return;

   CheatSpawnItem(stat_gasmask);
   }

/*
================
=
= GiveBulletProofArmor
=
================
*/

void GiveBulletProofArmor ()
   {
	if (PLAYER[0]->flags & FL_BPV)
     return;


   CheatSpawnItem(stat_bulletproof);
   }

/*
================
=
= GiveAsbestoArmor
=
================
*/

void GiveAsbestoArmor ()
   {
	if (PLAYER[0]->flags & FL_AV)
     return;

   CheatSpawnItem(stat_asbesto);
   }


/*
================
=
= OutfitPlayer
=
================
*/
void OutfitPlayer ()
{
   AddMessage ("Outfit Player!", MSG_CHEAT);

	locplayerstate->keys = 0xF;
	DrawKeys (false);
	HealPlayer (99, player);

   /*
   player->flags |= FL_BPV;
	player->flags &= ~(FL_GASMASK|FL_AV);
	locplayerstate->protectiontime = POWERUPTICS;
   GM_DrawBonus (stat_bulletproof);

	GiveWeapon (player,wp_mp40);
   */

#if (SHAREWARE == 0)
   CheatSpawnItem(stat_splitmissile);
#else
   CheatSpawnItem(stat_heatseeker);
#endif


}

/*
================
=
= KillPlayer
=
================
*/

void KillPlayer ()
{
   AddMessage ("Say Goodnight.", MSG_CHEAT);
	playstate = ex_died;
}



/*
================
=
= RestartCurrentLevel
=
================
*/

void RestartCurrentLevel (void)
{
	playstate         = ex_warped;
	
	GetEpisode (gamestate.mapon);
}

/*
================
=
= EndDemo
=
================
*/
void EndDemo ( void )
{
	char str[10];
	boolean esc;
	int demonumber;

   if (demorecord==false)
      return;

   ShutdownClientControls();

   CurrentFont = smallfont;

   demorecord = false;
   US_CenterWindow (26, 4);
   US_CPrint ("Save demo as:");
   US_Print  ("\n");
	US_CPrint ("Demo Number (1-4):");

   VW_UpdateScreen();

   esc = !US_LineInput (px, py, str, NULL, true, 1, 25, 13);

   if (!esc)
   {
      demonumber = ParseNum (str);
      if ((demonumber > 0) && (demonumber < 5))
      {
         SaveDemo (demonumber);
      }
   }

   IN_ClearKeysDown ();

   while (Keyboard[sc_Enter])
      IN_UpdateKeyboard ();
   while (Keyboard[sc_Escape])
      IN_UpdateKeyboard ();
	IN_ClearKeyboardQueue ();

   StartupClientControls();
   DisableScreenStretch();
}


/*
================
=
= RecordDemoQuery
=
================
*/
void RecordDemoQuery ( void )
{
   char str[10];
	boolean esc;
	int level;

   ShutdownClientControls();

   CurrentFont = smallfont;

   US_CenterWindow (26, 5);
   PrintY += 6;

   US_CPrint ("Record Demo");
   US_Print  ("\n");
#if (SHAREWARE==0)
   US_CPrint ("Which level (1-36):");
#else
   US_CPrint ("Which level (1-8):");
#endif

   VW_UpdateScreen();

   esc = !US_LineInput (px, py, str, NULL, true, 2, 25, 13);

   if (!esc)
      {
      level = ParseNum (str);
#if (SHAREWARE==0)
      if ((level > 0) && (level < 37))
#else
      if ((level > 0) && (level < 9))
#endif
         {
			EnableScreenStretch();//bna
			 gamestate.mapon = level-1;
			 playstate = ex_demorecord;
         }
      }

   while (Keyboard[sc_Enter])
      IN_UpdateKeyboard ();
   while (Keyboard[sc_Escape])
      IN_UpdateKeyboard ();
   IN_ClearKeyboardQueue ();

   StartupClientControls();
}

/*
================
=
= PlaybackDemoQuery
=
================
*/
void PlaybackDemoQuery ( void )
{
   char str[10];
	boolean esc;
	int level;

   ShutdownClientControls();

   CurrentFont = smallfont;
   US_CenterWindow (33, 4);
   US_CPrint ("Playback demo");
   US_Print ("\n");
   US_CPrint ("Enter demo number (1-4):");

   VW_UpdateScreen ();

   esc = !US_LineInput (px, py, str, NULL, true, 1, 25, 13);

   if (!esc)
   {
      level = ParseNum (str);
      if ((level > 0) && (level < 5))
         {
         if (DemoExists (level) == true)
            LoadDemo (level);
         }
   }

   while (Keyboard[sc_Enter])
      IN_UpdateKeyboard ();
   while (Keyboard[sc_Escape])
      IN_UpdateKeyboard ();
   IN_ClearKeyboardQueue ();

   StartupClientControls();

   EnableScreenStretch();
}

/*
================
=
= DebugKeys
=
================
*/

int DebugKeys (void)
{
#if (DEVELOPMENT == 1)
	char str[10];
	boolean esc;
	int level;
	int i,f,temp;
	static int whichpowerup    = 0;
	static int whichprotection = 0;

	if (Keyboard[sc_G])     // G = god mode
	{
      DoGodMode ();
      while (Keyboard[sc_G])
         IN_UpdateKeyboard ();
      return 1;
	}
   else if (Keyboard[sc_Q])         // Q = fast quit
      QuitGame ();
   else if (Keyboard[sc_W])         // W = warp to level
   {
      DoWarp ();
      return 1;
   }
	else if (Keyboard[sc_F])     // F = FPS
	{
		f=0;
		for (i=0;i<VBLCOUNTER*10;i+=tics)
			{
			ThreeDRefresh();
			DoSprites();
			CalcTics ();
			f++;
			}

		CurrentFont = smallfont;

		US_CenterWindow (12,2);
		temp=f*10;
		SoftError("fps  = %2ld.%2ld\n",temp/100,temp%100);

		US_Print ("FPS=");
		US_PrintUnsigned (temp/100);
		US_Print (".");
		US_PrintUnsigned (temp%100);

		VW_UpdateScreen();
		IN_Ack();

		return 1;
	}
	else if (Keyboard[sc_H])     // H = hurt self
	{
		IN_ClearKeysDown ();
		HurtPlayer ();
	}
	else if (Keyboard[sc_Z])     // Z = end level
	{
      EndLevel ();
	}
	else if (Keyboard[sc_P])     // P = step through powerups
	{
		whichpowerup++;
		if (whichpowerup == 6)
			whichpowerup = 0;

		switch (whichpowerup)
		{
			case 0:  // nothing
				if (player->flags & FL_ELASTO)
					player->flags &= ~FL_NOFRICTION;
				player->flags &= ~(FL_SHROOMS|FL_ELASTO|FL_FLEET|FL_GODMODE|FL_DOGMODE);
				locplayerstate->poweruptime = 0;
				GM_UpdateBonus (0, true);
			break;

			case 1:  // god mode
            DoGodModePowerup ();
			break;

			case 2:  // dog mode
            DoDogModePowerup ();
			break;

			case 3:  // fleet feet
            DoMercuryModePowerup ();
			break;

			case 4:  // elasto
            DoElastoModePowerup ();
			break;

			case 5:  // shrooms
            DoShroomsModePowerup ();
			break;
		}

		while (Keyboard[sc_P])
			IN_UpdateKeyboard ();
	}
	else if (Keyboard[sc_A])     // step through armor
	{
		whichprotection++;
		if (whichprotection == 4)
			whichprotection = 0;

		switch (whichprotection)
      {
         case 0:  // nothing
            player->flags &= ~(FL_BPV|FL_AV|FL_GASMASK);
				locplayerstate->protectiontime = 0;
            GM_UpdateBonus (0, false);
         break;

         case 1:  // gas mask
            GiveGasMask ();
         break;

         case 2:  // armor
            GiveBulletProofArmor ();
         break;

         case 3:  // fire vest
            GiveAsbestoArmor ();
         break;
      }

      while (Keyboard[sc_A])
         IN_UpdateKeyboard ();
   }
   else if (Keyboard[sc_O])     // O = outfit player
   {
      OutfitPlayer ();

		IN_ClearKeysDown ();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_K])     // K = kill self
	{
		IN_ClearKeysDown ();
      locplayerstate->lives = -1;
      KillPlayer ();
	}
	else if (Keyboard[sc_I])        // I = item cheat
	{
      DoItemCheat ();
		return 1;
	}
	else if (Keyboard[53])        // \ = back to normal
	{
      DoNormalThing ();
      return 1;
   }
   else if (Keyboard[sc_R])
   {
      RecordDemoQuery();
   }
   else if (Keyboard[sc_E])
   {
      EndDemo();
   }
   else if (Keyboard[sc_D])
   {
      PlaybackDemoQuery();
   }
#endif
   return (0);
}


/*
================
=
= WeaponCheat
=
================
*/

void WeaponCheat (int weapon)
   {
   if ((player->flags & FL_GODMODE) || (player->flags & FL_DOGMODE))
      return;

   if ((weapon <= wp_mp40) && (PLAYERSTATE[0].HASBULLETWEAPON[weapon]))
      return;

   CheatSpawnItem(GetItemForWeapon(weapon));

   }



/*
================
=
= CheckCode ()
=
================
*/



void CheckCode (int which)
{
   int pos = (LastLetter-1)&(MAXLETTERS-1);
   int num = 0;
   int start;

   start = pos;

   while ((toupper(LetterQueue[pos]) == Codes[which].code[num]) &&
          (num < Codes[which].length))
   {
      pos = (pos-1)&(MAXLETTERS-1);
      num++;
   }

	if (num == Codes[which].length)
   {
      // Kill last letter so the debug rtn will not keep triggering
      LetterQueue[start] = 0;

      switch (which)
      {
         case ENABLECHEAT:
         case ENABLECHEATALT:
            EnableCheatCodes ();
         break;

         case INVULNERABLE:
         case INVULNERABLEALT:
            DoGodMode ();
         break;

         case WARP:
         case WARPALT:
            DoWarp ();
         break;

         case ITEMS:
         case ITEMSALT:
            DoItemCheat ();
         break;

         case SOMEITEMS:
         case SOMEITEMSALT:
            DoSomeItemCheat ();
         break;

			case GODMODEPWUP:
			case GODMODEPWUPALT:
            DoGodModePowerup ();
         break;

#if (SHAREWARE == 0)
         case DOGMODEPWUP:
         case DOGMODEPWUPALT:
            DoDogModePowerup ();
         break;
#endif

         case MERCURYPWUP:
         case MERCURYPWUPALT:
            DoMercuryModePowerup ();
         break;

         case SHROOMSPWUP:
			case SHROOMSPWUPALT:
            DoShroomsModePowerup ();
         break;

         case ELASTOPWUP:
         case ELASTOPWUPALT:
            DoElastoModePowerup ();
         break;

         case RESTARTGAME:
         case RESTARTGAMEALT:
            RestartNormal ();
         break;

         case HURTPLAYER:
         case HURTPLAYERALT:
               HurtPlayer ();
         break;

         case TOMHALLMODE:
         case TOMHALLMODEALT:
            gamestate.autorun = true;
            AddMessage("Autorun enabled!",MSG_CHEAT);
         break;

         case NORMAL:
         case NORMALALT:
            DoNormalThing ();
         break;

         case LIGHTDIMON:
         case LIGHTDIMONALT:
            SetLightDiminish (false);
         break;

         case LIGHTDIMOFF:
         case LIGHTDIMOFFALT:
            SetLightDiminish (true);
         break;

         case FOGON:
         case FOGONALT:
            SetFog (true);
         break;

			case FOGOFF:
         case FOGOFFALT:
            SetFog (false);
         break;

         case QUITGAME:
         case QUITGAMEALT:
            QuitGame ();
         break;

         case ENDLEVEL:
         case ENDLEVELALT:
				EndLevel ();
         break;

         case FANDCOFF:
         case FANDCOFFALT:
            FloorandCeiling (false);
         break;

         case FANDCON:
         case FANDCONALT:
            FloorandCeiling (true);
         break;


         case AIMCROSS:
         case AIMCROSSALT:
            if (iG_aimCross == 0) {
			    iG_aimCross = 1;
				AddMessage("Crosshair on",MSG_GAME);
			}else{
				iG_aimCross = 0;
				AddMessage("Crosshair off",MSG_GAME);
			}
         break;


         case BULLETARMOR:
         case BULLETARMORALT:
            GiveBulletProofArmor ();
         break;

         case FIREARMOR:
         case FIREARMORALT:
            GiveAsbestoArmor ();
			break;

         case GASMASK:
         case GASMASKALT:
            GiveGasMask ();
         break;

         case OUTFIT:
         case OUTFITALT:
            OutfitPlayer ();
         break;

         case KILLPLAYER:
         case KILLPLAYERALT:
            KillPlayer ();
         break;

         case RESTARTLEVEL:
         case RESTARTLEVELALT:
				RestartCurrentLevel ();
			break;


         case WEAPONTWOPISTOL:
         case WEAPONTWOPISTOLALT:
             WeaponCheat(wp_twopistol);
             break;

         case WEAPONMP40:
         case WEAPONMP40ALT:
             WeaponCheat(wp_mp40);
             break;

         case WEAPONBAZOOKA:
         case WEAPONBAZOOKAALT:
             WeaponCheat(wp_bazooka);
             break;
         case WEAPONFIREBOMB:
         case WEAPONFIREBOMBALT:
             WeaponCheat(wp_firebomb);
             break;

         case WEAPONHEAT:
         case WEAPONHEATALT:
             WeaponCheat(wp_heatseeker);
             break;

         case WEAPONDRUNK:
         case WEAPONDRUNKALT:
             WeaponCheat(wp_drunk);
             break;

         case WEAPONFIREWALL:
         case WEAPONFIREWALLALT:
             WeaponCheat(wp_firewall);
             break;

         case WEAPONGOD:
         case WEAPONGODALT:
             WeaponCheat(wp_godhand);
             break;


#if (SHAREWARE == 0)

         case WEAPONSPLIT:
         case WEAPONSPLITALT:
             WeaponCheat(wp_split);
             break;

         case WEAPONKES:
         case WEAPONKESALT:
             WeaponCheat(wp_kes);
             break;

         case WEAPONBAT:
         case WEAPONBATALT:
             WeaponCheat(wp_bat);
             break;

         case WEAPONDOG:
         case WEAPONDOGALT:
             WeaponCheat(wp_dog);
             break;
#endif

         case MISSILECAMTOGGLE:
         case MISSILECAMTOGGLEALT:
             ToggleMissileCam();
             break;

         case HUDTOGGLE:
         case HUDTOGGLEALT:
             ToggleHUD();
             break;

         case ROTATIONFUN:
            ShutdownClientControls();
            RotationFun();
            StartupClientControls();
            SetupScreen( true );
            break;
         case DEMORECORD:
            RecordDemoQuery();
            break;
         case DEMOEND:
            EndDemo();
            break;
         case DEMOPLAYBACK:
            PlaybackDemoQuery();
            break;
         case CRAZYGIBS:
            if (gamestate.violence == vl_excessive)
               {

               ludicrousgibs ^= 1;
               if (ludicrousgibs == true)
                  AddMessage("EKG mode on!",MSG_GAME);
               else
                  AddMessage("EKG mode off!",MSG_GAME);
               }
            break;

         case JUKEBOX:
         case JUKEBOXALT:
            DoJukeBox();
         break;
         case MAPCHEAT:
         case MAPCHEATALT:
            DoMapCheat();
         break;
      }
   }
}


/*
================
=
= CheckDebug ()
=
================
*/

void CheckDebug (void)
{
	int which;
	if (DebugOk == false)
		{
      CheckCode (0);      // Check for Debug switch only
      CheckCode (1);      // Check for Debug switch only
      CheckCode (2);      // Check for Slacker pack
      CheckCode (3);      // Check for Slacker pack
      }
	else
      {
      if (demoplayback==true)
         {
         return;
         }
      else if (demorecord==true)
         {
         CheckCode (DEMORECORD);
         CheckCode (DEMOEND);
         }
      else
         {
         for (which = 0; which < MAXCODES; which++) // Check all debug codes
            CheckCode (which);
         }
      }
}
