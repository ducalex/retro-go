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
   module: GUSWAVE.H

   author: James R. Dose
   date:   March 23, 1994

   Public header for for GUSWAVE.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __GUSWAVE_H
#define __GUSWAVE_H

#define GUSWAVE_MinVoiceHandle 1

enum GUSWAVE_Errors
   {
   GUSWAVE_Warning  = -2,
   GUSWAVE_Error    = -1,
   GUSWAVE_Ok       = 0,
   GUSWAVE_GUSError,
   GUSWAVE_NotInstalled,
   GUSWAVE_NoVoices,
   GUSWAVE_UltraNoMem,
   GUSWAVE_UltraNoMemMIDI,
   GUSWAVE_VoiceNotFound,
   GUSWAVE_InvalidVOCFile,
   GUSWAVE_InvalidWAVFile
   };

char *GUSWAVE_ErrorString( int ErrorNumber );
int   GUSWAVE_VoicePlaying( int handle );
int   GUSWAVE_VoicesPlaying( void );
int   GUSWAVE_Kill( int handle );
int   GUSWAVE_KillAllVoices( void );
int   GUSWAVE_SetPitch( int handle, int pitchoffset );
int   GUSWAVE_SetPan3D( int handle, int angle, int distance );
void  GUSWAVE_SetVolume( int volume );
int   GUSWAVE_GetVolume( void );
int   GUSWAVE_VoiceAvailable( int priority );
int   GUSWAVE_PlayVOC( char *sample, int pitchoffset, int angle, int volume,
         int priority, unsigned long callbackval );
int   GUSWAVE_PlayWAV( char *sample, int pitchoffset, int angle, int volume,
         int priority, unsigned long callbackval );
int   GUSWAVE_StartDemandFeedPlayback( void ( *function )( char **ptr, unsigned long *length ),
         int channels, int bits, int rate, int pitchoffset, int angle,
         int volume, int priority, unsigned long callbackval );
void  GUSWAVE_SetCallBack( void ( *function )( unsigned long ) );
void  GUSWAVE_SetReverseStereo( int setting );
int   GUSWAVE_GetReverseStereo( void );
int   GUSWAVE_Init( int numvoices );
void  GUSWAVE_Shutdown( void );
#pragma aux GUSWAVE_Shutdown frame;

#endif
