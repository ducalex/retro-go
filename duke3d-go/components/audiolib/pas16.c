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
   module: PAS16.C

   author: James R. Dose
   date:   March 27, 1994

   Low level routines to support Pro AudioSpectrum and compatible
   sound cards.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dpmi.h"
#include "dma.h"
#include "interrup.h"
#include "irq.h"
#include "pas16.h"
#include "_pas16.h"

#define USESTACK

static const int PAS_Interrupts[ PAS_MaxIrq + 1 ]  =
   {
   INVALID, INVALID,     0xa,     0xb,
   INVALID,     0xd, INVALID,     0xf,
   INVALID, INVALID,    0x72,    0x73,
      0x74, INVALID, INVALID,    0x77
   };

static void    ( interrupt far *PAS_OldInt )( void );

static int PAS_IntController1Mask;
static int PAS_IntController2Mask;

static int PAS_Installed = FALSE;
static int PAS_TranslateCode = DEFAULT_BASE;

static int PAS_OriginalPCMLeftVolume  = 75;
static int PAS_OriginalPCMRightVolume = 75;

static int PAS_OriginalFMLeftVolume  = 75;
static int PAS_OriginalFMRightVolume = 75;

unsigned int PAS_DMAChannel;
static int PAS_Irq;

static MVState *PAS_State = NULL;
static MVFunc  *PAS_Func  = NULL;

static MVState PAS_OriginalState;
static int     PAS_SampleSizeConfig;

static char   *PAS_DMABuffer;
static char   *PAS_DMABufferEnd;
static char   *PAS_CurrentDMABuffer;
static int     PAS_TotalDMABufferSize;

static int      PAS_TransferLength   = 0;
static int      PAS_MixMode          = PAS_DefaultMixMode;
static unsigned PAS_SampleRate       = PAS_DefaultSampleRate;
static int      PAS_TimeInterval     = 0;

volatile int   PAS_SoundPlaying;

void ( *PAS_CallBack )( void );

// adequate stack size
#define kStackSize 2048

static unsigned short StackSelector = NULL;
static unsigned long  StackPointer;

static unsigned short oldStackSelector;
static unsigned long  oldStackPointer;

// This is defined because we can't create local variables in a
// function that switches stacks.
static int irqstatus;

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

int PAS_ErrorCode = PAS_Ok;

#define PAS_SetErrorCode( status ) \
   PAS_ErrorCode   = ( status );

/*---------------------------------------------------------------------
   Function: PAS_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *PAS_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case PAS_Warning :
      case PAS_Error :
         ErrorString = PAS_ErrorString( PAS_ErrorCode );
         break;

      case PAS_Ok :
         ErrorString = "Pro AudioSpectrum ok.";
         break;

      case PAS_DriverNotFound :
         ErrorString = "MVSOUND.SYS not loaded.";
         break;

      case PAS_DmaError :
         ErrorString = DMA_ErrorString( DMA_Error );
         break;

      case PAS_InvalidIrq :
         ErrorString = "Invalid Pro AudioSpectrum Irq.";
         break;

      case PAS_UnableToSetIrq :
         ErrorString = "Unable to set Pro AudioSpectrum IRQ.  Try selecting an IRQ of 7 or below.";
         break;

      case PAS_Dos4gwIrqError :
         ErrorString = "Unsupported Pro AudioSpectrum Irq.";
         break;

      case PAS_NoSoundPlaying :
         ErrorString = "No sound playing on Pro AudioSpectrum.";
         break;

      case PAS_CardNotFound :
         ErrorString = "Could not find Pro AudioSpectrum.";
         break;

      case PAS_DPMI_Error :
         ErrorString = "DPMI Error in PAS16.";
         break;

      case PAS_OutOfMemory :
         ErrorString = "Out of conventional memory in PAS16.";
         break;

      default :
         ErrorString = "Unknown Pro AudioSpectrum error code.";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define PAS_LockStart PAS_CheckForDriver


/*---------------------------------------------------------------------
   Function: PAS_CheckForDriver

   Checks to see if MVSOUND.SYS is installed.
---------------------------------------------------------------------*/

int PAS_CheckForDriver
   (
   void
   )

   {
   union REGS regs;
   unsigned   result;

   regs.w.ax = MV_CheckForDriver;
   regs.w.bx = 0x3f3f;

   #ifdef __386__
      int386( MV_SoundInt, &regs, &regs );
   #else
      int86( MV_SoundInt, &regs, &regs );
   #endif

   if ( regs.w.ax != MV_CheckForDriver )
      {
      PAS_SetErrorCode( PAS_DriverNotFound );
      return( PAS_Error );
      }

   result = regs.w.bx ^ regs.w.cx ^ regs.w.dx;
   if ( result != MV_Signature )
      {
      PAS_SetErrorCode( PAS_DriverNotFound );
      return( PAS_Error );
      }

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetStateTable

   Returns a pointer to the state table containing hardware state
   information.  The state table is necessary because the Pro Audio-
   Spectrum contains only write-only registers.
---------------------------------------------------------------------*/

MVState *PAS_GetStateTable
   (
   void
   )

   {
   union REGS   regs;
   MVState *ptr;

   regs.w.ax = MV_GetPointerToStateTable;

   #ifdef __386__
      int386( MV_SoundInt, &regs, &regs );
   #else
      int86( MV_SoundInt, &regs, &regs );
   #endif

   if ( regs.w.ax != MV_Signature )
      {
      PAS_SetErrorCode( PAS_DriverNotFound );
      return( NULL );
      }

   #if defined(__WATCOMC__) && defined(__FLAT__)
      ptr = ( MVState * )( ( ( ( unsigned )regs.w.dx ) << 4 ) +
         ( ( unsigned )regs.w.bx ) );
   #else
      ptr = MK_FP( regs.w.dx, regs.w.bx );
   #endif

   return( ptr );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetFunctionTable

   Returns a pointer to the function table containing addresses of
   driver functions.
---------------------------------------------------------------------*/

MVFunc *PAS_GetFunctionTable
   (
   void
   )

   {
   union REGS   regs;
   MVFunc *ptr;

   regs.w.ax = MV_GetPointerToFunctionTable;

   #ifdef __386__
      int386( MV_SoundInt, &regs, &regs );
   #else
      int86( MV_SoundInt, &regs, &regs );
   #endif

   if ( regs.w.ax != MV_Signature )
      {
      PAS_SetErrorCode( PAS_DriverNotFound );
      return( NULL );
      }

   #if defined(__WATCOMC__) && defined(__FLAT__)
      ptr = ( MVFunc * )( ( ( ( unsigned )regs.w.dx ) << 4 ) +
         ( ( unsigned )regs.w.bx ) );
   #else
      ptr = MK_FP( regs.w.dx, regs.w.bx );
   #endif

   return( ptr );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetCardSettings

   Returns the DMA and the IRQ channels of the sound card.
---------------------------------------------------------------------*/

int PAS_GetCardSettings
   (
   void
   )

   {
   union REGS   regs;
   int          status;

   regs.w.ax = MV_GetDmaIrqInt;

   #ifdef __386__
      int386( MV_SoundInt, &regs, &regs );
   #else
      int86( MV_SoundInt, &regs, &regs );
   #endif

   if ( regs.w.ax != MV_Signature )
      {
      PAS_SetErrorCode( PAS_DriverNotFound );
      return( PAS_Error );
      }

   PAS_DMAChannel = regs.w.bx;
   PAS_Irq        = regs.w.cx;

   if ( PAS_Irq > PAS_MaxIrq )
      {
      PAS_SetErrorCode( PAS_Dos4gwIrqError );
      return( PAS_Error );
      }

   if ( !VALID_IRQ( PAS_Irq ) )
      {
      PAS_SetErrorCode( PAS_InvalidIrq );
      return( PAS_Error );
      }

   if ( PAS_Interrupts[ PAS_Irq ] == INVALID )
      {
      PAS_SetErrorCode( PAS_InvalidIrq );
      return( PAS_Error );
      }

   status = DMA_VerifyChannel( PAS_DMAChannel );
   if ( status == DMA_Error )
      {
      PAS_SetErrorCode( PAS_DmaError );
      return( PAS_Error );
      }

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_EnableInterrupt

   Enables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

void PAS_EnableInterrupt
   (
   void
   )

   {
   int mask;
   int data;
   unsigned flags;

   flags = DisableInterrupts();

   if ( PAS_Irq < 8 )
      {
      mask = inp( 0x21 ) & ~( 1 << PAS_Irq );
      outp( 0x21, mask  );
      }
   else
      {
      mask = inp( 0xA1 ) & ~( 1 << ( PAS_Irq - 8 ) );
      outp( 0xA1, mask  );

      mask = inp( 0x21 ) & ~( 1 << 2 );
      outp( 0x21, mask  );
      }

   // Flush any pending interrupts
   PAS_Write( InterruptStatus, PAS_Read( InterruptStatus ) & 0x40 );

   // Enable the interrupt on the PAS
   data = PAS_State->intrctlr;
   data |= SampleBufferInterruptFlag;
   PAS_Write( InterruptControl, data );
   PAS_State->intrctlr = data;

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: PAS_DisableInterrupt

   Disables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

void PAS_DisableInterrupt
   (
   void
   )

   {
   int mask;
   int data;
   unsigned flags;

   flags = DisableInterrupts();

   // Disable the interrupt on the PAS
   data = PAS_State->intrctlr;
   data &= ~( SampleRateInterruptFlag | SampleBufferInterruptFlag );
   PAS_Write( InterruptControl, data );
   PAS_State->intrctlr = data;

   // Restore interrupt mask
   if ( PAS_Irq < 8 )
      {
      mask  = inp( 0x21 ) & ~( 1 << PAS_Irq );
      mask |= PAS_IntController1Mask & ( 1 << PAS_Irq );
      outp( 0x21, mask  );
      }
   else
      {
      mask  = inp( 0x21 ) & ~( 1 << 2 );
      mask |= PAS_IntController1Mask & ( 1 << 2 );
      outp( 0x21, mask  );

      mask  = inp( 0xA1 ) & ~( 1 << ( PAS_Irq - 8 ) );
      mask |= PAS_IntController2Mask & ( 1 << ( PAS_Irq - 8 ) );
      outp( 0xA1, mask  );
      }

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: PAS_ServiceInterrupt

   Handles interrupt generated by sound card at the end of a voice
   transfer.  Calls the user supplied callback function.
---------------------------------------------------------------------*/

void interrupt far PAS_ServiceInterrupt
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

   irqstatus = PAS_Read( InterruptStatus );
   if ( ( irqstatus & SampleBufferInterruptFlag ) == 0 )
      {
      #ifdef USESTACK
      // restore stack
      SetStack( oldStackSelector, oldStackPointer );
      #endif

      _chain_intr( PAS_OldInt );
      }

   // Clear the interrupt
   irqstatus &= ~SampleBufferInterruptFlag;
   PAS_Write( InterruptStatus, irqstatus );

   // send EOI to Interrupt Controller
   if ( PAS_Irq > 7 )
      {
      outp( 0xA0, 0x20 );
      }
   outp( 0x20, 0x20 );


   // Keep track of current buffer
   PAS_CurrentDMABuffer += PAS_TransferLength;
   if ( PAS_CurrentDMABuffer >= PAS_DMABufferEnd )
      {
      PAS_CurrentDMABuffer = PAS_DMABuffer;
      }

   // Call the caller's callback function
   if ( PAS_CallBack != NULL )
      {
      PAS_CallBack();
      }

   #ifdef USESTACK
   // restore stack
   SetStack( oldStackSelector, oldStackPointer );
   #endif
   }


/*---------------------------------------------------------------------
   Function: PAS_Write

   Writes a byte of data to the sound card.
---------------------------------------------------------------------*/

void PAS_Write
   (
   int Register,
   int Data
   )

   {
   int port;

   port = Register ^ PAS_TranslateCode;
   outp( port, Data );
   }


/*---------------------------------------------------------------------
   Function: PAS_Read

   Reads a byte of data from the sound card.
---------------------------------------------------------------------*/

int PAS_Read
   (
   int Register
   )

   {
   int port;
   int data;

   port = Register ^ PAS_TranslateCode;
   data = inp( port );
   return( data );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetSampleRateTimer

   Programs the Sample Rate Timer.
---------------------------------------------------------------------*/

void PAS_SetSampleRateTimer
   (
   void
   )

   {
   int LoByte;
   int HiByte;
   int data;
   unsigned flags;

   flags = DisableInterrupts();

   // Disable the Sample Rate Timer
   data = PAS_State->audiofilt;
   data &= ~SampleRateTimerGateFlag;
   PAS_Write( AudioFilterControl, data );
   PAS_State->audiofilt = data;

   // Select the Sample Rate Timer
   data = SelectSampleRateTimer;
   PAS_Write( LocalTimerControl, data );
   PAS_State->tmrctlr = data;

   LoByte = lobyte( PAS_TimeInterval );
   HiByte = hibyte( PAS_TimeInterval );

   // Program the Sample Rate Timer
   PAS_Write( SampleRateTimer, LoByte );
   PAS_Write( SampleRateTimer, HiByte );
   PAS_State->samplerate = PAS_TimeInterval;

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetSampleBufferCount

   Programs the Sample Buffer Count.
---------------------------------------------------------------------*/

void PAS_SetSampleBufferCount
   (
   void
   )

   {
   int LoByte;
   int HiByte;
   int count;
   int data;
   unsigned flags;

   flags = DisableInterrupts();

   // Disable the Sample Buffer Count
   data = PAS_State->audiofilt;
   data &= ~SampleBufferCountGateFlag;
   PAS_Write( AudioFilterControl, data );
   PAS_State->audiofilt = data;

   // Select the Sample Buffer Count
   data = SelectSampleBufferCount;
   PAS_Write( LocalTimerControl, data );
   PAS_State->tmrctlr = data;

   count = PAS_TransferLength;

   // Check if we're using a 16-bit DMA channel
   if ( PAS_DMAChannel > 3 )
      {
      count >>= 1;
      }

   LoByte = lobyte( count );
   HiByte = hibyte( count );

   // Program the Sample Buffer Count
   PAS_Write( SampleBufferCount, LoByte );
   PAS_Write( SampleBufferCount, HiByte );
   PAS_State->samplecnt = count;

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetPlaybackRate

   Sets the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

void PAS_SetPlaybackRate
   (
   unsigned rate
   )

   {
   if ( rate < PAS_MinSamplingRate )
      {
      rate = PAS_MinSamplingRate;
      }

   if ( rate > PAS_MaxSamplingRate )
      {
      rate = PAS_MaxSamplingRate;
      }

   PAS_TimeInterval = ( unsigned )CalcTimeInterval( rate );
   if ( PAS_MixMode & STEREO )
      {
      PAS_TimeInterval /= 2;
      }

   // Keep track of what the actual rate is
   PAS_SampleRate = CalcSamplingRate( PAS_TimeInterval );
   if ( PAS_MixMode & STEREO )
      {
      PAS_SampleRate /= 2;
      }
   }


/*---------------------------------------------------------------------
   Function: PAS_GetPlaybackRate

   Returns the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

unsigned PAS_GetPlaybackRate
   (
   void
   )

   {
   return( PAS_SampleRate );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetMixMode

   Sets the sound card to play samples in mono or stereo.
---------------------------------------------------------------------*/

int PAS_SetMixMode
   (
   int mode
   )

   {
   mode &= PAS_MaxMixMode;

   // Check board revision.  Revision # 0 can't play 16-bit data.
   if ( ( PAS_State->intrctlr & 0xe0 ) == 0 )
      {
      // Force the mode to 8-bit data.
      mode &= ~SIXTEEN_BIT;
      }

   PAS_MixMode = mode;

   PAS_SetPlaybackRate( PAS_SampleRate );

   return( mode );
   }


/*---------------------------------------------------------------------
   Function: PAS_StopPlayback

   Ends the DMA transfer of digitized sound to the sound card.
---------------------------------------------------------------------*/

void PAS_StopPlayback
   (
   void
   )

   {
   int data;

   // Don't allow anymore interrupts
   PAS_DisableInterrupt();

   // Stop the transfer of digital data
   data = PAS_State->crosschannel;
   data &= PAS_PCMStopMask;
   PAS_Write( CrossChannelControl, data );
   PAS_State->crosschannel = data;

   // Turn off 16-bit unsigned data
   data = PAS_Read( SampleSizeConfiguration );
   data &= PAS_SampleSizeMask;
   PAS_Write( SampleSizeConfiguration, data );

   // Disable the DMA channel
   DMA_EndTransfer( PAS_DMAChannel );

   PAS_SoundPlaying = FALSE;

   PAS_DMABuffer = NULL;
   }


/*---------------------------------------------------------------------
   Function: PAS_SetupDMABuffer

   Programs the DMAC for sound transfer.
---------------------------------------------------------------------*/

int PAS_SetupDMABuffer
   (
   char *BufferPtr,
   int   BufferSize,
   int   mode
   )

   {
   int DmaStatus;
   int data;

   // Enable PAS Dma
   data  = PAS_State->crosschannel;
   data |= PAS_DMAEnable;
   PAS_Write( CrossChannelControl, data );
   PAS_State->crosschannel = data;

   DmaStatus = DMA_SetupTransfer( PAS_DMAChannel, BufferPtr, BufferSize, mode );
   if ( DmaStatus == DMA_Error )
      {
      PAS_SetErrorCode( PAS_DmaError );
      return( PAS_Error );
      }

   PAS_DMABuffer          = BufferPtr;
   PAS_CurrentDMABuffer   = BufferPtr;
   PAS_TotalDMABufferSize = BufferSize;
   PAS_DMABufferEnd       = BufferPtr + BufferSize;

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetCurrentPos

   Returns the offset within the current sound being played.
---------------------------------------------------------------------*/

int PAS_GetCurrentPos
   (
   void
   )

   {
   char *CurrentAddr;
   int   offset;

   if ( !PAS_SoundPlaying )
      {
      PAS_SetErrorCode( PAS_NoSoundPlaying );
      return( PAS_Error );
      }

   CurrentAddr = DMA_GetCurrentPos( PAS_DMAChannel );
   if ( CurrentAddr == NULL )
      {
      PAS_SetErrorCode( PAS_DmaError );
      return( PAS_Error );
      }

   offset = ( int )( ( ( unsigned long )CurrentAddr ) -
      ( ( unsigned long )PAS_CurrentDMABuffer ) );

   if ( PAS_MixMode & SIXTEEN_BIT )
      {
      offset >>= 1;
      }

   if ( PAS_MixMode & STEREO )
      {
      offset >>= 1;
      }

   return( offset );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetFilterSetting

   Returns the bit settings for the appropriate filter level.
---------------------------------------------------------------------*/

int PAS_GetFilterSetting
   (
   int rate
   )

   {
   /* CD Quality 17897hz */
   if ( ( unsigned long )rate > ( unsigned long )17897L * 2 )
      {
      /* 00001b 20hz to 17.8khz */
      return( 0x01 );
      }

   /* Cassette Quality 15090hz */
   if ( ( unsigned long )rate > ( unsigned long )15909L * 2 )
      {
      /* 00010b 20hz to 15.9khz */
      return( 0x02 );
      }

   /* FM Radio Quality 11931hz */
   if ( ( unsigned long )rate > ( unsigned long )11931L * 2 )
      {
      /* 01001b 20hz to 11.9khz */
      return( 0x09 );
      }

   /* AM Radio Quality  8948hz */
   if ( ( unsigned long )rate > ( unsigned long )8948L * 2 )
      {
      /* 10001b 20hz to 8.9khz */
      return( 0x11 );
      }

   /* Telphone Quality  5965hz */
   if ( ( unsigned long )rate > ( unsigned long )5965L * 2 )
      {
      /* 00100b 20hz to 5.9khz */
      return( 0x19 );
      }

   /* Male voice quality 2982hz */
   /* 111001b 20hz to 2.9khz */
   return( 0x04 );
   }


/*---------------------------------------------------------------------
   Function: PAS_BeginTransfer

   Starts playback of digitized sound on the sound card.
---------------------------------------------------------------------*/

void PAS_BeginTransfer
   (
   int mode
   )

   {
   int data;

   PAS_SetSampleRateTimer();

   PAS_SetSampleBufferCount();

   PAS_EnableInterrupt();

   // Get sample size configuration
   data = PAS_Read( SampleSizeConfiguration );

   // Check board revision.  Revision # 0 can't play 16-bit data.
   if ( PAS_State->intrctlr & 0xe0 )
      {
      data &= PAS_SampleSizeMask;

      // set sample size bit
      if ( PAS_MixMode & SIXTEEN_BIT )
         {
         data |= PAS_16BitSampleFlag;
         }
      }

   // set oversampling rate
   data &= PAS_OverSamplingMask;
   data |= PAS_4xOverSampling;

   // Set sample size configuration
   PAS_Write( SampleSizeConfiguration, data );

   // Get Cross channel setting
   data  = PAS_State->crosschannel;
   data &= PAS_ChannelConnectMask;
   if ( mode == RECORD )
      {
      data |= PAS_PCMStartADC;
      }
   else
      {
      data |= PAS_PCMStartDAC;
      }

   // set stereo mode bit
   if ( !( PAS_MixMode & STEREO ) )
      {
      data |= PAS_StereoFlag;
      }

   PAS_Write( CrossChannelControl, data );
   PAS_State->crosschannel = data;

   // Get the filter appropriate filter setting
   data = PAS_GetFilterSetting( PAS_SampleRate );

   // Enable the Sample Rate Timer and Sample Buffer Count
   data |= SampleRateTimerGateFlag | SampleBufferCountGateFlag;

   if ( mode != RECORD )
      {
      // Enable audio (not Audio Mute)
      data |= PAS_AudioMuteFlag;
      }

   PAS_Write( AudioFilterControl, data );
   PAS_State->audiofilt = data;

   PAS_SoundPlaying = TRUE;
   }


/*---------------------------------------------------------------------
   Function: PAS_BeginBufferedPlayback

   Begins multibuffered playback of digitized sound on the sound card.
---------------------------------------------------------------------*/

int PAS_BeginBufferedPlayback
   (
   char *BufferStart,
   int   BufferSize,
   int   NumDivisions,
   unsigned SampleRate,
   int   MixMode,
   void ( *CallBackFunc )( void )
   )

   {
   int DmaStatus;

   PAS_StopPlayback();

   PAS_SetMixMode( MixMode );
   PAS_SetPlaybackRate( SampleRate );

   PAS_TransferLength = BufferSize / NumDivisions;
   PAS_SetCallBack( CallBackFunc );

   DmaStatus = PAS_SetupDMABuffer( BufferStart, BufferSize, DMA_AutoInitRead );
   if ( DmaStatus == PAS_Error )
      {
      return( PAS_Error );
      }

   PAS_BeginTransfer( PLAYBACK );

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_BeginBufferedRecord

   Begins multibuffered recording of digitized sound on the sound card.
---------------------------------------------------------------------*/

int PAS_BeginBufferedRecord
   (
   char *BufferStart,
   int   BufferSize,
   int   NumDivisions,
   unsigned SampleRate,
   int   MixMode,
   void ( *CallBackFunc )( void )
   )

   {
   int DmaStatus;

   PAS_StopPlayback();

   PAS_SetMixMode( MixMode );
   PAS_SetPlaybackRate( SampleRate );

   PAS_TransferLength = BufferSize / NumDivisions;
   PAS_SetCallBack( CallBackFunc );

   DmaStatus = PAS_SetupDMABuffer( BufferStart, BufferSize, DMA_AutoInitWrite );
   if ( DmaStatus == PAS_Error )
      {
      return( PAS_Error );
      }

   PAS_BeginTransfer( RECORD );

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_CallInt

   Calls interrupt 2fh.
---------------------------------------------------------------------*/

int PAS_CallInt( int ebx, int ecx, int edx );
#pragma aux PAS_CallInt = \
   "int 2fh",         \
   parm [ ebx ] [ ecx ] [ edx ] modify exact [ eax ebx ecx edx esi edi ] value [ ebx ];


/*---------------------------------------------------------------------
   Function: PAS_CallMVFunction

   Performs a call to a real mode function.
---------------------------------------------------------------------*/

int PAS_CallMVFunction
   (
   unsigned long function,
   int ebx,
   int ecx,
   int edx
   )

   {
   dpmi_regs callregs;
   int       status;

   callregs.EBX = ebx;
   callregs.ECX = ecx;
   callregs.EDX = edx;

   callregs.SS  = 0;
   callregs.SP  = 0;

   callregs.DS  = 0;
   callregs.ES  = 0;
   callregs.FS  = 0;
   callregs.GS  = 0;

   callregs.IP = function;
   callregs.CS = function >> 16;

   status = DPMI_CallRealModeFunction( &callregs );
   if ( status != DPMI_Ok )
      {
      return( PAS_Error );
      }

   return( callregs.EBX & 0xff );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetPCMVolume

   Sets the volume of digitized sound playback.
---------------------------------------------------------------------*/

int PAS_SetPCMVolume
   (
   int volume
   )

   {
   int status;

   volume = max( 0, volume );
   volume = min( volume, 255 );

   volume *= 100;
   volume /= 255;

   status = PAS_CallMVFunction( PAS_Func->SetMixer, volume,
      OUTPUTMIXER, L_PCM );
   if ( status == PAS_Error )
      {
      return( status );
      }

   status = PAS_CallMVFunction( PAS_Func->SetMixer, volume,
      OUTPUTMIXER, R_PCM );
   if ( status == PAS_Error )
      {
      return( status );
      }

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetPCMVolume

   Returns the current volume of digitized sound playback.
---------------------------------------------------------------------*/

int PAS_GetPCMVolume
   (
   void
   )

   {
   int leftvolume;
   int rightvolume;
   int totalvolume;

   if ( PAS_Func == NULL )
      {
      return( PAS_Error );
      }

   leftvolume = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, L_PCM );
   rightvolume = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, R_PCM );

   if ( ( leftvolume == PAS_Error ) || ( rightvolume == PAS_Error ) )
      {
      return( PAS_Error );
      }

   leftvolume  &= 0xff;
   rightvolume &= 0xff;

   totalvolume = ( rightvolume + leftvolume ) / 2;
   totalvolume *= 255;
   totalvolume /= 100;
   return( totalvolume );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetFMVolume

   Sets the volume of FM sound playback.
---------------------------------------------------------------------*/

void PAS_SetFMVolume
   (
   int volume
   )

   {
   volume = max( 0, volume );
   volume = min( volume, 255 );

   volume *= 100;
   volume /= 255;
   if ( PAS_Func )
      {
      PAS_CallMVFunction( PAS_Func->SetMixer, volume, OUTPUTMIXER, L_FM );
      PAS_CallMVFunction( PAS_Func->SetMixer, volume, OUTPUTMIXER, R_FM );
      }
   }


/*---------------------------------------------------------------------
   Function: PAS_GetFMVolume

   Returns the current volume of FM sound playback.
---------------------------------------------------------------------*/

int PAS_GetFMVolume
   (
   void
   )

   {
   int leftvolume;
   int rightvolume;
   int totalvolume;

   if ( PAS_Func == NULL )
      {
      return( 255 );
      }

   leftvolume = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, L_FM ) & 0xff;
   rightvolume = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, R_FM ) & 0xff;

   totalvolume  = ( rightvolume + leftvolume ) / 2;
   totalvolume *= 255;
   totalvolume /= 100;
   totalvolume  = min( 255, totalvolume );

   return( totalvolume );
   }


/*---------------------------------------------------------------------
   Function: PAS_GetCardInfo

   Returns the maximum number of bits that can represent a sample
   (8 or 16) and the number of channels (1 for mono, 2 for stereo).
---------------------------------------------------------------------*/

int PAS_GetCardInfo
   (
   int *MaxSampleBits,
   int *MaxChannels
   )

   {
   int status;

   if ( PAS_State == NULL )
      {
      status = PAS_CheckForDriver();
      if ( status != PAS_Ok )
         {
         return( status );
         }

      PAS_State = PAS_GetStateTable();
      if ( PAS_State == NULL )
         {
         return( PAS_Error );
         }
      }

   *MaxChannels = 2;

   // Check board revision.  Revision # 0 can't play 16-bit data.
   if ( ( PAS_State->intrctlr & 0xe0 ) == 0 )
      {
      *MaxSampleBits = 8;
      }
   else
      {
      *MaxSampleBits = 16;
      }

   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_SetCallBack

   Specifies the user function to call at the end of a sound transfer.
---------------------------------------------------------------------*/

void PAS_SetCallBack
   (
   void ( *func )( void )
   )

   {
   PAS_CallBack = func;
   }


/*---------------------------------------------------------------------
   Function: PAS_FindCard

   Auto-detects the port the Pro AudioSpectrum is set for.
---------------------------------------------------------------------*/

int PAS_FindCard
   (
   void
   )

   {
   int status;

   status = PAS_TestAddress( DEFAULT_BASE );
   if ( status == 0 )
      {
      PAS_TranslateCode = DEFAULT_BASE;
      return( PAS_Ok );
      }

   status = PAS_TestAddress( ALT_BASE_1 );
   if ( status == 0 )
      {
      PAS_TranslateCode = ALT_BASE_1;
      return( PAS_Ok );
      }

   status = PAS_TestAddress( ALT_BASE_2 );
   if ( status == 0 )
      {
      PAS_TranslateCode = ALT_BASE_2;
      return( PAS_Ok );
      }

   status = PAS_TestAddress( ALT_BASE_3 );
   if ( status == 0 )
      {
      PAS_TranslateCode = ALT_BASE_3;
      return( PAS_Ok );
      }

   PAS_SetErrorCode( PAS_CardNotFound );
   return( PAS_Error );
   }


/*---------------------------------------------------------------------
   Function: PAS_SaveMusicVolume

   Saves the user's FM mixer settings.
---------------------------------------------------------------------*/

int PAS_SaveMusicVolume
   (
   void
   )

   {
   int status;
   int data;

   if ( !PAS_Installed )
      {
      status = PAS_CheckForDriver();
      if ( status != PAS_Ok )
         {
         return( status );
         }

      PAS_State = PAS_GetStateTable();
      if ( PAS_State == NULL )
         {
         return( PAS_Error );
         }

      PAS_Func = PAS_GetFunctionTable();
      if ( PAS_Func == NULL )
         {
         return( PAS_Error );
         }

      status = PAS_GetCardSettings();
      if ( status != PAS_Ok )
         {
         return( status );
         }

      status = PAS_FindCard();
      if ( status != PAS_Ok )
         {
         return( status );
         }

      // Enable PAS Sound
      data  = PAS_State->audiofilt;
      data |= PAS_AudioMuteFlag;

      PAS_Write( AudioFilterControl, data );
      PAS_State->audiofilt = data;
      }

   status = PAS_CallMVFunction( PAS_Func->GetMixer, 0, OUTPUTMIXER, L_FM );
   if ( status != PAS_Error )
      {
      PAS_OriginalFMLeftVolume  = PAS_CallMVFunction( PAS_Func->GetMixer,
         0, OUTPUTMIXER, L_FM ) & 0xff;

      PAS_OriginalFMRightVolume = PAS_CallMVFunction( PAS_Func->GetMixer,
         0, OUTPUTMIXER, R_FM ) & 0xff;

      return( PAS_Ok );
      }

   return( PAS_Warning );
   }


/*---------------------------------------------------------------------
   Function: PAS_RestoreMusicVolume

   Restores the user's FM mixer settings.
---------------------------------------------------------------------*/

void PAS_RestoreMusicVolume
   (
   void
   )

   {
   if ( PAS_Func )
      {
      PAS_CallMVFunction( PAS_Func->SetMixer, PAS_OriginalFMLeftVolume,
         OUTPUTMIXER, L_FM );
      PAS_CallMVFunction( PAS_Func->SetMixer, PAS_OriginalFMRightVolume,
         OUTPUTMIXER, R_FM );
      }
   }


/*---------------------------------------------------------------------
   Function: PAS_SaveState

   Saves the original state of the PAS prior to use.
---------------------------------------------------------------------*/

void PAS_SaveState
   (
   void
   )

   {
   PAS_OriginalState.intrctlr     = PAS_State->intrctlr;
   PAS_OriginalState.audiofilt    = PAS_State->audiofilt;
   PAS_OriginalState.tmrctlr      = PAS_State->tmrctlr;
   PAS_OriginalState.samplerate   = PAS_State->samplerate;
   PAS_OriginalState.samplecnt    = PAS_State->samplecnt;
   PAS_OriginalState.crosschannel = PAS_State->crosschannel;
   PAS_SampleSizeConfig = PAS_Read( SampleSizeConfiguration );
   }


/*---------------------------------------------------------------------
   Function: PAS_RestoreState

   Restores the original state of the PAS after use.
---------------------------------------------------------------------*/

void PAS_RestoreState
   (
   void
   )

   {
   int LoByte;
   int HiByte;

   // Select the Sample Rate Timer
   PAS_Write( LocalTimerControl, SelectSampleRateTimer );
   PAS_State->tmrctlr = SelectSampleRateTimer;

   PAS_Write( SampleRateTimer, PAS_OriginalState.samplerate );
   PAS_State->samplerate = PAS_OriginalState.samplerate;

   // Select the Sample Buffer Count
   PAS_Write( LocalTimerControl, SelectSampleBufferCount );
   PAS_State->tmrctlr = SelectSampleBufferCount;

   LoByte = lobyte( PAS_OriginalState.samplecnt );
   HiByte = hibyte( PAS_OriginalState.samplecnt );
   PAS_Write( SampleRateTimer, LoByte );
   PAS_Write( SampleRateTimer, HiByte );
   PAS_State->samplecnt = PAS_OriginalState.samplecnt;

   PAS_Write( CrossChannelControl, PAS_OriginalState.crosschannel );
   PAS_State->crosschannel = PAS_OriginalState.crosschannel;

   PAS_Write( SampleSizeConfiguration, PAS_SampleSizeConfig );

   PAS_Write( InterruptControl, PAS_OriginalState.intrctlr );
   PAS_State->intrctlr = PAS_OriginalState.intrctlr;

   PAS_Write( AudioFilterControl, PAS_OriginalState.audiofilt );
   PAS_State->audiofilt = PAS_OriginalState.audiofilt;

   PAS_Write( LocalTimerControl, PAS_OriginalState.tmrctlr );
   PAS_State->tmrctlr = PAS_OriginalState.tmrctlr;
   }


/*---------------------------------------------------------------------
   Function: PAS_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void PAS_LockEnd
   (
   void
   )

   {
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
   Function: PAS_Init

   Initializes the sound card and prepares the module to play
   digitized sounds.
---------------------------------------------------------------------*/

int PAS_Init
   (
   void
   )

   {
   int Interrupt;
   int status;
   int data;

   if ( PAS_Installed )
      {
      return( PAS_Ok );
      }

   PAS_IntController1Mask = inp( 0x21 );
   PAS_IntController2Mask = inp( 0xA1 );

   status = PAS_CheckForDriver();
   if ( status != PAS_Ok )
      {
      return( status );
      }

   PAS_State = PAS_GetStateTable();
   if ( PAS_State == NULL )
      {
      return( PAS_Error );
      }

   PAS_Func = PAS_GetFunctionTable();
   if ( PAS_Func == NULL )
      {
      return( PAS_Error );
      }

   status = PAS_GetCardSettings();
   if ( status != PAS_Ok )
      {
      return( status );
      }

   status = PAS_FindCard();
   if ( status != PAS_Ok )
      {
      return( status );
      }

   PAS_SaveState();

   PAS_OriginalPCMLeftVolume  = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, L_PCM ) & 0xff;
   PAS_OriginalPCMRightVolume = PAS_CallMVFunction( PAS_Func->GetMixer, 0,
      OUTPUTMIXER, R_PCM ) & 0xff;

   PAS_SoundPlaying = FALSE;

   PAS_SetCallBack( NULL );

   PAS_DMABuffer = NULL;

   status = PAS_LockMemory();
   if ( status != PAS_Ok )
      {
      PAS_UnlockMemory();
      return( status );
      }

   StackSelector = allocateTimerStack( kStackSize );
   if ( StackSelector == NULL )
      {
      PAS_UnlockMemory();
      PAS_SetErrorCode( PAS_OutOfMemory );
      return( PAS_Error );
      }

   // Leave a little room at top of stack just for the hell of it...
   StackPointer = kStackSize - sizeof( long );

   // Install our interrupt handler
   Interrupt = PAS_Interrupts[ PAS_Irq ];
   PAS_OldInt = _dos_getvect( Interrupt );
   if ( PAS_Irq < 8 )
      {
      _dos_setvect( Interrupt, PAS_ServiceInterrupt );
      }
   else
      {
      status = IRQ_SetVector( Interrupt, PAS_ServiceInterrupt );
      if ( status != IRQ_Ok )
         {
         PAS_UnlockMemory();
         deallocateTimerStack( StackSelector );
         StackSelector = NULL;
         PAS_SetErrorCode( PAS_UnableToSetIrq );
         return( PAS_Error );
         }
      }

   // Enable PAS Sound
   data  = PAS_State->audiofilt;
   data |= PAS_AudioMuteFlag;

   PAS_Write( AudioFilterControl, data );
   PAS_State->audiofilt = data;

   PAS_SetPlaybackRate( PAS_DefaultSampleRate );
   PAS_SetMixMode( PAS_DefaultMixMode );

   PAS_Installed = TRUE;

   PAS_SetErrorCode( PAS_Ok );
   return( PAS_Ok );
   }


/*---------------------------------------------------------------------
   Function: PAS_Shutdown

   Ends transfer of sound data to the sound card and restores the
   system resources used by the card.
---------------------------------------------------------------------*/

void PAS_Shutdown
   (
   void
   )

   {
   int Interrupt;

   if ( PAS_Installed )
      {
      // Halt the DMA transfer
      PAS_StopPlayback();

      // Restore the original interrupt
      Interrupt = PAS_Interrupts[ PAS_Irq ];
      if ( PAS_Irq >= 8 )
         {
         IRQ_RestoreVector( Interrupt );
         }
      _dos_setvect( Interrupt, PAS_OldInt );

      PAS_SoundPlaying = FALSE;

      PAS_DMABuffer = NULL;

      PAS_SetCallBack( NULL );

      PAS_CallMVFunction( PAS_Func->SetMixer, PAS_OriginalPCMLeftVolume,
         OUTPUTMIXER, L_PCM );
      PAS_CallMVFunction( PAS_Func->SetMixer, PAS_OriginalPCMRightVolume,
         OUTPUTMIXER, R_PCM );

// DEBUG
//      PAS_RestoreState();

      PAS_UnlockMemory();

      deallocateTimerStack( StackSelector );
      StackSelector = NULL;

      PAS_Installed = FALSE;
      }
   }


/*---------------------------------------------------------------------
   Function: PAS_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

void PAS_UnlockMemory
   (
   void
   )

   {
   DPMI_UnlockMemoryRegion( PAS_LockStart, PAS_LockEnd );
   DPMI_Unlock( PAS_Interrupts );
   DPMI_Unlock( PAS_OldInt );
   DPMI_Unlock( PAS_IntController1Mask );
   DPMI_Unlock( PAS_IntController2Mask  );
   DPMI_Unlock( PAS_Installed );
   DPMI_Unlock( PAS_TranslateCode );
   DPMI_Unlock( PAS_OriginalPCMLeftVolume );
   DPMI_Unlock( PAS_OriginalPCMRightVolume );
   DPMI_Unlock( PAS_OriginalFMLeftVolume );
   DPMI_Unlock( PAS_OriginalFMRightVolume );
   DPMI_Unlock( PAS_DMAChannel );
   DPMI_Unlock( PAS_Irq );
   DPMI_Unlock( PAS_State );
   DPMI_Unlock( PAS_Func );
   DPMI_Unlock( PAS_OriginalState );
   DPMI_Unlock( PAS_SampleSizeConfig );
   DPMI_Unlock( PAS_DMABuffer );
   DPMI_Unlock( PAS_DMABufferEnd );
   DPMI_Unlock( PAS_CurrentDMABuffer );
   DPMI_Unlock( PAS_TotalDMABufferSize );
   DPMI_Unlock( PAS_TransferLength );
   DPMI_Unlock( PAS_MixMode );
   DPMI_Unlock( PAS_SampleRate );
   DPMI_Unlock( PAS_TimeInterval );
   DPMI_Unlock( PAS_SoundPlaying );
   DPMI_Unlock( PAS_CallBack );
   DPMI_Unlock( PAS_ErrorCode );
   DPMI_Unlock( irqstatus );
   }


/*---------------------------------------------------------------------
   Function: PAS_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int PAS_LockMemory
   (
   void
   )

   {
   int status;

   status  = DPMI_LockMemoryRegion( PAS_LockStart, PAS_LockEnd );
   status |= DPMI_Lock( PAS_Interrupts );
   status |= DPMI_Lock( PAS_OldInt );
   status |= DPMI_Lock( PAS_IntController1Mask );
   status |= DPMI_Lock( PAS_IntController2Mask  );
   status |= DPMI_Lock( PAS_Installed );
   status |= DPMI_Lock( PAS_TranslateCode );
   status |= DPMI_Lock( PAS_OriginalPCMLeftVolume );
   status |= DPMI_Lock( PAS_OriginalPCMRightVolume );
   status |= DPMI_Lock( PAS_OriginalFMLeftVolume );
   status |= DPMI_Lock( PAS_OriginalFMRightVolume );
   status |= DPMI_Lock( PAS_DMAChannel );
   status |= DPMI_Lock( PAS_Irq );
   status |= DPMI_Lock( PAS_State );
   status |= DPMI_Lock( PAS_Func );
   status |= DPMI_Lock( PAS_OriginalState );
   status |= DPMI_Lock( PAS_SampleSizeConfig );
   status |= DPMI_Lock( PAS_DMABuffer );
   status |= DPMI_Lock( PAS_DMABufferEnd );
   status |= DPMI_Lock( PAS_CurrentDMABuffer );
   status |= DPMI_Lock( PAS_TotalDMABufferSize );
   status |= DPMI_Lock( PAS_TransferLength );
   status |= DPMI_Lock( PAS_MixMode );
   status |= DPMI_Lock( PAS_SampleRate );
   status |= DPMI_Lock( PAS_TimeInterval );
   status |= DPMI_Lock( PAS_SoundPlaying );
   status |= DPMI_Lock( PAS_CallBack );
   status |= DPMI_Lock( PAS_ErrorCode );
   status |= DPMI_Lock( irqstatus );

   if ( status != DPMI_Ok )
      {
      PAS_UnlockMemory();
      PAS_SetErrorCode( PAS_DPMI_Error );
      return( PAS_Error );
      }

   return( PAS_Ok );
   }
