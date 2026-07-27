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
   module: MULTIVOC.C

   author: James R. Dose
   date:   December 20, 1993

   Routines to provide multichannel digitized sound playback for
   Sound Blaster compatible sound cards.

   (c) Copyright 1993 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef PLAT_DOS
#include <dos.h>
#include <conio.h>
#endif

#include "util.h"
#include "dpmi.h"
#include "usrhooks.h"
#include "interrup.h"
#include "dma.h"
#include "linklist.h"
#include "sndcards.h"

#ifdef PLAT_DOS
#include "blaster.h"
#include "sndscape.h"
#include "sndsrc.h"
#include "pas16.h"
#include "guswave.h"
#else
#include "dsl.h"
#endif

#include "pitch.h"
#include "multivoc.h"
#include "_multivc.h"
#include "debugio.h"
#include "sounddebugdefs.h"

// for the Mutex
#include <../SDL/SDL.h>

#define RoundFixed( fixedval, bits )            \
        (                                       \
          (                                     \
            (fixedval) + ( 1 << ( (bits) - 1 ) )\
          ) >> (bits)                           \
        )

// #define IS_QUIET( ptr )  ( ( void * )( ptr ) == ( void * )&MV_VolumeTable[ 0 ] )
#define IS_QUIET( vol ) ( ( vol ) == 0 )

static int       MV_ReverbLevel;
int       MV_ReverbDelay;
#if 0
static VOLUME16 *MV_ReverbTable = NULL;
#else
static int       MV_ReverbTable = -1;
#endif

//static signed short MV_VolumeTable[ MV_MaxVolume + 1 ][ 256 ];
//static signed short MV_VolumeTable[ 63 + 1 ][ 256 ];

//static Pan MV_PanTable[ MV_NumPanPositions ][ MV_MaxVolume + 1 ];
static Pan MV_PanTable[ MV_NumPanPositions ][ 63 + 1 ];

static int MV_Installed   = FALSE;
static int MV_SoundCard   = SoundBlaster;
static int MV_TotalVolume = MV_MaxTotalVolume;
static int MV_MaxVoices   = 1;
static int MV_Recording;

int MV_BufferSize = MixBufferSize;
static int MV_BufferLength;

static int MV_NumberOfBuffers = NumberOfBuffers;

static int MV_MixMode    = MONO_8BIT;
int MV_Channels   = 1;
static int MV_Bits       = 8;

static int MV_Silence    = SILENCE_8BIT;
static int MV_SwapLeftRight = FALSE;

static int MV_RequestedMixRate;
int MV_MixRate;

static int MV_DMAChannel = -1;
static int MV_BuffShift;

static int MV_TotalMemory;
static int MV_FooMemory;

static int   MV_BufferDescriptor;
static int   MV_BufferEmpty[ NumberOfBuffers ];
char *MV_MixBuffer[ NumberOfBuffers + 1 ];
int32_t *MV_FooBuffer = NULL;

static VoiceNode *MV_Voices = NULL;

static volatile VoiceNode VoiceList;
static volatile VoiceNode VoicePool;

/*static*/ int MV_MixPage      = 0;
static int MV_VoiceHandle  = MV_MinVoiceHandle;

SDL_mutex* reverbMutex;

static void ( *MV_CallBackFunc )( int32_t ) = NULL;
static void ( *MV_RecordFunc )( char *ptr, int length ) = NULL;
static void ( *MV_MixFunction )( VoiceNode *voice);

int MV_MaxVolume = 63;

int *MV_GLast, *MV_GPos, *MV_GVal;

// char  *MV_HarshClipTable;
char  *MV_MixDestination;
#if 0
short *MV_LeftVolume;
short *MV_RightVolume;
#else
int    MV_LeftVolume;
int    MV_RightVolume;
#endif
int    MV_SampleSize = 1;
int    MV_RightChannelOffset;

unsigned long MV_MixPosition;

int MV_ErrorCode = MV_Ok;

#define MV_SetErrorCode( status ) \
   MV_ErrorCode   = ( status );

static uint16_t MV_ReadLE16( const uint8_t *p )
{
   return (uint16_t)( p[0] | ( p[1] << 8 ) );
}

static uint32_t MV_ReadLE32( const uint8_t *p )
{
   return (uint32_t)( p[0] | ( p[1] << 8 ) | ( p[2] << 16 ) | ( p[3] << 24 ) );
}

static const int MV_IMAADPCMStepTable[ 89 ] =
{
      7,     8,     9,    10,    11,    12,    13,    14,
     16,    17,    19,    21,    23,    25,    28,    31,
     34,    37,    41,    45,    50,    55,    60,    66,
     73,    80,    88,    97,   107,   118,   130,   143,
    157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,
    724,   796,   876,   963,  1060,  1166,  1282,  1411,
   1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
   3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
   7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
  32767,
};

static int MV_DecodeIMAADPCMSymbol( int predictor, int *index, int code, int bitsPerSample )
{
   int step = MV_IMAADPCMStepTable[ *index ];
   int delta;

   if ( bitsPerSample == 2 )
      {
      delta = step * ( code & 1 ) + ( step >> 1 );
      predictor += ( code & 2 ) ? -delta : delta;
      *index += ( ( code & 1 ) * 3 ) - 1;
      }
   else if ( bitsPerSample == 3 )
      {
      static const int indexTable3[ 4 ] = { -1, -1, 1, 2 };

      delta = step >> 2;
      if ( code & 1 ) delta += step >> 1;
      if ( code & 2 ) delta += step;

      predictor += ( code & 4 ) ? -delta : delta;
      *index += indexTable3[ code & 3 ];
      }
   else
      {
      static const int indexTable4[ 8 ] = { -1, -1, -1, -1, 2, 4, 6, 8 };

      delta = step >> 3;
      if ( code & 1 ) delta += step >> 2;
      if ( code & 2 ) delta += step >> 1;
      if ( code & 4 ) delta += step;

      predictor += ( code & 8 ) ? -delta : delta;
      *index += indexTable4[ code & 7 ];
      }

   predictor = max( predictor, -32768 );
   predictor = min( predictor, 32767 );

   *index = max( *index, 0 );
   *index = min( *index, 88 );

   return predictor;
}

static int MV_ReadIMAADPCMChannelByte(
   const uint8_t *encoded,
   int encodedBytes,
   int channels,
   int ch,
   int j,
   uint8_t *outByte )
{
   int byteIndex;

   if ( channels == 1 )
      {
      byteIndex = j;
      }
   else
      {
      byteIndex = ( j & ~3 ) * channels + ( ch * 4 ) + ( j & 3 );
      }

   if ( byteIndex < 0 || byteIndex >= encodedBytes )
      {
      return FALSE;
      }

   *outByte = encoded[ byteIndex ];
   return TRUE;
}

static int MV_ReadIMAADPCMSymbol(
   const uint8_t *encoded,
   int encodedBytes,
   int channels,
   int ch,
   int bitsPerSample,
   int symbolIndex,
   int *outCode )
{
   int bitPos = symbolIndex * bitsPerSample;
   int code = 0;

   for ( int b = 0; b < bitsPerSample; ++b )
      {
      int absoluteBit = bitPos + b;
      int bytePos = absoluteBit >> 3;
      int bitInByte = absoluteBit & 7;
      uint8_t byteVal;

      if ( !MV_ReadIMAADPCMChannelByte( encoded, encodedBytes, channels, ch, bytePos, &byteVal ) )
         {
         return FALSE;
         }

      code |= ( ( byteVal >> bitInByte ) & 1 ) << b;
      }

   *outCode = code;
   return TRUE;
}

static int MV_DecodeWAVIMAADPCM
   (
   const uint8_t *dataPtr,
   uint32_t dataSize,
   int channels,
   int blockAlign,
   int bitsPerSample,
   int samplesPerBlockFromFmt,
   int16_t **decodedOut,
   uint32_t *decodedSamplesPerChannelOut
   )

   {
   uint32_t offset;
   uint64_t totalSamples;
   uint64_t totalBytes;
   int16_t *decoded;
   uint32_t writeIndex;

   if ( decodedOut == NULL || decodedSamplesPerChannelOut == NULL )
      {
      return FALSE;
      }

   *decodedOut = NULL;
   *decodedSamplesPerChannelOut = 0;

   if ( dataPtr == NULL || dataSize == 0 || ( channels != 1 && channels != 2 ) || blockAlign <= 0 )
      {
      return FALSE;
      }

   if ( bitsPerSample != 2 && bitsPerSample != 3 && bitsPerSample != 4 )
      {
      return FALSE;
      }

   totalSamples = 0;
   offset = 0;

   while ( offset < dataSize )
      {
      uint32_t remainingBytes = dataSize - offset;
      uint32_t actualBlockSize = min( (uint32_t)blockAlign, remainingBytes );
      int headerSize = 4 * channels;
      int encodedBytes;
      int bytesPerChannel;
      int derivedSamplesPerBlock;
      int blockSamplesPerChannel;

      if ( actualBlockSize < (uint32_t)headerSize )
         {
         break;
         }

      encodedBytes = (int)actualBlockSize - headerSize;
      bytesPerChannel = encodedBytes / channels;
      derivedSamplesPerBlock = 1 + ( bytesPerChannel * 8 ) / bitsPerSample;

      if ( samplesPerBlockFromFmt > 0 )
         {
         blockSamplesPerChannel = min( samplesPerBlockFromFmt, derivedSamplesPerBlock );
         }
      else
         {
         blockSamplesPerChannel = derivedSamplesPerBlock;
         }

      if ( blockSamplesPerChannel > 0 )
         {
         totalSamples += (uint64_t)blockSamplesPerChannel * (uint64_t)channels;
         }

      offset += actualBlockSize;
      }

   if ( totalSamples == 0 )
      {
      return FALSE;
      }

   totalBytes = totalSamples * sizeof( int16_t );
   if ( totalBytes > 0xffffffffu )
      {
      return FALSE;
      }

   if ( USRHOOKS_GetMem( ( void ** )&decoded, (uint32_t)totalBytes ) != USRHOOKS_Ok || decoded == NULL )
      {
      return FALSE;
      }

   writeIndex = 0;
   offset = 0;

   while ( offset < dataSize )
      {
      const uint8_t *block = dataPtr + offset;
      uint32_t remainingBytes = dataSize - offset;
      uint32_t actualBlockSize = min( (uint32_t)blockAlign, remainingBytes );
      int headerSize = 4 * channels;
      int predictor[ 2 ] = { 0, 0 };
      int index[ 2 ] = { 0, 0 };
      const uint8_t *encoded;
      int encodedBytes;
      int bytesPerChannel;
      int derivedSamplesPerBlock;
      int blockSamplesPerChannel;

      if ( actualBlockSize < (uint32_t)headerSize )
         {
         break;
         }

      for ( int ch = 0; ch < channels; ++ch )
         {
         const uint8_t *hdr = block + ( ch * 4 );

         predictor[ ch ] = (int)(int16_t)MV_ReadLE16( hdr );
         index[ ch ] = (int)hdr[ 2 ];

         if ( index[ ch ] > 88 )
            {
            USRHOOKS_FreeMem( decoded );
            return FALSE;
            }
         }

      encoded = block + headerSize;
      encodedBytes = (int)actualBlockSize - headerSize;
      bytesPerChannel = encodedBytes / channels;
      derivedSamplesPerBlock = 1 + ( bytesPerChannel * 8 ) / bitsPerSample;

      if ( samplesPerBlockFromFmt > 0 )
         {
         blockSamplesPerChannel = min( samplesPerBlockFromFmt, derivedSamplesPerBlock );
         }
      else
         {
         blockSamplesPerChannel = derivedSamplesPerBlock;
         }

      if ( blockSamplesPerChannel > 0 )
         {
         for ( int ch = 0; ch < channels; ++ch )
            {
            decoded[ writeIndex++ ] = (int16_t)predictor[ ch ];
            }
         }

      for ( int sampleIndex = 1; sampleIndex < blockSamplesPerChannel; ++sampleIndex )
         {
         int symbolIndex = sampleIndex - 1;

         for ( int ch = 0; ch < channels; ++ch )
            {
            int code;

            if ( !MV_ReadIMAADPCMSymbol( encoded, encodedBytes, channels, ch,
               bitsPerSample, symbolIndex, &code ) )
               {
               USRHOOKS_FreeMem( decoded );
               return FALSE;
               }

            predictor[ ch ] = MV_DecodeIMAADPCMSymbol( predictor[ ch ], &index[ ch ], code, bitsPerSample );
            decoded[ writeIndex++ ] = (int16_t)predictor[ ch ];
            }
         }

      offset += actualBlockSize;
      }

   *decodedOut = decoded;
   *decodedSamplesPerChannelOut = writeIndex / (uint32_t)channels;

   return TRUE;
   }


/*---------------------------------------------------------------------
   Function: MV_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *MV_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case MV_Warning :
      case MV_Error :
         ErrorString = MV_ErrorString( MV_ErrorCode );
         break;

      case MV_Ok :
         ErrorString = "Multivoc ok.\n";
         break;

      case MV_UnsupportedCard :
         ErrorString = "Selected sound card is not supported by Multivoc.\n";
         break;

      case MV_NotInstalled :
         ErrorString = "Multivoc not installed.\n";
         break;

      case MV_NoVoices :
         ErrorString = "No free voices available to Multivoc.\n";
         break;

      case MV_NoMem :
         ErrorString = "Out of memory in Multivoc.\n";
         break;

      case MV_VoiceNotFound :
         ErrorString = "No voice with matching handle found.\n";
         break;

#ifdef PLAT_DOS
      case MV_BlasterError :
         ErrorString = BLASTER_ErrorString( BLASTER_Error );
         break;

      case MV_PasError :
         ErrorString = PAS_ErrorString( PAS_Error );
         break;

      case MV_SoundScapeError :
         ErrorString = SOUNDSCAPE_ErrorString( SOUNDSCAPE_Error );
         break;

      #ifndef SOUNDSOURCE_OFF
      case MV_SoundSourceError :
         ErrorString = SS_ErrorString( SS_Error );
         break;
      #endif
#endif

      case MV_DPMI_Error :
         ErrorString = "DPMI Error in Multivoc.\n";
         break;

      case MV_InvalidVOCFile :
         ErrorString = "Invalid VOC file passed in to Multivoc.\n";
         break;

      case MV_InvalidWAVFile :
         ErrorString = "Invalid WAV file passed in to Multivoc.\n";
         break;

      case MV_InvalidMixMode :
         ErrorString = "Invalid mix mode request in Multivoc.\n";
         break;

      case MV_SoundSourceFailure :
         ErrorString = "Sound Source playback failed.\n";
         break;

      case MV_IrqFailure :
         ErrorString = "Playback failed, possibly due to an invalid or conflicting IRQ.\n";
         break;

      case MV_DMAFailure :
         ErrorString = "Playback failed, possibly due to an invalid or conflicting DMA channel.\n";
         break;

      case MV_DMA16Failure :
         ErrorString = "Playback failed, possibly due to an invalid or conflicting DMA channel.  \n"
                       "Make sure the 16-bit DMA channel is correct.\n";
         break;

      case MV_NullRecordFunction :
         ErrorString = "Null record function passed to MV_StartRecording.\n";
         break;

      default :
         ErrorString = "Unknown Multivoc error code.\n";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define MV_LockStart MV_Mix


/*---------------------------------------------------------------------
   Function: MV_Mix

   Mixes the sound into the buffer.
---------------------------------------------------------------------*/

IRAM_ATTR static void MV_Mix( VoiceNode *voice )
{
   uint8_t        *start;
   int            length;
   long           voclength;
   unsigned long  position;
   unsigned long  rate;
   unsigned long  FixedPointBufferSize;

   if ( ( voice->length == 0 )
	   && ( voice->GetSound( voice ) != KeepPlaying ) )
      {
      return;
      }

   length               = MixBufferSize;
   FixedPointBufferSize = voice->FixedPointBufferSize;

   MV_MixDestination    = (char*)MV_FooBuffer;
   MV_LeftVolume        = voice->LeftVolume;
   MV_RightVolume       = voice->RightVolume;
   MV_GLast             = &voice->GLast;
   MV_GPos              = &voice->GPos;
   MV_GVal              = (int*)&voice->GVal;

   if ( ( MV_Channels == 2 ) && ( IS_QUIET( MV_LeftVolume ) ) )
      {
      MV_LeftVolume      = MV_RightVolume;
      MV_MixDestination += sizeof(*MV_FooBuffer);
      }

   // Add this voice to the mix
   while( length > 0 )
      {
      start    = (uint8_t *)voice->sound;
      rate     = voice->RateScale;
      position = voice->position;

      // Check if the last sample in this buffer would be
      // beyond the length of the sample block
      if ( ( position + FixedPointBufferSize ) >= voice->length )
         {
         if ( position < voice->length )
            {
            voclength = ( voice->length - position + rate - 1 ) / rate;
            }
         else
            {
            voice->GetSound( voice );
            return;
            }
         }
      else
         {
         voclength = length;
         }

      voice->mix( position, rate, (const char *)start, voclength );

      if ( voclength & 1 )
         {
         MV_MixPosition += rate;
         voclength -= 1;
         }
      voice->position = MV_MixPosition;

      length -= voclength;

      if ( voice->position >= voice->length )
         {
         // Get the next block of sound
         if ( voice->GetSound( voice ) != KeepPlaying )
            {
            return;
            }

         if ( length > 0 )
            {
            // Get the position of the last sample in the buffer
            FixedPointBufferSize = voice->RateScale * ( length - 1 );
            }
         }
      }
}


/*---------------------------------------------------------------------
   Function: MV_PlayVoice

   Adds a voice to the play list.
---------------------------------------------------------------------*/

IRAM_ATTR void MV_PlayVoice( VoiceNode *voice )
{
   unsigned flags;

   flags = DisableInterrupts();
   LL_SortedInsertion( (VoiceNode *)&VoiceList, voice, prev, next, VoiceNode, priority );

   ++sounddebugActiveSounds;
   ++sounddebugAllocateSoundCalls;

   RestoreInterrupts( flags );
}


/*---------------------------------------------------------------------
   Function: MV_StopVoice

   Removes the voice from the play list and adds it to the free list.
---------------------------------------------------------------------*/

IRAM_ATTR void MV_StopVoice( VoiceNode *voice )
{
   VoiceNode* pPrev;
   VoiceNode* pNext;
   void* rawdataptr = NULL;
   int ownsRawData = FALSE;

   unsigned  flags;

   flags = DisableInterrupts();


   pPrev = voice->prev;
   pNext = voice->next;

   ownsRawData = voice->ownsRawData;
   rawdataptr = voice->rawdataptr;
   voice->ownsRawData = FALSE;
   voice->rawdataptr = NULL;
   voice->rawdatasiz = 0;

   // move the voice from the play list to the free list
   LL_Remove( voice, next, prev );
   LL_Add( (VoiceNode *)&VoicePool, voice, next, prev );

   if(pPrev == NULL)
   {
	   printf("(MV_StopVoice) pPrev is NULL, this could be a problem.\n");
   }

   if(pNext == NULL)
   {
	   printf("(MV_StopVoice) pNext is NULL, this could be a problem.\n");
   }

   --sounddebugActiveSounds;
   ++sounddebugDeallocateSoundCalls;


   RestoreInterrupts( flags );

   if ( ownsRawData && rawdataptr != NULL )
      {
      USRHOOKS_FreeMem( rawdataptr );
      }
}


/*---------------------------------------------------------------------
   Function: MV_ServiceVoc

   Starts playback of the waiting buffer and mixes the next one.
---------------------------------------------------------------------*/

// static int backcolor = 1;

IRAM_ATTR void MV_ServiceVoc
(
 void
 )

{
	VoiceNode *voice;
	VoiceNode *next;


	// Toggle which buffer we'll mix next
	MV_MixPage++;
	if ( MV_MixPage >= MV_NumberOfBuffers )
	{
		MV_MixPage -= MV_NumberOfBuffers;
	}

	{
		ClearBuffer_DW( MV_FooBuffer, 0, sizeof(*MV_FooBuffer) / 4 * MV_BufferSize / MV_SampleSize * MV_Channels);
		MV_BufferEmpty[ MV_MixPage ] = TRUE;
	}

	// Play any waiting voices
	for( voice = VoiceList.next; voice != &VoiceList; voice = next )
	{
		//      if ( ( voice < &MV_Voices[ 0 ] ) || ( voice > &MV_Voices[ 8 ] ) )
		//         {
		//         SetBorderColor(backcolor++);
		//         break;
		//         }

		if(NULL == voice->GetSound)
		{
			#ifdef _DEBUG
				printf("MV_ServiceVoc() voice->GetSound == NULL, break;\n");
			#endif

			// This sound is null, early out, or face a nasty crash.
			break;
		}

		MV_BufferEmpty[ MV_MixPage ] = FALSE;

		MV_MixFunction( voice );

		next = voice->next;

		// Is this voice done?
		if ( !voice->Playing )
		{
			MV_StopVoice( voice );

			if ( MV_CallBackFunc )
            {
				MV_CallBackFunc( voice->callbackval );
            }
		}
	}

	if ( MV_ReverbLevel > 0)
	{
		if (MV_ReverbTable != -1) MV_FPReverb(MV_ReverbTable);
	}

	{
		char *dest;
		int   count;

		dest = MV_MixBuffer[ MV_MixPage ];
		count = MV_BufferSize / MV_SampleSize * MV_Channels;
		if ( MV_Bits == 16 )
		{
			MV_16BitDownmix(dest, count);
		}
		else
		{
			MV_8BitDownmix(dest, count);
		}

	}
}

/*---------------------------------------------------------------------
   Function: MV_GetNextVOCBlock

   Interperate the information of a VOC format sound file.
---------------------------------------------------------------------*/

IRAM_ATTR playbackstatus MV_GetNextVOCBlock
   (
   VoiceNode *voice
   )

   {
   unsigned char *ptr;
   int            blocktype;
   int            lastblocktype;
   uint32_t       blocklength = 0;
   unsigned long  samplespeed = 0;
   unsigned int   tc;
   int            packtype;
   int            voicemode;
   int            done;
   unsigned       BitsPerSample;
   unsigned       Channels;
   unsigned       Format;

   if ( voice->BlockLength > 0 )
      {
      voice->position    -= voice->length;
      voice->sound       += voice->length >> 16;
      if ( voice->bits == 16 )
         {
         voice->sound += voice->length >> 16;
         }
      voice->length       = min( voice->BlockLength, 0x8000 );
      voice->BlockLength -= voice->length;
      voice->length     <<= 16;
      return( KeepPlaying );
      }

   if ( ( voice->length > 0 ) && ( voice->LoopEnd != NULL ) &&
      ( voice->LoopStart != NULL ) )
      {
      voice->BlockLength  = voice->LoopSize;
      voice->sound        = voice->LoopStart;
      voice->position     = 0;
      voice->length       = min( voice->BlockLength, 0x8000 );
      voice->BlockLength -= voice->length;
      voice->length     <<= 16;
      return( KeepPlaying );
      }

   ptr = ( unsigned char * )voice->NextBlock;

   voice->Playing = TRUE;

   voicemode = 0;
   lastblocktype = 0;
   packtype = 0;

   done = FALSE;
   while( !done )
      {
      // Stop playing if we get a NULL pointer
      if ( ptr == NULL )
         {
         voice->Playing = FALSE;
         done = TRUE;
         break;
         }

      blocktype = ( int )*ptr;
      blocklength = ( *( unsigned long * )( ptr + 1 ) ) & 0x00ffffff;
      ptr += 4;

      switch( blocktype )
         {
         case 0 :
            // End of data
            if ( ( voice->LoopStart == NULL ) ||
               ( (unsigned char *)voice->LoopStart >= ( ptr - 4 ) ) )
               {
               voice->Playing = FALSE;
               done = TRUE;
               }
            else
               {
               voice->BlockLength  = ( ptr - 4 ) - (unsigned char *)voice->LoopStart;
               voice->sound        = voice->LoopStart;
               voice->position     = 0;
               voice->length       = min( voice->BlockLength, 0x8000 );
               voice->BlockLength -= voice->length;
               voice->length     <<= 16;
               return( KeepPlaying );
               }
            break;

         case 1 :
            // Sound data block
            voice->bits  = 8;
            if ( lastblocktype != 8 )
               {
               tc = ( unsigned int )*ptr << 8;
               packtype = *( ptr + 1 );
               }

            ptr += 2;
            blocklength -= 2;

            samplespeed = 256000000L / ( 65536 - tc );

            // Skip packed or stereo data
            if ( ( packtype != 0 ) || ( voicemode != 0 ) )
               {
               ptr += blocklength;
               }
            else
               {
               done = TRUE;
               }
            voicemode = 0;
            break;

         case 2 :
            // Sound continuation block
            samplespeed = voice->SamplingRate;
            done = TRUE;
            break;

         case 3 :
            // Silence
            // Not implimented.
            ptr += blocklength;
            break;

         case 4 :
            // Marker
            // Not implimented.
            ptr += blocklength;
            break;

         case 5 :
            // ASCII string
            // Not implimented.
            ptr += blocklength;
            break;

         case 6 :
            // Repeat begin
            if ( voice->LoopEnd == NULL )
               {
               voice->LoopCount = *( uint16_t * )ptr;
               voice->LoopStart = (char *)(ptr + blocklength);
               }
            ptr += blocklength;
            break;

         case 7 :
            // Repeat end
            ptr += blocklength;
            if ( lastblocktype == 6 )
               {
               voice->LoopCount = 0;
               }
            else
               {
               if ( ( voice->LoopCount > 0 ) && ( voice->LoopStart != NULL ) )
                  {
                  ptr = (uint8_t *)voice->LoopStart;
                  if ( voice->LoopCount < 0xffff )
                     {
                     voice->LoopCount--;
                     if ( voice->LoopCount == 0 )
                        {
                        voice->LoopStart = NULL;
                        }
                     }
                  }
               }
            break;

         case 8 :
            // Extended block
            voice->bits  = 8;
            tc = *( uint16_t * )ptr;
            packtype = *( ptr + 2 );
            voicemode = *( ptr + 3 );
            ptr += blocklength;
            break;

         case 9 :
            // New sound data block

            samplespeed = *( unsigned long * )ptr;
            BitsPerSample = ( unsigned )*( ptr + 4 );
            Channels = ( unsigned )*( ptr + 5 );
            Format = ( unsigned )*( uint16_t * )( ptr + 6 );

            if ( ( BitsPerSample == 8 ) && ( Channels == 1 ) &&
               ( Format == VOC_8BIT ) )
               {
               ptr         += 12;
               blocklength -= 12;
               voice->bits  = 8;
               done         = TRUE;
               }
            else if ( ( BitsPerSample == 16 ) && ( Channels == 1 ) &&
               ( Format == VOC_16BIT ) )
               {
               ptr         += 12;
               blocklength -= 12;
               voice->bits  = 16;
               done         = TRUE;
               }
            else
               {
               ptr += blocklength;
               }
            break;

         default :
            // Unknown data.  Probably not a VOC file.
            voice->Playing = FALSE;
            done = TRUE;
            break;
         }

      lastblocktype = blocktype;
      }

   if ( voice->Playing )
      {
      voice->NextBlock    = (char *)(ptr + blocklength);
      voice->sound        = (char *)ptr;

      voice->SamplingRate = samplespeed;
      voice->RateScale    = ( voice->SamplingRate * voice->PitchScale ) / MV_MixRate;

      // Multiply by MixBufferSize - 1
      voice->FixedPointBufferSize = ( voice->RateScale * MixBufferSize ) -
         voice->RateScale;

      if ( voice->LoopEnd != NULL )
         {
         if ( blocklength > ( unsigned long )voice->LoopEnd )
            {
            blocklength = ( unsigned long )voice->LoopEnd;
            }
         else
            {
            voice->LoopEnd = ( char * )blocklength;
            }

         voice->LoopStart = voice->sound + ( unsigned long )voice->LoopStart;
         voice->LoopEnd   = voice->sound + ( unsigned long )voice->LoopEnd;
         voice->LoopSize  = voice->LoopEnd - voice->LoopStart;
         }

      if ( voice->bits == 16 )
         {
         blocklength /= 2;
         }

      voice->position     = 0;
      voice->length       = min( blocklength, 0x8000 );
      voice->BlockLength  = blocklength - voice->length;
      voice->length     <<= 16;

      MV_SetVoiceMixMode( voice );

      return( KeepPlaying );
      }

   return( NoMoreData );
   }


/*---------------------------------------------------------------------
   Function: MV_GetNextDemandFeedBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

IRAM_ATTR playbackstatus MV_GetNextDemandFeedBlock
   (
   VoiceNode *voice
   )

   {
   if ( voice->BlockLength > 0 )
      {
      voice->position    -= voice->length;
      voice->sound       += voice->length >> 16;
      voice->length       = min( voice->BlockLength, 0x8000 );
      voice->BlockLength -= voice->length;
      voice->length     <<= 16;

      return( KeepPlaying );
      }

   if ( voice->DemandFeed == NULL )
      {
      return( NoMoreData );
      }

   voice->position     = 0;
   ( voice->DemandFeed )( &voice->sound, &voice->BlockLength );
   voice->length       = min( voice->BlockLength, 0x8000 );
   voice->BlockLength -= voice->length;
   voice->length     <<= 16;

   if ( ( voice->length > 0 ) && ( voice->sound != NULL ) )
      {
      return( KeepPlaying );
      }
   return( NoMoreData );
   }


/*---------------------------------------------------------------------
   Function: MV_GetNextRawBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

IRAM_ATTR playbackstatus MV_GetNextRawBlock
   (
   VoiceNode *voice
   )

   {
   if ( voice->BlockLength <= 0 )
      {
      if ( voice->LoopStart == NULL )
         {
         voice->Playing = FALSE;
         return( NoMoreData );
         }

      voice->BlockLength = voice->LoopSize;
      voice->NextBlock   = voice->LoopStart;
      voice->length = 0;
      voice->position = 0;
      }

   voice->sound        = voice->NextBlock;
   voice->position    -= voice->length;
   voice->length       = min( voice->BlockLength, 0x8000 );
   voice->NextBlock   += voice->length;
   if ( voice->bits == 16 )
      {
      voice->NextBlock += voice->length;
      }
   voice->BlockLength -= voice->length;
   voice->length     <<= 16;

   return( KeepPlaying );
   }


/*---------------------------------------------------------------------
   Function: MV_GetNextWAVBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

IRAM_ATTR playbackstatus MV_GetNextWAVBlock
   (
   VoiceNode *voice
   )

   {
   if ( voice->BlockLength <= 0 )
      {
      if ( voice->LoopStart == NULL )
         {
         voice->Playing = FALSE;
         return( NoMoreData );
         }

      voice->BlockLength = voice->LoopSize;
      voice->NextBlock   = voice->LoopStart;
      voice->length      = 0;
      voice->position    = 0;
      }

   voice->sound        = voice->NextBlock;
   voice->position    -= voice->length;
   voice->length       = min( voice->BlockLength, 0x8000 );
   voice->NextBlock   += voice->length;
   if ( voice->bits == 16 )
      {
      voice->NextBlock += voice->length;
      }
   voice->BlockLength -= voice->length;
   voice->length     <<= 16;

   return( KeepPlaying );
   }


/*---------------------------------------------------------------------
   Function: MV_ServiceRecord

   Starts recording of the waiting buffer.
---------------------------------------------------------------------*/

#ifdef PLAT_DOS
static void MV_ServiceRecord
   (
   void
   )

   {
   if ( MV_RecordFunc )
      {
      MV_RecordFunc( MV_MixBuffer[ 0 ] + MV_MixPage * MixBufferSize,
         MixBufferSize );
      }

   // Toggle which buffer we'll mix next
   MV_MixPage++;
   if ( MV_MixPage >= NumberOfBuffers )
      {
      MV_MixPage = 0;
      }
   }
#endif


/*---------------------------------------------------------------------
   Function: MV_GetVoice

   Locates the voice with the specified handle.
---------------------------------------------------------------------*/

VoiceNode *MV_GetVoice
   (
   int handle
   )

   {
   VoiceNode *voice;
   unsigned  flags;

   flags = DisableInterrupts();

   for( voice = VoiceList.next; voice != &VoiceList; voice = voice->next )
      {
      if ( handle == voice->handle )
         {
         break;
         }
      }

   RestoreInterrupts( flags );

   if ( voice == &VoiceList )
      {
      MV_SetErrorCode( MV_VoiceNotFound );

      // SBF - should this return null?
      return NULL;
      }

   return( voice );
   }


/*---------------------------------------------------------------------
   Function: MV_VoicePlaying

   Checks if the voice associated with the specified handle is
   playing.
---------------------------------------------------------------------*/

int MV_VoicePlaying
   (
   int handle
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( FALSE );
      }

   voice = MV_GetVoice( handle );

   if ( voice == NULL )
      {
      return( FALSE );
      }

   return( TRUE );
   }


/*---------------------------------------------------------------------
   Function: MV_KillAllVoices

   Stops output of all currently active voices.
---------------------------------------------------------------------*/

int MV_KillAllVoices
   (
   void
   )

   {
   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   // Remove all the voices from the list
   while( VoiceList.next != &VoiceList )
      {
      MV_Kill( VoiceList.next->handle );
      }

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_Kill

   Stops output of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int MV_Kill
   (
   int handle
   )

   {
   VoiceNode *voice;
   unsigned  flags;
   unsigned  long callbackval;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   flags = DisableInterrupts();

   voice = MV_GetVoice( handle );
   if ( voice == NULL )
      {
      RestoreInterrupts( flags );
      MV_SetErrorCode( MV_VoiceNotFound );
      return( MV_Error );
      }

   callbackval = voice->callbackval;

   MV_StopVoice( voice );

   RestoreInterrupts( flags );

   if ( MV_CallBackFunc )
      {
      MV_CallBackFunc( callbackval );
      }

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_VoicesPlaying

   Determines the number of currently active voices.
---------------------------------------------------------------------*/

int MV_VoicesPlaying
   (
   void
   )

   {
   VoiceNode   *voice;
   int         NumVoices = 0;
   unsigned    flags;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( 0 );
      }

   flags = DisableInterrupts();

   for( voice = VoiceList.next; voice != &VoiceList; voice = voice->next )
      {
      NumVoices++;
      }

   RestoreInterrupts( flags );

   return( NumVoices );
   }


/*---------------------------------------------------------------------
   Function: MV_AllocVoice

   Retrieve an inactive or lower priority voice for output.
---------------------------------------------------------------------*/

VoiceNode *MV_AllocVoice
   (
   int priority
   )

   {
   VoiceNode   *voice;
   VoiceNode   *node;
   unsigned    flags;

//return( NULL );
   if ( MV_Recording )
      {
      return( NULL );
      }

   flags = DisableInterrupts();

   // Check if we have any free voices
   if ( LL_Empty( &VoicePool, next, prev ) )
      {
      // check if we have a higher priority than a voice that is playing.
      voice = VoiceList.next;
      for( node = voice->next; node != &VoiceList; node = node->next )
         {
         if ( node->priority < voice->priority )
            {
            voice = node;
            }
         }

      if ( priority >= voice->priority )
         {
         MV_Kill( voice->handle );
         }
      }

   // Check if any voices are in the voice pool
   if ( LL_Empty( &VoicePool, next, prev ) )
      {
      // No free voices
      RestoreInterrupts( flags );
      return( NULL );
      }

   voice = VoicePool.next;
   LL_Remove( voice, next, prev );
   RestoreInterrupts( flags );

   // Find a free voice handle
   do
      {
      MV_VoiceHandle++;
      if ( MV_VoiceHandle < MV_MinVoiceHandle )
         {
         MV_VoiceHandle = MV_MinVoiceHandle;
         }
      }
   while( MV_VoicePlaying( MV_VoiceHandle ) );

   voice->handle = MV_VoiceHandle;
   voice->rawdataptr = NULL;
   voice->rawdatasiz = 0;
   voice->ownsRawData = FALSE;

   return( voice );
   }


/*---------------------------------------------------------------------
   Function: MV_VoiceAvailable

   Checks if a voice can be play at the specified priority.
---------------------------------------------------------------------*/

int MV_VoiceAvailable
   (
   int priority
   )

   {
   VoiceNode   *voice;
   VoiceNode   *node;
   unsigned    flags;

   // Check if we have any free voices
   if ( !LL_Empty( &VoicePool, next, prev ) )
      {
      return( TRUE );
      }

   flags = DisableInterrupts();

   // check if we have a higher priority than a voice that is playing.
   voice = VoiceList.next;
   for( node = VoiceList.next; node != &VoiceList; node = node->next )
      {
      if ( node->priority < voice->priority )
         {
         voice = node;
         }
      }

   RestoreInterrupts( flags );

   if ( ( voice != &VoiceList ) && ( priority >= voice->priority ) )
      {
      return( TRUE );
      }

   return( FALSE );
   }


/*---------------------------------------------------------------------
   Function: MV_SetVoicePitch

   Sets the pitch for the specified voice.
---------------------------------------------------------------------*/

void MV_SetVoicePitch
   (
   VoiceNode *voice,
   unsigned long rate,
   int pitchoffset
   )

   {
   voice->SamplingRate = rate;
   voice->PitchScale   = PITCH_GetScale( pitchoffset );
   voice->RateScale    = ( rate * voice->PitchScale ) / MV_MixRate;

   // Multiply by MixBufferSize - 1
   voice->FixedPointBufferSize = ( voice->RateScale * MixBufferSize ) -
      voice->RateScale;

   }


/*---------------------------------------------------------------------
   Function: MV_SetPitch

   Sets the pitch for the voice associated with the specified handle.
---------------------------------------------------------------------*/

int MV_SetPitch
   (
   int handle,
   int pitchoffset
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   voice = MV_GetVoice( handle );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_VoiceNotFound );
      return( MV_Error );
      }

   MV_SetVoicePitch( voice, voice->SamplingRate, pitchoffset );

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_SetFrequency

   Sets the frequency for the voice associated with the specified handle.
---------------------------------------------------------------------*/

int MV_SetFrequency
   (
   int handle,
   int frequency
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   voice = MV_GetVoice( handle );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_VoiceNotFound );
      return( MV_Error );
      }

   MV_SetVoicePitch( voice, frequency, 0 );

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_GetVolumeTable

   Returns a pointer to the volume table associated with the specified
   volume.
---------------------------------------------------------------------*/

#if 0
static short *MV_GetVolumeTable
   (
   int vol
   )

   {
   int volume;
   short *table;

   volume = MIX_VOLUME( vol );

   table = &MV_VolumeTable[ volume ];

   return( table );
   }
#else
static int MV_GetVolumeTable
   (
   int vol
   )

   {
   int volume;

   volume = MIX_VOLUME( vol );

   return ( volume * MV_TotalVolume ) / MV_MaxTotalVolume;
   }
#endif

/*---------------------------------------------------------------------
   Function: MV_SetVoiceMixMode

   Selects which method should be used to mix the voice.
---------------------------------------------------------------------*/

void MV_SetVoiceMixMode
   (
   VoiceNode *voice
   )

   {
   unsigned flags;
   int test;

   flags = DisableInterrupts();

   test = T_DEFAULT;
   if ( voice->bits == 16 )
      {
      test |= T_16BITSOURCE;
      }

   if ( MV_Channels == 1 )
      {
      test |= T_MONO;
      }
   else
      {
      if ( IS_QUIET( voice->RightVolume ) )
         {
         test |= T_RIGHTQUIET;
         }
      else if ( IS_QUIET( voice->LeftVolume ) )
         {
         test |= T_LEFTQUIET;
         }
      }

   switch( test )
      {
      case T_MONO | T_16BITSOURCE :
         voice->mix = MV_MixFPMono16;
         break;

      case T_MONO :
         voice->mix = MV_MixFPMono8;
         break;

      case T_16BITSOURCE | T_LEFTQUIET :
         MV_LeftVolume = MV_RightVolume;
         voice->mix = MV_MixFPMono16;
         break;

      case T_LEFTQUIET :
         MV_LeftVolume = MV_RightVolume;
         voice->mix = MV_MixFPMono8;
         break;

      case T_16BITSOURCE | T_RIGHTQUIET :
         voice->mix = MV_MixFPMono16;
         break;

      case T_RIGHTQUIET :
         voice->mix = MV_MixFPMono8;
         break;

      case T_16BITSOURCE :
         voice->mix = MV_MixFPStereo16;
         break;

      case T_SIXTEENBIT_STEREO :
         voice->mix = MV_MixFPStereo8;
         break;

      default :
         voice->mix = MV_MixFPMono8;
      }
   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: MV_SetVoiceVolume

   Sets the stereo and mono volume level of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

void MV_SetVoiceVolume
   (
   VoiceNode *voice,
   int vol,
   int left,
   int right
   )

   {
   if ( MV_Channels == 1 )
      {
      left  = vol;
      right = vol;
      }

   if ( MV_SwapLeftRight )
      {
      // SBPro uses reversed panning
      voice->LeftVolume  = MV_GetVolumeTable( right );
      voice->RightVolume = MV_GetVolumeTable( left );
      }
   else
      {
      voice->LeftVolume  = MV_GetVolumeTable( left );
      voice->RightVolume = MV_GetVolumeTable( right );
      }

   MV_SetVoiceMixMode( voice );
   }


/*---------------------------------------------------------------------
   Function: MV_EndLooping

   Stops the voice associated with the specified handle from looping
   without stoping the sound.
---------------------------------------------------------------------*/

int MV_EndLooping
   (
   int handle
   )

   {
   VoiceNode *voice;
   unsigned flags;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   flags = DisableInterrupts();

   voice = MV_GetVoice( handle );
   if ( voice == NULL )
      {
      RestoreInterrupts( flags );
      MV_SetErrorCode( MV_VoiceNotFound );
      return( MV_Warning );
      }

   voice->LoopCount = 0;
   voice->LoopStart = NULL;
   voice->LoopEnd   = NULL;

   RestoreInterrupts( flags );

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_SetPan

   Sets the stereo and mono volume level of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int MV_SetPan
   (
   int handle,
   int vol,
   int left,
   int right
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   voice = MV_GetVoice( handle );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_VoiceNotFound );
      return( MV_Warning );
      }

   MV_SetVoiceVolume( voice, vol, left, right );

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_Pan3D

   Set the angle and distance from the listener of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

int MV_Pan3D
   (
   int handle,
   int angle,
   int distance
   )

   {
   int left;
   int right;
   int mid;
   int volume;
   int status;

   if ( distance < 0 )
      {
      distance  = -distance;
      angle    += MV_NumPanPositions / 2;
      }

   volume = MIX_VOLUME( distance );

   // Ensure angle is within 0 - 31
   angle &= MV_MaxPanPosition;

   left  = MV_PanTable[ angle ][ volume ].left;
   right = MV_PanTable[ angle ][ volume ].right;
   mid   = max( 0, 255 - distance );

   status = MV_SetPan( handle, mid, left, right );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_SetReverb

   Sets the level of reverb to add to mix.
---------------------------------------------------------------------*/

void MV_SetReverb
   (
   int reverb
   )

   {
   MV_ReverbLevel = MIX_VOLUME( reverb );
   MV_ReverbTable = ( MV_ReverbLevel * MV_TotalVolume ) / MV_MaxTotalVolume;// &MV_VolumeTable[ MV_ReverbLevel ];
   if (!reverb) MV_FPReverbFree();
   }


/*---------------------------------------------------------------------
   Function: MV_SetFastReverb

   Sets the level of reverb to add to mix.
---------------------------------------------------------------------*/

void MV_SetFastReverb
   (
   int reverb
   )

   {
   MV_ReverbLevel = max( 0, min( 16, reverb ) );
   MV_ReverbTable = -1;
   }


/*---------------------------------------------------------------------
   Function: MV_GetMaxReverbDelay

   Returns the maximum delay time for reverb.
---------------------------------------------------------------------*/

int MV_GetMaxReverbDelay
   (
   void
   )

   {
   int maxdelay;

   maxdelay = 65536; //MixBufferSize * MV_NumberOfBuffers;

   return maxdelay;
   }


/*---------------------------------------------------------------------
   Function: MV_GetReverbDelay

   Returns the current delay time for reverb.
---------------------------------------------------------------------*/

int MV_GetReverbDelay
   (
   void
   )

   {
   return MV_ReverbDelay; // / MV_SampleSize;
   }


/*---------------------------------------------------------------------
   Function: MV_SetReverbDelay

   Sets the delay level of reverb to add to mix.
---------------------------------------------------------------------*/

void MV_SetReverbDelay
   (
   int delay
   )

   {
   int maxdelay;

   if (!delay) // blah, ignore
      return;
   maxdelay = MV_GetMaxReverbDelay();
   MV_ReverbDelay = max( MixBufferSize, min( delay, maxdelay ) );
   // MV_ReverbDelay *= MV_SampleSize;
   }


/*---------------------------------------------------------------------
   Function: MV_SetMixMode

   Prepares Multivoc to play stereo of mono digitized sounds.
---------------------------------------------------------------------*/

int MV_SetMixMode
   (
   int numchannels,
   int samplebits
   )

   {
   int mode;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   mode = 0;
   if ( numchannels == 2 )
      {
      mode |= STEREO;
      }
   if ( samplebits == 16 )
      {
      mode |= SIXTEEN_BIT;
      }

#ifdef PLAT_DOS
   switch( MV_SoundCard )
      {
      case UltraSound :
         MV_MixMode = mode;
         break;

      case SoundBlaster :
      case Awe32 :
         MV_MixMode = BLASTER_SetMixMode( mode );
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         MV_MixMode = PAS_SetMixMode( mode );
         break;

      case SoundScape :
         MV_MixMode = SOUNDSCAPE_SetMixMode( mode );
         break;

      #ifndef SOUNDSOURCE_OFF
      case SoundSource :
      case TandySoundSource :
         MV_MixMode = SS_SetMixMode( mode );
         break;
      #endif
      }
#else
   MV_MixMode = mode;
#endif

   MV_Channels = 1;
   if ( MV_MixMode & STEREO )
      {
      MV_Channels = 2;
      }

   MV_Bits = 8;
   if ( MV_MixMode & SIXTEEN_BIT )
      {
      MV_Bits = 16;
      }

   MV_BuffShift  = 7 + MV_Channels;
   MV_SampleSize = sizeof( MONO8 ) * MV_Channels;

   if ( MV_Bits == 8 )
      {
      MV_Silence = SILENCE_8BIT;
      }
   else
      {
      MV_Silence     = SILENCE_16BIT;
      MV_BuffShift  += 1;
      MV_SampleSize *= 2;
      }

   MV_BufferSize = MixBufferSize * MV_SampleSize;
   MV_NumberOfBuffers = TotalBufferSize / MV_BufferSize;
   MV_BufferLength = TotalBufferSize;

   MV_RightChannelOffset = MV_SampleSize / 2;

#ifdef PLAT_DOS
   if ( ( MV_SoundCard == UltraSound ) && ( MV_Channels == 2 ) )
      {
      MV_SampleSize         /= 2;
      MV_BufferSize         /= 2;
      MV_RightChannelOffset  = MV_BufferSize * MV_NumberOfBuffers;
      MV_BufferLength       /= 2;
      }
#endif

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_StartPlayback

   Starts the sound playback engine.
---------------------------------------------------------------------*/

int MV_StartPlayback()

   {
   int status;
   int buffer;

   // Initialize the buffers
   ClearBuffer_DW( MV_MixBuffer[ 0 ], MV_Silence, TotalBufferSize >> 2 );
   for( buffer = 0; buffer < MV_NumberOfBuffers; buffer++ )
      {
      MV_BufferEmpty[ buffer ] = TRUE;
      }

   // Set the mix buffer variables
   MV_MixPage = 1;

   MV_MixFunction = MV_Mix;

//JIM
//   MV_MixRate = MV_RequestedMixRate;
//   return( MV_Ok );

   // Start playback
    status = DSL_BeginBufferedPlayback( MV_MixBuffer[ 0 ],
										TotalBufferSize,
										MV_NumberOfBuffers,
										MV_RequestedMixRate,
										MV_MixMode,
										MV_ServiceVoc );

    if ( status != DSL_Ok )
    {
		MV_SetErrorCode( MV_BlasterError );
		return( MV_Error );
    }

    MV_MixRate = DSL_GetPlaybackRate();

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_StopPlayback

   Stops the sound playback engine.
---------------------------------------------------------------------*/

void MV_StopPlayback
   (
   void
   )

   {
   VoiceNode   *voice;
   VoiceNode   *next;
   unsigned    flags;

#ifdef PLAT_DOS
   // Stop sound playback
   switch( MV_SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
         BLASTER_StopPlayback();
         break;

      case UltraSound :
         GUSWAVE_KillAllVoices();
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         PAS_StopPlayback();
         break;

      case SoundScape :
         SOUNDSCAPE_StopPlayback();
         break;

      #ifndef SOUNDSOURCE_OFF
      case SoundSource :
      case TandySoundSource :
         SS_StopPlayback();
         break;
      #endif
      }
#else
   DSL_StopPlayback();
#endif

   // Make sure all callbacks are done.
   flags = DisableInterrupts();

   for( voice = VoiceList.next; voice != &VoiceList; voice = next )
      {
      next = voice->next;

      MV_StopVoice( voice );

      if ( MV_CallBackFunc )
         {
         MV_CallBackFunc( voice->callbackval );
         }
      }

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: MV_StartRecording

   Starts the sound recording engine.
---------------------------------------------------------------------*/

int MV_StartRecording
   (
   int MixRate,
   void ( *function )( char *ptr, int length )
   )

   {
#ifdef PLAT_DOS
   int status;

   switch( MV_SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
      case ProAudioSpectrum :
      case SoundMan16 :
         break;

      default :
         MV_SetErrorCode( MV_UnsupportedCard );
         return( MV_Error );
         break;
      }

   if ( function == NULL )
      {
      MV_SetErrorCode( MV_NullRecordFunction );
      return( MV_Error );
      }

   MV_StopPlayback();

   // Initialize the buffers
   ClearBuffer_DW( MV_MixBuffer[ 0 ], SILENCE_8BIT, TotalBufferSize >> 2 );

   // Set the mix buffer variables
   MV_MixPage  = 0;

   MV_RecordFunc = function;

   // Start playback
   switch( MV_SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
         status = BLASTER_BeginBufferedRecord( MV_MixBuffer[ 0 ],
            TotalBufferSize, NumberOfBuffers, MixRate, MONO_8BIT,
            MV_ServiceRecord );

         if ( status != BLASTER_Ok )
            {
            MV_SetErrorCode( MV_BlasterError );
            return( MV_Error );
            }
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         status = PAS_BeginBufferedRecord( MV_MixBuffer[ 0 ],
            TotalBufferSize, NumberOfBuffers, MixRate, MONO_8BIT,
            MV_ServiceRecord );

         if ( status != PAS_Ok )
            {
            MV_SetErrorCode( MV_PasError );
            return( MV_Error );
            }
         break;
      }

   MV_Recording = TRUE;
   return( MV_Ok );
#else
   MV_SetErrorCode( MV_UnsupportedCard );
   return( MV_Error );
#endif
   }


/*---------------------------------------------------------------------
   Function: MV_StopRecord

   Stops the sound record engine.
---------------------------------------------------------------------*/

void MV_StopRecord
   (
   void
   )

   {
#ifdef PLAT_DOS
   // Stop sound playback
   switch( MV_SoundCard )
      {
      case SoundBlaster :
      case Awe32 :
         BLASTER_StopPlayback();
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         PAS_StopPlayback();
         break;
      }

   MV_Recording = FALSE;
   MV_StartPlayback();
#endif
   }


/*---------------------------------------------------------------------
   Function: MV_StartDemandFeedPlayback

   Plays a digitized sound from a user controlled buffering system.
---------------------------------------------------------------------*/

int MV_StartDemandFeedPlayback
   (
   void ( *function )( char **ptr, uint32_t *length ),
   int rate,
   int pitchoffset,
   int vol,
   int left,
   int right,
   int priority,
   unsigned long callbackval
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   // Request a voice from the voice pool
   voice = MV_AllocVoice( priority );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_NoVoices );
      return( MV_Error );
      }

   voice->wavetype    = DemandFeed;
   voice->bits        = 8;
   voice->GetSound    = MV_GetNextDemandFeedBlock;
   voice->NextBlock   = NULL;
   voice->DemandFeed  = function;
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->GLast       = -1;
   voice->GPos        = 0;
   voice->GVal[0]     = 0;
   voice->GVal[1]     = 0;
   voice->GVal[2]     = 0;
   voice->GVal[3]     = 0;
   voice->BlockLength = 0;
   voice->position    = 0;
   voice->sound       = NULL;
   voice->length      = 0;
   voice->BlockLength = 0;
   voice->Playing     = TRUE;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->callbackval = callbackval;

   MV_SetVoicePitch( voice, rate, pitchoffset );
   MV_SetVoiceVolume( voice, vol, left, right );
   MV_PlayVoice( voice );

   return( voice->handle );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayRaw

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayRaw
   (
   uint8_t *ptr,
   unsigned long length,
   unsigned rate,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   unsigned long callbackval
   )

   {
   int status;

   status = MV_PlayLoopedRaw( ptr, length, NULL, NULL, rate, pitchoffset,
      vol, left, right, priority, callbackval );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayLoopedRaw

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayLoopedRaw
   (
   uint8_t *ptr,
   unsigned long length,
   uint8_t *loopstart,
   uint8_t *loopend,
   unsigned rate,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   unsigned long callbackval
   )

   {
   VoiceNode *voice;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   // Request a voice from the voice pool
   voice = MV_AllocVoice( priority );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_NoVoices );
      return( MV_Error );
      }

   voice->wavetype    = Raw;
   voice->bits        = 8;
   voice->GetSound    = MV_GetNextRawBlock;
   voice->Playing     = TRUE;
   voice->NextBlock   = (char *)ptr;
   voice->position    = 0;
   voice->BlockLength = length;
   voice->length      = 0;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->GLast       = -1;
   voice->GPos        = 0;
   voice->GVal[0]     = 0;
   voice->GVal[1]     = 0;
   voice->GVal[2]     = 0;
   voice->GVal[3]     = 0;
   voice->callbackval = callbackval;
   voice->LoopStart   = (char *)loopstart;
   voice->LoopEnd     = (char *)loopend;
   voice->LoopSize    = ( voice->LoopEnd - voice->LoopStart ) + 1;

   MV_SetVoicePitch( voice, rate, pitchoffset );
   MV_SetVoiceVolume( voice, vol, left, right );
   MV_PlayVoice( voice );

   return( voice->handle );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayWAV

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayWAV
   (
   uint8_t *ptr,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   unsigned long callbackval
   )

   {
   int status;

   status = MV_PlayLoopedWAV( ptr, -1, -1, pitchoffset, vol, left, right,
      priority, callbackval );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayWAV3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int MV_PlayWAV3D
   (
   uint8_t *ptr,
   int  pitchoffset,
   int  angle,
   int  distance,
   int  priority,
   unsigned long callbackval
   )

   {
   int left;
   int right;
   int mid;
   int volume;
   int status;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   if ( distance < 0 )
      {
      distance  = -distance;
      angle    += MV_NumPanPositions / 2;
      }

   volume = MIX_VOLUME( distance );

   // Ensure angle is within 0 - 31
   angle &= MV_MaxPanPosition;

   left  = MV_PanTable[ angle ][ volume ].left;
   right = MV_PanTable[ angle ][ volume ].right;
   mid   = max( 0, 255 - distance );

   status = MV_PlayWAV( ptr, pitchoffset, mid, left, right, priority,
      callbackval );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayLoopedWAV

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayLoopedWAV
   (
   uint8_t *ptr,
   long  loopstart,
   long  loopend,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   unsigned long callbackval
   )

   {
   const uint8_t *fmtData = NULL;
   uint32_t fmtSize = 0;
   const uint8_t *dataPtr = NULL;
   uint32_t dataSize = 0;
   uint32_t wavLength;
   uint32_t offset;

   uint16_t wFormatTag;
   uint16_t nChannels;
   uint32_t nSamplesPerSec;
   uint16_t nBlockAlign;
   uint16_t nBitsPerSample;

   VoiceNode *voice;
   int bits;
   char *nextBlock;
   unsigned long blockLength;
   unsigned long sampleLength;
   long loopstartBytes;
   long loopendBytes;
   long absloopstart;
   long absloopend;

   int16_t *decodedPCM = NULL;
   uint32_t decodedSamplesPerChannel = 0;
   uint32_t decodedBytes = 0;
   int ownsRawData = FALSE;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   if ( ptr == NULL )
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   if ( memcmp( ptr, "RIFF", 4 ) != 0 || memcmp( ptr + 8, "WAVE", 4 ) != 0 )
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   wavLength = MV_ReadLE32( ptr + 4 ) + 8;
   if ( wavLength < 12 )
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   offset = 12;
   while ( offset + 8 <= wavLength )
      {
      const uint8_t *chunk = ptr + offset;
      uint32_t chunkSize = MV_ReadLE32( chunk + 4 );
      uint32_t chunkDataOffset = offset + 8;
      uint32_t chunkPaddedSize = chunkSize + ( chunkSize & 1 );

      if ( chunkDataOffset > wavLength || chunkSize > wavLength - chunkDataOffset )
         {
         MV_SetErrorCode( MV_InvalidWAVFile );
         return( MV_Error );
         }

      if ( memcmp( chunk, "fmt ", 4 ) == 0 )
         {
         fmtData = chunk + 8;
         fmtSize = chunkSize;
         }
      else if ( memcmp( chunk, "data", 4 ) == 0 )
         {
         dataPtr = chunk + 8;
         dataSize = chunkSize;
         }

      if ( chunkDataOffset + chunkPaddedSize < chunkDataOffset )
         {
         MV_SetErrorCode( MV_InvalidWAVFile );
         return( MV_Error );
         }

      if ( fmtData != NULL && dataPtr != NULL )
         {
         break;
         }

      offset = chunkDataOffset + chunkPaddedSize;
      }

   if ( fmtData == NULL || fmtSize < 16 || dataPtr == NULL || dataSize == 0 )
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   wFormatTag = MV_ReadLE16( fmtData + 0 );
   nChannels = MV_ReadLE16( fmtData + 2 );
   nSamplesPerSec = MV_ReadLE32( fmtData + 4 );
   nBlockAlign = MV_ReadLE16( fmtData + 12 );
   nBitsPerSample = MV_ReadLE16( fmtData + 14 );

   if ( nChannels < 1 || nChannels > 2 || nBlockAlign == 0 )
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   if ( wFormatTag == 1 )
      {
      // PCM source path is mono-only in this mixer implementation.
      if ( nChannels != 1 || ( nBitsPerSample != 8 && nBitsPerSample != 16 ) )
         {
         MV_SetErrorCode( MV_InvalidWAVFile );
         return( MV_Error );
         }

      bits = nBitsPerSample;
      nextBlock = (char *)dataPtr;

      if ( bits == 16 )
         {
         dataSize &= ~1u;
         sampleLength = dataSize / 2;
         }
      else
         {
         sampleLength = dataSize;
         }
      }
   else if ( wFormatTag == 0x0011 )
      {
      int samplesPerBlockFromFmt = 0;
      uint16_t cbSize = 0;

      if ( nBitsPerSample != 2 && nBitsPerSample != 3 && nBitsPerSample != 4 )
         {
         MV_SetErrorCode( MV_InvalidWAVFile );
         return( MV_Error );
         }

      if ( fmtSize >= 20 )
         {
         cbSize = MV_ReadLE16( fmtData + 16 );
         if ( cbSize >= 2 && fmtSize >= 22 )
            {
            samplesPerBlockFromFmt = MV_ReadLE16( fmtData + 18 );
            }
         }

      if ( !MV_DecodeWAVIMAADPCM( dataPtr, dataSize, nChannels, nBlockAlign,
         nBitsPerSample, samplesPerBlockFromFmt, &decodedPCM, &decodedSamplesPerChannel ) ||
         decodedPCM == NULL || decodedSamplesPerChannel == 0 )
         {
         MV_SetErrorCode( MV_InvalidWAVFile );
         return( MV_Error );
         }

      bits = 16;
      decodedBytes = decodedSamplesPerChannel * (uint32_t)nChannels * sizeof( int16_t );
      nextBlock = (char *)decodedPCM;
      sampleLength = decodedSamplesPerChannel * (uint32_t)nChannels;
      dataSize = decodedBytes;
      ownsRawData = TRUE;
      }
   else
      {
      MV_SetErrorCode( MV_InvalidWAVFile );
      return( MV_Error );
      }

   // Request a voice from the voice pool
   voice = MV_AllocVoice( priority );
   if ( voice == NULL )
      {
      if ( ownsRawData && decodedPCM != NULL )
         {
         USRHOOKS_FreeMem( decodedPCM );
         }

      MV_SetErrorCode( MV_NoVoices );
      return( MV_Error );
      }

   loopstartBytes = loopstart;
   loopendBytes = loopend;
   absloopstart = loopstart;
   absloopend = loopend;

   if ( bits == 16 )
      {
      loopstartBytes *= 2;
      loopendBytes *= 2;
      }

   loopendBytes = min( loopendBytes, (long)dataSize );
   absloopend = min( absloopend, (long)sampleLength );

   blockLength = absloopend;

   voice->wavetype    = WAV;
   voice->bits        = bits;
   voice->GetSound    = MV_GetNextWAVBlock;

   voice->Playing     = TRUE;
   voice->DemandFeed  = NULL;
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->position    = 0;
   voice->length      = 0;
   voice->BlockLength = blockLength;
   voice->NextBlock   = nextBlock;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->GLast       = -1;
   voice->GPos        = 0;
   voice->GVal[0]     = 0;
   voice->GVal[1]     = 0;
   voice->GVal[2]     = 0;
   voice->GVal[3]     = 0;
   voice->callbackval = callbackval;
   voice->rawdataptr  = ownsRawData ? (void *)decodedPCM : (void *)ptr;
   voice->rawdatasiz  = ownsRawData ? decodedBytes : wavLength;
   voice->ownsRawData = ownsRawData;

   voice->LoopStart   = voice->NextBlock + loopstartBytes;
   voice->LoopEnd     = voice->NextBlock + loopendBytes;
   voice->LoopSize    = absloopend - absloopstart;

   if ( ( loopstartBytes >= (long)dataSize ) || ( loopstartBytes < 0 ) )
      {
      voice->LoopStart = NULL;
      voice->LoopEnd   = NULL;
      voice->BlockLength = sampleLength;
      }

   MV_SetVoicePitch( voice, nSamplesPerSec, pitchoffset );
   MV_SetVoiceVolume( voice, vol, left, right );
   MV_PlayVoice( voice );

   return( voice->handle );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayVOC3D

   Begin playback of sound data at specified angle and distance
   from listener.
---------------------------------------------------------------------*/

int MV_PlayVOC3D
   (
   uint8_t *ptr,
   int  pitchoffset,
   int  angle,
   int  distance,
   int  priority,
   unsigned long callbackval
   )

   {
   int left;
   int right;
   int mid;
   int volume;
   int status;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   if ( distance < 0 )
      {
      distance  = -distance;
      angle    += MV_NumPanPositions / 2;
      }

   volume = MIX_VOLUME( distance );

   // Ensure angle is within 0 - 31
   angle &= MV_MaxPanPosition;

   left  = MV_PanTable[ angle ][ volume ].left;
   right = MV_PanTable[ angle ][ volume ].right;
   mid   = max( 0, 255 - distance );

   status = MV_PlayVOC( ptr, pitchoffset, mid, left, right, priority,
      callbackval );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayVOC

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayVOC
   (
   uint8_t *ptr,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   unsigned long callbackval
   )

   {
   int status;

   status = MV_PlayLoopedVOC( ptr, -1, -1, pitchoffset, vol, left, right,
      priority, callbackval );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MV_PlayLoopedVOC

   Begin playback of sound data with the given sound levels and
   priority.
---------------------------------------------------------------------*/

int MV_PlayLoopedVOC
   (
   uint8_t *ptr,
   long  loopstart,
   long  loopend,
   int   pitchoffset,
   int   vol,
   int   left,
   int   right,
   int   priority,
   uint32_t callbackval
   )

   {
   VoiceNode   *voice;
   int          status;

   if ( !MV_Installed )
      {
      MV_SetErrorCode( MV_NotInstalled );
      return( MV_Error );
      }

   // Make sure it's a valid VOC file.
   status = strncmp( (char*)ptr, "Creative Voice File", 19 );
   if ( status != 0 )
      {
      MV_SetErrorCode( MV_InvalidVOCFile );
      return( MV_Error );
      }

   // Request a voice from the voice pool
   voice = MV_AllocVoice( priority );
   if ( voice == NULL )
      {
      MV_SetErrorCode( MV_NoVoices );
      return( MV_Error );
      }

   voice->wavetype    = VOC;
   voice->bits        = 8;
   voice->GetSound    = MV_GetNextVOCBlock;
   voice->NextBlock   = (char *)(ptr + *( uint16_t  * )( ptr + 0x14 ));
   voice->DemandFeed  = NULL;
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->BlockLength = 0;
   voice->PitchScale  = PITCH_GetScale( pitchoffset );
   voice->length      = 0;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->GLast       = -1;
   voice->GPos        = 0;
   voice->GVal[0]     = 0;
   voice->GVal[1]     = 0;
   voice->GVal[2]     = 0;
   voice->GVal[3]     = 0;
   voice->callbackval = callbackval;
   voice->LoopStart   = ( char * )loopstart;
   voice->LoopEnd     = ( char * )loopend;
   voice->LoopSize    = loopend - loopstart + 1;

   if ( loopstart < 0 )
      {
      voice->LoopStart = NULL;
      voice->LoopEnd   = NULL;
      }

   MV_SetVoiceVolume( voice, vol, left, right );
   MV_PlayVoice( voice );

   return( voice->handle );
   }


/*---------------------------------------------------------------------
   Function: MV_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void MV_LockEnd
   (
   void
   )

   {
   }

#if 0
/*---------------------------------------------------------------------
   Function: MV_CreateVolumeTable

   Create the table used to convert sound data to a specific volume
   level.
---------------------------------------------------------------------*/

void MV_CreateVolumeTable
   (
   int index,
   int volume,
   int MaxVolume
   )

   {
   int val;
   int level;
   int i;

   level = ( volume * MaxVolume ) / MV_MaxTotalVolume;
   if ( MV_Bits == 16 )
      {
      for( i = 0; i < 65536; i += 256 )
         {
         val   = i - 0x8000;
         val  *= level;
         val  /= MV_MaxVolume;
         MV_VolumeTable[ index ][ i / 256 ] = val;
         }
      }
   else
      {
      for( i = 0; i < 256; i++ )
         {
         val   = i - 0x80;
         val  *= level;
         val  /= MV_MaxVolume;
         MV_VolumeTable[ volume ][ i ] = val;
         }
      }
   }
#endif

/*---------------------------------------------------------------------
   Function: MV_CalcVolume

   Create the table used to convert sound data to a specific volume
   level.
---------------------------------------------------------------------*/

void MV_CalcVolume(int MaxVolume)
   {
   }


/*---------------------------------------------------------------------
   Function: MV_CalcPanTable

   Create the table used to determine the stereo volume level of
   a sound located at a specific angle and distance from the listener.
---------------------------------------------------------------------*/

void MV_CalcPanTable
   (
   void
   )

   {
   int   level;
   int   angle;
   int   distance;
   int   HalfAngle;
   int   ramp;

   HalfAngle = ( MV_NumPanPositions / 2 );

   for( distance = 0; distance <= MV_MaxVolume; distance++ )
      {
      level = ( 255 * ( MV_MaxVolume - distance ) ) / MV_MaxVolume;
      for( angle = 0; angle <= HalfAngle / 2; angle++ )
         {
         ramp = level - ( ( level * angle ) /
            ( MV_NumPanPositions / 4 ) );

         MV_PanTable[ angle ][ distance ].left = ramp;
         MV_PanTable[ HalfAngle - angle ][ distance ].left = ramp;
         MV_PanTable[ HalfAngle + angle ][ distance ].left = level;
         MV_PanTable[ MV_MaxPanPosition - angle ][ distance ].left = level;

         MV_PanTable[ angle ][ distance ].right = level;
         MV_PanTable[ HalfAngle - angle ][ distance ].right = level;
         MV_PanTable[ HalfAngle + angle ][ distance ].right = ramp;
         MV_PanTable[ MV_MaxPanPosition - angle ][ distance ].right = ramp;
         }
      }
   }


/*---------------------------------------------------------------------
   Function: MV_SetVolume

   Sets the volume of digitized sound playback.
---------------------------------------------------------------------*/

void MV_SetVolume
   (
   int volume
   )

   {
   volume = max( 0, volume );
   volume = min( volume, MV_MaxTotalVolume );

   MV_TotalVolume = volume;

   // Calculate volume table
   MV_CalcVolume( volume );
   }


/*---------------------------------------------------------------------
   Function: MV_GetVolume

   Returns the volume of digitized sound playback.
---------------------------------------------------------------------*/

int MV_GetVolume
   (
   void
   )

   {
   return( MV_TotalVolume );
   }


/*---------------------------------------------------------------------
   Function: MV_SetCallBack

   Set the function to call when a voice stops.
---------------------------------------------------------------------*/

void MV_SetCallBack
   (
   void ( *function )( int32_t )
   )

   {
   MV_CallBackFunc = function;
   }


/*---------------------------------------------------------------------
   Function: MV_SetReverseStereo

   Set the orientation of the left and right channels.
---------------------------------------------------------------------*/

void MV_SetReverseStereo
   (
   int setting
   )

   {
   MV_SwapLeftRight = setting;
   }


/*---------------------------------------------------------------------
   Function: MV_GetReverseStereo

   Returns the orientation of the left and right channels.
---------------------------------------------------------------------*/

int MV_GetReverseStereo
   (
   void
   )

   {
   return( MV_SwapLeftRight );
   }


/*---------------------------------------------------------------------
   Function: MV_TestPlayback

   Checks if playback has started.
---------------------------------------------------------------------*/

int MV_TestPlayback
   (
   void
   )

   {
#ifdef PLAT_DOS
   unsigned flags;
   long time;
   int  start;
   int  status;
   int  pos;

   if ( MV_SoundCard == UltraSound )
      {
      return( MV_Ok );
      }

   flags = DisableInterrupts();
   _enable();

   status = MV_Error;
   start  = MV_MixPage;
   time   = clock() + CLOCKS_PER_SEC * 2;

   while( clock() < time )
      {
      if ( MV_MixPage != start )
         {
         status = MV_Ok;
         }
      }

   RestoreInterrupts( flags );

   if ( status != MV_Ok )
      {
      // Just in case an error doesn't get reported
      MV_SetErrorCode( MV_DMAFailure );

      switch( MV_SoundCard )
         {
         case SoundBlaster :
         case Awe32 :
            pos = BLASTER_GetCurrentPos();
            break;

         case ProAudioSpectrum :
         case SoundMan16 :
            pos = PAS_GetCurrentPos();
            break;

         case SoundScape :
            pos = SOUNDSCAPE_GetCurrentPos();
            break;

         #ifndef SOUNDSOURCE_OFF
         case SoundSource :
         case TandySoundSource :
            MV_SetErrorCode( MV_SoundSourceFailure );
            pos = -1;
            break;
         #endif

         default :
            MV_SetErrorCode( MV_UnsupportedCard );
            pos = -2;
            break;
         }

      if ( pos > 0 )
         {
         MV_SetErrorCode( MV_IrqFailure );
         }
      else if ( pos == 0 )
         {
         if ( MV_Bits == 16 )
            {
            MV_SetErrorCode( MV_DMA16Failure );
            }
         else
            {
            MV_SetErrorCode( MV_DMAFailure );
            }
         }
      }

   return( status );
#else
   return MV_Ok;
#endif
   }


/*---------------------------------------------------------------------
   Function: MV_Init

   Perform the initialization of variables and memory used by
   Multivoc.
---------------------------------------------------------------------*/

int MV_Init
   (
   int soundcard,
   int MixRate,
   int Voices,
   int numchannels,
   int samplebits
   )

   {
   char *ptr;
   int  status;
   int  buffer;
   int  index;
printf("MV_Init card: %d, rate: %d, voices: %d, channels: %d, bits: %d\n", soundcard, MixRate, Voices, numchannels, samplebits);
   if ( MV_Installed )
      {
      MV_Shutdown();
      }

   MV_SetErrorCode( MV_Ok );

   status = MV_LockMemory();
   if ( status != MV_Ok )
      {
      return( status );
      }

   MV_TotalMemory = Voices * sizeof( VoiceNode ); // + sizeof( HARSH_CLIP_TABLE_8 );
   status = USRHOOKS_GetMem( ( void ** )&ptr, MV_TotalMemory );
   if ( status != USRHOOKS_Ok )
      {
      MV_UnlockMemory();
      MV_SetErrorCode( MV_NoMem );
      return( MV_Error );
      }

   status = DPMI_LockMemory( ptr, MV_TotalMemory );
   if ( status != DPMI_Ok )
      {
      USRHOOKS_FreeMem( ptr );
      MV_UnlockMemory();
      MV_SetErrorCode( MV_DPMI_Error );
      return( MV_Error );
      }

   MV_Voices = ( VoiceNode * )ptr;
//   MV_HarshClipTable = ptr + ( MV_TotalMemory - sizeof( HARSH_CLIP_TABLE_8 ) );

   // Set number of voices before calculating volume table
   MV_MaxVoices = Voices;

   LL_Reset( (VoiceNode *)&VoiceList, next, prev );
   LL_Reset( (VoiceNode *)&VoicePool, next, prev );

   for( index = 0; index < Voices; index++ )
      {
      LL_Add( &VoicePool, &MV_Voices[ index ], next, prev );
      }

   // Allocate mix buffer within 1st megabyte
   status = DPMI_GetDOSMemory( ( void ** )&ptr, &MV_BufferDescriptor,
      2 * TotalBufferSize);

   if ( status )
      {
      DPMI_UnlockMemory( MV_Voices, MV_TotalMemory );
      USRHOOKS_FreeMem( MV_Voices );
      MV_Voices      = NULL;
      MV_TotalMemory = 0;
      MV_UnlockMemory();

      MV_SetErrorCode( MV_NoMem );
      return( MV_Error );
      }

   MV_SetReverseStereo( FALSE );

   // Initialize the sound card
#ifdef PLAT_DOS
   switch( soundcard )
      {
      case UltraSound :
         status = GUSWAVE_Init( 2 );
         if ( status != GUSWAVE_Ok )
            {
            //JIM
            MV_SetErrorCode( MV_BlasterError );
            }
         break;

      case SoundBlaster :
      case Awe32 :
         status = BLASTER_Init();
         if ( status != BLASTER_Ok )
            {
            MV_SetErrorCode( MV_BlasterError );
            }

         if ( ( BLASTER_Config.Type == SBPro ) ||
            ( BLASTER_Config.Type == SBPro2 ) )
            {
            MV_SetReverseStereo( TRUE );
            }
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         status = PAS_Init();
         if ( status != PAS_Ok )
            {
            MV_SetErrorCode( MV_PasError );
            }
         break;

      case SoundScape :
         status = SOUNDSCAPE_Init();
         if ( status != SOUNDSCAPE_Ok )
            {
            MV_SetErrorCode( MV_SoundScapeError );
            }
         break;

      #ifndef SOUNDSOURCE_OFF
      case SoundSource :
      case TandySoundSource :
         status = SS_Init( soundcard );
         if ( status != SS_Ok )
            {
            MV_SetErrorCode( MV_SoundSourceError );
            }
         break;
      #endif

      default :
         MV_SetErrorCode( MV_UnsupportedCard );
         break;
      }
#else
   status = DSL_Init();
   if ( status != DSL_Ok )
      {
      MV_SetErrorCode( MV_BlasterError );
      }
#endif

   if ( MV_ErrorCode != MV_Ok )
      {
      status = MV_ErrorCode;

      DPMI_UnlockMemory( MV_Voices, MV_TotalMemory );
      USRHOOKS_FreeMem( MV_Voices );
      MV_Voices      = NULL;
      MV_TotalMemory = 0;

      DPMI_FreeDOSMemory( MV_BufferDescriptor );
      MV_UnlockMemory();

      MV_SetErrorCode( status );
      return( MV_Error );
      }

   MV_SoundCard    = soundcard;
   MV_Installed    = TRUE;
   MV_CallBackFunc = NULL;
   MV_RecordFunc   = NULL;
   MV_Recording    = FALSE;
   MV_ReverbLevel  = 0;
   MV_ReverbTable  = -1;

   // Set the sampling rate
   MV_RequestedMixRate = MixRate;

   // Set Mixer to play stereo digitized sound
   MV_SetMixMode( numchannels, samplebits );
   MV_ReverbDelay = 14320; // MV_BufferSize * 3;
   //InitializeCriticalSection(&reverbCS);
   reverbMutex = SDL_CreateMutex();

#ifdef PLAT_DOS
   // Make sure we don't cross a physical page
   if ( ( ( unsigned long )ptr & 0xffff ) + TotalBufferSize > 0x10000 )
      {
      ptr = ( char * )( ( ( unsigned long )ptr & 0xff0000 ) + 0x10000 );
      }
#endif

   MV_MixBuffer[ MV_NumberOfBuffers ] = ptr;
   for( buffer = 0; buffer < MV_NumberOfBuffers; buffer++ )
      {
      MV_MixBuffer[ buffer ] = ptr;
      ptr += MV_BufferSize;
      }

   // Calculate pan table
   MV_CalcPanTable();

   MV_SetVolume( MV_MaxTotalVolume );


   MV_FooMemory = sizeof(*MV_FooBuffer) * MixBufferSize * numchannels + 1024;
   status = USRHOOKS_GetMem( ( void ** )&ptr, MV_FooMemory);
   if ( status != USRHOOKS_Ok )
   {
       DPMI_UnlockMemory( MV_Voices, MV_TotalMemory );
       USRHOOKS_FreeMem( MV_Voices );
       MV_Voices      = NULL;
       MV_TotalMemory = 0;
	   MV_UnlockMemory();
	   MV_SetErrorCode( MV_NoMem );
	   return( MV_Error);
   }

   status = DPMI_LockMemory( ptr, MV_FooMemory );
   if ( status != DPMI_Ok )
   {
	   USRHOOKS_FreeMem( ptr );
       DPMI_UnlockMemory( MV_Voices, MV_TotalMemory );
       USRHOOKS_FreeMem( MV_Voices );
       MV_Voices      = NULL;
       MV_TotalMemory = 0;
	   MV_UnlockMemory();
	   MV_SetErrorCode( MV_DPMI_Error );
	   return( MV_Error );
   }

   MV_FooBuffer = (int32_t *)ptr;

// Start the playback engine
   status = MV_StartPlayback();
   if ( status != MV_Ok )
      {
      // Preserve error code while we shutdown.
      status = MV_ErrorCode;
      MV_Shutdown();
      MV_SetErrorCode( status );
      return( MV_Error );
      }

   if ( MV_TestPlayback() != MV_Ok )
      {
      status = MV_ErrorCode;
      MV_Shutdown();
      MV_SetErrorCode( status );
      return( MV_Error );
      }

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_Shutdown

   Restore any resources allocated by Multivoc back to the system.
---------------------------------------------------------------------*/

int MV_Shutdown
   (
   void
   )

   {
   int      buffer;
   unsigned flags;

   if ( !MV_Installed )
      {
      return( MV_Ok );
      }

   flags = DisableInterrupts();

   MV_KillAllVoices();

   MV_Installed = FALSE;

   // Stop the sound recording engine
   if ( MV_Recording )
      {
      MV_StopRecord();
      }

   // Stop the sound playback engine
   MV_StopPlayback();

   // Free reverb buffer, if allocated
   MV_FPReverbFree();
   //DeleteCriticalSection(&reverbCS);
   SDL_DestroyMutex(reverbMutex);

   // Shutdown the sound card
#ifdef PLAT_DOS
   switch( MV_SoundCard )
      {
      case UltraSound :
         GUSWAVE_Shutdown();
         break;

      case SoundBlaster :
      case Awe32 :
         BLASTER_Shutdown();
         break;

      case ProAudioSpectrum :
      case SoundMan16 :
         PAS_Shutdown();
         break;

      case SoundScape :
         SOUNDSCAPE_Shutdown();
         break;

      #ifndef SOUNDSOURCE_OFF
      case SoundSource :
      case TandySoundSource :
         SS_Shutdown();
         break;
      #endif
      }
#else
   DSL_Shutdown();
#endif

   RestoreInterrupts( flags );

   // Free any voices we allocated
   DPMI_UnlockMemory( MV_FooBuffer, MV_FooMemory );
   USRHOOKS_FreeMem( MV_FooBuffer );
   MV_FooBuffer = NULL;
   MV_FooMemory = 0;

   DPMI_UnlockMemory( MV_Voices, MV_TotalMemory );
   USRHOOKS_FreeMem( MV_Voices );
   MV_Voices      = NULL;
   MV_TotalMemory = 0;

   LL_Reset( (VoiceNode *)&VoiceList, next, prev );
   LL_Reset( (VoiceNode *)&VoicePool, next, prev );

   MV_MaxVoices = 1;

   // Release the descriptor from our mix buffer
   DPMI_FreeDOSMemory( MV_BufferDescriptor );
   for( buffer = 0; buffer < NumberOfBuffers; buffer++ )
      {
      MV_MixBuffer[ buffer ] = NULL;
      }

   return( MV_Ok );
   }


/*---------------------------------------------------------------------
   Function: MV_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

void MV_UnlockMemory
   (
   void
   )

   {
   PITCH_UnlockMemory();

   DPMI_UnlockMemoryRegion( MV_LockStart, MV_LockEnd );
//   DPMI_Unlock( MV_VolumeTable );
   DPMI_Unlock( MV_PanTable );
   DPMI_Unlock( MV_Installed );
   DPMI_Unlock( MV_SoundCard );
   DPMI_Unlock( MV_TotalVolume );
   DPMI_Unlock( MV_MaxVoices );
   DPMI_Unlock( MV_BufferSize );
   DPMI_Unlock( MV_BufferLength );
   DPMI_Unlock( MV_SampleSize );
   DPMI_Unlock( MV_NumberOfBuffers );
   DPMI_Unlock( MV_MixMode );
   DPMI_Unlock( MV_Channels );
   DPMI_Unlock( MV_Bits );
   DPMI_Unlock( MV_Silence );
   DPMI_Unlock( MV_SwapLeftRight );
   DPMI_Unlock( MV_RequestedMixRate );
   DPMI_Unlock( MV_MixRate );
   DPMI_Unlock( MV_BufferDescriptor );
   DPMI_Unlock( MV_MixBuffer );
   DPMI_Unlock( MV_BufferEmpty );
   DPMI_Unlock( MV_Voices );
   DPMI_Unlock( MV_FooBuffer );
   DPMI_Unlock( VoiceList );
   DPMI_Unlock( VoicePool );
   DPMI_Unlock( MV_MixPage );
   DPMI_Unlock( MV_VoiceHandle );
   DPMI_Unlock( MV_CallBackFunc );
   DPMI_Unlock( MV_RecordFunc );
   DPMI_Unlock( MV_Recording );
   DPMI_Unlock( MV_MixFunction );
//   DPMI_Unlock( MV_HarshClipTable );
   DPMI_Unlock( MV_MixDestination );
   DPMI_Unlock( MV_LeftVolume );
   DPMI_Unlock( MV_RightVolume );
   DPMI_Unlock( MV_MixPosition );
   DPMI_Unlock( MV_ErrorCode );
   DPMI_Unlock( MV_DMAChannel );
   DPMI_Unlock( MV_BuffShift );
   DPMI_Unlock( MV_ReverbLevel );
   DPMI_Unlock( MV_ReverbDelay );
   DPMI_Unlock( MV_ReverbTable );
   }


/*---------------------------------------------------------------------
   Function: MV_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int MV_LockMemory
   (
   void
   )

   {
   int status;
   int pitchstatus;

   status  = DPMI_LockMemoryRegion( MV_LockStart, MV_LockEnd );
//   status |= DPMI_Lock( MV_VolumeTable );
   status |= DPMI_Lock( MV_PanTable );
   status |= DPMI_Lock( MV_Installed );
   status |= DPMI_Lock( MV_SoundCard );
   status |= DPMI_Lock( MV_TotalVolume );
   status |= DPMI_Lock( MV_MaxVoices );
   status |= DPMI_Lock( MV_BufferSize );
   status |= DPMI_Lock( MV_BufferLength );
   status |= DPMI_Lock( MV_SampleSize );
   status |= DPMI_Lock( MV_NumberOfBuffers );
   status |= DPMI_Lock( MV_MixMode );
   status |= DPMI_Lock( MV_Channels );
   status |= DPMI_Lock( MV_Bits );
   status |= DPMI_Lock( MV_Silence );
   status |= DPMI_Lock( MV_SwapLeftRight );
   status |= DPMI_Lock( MV_RequestedMixRate );
   status |= DPMI_Lock( MV_MixRate );
   status |= DPMI_Lock( MV_BufferDescriptor );
   status |= DPMI_Lock( MV_MixBuffer );
   status |= DPMI_Lock( MV_BufferEmpty );
   status |= DPMI_Lock( MV_Voices );
   status |= DPMI_Lock( MV_FooBuffer );
   status |= DPMI_Lock( VoiceList );
   status |= DPMI_Lock( VoicePool );
   status |= DPMI_Lock( MV_MixPage );
   status |= DPMI_Lock( MV_VoiceHandle );
   status |= DPMI_Lock( MV_CallBackFunc );
   status |= DPMI_Lock( MV_RecordFunc );
   status |= DPMI_Lock( MV_Recording );
   status |= DPMI_Lock( MV_MixFunction );
//   status |= DPMI_Lock( MV_HarshClipTable );
   status |= DPMI_Lock( MV_MixDestination );
   status |= DPMI_Lock( MV_LeftVolume );
   status |= DPMI_Lock( MV_RightVolume );
   status |= DPMI_Lock( MV_MixPosition );
   status |= DPMI_Lock( MV_ErrorCode );
   status |= DPMI_Lock( MV_DMAChannel );
   status |= DPMI_Lock( MV_BuffShift );
   status |= DPMI_Lock( MV_ReverbLevel );
   status |= DPMI_Lock( MV_ReverbDelay );
   status |= DPMI_Lock( MV_ReverbTable );

   pitchstatus = PITCH_LockMemory();
   if ( ( pitchstatus != PITCH_Ok ) || ( status != DPMI_Ok ) )
      {
      MV_UnlockMemory();
      MV_SetErrorCode( MV_DPMI_Error );
      return( MV_Error );
      }

   return( MV_Ok );
   }

#ifndef PLAT_DOS
IRAM_ATTR void ClearBuffer_DW( void *ptr, unsigned data, int length )
{
	unsigned *d = (unsigned *)ptr;

	while (length--) {
		*d = data;

		d++;
	}
}
#endif
