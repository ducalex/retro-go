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
   module: MIDI.H

   author: James R. Dose
   date:   May 25, 1994

   Public header for MIDI.C.  Midi song file playback routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __MIDI_H
#define __MIDI_H

enum MIDI_Errors
   {
   MIDI_Warning = -2,
   MIDI_Error   = -1,
   MIDI_Ok      = 0,
   MIDI_NullMidiModule,
   MIDI_InvalidMidiFile,
   MIDI_UnknownMidiFormat,
   MIDI_NoTracks,
   MIDI_InvalidTrack,
   MIDI_NoMemory,
   MIDI_DPMI_Error
   };


#define MIDI_PASS_THROUGH 1
#define MIDI_DONT_PLAY    0

#define MIDI_MaxVolume 255

extern char MIDI_PatchMap[ 128 ];

typedef struct
   {
   void ( *NoteOff )( int channel, int key, int velocity );
   void ( *NoteOn )( int channel, int key, int velocity );
   void ( *PolyAftertouch )( int channel, int key, int pressure );
   void ( *ControlChange )( int channel, int number, int value );
   void ( *ProgramChange )( int channel, int program );
   void ( *ChannelAftertouch )( int channel, int pressure );
   void ( *PitchBend )( int channel, int lsb, int msb );
   void ( *ReleasePatches )( void );
   void ( *LoadPatch )( int number );
   void ( *SetVolume )( int volume );
   int  ( *GetVolume )( void );
   } midifuncs;

void MIDI_RerouteMidiChannel( int channel, int cdecl ( *function )( int event, int c1, int c2 ) );
int  MIDI_AllNotesOff( void );
void MIDI_SetUserChannelVolume( int channel, int volume );
void MIDI_ResetUserChannelVolume( void );
int  MIDI_Reset( void );
int  MIDI_SetVolume( int volume );
int  MIDI_GetVolume( void );
void MIDI_SetMidiFuncs( midifuncs *funcs );
void MIDI_SetContext( int context );
int  MIDI_GetContext( void );
void MIDI_SetLoopFlag( int loopflag );
void MIDI_ContinueSong( void );
void MIDI_PauseSong( void );
int  MIDI_SongPlaying( void );
void MIDI_StopSong( void );
int  MIDI_PlaySong( unsigned char *song, int loopflag );
void MIDI_SetTempo( int tempo );
int  MIDI_GetTempo( void );
void MIDI_SetSongTick( unsigned long PositionInTicks );
void MIDI_SetSongTime( unsigned long milliseconds );
void MIDI_SetSongPosition( int measure, int beat, int tick );
void MIDI_GetSongPosition( songposition *pos );
void MIDI_GetSongLength( songposition *pos );
void MIDI_LoadTimbres( void );
void MIDI_UnlockMemory( void );
int  MIDI_LockMemory( void );

#endif
