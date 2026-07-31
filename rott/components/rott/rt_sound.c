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
#include "rt_sound.h"
#include "_rt_soun.h"
#include "fx_man.h"
#include "music.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_main.h"
#include "rt_ted.h"
#include "rt_menu.h"
#include "rt_playr.h"
#include "rt_util.h"
#include "rt_rand.h"
#include "watcom.h"
#include <stdio.h>
#include <stdlib.h>

#if PLATFORM_DOS
#include <mem.h>
#include <io.h>
#elif PLATFORM_UNIX
#include <unistd.h>
#endif

#include "rt_cfg.h"
#include "isr.h"
#include "develop.h"
#include "rt_net.h"

#include "rt_str.h"

#if (SHAREWARE==0)
#include "snd_reg.h"
#else
#include "snd_shar.h"
#endif
//MED
#include "memcheck.h"

// Local Variables

static int soundstart;
static int soundtype;
int SD_Started=false;
static boolean PositionStored=false;
static int NumBadSounds=0;
static int remotestart;
static boolean SoundsRemapped = false;

#ifdef DOS
int musicnums[ 11 ] = {
   -1, UltraSound, SoundBlaster, SoundMan16, ProAudioSpectrum,
   Awe32, SoundScape, WaveBlaster, GenMidi, SoundCanvas, Adlib
   };

int fxnums[ 11 ] = {
   -1, UltraSound, SoundBlaster, SoundMan16, ProAudioSpectrum,
   Awe32, SoundScape, Adlib, SoundSource, TandySoundSource, PC
   };
#else
int musicnums[ 11 ] = {
   -1, -1, -1, -1, -1, -1, SoundScape, -1, -1, -1, -1
   };

int fxnums[ 11 ] = {
   -1, -1, -1, -1, -1, -1, SoundScape, -1, -1, -1, -1
   };
#endif

#if 0
void MU_SetupGUSInitFile( void );
#endif

int MUSIC_GetPosition( void ) {
   songposition pos;

   MUSIC_GetSongPosition( &pos );
   return pos.milliseconds;
}

void MUSIC_SetPosition( int time ) {
   MUSIC_SetSongTime( ( unsigned long )time );
}


//***************************************************************************
//
// SoundNumber
//
//***************************************************************************

int SoundNumber ( int x )
{
   if ((x>=SD_REMOTEM1SND) && (x<=SD_REMOTEM10SND))
      return remotestart + x - SD_REMOTEM1SND;
//      sounds[x].snds[soundtype]+remotestart;
   else
      return sounds[x].snds[soundtype]+soundstart;
}


//***************************************************************************
//
// SD_MakeCacheable - Make a sound that has just finished playing cacheable again
//
//***************************************************************************
void SD_MakeCacheable( unsigned long sndnum )
{
   if (sndnum == (unsigned long) -1)
      {
      return;
      }

   if (sndnum>=MAXSOUNDS)
      {
      SoftError ("Illegal sound value in SD_MakeCacheable value=%ld\n",sndnum);
      return;
      }
   sounds[sndnum].count--;
   if (sounds[sndnum].count>0)
      return;
   else
      W_CacheLumpNum(SoundNumber(sndnum),PU_CACHE, CvtNull, 1);
}

#if 0
//***************************************************************************
//
// SD_PrintActive
//
//***************************************************************************
void SD_PrintActive ( void )
{
   int i;

   myprintf("Active Sounds\n");
   for (i=0;i<MAXSOUNDS;i++)
      {
      if (sounds[i].count>0)
         {
         myprintf("sound active #%ld\n",i);
         }
      }
}
#endif

//***************************************************************************
//
// SD_SetupFXCard - Initialize sound Tables and start up sound card
//
//***************************************************************************

int SD_SetupFXCard ( int * numvoices, int * numbits, int * numchannels)
{
   fx_device device;
   int status;
   int card;

   if (SD_Started==true)
      SD_Shutdown();

   if ( ( FXMode < 0 ) || ( FXMode >= 11 ) )
      {
      return( 0 );
      }

   card = fxnums[ FXMode ];
   if (card==-1) // Check if it is off
      return (0);
#ifdef DOS
   if ( ( card == SoundBlaster ) || ( card == Awe32 ) )
      {
      extern fx_blaster_config SBSettings;

      status = FX_SetupSoundBlaster( SBSettings, numvoices,
         numbits, numchannels );
      }
   else
      {
#endif
      status=FX_SetupCard( card, &device );
      if ( status == FX_Ok )
         {
         *numvoices=device.MaxVoices;
         *numbits=device.MaxSampleBits;
         *numchannels=device.MaxChannels;
         }
#ifdef DOS
      }
#endif

   return (status);
   }

//***************************************************************************
//
// SD_Startup - Initialize sound Tables and start up sound card
//
//***************************************************************************

int SD_Startup ( boolean bombonerror )
{
   int status;
   int card;
   int voices;
   int channels;
   int bits;
   int i;
   extern boolean IS8250;

   if (SD_Started==true)
      SD_Shutdown();

   if ( ( FXMode < 0 ) || ( FXMode >= 11 ) )
      {
      return( 0 );
      }
   card = fxnums[ FXMode ];
   if (card==-1) // Check if it is off
      return (0);

   switch (card)
      {
#ifdef DOS
      case UltraSound:
      case SoundBlaster:
      case SoundMan16:
      case ProAudioSpectrum:
      case Awe32:
      case SoundSource:
      case TandySoundSource:
#endif
      case SoundScape:
         soundstart=W_GetNumForName("digistrt")+1;
         soundtype=fx_digital;
         break;
#ifdef DOS
      case Adlib:
         soundstart=W_GetNumForName("adstart")+1;
         soundtype=fx_muse;
         break;
      case PC:
         soundstart=W_GetNumForName("pcstart")+1;
         soundtype=fx_muse;
         break;
#endif
      default:
         Error("FX: Unsupported Card number %d",FXMode);
         break;
      }

   if ( soundtype == fx_digital )
      {
      if ( SoundsRemapped == false )
         {
         for( i = 0; i < SD_LASTSOUND; i++ )
            {
            int snd;

            snd = sounds[ i ].snds[ fx_digital ];
            if ( snd >= 0)
               {
               sounds[ i ].snds[ fx_digital ] = W_GetNumForName(
                  W_GetNameForNum( snd + soundstart ) );
               }
            }
         SoundsRemapped = true;
         }
      soundstart = 0;
      }

   voices   = NumVoices;
   channels = NumChannels;
   bits     = NumBits;

   if ( IS8250 )
      {
      voices   = max( voices, 4 );
      channels = 1;
      bits     = 8;
      }

#ifdef DOS
   status=FX_Init( card, voices, channels, bits, 11000 );
#else
   status=FX_Init( card, voices, channels, bits, 11025 );
#endif

   if (status != FX_Ok)
      {
      if (bombonerror)
         {
         DeleteSoundFile ();
         Error( "%s\n", FX_ErrorString( status ) );
         }

      return (status);
      }

   if (stereoreversed == true)
      {
      FX_SetReverseStereo(!FX_GetReverseStereo());
      }

   FX_SetCallBack( SD_MakeCacheable );

   remotestart=W_GetNumForName("remostrt")+1;

   SD_Started=true;

   FX_SetVolume (FXvolume);

   return (0);
}

//***************************************************************************
//
// SD_SoundOkay - checks to see if the sound is okay
//
//***************************************************************************

boolean SD_SoundOkay ( int sndnum )
{
   if (SD_Started==false)
      return false;

   if (sndnum>=MAXSOUNDS)
      Error ("Illegal sound number, sound number = %d\n",sndnum);

   if (SoundOffset(sndnum)==-1)
      return false;

   if ( ( sounds[ sndnum ].flags & SD_PLAYONCE ) &&
      ( SD_SoundActive( sounds[ sndnum ].prevhandle ) ) )
      {
      return false;
      }

   return true;
}

//***************************************************************************
//
// SD_PlayIt - Play a pre-setup sound
//
//***************************************************************************

int SD_PlayIt ( int sndnum, int angle, int distance, int pitch )
{
   int voice;
   byte * snd;

#if (DEVELOPMENT == 1)
#if (SOUNDTEST == 1)
   SoftError("SOUND =%d \n",sndnum);
#endif
#endif

   if (!(sounds[sndnum].flags & SD_WRITE))
      {
      if (sounds[sndnum].count)
         {
         if (distance<=sounds[sndnum].prevdistance)
            FX_StopSound(sounds[sndnum].prevhandle);
         else
            return 0;
         }
      }

   if ( !FX_VoiceAvailable( sounds[sndnum].priority ) )
      {
      return( 0 );
      }

   sounds[sndnum].count++;

   snd=W_CacheLumpNum(SoundNumber(sndnum),PU_STATIC, CvtNull, 1);

   if ( *snd == 'C' )
      {
      voice = FX_PlayVOC3D( snd, pitch, angle, distance,
         sounds[sndnum].priority, (unsigned long) sndnum );
      }
   else
      {
      voice = FX_PlayWAV3D( snd, pitch, angle, distance,
         sounds[sndnum].priority, (unsigned long) sndnum );
      }

   if ( voice < FX_Ok )
      {
#if (DEVELOPMENT == 1)
/*
      if (MV_ErrorCode == MV_InvalidVOCFile)
         {
         Error("SD_PlayIt: Invalid VOC File snd=%p sndnum=%ld lump=%ld\n",snd,sndnum,SoundNumber(sndnum));
         }
*/
      NumBadSounds++;
      SoftError("SD_PlayIt: Error/Warning %s\n",FX_ErrorString( FX_Error ));
      SoftError("BadSoundNumber %ld time %ld\n",NumBadSounds,GetTicCount());
#endif
      SD_MakeCacheable( sndnum );

      return 0;
      }

   NumBadSounds=0;

   if (!(sounds[sndnum].flags & SD_WRITE))
      {
      sounds[sndnum].prevhandle=voice;
      sounds[sndnum].prevdistance=distance;
      }
   return voice;
}


//***************************************************************************
//
// SD_Play - Play a sample
//
//***************************************************************************

int SD_Play ( int sndnum )
{
   int voice;
   int pitch;

   if ( SD_SoundOkay ( sndnum ) == false )
      return 0;

   pitch = 0;

   if ( !( sounds[ sndnum ].flags & SD_PITCHSHIFTOFF ) )
      {
      pitch = PitchOffset();
      }

   voice = SD_PlayIt ( sndnum, 0, 0, pitch );

   return voice;

}

//***************************************************************************
//
// SD_Play3D - Play a positioned sample
//
//***************************************************************************

int SD_Play3D ( int sndnum, int angle, int distance )
{
   int voice;
   int pitch;

   if ( SD_SoundOkay ( sndnum ) == false )
      return 0;

   pitch = 0;
   if ( !( sounds[ sndnum ].flags & SD_PITCHSHIFTOFF ) )
      {
      pitch = PitchOffset();
      }

   voice = SD_PlayIt ( sndnum, angle, distance, pitch );

   return voice;

}

//***************************************************************************
//
// SD_PlayPositionedSound - Play a positioned sample
//
//***************************************************************************

int SD_PlayPositionedSound ( int sndnum, int px, int py, int x, int y )
{
   int voice;
   int angle;
   int distance;
   int dx;
   int dy;
   int pitch;

   if ( SD_SoundOkay ( sndnum ) == false )
      return 0;

   dx=(x-px);
   dy=(py-y);

   distance=FindDistance(dx,dy) >> SD_DISTANCESHIFT;

   if (distance>255)
      return 0;

   if (distance!=0)
      {
      angle = ( atan2_appx(dx,dy) & (FINEANGLES-1) ) >> 6;
      }
   else
      {
      angle=0;
      }

   pitch = 0;

   if ( !( sounds[ sndnum ].flags & SD_PITCHSHIFTOFF ) )
      {
      pitch = PitchOffset();
      }

   voice = SD_PlayIt ( sndnum, angle, distance, pitch );

   return voice;

}

//***************************************************************************
//
// SD_PlaySoundRTP - Play a positioned sample relative to the player
//
//***************************************************************************

int SD_PlaySoundRTP ( int sndnum, int x, int y )
{
   int voice;
   int angle;
   int distance;
   int dx;
   int dy;
   int pitch;


   if ( SD_SoundOkay ( sndnum ) == false )
      return 0;

   dx=(x-player->x);
   dy=(player->y-y);

   distance=FindDistance(dx,dy) >> SD_DISTANCESHIFT;

   if (distance>255)
      return 0;

   if (distance!=0)
      {
      angle = ( (player->angle - atan2_appx(dx,dy)) & (FINEANGLES-1) ) >> 6;
      }
   else
      {
      angle=0;
      }

   pitch = 0;

   if ( !( sounds[ sndnum ].flags & SD_PITCHSHIFTOFF ) )
      {
      pitch = PitchOffset();
      }

   voice = SD_PlayIt ( sndnum, angle, distance, pitch );

   return voice;
}

//***************************************************************************
//
// SD_PlayPitchedSound - Play a pitched sample
//
//***************************************************************************

int SD_PlayPitchedSound ( int sndnum, int volume, int pitch )
{
   int voice;
   int distance;

   if ( SD_SoundOkay ( sndnum ) == false )
      return 0;

   distance = 255 - volume;

   voice = SD_PlayIt ( sndnum, 0, distance, pitch );

   return voice;
}

//***************************************************************************
//
// SD_SetSoundPitch - sets the pitch of a sound
//
//***************************************************************************

void SD_SetSoundPitch ( int sndnum, int pitch )
{
   int status;

   if (SD_Started==false)
      return;

   if (!FX_SoundActive(sndnum))
      return;

   status=FX_SetPitch( sndnum, pitch );
   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_SetSoundPitch : %s\n",FX_ErrorString( status ));
#endif
      }
}

//***************************************************************************
//
// SD_PanRTP Sound - pan a positioned sample relative to the player
//
//***************************************************************************

void SD_PanRTP ( int handle, int x, int y )
{
   int angle;
   int distance;
   int dx;
   int dy;
   int status;

   if (SD_Started==false)
      return;

   if (!FX_SoundActive(handle))
      return;

   dx=(x-player->x);
   dy=(player->y-y);

   distance=FindDistance(dx,dy) >> SD_DISTANCESHIFT;

   if (distance>255)
      return;

   if (distance!=0)
      {
      angle = ( (player->angle - atan2_appx(dx,dy)) & (FINEANGLES-1) ) >> 6;
      }
   else
      {
      angle = 0;
      }

   status = FX_Pan3D ( handle, angle, distance );

   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_PanPositionedSound: %s\n",FX_ErrorString( status ));
#endif
      }
}

//***************************************************************************
//
// SD_SetPan - set the pan of a sample
//
//***************************************************************************

void SD_SetPan ( int handle, int vol, int left, int right )
{
   int status;

   if (SD_Started==false)
      return;

   if (!FX_SoundActive(handle))
      return;

   status=FX_SetPan( handle, vol, left, right );

   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_SetPan: %s\n",FX_ErrorString( status ));
#endif
      }
}

//***************************************************************************
//
// SD_PanPositioned Sound - pan a positioned sample
//
//***************************************************************************

void SD_PanPositionedSound ( int handle, int px, int py, int x, int y )
{
   int angle;
   int distance;
   int dx;
   int dy;
   int status;

   if (SD_Started==false)
      return;

   if (!FX_SoundActive(handle))
      return;

   dx=(x-px);
   dy=(py-y);

   distance=FindDistance(dx,dy) >> SD_DISTANCESHIFT;

   if (distance>255)
      return;

   if (distance!=0)
      {
      angle = ( atan2_appx(dx,dy) & (FINEANGLES-1) ) >> 6;
      }
   else
      {
      angle = 0;
      }

   status=FX_Pan3D( handle, angle, distance );

   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_PanPositionedSound: %s\n",FX_ErrorString( status ));
#endif
      }
}


//***************************************************************************
//
// SD_StopSound - Stop the current sound from playing
//
//***************************************************************************

void SD_StopSound ( int handle )
{
   int status;

   if (SD_Started==false)
      return;

   status=FX_StopSound( handle);

   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_StopSound: %s\n",FX_ErrorString( status ));
#endif
      }
}

//***************************************************************************
//
// SD_StopAllSounds - Stop All the sounds currently playing
//
//***************************************************************************

void  SD_StopAllSounds ( void )
{
   int status;

   if (SD_Started==false)
      return;

   status=FX_StopAllSounds();

   if (status != FX_Ok)
      {
#if (DEVELOPMENT == 1)
      SoftError("SD_StopAllSounds: %s\n",FX_ErrorString( status ));
#endif
      }
}

//***************************************************************************
//
// SD_SoundActive - See if a sound is active
//
//***************************************************************************

int SD_SoundActive ( int handle )
{
   if (SD_Started==false)
      {
      return false;
      }
   else
      {
      return (FX_SoundActive(handle));
      }
}

//***************************************************************************
//
// SD_WaitSound - wait until a sound has finished
//
//***************************************************************************
void SD_WaitSound ( int handle )
{
   int time;

   IN_ClearKeysDown();

   while (FX_SoundActive(handle)!=0)
      {
      time=GetTicCount()+1;
      while (time>GetTicCount()) {}
      if ((LastScan) || IN_GetMouseButtons())
         break;
      }
}


//***************************************************************************
//
// SD_Shutdown - Shutdown the sound system
//
//***************************************************************************

void SD_Shutdown (void)
{
   if (SD_Started==false)
      return;

   FX_Shutdown();
   SD_Started=false;
}


//***************************************************************************
//
// SD_PreCacheSound - PreCache sound
//
//***************************************************************************

void SD_PreCacheSound ( int num )
{
   if ( SD_SoundOkay ( num ) == false )
      return;

   PreCacheLump(SoundNumber(num),PU_CACHESOUNDS+sounds[num].priority,cache_other);
}

//***************************************************************************
//
// SD_PreCacheSoundGroup - PreCache sound group
//
//***************************************************************************

void SD_PreCacheSoundGroup ( int lo, int hi )
{
   int i;

   if (SD_Started==false)
      return;

   for (i=lo;i<=hi;i++)
      SD_PreCacheSound(i);
}


#if (SHAREWARE == 1)
#define MAXSONGS 18
static song_t rottsongs[MAXSONGS] = {
      { loop_no,  song_apogee  ,"FANFARE2","Apogee Fanfare"},
      { loop_yes, song_title   ,"RISE",    "Rise"},
      { loop_yes, song_menu    ,"MMMENU",  "MMMenu"},
      { loop_yes, song_christmas,"DEADLY", "Deadly Gentlemen"},
      { loop_yes, song_elevator,"GOINGUP", "Going up?"},
      { loop_yes, song_endlevel,"HOWDIDO", "How'd I do?"},
      { loop_yes, song_secretmenu,"FISHPOLK","Fish Polka"},
      { loop_yes, song_gameover,"YOUSUCK", "You Suck"},
      { loop_yes, song_youwin  ,"WATZNEXT","Watz Next?"},
      { loop_no,  song_gason   ,"GAZZ!",   "Gazz!"},
      { loop_yes, song_level   ,"FASTWAY", "Goin' Down The Fast Way"},
      { loop_yes, song_level   ,"MISTACHE","Mist Ache"},
      { loop_yes, song_level   ,"OWW",     "Oww!!!"},
      { loop_yes, song_level   ,"SMOKE",   "Smoke And Mirrors"},
      { loop_yes, song_level   ,"SPRAY",   "Spray"},
      { loop_yes, song_level   ,"RUNLIKE", "Run Like Smeg"},
      { loop_yes, song_level   ,"SMOOTH",  "Havana Smooth"},
      { loop_yes, song_level   ,"CHANT",   "Chant"},
      };
#else
#define MAXSONGS 34
static song_t rottsongs[MAXSONGS] = {
      { loop_no,  song_apogee  ,"FANFARE2","Apogee Fanfare"},
      { loop_yes, song_title   ,"RISE",    "Rise"},
      { loop_yes, song_menu    ,"MMMENU",  "MMMenu"},
      { loop_yes, song_christmas,"DEADLY", "Deadly Gentlemen"},
      { loop_yes, song_elevator,"GOINGUP", "Going up?"},
      { loop_yes, song_secretmenu,"FISHPOLK","Fish Polka"},
      { loop_yes, song_endlevel,"HOWDIDO", "How'd I do?"},
      { loop_yes, song_gameover,"YOUSUCK", "You Suck"},
      { loop_yes, song_cinematic2,"WATZNEXT","Watz Next?"},
      { loop_no,  song_gason   ,"GAZZ!",   "Gazz!"},
      { loop_yes, song_level   ,"FASTWAY", "Goin' Down The Fast Way"},
      { loop_yes, song_level   ,"MISTACHE","Mist Ache"},
      { loop_yes, song_level   ,"OWW",     "Oww!!!"},
      { loop_yes, song_level   ,"SMOKE",   "Smoke And Mirrors"},
      { loop_yes, song_level   ,"SPRAY",   "Spray"},
      { loop_yes, song_level   ,"RUNLIKE", "Run Like Smeg"},
      { loop_yes, song_level   ,"SMOOTH",  "Havana Smooth"},
      { loop_yes, song_level   ,"CHANT",   "Chant"},
      { loop_yes, song_level   ,"MEDIEV1A","Funeral of Queen Mary"},
      { loop_yes, song_level   ,"TASKFORC","Task Force"},
      { loop_yes, song_level   ,"KISSOFF", "KISS Off"},
      { loop_yes, song_level   ,"RADAGIO", "Adagio For Strings"},
      { loop_yes, song_level   ,"SHARDS",  "Shards"},
      { loop_yes, song_level   ,"STAIRS",  "I Choose the Stairs"},
      { loop_yes, song_level   ,"SUCKTHIS","Suck This"},
      { loop_yes, song_level   ,"EXCALIBR","Excalibur"},
      { loop_yes, song_level   ,"CCCOOL",   "CCCool"},
      { loop_yes, song_level   ,"WORK_DAY","Work Day"},
      { loop_yes, song_level   ,"WHERIZIT","Where Iz It?"},
      { loop_no,  song_bossdie,"BOSSBLOW", "Boss Blow"},
      { loop_yes, song_bosssee ,"HELLERO", "Hellero"},
      { loop_yes, song_cinematic1,"EVINRUDE","Evin Rude"},
      { loop_yes, song_youwin  ,"VICTORY", "Victory!"},
      { loop_yes, song_dogend  ,"HERE_BOY","Here Boy"}
      };
#endif

static byte * currentsong;
static int MU_Started=false;
static int lastsongnumber=-1;
int storedposition=0;

//****************************************************************************
//
// MU_JukeBoxMenu()
//
//****************************************************************************

void MU_PlayJukeBoxSong
   (
   int which
   )

   {
   if ( ( MusicMode > 0 ) && ( MU_Started == true ) )
      {
      SetMenuHeader( rottsongs[ which ].songname );
      MU_PlaySong( which );
      }
   }


//****************************************************************************
//
// MU_JukeBoxMenu()
//
//****************************************************************************

void MU_JukeBoxRedraw
   (
   void
   )

   {
   if ( ( MusicMode > 0 ) && ( MU_Started == true ) )
      {
      SetMenuHeader( rottsongs[ lastsongnumber ].songname );
      }
   }


//****************************************************************************
//
// MU_JukeBoxMenu()
//
//****************************************************************************

void MU_JukeBoxMenu
   (
   void
   )

   {
   char *SongNames[ MAXSONGS ];
   int   i;

   for( i = 0; i < MAXSONGS; i++ )
      {
      SongNames[ i ] = rottsongs[ i ].songname;
      }

   HandleMultiPageCustomMenu( SongNames, MAXSONGS, lastsongnumber,
      "Jukebox", MU_PlayJukeBoxSong, MU_JukeBoxRedraw, false );

   if ( rottsongs[ lastsongnumber ].loopflag == loop_no )
      {
      MU_StartSong(song_level);
      }
   }

//***************************************************************************
//
// MusicStarted - see if the music is started
//
//***************************************************************************
boolean MusicStarted( void )
{
   return MU_Started;
}

//***************************************************************************
//
// MU_Startup - Initialize music stuff
//
//***************************************************************************

int MU_Startup ( boolean bombonerror )
{
   int status;
   int card;

   if (MU_Started==true)
      {
      MU_StopSong();
      MU_Shutdown();
      }
   if ( ( MusicMode < 0 ) || ( MusicMode >= 11 ) )
      {
      return( 0 );
      }
   card = musicnums[ MusicMode ];
   if (card==-1) // Check if it is off
      return (0);

#ifdef DOS
   if ( ( card == SoundBlaster ) || ( card == Awe32 ) || ( card == WaveBlaster ) )
      {
      if ( SD_Started == false )
         {
         extern fx_blaster_config SBSettings;
         int numvoices;
         int numbits;
         int numchannels;

         FX_SetupSoundBlaster( SBSettings, &numvoices,
            &numbits, &numchannels );
         }
      }

   if (card== UltraSound)
      {
      MU_SetupGUSInitFile();
      }

   status=MUSIC_Init( card, MidiAddress );
#else
   /* Not DOS, no address config needed */
   status=MUSIC_Init( card, 0 );
#endif


   if (status != MUSIC_Ok) {
      if (bombonerror)
         {
         DeleteSoundFile ();
         Error( "%s\n", MUSIC_ErrorString( status ) );
         }
      else
         return (status);
   }

   currentsong=0;

   MU_Started=true;

   MU_SetVolume (MUvolume);

   return (0);
}

//***************************************************************************
//
// MU_Shutdown - Shutdown the music system
//
//***************************************************************************

void MU_Shutdown (void)
{
   if (MU_Started==false)
      return;
   MUSIC_Shutdown();
   MU_Started=false;
}

#ifdef DOS
//***************************************************************************
//
// MU_SetupGUSInitFile - initialize GUS stuff
//
//***************************************************************************

void MU_SetupGUSInitFile( void )
{
   char filename[ 128 ];

   GetPathFromEnvironment( filename, ApogeePath, GUSMIDIINIFILE );
   if (access (filename, F_OK) != 0)
      {
      int lump;

      lump=W_GetNumForName("gusmidi");

      SaveFile (filename, W_CacheLumpNum(lump,PU_CACHE, CvtNull, 1), W_LumpLength(lump));
      }
}

#endif

//***************************************************************************
//
// MU_GetNumForType - returns number of song in rottsongs of specific type
//
//***************************************************************************
int MU_GetNumForType ( int type )
{
   int i;

   for (i=0;i<MAXSONGS;i++)
      {
      if (rottsongs[i].songtype == type)
         return i;
      }
   Error("MU_GetNumForType: could not find song type in list\n");
   return -1;
}


//***************************************************************************
//
// MU_PlaySong - Play a specific song number
//
//***************************************************************************

void MU_PlaySong ( int num )
{
   int lump;
   int size;
   
   if (MU_Started==false)
      return;

   if (num<0)
      return;

   if (num>=MAXSONGS)
      Error("Song number out of range\n");

   MU_StopSong();

   lastsongnumber=num;

   lump = W_GetNumForName(rottsongs[num].lumpname);
   size = W_LumpLength(lump);

   currentsong=W_CacheLumpNum(lump,PU_STATIC, CvtNull, 1);

#ifdef PLATFORM_DOS
   if (rottsongs[num].loopflag == loop_yes)
      MUSIC_PlaySong(currentsong,size,MUSIC_LoopSong);
   else
      MUSIC_PlaySong(currentsong,size,MUSIC_PlayOnce);
#else 
   if (rottsongs[num].loopflag == loop_yes)
      MUSIC_PlaySongROTT(currentsong,size,MUSIC_LoopSong);
   else
      MUSIC_PlaySongROTT(currentsong,size,MUSIC_PlayOnce);
#endif

   MU_SetVolume (MUvolume);
}

//***************************************************************************
//
// MU_StopSong - Play a specific song number
//
//***************************************************************************

void MU_StopSong ( void )
{
   if (MU_Started==false)
      return;

   MUSIC_StopSong ();
   if (currentsong)
      {
      W_CacheLumpName(rottsongs[lastsongnumber].lumpname,PU_CACHE, CvtNull, 1);
      currentsong=0;
      }
}

//***************************************************************************
//
// MU_GetSongNumber - get current song number
//
//***************************************************************************

int MU_GetSongNumber ( void )
{
   return lastsongnumber;
}


//***************************************************************************
//
// MU_FadeToSong - Fade to a specific song in a certain time
//
//***************************************************************************

void MU_FadeToSong ( int num, int time )
{
   int t;

   if (MU_Started==false)
      return;

   MU_FadeOut(time>>1);

   while (MU_FadeActive())
      {
      t=GetTicCount();
      while (GetTicCount()==t) {}
      }

   MU_FadeIn (num,time>>1);
}

//***************************************************************************
//
// MU_FadeIn - Fade in
//
//***************************************************************************

void MU_FadeIn ( int num, int time )
{
   if (MU_Started==false)
      return;

   MUSIC_SetVolume(0);
   MU_PlaySong ( num );
   MUSIC_FadeVolume (MUvolume, time);
}

//***************************************************************************
//
// MU_FadeOut - Fade out
//
//***************************************************************************

void MU_FadeOut ( int time )
{
   if (MU_Started==false)
      return;
   if (!MUSIC_SongPlaying())
      {
#if (DEVELOPMENT == 1)
      SoftError("Called FadeOut with no song playing\n");
#endif
      return;
      }
   MUSIC_FadeVolume(0,time);
}


//***************************************************************************
//
// MU_StartSong - Start a context sensitive song
//
//***************************************************************************

void MU_StartSong ( int songtype )
{
   int songnum;

   if (MU_Started==false)
      return;

   MU_StopSong();

   songnum = MU_GetNumForType ( songtype );
   switch (songtype)
      {
      case song_level:
         if (IsChristmas())
            {
            songnum = MU_GetNumForType ( song_christmas );
            }
         else
            {
            songnum += GetSongForLevel ();
            }
         break;
      }
   MU_PlaySong (songnum);
}

//***************************************************************************
//
// MU_StoreSongPostition - Save off Song Position
//
//***************************************************************************

void MU_StoreSongPosition ( void )
{
   if (MU_Started==false)
      return;
   PositionStored=true;
   storedposition=MUSIC_GetPosition();
}

//***************************************************************************
//
// MU_RestoreSongPostition - Save off Song Position
//
//***************************************************************************

void MU_RestoreSongPosition ( void )
{
   if (MU_Started==false)
      return;
   if (PositionStored==false)
      return;
   PositionStored=false;

   MUSIC_SetPosition(storedposition);
}

//***************************************************************************
//
// MU_GetStoredPostition - Get Stored song Position
//
//***************************************************************************

int MU_GetStoredPosition ( void )
{
   if (PositionStored)
      return storedposition;
   else
      return -1;
}

//***************************************************************************
//
// MU_SetStoredPostition - Get Stored song Position
//
//***************************************************************************

void MU_SetStoredPosition ( int position )
{
   if (MU_Started==false)
      return;
   if (position==-1)
      return;
   PositionStored=true;
   storedposition=position;
}



//***************************************************************************
//
// MU_GetSongPostition - Get Song Position
//
//***************************************************************************

int MU_GetSongPosition ( void )	
{
   if (MU_Started==false)
      return 0;
   return MUSIC_GetPosition();
}

//***************************************************************************
//
// MU_SetSongPostition - Set Song Position
//
//***************************************************************************

void MU_SetSongPosition ( int position )
{
   if (MU_Started==false)
      return;
   MUSIC_SetPosition(position);
}

//***************************************************************************
//
// MU_SaveMusic
//
//***************************************************************************

void MU_SaveMusic (byte ** buf, int * size)
{
   int unitsize;
   byte *ptr;
   int vsize;
   int i;

   //
   // Size
   //

   unitsize=0;

	unitsize+=sizeof(i);
	unitsize+=sizeof(i);
	unitsize+=sizeof(i);


   *size = unitsize;
   *buf = (byte *) SafeMalloc (*size);

   ptr = *buf;

   i=MU_GetSongNumber();
   if (rottsongs[i].songtype == song_menu)
      {
      i = MU_GetNumForType ( song_level );
      if (IsChristmas())
         {
         i = MU_GetNumForType ( song_christmas );
         }
      else
         {
         i += GetSongForLevel ();
         }
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;

      i=MU_GetStoredPosition();
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;

      i=-1;
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;
      }
   else
      {
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;

      i=MU_GetSongPosition();
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;

      i=MU_GetStoredPosition();
      vsize=sizeof(i);
      memcpy(ptr,&i,vsize);
      ptr+=vsize;
      }
}


//***************************************************************************
//
// MU_LoadMusic
//
//***************************************************************************

void MU_LoadMusic (byte * buf, int size)
{
   int unitsize;
   byte *ptr;
   int i;
   int songnumber;
   boolean differentsong=false;
   int vsize;

   //
   // Size
   //

   unitsize=0;

	unitsize+=sizeof(i);
	unitsize+=sizeof(i);
	unitsize+=sizeof(i);

   if (size!=unitsize)
      Error("LoadMusic: Different number of parameters\n");

   ptr  = buf;

   vsize=sizeof(songnumber);
   memcpy(&songnumber,ptr,vsize);
   ptr+=vsize;
   if (MU_GetSongNumber () != songnumber)
      {
      MU_PlaySong(songnumber);
      differentsong=true;
      }

   vsize=sizeof(i);
   memcpy(&i,ptr,vsize);
   ptr+=vsize;
   if (differentsong==true)
      {
      MU_SetSongPosition(i);
      }

   vsize=sizeof(i);
   memcpy(&i,ptr,vsize);
   ptr+=vsize;
   MU_SetStoredPosition(i);

   // Check if game was saved with NOSOUND

   if (MU_GetSongNumber () != songnumber)
      {
      MU_StartSong ( song_level );
      }
}
