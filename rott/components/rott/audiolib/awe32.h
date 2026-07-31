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
   module: AWE32.H

   author: James R. Dose
   date:   August 23, 1994

   Public header for AWE32.C  Cover functions for calling the
   AWE32 low-level library.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __AWE32_H
#define __AWE32_H

enum AWE32_ERRORS
   {
   AWE32_Warning = -2,
   AWE32_Error = -1,
   AWE32_Ok = 0,
   AWE32_SoundBlasterError,
   AWE32_NotDetected,
   AWE32_UnableToInitialize,
   AWE32_MPU401Error,
   AWE32_DPMI_Error
   };

char *AWE32_ErrorString( int ErrorNumber );
int  AWE32_Init( void );
void AWE32_Shutdown( void );
void AWE32_NoteOff( int channel, int key, int velocity );
void AWE32_NoteOn( int channel, int key, int velocity );
void AWE32_PolyAftertouch( int channel, int key, int pressure );
void AWE32_ChannelAftertouch( int channel, int pressure );
void AWE32_ControlChange( int channel, int number, int value );
void AWE32_ProgramChange( int channel, int program );
void AWE32_PitchBend( int channel, int lsb, int msb );

#endif
