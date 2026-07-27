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
   file:   GUS.C

   author: James R. Dose
   date:   September 7, 1994

   Gravis Ultrasound initialization routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "usrhooks.h"
#include "interrup.h"
#include "newgf1.h"
#include "gusmidi.h"
#include "guswave.h"
#include "_guswave.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

// size of DMA buffer for patch loading
#define DMABUFFSIZE 2048U

struct gf1_dma_buff GUS_HoldBuffer;
static int          HoldBufferAllocated = FALSE;

static int GUS_Installed = 0;

extern VoiceNode   GUSWAVE_Voices[ VOICES ];
extern int GUSWAVE_Installed;

unsigned long GUS_TotalMemory;
int           GUS_MemConfig;

int GUS_AuxError  = 0;

int GUS_ErrorCode = GUS_Ok;

#define GUS_SetErrorCode( status ) \
   GUS_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: GUS_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *GUS_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case GUS_Warning :
      case GUS_Error :
         ErrorString = GUS_ErrorString( GUS_ErrorCode );
         break;

      case GUS_Ok :
         ErrorString = "Ultrasound music ok.";
         break;

      case GUS_OutOfMemory :
         ErrorString = "Out of memory in GusMidi.";
         break;

      case GUS_OutOfDosMemory :
         ErrorString = "Out of conventional (640K) memory in GusMidi.";
         break;

      case GUS_GF1Error :
         ErrorString = gf1_error_str( GUS_AuxError );
         break;

      case GUS_InvalidIrq :
         ErrorString = "Ultrasound IRQ must be 7 or less.";
         break;

      case GUS_ULTRADIRNotSet :
         ErrorString = "ULTRADIR environment variable not set.";
         break;

      case GUS_MissingConfig :
//         ErrorString = "Can't find GUSMIDI.INI file.";
         ErrorString = "Can't find ULTRAMID.INI file.";
         break;

      case GUS_FileError :
         ErrorString = strerror( GUS_AuxError );
         break;

      default :
         ErrorString = "Unknown Ultrasound error code.";
         break;
      }

   return( ErrorString );
   }



/*---------------------------------------------------------------------
   Function: D32DosMemAlloc

   Allocate a block of Conventional memory.
---------------------------------------------------------------------*/

void *D32DosMemAlloc
   (
   unsigned size
   )

   {
   union REGS r;

   // DPMI allocate DOS memory
   r.x.eax = 0x0100;

   // Number of paragraphs requested
   r.x.ebx = ( size + 15 ) >> 4;
   int386( 0x31, &r, &r );
   if ( r.x.cflag )
      {
      // Failed
      return( NULL );
      }

   return( ( void * )( ( r.x.eax & 0xFFFF ) << 4 ) );
   }


/*---------------------------------------------------------------------
   Function: GUS_Init

   Initializes the Gravis Ultrasound for sound and music playback.
---------------------------------------------------------------------*/

int GUS_Init
   (
   void
   )

   {
   struct load_os os;
   int ret;

   if ( GUS_Installed > 0 )
      {
      GUS_Installed++;
      return( GUS_Ok );
      }

   GUS_SetErrorCode( GUS_Ok );

   GUS_Installed = 0;

   GetUltraCfg( &os );

   if ( os.forced_gf1_irq > 7 )
      {
      GUS_SetErrorCode( GUS_InvalidIrq );
      return( GUS_Error );
      }

   if ( !HoldBufferAllocated )
      {
      GUS_HoldBuffer.vptr = D32DosMemAlloc( DMABUFFSIZE );
      if ( GUS_HoldBuffer.vptr == NULL )
         {
         GUS_SetErrorCode( GUS_OutOfDosMemory );
         return( GUS_Error );
         }
      GUS_HoldBuffer.paddr = ( unsigned long )GUS_HoldBuffer.vptr;

      HoldBufferAllocated = TRUE;
      }

   os.voices = 24;
   ret = gf1_load_os( &os );
   if ( ret )
      {
      GUS_AuxError = ret;
      GUS_SetErrorCode( GUS_GF1Error );
      return( GUS_Error );
      }

   GUS_TotalMemory = gf1_mem_avail();
   GUS_MemConfig   = ( GUS_TotalMemory - 1 ) >> 18;

   GUS_Installed = 1;
   return( GUS_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUS_Shutdown

   Ends use of the Gravis Ultrasound.  Must be called the same number
   of times as GUS_Init.
---------------------------------------------------------------------*/

void GUS_Shutdown
   (
   void
   )

   {
   if ( GUS_Installed > 0 )
      {
      GUS_Installed--;
      if ( GUS_Installed == 0 )
         {
         gf1_unload_os();
         }
      }
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_Shutdown

   Terminates use of the Gravis Ultrasound for digitized sound playback.
---------------------------------------------------------------------*/

void GUSWAVE_Shutdown
   (
   void
   )

   {
   int i;

   if ( GUSWAVE_Installed )
      {
      GUSWAVE_KillAllVoices();

      // free memory
      for ( i = 0; i < VOICES; i++ )
         {
         if ( GUSWAVE_Voices[ i ].mem != NULL )
            {
            gf1_free( GUSWAVE_Voices[ i ].mem );
            GUSWAVE_Voices[ i ].mem = NULL;
            }
         }

      GUS_Shutdown();
      GUSWAVE_Installed = FALSE;
      }
   }
