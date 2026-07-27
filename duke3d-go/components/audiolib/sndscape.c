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
   module: SNDSCAPE.C

   author: James R. Dose
   date:   October 25, 1994

   Low level routines to support the Ensoniq Soundscape.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "interrup.h"
#include "dpmi.h"
#include "dma.h"
#include "irq.h"
#include "sndscape.h"
#include "_sndscap.h"

const int SOUNDSCAPE_Interrupts[ SOUNDSCAPE_MaxIrq + 1 ]  =
   {
   INVALID, INVALID,     0xa, INVALID,
   INVALID,     0xd, INVALID,     0xf,
   INVALID, INVALID,    0x72, INVALID,
   INVALID, INVALID, INVALID, INVALID
   };

const int SOUNDSCAPE_SampleSize[ SOUNDSCAPE_MaxMixMode + 1 ] =
   {
   MONO_8BIT_SAMPLE_SIZE,  STEREO_8BIT_SAMPLE_SIZE,
   MONO_16BIT_SAMPLE_SIZE, STEREO_16BIT_SAMPLE_SIZE
   };

static void ( __interrupt __far *SOUNDSCAPE_OldInt )( void );

static int SOUNDSCAPE_Installed = FALSE;
static int SOUNDSCAPE_FoundCard = FALSE;

static char   *SOUNDSCAPE_DMABuffer;
static char   *SOUNDSCAPE_DMABufferEnd;
static char   *SOUNDSCAPE_CurrentDMABuffer;
static int     SOUNDSCAPE_TotalDMABufferSize;

static int      SOUNDSCAPE_TransferLength   = 0;
static int      SOUNDSCAPE_MixMode          = SOUNDSCAPE_DefaultMixMode;
static int      SOUNDSCAPE_SamplePacketSize = MONO_16BIT_SAMPLE_SIZE;
static unsigned SOUNDSCAPE_SampleRate       = SOUNDSCAPE_DefaultSampleRate;

volatile int   SOUNDSCAPE_SoundPlaying;

void ( *SOUNDSCAPE_CallBack )( void );

static int  SOUNDSCAPE_IntController1Mask;
static int  SOUNDSCAPE_IntController2Mask;

// some globals for chip type, ports, DMA, IRQs ... and stuff
static struct
   {
   int BasePort;  // base address of the Ensoniq gate-array chip
   int WavePort;  // the AD-1848 base address
   int DMAChan;   // the DMA channel used for PCM
   int WaveIRQ;   // the PCM IRQ
   int MIDIIRQ;   // the MPU-401 IRQ
   int ChipID;    // the Ensoniq chip type
   int SBEmul;    // SoundBlaster emulation flag
   int CDROM;     // CD-ROM flag
   int IRQIndx;   // the Wave IRQ index - for hardware regs
   int OldIRQs;   // Old IRQs flag to support older HW
   } SOUNDSCAPE_Config;

// adequate stack size
#define kStackSize 2048

static unsigned short StackSelector = NULL;
static unsigned long  StackPointer;

static unsigned short oldStackSelector;
static unsigned long  oldStackPointer;

// These declarations are necessary to use the inline assembly pragmas.

extern void GetStack(unsigned short *selptr,unsigned long *stackptr);
extern void SetStack(unsigned short selector,unsigned long stackptr);

// This function will get the current stack selector and pointer and save
// them off.
#pragma aux GetStack =	\
	"mov  [edi],esp"		\
	"mov	ax,ss"	 		\
	"mov  [esi],ax" 		\
	parm [esi] [edi]		\
	modify [eax esi edi];

// This function will set the stack selector and pointer to the specified
// values.
#pragma aux SetStack =	\
	"mov  ss,ax"			\
	"mov  esp,edx"			\
	parm [ax] [edx]		\
	modify [eax edx];

int SOUNDSCAPE_DMAChannel = -1;

int SOUNDSCAPE_ErrorCode = SOUNDSCAPE_Ok;

#define SOUNDSCAPE_SetErrorCode( status ) \
   SOUNDSCAPE_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *SOUNDSCAPE_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case SOUNDSCAPE_Warning :
      case SOUNDSCAPE_Error :
         ErrorString = SOUNDSCAPE_ErrorString( SOUNDSCAPE_ErrorCode );
         break;

      case SOUNDSCAPE_Ok :
         ErrorString = "SoundScape ok.";
         break;

      case SOUNDSCAPE_EnvNotFound :
         ErrorString = "SNDSCAPE environment variable not set.  This is used to locate \n"
                       "SNDSCAPE.INI which is used to describe your sound card setup.";
         break;

      case SOUNDSCAPE_InitFileNotFound :
         ErrorString = "Missing SNDSCAPE.INI file for SoundScape.  This file should be \n"
                       "located in the directory indicated by the SNDSCAPE environment \n"
                       "variable or in 'C:\SNDSCAPE' if SNDSCAPE is not set.";
         break;

      case SOUNDSCAPE_MissingProductInfo :
         ErrorString = "Missing 'Product' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingPortInfo :
         ErrorString = "Missing 'Port' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingDMAInfo :
         ErrorString = "Missing 'DMA' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingIRQInfo :
         ErrorString = "Missing 'IRQ' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingSBIRQInfo :
         ErrorString = "Missing 'SBIRQ' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingSBENABLEInfo :
         ErrorString = "Missing 'SBEnable' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_MissingWavePortInfo :
         ErrorString = "Missing 'WavePort' field in SNDSCAPE.INI file for SoundScape.";
         break;

      case SOUNDSCAPE_HardwareError :
         ErrorString = "Could not detect SoundScape.  Make sure your SNDSCAPE.INI file \n"
                       "contains correct information about your hardware setup.";
         break;

      case SOUNDSCAPE_NoSoundPlaying :
         ErrorString = "No sound playing on SoundScape.";
         break;

      case SOUNDSCAPE_InvalidSBIrq :
         ErrorString = "Invalid SoundScape Irq in SBIRQ field of SNDSCAPE.INI.";
         break;

      case SOUNDSCAPE_UnableToSetIrq :
         ErrorString = "Unable to set SoundScape IRQ.  Try selecting an IRQ of 7 or below.";
         break;

      case SOUNDSCAPE_DmaError :
         ErrorString = DMA_ErrorString( DMA_Error );
         break;

      case SOUNDSCAPE_DPMI_Error :
         ErrorString = "DPMI Error in SoundScape.";
         break;

      case SOUNDSCAPE_OutOfMemory :
         ErrorString = "Out of conventional memory in SoundScape.";
         break;

      default :
         ErrorString = "Unknown SoundScape error code.";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define SOUNDSCAPE_LockStart SOUNDSCAPE_EnableInterrupt


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_EnableInterrupt

   Enables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

static void SOUNDSCAPE_EnableInterrupt
   (
   void
   )

   {
   int mask;

   // Unmask system interrupt
   if ( SOUNDSCAPE_Config.WaveIRQ < 8 )
      {
      mask = inp( 0x21 ) & ~( 1 << SOUNDSCAPE_Config.WaveIRQ );
      outp( 0x21, mask  );
      }
   else
      {
      mask = inp( 0xA1 ) & ~( 1 << ( SOUNDSCAPE_Config.WaveIRQ - 8 ) );
      outp( 0xA1, mask  );

      mask = inp( 0x21 ) & ~( 1 << 2 );
      outp( 0x21, mask  );
      }

   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_DisableInterrupt

   Disables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

static void SOUNDSCAPE_DisableInterrupt
   (
   void
   )

   {
   int mask;

   // Restore interrupt mask
   if ( SOUNDSCAPE_Config.WaveIRQ < 8 )
      {
      mask  = inp( 0x21 ) & ~( 1 << SOUNDSCAPE_Config.WaveIRQ );
      mask |= SOUNDSCAPE_IntController1Mask & ( 1 << SOUNDSCAPE_Config.WaveIRQ );
      outp( 0x21, mask  );
      }
   else
      {
      mask  = inp( 0x21 ) & ~( 1 << 2 );
      mask |= SOUNDSCAPE_IntController1Mask & ( 1 << 2 );
      outp( 0x21, mask  );

      mask  = inp( 0xA1 ) & ~( 1 << ( SOUNDSCAPE_Config.WaveIRQ - 8 ) );
      mask |= SOUNDSCAPE_IntController2Mask & ( 1 << ( SOUNDSCAPE_Config.WaveIRQ - 8 ) );
      outp( 0xA1, mask  );
      }
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_ServiceInterrupt

   Handles interrupt generated by sound card at the end of a voice
   transfer.  Calls the user supplied callback function.
---------------------------------------------------------------------*/

static void __interrupt __far SOUNDSCAPE_ServiceInterrupt
   (
   void
   )

   {
   // save stack
   GetStack( &oldStackSelector, &oldStackPointer );

   // set our stack
   SetStack( StackSelector, StackPointer );

	if ( !( inp( SOUNDSCAPE_Config.WavePort + AD_STATUS ) & 0x01 ) )
      {
      // restore stack
      SetStack( oldStackSelector, oldStackPointer );

      // Wasn't our interrupt.  Call the old one.
      _chain_intr( SOUNDSCAPE_OldInt );
      }

   // clear the AD-1848 interrupt
	outp( SOUNDSCAPE_Config.WavePort + AD_STATUS, 0x00 );

   // Keep track of current buffer
   SOUNDSCAPE_CurrentDMABuffer += SOUNDSCAPE_TransferLength;
   if ( SOUNDSCAPE_CurrentDMABuffer >= SOUNDSCAPE_DMABufferEnd )
      {
      SOUNDSCAPE_CurrentDMABuffer = SOUNDSCAPE_DMABuffer;
      }

   // Call the caller's callback function
   if ( SOUNDSCAPE_CallBack != NULL )
      {
      SOUNDSCAPE_CallBack();
      }

   // restore stack
   SetStack( oldStackSelector, oldStackPointer );

   // send EOI to Interrupt Controller
   if ( SOUNDSCAPE_Config.WaveIRQ > 7 )
      {
      outp( 0xA0, 0x20 );
      }
   outp( 0x20, 0x20 );
   }


/*---------------------------------------------------------------------
   Function: ga_read

   Reads Ensoniq indirect registers.
---------------------------------------------------------------------*/

static int ga_read
   (
   int rnum
   )

   {
   int data;

   outp( SOUNDSCAPE_Config.BasePort + GA_REGADDR, rnum );
   data = inp( SOUNDSCAPE_Config.BasePort + GA_REGDATA );
   return( data );
   }


/*---------------------------------------------------------------------
   Function: ga_write

   Writes to Ensoniq indirect registers.
---------------------------------------------------------------------*/

static void ga_write
   (
   int rnum,
   int value
   )

   {
	outp( SOUNDSCAPE_Config.BasePort + GA_REGADDR, rnum );
	outp( SOUNDSCAPE_Config.BasePort + GA_REGDATA, value );
   }


/*---------------------------------------------------------------------
   Function: ad_read

   Reads the AD-1848 indirect registers.  This function should not be
   used while the AD-1848 mode change is enabled
---------------------------------------------------------------------*/

static int ad_read
   (
   int rnum
   )

   {
   int data;

	outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, rnum );
	data = inp( SOUNDSCAPE_Config.WavePort + AD_REGDATA );
   return( data );
   }


/*---------------------------------------------------------------------
   Function: ad_write

   Writes to the AD-1848 indirect registers.  This function should
   not be used while the AD-1848 mode change is enabled.
---------------------------------------------------------------------*/

static void ad_write
   (
   int rnum,
   int value
   )

   {
   outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, rnum );
   outp( SOUNDSCAPE_Config.WavePort + AD_REGDATA, value );
   }


/*---------------------------------------------------------------------
   Function: tdelay

   Delay function - 250ms - for AD-1848 re-synch and autocalibration.
---------------------------------------------------------------------*/

static void tdelay
   (
   void
   )

   {
	long time;
   unsigned flags;

   flags = DisableInterrupts();
   _enable();
	time = clock() + CLOCKS_PER_SEC/4;
	while(clock() < time)
		;

   RestoreInterrupts( flags );
   }


/*---------------------------------------------------------------------
   Function: pcm_format

   Sets the PCM data format.
---------------------------------------------------------------------*/

static void pcm_format
   (
   void
   )

   {
	int format;

	// build the register value based on format
	format = 0;

   switch( SOUNDSCAPE_SampleRate )
      {
      case 11025:
         format = 0x03;
         break;

      case 22050:
         format = 0x07;
         break;

      case 44100:
         format = 0x0b;
         break;

      default:
         // Set it to 11025 hz
         format = 0x03;
         break;
      }

	// set other format bits and format globals
   if ( SOUNDSCAPE_MixMode & SIXTEEN_BIT )
      {
      format |= 0x40;
      }

   if ( SOUNDSCAPE_MixMode & STEREO )
      {
      format |= 0x10;
      }

	// enable mode change, point to format reg
	outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, 0x40 | AD_FORMAT );

	// write the format
	outp( SOUNDSCAPE_Config.WavePort + AD_REGDATA, format );

	// delay for internal re-synch
	tdelay();

	// exit mode change state
	outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, 0x00 );

	// delay for autocalibration
	tdelay();
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_SetPlaybackRate

   Sets the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

void SOUNDSCAPE_SetPlaybackRate
   (
   unsigned rate
   )

   {
   if ( rate < 20000 )
      {
      rate = 11025;
      }
   else if ( rate < 30000 )
      {
      rate = 22050;
      }
   else
      {
      rate = 44100;
      }

   SOUNDSCAPE_SampleRate = rate;

   // Set the rate
   pcm_format();
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_GetPlaybackRate

   Returns the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

unsigned SOUNDSCAPE_GetPlaybackRate
   (
   void
   )

   {
   return( SOUNDSCAPE_SampleRate );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_SetMixMode

   Sets the sound card to play samples in mono or stereo.
---------------------------------------------------------------------*/

int SOUNDSCAPE_SetMixMode
   (
   int mode
   )

   {
   SOUNDSCAPE_MixMode = mode & SOUNDSCAPE_MaxMixMode;
   SOUNDSCAPE_SamplePacketSize = SOUNDSCAPE_SampleSize[ SOUNDSCAPE_MixMode ];

   // Set the mixmode
   pcm_format();

   return( mode );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_StopPlayback

   Ends the DMA transfer of digitized sound to the sound card.
---------------------------------------------------------------------*/

void SOUNDSCAPE_StopPlayback
   (
   void
   )

   {
   // Don't allow anymore interrupts
   SOUNDSCAPE_DisableInterrupt();

	/* stop the AD-1848 */
	ad_write( AD_CONFIG, 0x00 );

	/* let it finish it's cycles */
	tdelay();

   // Disable the DMA channel
   DMA_EndTransfer( SOUNDSCAPE_Config.DMAChan );

   SOUNDSCAPE_SoundPlaying = FALSE;

   SOUNDSCAPE_DMABuffer = NULL;
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_SetupDMABuffer

   Programs the DMAC for sound transfer.
---------------------------------------------------------------------*/

static int SOUNDSCAPE_SetupDMABuffer
   (
   char *BufferPtr,
   int   BufferSize,
   int   mode
   )

   {
   int DmaStatus;

   DmaStatus = DMA_SetupTransfer( SOUNDSCAPE_Config.DMAChan, BufferPtr, BufferSize, mode );
   if ( DmaStatus == DMA_Error )
      {
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_DmaError );
      return( SOUNDSCAPE_Error );
      }

   SOUNDSCAPE_DMAChannel         = SOUNDSCAPE_Config.DMAChan;
   SOUNDSCAPE_DMABuffer          = BufferPtr;
   SOUNDSCAPE_CurrentDMABuffer   = BufferPtr;
   SOUNDSCAPE_TotalDMABufferSize = BufferSize;
   SOUNDSCAPE_DMABufferEnd       = BufferPtr + BufferSize;

   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_GetCurrentPos

   Returns the offset within the current sound being played.
---------------------------------------------------------------------*/

int SOUNDSCAPE_GetCurrentPos
   (
   void
   )

   {
   char *CurrentAddr;
   int   offset;

   if ( !SOUNDSCAPE_SoundPlaying )
      {
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_NoSoundPlaying );
      return( SOUNDSCAPE_Error );
      }

   CurrentAddr = DMA_GetCurrentPos( SOUNDSCAPE_Config.DMAChan );

   offset = ( int )( ( ( unsigned long )CurrentAddr ) -
      ( ( unsigned long )SOUNDSCAPE_CurrentDMABuffer ) );

   if ( SOUNDSCAPE_MixMode & SIXTEEN_BIT )
      {
      offset >>= 1;
      }

   if ( SOUNDSCAPE_MixMode & STEREO )
      {
      offset >>= 1;
      }

   return( offset );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_BeginPlayback

   Starts playback of digitized sound.
---------------------------------------------------------------------*/

static int SOUNDSCAPE_BeginPlayback
   (
   int length
   )

   {
   int SampleLength;
   int LoByte;
   int HiByte;

   if ( SOUNDSCAPE_MixMode & SIXTEEN_BIT )
      {
      SampleLength = length / 2;
      }
   else
      {
      SampleLength = length;
      }

   if ( SOUNDSCAPE_MixMode & STEREO )
      {
      SampleLength >>= 1;
      }

   SampleLength--;

	// setup the AD-1848 interrupt count
	// set the interrupt count value based on the format.
	// count will decrement every sample period and generate
	// an interrupt when in rolls over. we want this always
	// to be at every 1/2 buffer, regardless of the data format,
	// so the count must be adjusted accordingly.
   HiByte = hibyte( SampleLength );
   LoByte = lobyte( SampleLength );
	ad_write( AD_LCOUNT, LoByte );
	ad_write( AD_UCOUNT, HiByte );

	/* unmask the host DMA controller */
   SOUNDSCAPE_EnableInterrupt();

	/* start the AD-1848 */
	ad_write(AD_CONFIG, 0x01);

   SOUNDSCAPE_SoundPlaying = TRUE;

   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_BeginBufferedPlayback

   Begins multibuffered playback of digitized sound on the sound card.
---------------------------------------------------------------------*/

int SOUNDSCAPE_BeginBufferedPlayback
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

   if ( SOUNDSCAPE_SoundPlaying )
      {
      SOUNDSCAPE_StopPlayback();
      }

   SOUNDSCAPE_SetMixMode( MixMode );

   DmaStatus = SOUNDSCAPE_SetupDMABuffer( BufferStart, BufferSize,
      DMA_AutoInitRead );
   if ( DmaStatus == SOUNDSCAPE_Error )
      {
      return( SOUNDSCAPE_Error );
      }

   SOUNDSCAPE_SetPlaybackRate( SampleRate );

   SOUNDSCAPE_SetCallBack( CallBackFunc );

   SOUNDSCAPE_EnableInterrupt();

   TransferLength = BufferSize / NumDivisions;
   SOUNDSCAPE_TransferLength = TransferLength;

   SOUNDSCAPE_BeginPlayback( TransferLength );

   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_GetCardInfo

   Returns the maximum number of bits that can represent a sample
   (8 or 16) and the number of channels (1 for mono, 2 for stereo).
---------------------------------------------------------------------*/

int SOUNDSCAPE_GetCardInfo
   (
   int *MaxSampleBits,
   int *MaxChannels
   )

   {
   int status;

   status = SOUNDSCAPE_FindCard();
   if ( status == SOUNDSCAPE_Ok )
      {
      *MaxChannels = 2;
      *MaxSampleBits = 16;
      return( SOUNDSCAPE_Ok );
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_SetCallBack

   Specifies the user function to call at the end of a sound transfer.
---------------------------------------------------------------------*/

void SOUNDSCAPE_SetCallBack
   (
   void ( *func )( void )
   )

   {
   SOUNDSCAPE_CallBack = func;
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void SOUNDSCAPE_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

static void SOUNDSCAPE_UnlockMemory
   (
   void
   )

   {
   DPMI_UnlockMemoryRegion( SOUNDSCAPE_LockStart, SOUNDSCAPE_LockEnd );
   DPMI_Unlock( SOUNDSCAPE_Config );
   DPMI_Unlock( SOUNDSCAPE_OldInt );
   DPMI_Unlock( SOUNDSCAPE_Installed );
   DPMI_Unlock( SOUNDSCAPE_DMABuffer );
   DPMI_Unlock( SOUNDSCAPE_DMABufferEnd );
   DPMI_Unlock( SOUNDSCAPE_CurrentDMABuffer );
   DPMI_Unlock( SOUNDSCAPE_TotalDMABufferSize );
   DPMI_Unlock( SOUNDSCAPE_TransferLength );
   DPMI_Unlock( SOUNDSCAPE_MixMode );
   DPMI_Unlock( SOUNDSCAPE_SamplePacketSize );
   DPMI_Unlock( SOUNDSCAPE_SampleRate );
   DPMI_Unlock( SOUNDSCAPE_SoundPlaying );
   DPMI_Unlock( SOUNDSCAPE_CallBack );
   DPMI_Unlock( SOUNDSCAPE_IntController1Mask );
   DPMI_Unlock( SOUNDSCAPE_IntController2Mask );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

static int SOUNDSCAPE_LockMemory
   (
   void
   )

   {
   int status;

   status  = DPMI_LockMemoryRegion( SOUNDSCAPE_LockStart, SOUNDSCAPE_LockEnd );
   status |= DPMI_Lock( SOUNDSCAPE_Config );
   status |= DPMI_Lock( SOUNDSCAPE_OldInt );
   status |= DPMI_Lock( SOUNDSCAPE_Installed );
   status |= DPMI_Lock( SOUNDSCAPE_DMABuffer );
   status |= DPMI_Lock( SOUNDSCAPE_DMABufferEnd );
   status |= DPMI_Lock( SOUNDSCAPE_CurrentDMABuffer );
   status |= DPMI_Lock( SOUNDSCAPE_TotalDMABufferSize );
   status |= DPMI_Lock( SOUNDSCAPE_TransferLength );
   status |= DPMI_Lock( SOUNDSCAPE_MixMode );
   status |= DPMI_Lock( SOUNDSCAPE_SamplePacketSize );
   status |= DPMI_Lock( SOUNDSCAPE_SampleRate );
   status |= DPMI_Lock( SOUNDSCAPE_SoundPlaying );
   status |= DPMI_Lock( SOUNDSCAPE_CallBack );
   status |= DPMI_Lock( SOUNDSCAPE_IntController1Mask );
   status |= DPMI_Lock( SOUNDSCAPE_IntController2Mask );

   if ( status != DPMI_Ok )
      {
      SOUNDSCAPE_UnlockMemory();
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_DPMI_Error );
      return( SOUNDSCAPE_Error );
      }

   return( SOUNDSCAPE_Ok );
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
   Function: parse

   Parses for the right hand string of an .INI file equate.
---------------------------------------------------------------------*/

static int parse
   (
   char *val,
   char *str,
   FILE *p1
   )

   {
	int  i;
	int  j;
   char tmpstr[ 81 ];

	rewind( p1 );

	while( !feof( p1 ) )
      {
      // get a new string
      fgets( tmpstr, 81, p1 );
      if( ( tmpstr[ 0 ] == '[' ) || ( tmpstr[ 0 ] == ';' ) ||
         ( tmpstr[ 0 ] == '\n' ) )
         {
         continue;
         }

		// parse up to the '='
      i = 0;
      while( ( tmpstr[ i ] != '=' ) && ( tmpstr[ i ] != '\n' ) )
         {
         i++;
         }

		if( tmpstr[ i ] != '=' )
         {
         continue;
         }

		tmpstr[ i ] = '\0';

		// see if it's the one we want
		if ( strcmp( tmpstr, str ) )
         {
         continue;
         }

		// copy the right hand value to the destination string
      i++;
		for( j = 0; j < 32; j++ )
         {
         if ( ( tmpstr[ i ] == ' ' ) || ( tmpstr[ i ] == '\t' ) ||
            ( tmpstr[ i ] == ',' ) || ( tmpstr[ i ] == '\n' ) )
            {
            break;
            }

			val[ j ] = tmpstr[ i ];
         i++;
         }
		val[j] = '\0';

      return( TRUE );
      }

   return( FALSE );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_FindCard

   Determines if a SoundScape is present and where it is located.
---------------------------------------------------------------------*/

static int SOUNDSCAPE_FindCard
   (
   void
   )

   {
   int   found;
   int   status;
   int   tmp;
	char *cp;
   char  str[ 33 ];
	FILE *fp;

   if ( SOUNDSCAPE_FoundCard )
      {
      return( SOUNDSCAPE_Ok );
      }

	cp = getenv( "SNDSCAPE" );
   if ( cp == NULL )
      {
      strcpy( str, "C:\\SNDSCAPE" );
      }
   else
      {
      strcpy( str, cp );
      }

   strcat(str, "\\SNDSCAPE.INI");

	fp = fopen( str, "r" );
   if ( fp == NULL )
      {
      if ( cp == NULL )
         {
         SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_EnvNotFound );
         return( SOUNDSCAPE_Error );
         }

      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_InitFileNotFound );
      return( SOUNDSCAPE_Error );
      }

	found = parse( str, "Product", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingProductInfo );
      return( SOUNDSCAPE_Error );
      }

	if( strstr( str, "SoundFX" ) == NULL )
      {
      SOUNDSCAPE_Config.OldIRQs = FALSE;
      }
	else
      {
		SOUNDSCAPE_Config.OldIRQs = TRUE;
      }

	found = parse( str, "Port", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingPortInfo );
      return( SOUNDSCAPE_Error );
      }

	SOUNDSCAPE_Config.BasePort = strtol( str, ( char ** )0, 16);

	found = parse( str, "DMA", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingDMAInfo );
      return( SOUNDSCAPE_Error );
      }

	SOUNDSCAPE_Config.DMAChan = ( int )strtol( str, ( char ** )0, 10 );
   status = DMA_VerifyChannel( SOUNDSCAPE_Config.DMAChan );
   if ( status == DMA_Error )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_DmaError );
      return( SOUNDSCAPE_Error );
      }

	found = parse( str, "IRQ", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingIRQInfo );
      return( SOUNDSCAPE_Error );
      }

	SOUNDSCAPE_Config.MIDIIRQ = ( int )strtol( str, ( char ** )0, 10 );
   if ( SOUNDSCAPE_Config.MIDIIRQ == 2 )
      {
      SOUNDSCAPE_Config.MIDIIRQ = 9;
      }

	found = parse( str, "SBIRQ", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingSBIRQInfo );
      return( SOUNDSCAPE_Error );
      }

	SOUNDSCAPE_Config.WaveIRQ = ( int )strtol( str, ( char ** )0, 10 );
	if ( SOUNDSCAPE_Config.WaveIRQ == 2 )
      {
		SOUNDSCAPE_Config.WaveIRQ = 9;
      }

   if ( !VALID_IRQ( SOUNDSCAPE_Config.WaveIRQ ) )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_InvalidSBIrq );
      return( SOUNDSCAPE_Error );
      }

   if ( SOUNDSCAPE_Interrupts[ SOUNDSCAPE_Config.WaveIRQ ] == INVALID )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_InvalidSBIrq );
      return( SOUNDSCAPE_Error );
      }

	found = parse( str, "SBEnable", fp );
   if ( !found )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingSBENABLEInfo );
      return( SOUNDSCAPE_Error );
      }

	if( !strcmp( str, "false" ) )
      {
      SOUNDSCAPE_Config.SBEmul = FALSE;
      }
	else
      {
      SOUNDSCAPE_Config.SBEmul = TRUE;
      }

	// do a hardware test
	outp( SOUNDSCAPE_Config.BasePort + GA_REGADDR, 0x00f5 );
	tmp = inp( SOUNDSCAPE_Config.BasePort + GA_REGADDR );
	if ( ( tmp & 0x000f ) != 0x0005 )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_HardwareError );
      return( SOUNDSCAPE_Error );
      }

	if( ( tmp & 0x00f0 ) == 0x00f0 )
      {
      fclose( fp );
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_HardwareError );
      return( SOUNDSCAPE_Error );
      }

	// formulate the chip ID
	tmp >>= 4;
	if( tmp == 0 )
      {
      SOUNDSCAPE_Config.ChipID = ODIE;
      }
	else if ( !( tmp & 0x0008 ) )
      {
		SOUNDSCAPE_Config.ChipID = OPUS;
      }
	else
      {
		SOUNDSCAPE_Config.ChipID = MMIC;
      }

	// parse for the AD-1848 address if necessary
	if( SOUNDSCAPE_Config.ChipID == ODIE )
      {
      found = parse( str, "WavePort", fp );
      if ( !found )
         {
         fclose( fp );
         SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_MissingWavePortInfo );
         return( SOUNDSCAPE_Error );
         }

      SOUNDSCAPE_Config.WavePort = strtol( str, ( char ** )0, 16 );
      }
	else
      {
      // otherwise, the base address is fixed
      SOUNDSCAPE_Config.WavePort = SOUNDSCAPE_Config.BasePort + AD_OFFSET;
      }

	// we're done with the file
	fclose( fp );

	// if it's an ODIE board, note CD-ROM decode enable
	if ( SOUNDSCAPE_Config.ChipID == ODIE )
      {
		SOUNDSCAPE_Config.CDROM = ga_read( GA_CDCFG ) & 0x80;
      }

	// build the Wave IRQ index value
	if( !SOUNDSCAPE_Config.OldIRQs )
      {
      switch( SOUNDSCAPE_Config.WaveIRQ )
         {
         case 9 :
            SOUNDSCAPE_Config.IRQIndx = 0;
            break;

         case 5 :
            SOUNDSCAPE_Config.IRQIndx = 1;
            break;

         case 7 :
            SOUNDSCAPE_Config.IRQIndx = 2;
            break;

         default :
            SOUNDSCAPE_Config.IRQIndx = 3;
            break;
         }
      }
   else
      {
      switch( SOUNDSCAPE_Config.WaveIRQ )
         {
         case 9 :
            SOUNDSCAPE_Config.IRQIndx = 0;
            break;

         case 5 :
            SOUNDSCAPE_Config.IRQIndx = 2;
            break;

         case 7 :
            SOUNDSCAPE_Config.IRQIndx = 1;
            break;

         default :
            SOUNDSCAPE_Config.IRQIndx = 3;
            break;
         }
      }

   SOUNDSCAPE_FoundCard = TRUE;
   SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_Ok );
   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_Setup

   Setup the Soundscape card for native mode PCM.
---------------------------------------------------------------------*/

static int SOUNDSCAPE_Setup
   (
   void
   )

   {
   int tmp;
   int Interrupt;
   int status;

	// if necessary, clear any pending SB ints
	if ( SOUNDSCAPE_Config.SBEmul )
      {
      inp( SB_IACK );
      }

   SOUNDSCAPE_DisableInterrupt();

	// make sure the AD-1848 is not running
	if ( ad_read( AD_CONFIG ) & 0x01 )
      {
      SOUNDSCAPE_StopPlayback();
      }

	// if necessary, do some signal re-routing
	if( SOUNDSCAPE_Config.ChipID != MMIC )
      {
      // get the gate-array off of the DMA channel
		ga_write( GA_DMACHB, 0x20 );

      if ( !SOUNDSCAPE_Config.OldIRQs )
         {
         switch( SOUNDSCAPE_Config.MIDIIRQ )
            {
            case 5 :
               tmp = 1;
               break;

            case 7 :
               tmp = 2;
               break;

            case 9 :
               tmp = 0;
               break;

            default :
               tmp = 3;
               break;
            }
         }
      else
         {
         switch( SOUNDSCAPE_Config.MIDIIRQ )
            {
            case 5 :
               tmp = 2;
               break;

            case 7 :
               tmp = 1;
               break;

            case 9 :
               tmp = 0;
               break;

            default :
               tmp = 3;
               break;
            }
         }

      // set HostIRQ to MIDIIRQ for now
      ga_write( GA_INTCFG, 0xf0 | ( tmp << 2 ) | tmp );

      // now, route the AD-1848 stuff ...
		if ( SOUNDSCAPE_Config.ChipID == OPUS )
         {
         // set the AD-1848 chip decode
         ga_write( GA_HMCTL, ( ga_read( GA_HMCTL ) & 0xcf ) | 0x10 );
         }
      // setup the DMA polarity
		ga_write( GA_DMACFG, 0x50 );

		// init the CD-ROM (AD-1848) config register
		ga_write( GA_CDCFG, 0x89 | ( SOUNDSCAPE_Config.DMAChan << 4 ) | ( SOUNDSCAPE_Config.IRQIndx << 1 ) );

      // enable mode change, point to config reg
		outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, 0x40 | AD_CONFIG );

      // set interf cnfg reg for DMA mode, single chan, autocal on
		outp( SOUNDSCAPE_Config.WavePort + AD_REGDATA, 0x0c );

      // exit mode change state
		outp( SOUNDSCAPE_Config.WavePort + AD_REGADDR, 0x00 );

      // delay for autocalibration
      tdelay();
      }

   // Install our interrupt handler
   Interrupt = SOUNDSCAPE_Interrupts[ SOUNDSCAPE_Config.WaveIRQ ];
   SOUNDSCAPE_OldInt = _dos_getvect( Interrupt );
   if ( SOUNDSCAPE_Config.WaveIRQ < 8 )
      {
      _dos_setvect( Interrupt, SOUNDSCAPE_ServiceInterrupt );
      }
   else
      {
      status = IRQ_SetVector( Interrupt, SOUNDSCAPE_ServiceInterrupt );
      if ( status != IRQ_Ok )
         {
         SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_UnableToSetIrq );
         return( SOUNDSCAPE_Error );
         }
      }

	// max left and right volumes
	ad_write( AD_LEFTOUT, 0 );
	ad_write( AD_RIGHTOUT, 0 );

	// clear any pending interrupt condition
	outp( SOUNDSCAPE_Config.WavePort + AD_STATUS, 0x00 );

	// enable the interrupt pin
	ad_write( AD_PINCTRL, ad_read( AD_PINCTRL ) | 0x02 );

   SOUNDSCAPE_EnableInterrupt();

   SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_Ok );
   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_GetMIDIPort

   Gets the address of the SoundScape MIDI port.
---------------------------------------------------------------------*/

int SOUNDSCAPE_GetMIDIPort
   (
   void
   )

   {
   int status;

   status = SOUNDSCAPE_FindCard();
   if ( status != SOUNDSCAPE_Ok )
      {
      return( status );
      }

   return( SOUNDSCAPE_Config.BasePort );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_Init

   Initializes the sound card and prepares the module to play
   digitized sounds.
---------------------------------------------------------------------*/

int SOUNDSCAPE_Init
   (
   void
   )

   {
   int status;

   if ( SOUNDSCAPE_Installed )
      {
      SOUNDSCAPE_Shutdown();
      }

   // Save the interrupt masks
   SOUNDSCAPE_IntController1Mask = inp( 0x21 );
   SOUNDSCAPE_IntController2Mask = inp( 0xA1 );

   SOUNDSCAPE_SoundPlaying = FALSE;
   SOUNDSCAPE_SetCallBack( NULL );
   SOUNDSCAPE_DMABuffer = NULL;

   status = SOUNDSCAPE_FindCard();
   if ( status != SOUNDSCAPE_Ok )
      {
      return( status );
      }

   status = SOUNDSCAPE_LockMemory();
   if ( status != SOUNDSCAPE_Ok )
      {
      SOUNDSCAPE_UnlockMemory();
      return( status );
      }

   StackSelector = allocateTimerStack( kStackSize );
   if ( StackSelector == NULL )
      {
      SOUNDSCAPE_UnlockMemory();
      SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_OutOfMemory );
      return( SOUNDSCAPE_Error );
      }

   // Leave a little room at top of stack just for the hell of it...
   StackPointer = kStackSize - sizeof( long );

   SOUNDSCAPE_Installed = TRUE;

   status = SOUNDSCAPE_Setup();
   if ( status != SOUNDSCAPE_Ok )
      {
      SOUNDSCAPE_Shutdown();
      return( status );
      }

//	printf("Testing DMA and IRQ ...\n");
//	if( test_dma_irq() )
//      {
//      printf("\t\007Hardware Not Responding\n\n");
//      close_soundscape();
//      return( SOUNDSCAPE_Error );
//      }

   SOUNDSCAPE_SetPlaybackRate( SOUNDSCAPE_DefaultSampleRate );
   SOUNDSCAPE_SetMixMode( SOUNDSCAPE_DefaultMixMode );

   SOUNDSCAPE_SetErrorCode( SOUNDSCAPE_Ok );
   return( SOUNDSCAPE_Ok );
   }


/*---------------------------------------------------------------------
   Function: SOUNDSCAPE_Shutdown

   Ends transfer of sound data to the sound card and restores the
   system resources used by the card.
---------------------------------------------------------------------*/

void SOUNDSCAPE_Shutdown
   (
   void
   )

   {
   int Interrupt;

   // Halt the DMA transfer
   SOUNDSCAPE_StopPlayback();

	// disable the AD-1848 interrupt pin
	ad_write( AD_PINCTRL, ad_read( AD_PINCTRL ) & 0xfd );

	// if necessary, do some signal re-routing
	if ( SOUNDSCAPE_Config.ChipID != MMIC )
      {
		// re-init the CD-ROM (AD-1848) config register as needed.
		// this will disable the AD-1848 interface.
		if ( SOUNDSCAPE_Config.ChipID == ODIE )
         {
         ga_write( GA_CDCFG, SOUNDSCAPE_Config.CDROM );
         }
		else
         {
         ga_write( GA_CDCFG, ga_read( GA_CDCFG ) & 0x7f);
         }

		// if necessary, reset the SoundBlaster IRQ
		if ( SOUNDSCAPE_Config.SBEmul )
         {
         ga_write( GA_INTCFG, ( ga_read( GA_INTCFG ) & 0xf3 ) |
            ( SOUNDSCAPE_Config.IRQIndx << 2 ) );
         }

      // re-assign the gate-array DMA channel
		ga_write( GA_DMACHB, 0x80 | ( SOUNDSCAPE_Config.DMAChan << 4 ) );
      }

   // Restore the original interrupt
   Interrupt = SOUNDSCAPE_Interrupts[ SOUNDSCAPE_Config.WaveIRQ ];
   if ( SOUNDSCAPE_Config.WaveIRQ >= 8 )
      {
      IRQ_RestoreVector( Interrupt );
      }
   _dos_setvect( Interrupt, SOUNDSCAPE_OldInt );

   SOUNDSCAPE_SoundPlaying = FALSE;

   SOUNDSCAPE_DMABuffer = NULL;

   SOUNDSCAPE_SetCallBack( NULL );

   SOUNDSCAPE_UnlockMemory();

   if ( StackSelector != NULL )
      {
      deallocateTimerStack( StackSelector );
      StackSelector = NULL;
      }

   SOUNDSCAPE_Installed = FALSE;
   }
