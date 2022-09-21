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
   module: MPU401.C

   author: James R. Dose
   date:   January 1, 1994

   Low level routines to support sending of MIDI data to MPU401
   compatible MIDI interfaces.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/
#if 0
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "dpmi.h"
#include "user.h"
#include "mpu401.h"

#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_AFTER_TCH   0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_AFTER_TOUCH      0xD0
#define MIDI_PITCH_BEND       0xE0
#define MIDI_META_EVENT       0xFF
#define MIDI_END_OF_TRACK     0x2F
#define MIDI_TEMPO_CHANGE     0x51
#define MIDI_MONO_MODE_ON     0x7E
#define MIDI_ALL_NOTES_OFF    0x7B

int MPU_BaseAddr = MPU_DefaultAddress;

//unsigned MPU_Delay = 500;
//unsigned MPU_Delay = 5000;
unsigned MPU_Delay = 0x5000;


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define MPU_LockStart MPU_SendMidi


/*---------------------------------------------------------------------
   Function: MPU_SendMidi

   Sends a byte of MIDI data to the music device.
---------------------------------------------------------------------*/

void MPU_SendMidi
   (
   int data
   )

   {
   int      port = MPU_BaseAddr + 1;
   unsigned count;

   count = MPU_Delay;
   while( count > 0 )
      {
      // check if status port says we're ready for write
      if ( !( inp( port ) & MPU_ReadyToWrite ) )
         {
         break;
         }

      count--;
      }

   port--;

   // Send the midi data
   outp( port, data );
   }


/*---------------------------------------------------------------------
   Function: MPU_NoteOff

   Sends a full MIDI note off event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOff
   (
   int channel,
   int key,
   int velocity
   )

   {
   MPU_SendMidi( MIDI_NOTE_OFF | channel );
   MPU_SendMidi( key );
   MPU_SendMidi( velocity );
   }


/*---------------------------------------------------------------------
   Function: MPU_NoteOn

   Sends a full MIDI note on event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOn
   (
   int channel,
   int key,
   int velocity
   )

   {
   MPU_SendMidi( MIDI_NOTE_ON | channel );
   MPU_SendMidi( key );
   MPU_SendMidi( velocity );
   }


/*---------------------------------------------------------------------
   Function: MPU_PolyAftertouch

   Sends a full MIDI polyphonic aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_PolyAftertouch
   (
   int channel,
   int key,
   int pressure
   )

   {
   MPU_SendMidi( MIDI_POLY_AFTER_TCH | channel );
   MPU_SendMidi( key );
   MPU_SendMidi( pressure );
   }


/*---------------------------------------------------------------------
   Function: MPU_ControlChange

   Sends a full MIDI control change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ControlChange
   (
   int channel,
   int number,
   int value
   )

   {
   MPU_SendMidi( MIDI_CONTROL_CHANGE | channel );
   MPU_SendMidi( number );
   MPU_SendMidi( value );
   }


/*---------------------------------------------------------------------
   Function: MPU_ProgramChange

   Sends a full MIDI program change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ProgramChange
   (
   int channel,
   int program
   )

   {
   MPU_SendMidi( MIDI_PROGRAM_CHANGE | channel );
   MPU_SendMidi( program );
   }


/*---------------------------------------------------------------------
   Function: MPU_ChannelAftertouch

   Sends a full MIDI channel aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_ChannelAftertouch
   (
   int channel,
   int pressure
   )

   {
   MPU_SendMidi( MIDI_AFTER_TOUCH | channel );
   MPU_SendMidi( pressure );
   }


/*---------------------------------------------------------------------
   Function: MPU_PitchBend

   Sends a full MIDI pitch bend event out to the music device.
---------------------------------------------------------------------*/

void MPU_PitchBend
   (
   int channel,
   int lsb,
   int msb
   )

   {
   MPU_SendMidi( MIDI_PITCH_BEND | channel );
   MPU_SendMidi( lsb );
   MPU_SendMidi( msb );
   }


/*---------------------------------------------------------------------
   Function: MPU_SendCommand

   Sends a command to the MPU401 card.
---------------------------------------------------------------------*/

void MPU_SendCommand
   (
   int data
   )

   {
   int      port = MPU_BaseAddr + 1;
   unsigned count;

   count = 0xffff;
   while( count > 0 )
      {
      // check if status port says we're ready for write
      if ( !( inp( port ) & MPU_ReadyToWrite ) )
         {
         break;
         }
      count--;
      }

   outp( port, data );
   }


/*---------------------------------------------------------------------
   Function: MPU_Reset

   Resets the MPU401 card.
---------------------------------------------------------------------*/

int MPU_Reset
   (
   void
   )

   {
   int      port = MPU_BaseAddr + 1;
   unsigned count;

   // Output "Reset" command via Command port
   MPU_SendCommand( MPU_CmdReset );

   // Wait for status port to be ready for read
   count = 0xffff;
   while( count > 0 )
      {
      if ( !( inp( port ) & MPU_ReadyToRead ) )
         {
         port--;

         // Check for a successful reset
         if ( inp( port ) == MPU_CmdAcknowledge )
            {
            return( MPU_Ok );
            }

         port++;
         }
      count--;
      }

   // Failed to reset : MPU-401 not detected
   return( MPU_NotFound );
   }


/*---------------------------------------------------------------------
   Function: MPU_EnterUART

   Sets the MPU401 card to operate in UART mode.
---------------------------------------------------------------------*/

int MPU_EnterUART
   (
   void
   )

   {
   int      port = MPU_BaseAddr + 1;
   unsigned count;

   // Output "Enter UART" command via Command port
   MPU_SendCommand( MPU_CmdEnterUART );

   // Wait for status port to be ready for read
   count = 0xffff;
   while( count > 0 )
      {
      if ( !( inp( port ) & MPU_ReadyToRead ) )
         {
         port--;

         // Check for a successful reset
         if ( inp( port ) == MPU_CmdAcknowledge )
            {
            return( MPU_Ok );
            }

         port++;
         }
      count--;
      }

   // Failed to reset : MPU-401 not detected
   return( MPU_UARTFailed );
   }


/*---------------------------------------------------------------------
   Function: MPU_Init

   Detects and initializes the MPU401 card.
---------------------------------------------------------------------*/

int MPU_Init
   (
   int addr
   )

   {
   int status;
   int count;
   char *ptr;

   ptr = USER_GetText( "MPUDELAY" );
   if ( ptr != NULL )
      {
      MPU_Delay = ( unsigned )atol( ptr );
      }

   MPU_BaseAddr = addr;

   count = 4;
   while( count > 0 )
      {
      status = MPU_Reset();
      if ( status == MPU_Ok )
         {
         return( MPU_EnterUART() );
         }
      count--;
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: MPU_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void MPU_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: MPU_UnlockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

void MPU_UnlockMemory
   (
   void
   )

   {
   DPMI_UnlockMemoryRegion( MPU_LockStart, MPU_LockEnd );
   DPMI_Unlock( MPU_BaseAddr );
   DPMI_Unlock( MPU_Delay );
   }


/*---------------------------------------------------------------------
   Function: MPU_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int MPU_LockMemory
   (
   void
   )

   {
   int status;

   status  = DPMI_LockMemoryRegion( MPU_LockStart, MPU_LockEnd );
   status |= DPMI_Lock( MPU_BaseAddr );
   status |= DPMI_Lock( MPU_Delay );

   if ( status != DPMI_Ok )
      {
      MPU_UnlockMemory();
      return( MPU_Error );
      }

   return( MPU_Ok );
   }
#endif