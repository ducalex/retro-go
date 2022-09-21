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

#ifdef _WIN32
#include "../../../Engine/src/windows/inttypes.h"
#else
#include <inttypes.h>
#endif

enum DPMI_Errors
   {
   DPMI_Warning = -2,
   DPMI_Error   = -1,
   DPMI_Ok      = 0
   };

typedef struct
   {
   uint32_t  EDI;
   uint32_t  ESI;
   uint32_t  EBP;
   uint32_t  Reserved;
   uint32_t  EBX;
   uint32_t  EDX;
   uint32_t  ECX;
   uint32_t  EAX;
   uint32_t Flags;
   uint32_t ES;
   uint32_t DS;
   uint32_t FS;
   uint32_t GS;
   uint32_t IP;
   uint16_t CS;
   uint16_t SP;
   uint16_t SS;
   } dpmi_regs;

uint32_t DPMI_GetRealModeVector( int num );

int  DPMI_CallRealModeFunction( dpmi_regs *callregs );
int  DPMI_GetDOSMemory( void **ptr, int *descriptor, unsigned length );
int  DPMI_FreeDOSMemory( int descriptor );
int  DPMI_LockMemory( void *address, unsigned length );
int  DPMI_LockMemoryRegion( void *start, void *end );
int  DPMI_UnlockMemory( void *address, unsigned length );
int  DPMI_UnlockMemoryRegion( void *start, void *end );

#define DPMI_Lock( variable ) \
   ( DPMI_LockMemory( &( variable ), sizeof( variable ) ) )

#define DPMI_Unlock( variable ) \
   ( DPMI_UnlockMemory( &( variable ), sizeof( variable ) ) )

#endif
