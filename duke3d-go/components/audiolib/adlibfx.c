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
   module: ADLIBFX.C

   author: James R. Dose
   date:   April 1, 1994

   Low level routines to support Adlib sound effects created by Muse.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

//#include <dos.h>
#include <stdlib.h>
//#include <conio.h>
#include "dpmi.h"
#include "task_man.h"
#include "interrup.h"
#include "al_midi.h"
#include "adlibfx.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

static void ADLIBFX_SendOutput( int reg, int data );
static void ADLIBFX_Service( task *Task );

static long     ADLIBFX_LengthLeft;
static int      ADLIBFX_Block;
static ALSound *ADLIBFX_Sound = NULL;
static char    *ADLIBFX_SoundPtr = NULL;
static int      ADLIBFX_Priority;
static unsigned long ADLIBFX_CallBackVal;
static void     ( *ADLIBFX_CallBackFunc )( unsigned long ) = NULL;
static int      ADLIBFX_SoundVolume;
static int      ADLIBFX_TotalVolume = ADLIBFX_MaxVolume;
static task    *ADLIBFX_ServiceTask = NULL;
static int      ADLIBFX_VoiceHandle = ADLIBFX_MinVoiceHandle;

int ADLIBFX_Installed = FALSE;

int ADLIBFX_ErrorCode = ADLIBFX_Ok;

#define ADLIBFX_SetErrorCode( status ) \
   ADLIBFX_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: ADLIBFX_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *ADLIBFX_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case ADLIBFX_Warning :
      case ADLIBFX_Error :
         ErrorString = ADLIBFX_ErrorString( ADLIBFX_ErrorCode );
         break;

      case ADLIBFX_Ok :
         ErrorString = "Adlib FX ok.";
         break;

      case ADLIBFX_NoVoices :
         ErrorString = "No free voices available in Adlib FX.";
         break;

      case ADLIBFX_VoiceNotFound :
         ErrorString = "No voice with matching handle found.";
         break;

      case ADLIBFX_DPMI_Error :
         ErrorString = "DPMI Error in AdlibFX.";
         break;

      default :
         ErrorString = "Unknown Adlib FX error code.";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define ADLIBFX_LockStart ADLIBFX_SendOutput

/*---------------------------------------------------------------------
   Function: ADLIBFX_SendOutput

   Writes a byte of data to the specified register on the Adlib card.
---------------------------------------------------------------------*/

static void ADLIBFX_SendOutput
   (
   int reg,
   int data
   )

   {
   int i;
   int adlib_port = 0x388;
   unsigned flags;

   flags = DisableInterrupts();

   outp( adlib_port, reg );

   for( i = 6; i ; i-- )
      {
      inp( adlib_port );
      }

   outp( adlib_port + 1, data );

   for( i = 35; i ; i-- )
      {
      inp( adlib_port );
      }

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_Stop

   Halts playback of the currently playing sound effect.
---------------------------------------------------------------------*/

int ADLIBFX_Stop
   (
   int handle
   )

   {
   unsigned flags;

   if ( ( handle != ADLIBFX_VoiceHandle ) || ( ADLIBFX_Sound == NULL ) )
      {
      ADLIBFX_SetErrorCode( ADLIBFX_VoiceNotFound );
      return( ADLIBFX_Warning );
      }

   flags = DisableInterrupts();

   ADLIBFX_SendOutput( 0xb0, 0 );

   ADLIBFX_Sound      = NULL;
   ADLIBFX_SoundPtr   = NULL;
   ADLIBFX_LengthLeft = 0;
   ADLIBFX_Priority   = 0;

   RestoreInterrupts( flags );

   if ( ADLIBFX_CallBackFunc )
      {
      ADLIBFX_CallBackFunc( ADLIBFX_CallBackVal );
      }

   return( ADLIBFX_Ok );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_Service

   Task Manager routine to perform the playback of a sound effect.
---------------------------------------------------------------------*/

static void ADLIBFX_Service
   (
   task *Task
   )

   {
   int value;

   if ( ADLIBFX_SoundPtr )
      {
      value = *ADLIBFX_SoundPtr++;
      if ( value != 0 )
         {
         ADLIBFX_SendOutput( 0xa0, value );
         ADLIBFX_SendOutput( 0xb0, ADLIBFX_Block );
         }
      else
         {
         ADLIBFX_SendOutput( 0xb0, 0 );
         }

      ADLIBFX_LengthLeft--;
      if ( ADLIBFX_LengthLeft <= 0 )
         {
         ADLIBFX_Stop( ADLIBFX_VoiceHandle );
         }
      }
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_SetVolume

   Sets the volume of the currently playing sound effect.
---------------------------------------------------------------------*/

int ADLIBFX_SetVolume
   (
   int handle,
   int volume
   )

   {
   unsigned flags;
   int      carrierlevel;

   flags = DisableInterrupts();
   if ( ( handle != ADLIBFX_VoiceHandle ) || ( ADLIBFX_Sound == NULL ) )
      {
      RestoreInterrupts( flags );
      ADLIBFX_SetErrorCode( ADLIBFX_VoiceNotFound );
      return( ADLIBFX_Warning );
      }

   volume  = min( volume, ADLIBFX_MaxVolume );
   volume  = max( volume, 0 );
   ADLIBFX_SoundVolume = volume;

   volume *= ADLIBFX_TotalVolume;
   volume /= ADLIBFX_MaxVolume;

   carrierlevel  = ADLIBFX_Sound->cScale & 0x3f;
   carrierlevel ^= 0x3f;
   carrierlevel *= ( volume / 2 ) + 0x80;
   carrierlevel /= ADLIBFX_MaxVolume;
   carrierlevel ^= 0x3f;
   carrierlevel |= ADLIBFX_Sound->cScale & 0xc0;

   ADLIBFX_SendOutput( 0x43, carrierlevel );

   RestoreInterrupts( flags );
   return( ADLIBFX_Ok );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_SetTotalVolume

   Sets the total volume of the sound effect.
---------------------------------------------------------------------*/

int ADLIBFX_SetTotalVolume
   (
   int volume
   )

   {
   volume = max( volume, 0 );
   volume = min( volume, ADLIBFX_MaxVolume );

   ADLIBFX_TotalVolume = volume;
   ADLIBFX_SetVolume( ADLIBFX_VoiceHandle, ADLIBFX_SoundVolume );

   return( ADLIBFX_Ok );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_GetTotalVolume

   Returns the total volume of the sound effect.
---------------------------------------------------------------------*/

int ADLIBFX_GetTotalVolume
   (
   void
   )

   {
   return( ADLIBFX_TotalVolume );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_VoiceAvailable

   Checks if a voice can be play at the specified priority.
---------------------------------------------------------------------*/

int ADLIBFX_VoiceAvailable
   (
   int priority
   )

   {
   if ( priority < ADLIBFX_Priority )
      {
      return( FALSE );
      }

   return( TRUE );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_Play

   Starts playback of a Muse sound effect.
---------------------------------------------------------------------*/

int ADLIBFX_Play
   (
   ALSound *sound,
   int volume,
   int priority,
   unsigned long callbackval
   )

   {
   unsigned flags;
   int      carrierlevel;

   if ( priority < ADLIBFX_Priority )
      {
      ADLIBFX_SetErrorCode( ADLIBFX_NoVoices );
      return( ADLIBFX_Warning );
      }

   ADLIBFX_Stop( ADLIBFX_VoiceHandle );

   ADLIBFX_VoiceHandle++;
   if ( ADLIBFX_VoiceHandle < ADLIBFX_MinVoiceHandle )
      {
      ADLIBFX_VoiceHandle = ADLIBFX_MinVoiceHandle;
      }

   flags = DisableInterrupts();

   ADLIBFX_LengthLeft  = sound->length;
   ADLIBFX_Priority    = priority;
   ADLIBFX_Sound       = sound;
   ADLIBFX_SoundPtr    = &sound->data;
   ADLIBFX_CallBackVal = callbackval;

   ADLIBFX_Block = ( ( sound->block & 7 ) << 2 ) | 0x20;

   volume = min( volume, ADLIBFX_MaxVolume );
   volume = max( volume, 0 );
   ADLIBFX_SoundVolume = volume;

   volume *= ADLIBFX_TotalVolume;
   volume /= ADLIBFX_MaxVolume;

   carrierlevel  = sound->cScale & 0x3f;
   carrierlevel ^= 0x3f;
   carrierlevel *= ( volume / 2 ) + 0x80;
   carrierlevel /= ADLIBFX_MaxVolume;
   carrierlevel ^= 0x3f;
   carrierlevel |= sound->cScale & 0xc0;

   ADLIBFX_SendOutput( 0x20, sound->mChar );
   ADLIBFX_SendOutput( 0x40, sound->mScale );
   ADLIBFX_SendOutput( 0x60, sound->mAttack );
   ADLIBFX_SendOutput( 0x80, sound->mSus );
   ADLIBFX_SendOutput( 0xe0, sound->mWave );

   ADLIBFX_SendOutput( 0x23, sound->cChar );
   ADLIBFX_SendOutput( 0x43, carrierlevel );
   ADLIBFX_SendOutput( 0x63, sound->cAttack );
   ADLIBFX_SendOutput( 0x83, sound->cSus );
   ADLIBFX_SendOutput( 0xe3, sound->cWave );

   ADLIBFX_SendOutput( 0xc0, 0 );

   RestoreInterrupts( flags );

   return( ADLIBFX_VoiceHandle );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_SoundPlaying

   Checks if a sound effect is currently playing.
---------------------------------------------------------------------*/

int ADLIBFX_SoundPlaying
   (
   int handle
   )

   {
   int status;

   status = FALSE;
   if ( ( handle == ADLIBFX_VoiceHandle ) && ( ADLIBFX_LengthLeft > 0 ) )
      {
      status = TRUE;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void ADLIBFX_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_SetCallBack

   Set the function to call when a voice stops.
---------------------------------------------------------------------*/

void ADLIBFX_SetCallBack
   (
   void ( *function )( unsigned long )
   )

   {
   ADLIBFX_CallBackFunc = function;
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_Init

   Initializes the sound effect engine.
---------------------------------------------------------------------*/

int ADLIBFX_Init
   (
   void
   )

   {
   int status;

   if ( ADLIBFX_Installed )
      {
      ADLIBFX_Shutdown();
      }

   status  = DPMI_LockMemoryRegion( ADLIBFX_LockStart, ADLIBFX_LockEnd );
   status |= DPMI_Lock( ADLIBFX_VoiceHandle );
   status |= DPMI_Lock( ADLIBFX_Sound );
   status |= DPMI_Lock( ADLIBFX_ErrorCode );
   status |= DPMI_Lock( ADLIBFX_SoundPtr );
   status |= DPMI_Lock( ADLIBFX_LengthLeft );
   status |= DPMI_Lock( ADLIBFX_Priority );
   status |= DPMI_Lock( ADLIBFX_CallBackFunc );
   status |= DPMI_Lock( ADLIBFX_Block );

   if ( status != DPMI_Ok )
      {
      ADLIBFX_SetErrorCode( ADLIBFX_DPMI_Error );
      return( ADLIBFX_Error );
      }

//JIM
//   AL_ReserveVoice( 0 );
   ADLIBFX_Stop( ADLIBFX_VoiceHandle );
   ADLIBFX_ServiceTask = TS_ScheduleTask( &ADLIBFX_Service, 140, 2, NULL );
   TS_Dispatch();
   ADLIBFX_Installed = TRUE;
   ADLIBFX_CallBackFunc = NULL;

   ADLIBFX_SetErrorCode( ADLIBFX_Ok );
   return( ADLIBFX_Ok );
   }


/*---------------------------------------------------------------------
   Function: ADLIBFX_Shutdown

   Ends the use of the sound effect engine.
---------------------------------------------------------------------*/

int ADLIBFX_Shutdown
   (
   void
   )

   {
   if ( ADLIBFX_Installed )
      {
      ADLIBFX_Stop( ADLIBFX_VoiceHandle );
      TS_Terminate( ADLIBFX_ServiceTask );
      ADLIBFX_ServiceTask = NULL;
//JIM
//      AL_ReleaseVoice( 0 );
      ADLIBFX_Installed = FALSE;

      DPMI_UnlockMemoryRegion( ADLIBFX_LockStart, ADLIBFX_LockEnd );
      DPMI_Unlock( ADLIBFX_VoiceHandle );
      DPMI_Unlock( ADLIBFX_Sound );
      DPMI_Unlock( ADLIBFX_ErrorCode );
      DPMI_Unlock( ADLIBFX_SoundPtr );
      DPMI_Unlock( ADLIBFX_LengthLeft );
      DPMI_Unlock( ADLIBFX_Priority );
      DPMI_Unlock( ADLIBFX_CallBackFunc );
      DPMI_Unlock( ADLIBFX_Block );
      }

   ADLIBFX_SetErrorCode( ADLIBFX_Ok );
   return( ADLIBFX_Ok );
   }
