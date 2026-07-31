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
#ifndef __MPU401_H
#define __MPU401_H

#define MPU_DefaultAddress 0x330

enum MPU_ERRORS
   {
   MPU_Warning = -2,
   MPU_Error = -1,
   MPU_Ok = 0,
   MPU_DPMI_Error
   };

#define MPU_NotFound       -1
#define MPU_UARTFailed     -2

#define MPU_ReadyToWrite   0x40
#define MPU_ReadyToRead    0x80
#define MPU_CmdEnterUART   0x3f
#define MPU_CmdReset       0xff
#define MPU_CmdAcknowledge 0xfe

extern int MPU_BaseAddr;
extern unsigned MPU_Delay;

void MPU_SendCommand( int data );
void MPU_SendMidi( int data );
int  MPU_Reset( void );
int  MPU_EnterUART( void );
int  MPU_Init( int addr );
void MPU_ResetMidi( void );
void MPU_NoteOff( int channel, int key, int velocity );
void MPU_NoteOn( int channel, int key, int velocity );
void MPU_PolyAftertouch( int channel, int key, int pressure );
void MPU_ControlChange( int channel, int number, int value );
void MPU_ProgramChange( int channel, int program );
void MPU_ChannelAftertouch( int channel, int pressure );
void MPU_PitchBend( int channel, int lsb, int msb );
void MPU_UnlockMemory( void );
int  MPU_LockMemory( void );

#endif
