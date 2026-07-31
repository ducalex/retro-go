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
   module: FX_MAN.C

   author: James R. Dose
   date:   March 17, 1994

   Device independant sound effect routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include "rt_def.h"
#include <stdio.h>
#include <stdlib.h>
#include "sndcards.h"
#include "multivoc.h"

#ifdef PLAT_DOS
#include "blaster.h"
#include "pas16.h"
#include "sndscape.h"
#include "guswave.h"
#include "sndsrc.h"
#else
#include "dsl.h"
#endif

#include "ll_man.h"
#include "user.h"
#include "fx_man.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

static unsigned FX_MixRate;

int FX_SoundDevice = -1;
int FX_ErrorCode = FX_Ok;
int FX_Installed = FALSE;

#define FX_SetErrorCode( status ) \
   FX_ErrorCode = ( status );

/*---------------------------------------------------------------------
   Function: FX_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *FX_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case FX_Warning :
      case FX_Error :
         ErrorString = FX_ErrorString( FX_ErrorCode );
         break;

      case FX_Ok :
         ErrorString = "Fx ok.";
         break;

      case FX_ASSVersion :
         ErrorString = "Apogee Sound System Version " ASS_VERSION_STRING "  "
            "Programmed by Jim Dose\n"
            "(c) Copyright 1995 James R. Dose.  All Rights Reserved.\n";
         break;

#ifdef PLAT_DOS
      case FX_BlasterError :
         ErrorString = BLASTER_ErrorString( BLASTER_Error );
         break;
#endif

      case FX_SoundCardError :
#ifdef PLAT_DOS
         switch( FX_SoundDevice )
         {
            case SoundBlaster :
            case Awe32 :
               ErrorString = BLASTER_ErrorString( BLASTER_Error );
               break;

            case ProAudioSpectrum :
            case SoundMan16 :
               ErrorString = PAS_ErrorString( PAS_Error );
               break;

            case SoundScape :
               ErrorString = SOUNDSCAPE_ErrorString( SOUNDSCAPE_Error );
               break;

            case UltraSound :
               ErrorString = GUSWAVE_ErrorString( GUSWAVE_Error );
               break;

            case SoundSource :
            case TandySoundSource :
               ErrorString = SS_ErrorString( SS_Error );
               break;
            }
#else
         ErrorString = DSL_ErrorString( DSL_Error );
#endif
         break;

      case FX_InvalidCard :
         ErrorString = "Invalid Sound Fx device.";
         break;

      case FX_MultiVocError :
         ErrorString = MV_ErrorString( MV_Error );
         break;

      case FX_DPMI_Error :
         ErrorString = "DPMI Error in FX_MAN.";
         break;

      default :
         ErrorString = "Unknown Fx error code.";
         break;
      }

   return( ErrorString );
   }


/*---------------------------------------------------------------------
   Function: FX_SetupCard

   Sets the configuration of a sound device.
---------------------------------------------------------------------*/

int FX_SetupCard
   (
   int SoundCard,
   fx_device *device
   )

   {
   int status;
   int DeviceStatus;

   if ( USER_CheckParameter( "ASSVER" ) )
      {
      FX_SetErrorCode( FX_ASSVersion );
      return( FX_Error );
      }

   FX_SoundDevice = SoundCard;

   status = FX_Ok;
   FX_SetErrorCode( FX_Ok );

#ifdef PLAT_DOS
   switch( SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
         DeviceStatus = BLASTER_Init();
         if ( DeviceStatus != BLASTER_Ok )
            {
            FX_SetErrorCode( FX_SoundCardError );
            status = FX_Error;
            break;
            }

         device->MaxVoices = 32;
         BLASTER_GetCardInfo( &device->MaxSampleBits, &device->MaxChannels );
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         DeviceStatus = PAS_Init();
         if ( DeviceStatus != PAS_Ok )
            {
            FX_SetErrorCode( FX_SoundCardError );
            status = FX_Error;
            break;
            }

         device->MaxVoices = 32;
         PAS_GetCardInfo( &device->MaxSampleBits, &device->MaxChannels );
         break;

      case GenMidi :
      case SoundCanvas :
      case WaveBlaster :
         device->MaxVoices     = 0;
         device->MaxSampleBits = 0;
         device->MaxChannels   = 0;
         break;

      case SoundScape :
         device->MaxVoices = 32;
         DeviceStatus = SOUNDSCAPE_GetCardInfo( &device->MaxSampleBits,
            &device->MaxChannels );
         if ( DeviceStatus != SOUNDSCAPE_Ok )
            {
            FX_SetErrorCode( FX_SoundCardError );
            status = FX_Error;
            }
         break;

      case UltraSound :
         if ( GUSWAVE_Init( 8 ) != GUSWAVE_Ok )
            {
            FX_SetErrorCode( FX_SoundCardError );
            status = FX_Error;
            break;
            }

         device->MaxVoices     = 8;
         device->MaxSampleBits = 0;
         device->MaxChannels   = 0;
         break;

      case SoundSource :
      case TandySoundSource :
         DeviceStatus = SS_Init( SoundCard );
         if ( DeviceStatus != SS_Ok )
            {
            FX_SetErrorCode( FX_SoundCardError );
            status = FX_Error;
            break;
            }
         SS_Shutdown();
         device->MaxVoices     = 32;
         device->MaxSampleBits = 8;
         device->MaxChannels   = 1;
         break;
      default :
         FX_SetErrorCode( FX_InvalidCard );
         status = FX_Error;
      }
#else
      DeviceStatus = DSL_Init();
      if ( DeviceStatus != DSL_Ok )
         {
         FX_SetErrorCode( FX_SoundCardError );
         status = FX_Error;
         }
         else
         {
         device->MaxVoices     = 32;
         device->MaxSampleBits = 0;
         device->MaxChannels   = 0;
         }
#endif

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_GetBlasterSettings

   Returns the current BLASTER environment variable settings.
---------------------------------------------------------------------*/

int FX_GetBlasterSettings
   (
   fx_blaster_config *blaster
   )

   {
#ifdef PLAT_DOS
   int status;
   BLASTER_CONFIG Blaster;

   FX_SetErrorCode( FX_Ok );

   status = BLASTER_GetEnv( &Blaster );
   if ( status != BLASTER_Ok )
      {
      FX_SetErrorCode( FX_BlasterError );
      return( FX_Error );
      }

   blaster->Type      = Blaster.Type;
   blaster->Address   = Blaster.Address;
   blaster->Interrupt = Blaster.Interrupt;
   blaster->Dma8      = Blaster.Dma8;
   blaster->Dma16     = Blaster.Dma16;
   blaster->Midi      = Blaster.Midi;
   blaster->Emu       = Blaster.Emu;
#endif

   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_SetupSoundBlaster

   Handles manual setup of the Sound Blaster information.
---------------------------------------------------------------------*/

int FX_SetupSoundBlaster
   (
   fx_blaster_config blaster,
   int *MaxVoices,
   int *MaxSampleBits,
   int *MaxChannels
   )

   {
#ifdef PLAT_DOS
   int DeviceStatus;
   BLASTER_CONFIG Blaster;

   FX_SetErrorCode( FX_Ok );

   FX_SoundDevice = SoundBlaster;

   Blaster.Type      = blaster.Type;
   Blaster.Address   = blaster.Address;
   Blaster.Interrupt = blaster.Interrupt;
   Blaster.Dma8      = blaster.Dma8;
   Blaster.Dma16     = blaster.Dma16;
   Blaster.Midi      = blaster.Midi;
   Blaster.Emu       = blaster.Emu;

   BLASTER_SetCardSettings( Blaster );

   DeviceStatus = BLASTER_Init();
   if ( DeviceStatus != BLASTER_Ok )
      {
      FX_SetErrorCode( FX_SoundCardError );
      return( FX_Error );
      }

   *MaxVoices = 8;
   BLASTER_GetCardInfo( MaxSampleBits, MaxChannels );
#endif

   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_Init

   Selects which sound device to use.
---------------------------------------------------------------------*/

int FX_Init
   (
   int SoundCard,
   int numvoices,
   int numchannels,
   int samplebits,
   unsigned mixrate
   )

   {
   int status;
   int devicestatus;

   if ( FX_Installed )
      {
      FX_Shutdown();
      }

   if ( USER_CheckParameter( "ASSVER" ) )
      {
      FX_SetErrorCode( FX_ASSVersion );
      return( FX_Error );
      }

   status = LL_LockMemory();
   if ( status != LL_Ok )
      {
      FX_SetErrorCode( FX_DPMI_Error );
      return( FX_Error );
      }

   FX_MixRate = mixrate;

   status = FX_Ok;
   FX_SoundDevice = SoundCard;
   switch( SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
      case SoundScape :
      case SoundSource :
      case TandySoundSource :
      case UltraSound :
         devicestatus = MV_Init( SoundCard, FX_MixRate, numvoices,
            numchannels, samplebits );
         if ( devicestatus != MV_Ok )
            {
            FX_SetErrorCode( FX_MultiVocError );
            status = FX_Error;
            }
         break;

      default :
         FX_SetErrorCode( FX_InvalidCard );
         status = FX_Error;
      }

   if ( status != FX_Ok )
      {
      LL_UnlockMemory();
      }
   else
      {
      FX_Installed = TRUE;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_Shutdown

   Terminates use of sound device.
---------------------------------------------------------------------*/

int FX_Shutdown
   (
   void
   )

   {
   int status;

   if ( !FX_Installed )
      {
      return( FX_Ok );
      }

   status = FX_Ok;
   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
      case SoundScape :
      case SoundSource :
      case TandySoundSource :
      case UltraSound :
         status = MV_Shutdown();
         if ( status != MV_Ok )
            {
            FX_SetErrorCode( FX_MultiVocError );
            status = FX_Error;
            }
         break;

      default :
         FX_SetErrorCode( FX_InvalidCard );
         status = FX_Error;
      }

   FX_Installed = FALSE;
   LL_UnlockMemory();

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_SetCallback

   Sets the function to call when a voice is done.
---------------------------------------------------------------------*/

int FX_SetCallBack
   (
   void ( *function )( unsigned long )
   )

   {
   int status;

   status = FX_Ok;

   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
      case SoundScape :
      case SoundSource :
      case TandySoundSource :
      case UltraSound :
         MV_SetCallBack( function );
         break;

      default :
         FX_SetErrorCode( FX_InvalidCard );
         status = FX_Error;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_SetVolume

   Sets the volume of the current sound device.
---------------------------------------------------------------------*/

void FX_SetVolume
   (
   int volume
   )

   {
   int status;

#ifdef PLAT_DOS
   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
         if ( BLASTER_CardHasMixer() )
            {
            BLASTER_SetVoiceVolume( volume );
            }
         else
            {
            MV_SetVolume( volume );
            }
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         status = PAS_SetPCMVolume( volume );
         if ( status != PAS_Ok )
            {
            MV_SetVolume( volume );
            }
         break;

      case GenMidi :
      case SoundCanvas :
      case WaveBlaster :
         break;

      case SoundScape :
         MV_SetVolume( volume );
         break;

      case UltraSound :
         GUSWAVE_SetVolume( volume );
         break;

      case SoundSource :
      case TandySoundSource :
         MV_SetVolume( volume );
         break;
      }
#else
   MV_SetVolume( volume );
#endif
   }


/*---------------------------------------------------------------------
   Function: FX_GetVolume

   Returns the volume of the current sound device.
---------------------------------------------------------------------*/

int FX_GetVolume
   (
   void
   )

   {
   int volume;

#ifdef PLAT_DOS
   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
         if ( BLASTER_CardHasMixer() )
            {
            volume = BLASTER_GetVoiceVolume();
            }
         else
            {
            volume = MV_GetVolume();
            }
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         volume = PAS_GetPCMVolume();
         if ( volume == PAS_Error )
            {
            volume = MV_GetVolume();
            }
         break;

      case GenMidi :
      case SoundCanvas :
      case WaveBlaster :
         volume = 255;
         break;

      case SoundScape :
         volume = MV_GetVolume();
         break;

      case UltraSound :
         volume = GUSWAVE_GetVolume();
         break;

      case SoundSource :
      case TandySoundSource :
         volume = MV_GetVolume();
         break;

      default :
         volume = 0;
      }
#else
   volume = MV_GetVolume();
#endif

   return( volume );
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverseStereo

   Set the orientation of the left and right channels.
---------------------------------------------------------------------*/

void FX_SetReverseStereo
   (
   int setting
   )

   {
   MV_SetReverseStereo( setting );
   }


/*---------------------------------------------------------------------
   Function: FX_GetReverseStereo

   Returns the orientation of the left and right channels.
---------------------------------------------------------------------*/

int FX_GetReverseStereo
   (
   void
   )

   {
   return MV_GetReverseStereo();
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverb

   Sets the reverb level.
---------------------------------------------------------------------*/

void FX_SetReverb
   (
   int reverb
   )

   {
   MV_SetReverb( reverb );
   }


/*---------------------------------------------------------------------
   Function: FX_SetFastReverb

   Sets the reverb level.
---------------------------------------------------------------------*/

void FX_SetFastReverb
   (
   int reverb
   )

   {
   MV_SetFastReverb( reverb );
   }


/*---------------------------------------------------------------------
   Function: FX_GetMaxReverbDelay

   Returns the maximum delay time for reverb.
---------------------------------------------------------------------*/

int FX_GetMaxReverbDelay
   (
   void
   )

   {
   return MV_GetMaxReverbDelay();
   }


/*---------------------------------------------------------------------
   Function: FX_GetReverbDelay

   Returns the current delay time for reverb.
---------------------------------------------------------------------*/

int FX_GetReverbDelay
   (
   void
   )

   {
   return MV_GetReverbDelay();
   }


/*---------------------------------------------------------------------
   Function: FX_SetReverbDelay

   Sets the delay level of reverb to add to mix.
---------------------------------------------------------------------*/

void FX_SetReverbDelay
   (
   int delay
   )

   {
   MV_SetReverbDelay( delay );
   }


/*---------------------------------------------------------------------
   Function: FX_VoiceAvailable

   Checks if a voice can be play at the specified priority.
---------------------------------------------------------------------*/

int FX_VoiceAvailable
   (
   int priority
   )

   {
   return MV_VoiceAvailable( priority );
   }

/*---------------------------------------------------------------------
   Function: FX_EndLooping

   Stops the voice associated with the specified handle from looping
   without stoping the sound.
---------------------------------------------------------------------*/

int FX_EndLooping
   (
   int handle
   )

   {
   int status;

   status = MV_EndLooping( handle );
   if ( status == MV_Error )
      {
      FX_SetErrorCode( FX_MultiVocError );
      status = FX_Warning;
      }

   return( status );
   }

/*---------------------------------------------------------------------
   Function: FX_SetPan

   Sets the stereo and mono volume level of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int FX_SetPan
   (
   int handle,
   int vol,
   int left,
   int right
   )

   {
   int status;

   status = MV_SetPan( handle, vol, left, right );
   if ( status == MV_Error )
      {
      FX_SetErrorCode( FX_MultiVocError );
      status = FX_Warning;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_SetPitch

   Sets the pitch of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int FX_SetPitch
   (
   int handle,
   int pitchoffset
   )

   {
   int status;

   status = MV_SetPitch( handle, pitchoffset );
   if ( status == MV_Error )
      {
      FX_SetErrorCode( FX_MultiVocError );
      status = FX_Warning;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_SetFrequency

   Sets the frequency of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int FX_SetFrequency
   (
   int handle,
   int frequency
   )

   {
   int status;

   status = MV_SetFrequency( handle, frequency );
   if ( status == MV_Error )
      {
      FX_SetErrorCode( FX_MultiVocError );
      status = FX_Warning;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayVOC

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayVOC
   (
   char *ptr,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayVOC( ptr, pitchoffset, vol, left, right,
      priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayLoopedVOC

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedVOC
   (
   char *ptr,
   long loopstart,
   long loopend,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayLoopedVOC( ptr, loopstart, loopend, pitchoffset,
      vol, left, right, priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayWAV
   (
   char *ptr,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayWAV( ptr, pitchoffset, vol, left, right,
      priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV

   Begin playback of sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedWAV
   (
   char *ptr,
   long loopstart,
   long loopend,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayLoopedWAV( ptr, loopstart, loopend,
      pitchoffset, vol, left, right, priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayVOC3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int FX_PlayVOC3D
   (
   char *ptr,
   int pitchoffset,
   int angle,
   int distance,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayVOC3D( ptr, pitchoffset, angle, distance,
      priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayWAV3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int FX_PlayWAV3D
   (
   char *ptr,
   int pitchoffset,
   int angle,
   int distance,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayWAV3D( ptr, pitchoffset, angle, distance,
      priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayRaw

   Begin playback of raw sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayRaw
   (
   char *ptr,
   unsigned long length,
   unsigned rate,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayRaw( ptr, length, rate, pitchoffset,
      vol, left, right, priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_PlayLoopedRaw

   Begin playback of raw sound data with the given volume and priority.
---------------------------------------------------------------------*/

int FX_PlayLoopedRaw
   (
   char *ptr,
   unsigned long length,
   char *loopstart,
   char *loopend,
   unsigned rate,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_PlayLoopedRaw( ptr, length, loopstart, loopend,
      rate, pitchoffset, vol, left, right, priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_Pan3D

   Set the angle and distance from the listener of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int FX_Pan3D
   (
   int handle,
   int angle,
   int distance
   )

   {
   int status;

   status = MV_Pan3D( handle, angle, distance );
   if ( status != MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      status = FX_Warning;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_SoundActive

   Tests if the specified sound is currently playing.
---------------------------------------------------------------------*/

int FX_SoundActive
   (
   int handle
   )

   {
   return( MV_VoicePlaying( handle ) );
   }


/*---------------------------------------------------------------------
   Function: FX_SoundsPlaying

   Reports the number of voices playing.
---------------------------------------------------------------------*/

int FX_SoundsPlaying
   (
   void
   )

   {
   return( MV_VoicesPlaying() );
   }


/*---------------------------------------------------------------------
   Function: FX_StopSound

   Halts playback of a specific voice
---------------------------------------------------------------------*/

int FX_StopSound
   (
   int handle
   )

   {
   int status;

   status = MV_Kill( handle );
   if ( status != MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      return( FX_Warning );
      }

   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_StopAllSounds

   Halts playback of all sounds.
---------------------------------------------------------------------*/

int FX_StopAllSounds
   (
   void
   )

   {
   int status;

   status = MV_KillAllVoices();
   if ( status != MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      return( FX_Warning );
      }

   return( FX_Ok );
   }


/*---------------------------------------------------------------------
   Function: FX_StartDemandFeedPlayback

   Plays a digitized sound from a user controlled buffering system.
---------------------------------------------------------------------*/

int FX_StartDemandFeedPlayback
   (
   void ( *function )( char **ptr, unsigned long *length ),
   int rate,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   int handle;

   handle = MV_StartDemandFeedPlayback( function, rate,
      pitchoffset, vol, left, right, priority, callbackval );
   if ( handle < MV_Ok )
      {
      FX_SetErrorCode( FX_MultiVocError );
      handle = FX_Warning;
      }

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: FX_StartRecording

   Starts the sound recording engine.
---------------------------------------------------------------------*/

int FX_StartRecording
   (
   int MixRate,
   void ( *function )( char *ptr, int length )
   )

   {
   int status;

#ifdef PLAT_DOS
   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
         status = MV_StartRecording( MixRate, function );
         if ( status != MV_Ok )
            {
            FX_SetErrorCode( FX_MultiVocError );
            status = FX_Warning;
            }
         else
            {
            status = FX_Ok;
            }
         break;

      default :
         FX_SetErrorCode( FX_InvalidCard );
         status = FX_Warning;
         break;
      }
#else
   FX_SetErrorCode( FX_InvalidCard );
   status = FX_Warning;
#endif

   return( status );
   }


/*---------------------------------------------------------------------
   Function: FX_StopRecord

   Stops the sound record engine.
---------------------------------------------------------------------*/

void FX_StopRecord
   (
   void
   )

   {
#ifdef PLAT_DOS
   // Stop sound playback
   switch( FX_SoundDevice )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
         MV_StopRecord();
         break;
      }
#endif
   }
