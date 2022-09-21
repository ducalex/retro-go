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
   file:   _MULTIVC.H

   author: James R. Dose
   date:   December 20, 1993

   Private header for MULTIVOC.C

   (c) Copyright 1993 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef ___MULTIVC_H
#define ___MULTIVC_H

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

#define VOC_8BIT            0x0
#define VOC_CT4_ADPCM       0x1
#define VOC_CT3_ADPCM       0x2
#define VOC_CT2_ADPCM       0x3
#define VOC_16BIT           0x4
#define VOC_ALAW            0x6
#define VOC_MULAW           0x7
#define VOC_CREATIVE_ADPCM  0x200

#define T_SIXTEENBIT_STEREO 0
#define T_8BITS       1
#define T_MONO        2
#define T_16BITSOURCE 4
#define T_LEFTQUIET   8
#define T_RIGHTQUIET  16
#define T_DEFAULT     T_SIXTEENBIT_STEREO

#define MV_MaxPanPosition  31
#define MV_NumPanPositions ( MV_MaxPanPosition + 1 )
#define MV_MaxTotalVolume  255
//#define MV_MaxVolume       63
//#define MV_NumVoices       8

#define MIX_VOLUME( volume ) \
   ( ( max( 0, min( ( volume ), 255 ) ) * ( MV_MaxVolume + 1 ) ) >> 8 )
//   ( ( max( 0, min( ( volume ), 255 ) ) ) >> 2 )

//#define SILENCE_16BIT     0x80008000
#define SILENCE_16BIT     0
#define SILENCE_8BIT      0x80808080
//#define SILENCE_16BIT_PAS 0

#define MixBufferSize     256

#define NumberOfBuffers   16
#define TotalBufferSize   ( MixBufferSize * NumberOfBuffers )

#define PI                3.1415926536

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
   KeepPlaying
   } playbackstatus;


typedef struct VoiceNode
   {
   struct VoiceNode *next;
   struct VoiceNode *prev;

   wavedata      wavetype;
   char          bits;

   playbackstatus ( *GetSound )( struct VoiceNode *voice );

   void ( *mix )( unsigned long position, unsigned long rate,
      uint8_t *start, unsigned long length );

   uint8_t      *NextBlock;
   uint8_t      *LoopStart;
   uint8_t      *LoopEnd;
   unsigned      LoopCount;
   unsigned long LoopSize;
   unsigned long BlockLength;

   unsigned long PitchScale;
   unsigned long FixedPointBufferSize;

   uint8_t       *sound;
   unsigned long length;
   uint32_t SamplingRate;
   unsigned long RateScale;
   unsigned long position;
   int           Playing;

   int           handle;
   int           priority;

   void          ( *DemandFeed )( char **ptr, uint32_t *length );

#if 0
   short        *LeftVolume;
   short        *RightVolume;
#else
   int           LeftVolume;
   int           RightVolume;
#endif

   int           GLast;
   int           GPos;
   int           GVal[4];

   unsigned long callbackval;

   } VoiceNode;

typedef struct
   {
   VoiceNode *start;
   VoiceNode *end;
   } VList;

typedef struct
   {
   unsigned char left;
   unsigned char right;
   } Pan;

typedef signed short MONO16;
typedef signed char  MONO8;

typedef struct
   {
   MONO16 left;
   MONO16 right;
//   unsigned short left;
//   unsigned short right;
   } STEREO16;

typedef struct
   {
   MONO16 left;
   MONO16 right;
   } SIGNEDSTEREO16;

typedef struct
   {
//   MONO8 left;
//   MONO8 right;
   char left;
   char right;
   } STEREO8;

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
   char DATA[ 4 ];
   unsigned long size;
   } data_header;

typedef MONO8  VOLUME8[ 256 ];
typedef MONO16 VOLUME16[ 256 ];

// typedef char HARSH_CLIP_TABLE_8[ MV_NumVoices * 256 ];

//static void MV_Mix( VoiceNode *voice);
//static void MV_PlayVoice( VoiceNode *voice );
//static void MV_StopVoice( VoiceNode *voice );
//static void MV_ServiceVoc( void );

//static playbackstatus MV_GetNextVOCBlock( VoiceNode *voice );
//static playbackstatus MV_GetNextDemandFeedBlock( VoiceNode *voice );
//static playbackstatus MV_GetNextRawBlock( VoiceNode *voice );
//static playbackstatus MV_GetNextWAVBlock( VoiceNode *voice );

//static void       MV_ServiceRecord( void );
//static VoiceNode *MV_GetVoice( int handle );
//static VoiceNode *MV_AllocVoice( int priority );

//#if 0
//static short     *MV_GetVolumeTable( int vol );
//#else
//static int        MV_GetVolumeTable( int vol );
//#endif

void       MV_SetVoiceMixMode( VoiceNode *voice );

//static void       MV_SetVoicePitch( VoiceNode *voice, unsigned long rate, int pitchoffset );
//static void       MV_CalcVolume( int MaxLevel );
//static void       MV_CalcPanTable( void );

#ifdef PLAT_DOS
#define ATR_INDEX               0x3c0
#define STATUS_REGISTER_1       0x3da

#define SetBorderColor(color) \
   { \
   inp  (STATUS_REGISTER_1); \
   outp (ATR_INDEX,0x31);    \
   outp (ATR_INDEX,color);   \
   }
#endif

void ClearBuffer_DW( void *ptr, unsigned data, int length );

#ifdef PLAT_DOS
#pragma aux ClearBuffer_DW = \
   "cld",                    \
   "push   es",              \
   "push   ds",              \
   "pop    es",              \
   "rep    stosd",           \
   "pop    es",              \
parm [ edi ] [ eax ] [ ecx ] modify exact [ ecx edi ];
#endif

/*
void MV_Mix8BitMono( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_Mix8BitStereo( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );

void MV_Mix16BitMono( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );

void MV_Mix16BitStereo( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );

void MV_Mix16BitMono16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );

void MV_Mix8BitMono16( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_Mix8BitStereo16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );

void MV_Mix16BitStereo16( unsigned long position,
   unsigned long rate, const char *start, unsigned long length );
*/

void MV_MixFPMono8( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_MixFPMono16( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_MixFPStereo8( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_MixFPStereo16( unsigned long position, unsigned long rate,
   const char *start, unsigned long length );

void MV_FPReverb(int volume);

void MV_FPReverbFree(void);

void MV_16BitDownmix(char *dest, int count);

void MV_8BitDownmix(char *dest, int count);

/*
void MV_16BitReverbFast( const char *src, char *dest, int count, int shift );

void MV_8BitReverbFast( const signed char *src, signed char *dest, int count, int shift );
*/
#ifdef PLAT_DOS
#pragma aux MV_16BitReverb parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx ecx edx esi edi]
#pragma aux MV_8BitReverb parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx ecx edx esi edi]
#pragma aux MV_16BitReverbFast parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx ecx edx esi edi]
#pragma aux MV_8BitReverbFast parm [eax] [edx] [ebx] [ecx] modify exact [eax ebx ecx edx esi edi]
#endif

#endif
