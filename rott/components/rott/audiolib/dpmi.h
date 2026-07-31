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
   module: DPMI.H

   author: James R. Dose
   date:   March 31, 1994

   Inline functions for performing DPMI calls.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __DPMI_H
#define __DPMI_H

enum DPMI_Errors
   {
   DPMI_Warning = -2,
   DPMI_Error   = -1,
   DPMI_Ok      = 0
   };

typedef struct
   {
   unsigned long  EDI;
   unsigned long  ESI;
   unsigned long  EBP;
   unsigned long  Reserved;
   unsigned long  EBX;
   unsigned long  EDX;
   unsigned long  ECX;
   unsigned long  EAX;
   unsigned short Flags;
   unsigned short ES;
   unsigned short DS;
   unsigned short FS;
   unsigned short GS;
   unsigned short IP;
   unsigned short CS;
   unsigned short SP;
   unsigned short SS;
   } dpmi_regs;

unsigned long DPMI_GetRealModeVector( int num );
void DPMI_SetRealModeVector( int num, unsigned long vector );
int  DPMI_CallRealModeFunction( dpmi_regs *callregs );
int  DPMI_GetDOSMemory( void **ptr, long *descriptor, unsigned length );
int  DPMI_FreeDOSMemory( long descriptor );
int  DPMI_LockMemory( void *address, unsigned length );
int  DPMI_LockMemoryRegion( void *start, void *end );
int  DPMI_UnlockMemory( void *address, unsigned length );
int  DPMI_UnlockMemoryRegion( void *start, void *end );

#define DPMI_Lock( variable ) \
   ( DPMI_LockMemory( (void *) &( variable ), sizeof( variable ) ) )

#define DPMI_Unlock( variable ) \
   ( DPMI_UnlockMemory( (void *) &( variable ), sizeof( variable ) ) )

#ifdef PLAT_DOS
#pragma aux DPMI_GetDOSMemory = \
   "mov    eax, 0100h",         \
   "add    ebx, 15",            \
   "shr    ebx, 4",             \
   "int    31h",                \
   "jc     DPMI_Exit",          \
   "movzx  eax, ax",            \
   "shl    eax, 4",             \
   "mov    [ esi ], eax",       \
   "mov    [ edi ], edx",       \
   "sub    eax, eax",           \
   "DPMI_Exit:",                \
   parm [ esi ] [ edi ] [ ebx ] modify exact [ eax ebx edx ];

#pragma aux DPMI_FreeDOSMemory = \
   "mov    eax, 0101h",          \
   "int    31h",                 \
   "jc     DPMI_Exit",           \
   "sub    eax, eax",            \
   "DPMI_Exit:",                 \
   parm [ edx ] modify exact [ eax ];
#endif

#endif
