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
   module: PAS16.H

   author: James R. Dose
   date:   March 27, 1994

   Public header for for PAS16.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __PAS16_H
#define __PAS16_H

enum PAS_ERRORS
   {
   PAS_Warning = -2,
   PAS_Error   = -1,
   PAS_Ok      = 0,
   PAS_DriverNotFound,
   PAS_DmaError,
   PAS_InvalidIrq,
   PAS_UnableToSetIrq,
   PAS_Dos4gwIrqError,
   PAS_NoSoundPlaying,
   PAS_CardNotFound,
   PAS_DPMI_Error,
   PAS_OutOfMemory
   };

#define PAS_MaxMixMode        STEREO_16BIT
#define PAS_DefaultSampleRate 11000
#define PAS_DefaultMixMode    MONO_8BIT
#define PAS_MaxIrq            15

#define PAS_MinSamplingRate   4000
#define PAS_MaxSamplingRate   44000

extern unsigned int PAS_DMAChannel;

char *PAS_ErrorString( int ErrorNumber );
void  PAS_SetPlaybackRate( unsigned rate );
unsigned PAS_GetPlaybackRate( void );
int   PAS_SetMixMode( int mode );
void  PAS_StopPlayback( void );
int   PAS_GetCurrentPos( void );
int   PAS_BeginBufferedPlayback( char *BufferStart, int BufferSize, int NumDivisions, unsigned SampleRate, int MixMode, void ( *CallBackFunc )( void ) );
int   PAS_BeginBufferedRecord( char *BufferStart, int BufferSize, int NumDivisions, unsigned SampleRate, int MixMode, void ( *CallBackFunc )( void ) );
int   PAS_SetPCMVolume( int volume );
int   PAS_GetPCMVolume( void );
void  PAS_SetFMVolume( int volume );
int   PAS_GetFMVolume( void );
int   PAS_GetCardInfo( int *MaxSampleBits, int *MaxChannels );
void  PAS_SetCallBack( void ( *func )( void ) );
int   PAS_SaveMusicVolume( void );
void  PAS_RestoreMusicVolume( void );
int   PAS_Init( void );
void  PAS_Shutdown( void );
void  PAS_UnlockMemory( void );
int   PAS_LockMemory( void );

#endif
