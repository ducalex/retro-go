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
   module: FX_MAN.H

   author: James R. Dose
   date:   March 17, 1994

   Public header for FX_MAN.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __FX_MAN_H
#define __FX_MAN_H

#include "sndcards.h"

typedef struct
   {
   int MaxVoices;
   int MaxSampleBits;
   int MaxChannels;
   } fx_device;

#define MonoFx   1
#define StereoFx 2

typedef struct
   {
   unsigned long Address;
   unsigned long Type;
   unsigned long Interrupt;
   unsigned long Dma8;
   unsigned long Dma16;
   unsigned long Midi;
   unsigned long Emu;
   } fx_blaster_config;

enum FX_ERRORS
   {
   FX_Warning = -2,
   FX_Error = -1,
   FX_Ok = 0,
   FX_ASSVersion,
   FX_BlasterError,
   FX_SoundCardError,
   FX_InvalidCard,
   FX_MultiVocError,
   FX_DPMI_Error
   };

enum fx_BLASTER_Types
   {
   fx_SB     = 1,
   fx_SBPro  = 2,
   fx_SB20   = 3,
   fx_SBPro2 = 4,
   fx_SB16   = 6
   };


char *FX_ErrorString( int ErrorNumber );
int   FX_SetupCard( int SoundCard, fx_device *device );
int   FX_GetBlasterSettings( fx_blaster_config *blaster );
int   FX_SetupSoundBlaster( fx_blaster_config blaster, int *MaxVoices, int *MaxSampleBits, int *MaxChannels );
int   FX_Init( int SoundCard, int numvoices, int numchannels, int samplebits, unsigned mixrate );
int   FX_Shutdown( void );
int   FX_SetCallBack( void ( *function )( unsigned long ) );
void  FX_SetVolume( int volume );
int   FX_GetVolume( void );

void  FX_SetReverseStereo( int setting );
int   FX_GetReverseStereo( void );
void  FX_SetReverb( int reverb );
void  FX_SetFastReverb( int reverb );
int   FX_GetMaxReverbDelay( void );
int   FX_GetReverbDelay( void );
void  FX_SetReverbDelay( int delay );

int FX_VoiceAvailable( int priority );
int FX_EndLooping( int handle );
int FX_SetPan( int handle, int vol, int left, int right );
int FX_SetPitch( int handle, int pitchoffset );
int FX_SetFrequency( int handle, int frequency );

int FX_PlayVOC( char *ptr, int pitchoffset, int vol, int left, int right,
       int priority, unsigned long callbackval );
int FX_PlayLoopedVOC( char *ptr, long loopstart, long loopend,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval );
int FX_PlayWAV( char *ptr, int pitchoffset, int vol, int left, int right,
       int priority, unsigned long callbackval );
int FX_PlayLoopedWAV( char *ptr, long loopstart, long loopend,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval );
int FX_PlayVOC3D( char *ptr, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval );
int FX_PlayWAV3D( char *ptr, int pitchoffset, int angle, int distance,
       int priority, unsigned long callbackval );
int FX_PlayRaw( char *ptr, unsigned long length, unsigned rate,
       int pitchoffset, int vol, int left, int right, int priority,
       unsigned long callbackval );
int FX_PlayLoopedRaw( char *ptr, unsigned long length, char *loopstart,
       char *loopend, unsigned rate, int pitchoffset, int vol, int left,
       int right, int priority, unsigned long callbackval );
int FX_Pan3D( int handle, int angle, int distance );
int FX_SoundActive( int handle );
int FX_SoundsPlaying( void );
int FX_StopSound( int handle );
int FX_StopAllSounds( void );
int FX_StartDemandFeedPlayback( void ( *function )( char **ptr, unsigned long *length ),
       int rate, int pitchoffset, int vol, int left, int right,
       int priority, unsigned long callbackval );
int  FX_StartRecording( int MixRate, void ( *function )( char *ptr, int length ) );
void FX_StopRecord( void );

#endif
