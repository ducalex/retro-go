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
//****************************************************************************
//
// RT_CFG.C
//
//****************************************************************************

#include "rt_def.h"
#define _ROTT_

#ifdef DOS
#include <io.h>
#include <bios.h>
#include <conio.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#ifdef _ROTT_
#else
#include "st_def.h"
#endif

#include "rt_cfg.h"
#include "version.h"

#ifdef _ROTT_

#include "scriplib.h"
#include "rt_playr.h"
#include "rt_menu.h"
#include "rt_game.h"
#include "rt_in.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_crc.h"
#include "rt_sound.h"
#include "rt_util.h"
#include "rt_main.h"
#include "rt_view.h"
#include "rt_msg.h"
#include "rt_battl.h"
#include "rt_net.h"
#include "isr.h"
#include "fx_man.h"
#include "develop.h"

#else

#include "st_def.h"
#include "rt_cfg.h"
#include "scriplib.h"
#include "rt_sound.h"
#include "st_util.h"

#endif
//MED
#include "memcheck.h"


//******************************************************************************
//
// GLOBALS
//
//******************************************************************************

extern int G_weaponscale;
extern boolean iG_aimCross;

boolean WriteSoundFile   = true;

int     FXMode           = 0;
int     MusicMode        = 0;

int     MUvolume         = 196;
int     FXvolume         = 196;

#ifdef DOS
fx_blaster_config SBSettings =
   {
   0x220, fx_SB, 7, 1, 5, 0x330, 0x620
   };
#endif

boolean mouseenabled     = 1;
boolean usemouselook     = 0;
int     inverse_mouse    = 1; //set  to -1 to invert mouse
boolean usejump          = 0;
boolean sdl_fullscreen   = 1;

boolean joystickenabled  = 0;
boolean joypadenabled    = 0;
int     joystickport     = 0;
int     mouseadjustment  = 5;
int     threshold        = 1;
int     NumVoices        = 4;
int     NumChannels      = 1;
int     NumBits          = 8;
#ifdef DOS
int     MidiAddress      = 0x330;
#endif
boolean cybermanenabled  = false;
boolean assassinenabled  = false;
boolean spaceballenabled = false;
boolean AutoDetailOn     = true;
int     DoubleClickSpeed = 20;
boolean BobbinOn         = true;
int     Menuflipspeed    = 15;
int     DetailLevel      = 2;         //HI DETAIL
int     fandc            = 1;
int     blanktime        = (2*60*VBLCOUNTER);
boolean ConfigLoaded     = false;
boolean stereoreversed   = false;

int     DefaultDifficulty      = 2;
int     DefaultPlayerCharacter = 0;
int     DefaultPlayerColor     = 0;
byte    passwordstring[20];

#ifndef _ROTT_

int     fulllight        = 0;
int     viewsize         = 7;

#endif
MacroList CommbatMacros[MAXMACROS];

#ifdef DOS
char *ApogeePath = "APOGEECD";
#else
char ApogeePath[256];
#endif

//******************************************************************************
//
// LOCALS
//
//******************************************************************************

static char SoundName[13]  = "sound.rot";

#ifdef _ROTT_

static char *ConfigName = "config.rot";
static char *ScoresName = "scores.rot";
static char *ROTT       = "rott.rot";
static char *CONFIG     = "setup.rot";
static char *BattleName = "battle.rot";

AlternateInformation RemoteSounds;
//AlternateInformation PlayerGraphics;
AlternateInformation GameLevels;
AlternateInformation BattleLevels;
char CodeName[MAXCODENAMELENGTH];

#endif


#ifdef _ROTT_

//******************************************************************************
//
// ReadScores ()
//
//******************************************************************************

void ReadScores (void)
{
   int file;
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, ScoresName );
   if (access (filename, F_OK) == 0)
      {
      file = SafeOpenRead (filename);
      SafeRead (file, &Scores, sizeof (Scores));
      close(file);
      }
   else
      gamestate.violence = 0;
}

#endif

//******************************************************************************
//
// ReadInt
//
//******************************************************************************

void ReadInt (const char * s1, int * val)
{
   GetToken (true);
   if (!strcmpi (token,s1))
      {
      if (TokenAvailable()==true)
         {
         GetToken(false);
         *val=ParseNum(token);
         }
      }
}

//******************************************************************************
//
// ReadBoolean
//
//******************************************************************************

void ReadBoolean (const char * s1, boolean * val)
{
   int temp;

   temp = (int)(*val);
   ReadInt (s1,&temp);
   *val = (boolean) temp;
}

//******************************************************************************
//
// ReadUnsigned
//
//******************************************************************************

void ReadUnsigned (const char * s1, unsigned long * val)
{
   int temp;

   temp = (int)(*val);
   ReadInt (s1,&temp);
   *val = (unsigned) temp;
}

//******************************************************************************
//
// ParseSoundFile ()
//
//******************************************************************************

boolean ParseSoundFile (void)
{
   boolean retval = true;
   int version    = 0;

   ReadInt("Version",&version);

   if (version == ROTTVERSION)
   {
      // Read in Music Mode

      ReadInt ("MusicMode",&MusicMode);

      // Read in FX Mode

      ReadInt ("FXMode",&FXMode);

      // Read in Music Volume

      ReadInt ("MusicVolume", &MUvolume);

      // Read in FX Volume

      ReadInt ("FXVolume", &FXvolume);

      // Read in numvoices

      ReadInt ("NumVoices",&NumVoices);

      // Read in numchannels

      ReadInt ("NumChannels",&NumChannels);

      // Read in numbits

      ReadInt ("NumBits",&NumBits);

#ifdef DOS
      // Read in Midi Address

      ReadInt ("MidiAddress",&MidiAddress);
#endif

      // Read in stereo reversal

      ReadBoolean ("StereoReverse",&stereoreversed);

#ifdef DOS
      // Read in Sound Blaster info
      ReadUnsigned ("SBType",  &SBSettings.Type );
      ReadUnsigned ("SBPort",  &SBSettings.Address );
      ReadUnsigned ("SBIrq",   &SBSettings.Interrupt );
      ReadUnsigned ("SBDma8",  &SBSettings.Dma8 );
      ReadUnsigned ("SBDma16", &SBSettings.Dma16 );
      ReadUnsigned ("SBMidi",  &SBSettings.Midi );
      ReadUnsigned ("SBEmu",   &SBSettings.Emu );
#endif
   }
   else
      retval = false;

   return (retval);
}



//******************************************************************************
//
// SetSoundDefaultValues ()
//
//******************************************************************************
void SetSoundDefaultValues
   (
   void
   )

   {
#ifdef DOS
   fx_blaster_config blaster;
#endif
   int status;

   //
   //  no config file, so select default values
   //
   #if !defined(PLATFORM_DOS)
   // icculus' SDL_mixer driver looks like a soundscape to us
   MusicMode   = 6;
   FXMode      = 6;
   NumVoices   = 8;
   NumChannels = 1;
   NumBits     = 8;
   stereoreversed = false;

   #else
   MusicMode   = 0;
   FXMode      = 0;
   NumVoices   = 4;
   NumChannels = 1;
   NumBits     = 8;
   MidiAddress = 0x330;
   stereoreversed = false;

   status = FX_GetBlasterSettings( &blaster );
   if ( status == FX_Ok )
      {
      SBSettings.Type      = blaster.Type;
      SBSettings.Address   = blaster.Address;
      SBSettings.Interrupt = blaster.Interrupt;
      SBSettings.Dma8      = blaster.Dma8;
      SBSettings.Dma16     = blaster.Dma16;
      SBSettings.Midi      = blaster.Midi;
      SBSettings.Emu       = blaster.Emu;
      }
   #endif
   }


#ifdef _ROTT_

extern char    pword[ 13 ];
//******************************************************************************
//
// ConvertStringToPasswordString ()
//
//******************************************************************************

#define PASSWORDENCRYPTER "7d7e4a2d3b6a0319554654231f6d2a"

void ConvertStringToPasswordString ( char * string )
{
   int i;
   unsigned int j;
   char temp[3];

   memset(temp,0,sizeof(temp));

   for (i=0;i<13;i++)
      {
      memcpy(&temp[0],&string[i<<1],2);
      sscanf(&temp[0],"%x",&j);
      passwordstring[i+0] = j & 0xff; j >>= 8;
      passwordstring[i+1] = j & 0xff; j >>= 8;
      passwordstring[i+2] = j & 0xff; j >>= 8;
      passwordstring[i+3] = j & 0xff;
      }
}

//******************************************************************************
//
// ConvertPasswordStringToPassword ()
//
//******************************************************************************

void ConvertPasswordStringToPassword ( void )
{
   int i;
   int x;
   char temp[3];
   char key[40];

   memset(temp,0,sizeof(temp));
   strcpy(&key[0],PASSWORDENCRYPTER);

   for (i=0;i<12;i++)
      {
      memcpy(&temp[0],&key[i<<1],2);
      sscanf(&temp[0],"%x",&x);
      pword[i]=passwordstring[i]^x;
      }
   memcpy(&temp[0],&key[i<<1],2);
   sscanf(&temp[0],"%x",&x);
   gamestate.violence=passwordstring[i]^x;
   if (
       (gamestate.violence<0) ||
       (gamestate.violence>3)
      )
      gamestate.violence=0;
}

//******************************************************************************
//
// ConvertPasswordStringToString ()
//
//******************************************************************************

void ConvertPasswordStringToString ( char * string )
{
   int i;
   char temp[8];

   memset(temp,0,sizeof(temp));

   for (i=0;i<13;i++)
      {
      itoa((passwordstring[i]>>4),&temp[0],16);
      string[(i<<1)+0]=temp[0];
      itoa((passwordstring[i]&0xf),&temp[0],16);
      string[(i<<1)+1]=temp[0];
      }
}

//******************************************************************************
//
// ConvertPasswordToPasswordString ()
//
//******************************************************************************

void ConvertPasswordToPasswordString ( void )
{
   int i;
   int x;
   char temp[3];
   char key[40];

   memset(temp,0,sizeof(temp));
   strcpy(&key[0],PASSWORDENCRYPTER);

   for (i=0;i<12;i++)
      {
      memcpy(&temp[0],&key[i<<1],2);
      sscanf(&temp[0],"%x",&x);
      passwordstring[i]=pword[i]^x;
      }
   memcpy(&temp[0],&key[i<<1],2);
   sscanf(&temp[0],"%x",&x);
   passwordstring[i]=gamestate.violence^x;
}

//******************************************************************************
//
// ParseConfigFile ()
//
//******************************************************************************

boolean ParseConfigFile (void)
{
//   int temp;
   boolean retval = true;
   int version    = 0;

   ReadInt("Version",&version);

   if (version == ROTTVERSION)
   {
      // Read in MouseEnabled
      ReadBoolean("MouseEnabled",&mouseenabled);

      // Read in UseMouseLook
      ReadBoolean("UseMouseLook",&usemouselook);

      ReadInt("InverseMouse",&inverse_mouse);

	  // Read in UseJump
      ReadBoolean("UseJump",&usejump);

      // Read in CrossHair
      ReadBoolean("CrossHair",&iG_aimCross);

      // Read in JoystickEnabled
      ReadBoolean("JoystickEnabled",&joystickenabled);

      // Read in JoypadEnabled
      ReadBoolean("JoypadEnabled",&joypadenabled);

      // Read in JoystickPort

      ReadInt("JoystickPort",&joystickport);
      
      // Read in fullscreen
      ReadBoolean("FullScreen", &sdl_fullscreen);
      
      // Read in resolution
#ifndef RETRO_GO
      ReadInt("ScreenWidth", &iGLOBAL_SCREENWIDTH);
      ReadInt("ScreenHeight", &iGLOBAL_SCREENHEIGHT);
#endif

      // Read in ViewSize

      ReadInt("ViewSize",&viewsize);

      // Read in Weaponscale

      ReadInt("Weaponscale",&G_weaponscale);//bna added
	   if ((G_weaponscale <150)||(G_weaponscale>600)){
		   if (iGLOBAL_SCREENWIDTH == 320){
				G_weaponscale=168;
		   }else if (iGLOBAL_SCREENWIDTH == 640){
				G_weaponscale=299;		
		   }else if (iGLOBAL_SCREENWIDTH == 800) {
				G_weaponscale=376;
		   }	   
	   }

      // Read in MouseAdjustment

      ReadInt("MouseAdjustment",&mouseadjustment);

      // Read in threshold

      ReadInt("Threshold",&threshold);

      // Read in Auto Detail

      ReadBoolean ("AutoDetail", &AutoDetailOn);

      // Read in Light Dim

      ReadInt ("LightDim", &fulllight);

      // Read in Bobbin' On

      ReadBoolean ("BobbingOn", &BobbinOn);

      // Read in Double Click Speed

      ReadInt ("DoubleClickSpeed", &DoubleClickSpeed);

      // Read in Menu Flip Speed

      ReadInt ("MenuFlipSpeed", &Menuflipspeed);

      // Read in Detail Level

      ReadInt ("DetailLevel", &DetailLevel);

      // Read in Floor and Ceiling

      ReadInt ("FloorCeiling", &fandc);

      // Read in MessagesEnabled

      ReadBoolean ("Messages", &MessagesEnabled );

      // Read in Autorun

      ReadInt ("AutoRun", &gamestate.autorun );

      // Read in GammaIndex

      ReadInt ("GammaIndex", &gammaindex);

      // Read screen blanking time

      ReadInt ("BlankTime", &blanktime);

      blanktime=blanktime*60*VBLCOUNTER;

      // Read keys

      ReadInt ("Fire",        &buttonscan[0]);
      ReadInt ("Strafe",      &buttonscan[1]);
      ReadInt ("Run",         &buttonscan[2]);
      ReadInt ("Use",         &buttonscan[3]);
      ReadInt ("LookUp",      &buttonscan[4]);
      ReadInt ("LookDn",      &buttonscan[5]);
      ReadInt ("Swap",        &buttonscan[6]);
      ReadInt ("Drop",        &buttonscan[7]);
      ReadInt ("TargetUp",    &buttonscan[8]);
      ReadInt ("TargetDn",    &buttonscan[9]);
      ReadInt ("SelPistol",   &buttonscan[10]);
      ReadInt ("SelDualPistol",&buttonscan[11]);
      ReadInt ("SelMP40",     &buttonscan[12]);
      ReadInt ("SelMissile",  &buttonscan[13]);
      ReadInt ("AutoRun",     &buttonscan[14]);
      ReadInt ("LiveRemRid",  &buttonscan[15]);
      ReadInt ("StrafeLeft",  &buttonscan[16]);
      ReadInt ("StrafeRight", &buttonscan[17]);
      ReadInt ("VolteFace",   &buttonscan[18]);
      ReadInt ("Aim",         &buttonscan[19]);
      ReadInt ("Forward",     &buttonscan[20]);
      ReadInt ("Right",       &buttonscan[21]);
      ReadInt ("Backward",    &buttonscan[22]);
      ReadInt ("Left",        &buttonscan[23]);
      ReadInt ("Map",         &buttonscan[24]);
      ReadInt ("SendMessage", &buttonscan[25]);
      ReadInt ("DirectMessage",&buttonscan[26]);

      ReadInt ("MouseButton0",&buttonmouse[0]);
      ReadInt ("MouseButton1",&buttonmouse[1]);
      ReadInt ("MouseButton2",&buttonmouse[2]);
      ReadInt ("DblClickB0",  &buttonmouse[3]);
      ReadInt ("DblClickB1",  &buttonmouse[4]);
      ReadInt ("DblClickB2",  &buttonmouse[5]);

      ReadInt ("JoyButton0",  &buttonjoy[0]);
      ReadInt ("JoyButton1",  &buttonjoy[1]);
      ReadInt ("JoyButton2",  &buttonjoy[2]);
      ReadInt ("JoyButton3",  &buttonjoy[3]);
      ReadInt ("DblClickJB0", &buttonjoy[4]);
      ReadInt ("DblClickJB1", &buttonjoy[5]);
      ReadInt ("DblClickJB2", &buttonjoy[6]);
      ReadInt ("DblClickJB3", &buttonjoy[7]);

      ReadInt ("JoyMaxX",     &joyxmax);
      ReadInt ("JoyMaxY",     &joyymax);
      ReadInt ("JoyMinX",     &joyxmin);
      ReadInt ("JoyMinY",     &joyymin);

      ReadInt( "DefaultDifficulty", &DefaultDifficulty );
      ReadInt( "DefaultPlayerCharacter", &DefaultPlayerCharacter );
      ReadInt( "DefaultPlayerColor", &DefaultPlayerColor );

      // Get Password string
      GetToken (true);
      if (!stricmp (token, "SecretPassword"))
         {
         GetTokenEOL (false);
         ConvertStringToPasswordString ( &name[0] );
         }

      if (!CybermanPresent)
         cybermanenabled = false;

      if (!AssassinPresent)
         assassinenabled = false;

      if (!SpaceBallPresent)
         spaceballenabled = false;

      if (!MousePresent)
         mouseenabled = false;

      if (!JoysPresent[joystickport])
         joystickenabled = false;

      // precaution

      if (!joyxmin || !joyxmax || !joyymin || !joyymax)
         joystickenabled = false;

      if (joystickenabled)
         IN_SetupJoy (joystickport, joyxmin, joyxmax, joyymin, joyymax);
   }
   else
      retval = false;

   return (retval);
}


//******************************************************************************
//
// ParseBattleFile ()
//
//******************************************************************************
boolean ParseBattleFile (void)
{
   boolean retval = true;
   int version    = 0;
   int index;
   int temp;
   extern specials BattleSpecialsTimes;

   ReadInt("Version",&version);
   if (version != ROTTVERSION)
      retval = false;
   else
      {
      ReadBoolean( "ShowKillCount", &BATTLE_ShowKillCount );

      ReadInt( "GodModeTime",                &BattleSpecialsTimes.GodModeTime );
      ReadInt( "DogModeTime",                &BattleSpecialsTimes.DogModeTime );
      ReadInt( "ShroomsModeTime",            &BattleSpecialsTimes.ShroomsModeTime );
      ReadInt( "ElastoModeTime",             &BattleSpecialsTimes.ElastoModeTime );
      ReadInt( "AsbestosVestTime",           &BattleSpecialsTimes.AsbestosVestTime );
      ReadInt( "BulletProofVestTime",        &BattleSpecialsTimes.BulletProofVestTime );
      ReadInt( "GasMaskTime",                &BattleSpecialsTimes.GasMaskTime );
      ReadInt( "MercuryModeTime",            &BattleSpecialsTimes.MercuryModeTime );
      ReadInt( "GodModeRespawnTime",         &BattleSpecialsTimes.GodModeRespawnTime );
      ReadInt( "DogModeRespawnTime",         &BattleSpecialsTimes.DogModeRespawnTime );
      ReadInt( "ShroomsModeRespawnTime",     &BattleSpecialsTimes.ShroomsModeRespawnTime );
      ReadInt( "ElastoModeRespawnTime",      &BattleSpecialsTimes.ElastoModeRespawnTime );
      ReadInt( "AsbestosVestRespawnTime",    &BattleSpecialsTimes.AsbestosVestRespawnTime );
      ReadInt( "BulletProofVestRespawnTime", &BattleSpecialsTimes.BulletProofVestRespawnTime );
      ReadInt( "GasMaskRespawnTime",         &BattleSpecialsTimes.GasMaskRespawnTime );
      ReadInt( "MercuryModeRespawnTime",     &BattleSpecialsTimes.MercuryModeRespawnTime );

      ReadBoolean( "EKG", &battlegibs );

      for( index = battle_Normal; index < battle_NumBattleModes; index++ )
         {
         // Read Gravity
         temp = BATTLE_Options[ index ].Gravity;
         ReadInt( "Gravity", &temp );
			BATTLE_Options[ index ].Gravity = temp;

         // Read Speed
         temp = bo_normal_speed;
         ReadInt( "Speed", &temp );
         if ( ( temp >= bo_normal_speed ) &&
            ( temp <= bo_fast_speed ) )
            {
            BATTLE_Options[ index ].Speed = temp;
            }

         if ( ( index != battle_Collector ) && ( index != battle_Tag ) &&
            ( index != battle_Eluder ) )
            {
            // Read Ammo
            temp = bo_normal_shots;
            BATTLE_Options[ index ].Ammo = bo_normal_shots;
            ReadInt( "Ammo", &temp );
            if ( ( temp >= bo_one_shot ) &&
               ( temp <= bo_infinite_shots ) )
               {
               BATTLE_Options[ index ].Ammo = temp;
               }
            }

         if ( index != battle_Eluder )
            {
            // Read Hitpoints
            temp = BATTLE_Options[ index ].HitPoints;
            ReadInt( "Hitpoints", &temp );
            BATTLE_Options[ index ].HitPoints = temp;
            }

         // Read Spawn Dangers
         temp = 1;
         ReadInt( "SpawnDangers", &temp );
         BATTLE_Options[ index ].SpawnDangers = temp;

         if ( index != battle_Eluder )
            {
            // Read Spawn Health
            temp = 1;
            ReadInt( "SpawnHealth", &temp );
            BATTLE_Options[ index ].SpawnHealth = temp;

            // Read Spawn Mines
            temp = 0;
            ReadInt( "SpawnMines", &temp );
            BATTLE_Options[ index ].SpawnMines = temp;
            }

         if ( ( index != battle_Collector ) && ( index != battle_Tag ) &&
            ( index != battle_Eluder ) )
            {
            // Read Spawn Weapons
            temp = 1;
            ReadInt( "SpawnWeapons", &temp );
            BATTLE_Options[ index ].SpawnWeapons = temp;

            // Read Random Weapons
            temp = 0;
            ReadInt( "RandomWeapons", &temp );
            BATTLE_Options[ index ].RandomWeapons = temp;

            // Read Weapon Persistence
            temp = 0;
            ReadInt( "WeaponPersistence", &temp );
            BATTLE_Options[ index ].WeaponPersistence = temp;
            }

         if ( ( index == battle_Normal ) || ( index == battle_ScoreMore ) ||
            ( index == battle_Hunter ) || ( index == battle_Tag ) )
            {
            // Read Friendly Fire
            temp = 1;
            ReadInt( "FriendlyFire", &temp );
            BATTLE_Options[ index ].FriendlyFire = temp;
            }

         if ( index != battle_Eluder )
            {
            // Read Respawn Items
            temp = 1;
            ReadInt( "RespawnItems", &temp );
            BATTLE_Options[ index ].RespawnItems = temp;
            }

         // Read Light Level
         temp = bo_light_normal;
         ReadInt( "LightLevel", &temp );
         if ( ( temp >= bo_light_dark ) &&
            ( temp <= bo_light_lightning ) )
            {
            BATTLE_Options[ index ].LightLevel = temp;
            }

         if ( ( index != battle_Collector ) && ( index != battle_Scavenger ) )
            {
            // Read Point Goal
            temp = bo_kills_default;
            ReadInt( "PointGoal", &temp );
            BATTLE_Options[ index ].Kills = temp;
            if ( temp < bo_kills_random )
               {
               BATTLE_Options[ index ].Kills = bo_kills_default;
               }
            }

         if ( index != battle_Eluder )
            {
            // Read Danger Damage
            temp = bo_danger_normal;
            ReadInt( "DangerDamage", &temp );
            BATTLE_Options[ index ].DangerDamage = temp;
            }

         // Read Time Limit
         temp = bo_time_infinite;
         ReadInt( "TimeLimit", &temp );
         if ( ( index == battle_Hunter ) && ( temp == bo_time_infinite ) )
            {
            temp = 99;
            }
         BATTLE_Options[ index ].TimeLimit = temp;

         // Read Respawn time
         temp = bo_normal_respawn_time;
         ReadInt( "RespawnTime", &temp );
         BATTLE_Options[ index ].RespawnTime = temp;
         }
      }

   return (retval);
   }

//******************************************************************************
//
// SetBattleDefaultValues ()
//
//******************************************************************************

void SetBattleDefaultValues (void)
{
   int index;

   //
   //  no config file, so select default values
   //
   for( index = battle_StandAloneGame; index < battle_NumBattleModes;
      index++ )
      {
      BATTLE_Options[ index ].Gravity      = NORMAL_GRAVITY;
      BATTLE_Options[ index ].Speed        = bo_normal_speed;
      BATTLE_Options[ index ].Ammo         = bo_normal_shots;
      BATTLE_Options[ index ].HitPoints    = bo_default_hitpoints;
      BATTLE_Options[ index ].SpawnDangers = 1;
      BATTLE_Options[ index ].SpawnHealth  = 1;
      BATTLE_Options[ index ].SpawnMines   = 0;
      BATTLE_Options[ index ].SpawnWeapons = 1;
      BATTLE_Options[ index ].RespawnItems = 1;
      BATTLE_Options[ index ].RandomWeapons = 0;
      BATTLE_Options[ index ].WeaponPersistence = 0;
      BATTLE_Options[ index ].FriendlyFire = 1;
      BATTLE_Options[ index ].LightLevel   = bo_light_normal;
      BATTLE_Options[ index ].Kills        = bo_kills_default;
      BATTLE_Options[ index ].DangerDamage = bo_danger_normal;
      BATTLE_Options[ index ].TimeLimit    = bo_time_infinite;
      BATTLE_Options[ index ].RespawnTime  = bo_normal_respawn_time;
      }

   BATTLE_Options[ battle_CaptureTheTriad ].Kills  = 1;
   BATTLE_Options[ battle_Hunter ].TimeLimit       = 1;
   BATTLE_Options[ battle_Eluder ].SpawnHealth     = 0;
   BATTLE_Options[ battle_Eluder ].RespawnItems    = 0;
   BATTLE_Options[ battle_Eluder ].SpawnWeapons    = 0;
   BATTLE_Options[ battle_Eluder ].FriendlyFire    = 0;
   BATTLE_Options[ battle_Collector ].SpawnWeapons = 0;
   BATTLE_Options[ battle_Collector ].FriendlyFire = 0;
   BATTLE_Options[ battle_Tag ].SpawnWeapons       = 0;
   battlegibs=false;
   BATTLE_ShowKillCount = true;
   }

//******************************************************************************
//
// SetConfigDefaultValues ()
//
//******************************************************************************

void SetConfigDefaultValues (void)
{
   //
   //  no config file, so select default values
   //
   if (MousePresent)
      mouseenabled = true;

   joystickenabled = false;
   joypadenabled   = false;
   joystickport    = 0;
   viewsize        = 7;
   mouseadjustment = 5;
   gammaindex      = 0;
   gamestate.violence = 3;
   passwordstring[0]=0x7d;
   passwordstring[1]=0x7e;
   passwordstring[2]=0x4a;
   passwordstring[3]=0x2d;
   passwordstring[4]=0x3b;
   passwordstring[5]=0x6a;
   passwordstring[6]=0x03;
   passwordstring[7]=0x19;
   passwordstring[8]=0x55;
   passwordstring[9]=0x46;
   passwordstring[10]=0x54;
   passwordstring[11]=0x23;
   passwordstring[12]=0x1c;
}
#endif

//******************************************************************************
//
// DeleteSoundFile ()
//
//******************************************************************************
void DeleteSoundFile ( void )
{
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, SoundName );
   unlink (filename);          // Delete SOUND.ROT
}

//******************************************************************************
//
// ReadConfig ()
//
//******************************************************************************


void ReadConfig (void)
{
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, SoundName );
   SetSoundDefaultValues ();

   if (access (filename, F_OK) == 0)
      {
      LoadScriptFile (filename);

      if (ParseSoundFile () == false)
         {
         DeleteSoundFile();
         }

      Z_Free (scriptbuffer);
      }
#ifdef DOS
   else if ( !SOUNDSETUP )
      {
      Error( "Could not find SOUND.ROT.  Please run SNDSETUP to configure "
         "your sound hardware." );
      }
#endif


#ifdef _ROTT_
   ReadScores();

   GetPathFromEnvironment( filename, ApogeePath, ConfigName );
   SetConfigDefaultValues ();
   if (access(filename,F_OK)==0)
      {
      LoadScriptFile(filename);

      if (ParseConfigFile () == false)
         {
         unlink (filename);          // Delete CONFIG.ROT
         }

      Z_Free(scriptbuffer);
      }

   GetPathFromEnvironment( filename, ApogeePath, BattleName );
   SetBattleDefaultValues ();
   if (access(filename,F_OK)==0)
      {
      LoadScriptFile(filename);

      if (ParseBattleFile() == false)
         {
         unlink (filename);          // Delete BATTLE.ROT
         }

      Z_Free(scriptbuffer);
      }
#endif
   ConfigLoaded = true;
}

//******************************************************************************
//
// CheckVendor ()
//
//******************************************************************************

#if (SHAREWARE==1)
#define VENDORDOC ("VENDOR.DOC")
#define VENDORLUMP ("VENDOR")
#else
#define VENDORDOC ("LICENSE.DOC")
#define VENDORLUMP ("LICENSE")
#endif

void CheckVendor (void)
{
   return;
   boolean saveout=false;
   int wadcrc;
   int filecrc;
   int size;
   int lump;
   byte * vendor;
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, VENDORDOC );
   if (access (filename, F_OK) == 0)
      {
      size = LoadFile(filename,(void **)&vendor);
      filecrc = CalculateCRC (vendor, size);
      SafeFree(vendor);
      lump=W_GetNumForName(VENDORLUMP);
      vendor = W_CacheLumpNum(lump,PU_CACHE, CvtNull, 1);
      size=W_LumpLength(lump);
      wadcrc = CalculateCRC (vendor, size);
      if (wadcrc != filecrc)
         saveout=true;
      }
   else
      saveout=true;

   if (saveout==true)
      {
      lump=W_GetNumForName(VENDORLUMP);
      vendor = W_CacheLumpNum(lump,PU_CACHE, CvtNull, 1);
      size = W_LumpLength(lump);
      SaveFile (filename,vendor,size);
      }
}

//******************************************************************************
//
// WriteParameter
//
//******************************************************************************

void WriteParameter (int file, const char * s1, int val)
{
   char s[50];

   // Write out Header
   SafeWriteString (file, (char *)s1);

   // Write out space character
   strcpy (&s[0],(const char *)"  ");
   SafeWriteString (file, &s[0]);

   // Write out value
   itoa(val,&s[0],10);
   SafeWriteString (file, &s[0]);

   // Write out EOL character
   strcpy (&s[0],(const char *)"\n");
   SafeWriteString (file, &s[0]);
}


//******************************************************************************
//
// WriteParameterHex
//
//******************************************************************************

void WriteParameterHex (int file, const char * s1, int val)
{
   char s[50];

   // Write out Header
   SafeWriteString (file, (char *)s1);

   // Write out space character
   strcpy (&s[0],(const char *)"  $");
   SafeWriteString (file, &s[0]);

   // Write out value
   itoa(val,&s[0],16);
   SafeWriteString (file, &s[0]);

   // Write out EOL character
   strcpy (&s[0],(const char *)"\n");
   SafeWriteString (file, &s[0]);
}



#ifdef _ROTT_

//******************************************************************************
//
// WriteScores ()
//
//******************************************************************************

void WriteScores (void)
{
   int file;
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, ScoresName );
   file=SafeOpenWrite( filename );
   SafeWrite (file, &Scores, sizeof (Scores));
   close(file);
}


//******************************************************************************
//
// WriteBattleConfig ()
//
//******************************************************************************

void WriteBattleConfig
   (
   void
   )

   {
   int  file;
   int  index;
   char filename[ 128 ];
   extern specials BattleSpecialsTimes;

   // Write Battle File
   GetPathFromEnvironment( filename, ApogeePath, BattleName );
   file = open( filename, O_RDWR | O_TEXT | O_CREAT | O_TRUNC,
      S_IREAD | S_IWRITE );

   if ( file == -1 )
      {
      Error( "Error opening %s: %s", filename, strerror( errno ) );
      }

   // Write out BATTLECONFIG header
   SafeWriteString( file,
      ";Rise of the Triad Battle Configuration File\n"
      ";                  (c) 1995\n"
      ";\n"
      ";You may change these options at you own risk.  Using any values\n"
      ";other than the ones documented may make the game unplayable.\n"
      ";If this happens, you may delete this file (BATTLE.ROT) and ROTT\n"
      ";will recreate it with the default values selected.\n"
      ";\n"
      ";With that in mind, have fun!\n"
      ";\n"
      "\n" );

   // Write out Version
   WriteParameter( file, "Version                        ", ROTTVERSION );

   // Write out BATTLE_ShowKillPics
   SafeWriteString(file, "\n;\n");
	WriteParameter( file, "; Yes                        - ", 1 );
   WriteParameter( file, "; No                         - ", 0 );
   WriteParameter( file, "ShowKillCount                  ", BATTLE_ShowKillCount );

   // Write out specials' times
   SafeWriteString(file, "\n;\n"
      "; These are the time in seconds of the various powerups.\n"
      "; You could modify these to give you infinite Mercury mode,\n"
      "; stronger vests, or to make them persistent.\n;\n" );

   WriteParameter( file, "GodModeTime                ", BattleSpecialsTimes.GodModeTime );
   WriteParameter( file, "DogModeTime                ", BattleSpecialsTimes.DogModeTime );
   WriteParameter( file, "ShroomsModeTime            ", BattleSpecialsTimes.ShroomsModeTime );
   WriteParameter( file, "ElastoModeTime             ", BattleSpecialsTimes.ElastoModeTime );
   WriteParameter( file, "AsbestosVestTime           ", BattleSpecialsTimes.AsbestosVestTime );
   WriteParameter( file, "BulletProofVestTime        ", BattleSpecialsTimes.BulletProofVestTime );
   WriteParameter( file, "GasMaskTime                ", BattleSpecialsTimes.GasMaskTime );
   WriteParameter( file, "MercuryModeTime            ", BattleSpecialsTimes.MercuryModeTime );
   WriteParameter( file, "GodModeRespawnTime         ", BattleSpecialsTimes.GodModeRespawnTime );
   WriteParameter( file, "DogModeRespawnTime         ", BattleSpecialsTimes.DogModeRespawnTime );
   WriteParameter( file, "ShroomsModeRespawnTime     ", BattleSpecialsTimes.ShroomsModeRespawnTime );
   WriteParameter( file, "ElastoModeRespawnTime      ", BattleSpecialsTimes.ElastoModeRespawnTime );
   WriteParameter( file, "AsbestosVestRespawnTime    ", BattleSpecialsTimes.AsbestosVestRespawnTime );
   WriteParameter( file, "BulletProofVestRespawnTime ", BattleSpecialsTimes.BulletProofVestRespawnTime );
   WriteParameter( file, "GasMaskRespawnTime         ", BattleSpecialsTimes.GasMaskRespawnTime );
   WriteParameter( file, "MercuryModeRespawnTime     ", BattleSpecialsTimes.MercuryModeRespawnTime );

   // Write out battlegibs
   SafeWriteString(file, "\n;\n");
	WriteParameter( file, "; Yes                        - ", 1 );
   WriteParameter( file, "; No                         - ", 0 );
   WriteParameter( file, "EKG                            ", battlegibs );

   // Describe options

   // Write out Gravity
   SafeWriteString(file, "\n"
                         ";\n"
                         "; Here is a description of the possible values for"
                         " each option:\n"
                         ";\n"
                         "; Gravity options:\n" );
   WriteParameter( file, ";    Low Gravity             - ", LOW_GRAVITY );
   WriteParameter( file, ";    Normal Gravity          - ", NORMAL_GRAVITY );
   WriteParameter( file, ";    High Gravity            - ", HIGH_GRAVITY );

   // Write out Speed
   SafeWriteString(file, ";\n"
                         "; Speed options:\n" );
   WriteParameter( file, ";    Normal Speed            - ", bo_normal_speed );
   WriteParameter( file, ";    Fast Speed              - ", bo_fast_speed );

   // Write out Ammo
   SafeWriteString(file, ";\n"
                         "; Ammo options:\n" );
   WriteParameter( file, ";    One Shot                - ", bo_one_shot );
   WriteParameter( file, ";    Normal Shots            - ", bo_normal_shots );
   WriteParameter( file, ";    Infinite Shots          - ", bo_infinite_shots );

   // Write out Hit Points
   SafeWriteString(file, ";\n"
                         "; Hitpoint options:\n" );
   WriteParameter( file, ";    Character Hitpoints     - ", bo_character_hitpoints );
   WriteParameter( file, ";       1 Hitpoint           - ", 1 );
   WriteParameter( file, ";      25 Hitpoints          - ", 25 );
   WriteParameter( file, ";     100 Hitpoints          - ", 100 );
   WriteParameter( file, ";     500 Hitpoints          - ", 500 );
   WriteParameter( file, ";     250 Hitpoints          - ", 250 );
   WriteParameter( file, ";    4000 Hitpoints          - ", 4000 );

   // Write out Danger Spawning
   SafeWriteString(file, ";\n"
                         "; SpawnDangers options:\n"
                         ";    Spawn Dangers           -   1\n"
                         ";    Don't Spawn Dangers     -   0\n" );

   // Write out Health Spawning
   SafeWriteString(file, ";\n"
                         "; SpawnHealth options:\n"
                         ";    Spawn Health            -   1\n"
                         ";    Don't Spawn Health      -   0\n" );

   // Write out Mine Spawning
   SafeWriteString(file, ";\n"
                         "; SpawnMines options:\n"
                         ";    Spawn Mines             -   1\n"
                         ";    Don't Spawn Mines       -   0\n" );

   // Write out Weapon Spawning
   SafeWriteString(file, ";\n"
                         "; SpawnWeapons options:\n"
                         ";    Spawn Weapons           -   1\n"
                         ";    Don't Spawn Weapons     -   0\n" );

   // Write out Random Weapons
   SafeWriteString(file, ";\n"
                         "; RandomWeapons options:\n"
                         ";    Randomize Weapons       -   1\n"
                         ";    Don't Randomize Weapons -   0\n" );

   // Write out Weapon Persistence
   SafeWriteString(file, ";\n"
                         "; WeaponPersistence options:\n"
                         ";    Weapons Persist         -   1\n"
                         ";    Weapons don't Persist   -   0\n" );

   // Write out Friendly Fire
   SafeWriteString(file, ";\n"
                         "; FriendlyFire options:\n"
                         ";    Penalize Friendly Fire  -   1\n"
                         ";    No penalty              -   0\n" );

   // Write out Respawn Items
   SafeWriteString(file, ";\n"
                         "; RespawnItems options:\n"
                         ";    Respawn Items           -   1\n"
                         ";    Don't Respawn Items     -   0\n" );

   // Write out Light Level
   SafeWriteString(file, ";\n"
                         "; LightLevel options:\n" );
   WriteParameter( file, ";    Dark                    - ", bo_light_dark );
   WriteParameter( file, ";    Normal Light Levels     - ", bo_light_normal );
   WriteParameter( file, ";    Bright                  - ", bo_light_bright );
   WriteParameter( file, ";    Fog                     - ", bo_light_fog );
   WriteParameter( file, ";    Periodic light          - ", bo_light_periodic );
   WriteParameter( file, ";    Lightning               - ", bo_light_lightning );

   // Write out Point Goal
   SafeWriteString(file, ";\n"
                         "; PointGoal options:\n" );
   WriteParameter( file, ";           1 Point          - ", 1 );
   WriteParameter( file, ";           5 Points         - ", 5 );
   WriteParameter( file, ";          11 Points         - ", 11 );
   WriteParameter( file, ";          21 Points         - ", 21 );
   WriteParameter( file, ";          50 Points         - ", 50 );
   WriteParameter( file, ";         100 Points         - ", 100 );
   WriteParameter( file, ";      Random Points         - ", bo_kills_random );
   WriteParameter( file, ";       Blind Points         - ", bo_kills_blind );
   WriteParameter( file, ";    Infinite Points         - ", bo_kills_infinite );

   // Write out Danger Damage
   SafeWriteString(file, ";\n"
                         "; DangerDamage options:\n" );
   WriteParameter( file, ";    Normal Damage           - ", bo_danger_normal );
   WriteParameter( file, ";    Low Damage              - ", bo_danger_low );
   WriteParameter( file, ";    Kill                    - ", bo_danger_kill );

   // Write out TimeLimit
   SafeWriteString(file, ";\n"
                         "; TimeLimit options:\n" );
   WriteParameter( file, ";     1 minute               - ", 1 );
   WriteParameter( file, ";     2 minute               - ", 2 );
   WriteParameter( file, ";     5 minutes              - ", 5 );
   WriteParameter( file, ";    10 minutes              - ", 10 );
   WriteParameter( file, ";    21 minutes              - ", 21 );
   WriteParameter( file, ";    30 minutes              - ", 30 );
   WriteParameter( file, ";    99 minutes              - ", 99 );
   WriteParameter( file, ";    No limit                - ", bo_time_infinite );

   // Write out RespawnTime
   SafeWriteString(file, ";\n"
                         "; RespawnTime options:\n" );
   WriteParameter( file, ";     1 second               - ", 1 );
   WriteParameter( file, ";     1 minute               - ", 60 );
   WriteParameter( file, ";     2 minutes              - ", 120 );
   WriteParameter( file, ";       normal               - ", bo_normal_respawn_time );

   for( index = battle_Normal; index < battle_NumBattleModes; index++ )
      {
      SafeWriteString(file, "\n;\n");
      switch( index )
         {
         case battle_Normal :
            SafeWriteString( file, "; Standard battle options\n;\n" );
            break;

         case battle_ScoreMore :
            SafeWriteString( file, "; Score More battle options\n;\n" );
            break;

         case battle_Collector :
            SafeWriteString( file, "; Collector battle options\n;\n" );
            break;

         case battle_Scavenger :
            SafeWriteString( file, "; Scavenger battle options\n;\n" );
            break;

         case battle_Hunter :
            SafeWriteString( file, "; Hunter battle options\n;\n" );
            break;

         case battle_Tag :
            SafeWriteString( file, "; Tag battle options\n;\n" );
            break;

         case battle_Eluder :
            SafeWriteString( file, "; Eluder battle options\n;\n" );
            break;

         case battle_Deluder :
            SafeWriteString( file, "; Deluder battle options\n;\n" );
            break;

			case battle_CaptureTheTriad :
				SafeWriteString( file, "; Capture the Triad battle options\n;\n" );
				break;
			}

		// Write out Gravity
      WriteParameter( file, "Gravity          ",
         BATTLE_Options[ index ].Gravity );

      // Write out Speed
      WriteParameter( file, "Speed            ",
         BATTLE_Options[ index ].Speed );

      if ( ( index != battle_Collector ) && ( index != battle_Tag ) &&
         ( index != battle_Eluder ) )
         {
         // Write out Ammo
         WriteParameter( file, "Ammo             ",
            BATTLE_Options[ index ].Ammo );
         }

      if ( index != battle_Eluder )
         {
         // Write out Hit Points
         WriteParameter( file, "Hitpoints        ",
            BATTLE_Options[ index ].HitPoints );
         }

      // Write out Danger Spawning
      WriteParameter( file, "SpawnDangers     ",
         BATTLE_Options[ index ].SpawnDangers );

      if ( index != battle_Eluder )
         {
         // Write out Health Spawning
         WriteParameter( file, "SpawnHealth      ",
            BATTLE_Options[ index ].SpawnHealth );

         // Write out Mine Spawning
         WriteParameter( file, "SpawnMines       ",
            BATTLE_Options[ index ].SpawnMines );
         }

      if ( ( index != battle_Collector ) && ( index != battle_Tag ) &&
         ( index != battle_Eluder ) )
         {
         // Write out Weapon Spawning
         WriteParameter( file, "SpawnWeapons     ",
            BATTLE_Options[ index ].SpawnWeapons );

         // Write out Random Weapons
         WriteParameter( file, "RandomWeapons    ",
            BATTLE_Options[ index ].RandomWeapons );

         // Write out Weapon Persistence
         WriteParameter( file, "WeaponPersistence",
            BATTLE_Options[ index ].WeaponPersistence );
         }

      if ( ( index == battle_Normal ) || ( index == battle_ScoreMore ) ||
         ( index == battle_Hunter ) || ( index == battle_Tag ) )
         {
         // Write out Friendly Fire
         WriteParameter( file, "FriendlyFire     ",
            BATTLE_Options[ index ].FriendlyFire );
         }

      if ( index != battle_Eluder )
         {
         // Write out Respawn Items
         WriteParameter( file, "RespawnItems     ",
            BATTLE_Options[ index ].RespawnItems );
         }

      // Write out Light Level
      WriteParameter( file, "LightLevel       ",
         BATTLE_Options[ index ].LightLevel );

      if ( ( index != battle_Collector ) && ( index != battle_Scavenger ) )
         {
         // Write out Point Goal
         WriteParameter( file, "PointGoal        ",
            BATTLE_Options[ index ].Kills );
         }

      if ( index != battle_Eluder )
         {
         // Write out Danger Damage
         WriteParameter( file, "DangerDamage     ",
            BATTLE_Options[ index ].DangerDamage );
         }

      // Write out TimeLimit
      WriteParameter( file, "TimeLimit        ",
         BATTLE_Options[ index ].TimeLimit );

      // Write out RespawnTime
      WriteParameter( file, "RespawnTime      ",
         BATTLE_Options[ index ].RespawnTime );
      }

   close( file );
   }

#endif

//******************************************************************************
//
// WriteSoundConfig ()
//
//******************************************************************************

void WriteSoundConfig
   (
   void
   )

   {
   int file;
   char filename[ 128 ];

   if ( !WriteSoundFile )
      {
      return;
      }

   GetPathFromEnvironment( filename, ApogeePath, SoundName );
   file = open ( filename, O_RDWR | O_TEXT | O_CREAT | O_TRUNC,
      S_IREAD | S_IWRITE);

   if (file == -1)
      Error ("Error opening %s: %s", filename, strerror(errno));

   // Write out ROTTSOUND header

   SafeWriteString (file, ";Rise of the Triad Sound File\n");
   SafeWriteString (file, ";                  (c) 1995\n\n");

   // Write out Version

   WriteParameter(file,"Version          ",ROTTVERSION);

   // Write out Music Mode

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Music Modes\n");
   SafeWriteString(file,"; 0  -  Off\n");
#ifdef DOS
   SafeWriteString(file,"; 1  -  UltraSound\n");
   SafeWriteString(file,"; 2  -  Sound Blaster\n");
   SafeWriteString(file,"; 3  -  Sound Man 16\n");
   SafeWriteString(file,"; 4  -  Pro Audio Spectrum\n");
   SafeWriteString(file,"; 5  -  Awe32\n");
   SafeWriteString(file,"; 6  -  SoundScape\n");
   SafeWriteString(file,"; 7  -  Wave Blaster\n");
   SafeWriteString(file,"; 8  -  General Midi\n");
   SafeWriteString(file,"; 9  -  Sound Canvas\n");
   SafeWriteString(file,"; 10 -  Adlib\n");
#else
   SafeWriteString(file,"; 6  -  On\n");
#endif
   WriteParameter(file,"MusicMode        ",MusicMode);

   // Write out FX Mode

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; FX Modes\n");
   SafeWriteString(file,"; 0  -  Off\n");
#ifdef DOS
   SafeWriteString(file,"; 1  -  UltraSound\n");
   SafeWriteString(file,"; 2  -  Sound Blaster\n");
   SafeWriteString(file,"; 3  -  Sound Man 16\n");
   SafeWriteString(file,"; 4  -  Pro Audio Spectrum\n");
   SafeWriteString(file,"; 5  -  Awe32\n");
   SafeWriteString(file,"; 6  -  SoundScape\n");
   SafeWriteString(file,"; 7  -  Adlib\n");
   SafeWriteString(file,"; 8  -  Disney Sound Source\n");
   SafeWriteString(file,"; 9  -  Tandy Sound Source\n");
   SafeWriteString(file,"; 10 -  PC Speaker\n");
#else
   SafeWriteString(file,"; 6  -  On\n");
#endif
   WriteParameter(file,"FXMode           ",FXMode);

   // Write in Music Volume

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Music Volume\n");
   SafeWriteString(file,"; (low) 0 - 255 (high)\n");
   WriteParameter (file, "MusicVolume    ", MUvolume);

   // Write in FX Volume

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; FX Volume\n");
   SafeWriteString(file,"; (low) 0 - 255 (high)\n");
   WriteParameter (file, "FXVolume       ", FXvolume);

   // Write out numvoices

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Number of Voices\n");
   SafeWriteString(file,"; 1 - 8\n");
   WriteParameter(file,"NumVoices        ",NumVoices);

   // Write out numchannels

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Stereo or Mono\n");
   SafeWriteString(file,"; 1 - Mono\n");
   SafeWriteString(file,"; 2 - Stereo\n");
   WriteParameter(file,"NumChannels      ",NumChannels);

   // Write out numbits

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Resolution\n");
   SafeWriteString(file,"; 8 bit\n");
   SafeWriteString(file,"; 16 bit\n");
   WriteParameter(file,"NumBits          ",NumBits);

#ifdef DOS
   // Write out Midi Address

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Midi Addresses\n");
   SafeWriteString(file,"; $300\n");
   SafeWriteString(file,"; $310\n");
   SafeWriteString(file,"; $320\n");
   SafeWriteString(file,"; $330\n");
   SafeWriteString(file,"; $340\n");
   SafeWriteString(file,"; $350\n");
   SafeWriteString(file,"; $360\n");
   SafeWriteString(file,"; $370\n");
   SafeWriteString(file,"; $380\n");
   WriteParameterHex(file,"MidiAddress      ",MidiAddress);
#endif

   // Write out stereo reversal

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; ReverseStereo\n");
   SafeWriteString(file,"; 0 no reversal\n");
   SafeWriteString(file,"; 1 reverse stereo\n");
   WriteParameter (file,"StereoReverse      ",stereoreversed);

#ifdef DOS
   // Write out Sound Blaster info

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Sound Blaster Settings\n");
   WriteParameter(file, "SBType           ", SBSettings.Type );
   WriteParameterHex(file, "SBPort           ", SBSettings.Address );
   WriteParameter(file, "SBIrq            ", SBSettings.Interrupt );
   WriteParameter(file, "SBDma8           ", SBSettings.Dma8 );
   WriteParameter(file, "SBDma16          ", SBSettings.Dma16 );
   WriteParameterHex(file, "SBMidi           ", SBSettings.Midi );
   WriteParameterHex(file, "SBEmu            ", SBSettings.Emu );
#endif

   close (file);
   }


//******************************************************************************
//
// WriteConfig ()
//
//******************************************************************************

void WriteConfig (void)
{
   int file;
   char filename[ 128 ];
   char passwordtemp[50];
   static int inconfig = 0;

   if (inconfig > 0)
      return;

   inconfig++ ;

   if ( !ConfigLoaded )
      {
      return;
      }

   // Write Sound File
   WriteSoundConfig();

  // Write Config, Battle and Score files
#ifdef _ROTT_
   WriteScores();
   WriteBattleConfig();

   GetPathFromEnvironment( filename, ApogeePath, ConfigName );
   file = open( filename,O_RDWR | O_TEXT | O_CREAT | O_TRUNC
   , S_IREAD | S_IWRITE);

   if (file == -1)
      Error ("Error opening %s: %s",filename,strerror(errno));

   // Write out ROTTCONFIG header

   SafeWriteString (file, ";Rise of the Triad Configuration File\n");
   SafeWriteString (file, ";                  (c) 1995\n\n");

   // Write out Version

   WriteParameter(file,"Version          ",ROTTVERSION);

   // Write out MouseEnabled

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Mouse Enabled\n");
   SafeWriteString(file,"; 0 - Mouse Disabled\n");
   WriteParameter(file,"MouseEnabled     ",mouseenabled);

   // Write out UseMouseLook
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - UseMouseLook Enabled\n");
   SafeWriteString(file,"; 0 - UseMouseLook Disabled\n");
   WriteParameter(file,"UseMouseLook     ",usemouselook);

   // Write out InverseMouse
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Normal Mouse Enabled\n");
   SafeWriteString(file,"; -1 - Inverse Mouse Enabled\n");
   WriteParameter(file,"InverseMouse     ",inverse_mouse);

   // Write out UseJump
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - usejump Enabled\n");
   SafeWriteString(file,"; 0 - usejump Disabled\n");
   WriteParameter(file,"UseJump          ",usejump);

   // Write out CrossHair
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - CrossHair Enabled\n");
   SafeWriteString(file,"; 0 - CrossHair Disabled\n");
   WriteParameter(file,"CrossHair        ", iG_aimCross);

   // Write out JoystickEnabled

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Joystick Enabled\n");
   SafeWriteString(file,"; 0 - Joystick Disabled\n");
   WriteParameter(file,"JoystickEnabled  ",joystickenabled);

   // Write out JoypadEnabled

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Joypad Enabled\n");
   SafeWriteString(file,"; 0 - Joypad Disabled\n");
   WriteParameter(file,"JoypadEnabled    ",joypadenabled);

   // Write out JoystickPort

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 0 - Use Joystick Port 1\n");
   SafeWriteString(file,"; 1 - Use Joystick Port 2\n");
   WriteParameter(file,"JoystickPort     ",joystickport);

   // Write out fullscreen
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 0 - Start in windowed mode\n");
   SafeWriteString(file,"; 1 - Start in fullscreen mode\n");
   WriteParameter(file,"FullScreen       ",sdl_fullscreen);
      
   // Write out resolution
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Screen Resolution, supported resolutions: \n");
   SafeWriteString(file,"; 320x200, 640x480 and 800x600\n");
   WriteParameter(file,"ScreenWidth      ",iGLOBAL_SCREENWIDTH);
   WriteParameter(file,"ScreenHeight     ",iGLOBAL_SCREENHEIGHT);

   // Write out ViewSize

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Size of View port.\n");
   SafeWriteString(file,"; (smallest) 0 - 10 (largest)\n");
   WriteParameter(file,"ViewSize         ",viewsize);

   // Write out WEAPONSCALE  bna added

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Size of Weaponscale.\n");
   SafeWriteString(file,"; (smallest) 150 - 600 (largest)\n");
   G_weaponscale = (weaponscale * 168 )/65536;

   if ((G_weaponscale <150)||(G_weaponscale>600)){
	   if (iGLOBAL_SCREENWIDTH == 320){
			G_weaponscale=168;
	   }else if (iGLOBAL_SCREENWIDTH == 640){
			G_weaponscale=299;		
	   }else if (iGLOBAL_SCREENWIDTH == 800) {
			G_weaponscale=376;
	   }	   
   }
   WriteParameter(file,"Weaponscale         ",G_weaponscale);


   // Write out MouseAdjustment

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Sensitivity of Mouse\n");
   SafeWriteString(file,"; (lowest) 0 - 11 (highest)\n");
   WriteParameter(file,"MouseAdjustment  ",mouseadjustment);

   // Write out threshold

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Threshold of Mouse and Joystick\n");
   SafeWriteString(file,"; (smallest) 1 - 15 (largest)\n");
   WriteParameter(file,"Threshold        ",threshold);

   // Write in Cyberman Enabled

//   SafeWriteString(file,"\n;\n");
//   SafeWriteString(file,"; 1 - Cyberman Enabled\n");
//   SafeWriteString(file,"; 0 - Cyberman Disabled\n");
//   WriteParameter(file,"CybermanEnabled  ",cybermanenabled);

   // Write in Spaceball Enabled

//   SafeWriteString(file,"\n;\n");
//   SafeWriteString(file,"; 1 - Spaceball Enabled\n");
//   SafeWriteString(file,"; 0 - Spaceball Disabled\n");
//   WriteParameter(file,"SpaceballEnabled ",spaceballenabled);

   // Write in Auto Detail

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Auto Detail on\n");
   SafeWriteString(file,"; 0 - Auto Detail off\n");
   WriteParameter (file,"AutoDetail       ", AutoDetailOn);

   // Write in Light Dim

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Light Diminishing on\n");
   SafeWriteString(file,"; 0 - Light Diminishing off\n");
   WriteParameter (file,"LightDim         ", fulllight);

   // Write in Bobbin' On

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Bobbing on\n");
   SafeWriteString(file,"; 0 - Bobbing off\n");
   WriteParameter (file,"BobbingOn        ", BobbinOn);

   // Write in Double Click Speed

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; (slowest) 50 - 5 (fastest)\n");
   WriteParameter (file,"DoubleClickSpeed ", DoubleClickSpeed);

   // Write in Menu Flip Speed

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Menu Flip Speed\n");
   SafeWriteString(file,"; (slowest) 100 - 5 (fastest)\n");
   WriteParameter (file,"MenuFlipSpeed    ", Menuflipspeed);

   // Write in Detail Level

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 0 - Detail Level Low\n");
   SafeWriteString(file,"; 1 - Detail Level Medium\n");
   SafeWriteString(file,"; 2 - Detail Level High\n");
   WriteParameter (file,"DetailLevel      ", DetailLevel);

   // Write in Floor and Ceiling

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Floor and Ceiling on\n");
   SafeWriteString(file,"; 0 - Floor and Ceiling off\n");
   WriteParameter (file,"FloorCeiling     ", fandc);

   // Write in DisableMessages

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - Messages on\n");
   SafeWriteString(file,"; 0 - Messages off\n");
   WriteParameter (file,"Messages         ", MessagesEnabled );

   // Write in AutoRun

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 1 - AutoRun on\n");
   SafeWriteString(file,"; 0 - AutoRun off\n");
   WriteParameter (file,"AutoRun          ", gamestate.autorun );

   // Write in GammaIndex

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; 0 - Gamma Correction level 1\n");
   SafeWriteString(file,"; 1 - Gamma Correction level 2\n");
   SafeWriteString(file,"; 2 - Gamma Correction level 3\n");
   SafeWriteString(file,"; 3 - Gamma Correction level 4\n");
   SafeWriteString(file,"; 4 - Gamma Correction level 5\n");
   WriteParameter (file,"GammaIndex       ", gammaindex);

   // Write out screen saver time
   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Minutes before screen blanking\n");
   WriteParameter (file,"BlankTime        ", blanktime/(VBLCOUNTER*60));


   // Write out keys

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Scan codes for keyboard buttons\n");
   WriteParameter (file,"Fire             ", buttonscan[0]);
   WriteParameter (file,"Strafe           ", buttonscan[1]);
   WriteParameter (file,"Run              ", buttonscan[2]);
   WriteParameter (file,"Use              ", buttonscan[3]);
   WriteParameter (file,"LookUp           ", buttonscan[4]);
   WriteParameter (file,"LookDn           ", buttonscan[5]);
   WriteParameter (file,"Swap             ", buttonscan[6]);
   WriteParameter (file,"Drop             ", buttonscan[7]);
   WriteParameter (file,"TargetUp         ", buttonscan[8]);
   WriteParameter (file,"TargetDn         ", buttonscan[9]);
   WriteParameter (file,"SelPistol        ", buttonscan[10]);
   WriteParameter (file,"SelDualPistol    ", buttonscan[11]);
   WriteParameter (file,"SelMP40          ", buttonscan[12]);
   WriteParameter (file,"SelMissile       ", buttonscan[13]);
   WriteParameter (file,"AutoRun          ", buttonscan[14]);
   WriteParameter (file,"LiveRemRid       ", buttonscan[15]);
   WriteParameter (file,"StrafeLeft       ", buttonscan[16]);
   WriteParameter (file,"StrafeRight      ", buttonscan[17]);
   WriteParameter (file,"VolteFace        ", buttonscan[18]);
   WriteParameter (file,"Aim              ", buttonscan[19]);
   WriteParameter (file,"Forward          ", buttonscan[20]);
   WriteParameter (file,"Right            ", buttonscan[21]);
   WriteParameter (file,"Backward         ", buttonscan[22]);
   WriteParameter (file,"Left             ", buttonscan[23]);
   WriteParameter (file,"Map              ", buttonscan[24]);
   WriteParameter (file,"SendMessage      ", buttonscan[25]);
   WriteParameter (file,"DirectMessage    ", buttonscan[26]);

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Mouse buttons\n");

   WriteParameter (file,"MouseButton0     ", buttonmouse[0]);
   WriteParameter (file,"MouseButton1     ", buttonmouse[1]);
   WriteParameter (file,"MouseButton2     ", buttonmouse[2]);
   WriteParameter (file,"DblClickB0       ", buttonmouse[3]);
   WriteParameter (file,"DblClickB1       ", buttonmouse[4]);
   WriteParameter (file,"DblClickB2       ", buttonmouse[5]);

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Joystick buttons\n");

   WriteParameter (file,"JoyButton0       ", buttonjoy[0]);
   WriteParameter (file,"JoyButton1       ", buttonjoy[1]);
   WriteParameter (file,"JoyButton2       ", buttonjoy[2]);
   WriteParameter (file,"JoyButton3       ", buttonjoy[3]);
   WriteParameter (file,"DblClickJB0      ", buttonjoy[4]);
   WriteParameter (file,"DblClickJB1      ", buttonjoy[5]);
   WriteParameter (file,"DblClickJB2      ", buttonjoy[6]);
   WriteParameter (file,"DblClickJB3      ", buttonjoy[7]);

   SafeWriteString(file,"\n;\n");
   SafeWriteString(file,"; Joystick calibration coordinates\n");

   WriteParameter (file,"JoyMaxX          ", joyxmax);
   WriteParameter (file,"JoyMaxY          ", joyymax);
   WriteParameter (file,"JoyMinX          ", joyxmin);
   WriteParameter (file,"JoyMinY          ", joyymin);

   // Write out DefaultDifficulty
   SafeWriteString(file,"\n;\n");
	WriteParameter(file,"; Easy             - ", gd_baby );
	WriteParameter(file,"; Medium           - ", gd_easy );
	WriteParameter(file,"; Hard             - ", gd_medium );
	WriteParameter(file,"; Crezzy           - ", gd_hard );
   WriteParameter(file,"DefaultDifficulty    ", DefaultDifficulty );

   // Write out DefaultPlayerCharacter
   SafeWriteString(file,"\n;\n");
	WriteParameter(file,"; Taradino Cassatt   - ", 0 );
	WriteParameter(file,"; Thi Barrett        - ", 1 );
	WriteParameter(file,"; Doug Wendt         - ", 2 );
	WriteParameter(file,"; Lorelei Ni         - ", 3 );
	WriteParameter(file,"; Ian Paul Freeley   - ", 4 );
   WriteParameter(file,"DefaultPlayerCharacter ", DefaultPlayerCharacter );

   // Write out DefaultPlayerColor
   SafeWriteString(file,"\n;\n");
	WriteParameter(file,"; Gray             - ", 0 );
	WriteParameter(file,"; Brown            - ", 1 );
	WriteParameter(file,"; Black            - ", 2 );
	WriteParameter(file,"; Tan              - ", 3 );
	WriteParameter(file,"; Red              - ", 4 );
	WriteParameter(file,"; Olive            - ", 5 );
	WriteParameter(file,"; Blue             - ", 6 );
	WriteParameter(file,"; White            - ", 7 );
	WriteParameter(file,"; Green            - ", 8 );
	WriteParameter(file,"; Purple           - ", 9 );
	WriteParameter(file,"; Orange           - ", 10 );
   WriteParameter(file,"DefaultPlayerColor   ", DefaultPlayerColor );

   // Writeout password Password string
   SafeWriteString(file,"\n;\nSecretPassword         ");
   memset(passwordtemp,0,sizeof(passwordtemp));
   ConvertPasswordStringToString ( &passwordtemp[0] );
   SafeWriteString(file,&passwordtemp[0]);

   close (file);
#endif
   inconfig--;
}

#ifdef _ROTT_


//****************************************************************************
//
// GetAlternatePath ()
//
//****************************************************************************

void GetAlternatePath (char * tokenstr, AlternateInformation *info)
{
   strcpy (&info->path[0], ".\0");
   GetToken (true);
   if (!stricmp (token, tokenstr))
      {
      GetTokenEOL (false);
      memset (&info->path[0], 0, sizeof (info->path));
      strcpy (&info->path[0], &name[0]);
      }
}


//****************************************************************************
//
// GetAlternateFile ()
//
//****************************************************************************

void GetAlternateFile (char * tokenstr, AlternateInformation *info)
{
   // Read in remote sound file
   //
   strcpy (&info->file[0], "foo.foo\0");
   GetToken (true);
   if (!stricmp (token, tokenstr))
      {
      if (TokenAvailable()==true)
         {
         GetToken (false);
         if (stricmp (token, "~"))
            {
            #if (SHAREWARE == 0)
            info->avail = true;
            memset (&info->file[0], 0, sizeof (info->file));
            strcpy (&info->file[0], &token[0]);
            #else
            printf("Alternate file %s ignored.\n",token);
            memset (&info->file[0], 0, sizeof (info->file));
            #endif
            }
         }
      }
}


//****************************************************************************
//
// ReadSETUPFiles ()
//
//****************************************************************************

void ReadSETUPFiles (void)
{
   char filename[ 128 ];
   int i;

   RemoteSounds.avail   = false;
//   PlayerGraphics.avail = false;
   GameLevels.avail     = false;
   BattleLevels.avail   = false;

   GetPathFromEnvironment( filename, ApogeePath, CONFIG );
   if (access (filename, F_OK) == 0)
   {
      LoadScriptFile (filename);

      GetTokenEOL (true);     //MODEMNAME
      GetTokenEOL (true);     //MODEMINITSTR
      GetTokenEOL (true);     //MODEMHANGUP
      GetTokenEOL (true);     //RATE
      GetTokenEOL (true);     //COMPORT
      GetTokenEOL (true);     //IRQ
      GetTokenEOL (true);     //UART
      GetTokenEOL (true);     //PULSE
      GetTokenEOL (true);     //AUTOMATICDIALOUT

      GetAlternatePath ("REMOTESOUNDPATH", &RemoteSounds);
//      GetAlternatePath ("PLAYERGRAPHICSPATH", &PlayerGraphics);
      GetAlternatePath ("GAMELEVELPATH", &GameLevels);
      GetAlternatePath ("BATTLELEVELPATH", &BattleLevels);

      // Get CodeName
      GetToken (true);
      if (stricmp (token, "CODENAME"))
         Error ("Can't find %s token.\n", "CODENAME");

      GetTokenEOL (false);
      memset (&CodeName[0], 0, sizeof (CodeName));
      if (stricmp (name, "~"))
         {

         // Get First (MAXCODENAMELENGTH-1) characters
         for (i=0;i<MAXCODENAMELENGTH-1;i++)
            CodeName[i]=name[i];
         }
      GetTokenEOL (true);     //NUMPLAYERS
      GetTokenEOL (true);     //NETWORKSOCKET
      GetTokenEOL (true);     //DEFAULT
      for (i=0;i<14;i++)
         GetTokenEOL (true);  //NUMBERLIST

      memset (CommbatMacros, 0, sizeof(CommbatMacros) );

      for (i=0;i<MAXMACROS;i++)
         {
         GetToken (true);

         GetTokenEOL (true);

         if (name[0] != '~')
            {
            memcpy (&CommbatMacros[i].macro[0], &name[0], strlen (name));
            CommbatMacros[i].avail = 1;
            }
         }

      Z_Free (scriptbuffer);
   }

   GetPathFromEnvironment( filename, ApogeePath, ROTT );
   if (access (filename, F_OK) == 0)
   {
      LoadScriptFile (filename);

      GetTokenEOL (true);     //PHONENUMBER

      GetAlternateFile ("REMOTESOUNDFILE", &RemoteSounds);
//      GetAlternateFile ("PLAYERGRAPHICSFILE", &PlayerGraphics);
      GetAlternateFile ("GAMELEVELFILE", &GameLevels);
      GetAlternateFile ("COMMBATLEVELFILE", &BattleLevels);

      Z_Free (scriptbuffer);

      unlink (filename);          // Delete ROTT.ROT
   }
}

#endif

