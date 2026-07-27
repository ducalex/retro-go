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
   module: DMA.C

   author: James R. Dose
   date:   February 4, 1994

   Low level routines to for programming the DMA controller for 8 bit
   and 16 bit transfers.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include "dma.h"

#define DMA_MaxChannel 7

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define BYTE 0
#define WORD 1

typedef struct
   {
   int Valid;
   int Width;
   int Mask;
   int Mode;
   int Clear;
   int Page;
   int Address;
   int Length;
   } DMA_PORT;

static const DMA_PORT DMA_PortInfo[ DMA_MaxChannel + 1 ] =
   {
      {   VALID, BYTE,  0xA,  0xB,  0xC, 0x87,  0x0,  0x1 },
      {   VALID, BYTE,  0xA,  0xB,  0xC, 0x83,  0x2,  0x3 },
      { INVALID, BYTE,  0xA,  0xB,  0xC, 0x81,  0x4,  0x5 },
      {   VALID, BYTE,  0xA,  0xB,  0xC, 0x82,  0x6,  0x7 },
      { INVALID, WORD, 0xD4, 0xD6, 0xD8, 0x8F, 0xC0, 0xC2 },
      {   VALID, WORD, 0xD4, 0xD6, 0xD8, 0x8B, 0xC4, 0xC6 },
      {   VALID, WORD, 0xD4, 0xD6, 0xD8, 0x89, 0xC8, 0xCA },
      {   VALID, WORD, 0xD4, 0xD6, 0xD8, 0x8A, 0xCC, 0xCE },
   };

int DMA_ErrorCode = DMA_Ok;

#define DMA_SetErrorCode( status ) \
   DMA_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: DMA_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *DMA_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case DMA_Error :
         ErrorString = DMA_ErrorString( DMA_ErrorCode );
         break;

      case DMA_Ok :
         ErrorString = "DMA channel ok.";
         break;

      case DMA_ChannelOutOfRange :
         ErrorString = "DMA channel out of valid range.";
         break;

      case DMA_InvalidChannel :
         ErrorString = "Unsupported DMA channel.";
         break;

      default :
         ErrorString = "Unknown DMA error code.";
         break;
      }

   return( ErrorString );
   }


/*---------------------------------------------------------------------
   Function: DMA_VerifyChannel

   Verifies whether a DMA channel is available to transfer data.
---------------------------------------------------------------------*/

int DMA_VerifyChannel
   (
   int channel
   )

   {
   int      status;
   int      Error;

   status = DMA_Ok;
   Error  = DMA_Ok;

   if ( ( channel < 0 ) || ( DMA_MaxChannel < channel ) )
      {
      Error = DMA_ChannelOutOfRange;
      status = DMA_Error;
      }
   else if ( DMA_PortInfo[ channel ].Valid == INVALID )
      {
      Error = DMA_InvalidChannel;
      status = DMA_Error;
      }

   DMA_SetErrorCode( Error );
   return( status );
   }


/*---------------------------------------------------------------------
   Function: DMA_SetupTransfer

   Programs the specified DMA channel to transfer data.
---------------------------------------------------------------------*/

int DMA_SetupTransfer
   (
   int  channel,
   char *address,
   int  length,
   int  mode
   )

   {
   DMA_PORT *Port;
   int      addr;
   int      ChannelSelect;
   int      Page;
   int      HiByte;
   int      LoByte;
   int      TransferLength;
   int      status;

   status = DMA_VerifyChannel( channel );

   if ( status == DMA_Ok )
      {
      Port = &DMA_PortInfo[ channel ];
      ChannelSelect = channel & 0x3;

      addr = ( int )address;

      if ( Port->Width == WORD )
         {
         Page   = ( addr >> 16 ) & 255;
         HiByte = ( addr >> 9 ) & 255;
         LoByte = ( addr >> 1 ) & 255;

         // Convert the length in bytes to the length in words
         TransferLength = ( length + 1 ) >> 1;

         // The length is always one less the number of bytes or words
         // that we're going to send
         TransferLength--;
         }
      else
         {
         Page   = ( addr >> 16 ) & 255;
         HiByte = ( addr >> 8 ) & 255;
         LoByte = addr & 255;

         // The length is always one less the number of bytes or words
         // that we're going to send
         TransferLength = length - 1;
         }

      // Mask off DMA channel
      outp( Port->Mask, 4 | ChannelSelect );

      // Clear flip-flop to lower byte with any data
      outp( Port->Clear, 0 );

      // Set DMA mode
      switch( mode )
         {
         case DMA_SingleShotRead :
            outp( Port->Mode, 0x48 | ChannelSelect );
            break;

         case DMA_SingleShotWrite :
            outp( Port->Mode, 0x44 | ChannelSelect );
            break;

         case DMA_AutoInitRead :
            outp( Port->Mode, 0x58 | ChannelSelect );
            break;

         case DMA_AutoInitWrite :
            outp( Port->Mode, 0x54 | ChannelSelect );
            break;
         }

      // Send address
      outp( Port->Address, LoByte );
      outp( Port->Address, HiByte );

      // Send page
      outp( Port->Page, Page );

      // Send length
      outp( Port->Length, TransferLength );
      outp( Port->Length, TransferLength >> 8 );

      // enable DMA channel
      outp( Port->Mask, ChannelSelect );
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: DMA_EndTransfer

   Ends use of the specified DMA channel.
---------------------------------------------------------------------*/

int DMA_EndTransfer
   (
   int channel
   )

   {
   DMA_PORT *Port;
   int       ChannelSelect;
   int       status;

   status = DMA_VerifyChannel( channel );
   if ( status == DMA_Ok )
      {
      Port = &DMA_PortInfo[ channel ];
      ChannelSelect = channel & 0x3;

      // Mask off DMA channel
      outp( Port->Mask, 4 | ChannelSelect );

      // Clear flip-flop to lower byte with any data
      outp( Port->Clear, 0 );
      }

   return( status );
   }


/*---------------------------------------------------------------------
   Function: DMA_GetCurrentPos

   Returns the position of the specified DMA transfer.
---------------------------------------------------------------------*/

char *DMA_GetCurrentPos
   (
   int channel
   )

   {
   DMA_PORT      *Port;
   unsigned long addr;
   int           status;

   addr   = NULL;
   status = DMA_VerifyChannel( channel );

   if ( status == DMA_Ok )
      {
      Port = &DMA_PortInfo[ channel ];

      if ( Port->Width == WORD )
         {
         // Get address
         addr  = inp( Port->Address ) << 1;
         addr |= inp( Port->Address ) << 9;

         // Get page
         addr |= inp( Port->Page ) << 16;
         }
      else
         {
         // Get address
         addr = inp( Port->Address );
         addr |= inp( Port->Address ) << 8;

         // Get page
         addr |= inp( Port->Page ) << 16;
         }
      }

   return( ( char * )addr );
   }


/*---------------------------------------------------------------------
   Function: DMA_GetTransferCount

   Returns how many bytes are left in the DMA's transfer.
---------------------------------------------------------------------*/

int DMA_GetTransferCount
   (
   int channel
   )

   {
   DMA_PORT      *Port;
   int           count;
   int           status;

   status = DMA_Ok;

   count = 0;

   if ( ( channel < 0 ) || ( DMA_MaxChannel < channel ) )
      {
      status = DMA_ChannelOutOfRange;
      }
   else if ( DMA_PortInfo[ channel ].Valid == INVALID )
      {
      status = DMA_InvalidChannel;
      }

   if ( status == DMA_Ok )
      {
      Port = &DMA_PortInfo[ channel ];

      outp( Port->Clear, 0 );
      count  = inp( Port->Length );
      count += inp( Port->Length ) << 8;

      if ( Port->Width == WORD )
         {
         count <<= 1;
         }
      }

   DMA_SetErrorCode( status );

   return( count );
   }
