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
#ifndef _rt_sound_public
#define _rt_sound_public

#include "music.h"
#include "develop.h"

typedef enum {

//	MENU SOUNDS

		   SD_MENUFLIP,
		   SD_ESCPRESSEDSND,
		   SD_MOVECURSORSND,
		   SD_SELECTSND,
		   SD_WARNINGBOXSND,
		   SD_INFOBOXSND,
		   SD_QUESTIONBOXSND,
		   SD_NOPESND,
		   SD_QUIT1SND,
		   SD_QUIT2SND,
		   SD_QUIT3SND,
		   SD_QUIT4SND,
		   SD_QUIT5SND,
		   SD_QUIT6SND,
		   SD_QUIT7SND,


//	GAME SOUNDS

		   SD_LEVELSTARTSND,
		   SD_LEVELDONESND,
		   SD_GAMEOVERSND,

//	LEVEL END SCREEN

		   SD_ENDBONUS1SND,
		   SD_NOBONUSSND,
		   SD_PERCENT100SND,

//	PLAYER SOUNDS

		   SD_HITWALLSND,
		   SD_SELECTWPNSND,
		   SD_NOWAYSND,
		   SD_DONOTHINGSND,
		   SD_NOITEMSND,
		   SD_PLAYERDYINGSND,

         SD_PLAYERTCDEATHSND,
         SD_PLAYERTBDEATHSND,
         SD_PLAYERDWDEATHSND,
         SD_PLAYERLNDEATHSND,
         SD_PLAYERIPFDEATHSND,
         SD_PLAYERTCHURTSND,
         SD_PLAYERTBHURTSND,
         SD_PLAYERDWHURTSND,
         SD_PLAYERLNHURTSND,
         SD_PLAYERIPFHURTSND,
         SD_PLAYERTCSND,
         SD_PLAYERTBSND,
         SD_PLAYERDWSND,
         SD_PLAYERLNSND,
         SD_PLAYERIPFSND,
//         SD_WALK1SND,
//         SD_WALK2SND,
		   SD_PLAYERBURNEDSND,
		   SD_PLAYERLANDSND,
		   SD_PLAYERCOUGHMSND,
		   SD_PLAYERCOUGHFSND,
		   SD_NETWIGGLESND,
		   SD_NETFALLSND,

//	PLAYER WEAPONS

		   SD_ATKPISTOLSND,
         SD_ATKTWOPISTOLSND,
		   SD_ATKMP40SND,
         SD_RICOCHET1SND,
         SD_RICOCHET2SND,
         SD_RICOCHET3SND,
		   SD_BAZOOKAFIRESND,
		   SD_FIREBOMBFIRESND,
		   SD_HEATSEEKFIRESND,
		   SD_DRUNKFIRESND,
		   SD_FLAMEWALLFIRESND,
		   SD_FLAMEWALLSND,
		   SD_SPLITFIRESND,
		   SD_SPLITSND,
		   SD_GRAVBUILDSND,
		   SD_GRAVFIRESND,
		   SD_GRAVSND,
		   SD_GRAVHITSND,
		   SD_FIREHITSND,
         SD_MISSILEFLYSND,
		   SD_MISSILEHITSND,
		   SD_EXCALIBOUNCESND,
		   SD_EXCALISWINGSND,
		   SD_EXCALIHITSND,
		   SD_EXCALIBUILDSND,
		   SD_EXCALIBLASTSND,
		   SD_GODMODEFIRESND,
		   SD_GODMODE1SND,
		   SD_GODMODE2SND,
		   SD_GODMODE3SND,
		   SD_LOSEMODESND,
		   SD_DOGMODEPANTSND,
         SD_DOGMODEBITE1SND,
         SD_DOGMODEBITE2SND,
         SD_DOGMODELICKSND,
         SD_DOGMODEBLASTSND,
         SD_DOGMODEPREPBLASTSND,
         SD_DOGMANSND,
         SD_DOGWOMANSND,
         SD_GODMANSND,
         SD_GODWOMANSND,
         SD_FLYINGSND,

//	PLAYER-CAUSED SOUNDS

		   SD_GLASSBREAKSND,
		   SD_ITEMBLOWSND,
		   SD_BONUSBARRELSND,
		   SD_TOUCHPLATESND,
		   SD_BADTOUCHSND,
         SD_EXPLODEFLOORSND,
		   SD_EXPLODESND,
		   SD_GASSTARTSND,
		   SD_GASHISSSND,
		   SD_GASENDSND,
		   SD_GASMASKSND,

//	GET ITEM SOUNDS

		   SD_GETKEYSND,
         SD_GETBONUSSND,
		   SD_GETHEALTH1SND,
		   SD_GETHEALTH2SND,
		   SD_COOKHEALTHSND,

		   SD_GETWEAPONSND,
		   SD_GETKNIFESND,
		   SD_GETGODSND,
		   SD_GETDOGSND,
		   SD_GETFLEETSND,
		   SD_GETELASTSND,
		   SD_GETSHROOMSSND,
		   SD_GETBVESTSND,
		   SD_GETAVESTSND,
		   SD_GETMASKSND,
		   SD_GETBATSND,
		   SD_GETHEADSND,

		   SD_GET1UPSND,
		   SD_GET3UPSND,
         SD_RESPAWNSND,
         SD_PLAYERSPAWNSND,

//	ACTOR SOUNDS

		   SD_LOWGUARD1SEESND,
		   SD_LOWGUARD1ASEESND,
		   SD_LOWGUARD1SEE3SND,
		   SD_LOWGUARD2SEESND,
		   SD_LOWGUARD2ASEESND,
		   SD_LOWGUARD2SEE3SND,
		   SD_LOWGUARDFIRESND,
		   SD_LOWGUARDOUCHSND,
         SD_LOWGUARD1DIESND,
         SD_LOWGUARD2DIESND,
		   SD_SNEAKYSPRINGMSND,
		   SD_SNEAKYSPRINGFSND,

         SD_HIGHGUARD1SEESND,
		   SD_HIGHGUARD2SEESND,
         SD_HIGHGUARDFIRESND,
		   SD_HIGHGUARDOUCHSND,
		   SD_HIGHGUARDDIESND,

         SD_OVERP1SEESND,
		   SD_OVERP2SEESND,
         SD_OVERPFIRESND,
		   SD_OVERPNETSND,
		   SD_OVERPOUCHSND,
		   SD_OVERPDIESND,

         SD_STRIKE1SEESND,
		   SD_STRIKE2SEESND,
         SD_STRIKEFIRESND,
		   SD_STRIKEROLLSND,
		   SD_STRIKEOUCHSND,
		   SD_STRIKEDIESND,

         SD_BLITZ1SEESND,
         SD_BLITZ2SEESND,
         SD_BLITZFIRESND,
		   SD_BLITZSTEALSND,
		   SD_BLITZOUCHSND,
		   SD_BLITZDIESND,
         SD_BLITZPLEADSND,
         SD_BLITZPLEAD1SND,
         SD_BLITZPLEAD2SND,

		   SD_ENFORCERSEESND,
		   SD_ENFORCERFIRESND,
		   SD_ENFORCERTHROWSND,
		   SD_ENFORCEROUCHSND,
		   SD_ENFORCERDIESND,

		   SD_MONKSEESND,
		   SD_MONKGRABSND,
		   SD_MONKOUCHSND,
		   SD_MONKDIESND,

		   SD_FIREMONKSEESND,
		   SD_FIREMONKFIRESND,
		   SD_FIREMONKOUCHSND,
		   SD_FIREMONKDIESND,

		   SD_ROBOTSEESND,
		   SD_ROBOTFIRESND,
		   SD_ROBOTDIESND,
         SD_ROBOTMOVESND,

         SD_BALLISTIKRAFTSEESND,
         SD_BALLISTIKRAFTFIRESND,

         SD_DARIANSEESND,
         SD_DARIANFIRESND,
		   SD_DARIANGONNAUSESND,
		   SD_DARIANUSESND,
		   SD_DARIANHIDESND,
		   SD_DARIANDIESND,
         SD_DARIANSAY1,
         SD_DARIANSAY2,
         SD_DARIANSAY3,

		   SD_KRISTSEESND,
		   SD_KRISTFIRESND,
		   SD_KRISTMOTORSND,
		   SD_KRISTTURNSND,
		   SD_KRISTDROPSND,
		   SD_KRISTMINEBEEPSND,
		   SD_KRISTMINEHITSND,
		   SD_KRISTDIESND,
         SD_KRISTSAY1,
         SD_KRISTSAY2,
         SD_KRISTSAY3,

		   SD_NMESEESND,
		   SD_NMEREADYSND,
		   SD_NMEFIRE1SND,
		   SD_NMEAPARTSND,
		   SD_NMEUFOSND,
		   SD_NMEDIESND,

		   SD_DARKMONKSEESND,
		   SD_DARKMONKFIRE1SND,
		   SD_DARKMONKFIRE2SND,
		   SD_DARKMONKFIRE3SND,
		   SD_DARKMONKFIRE4SND,
         SD_DARKMONKRECHARGESND,
		   SD_DARKMONKFLOATSND,
		   SD_DARKMONKDIESND,
         SD_DARKMONKSAY1,
         SD_DARKMONKSAY2,
         SD_DARKMONKSAY3,

		   SD_SNAKESEESND,
		   SD_SNAKEREADYSND,
		   SD_SNAKECHARGESND,
		   SD_SNAKEOUCHSND,
		   SD_SNAKEDIESND,
		   SD_SNAKESPITSND,
         SD_SNAKESAY1,
         SD_SNAKESAY2,
         SD_SNAKESAY3,

		   SD_EMPLACEMENTSEESND,
		   SD_EMPLACEMENTFIRESND,
		   SD_BIGEMPLACEFIRESND,


//	ENVIRONMENT SOUNDS

         SD_OPENDOORSND,
         SD_CLOSEDOORSND,
         SD_DOORHITSND,
         SD_FIRECHUTESND,
		   SD_FIREBALLSND,
		   SD_FIREBALLHITSND,
		   SD_BLADESPINSND,
		   SD_PUSHWALLSND,
		   SD_PUSHWALLHITSND,
		   SD_GOWALLSND,
		   SD_TURBOWALLSND,
		   SD_BOULDERHITSND,
		   SD_BOULDERROLLSND,
		   SD_BOULDERFALLSND,
		   SD_PITTRAPSND,
		   SD_FIREJETSND,
		   SD_ACTORSQUISHSND,
		   SD_ACTORBURNEDSND,
         SD_ACTORSKELETONSND,

		   SD_SPEARSTABSND,
		   SD_CYLINDERMOVESND,
		   SD_ELEVATORONSND,
         SD_ELEVATORENDSND,

		   SD_SPRINGBOARDSND,
         SD_LIGHTNINGSND,
         SD_WINDSND,
         SD_WATERSND,
         SD_BODYLANDSND,
         SD_GIBSPLASHSND,
         SD_ACTORLANDSND,

//	SECRET SOUNDS

		   SD_DOPEFISHSND,
		   SD_YOUSUCKSND,

		   SD_SILLYMOVESND,
		   SD_SOUNDSELECTSND,
		   SD_SOUNDESCSND,

//	REMOTE SOUNDS (shift number row)

		   SD_REMOTEM1SND,
		   SD_REMOTEM2SND,
		   SD_REMOTEM3SND,
		   SD_REMOTEM4SND,
		   SD_REMOTEM5SND,
		   SD_REMOTEM6SND,
		   SD_REMOTEM7SND,
		   SD_REMOTEM8SND,
		   SD_REMOTEM9SND,
		   SD_REMOTEM10SND,

         SD_LASTSOUND,

         MAXSOUNDS

	          } game_sounds;


typedef enum
         {
// REMOTE SOUNDS (shift number row)

		   D_REMOTEM1SND,
		   D_REMOTEM2SND,
		   D_REMOTEM3SND,
		   D_REMOTEM4SND,
		   D_REMOTEM5SND,
		   D_REMOTEM6SND,
		   D_REMOTEM7SND,
		   D_REMOTEM8SND,
		   D_REMOTEM9SND,
		   D_REMOTEM10SND,
         } remotesounds;

typedef enum {
		MUSE_MENUFLIPSND,             // 0
		MUSE_ESCPRESSEDSND,           // 1
		MUSE_MOVECURSORSND,           // 2
		MUSE_SELECTSND,               // 3
		MUSE_WARNINGBOXSND,           // 4
		MUSE_INFOBOXSND,              // 5
		MUSE_QUESTIONBOXSND,          // 6
		MUSE_NOPESND,                 // 7
		MUSE_LEVELSTARTSND,           // 8
		MUSE_LEVELENDSND,             // 9
		MUSE_GAMEOVERSND,             // 10
		MUSE_ENDBONUS1SND,            // 11
		MUSE_ENDBONUS2SND,            // 12
		MUSE_NOBONUSSND,              // 13
		MUSE_PERCENT100SND,           // 14
		MUSE_HITWALLSND,              // 15
		MUSE_SELECTWPNSND,            // 16
		MUSE_NOWAYSND,                // 17
		MUSE_DONOTHINGSND,            // 18
		MUSE_NOITEMSND,               // 19
		MUSE_PLAYERDYINGSND,          // 20
		MUSE_PLAYERDEATHSND,          // 21
		MUSE_PLAYERHURTSND,           // 22
		MUSE_PLAYERYESSND,            // 23
		MUSE_WALK1SND,                // 24
		MUSE_WALK2SND,                // 25
		MUSE_PLAYERLANDSND,           // 26
		MUSE_NETFALLSND,              // 27
		MUSE_ATKKNIFESND,             // 28
		MUSE_ATKPISTOLSND,            // 29
		MUSE_ATKMP40SND,              // 30
		MUSE_RICOCHETSND,             // 31
		MUSE_MISSILEFIRESND,          // 32
		MUSE_FLAMEWALLSND,            // 33
		MUSE_MISSILEHITSND,           // 34
		MUSE_WEAPONBUILDSND,          // 35
		MUSE_STABBERSND,              // 36
		MUSE_ENERGYFIRESND,           // 37
		MUSE_GAINMODESND,             // 38
		MUSE_LOSEMODESND,             // 39
		MUSE_DOGLICKSND,              // 40
		MUSE_DOGBITESND,              // 41
		MUSE_GLASSBREAKSND,           // 42
		MUSE_EXPLOSIONSND,            // 43
		MUSE_TOUCHPLATESND,           // 44
		MUSE_BADTOUCHSND,             // 45
		MUSE_SWITCHSND,               // 46
		MUSE_GETKEYSND,               // 47
		MUSE_GETBONUSSND,             // 48
		MUSE_GETHEALTHSND,            // 49
		MUSE_GETWEAPONSND,            // 50
		MUSE_GETMWEAPONSND,           // 51
		MUSE_GETPOWERUPSND,           // 52
		MUSE_GETPOWERDOWNSND,         // 53
		MUSE_GETARMORSND,             // 54
		MUSE_GETWEIRDSND,             // 55
		MUSE_GETLIFESND,              // 56
		MUSE_ACTORSEESND,             // 57
		MUSE_ACTORFIRESND,            // 58
		MUSE_ACTOROUCHSND,            // 59
		MUSE_ACTORDIESND,             // 60
		MUSE_ACTORTHROWSND,           // 61
		MUSE_ACTORROLLSND,            // 62
		MUSE_ACTORDOITSND,            // 63
		MUSE_ACTORUSESND,             // 64
		MUSE_BOSSSEESND,              // 65
		MUSE_BOSSOUCHSND,             // 66
		MUSE_BOSSDIESND,              // 67
		MUSE_BOSSDOSND,               // 68
		MUSE_BOSSBEEPSND,             // 69
		MUSE_BOSSHEYSND,              // 70
		MUSE_BOSSFIRESND,             // 71
		MUSE_BOSSWARNSND,             // 72
		MUSE_BOSSFIRE2SND,            // 73
		MUSE_EMPFIRESND,              // 74
		MUSE_OPENDOORSND,             // 75
		MUSE_CLOSEDOORSND,            // 76
		MUSE_SPINBLADESND,            // 77
		MUSE_PUSHWALLSND,             // 78
		MUSE_BOULDERSND,              // 79
		MUSE_PITTRAPSND,              // 80
		MUSE_FIREJETSND,              // 81
		MUSE_ACTORSQUISHSND,          // 82
		MUSE_CYLINDERHITSND,          // 83
		MUSE_ELEVATORSND,             // 84
		MUSE_SPRINGBOARDSND,          // 85
		MUSE_LASTSOUND=-1
	     } musesounds;

#define NUMCARDS 6

typedef enum {
   fx_digital,
   fx_muse
} fxtypes;

typedef enum {
   ASS_UltraSound,
   ASS_SoundBlaster,
   ASS_SoundMan16,
   ASS_PAS,
   ASS_AWE32,
   ASS_SoundScape,
   ASS_WaveBlaster,
   ASS_Adlib,
   ASS_GeneralMidi,
   ASS_SoundCanvas,
   ASS_SoundSource,
   ASS_TandySoundSource,
   ASS_PCSpeaker,
   ASS_Off
} ASSTypes;


extern int SD_Started;

int SD_SetupFXCard ( int * numvoices, int * numbits, int * numchannels);
int SD_Startup ( boolean bombonerror );
int SD_Play ( int sndnum );
void SD_Shutdown (void);

int SD_PlayPositionedSound ( int sndnum, int px, int py, int x, int y );
int SD_PlaySoundRTP        ( int sndnum, int x, int y );
void SD_PanPositionedSound ( int handle, int px, int py, int x, int y );
void SD_PanRTP ( int handle, int x, int y );
void SD_SetPan ( int handle, int vol, int left, int right );
int SD_Play3D ( int sndnum, int angle, int distance );
int SD_PlayPitchedSound ( int sndnum, int volume, int pitch );
void SD_SetSoundPitch ( int sndnum, int pitch );
boolean SD_SoundOkay ( int sndnum );

//***************************************************************************
//
// SD_WaitSound - wait until a sound has finished
//
//***************************************************************************
void SD_WaitSound ( int handle );

//***************************************************************************
//
// SD_StopSound
//
//***************************************************************************
void  SD_StopSound ( int handle );

//***************************************************************************
//
// SD_SoundActive
//
//***************************************************************************
int SD_SoundActive ( int handle );

//***************************************************************************
//
// SD_StopAllSounds
//
//***************************************************************************
void  SD_StopAllSounds ( void );


typedef enum {
   song_gason,
   song_bosssee,
   song_bossdie,
   song_endlevel,
   song_dogend,
   song_title,
   song_apogee,
   song_youwin,
   song_level,
   song_elevator,
   song_secretmenu,
   song_cinematic1,
   song_cinematic2,
   song_cinematic3,
   song_gameover,
   song_christmas,
   song_snakechase,
   song_menu
} songtypes;



void MU_Shutdown ( void );
int MU_Startup ( boolean bombonerror );
void MU_PlaySong ( int num );
void MU_StopSong ( void );

//***************************************************************************
//
// MU_Continue
//
//***************************************************************************
#define MU_Continue() MUSIC_Continue()

//***************************************************************************
//
// MU_Pause
//
//***************************************************************************
#define MU_Pause() MUSIC_Pause()

//***************************************************************************
//
// MU_GetVolume
//
//***************************************************************************
#define MU_GetVolume() MUSIC_GetVolume()

//***************************************************************************
//
// MU_SetVolume
//
//***************************************************************************
#define MU_SetVolume(x) MUSIC_SetVolume(x)

//***************************************************************************
//
// MU_SongPlaying
//
//***************************************************************************
#define MU_SongPlaying() MUSIC_SongPlaying()

//***************************************************************************
//
// MU_FadeVolume
//
//***************************************************************************
#define MU_FadeVolume(v,m) MUSIC_FadeVolume(v,m)

//***************************************************************************
//
// MU_FadeActive
//
//***************************************************************************
#define MU_FadeActive() MUSIC_FadeActive()

//***************************************************************************
//
// MU_StopFade
//
//***************************************************************************
#define MU_StopFade() MUSIC_StopFade()


//***************************************************************************
//
// MU_FadeIn
//
//***************************************************************************
void MU_FadeIn ( int num, int time );

//***************************************************************************
//
// MU_FadeOut
//
//***************************************************************************
void MU_FadeOut ( int time );

//***************************************************************************
//
// MU_FadeToSong
//
//***************************************************************************
void MU_FadeToSong ( int num, int time );

//***************************************************************************
//
// SD_PreCacheSoundGroup
//
//***************************************************************************
void SD_PreCacheSoundGroup ( int lo, int hi );

//***************************************************************************
//
// SD_PreCacheSound
//
//***************************************************************************

void SD_PreCacheSound ( int num );

void MU_StartSong ( int songtype );

void MU_JukeBoxMenu( void );

void MU_StoreSongPosition ( void );

void MU_RestoreSongPosition ( void );

int MU_GetSongPosition ( void );

void MU_SetSongPosition ( int position );

int MU_GetSongNumber ( void );

int MU_GetNumForType ( int songtype );

int MU_GetStoredPosition ( void );

void MU_SetStoredPosition ( int position );

void MU_LoadMusic (byte * buf, int size);

void MU_SaveMusic (byte ** buf, int * size);

boolean MusicStarted( void );
void FX_SetReverb (int level);

#endif
