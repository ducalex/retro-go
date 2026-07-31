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
   module: PITCH.H

   author: James R. Dose
   date:   June 14, 1994

   Public header for PITCH.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __PITCH_H
#define __PITCH_H

enum PITCH_ERRORS
   {
   PITCH_Warning = -2,
   PITCH_Error = -1,
   PITCH_Ok = 0,
   };

//void          PITCH_Init( void );
unsigned long PITCH_GetScale( int pitchoffset );
void          PITCH_UnlockMemory( void );
int           PITCH_LockMemory( void );
#endif
