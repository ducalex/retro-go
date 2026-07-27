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
   file:   _GUSWAVE.H

   author: James R. Dose
   date:   March 23, 1994

   Private header for GUSWAVE.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef ___GUSWAVE_H
#define ___GUSWAVE_H

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

#define LOADDS _loadds

#define VOC_8BIT            0x0
#define VOC_CT4_ADPCM       0x1
#define VOC_CT3_ADPCM       0x2
#define VOC_CT2_ADPCM       0x3
#define VOC_16BIT           0x4
#define VOC_ALAW            0x6
#define VOC_MULAW           0x7
#define VOC_CREATIVE_ADPCM  0x200

#define MAX_BLOCK_LENGTH 0x8000

#define GF1BSIZE   896L   /* size of buffer per wav on GUS */
//#define GF1BSIZE   512L   /* size of buffer per wav on GUS */

//#define VOICES     8      /* maximum amount of concurrent wav files */
#define VOICES     2      /* maximum amount of concurrent wav files */
#define MAX_VOICES 32     /* This should always be 32 */
#define MAX_VOLUME 4095
#define BUFFER     2048U  /* size of DMA buffer for patch loading */

typedef enum
   {
   Raw,
   VOC,
   DemandFeed,
   WAV
   } wavedata;

typedef enum
   {
   NoMoreData,
   KeepPlaying,
   SoundDone
   } playbackstatus;


typedef volatile struct VoiceNode
   {
   struct VoiceNode *next;
   struct VoiceNode *prev;

   wavedata      wavetype;
   int           bits;
   playbackstatus ( *GetSound )( struct VoiceNode *voice );

   int num;

   unsigned long  mem;           /* location in ultrasound memory */
   int            Active;        /* this instance in use */
   int            GF1voice;      /* handle to active voice */

   char          *NextBlock;
   char          *LoopStart;
   char          *LoopEnd;
   unsigned       LoopCount;
   unsigned long  LoopSize;
   unsigned long  BlockLength;

   unsigned long  PitchScale;

   unsigned char *sound;
   unsigned long  length;
   unsigned long  SamplingRate;
   unsigned long  RateScale;
   int            Playing;

   int    handle;
   int    priority;

   void          ( *DemandFeed )( char **ptr, unsigned long *length );

   unsigned long callbackval;

   int    Volume;
   int    Pan;
   }
VoiceNode;

typedef struct
   {
   VoiceNode *start;
   VoiceNode *end;
   }
voicelist;

typedef volatile struct voicestatus
   {
   VoiceNode *Voice;
   int playing;
   }
voicestatus;

typedef struct
   {
   char          RIFF[ 4 ];
   unsigned long file_size;
   char          WAVE[ 4 ];
   char          fmt[ 4 ];
   unsigned long format_size;
   } riff_header;

typedef struct
   {
   unsigned short wFormatTag;
   unsigned short nChannels;
   unsigned long  nSamplesPerSec;
   unsigned long  nAvgBytesPerSec;
   unsigned short nBlockAlign;
   unsigned short nBitsPerSample;
   } format_header;

typedef struct
   {
   unsigned char DATA[ 4 ];
   unsigned long size;
   } data_header;

playbackstatus GUSWAVE_GetNextVOCBlock( VoiceNode *voice );
VoiceNode *GUSWAVE_GetVoice( int handle );

int GUSWAVE_Play( VoiceNode *voice, int angle, int volume, int channels );

VoiceNode *GUSWAVE_AllocVoice( int priority );
static int GUSWAVE_InitVoices( void );

#endif
