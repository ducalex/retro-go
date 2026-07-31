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
#ifndef _rt_swift_public
#define _rt_swift_public

//***************************************************************************
//
// Public header for RT_SWIFT.C.
//
//***************************************************************************

#include "rt_playr.h"

//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

int SWIFT_Initialize (void);
//
// Test for presence of SWIFT extensions and SWIFT device
// Returns 1 (TRUE) if SWIFT features are available, 0 otherwise.
// Remember to call SWIFT_Terminate() if SWIFT_Initialize succeeds!
//

void SWIFT_Terminate (void);
//
// Free resources required for SWIFT support.  If SWIFT_Initialize has
// not been called, or returned FALSE, this function does nothing.
// SWIFT_Terminate should always be called at some time after a call to
// SWIFT_Initialize has returned TRUE.
//

int SWIFT_GetAttachedDevice (void);
//
// Returns the device-type code for the attached SWIFT device, if any.
//

int SWIFT_GetStaticDeviceInfo (SWIFT_StaticData far *psd);
//
// Reads static device data.						
//

void SWIFT_Get3DStatus (SWIFT_3DStatus far *pstat);
//
// Read the current input state of the device.				
//

void SWIFT_TactileFeedback (int d, int on, int off);
//
// Generates tactile feedback to user.					
// d   = duration of tactile burst, in milliseconds.			
// on  = motor on-time per cycle, in milliseconds.			
// off = motor off-time per cycle, in milliseconds.			
//

unsigned SWIFT_GetDynamicDeviceData (void);
//
// Returns Dynamic Device Data word - see SDD_* above			
//


#endif
