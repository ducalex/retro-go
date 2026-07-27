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
#ifndef ___AL_MIDI_H
#define ___AL_MIDI_H

#define NO_ADLIB_DETECTION    "NOAL"

#define STEREO_DETUNE 5

#define lobyte( num )   ( ( unsigned )*( ( char * )&( num ) ) )
#define hibyte( num )   ( ( unsigned )*( ( ( char * )&( num ) ) + 1 ) )

#define AL_VoiceNotFound -1

#define  alFreqH     0xb0
#define  alEffects   0xbd

/* Number of slots for the voices on the chip */
#define NumChipSlots 18

#define NUM_VOICES      9
#define NUM_CHANNELS    16

#define NOTE_ON         0x2000  /* Used to turn note on or toggle note */
#define NOTE_OFF        0x0000

#define MAX_VELOCITY   0x7f
#define MAX_OCTAVE     7
#define MAX_NOTE       ( MAX_OCTAVE * 12 + 11 )
#define FINETUNE_MAX   31
#define FINETUNE_RANGE ( FINETUNE_MAX + 1 )

#define PITCHBEND_CENTER 1638400

#define note_off             0x80
#define note_on              0x90
#define poly_aftertouch      0xa0
#define control_change       0xb0
#define program_chng         0xc0
#define channel_aftertouch   0xd0
#define pitch_wheel          0xe0

#define MIDI_VOLUME          7
#define MIDI_PAN             10
#define MIDI_DETUNE          94
#define MIDI_ALL_NOTES_OFF   0x7B
#define MIDI_RESET_ALL_CONTROLLERS 0x79
#define MIDI_RPN_MSB               100
#define MIDI_RPN_LSB               101
#define MIDI_DATAENTRY_MSB         6
#define MIDI_DATAENTRY_LSB         38
#define MIDI_PITCHBEND_RPN         0

enum cromatic_scale
   {
   C       = 0x157,
   C_SHARP = 0x16B,
   D_FLAT  = 0x16B,
   D       = 0x181,
   D_SHARP = 0x198,
   E_FLAT  = 0x198,
   E       = 0x1B0,
   F_FLAT  = 0x1B0,
   E_SHARP = 0x1CA,
   F       = 0x1CA,
   F_SHARP = 0x1E5,
   G_FLAT  = 0x1E5,
   G       = 0x202,
   G_SHARP = 0x220,
   A_FLAT  = 0x220,
   A       = 0x241,
   A_SHARP = 0x263,
   B_FLAT  = 0x263,
   B       = 0x287,
   C_FLAT  = 0x287,
   B_SHARP = 0x2AE,
   };

/* Definition of octave information to be ORed onto F-Number */

enum octaves
   {
   OCTAVE_0 = 0x0000,
   OCTAVE_1 = 0x0400,
   OCTAVE_2 = 0x0800,
   OCTAVE_3 = 0x0C00,
   OCTAVE_4 = 0x1000,
   OCTAVE_5 = 0x1400,
   OCTAVE_6 = 0x1800,
   OCTAVE_7 = 0x1C00
   };

typedef struct VOICE
   {
   struct VOICE *next;
   struct VOICE *prev;

   unsigned num;
   unsigned key;
   unsigned velocity;
   unsigned channel;
   unsigned pitchleft;
   unsigned pitchright;
   int      timbre;
   int      port;
   unsigned status;
   } VOICE;

typedef struct
   {
   VOICE *start;
   VOICE *end;
   } VOICELIST;

typedef struct
   {
   VOICELIST Voices;
   int       Timbre;
   int       Pitchbend;
   int       KeyOffset;
   unsigned  KeyDetune;
   unsigned  Volume;
   unsigned  EffectiveVolume;
   int       Pan;
   int       Detune;
   unsigned  RPN;
   short     PitchBendRange;
   short     PitchBendSemiTones;
   short     PitchBendHundreds;
   } CHANNEL;

typedef struct
   {
   unsigned char SAVEK[ 2 ];
   unsigned char Level[ 2 ];
   unsigned char Env1[ 2 ];
   unsigned char Env2[ 2 ];
   unsigned char Wave[ 2 ];
   unsigned char Feedback;
   signed   char Transpose;
   signed   char Velocity;
   } TIMBRE;

extern TIMBRE ADLIB_TimbreBank[ 256 ];

static void AL_ResetVoices( void );
static void AL_CalcPitchInfo( void );
static void AL_SetVoiceTimbre( int voice );
static void AL_SetVoiceVolume( int voice );
static int  AL_AllocVoice( void );
static int  AL_GetVoice( int channel, int key );
static void AL_SetVoicePitch( int voice );
static void AL_SetChannelVolume( int channel, int volume );
static void AL_SetChannelPan( int channel, int pan );
static void AL_SetChannelDetune( int channel, int detune );

#endif
