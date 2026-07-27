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
   module: AWE32.C

   author: James R. Dose
   date:   August 23, 1994

   Cover functions for calling the AWE32 low-level library.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <string.h>
#include "dpmi.h"
#include "blaster.h"
#include "ctaweapi.h"
#include "awe32.h"

#define _inp    inp
#define _outp   outp

/*  DSP defines  */
#define MPU_ACK_OK          0xfe
#define MPU_RESET_CMD       0xff
#define MPU_ENTER_UART      0x3f

static WORD wSBCBaseAddx;            /* Sound Blaster base address */
static WORD wEMUBaseAddx;            /* EMU8000 subsystem base address */
static WORD wMpuBaseAddx;            /* MPU401 base address */

static unsigned short NoteFlags[ 128 ];

/*  macros  */
#define SBCPort( x )  ( ( x ) + wSBCBaseAddx )
#define MPUPort( x )  ( ( x ) + wMpuBaseAddx )

static SOUND_PACKET spSound =
   {
   0
   };

static LONG lBankSizes[ MAXBANKS ] =
   {
   0
   };

unsigned SetES( void );
#pragma aux SetES = \
        "xor eax, eax" \
        "mov ax, es" \
        "mov bx, ds" \
        "mov es, bx" \
        modify [ eax ebx ];

void RestoreES( unsigned num );
#pragma aux RestoreES = \
        "mov  es, ax" \
        parm [ eax ];

int AWE32_ErrorCode = AWE32_Ok;

#define AWE32_SetErrorCode( status ) \
   AWE32_ErrorCode = ( status );


/*---------------------------------------------------------------------
   Function: AWE32_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *AWE32_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case AWE32_Warning :
      case AWE32_Error :
         ErrorString = AWE32_ErrorString( AWE32_ErrorCode );
         break;

      case AWE32_Ok :
         ErrorString = "AWE32 ok.";
         break;

      case AWE32_SoundBlasterError :
         ErrorString = BLASTER_ErrorString( BLASTER_Error );
         break;

      case AWE32_NotDetected :
         ErrorString = "Could not detect AWE32.";
         break;

      case AWE32_UnableToInitialize :
         ErrorString = "Unable to initialize AWE32.";

      case AWE32_MPU401Error :
         ErrorString = "MPU-401 initialization failed in AWE32.";
         break;

      case AWE32_DPMI_Error :
         ErrorString = "DPMI Error in AWE32.";
         break;

      default :
         ErrorString = "Unknown AWE32 error code.";
         break;
      }

   return( ErrorString );
   }


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define AWE32_LockStart AWE32_NoteOff


void AWE32_NoteOff
   (
   int channel,
   int key,
   int velocity
   )

   {
   unsigned temp;

   temp = SetES();
   awe32NoteOff( channel, key, velocity );
   RestoreES( temp );
   NoteFlags[ key ] ^= ( 1 << channel );
   }

void AWE32_NoteOn
   (
   int channel,
   int key,
   int velocity
   )

   {
   unsigned temp;

   temp = SetES();
   awe32NoteOn( channel, key, velocity );
   RestoreES( temp );
   NoteFlags[ key ] |= ( 1 << channel );
   }

void AWE32_PolyAftertouch
   (
   int channel,
   int key,
   int pressure
   )

   {
   unsigned temp;

   temp = SetES();
   awe32PolyKeyPressure( channel, key, pressure );
   RestoreES( temp );
   }

void AWE32_ChannelAftertouch
   (
   int channel,
   int pressure
   )

   {
   unsigned temp;

   temp = SetES();
   awe32ChannelPressure( channel, pressure );
   RestoreES( temp );
   }

void AWE32_ControlChange
   (
   int channel,
   int number,
   int value
   )

   {
   unsigned temp;
   int i;
   unsigned channelmask;

   temp = SetES();

   if ( number == 0x7b )
      {
      channelmask = 1 << channel;
      for( i = 0; i < 128; i++ )
         {
         if ( NoteFlags[ i ] & channelmask )
            {
            awe32NoteOff( channel, i, 0 );
            NoteFlags[ i ] ^= channelmask;
            }
         }
      }
   else
      {
      awe32Controller( channel, number, value );
      }
   RestoreES( temp );
   }

void AWE32_ProgramChange
   (
   int channel,
   int program
   )

   {
   unsigned temp;

   temp = SetES();
   awe32ProgramChange( channel, program );
   RestoreES( temp );
   }

void AWE32_PitchBend
   (
   int channel,
   int lsb,
   int msb
   )

   {
   unsigned temp;

   temp = SetES();
   awe32PitchBend( channel, lsb, msb );
   RestoreES( temp );
   }


/*---------------------------------------------------------------------
   Function: AWE32_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void AWE32_LockEnd
   (
   void
   )

   {
   }


/*
static int InitMPU
   (
   void
   )

   {
   volatile DWORD dwCount;

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x40) --dwCount;
   _outp(MPUPort(1), MPU_RESET_CMD);

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x80) --dwCount;
   _inp(MPUPort(0));

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x40) --dwCount;
   _outp(MPUPort(1), MPU_RESET_CMD);

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x80) --dwCount;
   _inp(MPUPort(0));

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x40) --dwCount;
   _outp(MPUPort(1), MPU_ENTER_UART);

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x80) --dwCount;
   if (!dwCount) return TRUE;
   if (_inp(MPUPort(0)) != MPU_ACK_OK) return TRUE;

   // mask MPU-401 interrupt
   _outp(SBCPort(0x4), 0x83);
   _outp(SBCPort(0x5), _inp(SBCPort(0x5)) & ~0x04);

   return FALSE;
   }
*/

/*ีออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ*/
/*ณ ShutdownMPU                                                    ณ*/
/*ณ Cleans up Sound Blaster to normal state.                               ณ*/
/*ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

static void ShutdownMPU
   (
   void
   )

   {
   volatile DWORD dwCount;

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x40) --dwCount;
   _outp(MPUPort(1), MPU_RESET_CMD);
   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   _inp(MPUPort(0));

   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   dwCount = 0x2000;
   while (dwCount && _inp(MPUPort(1)) & 0x40) --dwCount;
   _outp(MPUPort(1), MPU_RESET_CMD);
   for (dwCount=0; dwCount<0x2000; dwCount++) ;
   _inp(MPUPort(0));
   }


/*ีออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ*/
/*ณ LoadSBK                                                                ณ*/
/*ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

static void LoadSBK
   (
   void
   )

   {
   /* use embeded preset objects */
   spSound.bank_no = 0;            /* load as Bank 0 */
   spSound.total_banks = 1;        /* use 1 bank first */
   lBankSizes[ 0 ] = 0;            /* ram is not needed */

   spSound.banksizes = lBankSizes;
   awe32DefineBankSizes( &spSound );
   awe32SoundPad.SPad1 = awe32SPad1Obj;
   awe32SoundPad.SPad2 = awe32SPad2Obj;
   awe32SoundPad.SPad3 = awe32SPad3Obj;
   awe32SoundPad.SPad4 = awe32SPad4Obj;
   awe32SoundPad.SPad5 = awe32SPad5Obj;
   awe32SoundPad.SPad6 = awe32SPad6Obj;
   awe32SoundPad.SPad7 = awe32SPad7Obj;
   }


int AWE32_Init
   (
   void
   )

   {
   int status;
   BLASTER_CONFIG Blaster;

   wSBCBaseAddx = 0x220;
   wEMUBaseAddx = 0x620;
   wMpuBaseAddx = 0x330;

   status = BLASTER_GetCardSettings( &Blaster );
   if ( status != BLASTER_Ok )
      {
      status = BLASTER_GetEnv( &Blaster );
      if ( status != BLASTER_Ok )
         {
         AWE32_SetErrorCode( AWE32_SoundBlasterError );
         return( AWE32_Error );
         }
      }

   wSBCBaseAddx = Blaster.Address;
   if ( wSBCBaseAddx == UNDEFINED )
      {
      wSBCBaseAddx = 0x220;
      }

   wMpuBaseAddx = Blaster.Midi;
   if ( wMpuBaseAddx == UNDEFINED )
      {
      wMpuBaseAddx = 0x330;
      }

   wEMUBaseAddx = Blaster.Emu;
   if ( wEMUBaseAddx <= 0 )
      {
      wEMUBaseAddx = wSBCBaseAddx + 0x400;
      }

   status = awe32Detect( wEMUBaseAddx );
   if ( status )
      {
      AWE32_SetErrorCode( AWE32_NotDetected );
      return( AWE32_Error );
      }

   status = awe32InitHardware();
   if ( status )
      {
      AWE32_SetErrorCode( AWE32_UnableToInitialize );
      return( AWE32_Error );
      }


   status = awe32InitMIDI();
   if ( status )
      {
      AWE32_Shutdown();
      AWE32_SetErrorCode( AWE32_MPU401Error )
      return( AWE32_Error );
      }

/*
    status = InitMPU();
   if ( status )
      {
      ShutdownMPU();
      status = InitMPU();
      if ( status )
         {
         ShutdownMPU();
         status = InitMPU();
         if ( status )
            {
            AWE32_Shutdown();
            AWE32_SetErrorCode( AWE32_MPU401Error )
            return( AWE32_Error );
            }
         }
      }
*/
   status  = DPMI_LockMemoryRegion( AWE32_LockStart, AWE32_LockEnd );
   status |= DPMI_Lock( wSBCBaseAddx );
   status |= DPMI_Lock( wEMUBaseAddx );
   status |= DPMI_Lock( wMpuBaseAddx );
   status |= DPMI_Lock( spSound );
   status |= DPMI_Lock( lBankSizes );
   status |= DPMI_LockMemory( NoteFlags, sizeof( NoteFlags ) );

   // Lock awe32 library
   status  = DPMI_LockMemoryRegion( __midieng_code, __midieng_ecode );
   status  = DPMI_LockMemoryRegion( __midieng_code(), __midieng_ecode() );
   status  = DPMI_LockMemoryRegion( __nrpn_code, __nrpn_ecode );
   status  = DPMI_LockMemoryRegion( __nrpn_code(), __nrpn_ecode() );
   status  = DPMI_LockMemoryRegion( &__midivar_data, &__midivar_edata );
   status  = DPMI_LockMemoryRegion( &__nrpnvar_data, &__nrpnvar_edata );
   status  = DPMI_LockMemoryRegion( &__embed_data, &__embed_edata );

   if ( status != DPMI_Ok )
      {
      ShutdownMPU();
      awe32Terminate();
      AWE32_SetErrorCode( AWE32_DPMI_Error );
      return( AWE32_Error );
      }

   // Set the number of voices to use to 32
   awe32NumG = 32;

   awe32TotalPatchRam(&spSound);

   LoadSBK();
   awe32InitMIDI();
   awe32InitNRPN();

   memset( NoteFlags, 0, sizeof( NoteFlags ) );

   return( AWE32_Ok );
   }

void AWE32_Shutdown
   (
   void
   )

   {
   ShutdownMPU();
   awe32Terminate();

   DPMI_UnlockMemoryRegion( AWE32_LockStart, AWE32_LockEnd );
   DPMI_Unlock( wSBCBaseAddx );
   DPMI_Unlock( wEMUBaseAddx );
   DPMI_Unlock( wMpuBaseAddx );
   DPMI_Unlock( spSound );
   DPMI_Unlock( lBankSizes );
   DPMI_UnlockMemory( NoteFlags, sizeof( NoteFlags ) );

   // Unlock awe32 library
   DPMI_UnlockMemoryRegion( __midieng_code, __midieng_ecode );
   DPMI_UnlockMemoryRegion( __midieng_code(), __midieng_ecode() );
   DPMI_UnlockMemoryRegion( __nrpn_code, __nrpn_ecode );
   DPMI_UnlockMemoryRegion( __nrpn_code(), __nrpn_ecode() );
   DPMI_UnlockMemoryRegion( &__midivar_data, &__midivar_edata );
   DPMI_UnlockMemoryRegion( &__nrpnvar_data, &__nrpnvar_edata );
   DPMI_UnlockMemoryRegion( &__embed_data, &__embed_edata );
   }
