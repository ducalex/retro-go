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
   module: BLASTER.C

   author: James R. Dose
   date:   February 4, 1994

   Low level routines to support Sound Blaster, Sound Blaster Pro,
   Sound Blaster 16, and compatible sound cards.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dpmi.h"
#include "dma.h"
#include "irq.h"
#include "blaster.h"
#include "_blaster.h"

#define USESTACK

const int BLASTER_Interrupts[ BLASTER_MaxIrq + 1 ]  =
   {
   INVALID, INVALID,     0xa,     0xb,
   INVALID,     0xd, INVALID,     0xf,
   INVALID, INVALID,    0x72,    0x73,
      0x74, INVALID, INVALID,    0x77
   };

const int BLASTER_SampleSize[ BLASTER_MaxMixMode + 1 ] =
   {
   MONO_8BIT_SAMPLE_SIZE,  STEREO_8BIT_SAMPLE_SIZE,
   MONO_16BIT_SAMPLE_SIZE, STEREO_16BIT_SAMPLE_SIZE
   };

const CARD_CAPABILITY BLASTER_CardConfig[ BLASTER_MaxCardType + 1 ] =
   {
      { FALSE, INVALID,      INVALID, INVALID, INVALID }, // Unsupported
      {  TRUE,      NO,    MONO_8BIT,    4000,   23000 }, // SB 1.0
      {  TRUE,     YES,  STEREO_8BIT,    4000,   44100 }, // SBPro
      {  TRUE,      NO,    MONO_8BIT,    4000,   23000 }, // SB 2.xx
      {  TRUE,     YES,  STEREO_8BIT,    4000,   44100 }, // SBPro 2
      { FALSE, INVALID,      INVALID, INVALID, INVALID }, // Unsupported
      {  TRUE,     YES, STEREO_16BIT,    5000,   44100 }, // SB16
   };

CARD_CAPABILITY BLASTER_Card;

static void    ( __interrupt __far *BLASTER_OldInt )( void );

BLASTER_CONFIG BLASTER_Config =
   {
   UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED
   };

static int BLASTER_Installed = FALSE;

int BLASTER_Version;

static char   *BLASTER_DMABuffer;
static char   *BLASTER_DMABufferEnd;
static char   *BLASTER_CurrentDMABuffer;
static int     BLASTER_TotalDMABufferSize;

static int      BLASTER_TransferLength   = 0;
static int      BLASTER_MixMode          = BLASTER_DefaultMixMode;
static int      BLASTER_SamplePacketSize = MONO_16BIT_SAMPLE_SIZE;
static unsigned BLASTER_SampleRate       = BLASTER_DefaultSampleRate;

static unsigned BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

volatile int   BLASTER_SoundPlaying;
volatile int   BLASTER_SoundRecording;

void ( *BLASTER_CallBack )( void );

static int  BLASTER_IntController1Mask;
static int  BLASTER_IntController2Mask;

static int BLASTER_MixerAddress = UNDEFINED;
static int BLASTER_MixerType    = 0;
static int BLASTER_OriginalMidiVolumeLeft   = 255;
static int BLASTER_OriginalMidiVolumeRight  = 255;
static int BLASTER_OriginalVoiceVolumeLeft  = 255;
static int BLASTER_OriginalVoiceVolumeRight = 255;

static int BLASTER_WaveBlasterState = 0x0F;

// adequate stack size
#define kStackSize 2048

static unsigned short StackSelector = NULL;
static unsigned long  StackPointer;

static unsigned short oldStackSelector;
static unsigned long  oldStackPointer;

// This is defined because we can't create local variables in a
// function that switches stacks.
static int GlobalStatus;

// These declarations are necessary to use the inline assembly pragmas.

extern void GetStack(unsigned short *selptr,unsigned long *stackptr);
extern void SetStack(unsigned short selector,unsigned long stackptr);

// This function will get the current stack selector and pointer and save
// them off.
#pragma aux GetStack =  \
   "mov  [edi],esp"     \
   "mov  ax,ss"         \
   "mov  [esi],ax"      \
   parm [esi] [edi]     \
   modify [eax esi edi];

// This function will set the stack selector and pointer to the specified
// values.
#pragma aux SetStack =  \
   "mov  ss,ax"         \
   "mov  esp,edx"       \
   parm [ax] [edx]      \
   modify [eax edx];

int BLASTER_DMAChannel;

int BLASTER_ErrorCode = BLASTER_Ok;

#define BLASTER_SetErrorCode( status ) \
   BLASTER_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: BLASTER_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *BLASTER_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case BLASTER_Warning :
      case BLASTER_Error :
         ErrorString = BLASTER_ErrorString( BLASTER_ErrorCode );
         break;

      case BLASTER_Ok :
         ErrorString = "Sound Blaster ok.";
         break;

      case BLASTER_EnvNotFound :
         ErrorString = "BLASTER environment variable not set.";
         break;

      case BLASTER_AddrNotSet :
         ErrorString = "Sound Blaster address not set.";
         break;

      case BLASTER_DMANotSet :
         ErrorString = "Sound Blaster 8-bit DMA channel not set.";
         break;

      case BLASTER_DMA16NotSet :
         ErrorString = "Sound Blaster 16-bit DMA channel not set.";
         break;

      case BLASTER_InvalidParameter :
         ErrorString = "Invalid parameter in BLASTER environment variable.";
         break;

      case BLASTER_CardNotReady :
         ErrorString = "Sound Blaster not responding on selected port.";
         break;

      case BLASTER_NoSoundPlaying :
         ErrorString = "No sound playing on Sound Blaster.";
         break;

      case BLASTER_InvalidIrq :
         ErrorString = "Invalid Sound Blaster Irq.";
         break;

      case BLASTER_UnableToSetIrq :
         ErrorString = "Unable to set Sound Blaster IRQ.  Try selecting an IRQ of 7 or below.";
         break;

      case BLASTER_DmaError :
         ErrorString = DMA_ErrorString( DMA_Error );
         break;

      case BLASTER_NoMixer :
         ErrorString = "Mixer not available on selected Sound Blaster card.";
         break;

      case BLASTER_DPMI_Error :
         ErrorString = "DPMI Error in Blaster.";
         break;

      case BLASTER_OutOfMemory :
         ErrorString = "Out of conventional memory in Blaster.";
         break;

      default :
         ErrorString = "Unknown Sound Blaster error code.";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define BLASTER_LockStart BLASTER_EnableInterrupt


/*---------------------------------------------------------------------
   Function: BLASTER_EnableInterrupt

   Enables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

void BLASTER_EnableInterrupt
   (
   void
   )

   {
   int Irq;
   int mask;

   // Unmask system interrupt
   Irq  = BLASTER_Config.Interrupt;
   if ( Irq < 8 )
      {
      mask = inp( 0x21 ) & ~( 1 << Irq );
      outp( 0x21, mask  );
      }
   else
      {
      mask = inp( 0xA1 ) & ~( 1 << ( Irq - 8 ) );
      outp( 0xA1, mask  );

      mask = inp( 0x21 ) & ~( 1 << 2 );
      outp( 0x21, mask  );
      }

   }


/*---------------------------------------------------------------------
   Function: BLASTER_DisableInterrupt

   Disables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

void BLASTER_DisableInterrupt
   (
   void
   )

   {
   int Irq;
   int mask;

   // Restore interrupt mask
   Irq  = BLASTER_Config.Interrupt;
   if ( Irq < 8 )
      {
      mask  = inp( 0x21 ) & ~( 1 << Irq );
      mask |= BLASTER_IntController1Mask & ( 1 << Irq );
      outp( 0x21, mask  );
      }
   else
      {
      mask  = inp( 0x21 ) & ~( 1 << 2 );
      mask |= BLASTER_IntController1Mask & ( 1 << 2 );
      outp( 0x21, mask  );

      mask  = inp( 0xA1 ) & ~( 1 << ( Irq - 8 ) );
      mask |= BLASTER_IntController2Mask & ( 1 << ( Irq - 8 ) );
      outp( 0xA1, mask  );
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_ServiceInterrupt

   Handles interrupt generated by sound card at the end of a voice
   transfer.  Calls the user supplied callback function.
---------------------------------------------------------------------*/

void __interrupt __far BLASTER_ServiceInterrupt
   (
   void
   )

   {
   #ifdef USESTACK
   // save stack
   GetStack( &oldStackSelector, &oldStackPointer );

   // set our stack
   SetStack( StackSelector, StackPointer );
   #endif

   // Acknowledge interrupt
   // Check if this is this an SB16 or newer
   if ( BLASTER_Version >= DSP_Version4xx )
      {
      outp( BLASTER_Config.Address + BLASTER_MixerAddressPort,
         MIXER_DSP4xxISR_Ack );

      GlobalStatus = inp( BLASTER_Config.Address + BLASTER_MixerDataPort );

      // Check if a 16-bit DMA interrupt occurred
      if ( GlobalStatus & MIXER_16BITDMA_INT )
         {
         // Acknowledge 16-bit transfer interrupt
         inp( BLASTER_Config.Address + BLASTER_16BitDMAAck );
         }
      else if ( GlobalStatus & MIXER_8BITDMA_INT )
         {
         inp( BLASTER_Config.Address + BLASTER_DataAvailablePort );
         }
      else
         {
         #ifdef USESTACK
         // restore stack
         SetStack( oldStackSelector, oldStackPointer );
         #endif

         // Wasn't our interrupt.  Call the old one.
         _chain_intr( BLASTER_OldInt );
         }
      }
   else
      {
      // Older card - can't detect if an interrupt occurred.
      inp( BLASTER_Config.Address + BLASTER_DataAvailablePort );
      }

   // Keep track of current buffer
   BLASTER_CurrentDMABuffer += BLASTER_TransferLength;

   if ( BLASTER_CurrentDMABuffer >= BLASTER_DMABufferEnd )
      {
      BLASTER_CurrentDMABuffer = BLASTER_DMABuffer;
      }

   // Continue playback on cards without autoinit mode
   if ( BLASTER_Version < DSP_Version2xx )
      {
      if ( BLASTER_SoundPlaying )
         {
         BLASTER_DSP1xx_BeginPlayback( BLASTER_TransferLength );
         }

      if ( BLASTER_SoundRecording )
         {
         BLASTER_DSP1xx_BeginRecord( BLASTER_TransferLength );
         }
      }

   // Call the caller's callback function
   if ( BLASTER_CallBack != NULL )
      {
      BLASTER_CallBack();
      }

   #ifdef USESTACK
   // restore stack
   SetStack( oldStackSelector, oldStackPointer );
   #endif

   // send EOI to Interrupt Controller
   if ( BLASTER_Config.Interrupt > 7 )
      {
      outp( 0xA0, 0x20 );
      }

   outp( 0x20, 0x20 );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_WriteDSP

   Writes a byte of data to the sound card's DSP.
---------------------------------------------------------------------*/

int BLASTER_WriteDSP
   (
   unsigned data
   )

   {
   int      port;
   unsigned count;
   int      status;

   port = BLASTER_Config.Address + BLASTER_WritePort;

   status = BLASTER_Error;

   count = 0xFFFF;

   do
      {
      if ( ( inp( port ) & 0x80 ) == 0 )
         {
         outp( port, data );
         status = BLASTER_Ok;
         break;
         }

      count--;
      }
   while( count > 0 );

   if ( status != BLASTER_Ok )
      {
      BLASTER_SetErrorCode( BLASTER_CardNotReady );
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_ReadDSP

   Reads a byte of data from the sound card's DSP.
---------------------------------------------------------------------*/

int BLASTER_ReadDSP
   (
   void
   )

   {
   int      port;
   unsigned count;
   int      status;

   port = BLASTER_Config.Address + BLASTER_DataAvailablePort;

   status = BLASTER_Error;

   count = 0xFFFF;

   do
      {
      if ( inp( port ) & 0x80 )
         {
         status = inp( BLASTER_Config.Address + BLASTER_ReadPort );
         break;
         }

      count--;
      }
   while( count > 0 );

   if ( status == BLASTER_Error )
      {
      BLASTER_SetErrorCode( BLASTER_CardNotReady );
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_ResetDSP

   Sends a reset command to the sound card's Digital Signal Processor
   (DSP), causing it to perform an initialization.
---------------------------------------------------------------------*/

int BLASTER_ResetDSP
   (
   void
   )

   {
   volatile int count;
   int port;
   int status;

   port = BLASTER_Config.Address + BLASTER_ResetPort;

   status = BLASTER_CardNotReady;

   outp( port, 1 );

/* What the hell am I doing here?
   count = 100;

   do
      {
      if ( inp( port ) == 255 )
         {
         break;
         }

      count--;
      }
   while( count > 0 );
*/

   count = 0x100;
   do
      {
      count--;
      }
   while( count > 0 );

   outp( port, 0 );

   count = 100;

   do
      {
      if ( BLASTER_ReadDSP() == BLASTER_Ready )
         {
         status = BLASTER_Ok;
         break;
         }

      count--;
      }
   while( count > 0 );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetDSPVersion

   Returns the version number of the sound card's DSP.
---------------------------------------------------------------------*/

int BLASTER_GetDSPVersion
   (
   void
   )

   {
   int MajorVersion;
   int MinorVersion;
   int version;

   BLASTER_WriteDSP( DSP_GetVersion );

   MajorVersion   = BLASTER_ReadDSP();
   MinorVersion   = BLASTER_ReadDSP();

   if ( ( MajorVersion == BLASTER_Error ) ||
      ( MinorVersion == BLASTER_Error ) )
      {
      BLASTER_SetErrorCode( BLASTER_CardNotReady );
      return( BLASTER_Error );
      }

   version = ( MajorVersion << 8 ) + MinorVersion;

   if ( version >= DSP_Version4xx )
      {
      BLASTER_Card.IsSupported     = TRUE;
      BLASTER_Card.HasMixer        = YES;
      BLASTER_Card.MaxMixMode      = STEREO_16BIT;
      BLASTER_Card.MinSamplingRate = 5000;
      BLASTER_Card.MaxSamplingRate = 44100;
      BLASTER_MixerType = SB16;
      }
   else if ( version >= DSP_Version3xx )
      {
      BLASTER_Card.IsSupported     = TRUE;
      BLASTER_Card.HasMixer        = YES;
      BLASTER_Card.MaxMixMode      = STEREO_8BIT;
      BLASTER_Card.MinSamplingRate = 4000;
      BLASTER_Card.MaxSamplingRate = 44100;
      BLASTER_MixerType = SBPro;
      }
   else if ( version >= DSP_Version2xx )
      {
      BLASTER_Card.IsSupported     = TRUE;
      BLASTER_Card.HasMixer        = NO;
      BLASTER_Card.MaxMixMode      = MONO_8BIT;
      BLASTER_Card.MinSamplingRate = 4000;
      BLASTER_Card.MaxSamplingRate = 23000;
      BLASTER_MixerType = 0;
      }
   else
      {
      // DSP_Version1xx
      BLASTER_Card.IsSupported     = TRUE;
      BLASTER_Card.HasMixer        = NO;
      BLASTER_Card.MaxMixMode      = MONO_8BIT;
      BLASTER_Card.MinSamplingRate = 4000;
      BLASTER_Card.MaxSamplingRate = 23000;
      BLASTER_MixerType = 0;
      }

   return( version );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SpeakerOn

   Enables output from the DAC.
---------------------------------------------------------------------*/

void BLASTER_SpeakerOn
   (
   void
   )

   {
   BLASTER_WriteDSP( DSP_SpeakerOn );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SpeakerOff

   Disables output from the DAC.
---------------------------------------------------------------------*/

void BLASTER_SpeakerOff
   (
   void
   )

   {
   BLASTER_WriteDSP( DSP_SpeakerOff );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetPlaybackRate

   Sets the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

void BLASTER_SetPlaybackRate
   (
   unsigned rate
   )

   {
   int LoByte;
   int HiByte;

   if ( BLASTER_Version < DSP_Version4xx )
      {
      int  timeconstant;
      long ActualRate;

      // Send sampling rate as time constant for older Sound
      // Blaster compatible cards.

      ActualRate = rate * BLASTER_SamplePacketSize;
      if ( ActualRate < BLASTER_Card.MinSamplingRate )
         {
         rate = BLASTER_Card.MinSamplingRate / BLASTER_SamplePacketSize;
         }

      if ( ActualRate > BLASTER_Card.MaxSamplingRate )
         {
         rate = BLASTER_Card.MaxSamplingRate / BLASTER_SamplePacketSize;
         }

      timeconstant = ( int )CalcTimeConstant( rate, BLASTER_SamplePacketSize );

      // Keep track of what the actual rate is
      BLASTER_SampleRate  = ( unsigned )CalcSamplingRate( timeconstant );
      BLASTER_SampleRate /= BLASTER_SamplePacketSize;

      BLASTER_WriteDSP( DSP_SetTimeConstant );
      BLASTER_WriteDSP( timeconstant );
      }
   else
      {
      // Send literal sampling rate for cards with DSP version
      // 4.xx (Sound Blaster 16)

      BLASTER_SampleRate = rate;

      if ( BLASTER_SampleRate < BLASTER_Card.MinSamplingRate )
         {
         BLASTER_SampleRate = BLASTER_Card.MinSamplingRate;
         }

      if ( BLASTER_SampleRate > BLASTER_Card.MaxSamplingRate )
         {
         BLASTER_SampleRate = BLASTER_Card.MaxSamplingRate;
         }

      HiByte = hibyte( BLASTER_SampleRate );
      LoByte = lobyte( BLASTER_SampleRate );

      // Set playback rate
      BLASTER_WriteDSP( DSP_Set_DA_Rate );
      BLASTER_WriteDSP( HiByte );
      BLASTER_WriteDSP( LoByte );

      // Set recording rate
      BLASTER_WriteDSP( DSP_Set_AD_Rate );
      BLASTER_WriteDSP( HiByte );
      BLASTER_WriteDSP( LoByte );
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetPlaybackRate

   Returns the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

unsigned BLASTER_GetPlaybackRate
   (
   void
   )

   {
   return( BLASTER_SampleRate );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetMixMode

   Sets the sound card to play samples in mono or stereo.
---------------------------------------------------------------------*/

int BLASTER_SetMixMode
   (
   int mode
   )

   {
   int   port;
   int   data;
   int   CardType;

   CardType = BLASTER_Config.Type;

   mode &= BLASTER_MaxMixMode;

   if ( !( BLASTER_Card.MaxMixMode & STEREO ) )
      {
      mode &= ~STEREO;
      }

   if ( !( BLASTER_Card.MaxMixMode & SIXTEEN_BIT ) )
      {
      mode &= ~SIXTEEN_BIT;
      }

   BLASTER_MixMode = mode;
   BLASTER_SamplePacketSize = BLASTER_SampleSize[ mode ];

   // For the Sound Blaster Pro, we have to set the mixer chip
   // to play mono or stereo samples.

   if ( ( CardType == SBPro ) || ( CardType == SBPro2 ) )
      {
      port = BLASTER_Config.Address + BLASTER_MixerAddressPort;
      outp( port, MIXER_SBProOutputSetting );

      port = BLASTER_Config.Address + BLASTER_MixerDataPort;

      // Get current mode
      data = inp( port );

      // set stereo mode bit
      if ( mode & STEREO )
         {
         data |= MIXER_SBProStereoFlag;
         }
      else
         {
         data &= ~MIXER_SBProStereoFlag;
         }

      // set the mode
      outp( port, data );

      BLASTER_SetPlaybackRate( BLASTER_SampleRate );
      }

   return( mode );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_StopPlayback

   Ends the DMA transfer of digitized sound to the sound card.
---------------------------------------------------------------------*/

void BLASTER_StopPlayback
   (
   void
   )

   {
   int DmaChannel;

   // Don't allow anymore interrupts
   BLASTER_DisableInterrupt();

   if ( BLASTER_HaltTransferCommand == DSP_Reset )
      {
      BLASTER_ResetDSP();
      }
   else
      {
      BLASTER_WriteDSP( BLASTER_HaltTransferCommand );
      }

   // Disable the DMA channel
   if ( BLASTER_MixMode & SIXTEEN_BIT )
      {
      DmaChannel = BLASTER_Config.Dma16;
      }
   else
      {
      DmaChannel = BLASTER_Config.Dma8;
      }
   DMA_EndTransfer( DmaChannel );

   // Turn off speaker
   BLASTER_SpeakerOff();

   BLASTER_SoundPlaying = FALSE;
   BLASTER_SoundRecording = FALSE;

   BLASTER_DMABuffer = NULL;
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetupDMABuffer

   Programs the DMAC for sound transfer.
---------------------------------------------------------------------*/

int BLASTER_SetupDMABuffer
   (
   char *BufferPtr,
   int   BufferSize,
   int   mode
   )

   {
   int DmaChannel;
   int DmaStatus;
   int errorcode;

   if ( BLASTER_MixMode & SIXTEEN_BIT )
      {
      DmaChannel = BLASTER_Config.Dma16;
      errorcode  = BLASTER_DMA16NotSet;
      }
   else
      {
      DmaChannel = BLASTER_Config.Dma8;
      errorcode  = BLASTER_DMANotSet;
      }

   if ( DmaChannel == UNDEFINED )
      {
      BLASTER_SetErrorCode( errorcode );
      return( BLASTER_Error );
      }

   DmaStatus = DMA_SetupTransfer( DmaChannel, BufferPtr, BufferSize, mode );
   if ( DmaStatus == DMA_Error )
      {
      BLASTER_SetErrorCode( BLASTER_DmaError );
      return( BLASTER_Error );
      }

   BLASTER_DMAChannel = DmaChannel;

   BLASTER_DMABuffer          = BufferPtr;
   BLASTER_CurrentDMABuffer   = BufferPtr;
   BLASTER_TotalDMABufferSize = BufferSize;
   BLASTER_DMABufferEnd       = BufferPtr + BufferSize;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetCurrentPos

   Returns the offset within the current sound being played.
---------------------------------------------------------------------*/

int BLASTER_GetCurrentPos
   (
   void
   )

   {
   char *CurrentAddr;
   int   DmaChannel;
   int   offset;

   if ( !BLASTER_SoundPlaying )
      {
      BLASTER_SetErrorCode( BLASTER_NoSoundPlaying );
      return( BLASTER_Error );
      }

   if ( BLASTER_MixMode & SIXTEEN_BIT )
      {
      DmaChannel = BLASTER_Config.Dma16;
      }
   else
      {
      DmaChannel = BLASTER_Config.Dma8;
      }

   if ( DmaChannel == UNDEFINED )
      {
      BLASTER_SetErrorCode( BLASTER_DMANotSet );
      return( BLASTER_Error );
      }

   CurrentAddr = DMA_GetCurrentPos( DmaChannel );

   offset = ( int )( ( ( unsigned long )CurrentAddr ) -
      ( ( unsigned long )BLASTER_CurrentDMABuffer ) );

   if ( BLASTER_MixMode & SIXTEEN_BIT )
      {
      offset >>= 1;
      }

   if ( BLASTER_MixMode & STEREO )
      {
      offset >>= 1;
      }

   return( offset );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP1xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 1.xx.
---------------------------------------------------------------------*/

int BLASTER_DSP1xx_BeginPlayback
   (
   int length
   )

   {
   int SampleLength;
   int LoByte;
   int HiByte;

   SampleLength = length - 1;
   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   // Program DSP to play sound
   BLASTER_WriteDSP( DSP_Old8BitDAC );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

   BLASTER_SoundPlaying = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP2xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 2.xx.
---------------------------------------------------------------------*/

int BLASTER_DSP2xx_BeginPlayback
   (
   int length
   )

   {
   int SampleLength;
   int LoByte;
   int HiByte;

   SampleLength = length - 1;
   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   BLASTER_WriteDSP( DSP_SetBlockLength );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   if ( ( BLASTER_Version >= DSP_Version201 ) && ( DSP_MaxNormalRate <
      ( BLASTER_SampleRate * BLASTER_SamplePacketSize ) ) )
      {
      BLASTER_WriteDSP( DSP_8BitHighSpeedAutoInitMode );
      BLASTER_HaltTransferCommand = DSP_Reset;
      }
   else
      {
      BLASTER_WriteDSP( DSP_8BitAutoInitMode );
      BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;
      }

   BLASTER_SoundPlaying = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP4xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 4.xx, such as the Sound Blaster 16.
---------------------------------------------------------------------*/

int BLASTER_DSP4xx_BeginPlayback
   (
   int length
   )

   {
   int TransferCommand;
   int TransferMode;
   int SampleLength;
   int LoByte;
   int HiByte;

   if ( BLASTER_MixMode & SIXTEEN_BIT )
      {
      TransferCommand = DSP_16BitDAC;
      SampleLength = ( length / 2 ) - 1;
      BLASTER_HaltTransferCommand = DSP_Halt16bitTransfer;
      if ( BLASTER_MixMode & STEREO )
         {
         TransferMode = DSP_SignedStereoData;
         }
      else
         {
         TransferMode = DSP_SignedMonoData;
         }
      }
   else
      {
      TransferCommand = DSP_8BitDAC;
      SampleLength = length - 1;
      BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;
      if ( BLASTER_MixMode & STEREO )
         {
         TransferMode = DSP_UnsignedStereoData;
         }
      else
         {
         TransferMode = DSP_UnsignedMonoData;
         }
      }

   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   // Program DSP to play sound
   BLASTER_WriteDSP( TransferCommand );
   BLASTER_WriteDSP( TransferMode );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   BLASTER_SoundPlaying = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_BeginBufferedPlayback

   Begins multibuffered playback of digitized sound on the sound card.
---------------------------------------------------------------------*/

int BLASTER_BeginBufferedPlayback
   (
   char    *BufferStart,
   int      BufferSize,
   int      NumDivisions,
   unsigned SampleRate,
   int      MixMode,
   void  ( *CallBackFunc )( void )
   )

   {
   int DmaStatus;
   int TransferLength;

//JIM
//   if ( BLASTER_SoundPlaying || BLASTER_SoundRecording )
      {
      BLASTER_StopPlayback();
      }

   BLASTER_SetMixMode( MixMode );

   DmaStatus = BLASTER_SetupDMABuffer( BufferStart, BufferSize, DMA_AutoInitRead );
   if ( DmaStatus == BLASTER_Error )
      {
      return( BLASTER_Error );
      }

   BLASTER_SetPlaybackRate( SampleRate );

   BLASTER_SetCallBack( CallBackFunc );

   BLASTER_EnableInterrupt();

   // Turn on speaker
   BLASTER_SpeakerOn();

   TransferLength = BufferSize / NumDivisions;
   BLASTER_TransferLength = TransferLength;

   //  Program the sound card to start the transfer.
   if ( BLASTER_Version < DSP_Version2xx )
      {
      BLASTER_DSP1xx_BeginPlayback( TransferLength );
      }
   else if ( BLASTER_Version < DSP_Version4xx )
      {
      BLASTER_DSP2xx_BeginPlayback( TransferLength );
      }
   else
      {
      BLASTER_DSP4xx_BeginPlayback( TransferLength );
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP4xx_BeginRecord

   Starts recording of digitized sound on cards compatible with DSP
   version 4.xx, such as the Sound Blaster 16.
---------------------------------------------------------------------*/

int BLASTER_DSP4xx_BeginRecord
   (
   int length
   )

   {
   int TransferCommand;
   int TransferMode;
   int SampleLength;
   int LoByte;
   int HiByte;

   TransferCommand = DSP_8BitADC;
   SampleLength = length - 1;
   BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

   TransferMode = DSP_UnsignedMonoData;

   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   // Program DSP to play sound
   BLASTER_WriteDSP( TransferCommand );
   BLASTER_WriteDSP( TransferMode );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   BLASTER_SoundRecording = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP2xx_BeginRecord

   Starts recording of digitized sound on cards compatible with DSP
   version 2.xx.
---------------------------------------------------------------------*/

int BLASTER_DSP2xx_BeginRecord
   (
   int length
   )

   {
   int SampleLength;
   int LoByte;
   int HiByte;

   SampleLength = length - 1;
   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   BLASTER_WriteDSP( DSP_SetBlockLength );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   if ( ( BLASTER_Version >= DSP_Version201 ) && ( DSP_MaxNormalRate <
      ( BLASTER_SampleRate * BLASTER_SamplePacketSize ) ) )
      {
      BLASTER_WriteDSP( DSP_8BitHighSpeedAutoInitRecord );
      BLASTER_HaltTransferCommand = DSP_Reset;
      }
   else
      {
      BLASTER_WriteDSP( DSP_8BitAutoInitRecord );
      BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;
      }

   BLASTER_SoundRecording = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_DSP1xx_BeginRecord

   Starts recording of digitized sound on cards compatible with DSP
   version 1.xx.
---------------------------------------------------------------------*/

int BLASTER_DSP1xx_BeginRecord
   (
   int length
   )

   {
   int SampleLength;
   int LoByte;
   int HiByte;

   SampleLength = length - 1;
   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );

   // Program DSP to play sound
   BLASTER_WriteDSP( DSP_Old8BitADC );
   BLASTER_WriteDSP( LoByte );
   BLASTER_WriteDSP( HiByte );

   BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

   BLASTER_SoundRecording = TRUE;

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_BeginBufferedRecord

   Begins multibuffered recording of digitized sound on the sound card.
---------------------------------------------------------------------*/

int BLASTER_BeginBufferedRecord
   (
   char    *BufferStart,
   int      BufferSize,
   int      NumDivisions,
   unsigned SampleRate,
   int      MixMode,
   void  ( *CallBackFunc )( void )
   )

   {
   int DmaStatus;
   int TransferLength;

//JIM
//   if ( BLASTER_SoundPlaying || BLASTER_SoundRecording )
      {
      BLASTER_StopPlayback();
      }

   BLASTER_SetMixMode( MixMode );

   DmaStatus = BLASTER_SetupDMABuffer( BufferStart, BufferSize, DMA_AutoInitWrite );
   if ( DmaStatus == BLASTER_Error )
      {
      return( BLASTER_Error );
      }

   BLASTER_SetPlaybackRate( SampleRate );

   BLASTER_SetCallBack( CallBackFunc );

   BLASTER_EnableInterrupt();

   // Turn off speaker
   BLASTER_SpeakerOff();

   TransferLength = BufferSize / NumDivisions;
   BLASTER_TransferLength = TransferLength;

   //  Program the sound card to start the transfer.
   if ( BLASTER_Version < DSP_Version2xx )
      {
      BLASTER_DSP1xx_BeginRecord( TransferLength );
      }
   else if ( BLASTER_Version < DSP_Version4xx )
      {
      BLASTER_DSP2xx_BeginRecord( TransferLength );
      }
   else
      {
      BLASTER_DSP4xx_BeginRecord( TransferLength );
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_WriteMixer

   Writes a byte of data to the Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

void BLASTER_WriteMixer
   (
   int reg,
   int data
   )

   {
   outp( BLASTER_MixerAddress + BLASTER_MixerAddressPort, reg );
   outp( BLASTER_MixerAddress + BLASTER_MixerDataPort, data );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_ReadMixer

   Reads a byte of data from the Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

int BLASTER_ReadMixer
   (
   int reg
   )

   {
   int data;

   outp( BLASTER_MixerAddress + BLASTER_MixerAddressPort, reg );
   data = inp( BLASTER_MixerAddress + BLASTER_MixerDataPort );
   return( data );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetVoiceVolume

   Reads the average volume of the digitized sound channel from the
   Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

int BLASTER_GetVoiceVolume
   (
   void
   )

   {
   int volume;
   int left;
   int right;

   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         left   = BLASTER_ReadMixer( MIXER_SBProVoice );
         right  = ( left & 0x0f ) << 4;
         left  &= 0xf0;
         volume = ( left + right ) / 2;
         break;

      case SB16 :
         left  = BLASTER_ReadMixer( MIXER_SB16VoiceLeft );
         right = BLASTER_ReadMixer( MIXER_SB16VoiceRight );
         volume = ( left + right ) / 2;
         break;

      default :
         BLASTER_SetErrorCode( BLASTER_NoMixer );
         volume = BLASTER_Error;
      }

   return( volume );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetVoiceVolume

   Sets the volume of the digitized sound channel on the Sound
   Blaster's mixer chip.
---------------------------------------------------------------------*/

int BLASTER_SetVoiceVolume
   (
   int volume
   )

   {
   int data;
   int status;

   volume = min( 255, volume );
   volume = max( 0, volume );

   status = BLASTER_Ok;
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         data = ( volume & 0xf0 ) + ( volume >> 4 );
         BLASTER_WriteMixer( MIXER_SBProVoice, data );
         break;

      case SB16 :
         BLASTER_WriteMixer( MIXER_SB16VoiceLeft, volume & 0xf8 );
         BLASTER_WriteMixer( MIXER_SB16VoiceRight, volume & 0xf8 );
         break;

      default :
         BLASTER_SetErrorCode( BLASTER_NoMixer );
         status = BLASTER_Error;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetMidiVolume

   Reads the average volume of the Midi sound channel from the
   Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

int BLASTER_GetMidiVolume
   (
   void
   )

   {
   int volume;
   int left;
   int right;

   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         left   = BLASTER_ReadMixer( MIXER_SBProMidi );
         right  = ( left & 0x0f ) << 4;
         left  &= 0xf0;
         volume = ( left + right ) / 2;
         break;

      case SB16 :
         left  = BLASTER_ReadMixer( MIXER_SB16MidiLeft );
         right = BLASTER_ReadMixer( MIXER_SB16MidiRight );
         volume = ( left + right ) / 2;
         break;

      default :
         BLASTER_SetErrorCode( BLASTER_NoMixer );
         volume = BLASTER_Error;
      }

   return( volume );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetMidiVolume

   Sets the volume of the Midi sound channel on the Sound
   Blaster's mixer chip.
---------------------------------------------------------------------*/

int BLASTER_SetMidiVolume
   (
   int volume
   )

   {
   int data;
   int status;

   volume = min( 255, volume );
   volume = max( 0, volume );

   status = BLASTER_Ok;
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         data = ( volume & 0xf0 ) + ( volume >> 4 );
         BLASTER_WriteMixer( MIXER_SBProMidi, data );
         break;

      case SB16 :
         BLASTER_WriteMixer( MIXER_SB16MidiLeft, volume & 0xf8 );
         BLASTER_WriteMixer( MIXER_SB16MidiRight, volume & 0xf8 );
         break;

      default :
         BLASTER_SetErrorCode( BLASTER_NoMixer );
         status = BLASTER_Error;
      }

   return( status );
   }

/*---------------------------------------------------------------------
   Function: BLASTER_CardHasMixer

   Checks if the selected Sound Blaster card has a mixer.
---------------------------------------------------------------------*/

int BLASTER_CardHasMixer
   (
   void
   )

   {
   return( BLASTER_Card.HasMixer );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SaveVoiceVolume

   Saves the user's voice mixer settings.
---------------------------------------------------------------------*/

void BLASTER_SaveVoiceVolume
   (
   void
   )

   {
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         BLASTER_OriginalVoiceVolumeLeft =
            BLASTER_ReadMixer( MIXER_SBProVoice );
         break;

      case SB16 :
         BLASTER_OriginalVoiceVolumeLeft =
            BLASTER_ReadMixer( MIXER_SB16VoiceLeft );
         BLASTER_OriginalVoiceVolumeRight =
            BLASTER_ReadMixer( MIXER_SB16VoiceRight );
         break;
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_RestoreVoiceVolume

   Restores the user's voice mixer settings.
---------------------------------------------------------------------*/

void BLASTER_RestoreVoiceVolume
   (
   void
   )

   {
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         BLASTER_WriteMixer( MIXER_SBProVoice,
            BLASTER_OriginalVoiceVolumeLeft );
         break;

      case SB16 :
         BLASTER_WriteMixer( MIXER_SB16VoiceLeft,
            BLASTER_OriginalVoiceVolumeLeft );
         BLASTER_WriteMixer( MIXER_SB16VoiceRight,
            BLASTER_OriginalVoiceVolumeRight );
         break;
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SaveMidiVolume

   Saves the user's FM mixer settings.
---------------------------------------------------------------------*/

void BLASTER_SaveMidiVolume
   (
   void
   )

   {
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         BLASTER_OriginalMidiVolumeLeft =
            BLASTER_ReadMixer( MIXER_SBProMidi );
         break;

      case SB16 :
         BLASTER_OriginalMidiVolumeLeft =
            BLASTER_ReadMixer( MIXER_SB16MidiLeft );
         BLASTER_OriginalMidiVolumeRight =
            BLASTER_ReadMixer( MIXER_SB16MidiRight );
         break;
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_RestoreMidiVolume

   Restores the user's FM mixer settings.
---------------------------------------------------------------------*/

void BLASTER_RestoreMidiVolume
   (
   void
   )

   {
   switch( BLASTER_MixerType )
      {
      case SBPro :
      case SBPro2 :
         BLASTER_WriteMixer( MIXER_SBProMidi,
            BLASTER_OriginalMidiVolumeLeft );
         break;

      case SB16 :
         BLASTER_WriteMixer( MIXER_SB16MidiLeft,
            BLASTER_OriginalMidiVolumeLeft );
         BLASTER_WriteMixer( MIXER_SB16MidiRight,
            BLASTER_OriginalMidiVolumeRight );
         break;
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetEnv

   Retrieves the BLASTER environment settings and returns them to
   the caller.
---------------------------------------------------------------------*/

int BLASTER_GetEnv
   (
   BLASTER_CONFIG *Config
   )

   {
   char *Blaster;
   char parameter;

   Config->Address   = UNDEFINED;
   Config->Type      = UNDEFINED;
   Config->Interrupt = UNDEFINED;
   Config->Dma8      = UNDEFINED;
   Config->Dma16     = UNDEFINED;
   Config->Midi      = UNDEFINED;
   Config->Emu       = UNDEFINED;

   Blaster = getenv( "BLASTER" );
   if ( Blaster == NULL )
      {
      BLASTER_SetErrorCode( BLASTER_EnvNotFound );
      return( BLASTER_Error );
      }

   while( *Blaster != 0 )
      {
      if ( *Blaster == ' ' )
         {
         Blaster++;
         continue;
         }

      parameter = toupper( *Blaster );
      Blaster++;

      if ( !isxdigit( *Blaster ) )
         {
         BLASTER_SetErrorCode( BLASTER_InvalidParameter );
         return( BLASTER_Error );
         }

      switch( parameter )
         {
         case BlasterEnv_Address :
            sscanf( Blaster, "%x", &Config->Address );
            break;
         case BlasterEnv_Interrupt :
            sscanf( Blaster, "%d", &Config->Interrupt );
            break;
         case BlasterEnv_8bitDma :
            sscanf( Blaster, "%d", &Config->Dma8 );
            break;
         case BlasterEnv_Type :
            sscanf( Blaster, "%d", &Config->Type );
            break;
         case BlasterEnv_16bitDma :
            sscanf( Blaster, "%d", &Config->Dma16 );
            break;
         case BlasterEnv_Midi :
            sscanf( Blaster, "%x", &Config->Midi );
            break;
         case BlasterEnv_EmuAddress :
            sscanf( Blaster, "%x", &Config->Emu );
            break;
         default  :
            // Skip the offending data
            // sscanf( Blaster, "%*s" );
            break;
         }

      while( isxdigit( *Blaster ) )
         {
         Blaster++;
         }
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetCardSettings

   Sets up the sound card's parameters.
---------------------------------------------------------------------*/

int BLASTER_SetCardSettings
   (
   BLASTER_CONFIG Config
   )

   {
   if ( BLASTER_Installed )
      {
      BLASTER_Shutdown();
      }

   BLASTER_Config.Address   = Config.Address;
   BLASTER_Config.Type      = Config.Type;
   BLASTER_Config.Interrupt = Config.Interrupt;
   BLASTER_Config.Dma8      = Config.Dma8;
   BLASTER_Config.Dma16     = Config.Dma16;
   BLASTER_Config.Midi      = Config.Midi;
   BLASTER_Config.Emu       = Config.Emu;
   BLASTER_MixerAddress     = Config.Address;
   BLASTER_MixerType        = Config.Type;

   if ( BLASTER_Config.Emu == UNDEFINED )
      {
      BLASTER_Config.Emu = BLASTER_Config.Address + 0x400;
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetCardSettings

   Sets up the sound card's parameters.
---------------------------------------------------------------------*/

int BLASTER_GetCardSettings
   (
   BLASTER_CONFIG *Config
   )

   {
   if ( BLASTER_Config.Address == UNDEFINED )
      {
      return( BLASTER_Warning );
      }
   else
      {
      Config->Address   = BLASTER_Config.Address;
      Config->Type      = BLASTER_Config.Type;
      Config->Interrupt = BLASTER_Config.Interrupt;
      Config->Dma8      = BLASTER_Config.Dma8;
      Config->Dma16     = BLASTER_Config.Dma16;
      Config->Midi      = BLASTER_Config.Midi;
      Config->Emu       = BLASTER_Config.Emu;
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_GetCardInfo

   Returns the maximum number of bits that can represent a sample
   (8 or 16) and the number of channels (1 for mono, 2 for stereo).
---------------------------------------------------------------------*/

int BLASTER_GetCardInfo
   (
   int *MaxSampleBits,
   int *MaxChannels
   )

   {
   if ( BLASTER_Card.MaxMixMode & STEREO )
      {
      *MaxChannels = 2;
      }
   else
      {
      *MaxChannels = 1;
      }

   if ( BLASTER_Card.MaxMixMode & SIXTEEN_BIT )
      {
      *MaxSampleBits = 16;
      }
   else
      {
      *MaxSampleBits = 8;
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetCallBack

   Specifies the user function to call at the end of a sound transfer.
---------------------------------------------------------------------*/

void BLASTER_SetCallBack
   (
   void ( *func )( void )
   )

   {
   BLASTER_CallBack = func;
   }


/*---------------------------------------------------------------------
   Function: BLASTER_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void BLASTER_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: BLASTER_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

void BLASTER_UnlockMemory
   (
   void
   )

   {
   DPMI_UnlockMemoryRegion( BLASTER_LockStart, BLASTER_LockEnd );
   DPMI_UnlockMemory( ( void * )&BLASTER_Interrupts[ 0 ],
      sizeof( BLASTER_Interrupts ) );
   DPMI_UnlockMemory( ( void * )&BLASTER_SampleSize[ 0 ],
      sizeof( BLASTER_SampleSize ) );
   DPMI_Unlock( BLASTER_Card );
   DPMI_Unlock( BLASTER_OldInt );
   DPMI_Unlock( BLASTER_Config );
   DPMI_Unlock( BLASTER_Installed );
   DPMI_Unlock( BLASTER_Version );
   DPMI_Unlock( BLASTER_DMABuffer );
   DPMI_Unlock( BLASTER_DMABufferEnd );
   DPMI_Unlock( BLASTER_CurrentDMABuffer );
   DPMI_Unlock( BLASTER_TotalDMABufferSize );
   DPMI_Unlock( BLASTER_TransferLength );
   DPMI_Unlock( BLASTER_MixMode );
   DPMI_Unlock( BLASTER_SamplePacketSize );
   DPMI_Unlock( BLASTER_SampleRate );
   DPMI_Unlock( BLASTER_HaltTransferCommand );
   DPMI_Unlock( ( int )BLASTER_SoundPlaying );
   DPMI_Unlock( ( int )BLASTER_SoundRecording );
   DPMI_Unlock( BLASTER_CallBack );
   DPMI_Unlock( BLASTER_IntController1Mask );
   DPMI_Unlock( BLASTER_IntController2Mask );
   DPMI_Unlock( BLASTER_MixerAddress );
   DPMI_Unlock( BLASTER_MixerType );
   DPMI_Unlock( BLASTER_OriginalMidiVolumeLeft );
   DPMI_Unlock( BLASTER_OriginalMidiVolumeRight );
   DPMI_Unlock( BLASTER_OriginalVoiceVolumeLeft );
   DPMI_Unlock( BLASTER_OriginalVoiceVolumeRight );
   DPMI_Unlock( GlobalStatus );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int BLASTER_LockMemory
   (
   void
   )

   {
   int status;

   status  = DPMI_LockMemoryRegion( BLASTER_LockStart, BLASTER_LockEnd );
   status |= DPMI_LockMemory( ( void * )&BLASTER_Interrupts[ 0 ],
      sizeof( BLASTER_Interrupts ) );
   status |= DPMI_LockMemory( ( void * )&BLASTER_SampleSize[ 0 ],
      sizeof( BLASTER_SampleSize ) );
   status |= DPMI_Lock( BLASTER_Card );
   status |= DPMI_Lock( BLASTER_OldInt );
   status |= DPMI_Lock( BLASTER_Config );
   status |= DPMI_Lock( BLASTER_Installed );
   status |= DPMI_Lock( BLASTER_Version );
   status |= DPMI_Lock( BLASTER_DMABuffer );
   status |= DPMI_Lock( BLASTER_DMABufferEnd );
   status |= DPMI_Lock( BLASTER_CurrentDMABuffer );
   status |= DPMI_Lock( BLASTER_TotalDMABufferSize );
   status |= DPMI_Lock( BLASTER_TransferLength );
   status |= DPMI_Lock( BLASTER_MixMode );
   status |= DPMI_Lock( BLASTER_SamplePacketSize );
   status |= DPMI_Lock( BLASTER_SampleRate );
   status |= DPMI_Lock( BLASTER_HaltTransferCommand );
   status |= DPMI_Lock( ( ( int )BLASTER_SoundPlaying ) );
   status |= DPMI_Lock( ( ( int )BLASTER_SoundRecording ) );
   status |= DPMI_Lock( BLASTER_CallBack );
   status |= DPMI_Lock( BLASTER_IntController1Mask );
   status |= DPMI_Lock( BLASTER_IntController2Mask );
   status |= DPMI_Lock( BLASTER_MixerAddress );
   status |= DPMI_Lock( BLASTER_MixerType );
   status |= DPMI_Lock( BLASTER_OriginalMidiVolumeLeft );
   status |= DPMI_Lock( BLASTER_OriginalMidiVolumeRight );
   status |= DPMI_Lock( BLASTER_OriginalVoiceVolumeLeft );
   status |= DPMI_Lock( BLASTER_OriginalVoiceVolumeRight );
   status |= DPMI_Lock( GlobalStatus );

   if ( status != DPMI_Ok )
      {
      BLASTER_UnlockMemory();
      BLASTER_SetErrorCode( BLASTER_DPMI_Error );
      return( BLASTER_Error );
      }

   return( BLASTER_Ok );
   }


/*---------------------------------------------------------------------
   Function: allocateTimerStack

   Allocate a block of memory from conventional (low) memory and return
   the selector (which can go directly into a segment register) of the
   memory block or 0 if an error occured.
---------------------------------------------------------------------*/

static unsigned short allocateTimerStack
   (
   unsigned short size
   )

   {
   union REGS regs;

   // clear all registers
   memset( &regs, 0, sizeof( regs ) );

   // DPMI allocate conventional memory
   regs.w.ax = 0x100;

   // size in paragraphs
   regs.w.bx = ( size + 15 ) / 16;

   int386( 0x31, &regs, &regs );
   if (!regs.w.cflag)
      {
      // DPMI call returns selector in dx
      // (ax contains real mode segment
      // which is ignored here)

      return( regs.w.dx );
      }

   // Couldn't allocate memory.
   return( NULL );
   }


/*---------------------------------------------------------------------
   Function: deallocateTimerStack

   Deallocate a block of conventional (low) memory given a selector to
   it.  Assumes the block was allocated with DPMI function 0x100.
---------------------------------------------------------------------*/

static void deallocateTimerStack
   (
   unsigned short selector
   )

   {
   union REGS regs;

   if ( selector != NULL )
      {
      // clear all registers
      memset( &regs, 0, sizeof( regs ) );

      regs.w.ax = 0x101;
      regs.w.dx = selector;
      int386( 0x31, &regs, &regs );
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_SetupWaveBlaster

   Allows the WaveBlaster to play music while the Sound Blaster 16
   plays digital sound.
---------------------------------------------------------------------*/

void BLASTER_SetupWaveBlaster
   (
   void
   )

   {

   if ( BLASTER_MixerType == SB16 )
      {
      // Disable MPU401 interrupts.  If they are not disabled,
      // the SB16 will not produce sound or music.
      BLASTER_WaveBlasterState = BLASTER_ReadMixer( MIXER_DSP4xxISR_Enable );
      BLASTER_WriteMixer( MIXER_DSP4xxISR_Enable, MIXER_DisableMPU401Interrupts );
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_ShutdownWaveBlaster

   Restores WaveBlaster mixer to original state.
---------------------------------------------------------------------*/

void BLASTER_ShutdownWaveBlaster
   (
   void
   )

   {
   if ( BLASTER_MixerType == SB16 )
      {
      // Restore the state of MPU401 interrupts.  If they are not disabled,
      // the SB16 will not produce sound or music.
      BLASTER_WriteMixer( MIXER_DSP4xxISR_Enable, BLASTER_WaveBlasterState );
      }
   }


/*---------------------------------------------------------------------
   Function: BLASTER_Init

   Initializes the sound card and prepares the module to play
   digitized sounds.
---------------------------------------------------------------------*/

int BLASTER_Init
   (
   void
   )

   {
   int Irq;
   int Interrupt;
   int status;

   if ( BLASTER_Installed )
      {
      BLASTER_Shutdown();
      }

   if ( BLASTER_Config.Address == UNDEFINED )
      {
      BLASTER_SetErrorCode( BLASTER_AddrNotSet );
      return( BLASTER_Error );
      }

   // Save the interrupt masks
   BLASTER_IntController1Mask = inp( 0x21 );
   BLASTER_IntController2Mask = inp( 0xA1 );

   status = BLASTER_ResetDSP();
   if ( status == BLASTER_Ok )
      {
      BLASTER_SaveVoiceVolume();

      BLASTER_SoundPlaying = FALSE;

      BLASTER_SetCallBack( NULL );

      BLASTER_DMABuffer = NULL;

      BLASTER_Version = BLASTER_GetDSPVersion();

      BLASTER_SetPlaybackRate( BLASTER_DefaultSampleRate );
      BLASTER_SetMixMode( BLASTER_DefaultMixMode );

      if ( BLASTER_Config.Dma16 != UNDEFINED )
         {
         status = DMA_VerifyChannel( BLASTER_Config.Dma16 );
         if ( status == DMA_Error )
            {
            BLASTER_SetErrorCode( BLASTER_DmaError );
            return( BLASTER_Error );
            }
         }

      if ( BLASTER_Config.Dma8 != UNDEFINED )
         {
         status = DMA_VerifyChannel( BLASTER_Config.Dma8 );
         if ( status == DMA_Error )
            {
            BLASTER_SetErrorCode( BLASTER_DmaError );
            return( BLASTER_Error );
            }
         }

      // Install our interrupt handler
      Irq = BLASTER_Config.Interrupt;
      if ( !VALID_IRQ( Irq ) )
         {
         BLASTER_SetErrorCode( BLASTER_InvalidIrq );
         return( BLASTER_Error );
         }

      Interrupt = BLASTER_Interrupts[ Irq ];
      if ( Interrupt == INVALID )
         {
         BLASTER_SetErrorCode( BLASTER_InvalidIrq );
         return( BLASTER_Error );
         }

      status = BLASTER_LockMemory();
      if ( status != BLASTER_Ok )
         {
         BLASTER_UnlockMemory();
         return( status );
         }

      StackSelector = allocateTimerStack( kStackSize );
      if ( StackSelector == NULL )
         {
         BLASTER_UnlockMemory();
         BLASTER_SetErrorCode( BLASTER_OutOfMemory );
         return( BLASTER_Error );
         }

      // Leave a little room at top of stack just for the hell of it...
      StackPointer = kStackSize - sizeof( long );

      BLASTER_OldInt = _dos_getvect( Interrupt );
      if ( Irq < 8 )
         {
         _dos_setvect( Interrupt, BLASTER_ServiceInterrupt );
         }
      else
         {
         status = IRQ_SetVector( Interrupt, BLASTER_ServiceInterrupt );
         if ( status != IRQ_Ok )
            {
            BLASTER_UnlockMemory();
            deallocateTimerStack( StackSelector );
            StackSelector = NULL;
            BLASTER_SetErrorCode( BLASTER_UnableToSetIrq );
            return( BLASTER_Error );
            }
         }

      BLASTER_Installed = TRUE;
      status = BLASTER_Ok;
      }

   BLASTER_SetErrorCode( status );
   return( status );
   }


/*---------------------------------------------------------------------
   Function: BLASTER_Shutdown

   Ends transfer of sound data to the sound card and restores the
   system resources used by the card.
---------------------------------------------------------------------*/

void BLASTER_Shutdown
   (
   void
   )

   {
   int Irq;
   int Interrupt;

   // Halt the DMA transfer
   BLASTER_StopPlayback();

   BLASTER_RestoreVoiceVolume();

   // Reset the DSP
   BLASTER_ResetDSP();

   // Restore the original interrupt
   Irq = BLASTER_Config.Interrupt;
   Interrupt = BLASTER_Interrupts[ Irq ];
   if ( Irq >= 8 )
      {
      IRQ_RestoreVector( Interrupt );
      }
   _dos_setvect( Interrupt, BLASTER_OldInt );

   BLASTER_SoundPlaying = FALSE;

   BLASTER_DMABuffer = NULL;

   BLASTER_SetCallBack( NULL );

   BLASTER_UnlockMemory();

   deallocateTimerStack( StackSelector );
   StackSelector = NULL;

   BLASTER_Installed = FALSE;
   }
