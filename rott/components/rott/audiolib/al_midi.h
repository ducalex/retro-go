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
#ifndef __AL_MIDI_H
#define __AL_MIDI_H

enum AL_Errors
   {
   AL_Warning  = -2,
   AL_Error    = -1,
   AL_Ok       = 0,
   };

#define AL_MaxVolume             127
#define AL_DefaultChannelVolume  90
//#define AL_DefaultPitchBendRange 2
#define AL_DefaultPitchBendRange 200

#define ADLIB_PORT 0x388

void AL_SendOutputToPort( int port, int reg, int data );
void AL_SendOutput( int  voice, int reg, int data );
void AL_StereoOn( void );
void AL_StereoOff( void );
int  AL_ReserveVoice( int voice );
int  AL_ReleaseVoice( int voice );
void AL_Shutdown( void );
int  AL_Init( int soundcard );
void AL_SetMaxMidiChannel( int channel );
void AL_Reset( void );
void AL_NoteOff( int channel, int key, int velocity );
void AL_NoteOn( int channel, int key, int vel );
//Turned off to test if it works with Watcom 10a
//   #pragma aux AL_NoteOn frame;
void AL_AllNotesOff( int channel );
void AL_ControlChange( int channel, int type, int data );
void AL_ProgramChange( int channel, int patch );
void AL_SetPitchBend( int channel, int lsb, int msb );
int  AL_DetectFM( void );
void AL_RegisterTimbreBank( unsigned char *timbres );

#endif
