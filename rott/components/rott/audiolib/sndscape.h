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
   module: SNDSCAPE.H

   author: James R. Dose
   date:   October 26, 1994

   Public header for SNDSCAPE.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __SNDSCAPE_H
#define __SNDSCAPE_H

extern int SOUNDSCAPE_DMAChannel;
extern int SOUNDSCAPE_ErrorCode;

enum SOUNDSCAPE_ERRORS
   {
   SOUNDSCAPE_Warning = -2,
   SOUNDSCAPE_Error = -1,
   SOUNDSCAPE_Ok = 0,
   SOUNDSCAPE_EnvNotFound,
   SOUNDSCAPE_InitFileNotFound,
   SOUNDSCAPE_MissingProductInfo,
   SOUNDSCAPE_MissingPortInfo,
   SOUNDSCAPE_MissingDMAInfo,
   SOUNDSCAPE_MissingIRQInfo,
   SOUNDSCAPE_MissingSBIRQInfo,
   SOUNDSCAPE_MissingSBENABLEInfo,
   SOUNDSCAPE_MissingWavePortInfo,
   SOUNDSCAPE_HardwareError,
   SOUNDSCAPE_NoSoundPlaying,
   SOUNDSCAPE_InvalidSBIrq,
   SOUNDSCAPE_UnableToSetIrq,
   SOUNDSCAPE_DmaError,
   SOUNDSCAPE_DPMI_Error,
   SOUNDSCAPE_OutOfMemory
   };

char    *SOUNDSCAPE_ErrorString( int ErrorNumber );
void     SOUNDSCAPE_SetPlaybackRate( unsigned rate );
unsigned SOUNDSCAPE_GetPlaybackRate( void );
int      SOUNDSCAPE_SetMixMode( int mode );
void     SOUNDSCAPE_StopPlayback( void );
int      SOUNDSCAPE_GetCurrentPos( void );
int      SOUNDSCAPE_BeginBufferedPlayback( char *BufferStart, int BufferSize, int NumDivisions, unsigned SampleRate, int MixMode, void ( *CallBackFunc )( void ) );
int      SOUNDSCAPE_GetCardInfo( int *MaxSampleBits, int *MaxChannels );
void     SOUNDSCAPE_SetCallBack( void ( *func )( void ) );
int      SOUNDSCAPE_GetMIDIPort( void );
int      SOUNDSCAPE_Init( void );
void     SOUNDSCAPE_Shutdown( void );

#endif
