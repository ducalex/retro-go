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
   module: LL_MAN.C

   author: James R. Dose
   date:   January 1, 1994

   Linked list management routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#define LOCKMEMORY

#include <stddef.h>
#include "ll_man.h"

#ifdef LOCKMEMORY
#include "dpmi.h"
#endif

#define OFFSET( structure, offset ) \
   ( *( ( char ** )&( structure )[ offset ] ) )


/**********************************************************************

   Memory locked functions:

**********************************************************************/


#define LL_LockStart LL_AddNode


void LL_AddNode
   (
   char *item,
   char **head,
   char **tail,
   int next,
   int prev
   )

   {
   OFFSET( item, prev ) = NULL;
   OFFSET( item, next ) = *head;

   if ( *head )
      {
      OFFSET( *head, prev ) = item;
      }
   else
      {
      *tail = item;
      }

   *head = item;
   }

void LL_RemoveNode
   (
   char *item,
   char **head,
   char **tail,
   int next,
   int prev
   )

   {
   if ( OFFSET( item, prev ) == NULL )
      {
      *head = OFFSET( item, next );
      }
   else
      {
      OFFSET( OFFSET( item, prev ), next ) = OFFSET( item, next );
      }

   if ( OFFSET( item, next ) == NULL )
      {
      *tail = OFFSET( item, prev );
      }
   else
      {
      OFFSET( OFFSET( item, next ), prev ) = OFFSET( item, prev );
      }

   OFFSET( item, next ) = NULL;
   OFFSET( item, prev ) = NULL;
   }


/*---------------------------------------------------------------------
   Function: LL_LockEnd

   Used for determining the length of the functions to lock in memory.
---------------------------------------------------------------------*/

static void LL_LockEnd
   (
   void
   )

   {
   }


/*---------------------------------------------------------------------
   Function: LL_UnlockMemory

   Unlocks all neccessary data.
---------------------------------------------------------------------*/

void LL_UnlockMemory
   (
   void
   )

   {
#ifdef LOCKMEMORY

   DPMI_UnlockMemoryRegion( LL_LockStart, LL_LockEnd );

#endif
   }


/*---------------------------------------------------------------------
   Function: LL_LockMemory

   Locks all neccessary data.
---------------------------------------------------------------------*/

int LL_LockMemory
   (
   void
   )

   {

#ifdef LOCKMEMORY

   int status;

   status = DPMI_LockMemoryRegion( LL_LockStart, LL_LockEnd );
   if ( status != DPMI_Ok )
      {
      return( LL_Error );
      }

#endif

   return( LL_Ok );
   }
