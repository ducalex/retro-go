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
   file:   MULTIVOC.H

   author: James R. Dose
   date:   December 20, 1993

   Public header for MULTIVOC.C

   (c) Copyright 1993 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __MULTIVOC_H
#define __MULTIVOC_H

// platform.h is in buildengine, but I need the byteswapping macros... --ryan.
#include "platform.h"

#define MV_MinVoiceHandle  1

extern int MV_ErrorCode;

enum MV_Errors
   {
   MV_Warning = -2,
   MV_Error   = -1,
   MV_Ok      = 0,
   MV_UnsupportedCard,
   MV_NotInstalled,
   MV_NoVoices,
   MV_NoMem,
   MV_VoiceNotFound,
   MV_BlasterError,
   MV_PasError,
   MV_SoundScapeError,
   MV_SoundSourceError,
   MV_DPMI_Error,
   MV_InvalidVOCFile,
   MV_InvalidWAVFile,
   MV_InvalidMixMode,
   MV_SoundSourceFailure,
   MV_IrqFailure,
   MV_DMAFailure,
   MV_DMA16Failure,
   MV_NullRecordFunction
   };

char *MV_ErrorString( int ErrorNumber );
int   MV_VoicePlaying( int handle );
int   MV_KillAllVoices( void );
int   MV_Kill( int handle );
int   MV_VoicesPlaying( void );
int   MV_VoiceAvailable( int priority );
int   MV_SetPitch( int handle, int pitchoffset );
int   MV_SetFrequency( int handle, int frequency );
int   MV_EndLooping( int handle );
int   MV_SetPan( int handle, int vol, int left, int right );
int   MV_Pan3D( int handle, int angle, int distance );
void  MV_SetReverb( int reverb );
void  MV_SetFastReverb( int reverb );
int   MV_GetMaxReverbDelay( void );
int   MV_GetReverbDelay( void );
void  MV_SetReverbDelay( int delay );
int   MV_SetMixMode( int numchannels, int samplebits );
int   MV_StartPlayback( void );
void  MV_StopPlayback( void );
int   MV_StartRecording( int MixRate, void ( *function )( char *ptr, int length ) );
void  MV_StopRecord( void );
int   MV_StartDemandFeedPlayback( void ( *function )( char **ptr, unsigned long *length ),
         int rate, int pitchoffset, int vol, int left, int right,
         int priority, unsigned long callbackval );
int   MV_PlayRaw( char *ptr, unsigned long length,
         unsigned rate, int pitchoffset, int vol, int left,
         int right, int priority, unsigned long callbackval );
int   MV_PlayLoopedRaw( char *ptr, unsigned long length,
         char *loopstart, char *loopend, unsigned rate, int pitchoffset,
         int vol, int left, int right, int priority,
         unsigned long callbackval );
int   MV_PlayWAV( char *ptr, int pitchoffset, int vol, int left,
         int right, int priority, unsigned long callbackval );
int   MV_PlayWAV3D( char *ptr, int pitchoffset, int angle, int distance,
         int priority, unsigned long callbackval );
int   MV_PlayLoopedWAV( char *ptr, long loopstart, long loopend,
         int pitchoffset, int vol, int left, int right, int priority,
         unsigned long callbackval );
int   MV_PlayVOC3D( char *ptr, int pitchoffset, int angle, int distance,
         int priority, unsigned long callbackval );
int   MV_PlayVOC( char *ptr, int pitchoffset, int vol, int left, int right,
         int priority, unsigned long callbackval );
int   MV_PlayLoopedVOC( char *ptr, long loopstart, long loopend,
         int pitchoffset, int vol, int left, int right, int priority,
         unsigned long callbackval );
void  MV_CreateVolumeTable( int index, int volume, int MaxVolume );
void  MV_SetVolume( int volume );
int   MV_GetVolume( void );
void  MV_SetCallBack( void ( *function )( unsigned long ) );
void  MV_SetReverseStereo( int setting );
int   MV_GetReverseStereo( void );
int   MV_Init( int soundcard, int MixRate, int Voices, int numchannels,
         int samplebits );
int   MV_Shutdown( void );
void  MV_UnlockMemory( void );
int   MV_LockMemory( void );

#endif
