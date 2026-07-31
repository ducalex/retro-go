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
   module: MUSIC.H

   author: James R. Dose
   date:   March 25, 1994

   Public header for MUSIC.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __MUSIC_H
#define __MUSIC_H

#include "sndcards.h"

#ifndef PLAT_DOS
#define cdecl
#endif

extern int MUSIC_ErrorCode;

enum MUSIC_ERRORS
   {
   MUSIC_Warning = -2,
   MUSIC_Error   = -1,
   MUSIC_Ok      = 0,
   MUSIC_ASSVersion,
   MUSIC_SoundCardError,
   MUSIC_MPU401Error,
   MUSIC_InvalidCard,
   MUSIC_MidiError,
   MUSIC_TaskManError,
   MUSIC_FMNotDetected,
   MUSIC_DPMI_Error
   };

typedef struct
   {
   unsigned long tickposition;
   unsigned long milliseconds;
   unsigned int  measure;
   unsigned int  beat;
   unsigned int  tick;
   } songposition;

#define MUSIC_LoopSong ( 1 == 1 )
#define MUSIC_PlayOnce ( !MUSIC_LoopSong )

char *MUSIC_ErrorString( int ErrorNumber );
int   MUSIC_Init( int SoundCard, int Address );
int   MUSIC_Shutdown( void );
void  MUSIC_SetMaxFMMidiChannel( int channel );
void  MUSIC_SetVolume( int volume );
void  MUSIC_SetMidiChannelVolume( int channel, int volume );
void  MUSIC_ResetMidiChannelVolumes( void );
int   MUSIC_GetVolume( void );
void  MUSIC_SetLoopFlag( int loopflag );
int   MUSIC_SongPlaying( void );
void  MUSIC_Continue( void );
void  MUSIC_Pause( void );
int   MUSIC_StopSong( void );
int   MUSIC_PlaySong( unsigned char *song, int loopflag );
void  MUSIC_SetContext( int context );
int   MUSIC_GetContext( void );
void  MUSIC_SetSongTick( unsigned long PositionInTicks );
void  MUSIC_SetSongTime( unsigned long milliseconds );
void  MUSIC_SetSongPosition( int measure, int beat, int tick );
void  MUSIC_GetSongPosition( songposition *pos );
void  MUSIC_GetSongLength( songposition *pos );
int   MUSIC_FadeVolume( int tovolume, int milliseconds );
int   MUSIC_FadeActive( void );
void  MUSIC_StopFade( void );
void  MUSIC_RerouteMidiChannel( int channel, int cdecl ( *function )( int event, int c1, int c2 ) );
void  MUSIC_RegisterTimbreBank( unsigned char *timbres );

#endif
