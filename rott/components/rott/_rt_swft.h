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
#ifndef _rt_swift_private
#define _rt_swift_private

#ifdef DOS
//****************************************************************************
//
// Private header for RT_SWIFT.C.
//
//****************************************************************************



//****************************************************************************
//
// DEFINES
//
//****************************************************************************

#define DPMI_INT     0x31
#define MOUSE_INT    0x33
#define DOSMEMSIZE   64                // enough for any SWIFT structure

//
// device type codes, returned in deviceType field (SWIFT_StaticData)		
//
#define SWIFT_DEV_NONE		0
#define SWIFT_DEV_CYBERMAN	1

//
// Dynamic device data
//
#define SDD_EXTERNAL_POWER_CONNECTED	1
#define SDD_EXTERNAL_POWER_TOO_HIGH	   2

#define AX(r) ((r).x.eax)
#define BX(r) ((r).x.ebx)
#define CX(r) ((r).x.ecx)
#define DX(r) ((r).x.edx)
#define SI(r) ((r).x.esi)
#define DI(r) ((r).x.edi)


//****************************************************************************
//
// TYPEDEFS
//
//****************************************************************************

                                          // Active flag:
static int fActive;                       //  TRUE after successful init
                                          //  and before termination
static int nAttached = SWIFT_DEV_NONE;    // type of SWIFT device

union REGS regs;
struct SREGS sregs;

short selector;                           // selector of DOS memory block
short segment;                            // segment of DOS memory block
void far *pdosmem;                        // pointer to DOS memory block

// DPMI real mode interrupt structure
static struct rminfo
{
    long di;
    long si;
    long bp;
    long reserved_by_system;
    long bx;
    long dx;
    long cx;
    long ax;
    short flags;
    short es, ds, fs, gs, ip, cs, sp, ss;
} RMI;


//****************************************************************************
//
// PROTOTYPES
//
//****************************************************************************

void MouseInt (struct rminfo *prmi);
static void far *allocDOS (unsigned nbytes, short *pseg, short *psel);
static void freeDOS (short sel);

#endif

#endif

