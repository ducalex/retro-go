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
   module: DPMI.C

   author: James R. Dose
   date:   April 8, 1994

   Functions for performing DPMI calls.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/
#if 0
#include <dos.h>
#include <string.h>
#include "dpmi.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

static union  REGS  Regs;
static struct SREGS SegRegs;


/*---------------------------------------------------------------------
   Function: DPMI_GetRealModeVector

   Returns the vector of a real mode interrupt.
---------------------------------------------------------------------*/

unsigned long DPMI_GetRealModeVector
   (
   int num
   )

   {
   unsigned long vector;

   Regs.x.eax = 0x0200;
   Regs.h.bl  = num;
   int386( 0x31, &Regs, &Regs );

   vector   = Regs.w.cx & 0xffff;
   vector <<= 16;
   vector  |= Regs.w.dx & 0xffff;

   return( vector );
   }


/*---------------------------------------------------------------------
   Function: DPMI_SetRealModeVector

   Sets the vector of a real mode interrupt.
---------------------------------------------------------------------*/

void DPMI_SetRealModeVector
   (
   int num,
   unsigned long vector
   )

   {
   Regs.x.eax = 0x0201;
   Regs.h.bl  = num;
   Regs.w.dx = vector & 0xffff;
   Regs.w.cx = ( vector >> 16 ) & 0xffff;

   int386( 0x31, &Regs, &Regs );
   }


/*---------------------------------------------------------------------
   Function: DPMI_CallRealModeFunction

   Performs a call to a real mode function.
---------------------------------------------------------------------*/

int DPMI_CallRealModeFunction
   (
   dpmi_regs *callregs
   )

   {
   // Setup our registers to call DPMI
   Regs.w.ax = 0x0301;
   Regs.h.bl = 0;
   Regs.h.bh = 0;
   Regs.w.cx = 0;

   SegRegs.es = FP_SEG( callregs );
   Regs.x.edi = FP_OFF( callregs );

   // Call Real-mode procedure with Far Return Frame
   int386x( 0x31, &Regs, &Regs, &SegRegs );

   if ( Regs.x.cflag )
      {
      return( DPMI_Error );
      }

   return( DPMI_Ok );
   }


/*---------------------------------------------------------------------
   Function: DPMI_LockMemory

   Locks a region of memory to keep the virtual memory manager from
   paging the region out.
---------------------------------------------------------------------*/

int DPMI_LockMemory
   (
   void *address,
   unsigned length
   )

   {
   unsigned linear;

   // Thanks to DOS/4GW's zero-based flat memory model, converting
   // a pointer of any type to a linear address is trivial.

   linear = (unsigned) address;

   // DPMI Lock Linear Region
   Regs.w.ax = 0x600;

   // Linear address in BX:CX
   Regs.w.bx = (linear >> 16);
   Regs.w.cx = (linear & 0xFFFF);

   // Length in SI:DI
   Regs.w.si = (length >> 16);
   Regs.w.di = (length & 0xFFFF);

   int386 (0x31, &Regs, &Regs);

   // Return 0 if can't lock
   if ( Regs.w.cflag )
      {
      return( DPMI_Error );
      }

   return ( DPMI_Ok );
   }


/*---------------------------------------------------------------------
   Function: DPMI_LockMemoryRegion

   Locks a region of memory to keep the virtual memory manager from
   paging the region out.
---------------------------------------------------------------------*/

int DPMI_LockMemoryRegion
   (
   void *start,
   void *end
   )

   {
   int status;

   status = DPMI_LockMemory( start, ( char * )end - ( char * )start );

   return( status );
   }


/*---------------------------------------------------------------------
   Function: DPMI_UnlockMemory

   Unlocks a region of memory that was previously locked.
---------------------------------------------------------------------*/

int DPMI_UnlockMemory
   (
   void *address,
   unsigned length
   )

   {
   unsigned linear;

   // Thanks to DOS/4GW's zero-based flat memory model, converting
   // a pointer of any type to a linear address is trivial.

   linear = (unsigned) address;

   // DPMI Unlock Linear Region
   Regs.w.ax = 0x601;

   // Linear address in BX:CX
   Regs.w.bx = (linear >> 16);
   Regs.w.cx = (linear & 0xFFFF);

   // Length in SI:DI
   Regs.w.si = (length >> 16);
   Regs.w.di = (length & 0xFFFF);

   int386 (0x31, &Regs, &Regs);

   // Return 0 if can't unlock
   if ( Regs.w.cflag )
      {
      return( DPMI_Error );
      }

   return ( DPMI_Ok );
   }


/*---------------------------------------------------------------------
   Function: DPMI_UnlockMemoryRegion

   Unlocks a region of memory that was previously locked.
---------------------------------------------------------------------*/

int DPMI_UnlockMemoryRegion
   (
   void *start,
   void *end
   )

   {
   int status;

   status = DPMI_UnlockMemory( start, ( char * )end - ( char * )start );

   return( status );
   }
#endif