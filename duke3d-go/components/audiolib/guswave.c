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
   file:   GUSWAVE.C

   author: James R. Dose
   date:   March 23, 1994

   Digitized sound playback routines for the Gravis Ultrasound.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include "debugio.h"
#include "interrup.h"
#include "ll_man.h"
#include "pitch.h"
#include "user.h"
#include "multivoc.h"
#include "_guswave.h"
#include "newgf1.h"
#include "gusmidi.h"
#include "guswave.h"

#define ATR_INDEX               0x3c0
#define STATUS_REGISTER_1       0x3da

#define SetBorderColor(color) \
   { \
   inp  (STATUS_REGISTER_1); \
   outp (ATR_INDEX,0x31);    \
   outp (ATR_INDEX,color);   \
   }

static const int GUSWAVE_PanTable[ 32 ] =
   {
      8,  9, 10, 11, 11, 12, 13, 14,
     15, 14, 13, 12, 11, 10,  9,  8,
      7,  6,  5,  4,  4,  3,  2,  1,
      0,  1,  2,  3,  4,  5,  6,  7
   };

static voicelist VoiceList;
static voicelist VoicePool;

static voicestatus VoiceStatus[ MAX_VOICES ];
//static
VoiceNode GUSWAVE_Voices[ VOICES ];

static int GUSWAVE_VoiceHandle  = GUSWAVE_MinVoiceHandle;
static int GUSWAVE_MaxVoices = VOICES;
//static
int GUSWAVE_Installed = FALSE;

static void ( *GUSWAVE_CallBackFunc )( unsigned long ) = NULL;

// current volume for dig audio - from 0 to 4095
static int GUSWAVE_Volume = MAX_VOLUME;

static int GUSWAVE_SwapLeftRight = FALSE;

static int GUS_Debug = FALSE;

extern int GUSMIDI_Installed;

int GUSWAVE_ErrorCode = GUSWAVE_Ok;

#define GUSWAVE_SetErrorCode( status ) \
   GUSWAVE_ErrorCode   = ( status );


/*---------------------------------------------------------------------
   Function: GUSWAVE_ErrorString

   Returns a pointer to the error message associated with an error
   number.  A -1 returns a pointer the current error.
---------------------------------------------------------------------*/

char *GUSWAVE_ErrorString
   (
   int ErrorNumber
   )

   {
   char *ErrorString;

   switch( ErrorNumber )
      {
      case GUSWAVE_Warning :
      case GUSWAVE_Error :
         ErrorString = GUSWAVE_ErrorString( GUSWAVE_ErrorCode );
         break;

      case GUSWAVE_Ok :
         ErrorString = "GUSWAVE ok.";
         break;

      case GUSWAVE_GUSError :
         ErrorString = GUS_ErrorString( GUS_Error );
         break;

      case GUSWAVE_NotInstalled :
         ErrorString = "GUSWAVE not installed.";
         break;

      case GUSWAVE_NoVoices :
         ErrorString = "No free voices available to GUSWAVE.";
         break;

      case GUSWAVE_UltraNoMem :
         ErrorString = "Not enough Ultrasound memory available for GUSWAVE.";
         break;

      case GUSWAVE_UltraNoMemMIDI :
         ErrorString = "Not enough Ultrasound memory available for GUSWAVE.  "
            "Try initializing Sound FX before Music.";
         break;

      case GUSWAVE_VoiceNotFound :
         ErrorString = "No voice with matching handle found.";
         break;

      case GUSWAVE_InvalidVOCFile :
         ErrorString = "Invalid VOC file passed in to GUSWAVE.";
         break;

      case GUSWAVE_InvalidWAVFile :
         ErrorString = "Invalid WAV file passed in to GUSWAVE.";
         break;

      default :
         ErrorString = "Unknown GUSWAVE error code.";
         break;
      }

   return( ErrorString );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_CallBack

   GF1 callback service routine.
---------------------------------------------------------------------*/

char GUS_Silence8[ 1024 ] = //256 ] =
   {
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,

   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80


   };

//unsigned short GUS_Silence16[ 128 ] =
unsigned short GUS_Silence16[ 512 ] =
   {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,



   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

static int LOADDS GUSWAVE_CallBack
   (
   int reason,
   int voice,
   unsigned char **buf,
   unsigned long *size
   )

   {
   VoiceNode *Voice;
   playbackstatus status;

   // this function is called from an interrupt
   // remember not to make any DOS or BIOS calls from here
   // also don't call any C library functions unless you are sure that
   // they are reentrant
   // restore our DS register

   if ( VoiceStatus[ voice ].playing == FALSE )
      {
      return( DIG_DONE );
      }

   if ( reason == DIG_MORE_DATA )
      {
//      SetBorderColor(16);
      Voice = VoiceStatus[ voice ].Voice;

      if ( ( Voice != NULL ) && ( Voice->Playing ) )
/*
         {
         *buf = ( unsigned char * )GUS_Silence16;
         *size = 1024;

         SetBorderColor(0);
         return( DIG_MORE_DATA );
         }
 */
         {
         status = Voice->GetSound( Voice );
         if ( status != SoundDone )
            {
            if ( ( Voice->sound == NULL ) || ( status == NoMoreData ) )
               {
               if ( Voice->bits == 8 )
                  {
                  *buf = GUS_Silence8;
                  }
               else
                  {
                  *buf = ( unsigned char * )GUS_Silence16;
                  }
               *size = 256;
               }
            else
               {
               *buf  = Voice->sound;
               *size = Voice->length;
               }
            return( DIG_MORE_DATA );
            }
         }
//      SetBorderColor(16);
      return( DIG_DONE );
      }

   if ( reason == DIG_DONE )
      {
      Voice = VoiceStatus[ voice ].Voice;
      VoiceStatus[ voice ].playing = FALSE;

      if ( Voice != NULL )
         {
         Voice->Active   = FALSE;
         Voice->Playing  = FALSE;

// I'm commenting this out because a -1 could cause a crash if it
// is sent to the GF1 code.  This shouldn't be necessary since
// Active should be false when GF1voice is -1, but this is just
// a precaution.  Adjust the pan on the wrong voice is a lot
// more pleasant than a crash!
//         Voice->GF1voice = -1;

         LL_Remove( VoiceNode, &VoiceList, Voice );
         LL_AddToTail( VoiceNode, &VoicePool, Voice );
         }

      if ( GUSWAVE_CallBackFunc )
         {
         GUSWAVE_CallBackFunc( Voice->callbackval );
         }
      }

   return( DIG_DONE );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_DebugCallBack

   GF1 callback service routine with debugging info.
---------------------------------------------------------------------*/

static int LOADDS GUSWAVE_DebugCallBack
   (
   int reason,
   int voice,
   unsigned char **buf,
   unsigned long *size
   )

   {
   VoiceNode *Voice;

   // this function is called from an interrupt
   // remember not to make any DOS or BIOS calls from here
   // also don't call any C library functions unless you are sure that
   // they are reentrant
   // restore our DS register

   if ( VoiceStatus[ voice ].playing == FALSE )
      {
//      DB_printf( "GUS Voice %d not playing.\n", voice );
      DB_printf( "GUS Voice " );
      DB_PrintNum( voice );
      DB_printf( " not playing.\n" );
      return( DIG_DONE );
         }

      if ( reason == DIG_MORE_DATA )
         {
         Voice = VoiceStatus[ voice ].Voice;

//         DB_printf( "Voice %d : More data -- ", Voice );
         DB_printf( "Voice " );
         DB_PrintNum( voice );
         DB_printf( " : More data -- " );
         if ( Voice != NULL )
            {
            if ( Voice->Playing )
               {
               GUSWAVE_GetNextVOCBlock( Voice );
               if ( Voice->Playing )
                  {
//                  DB_printf( "More data -- size = %u blocklength = %u\n",
//                     Voice->length, Voice->BlockLength );
                  DB_printf( "More data -- size = " );
                  DB_PrintNum( Voice->length );
                  DB_printf( " blocklength = " );
                  DB_PrintNum( Voice->BlockLength );
                  DB_printf( "\n" );
                  *buf  = Voice->sound;
                  *size = Voice->length;
                  return( DIG_MORE_DATA );
                  }
               else
                  {
                  DB_printf( "Voice done.\n" );
                  }
               }
            else
               {
               DB_printf( "Voice not active.\n" );
               }
            }
         else
            {
            DB_printf( " NULL Voice\n" );
            }

         return( DIG_DONE );
         }

      if ( reason == DIG_DONE )
         {
         VoiceStatus[ voice ].playing = FALSE;
         Voice = VoiceStatus[ voice ].Voice;
//         DB_printf( "Voice %d : Done -- ", Voice );
         DB_printf( "Voice " );
         DB_PrintNum( voice );
         DB_printf( " : Done -- " );

         if ( Voice != NULL )
            {
            DB_printf( "Ok\n" );

            Voice->Active   = FALSE;
            Voice->Playing  = FALSE;

// I'm commenting this out because a -1 could cause a crash if it
// is sent to the GF1 code.  This shouldn't be necessary since
// Active should be false when GF1voice is -1, but this is just
// a precaution.  Adjust the pan on the wrong voice is a lot
// more pleasant than a crash!
//         Voice->GF1voice = -1;

         LL_Remove( VoiceNode, &VoiceList, Voice );
         LL_AddToTail( VoiceNode, &VoicePool, Voice );
         }
      else
         {
         DB_printf( "Null voice\n" );
         }

      if ( GUSWAVE_CallBackFunc )
         {
         GUSWAVE_CallBackFunc( Voice->callbackval );
         }
      }

   return( DIG_DONE );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetVoice

   Locates the voice with the specified handle.
---------------------------------------------------------------------*/

static VoiceNode *GUSWAVE_GetVoice
   (
   int handle
   )

   {
   VoiceNode *voice;
   unsigned  flags;

   flags = DisableInterrupts();

   voice = VoiceList.start;

   while( voice != NULL )
      {
      if ( handle == voice->handle )
         {
         break;
         }

      voice = voice->next;
      }

   RestoreInterrupts( flags );

   if ( voice == NULL )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_VoiceNotFound );
      }

   return( voice );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_VoicePlaying

   Checks if the voice associated with the specified handle is
   playing.
---------------------------------------------------------------------*/

int GUSWAVE_VoicePlaying
   (
   int handle
   )

   {
   VoiceNode   *voice;

   voice = GUSWAVE_GetVoice( handle );
   if ( voice != NULL )
      {
      return( voice->Active );
      }

   return( FALSE );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_VoicesPlaying

   Determines the number of currently active voices.
---------------------------------------------------------------------*/

int GUSWAVE_VoicesPlaying
   (
   void
   )

   {
   int         index;
   int         NumVoices = 0;
   unsigned    flags;

   flags = DisableInterrupts();

   for( index = 0; index < GUSWAVE_MaxVoices; index++ )
      {
      if ( GUSWAVE_Voices[ index ].Active )
         {
         NumVoices++;
         }
      }

   RestoreInterrupts( flags );

   if ( GUS_Debug )
      {
      DB_printf( "Number of voices = %d.\n", NumVoices );
      }

   return( NumVoices );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_Kill

   Stops output of the voice associated with the specified handle.
---------------------------------------------------------------------*/

int GUSWAVE_Kill
   (
   int handle
   )

   {
   VoiceNode *voice;
   unsigned  flags;

   flags = DisableInterrupts();

   voice = GUSWAVE_GetVoice( handle );

   if ( voice == NULL )
      {
      RestoreInterrupts( flags );
      GUSWAVE_SetErrorCode( GUSWAVE_VoiceNotFound );

      if ( GUS_Debug )
         {
         DB_printf( "Could not find voice to kill.\n" );
         }

      return( GUSWAVE_Warning );
      }

   RestoreInterrupts( flags );

   if ( !GUS_Debug )
      {
      if ( voice->Active )
         {
         gf1_stop_digital( voice->GF1voice );
         }
      }
   else
      {
      DB_printf( "Kill - GUS Voice %d ", voice->GF1voice );
      if ( voice->Active )
         {
         DB_printf( "active\n" );
         gf1_stop_digital( voice->GF1voice );
         }
      else
         {
         DB_printf( "inactive\n" );
         }
      }

//   RestoreInterrupts( flags );

   return( GUSWAVE_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_KillAllVoices

   Stops output of all currently active voices.
---------------------------------------------------------------------*/

int GUSWAVE_KillAllVoices
   (
   void
   )

   {
   int i;
   unsigned  flags;

   if ( !GUSWAVE_Installed )
      {
      return( GUSWAVE_Ok );
      }

   if ( GUS_Debug )
      {
      DB_printf( "Kill All Voices\n" );
      }

   flags = DisableInterrupts();

   // Remove all the voices from the list
   for( i = 0; i < GUSWAVE_MaxVoices; i++ )
      {
      if ( GUSWAVE_Voices[ i ].Active )
         {
//         GUSWAVE_Kill( GUSWAVE_Voices[ i ].handle );

         gf1_stop_digital( GUSWAVE_Voices[ i ].GF1voice );
         }
      }

   for( i = 0; i < MAX_VOICES; i++ )
      {
      VoiceStatus[ i ].playing = FALSE;
      VoiceStatus[ i ].Voice   = NULL;
      }

   VoicePool.start = NULL;
   VoicePool.end   = NULL;
   VoiceList.start = NULL;
   VoiceList.end   = NULL;

   for( i = 0; i < GUSWAVE_MaxVoices; i++ )
      {
      GUSWAVE_Voices[ i ].Active = FALSE;
      if ( GUSWAVE_Voices[ i ].mem != NULL )
         {
         LL_AddToTail( VoiceNode, &VoicePool, &GUSWAVE_Voices[ i ] );
         }
      }

   RestoreInterrupts( flags );

   return( GUSWAVE_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_SetPitch

   Sets the pitch for the voice associated with the specified handle.
---------------------------------------------------------------------*/

int GUSWAVE_SetPitch
   (
   int handle,
   int pitchoffset
   )

   {
   VoiceNode *voice;
   unsigned  flags;

   flags = DisableInterrupts();

   voice = GUSWAVE_GetVoice( handle );

   if ( voice == NULL )
      {
      RestoreInterrupts( flags );

      GUSWAVE_SetErrorCode( GUSWAVE_VoiceNotFound );
      return( GUSWAVE_Warning );
      }

   if ( voice->Active )
      {
      voice->PitchScale  = PITCH_GetScale( pitchoffset );
      voice->RateScale   = ( voice->SamplingRate * voice->PitchScale ) >> 16;
      gf1_dig_set_freq( voice->GF1voice, voice->RateScale );
      }

   RestoreInterrupts( flags );

   return( GUSWAVE_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_SetPan3D

   Sets the pan position of the voice with the specified handle.
---------------------------------------------------------------------*/

int GUSWAVE_SetPan3D
   (
   int handle,
   int angle,
   int distance
   )

   {
   VoiceNode *voice;
   int        pan;
   unsigned  flags;

   flags = DisableInterrupts();

   voice = GUSWAVE_GetVoice( handle );

   if ( voice == NULL )
      {
      RestoreInterrupts( flags );

      GUSWAVE_SetErrorCode( GUSWAVE_VoiceNotFound );
      return( GUSWAVE_Warning );
      }

   if ( voice->Active )
      {
      angle &= 31;

      pan = GUSWAVE_PanTable[ angle ];
      if ( GUSWAVE_SwapLeftRight )
         {
         pan = 15 - pan;
         }

      distance = max( 0, distance );
      distance = min( 255, distance );

      voice->Volume = 255 - distance;
      voice->Pan    = pan;

      gf1_dig_set_pan( voice->GF1voice, pan );
      gf1_dig_set_vol( voice->GF1voice, GUSWAVE_Volume - distance * 4 );
      }

   RestoreInterrupts( flags );

   return( GUSWAVE_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_SetVolume

   Sets the total volume of the digitized sounds.
---------------------------------------------------------------------*/

void GUSWAVE_SetVolume
   (
   int volume
   )

   {
   int i;

   volume = max( 0, volume );
   volume = min( 255, volume );
   GUSWAVE_Volume = MAX_VOLUME - ( 255 - volume ) * 4;

   for( i = 0; i < GUSWAVE_MaxVoices; i++ )
      {
      if ( GUSWAVE_Voices[ i ].Active )
         {
         gf1_dig_set_vol( GUSWAVE_Voices[ i ].GF1voice,
            GUSWAVE_Volume - ( 255 - GUSWAVE_Voices[ i ].Volume ) * 4 );
         }
      }
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetVolume

   Returns the total volume of the digitized sounds.
---------------------------------------------------------------------*/

int GUSWAVE_GetVolume
   (
   void
   )

   {
   return( 255 - ( ( MAX_VOLUME - GUSWAVE_Volume ) / 4 ) );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_AllocVoice

   Retrieve an inactive or lower priority voice for output.
---------------------------------------------------------------------*/

static VoiceNode *GUSWAVE_AllocVoice
   (
   int priority
   )

   {
   VoiceNode   *voice;
   VoiceNode   *node;
   unsigned    flags;

   // If we don't have any free voices, check if we have a higher
   // priority than one that is playing.
   if ( GUSWAVE_VoicesPlaying() >= GUSWAVE_MaxVoices )
      {
      flags = DisableInterrupts();

      node = VoiceList.start;
      voice = node;
      while( node != NULL )
         {
         if ( node->priority < voice->priority )
            {
            voice = node;
            }

         node = node->next;
         }

      RestoreInterrupts( flags );

      if ( priority >= voice->priority )
         {
         GUSWAVE_Kill( voice->handle );
         }
      }

   // Check if any voices are in the voice pool
   flags = DisableInterrupts();

   voice = VoicePool.start;
   if ( voice != NULL )
      {
      LL_Remove( VoiceNode, &VoicePool, voice );
      }

   RestoreInterrupts( flags );

   if ( voice != NULL )
      {
      do
         {
         GUSWAVE_VoiceHandle++;
         if ( GUSWAVE_VoiceHandle < GUSWAVE_MinVoiceHandle )
            {
            GUSWAVE_VoiceHandle = GUSWAVE_MinVoiceHandle;
            }
         }
      while( GUSWAVE_VoicePlaying( GUSWAVE_VoiceHandle ) );

      voice->handle = GUSWAVE_VoiceHandle;
      }

   return( voice );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_VoiceAvailable

   Checks if a voice can be play at the specified priority.
---------------------------------------------------------------------*/

int GUSWAVE_VoiceAvailable
   (
   int priority
   )

   {
   VoiceNode   *voice;
   VoiceNode   *node;
   unsigned    flags;

   if ( GUSWAVE_VoicesPlaying() < GUSWAVE_MaxVoices )
      {
      return( TRUE );
      }

   flags = DisableInterrupts();

   node = VoiceList.start;
   voice = node;
   while( node != NULL )
      {
      if ( node->priority < voice->priority )
         {
         voice = node;
         }

      node = node->next;
      }

   RestoreInterrupts( flags );

   if ( priority >= voice->priority )
      {
      return( TRUE );
      }

   return( FALSE );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetNextVOCBlock

   Interperate the information of a VOC format sound file.
---------------------------------------------------------------------*/

playbackstatus GUSWAVE_GetNextVOCBlock
   (
   VoiceNode *voice
   )

   {
   unsigned char *ptr;
   int            blocktype;
   int            lastblocktype;
   unsigned long  blocklength;
   unsigned long  samplespeed;
   unsigned int   tc;
   int            packtype;
   int            voicemode;
   int            done;
   unsigned       BitsPerSample;
   unsigned       Channels;
   unsigned       Format;

   if ( voice->BlockLength > 0 )
      {
      voice->sound       += MAX_BLOCK_LENGTH;
      voice->length       = min( voice->BlockLength, MAX_BLOCK_LENGTH );
      voice->BlockLength -= voice->length;
      return( KeepPlaying );
      }

   ptr = ( unsigned char * )voice->NextBlock;

   voice->Playing = TRUE;

   voicemode = 0;
   lastblocktype = 0;
   packtype = 0;

   done = FALSE;
   while( !done )
      {
      // Stop playing if we get a NULL pointer
      if ( ptr == NULL )
         {
         voice->Playing = FALSE;
         done = TRUE;
         break;
         }

      blocktype = ( int )*ptr;
      blocklength = ( *( unsigned long * )( ptr + 1 ) ) & 0x00ffffff;
      ptr += 4;

      switch( blocktype )
         {
         case 0 :
            // End of data
            voice->Playing = FALSE;
            done = TRUE;
            break;

         case 1 :
            // Sound data block
            voice->bits = 8;
            if ( lastblocktype != 8 )
               {
               tc = ( unsigned int )*ptr << 8;
               packtype = *( ptr + 1 );
               }

            ptr += 2;
            blocklength -= 2;

            samplespeed = 256000000L / ( 65536 - tc );

            // Skip packed or stereo data
            if ( ( packtype != 0 ) || ( voicemode != 0 ) )
               {
               ptr += blocklength;
               }
            else
               {
               done = TRUE;
               }
            voicemode = 0;
            break;

         case 2 :
            // Sound continuation block
            samplespeed = voice->SamplingRate;
            done = TRUE;
            break;

         case 3 :
            // Silence
            // Not implimented.
            ptr += blocklength;
            break;

         case 4 :
            // Marker
            // Not implimented.
            ptr += blocklength;
            break;

         case 5 :
            // ASCII string
            // Not implimented.
            ptr += blocklength;
            break;

         case 6 :
            // Repeat begin
            voice->LoopCount = *( unsigned short * )ptr;
            ptr += blocklength;
            voice->LoopStart = ptr;
            break;

         case 7 :
            // Repeat end
            ptr += blocklength;
            if ( lastblocktype == 6 )
               {
               voice->LoopCount = 0;
               }
            else
               {
               if ( ( voice->LoopCount > 0 ) && ( voice->LoopStart != NULL ) )
                  {
                  ptr = voice->LoopStart;
                  if ( voice->LoopCount < 0xffff )
                     {
                     voice->LoopCount--;
                     if ( voice->LoopCount == 0 )
                        {
                        voice->LoopStart = NULL;
                        }
                     }
                  }
               }
            break;

         case 8 :
            // Extended block
            voice->bits = 8;
            tc = *( unsigned short * )ptr;
            packtype = *( ptr + 2 );
            voicemode = *( ptr + 3 );
            ptr += blocklength;
            break;

         case 9 :
            // New sound data block
            samplespeed = *( unsigned long * )ptr;
            BitsPerSample = ( unsigned )*( ptr + 4 );
            Channels = ( unsigned )*( ptr + 5 );
            Format = ( unsigned )*( unsigned short * )( ptr + 6 );

            if ( ( BitsPerSample == 8 ) && ( Channels == 1 ) &&
               ( Format == VOC_8BIT ) )
               {
               ptr += 12;
               blocklength -= 12;
               voice->bits  = 8;
               done = TRUE;
               }
            else if ( ( BitsPerSample == 16 ) && ( Channels == 1 ) &&
               ( Format == VOC_16BIT ) )
               {
               ptr         += 12;
               blocklength -= 12;
               voice->bits  = 16;
               done         = TRUE;
               }
            else
               {
               ptr += blocklength;
               }
            break;

         default :
            // Unknown data.  Probably not a VOC file.
            voice->Playing = FALSE;
            done = TRUE;
            break;
         }

      lastblocktype = blocktype;
      }

   if ( voice->Playing )
      {
      voice->NextBlock    = ptr + blocklength;
      voice->sound        = ptr;

      voice->SamplingRate = samplespeed;
      voice->RateScale    = ( voice->SamplingRate * voice->PitchScale ) >> 16;

      voice->length       = min( blocklength, MAX_BLOCK_LENGTH );
      voice->BlockLength  = blocklength - voice->length;

      return( KeepPlaying );
      }

   return( SoundDone );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetNextWAVBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

playbackstatus GUSWAVE_GetNextWAVBlock
   (
   VoiceNode *voice
   )

   {
   if ( voice->BlockLength <= 0 )
      {
      if ( voice->LoopStart == NULL )
         {
         voice->Playing = FALSE;
         return( SoundDone );
         }

      voice->BlockLength = voice->LoopSize;
      voice->NextBlock   = voice->LoopStart;
      voice->length      = 0;
      }

   voice->sound        = voice->NextBlock;
   voice->length       = min( voice->BlockLength, 0x8000 );
   voice->NextBlock   += voice->length;
   voice->BlockLength -= voice->length;

   return( KeepPlaying );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetNextDemandFeedBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

playbackstatus GUSWAVE_GetNextDemandFeedBlock
   (
   VoiceNode *voice
   )

   {
   if ( voice->BlockLength > 0 )
      {
      voice->sound       += voice->length;
      voice->length       = min( voice->BlockLength, 0x8000 );
      voice->BlockLength -= voice->length;

      return( KeepPlaying );
      }

   if ( voice->DemandFeed == NULL )
      {
      return( SoundDone );
      }

   ( voice->DemandFeed )( &voice->sound, &voice->BlockLength );
//   voice->sound = GUS_Silence16;
//   voice->BlockLength = 256;

   voice->length       = min( voice->BlockLength, 0x8000 );
   voice->BlockLength -= voice->length;

   if ( ( voice->length > 0 ) && ( voice->sound != NULL ) )
      {
      return( KeepPlaying );
      }
   return( NoMoreData );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_Play

   Begins playback of digitized sound.
---------------------------------------------------------------------*/

int GUSWAVE_Play
   (
   VoiceNode *voice,
   int angle,
   int volume,
   int channels
   )

   {
   int      VoiceNumber;
   int      type;
   int      pan;
   unsigned flags;
   int ( *servicefunction )( int reason, int voice, unsigned char **buf, unsigned long *size );

   type = 0;
   if ( channels != 1 )
      {
      type |= TYPE_STEREO;
      }

   if ( voice->bits == 8 )
      {
      type |= TYPE_8BIT;
      type |= TYPE_INVERT_MSB;
      }

   voice->GF1voice  = -1;

   angle &= 31;
   pan = GUSWAVE_PanTable[ angle ];
   if ( GUSWAVE_SwapLeftRight )
      {
      pan = 15 - pan;
      }

   voice->Pan = pan;

   volume = max( 0, volume );
   volume = min( 255, volume );
   voice->Volume = volume;

   if ( !GUS_Debug )
      {
      servicefunction = GUSWAVE_CallBack;
      }
   else
      {
      servicefunction = GUSWAVE_DebugCallBack;
      }

   VoiceNumber = gf1_play_digital( 0, voice->sound, voice->length,
      voice->mem, GUSWAVE_Volume - ( 255 - volume ) * 4, pan,
      voice->RateScale, type, &GUS_HoldBuffer, servicefunction );

   if ( VoiceNumber == NO_MORE_VOICES )
      {
      if ( GUS_Debug )
         {
         DB_printf( "Out of voices.\n" );
         }

      flags = DisableInterrupts();
      LL_AddToTail( VoiceNode, &VoicePool, voice );
      RestoreInterrupts( flags );

      GUSWAVE_SetErrorCode( GUSWAVE_NoVoices );
      return( GUSWAVE_Warning );
      }

   flags = DisableInterrupts();
   voice->GF1voice = VoiceNumber;
   voice->Active   = TRUE;
   LL_AddToTail( VoiceNode, &VoiceList, voice );
   VoiceStatus[ VoiceNumber ].playing = TRUE;
   VoiceStatus[ VoiceNumber ].Voice   = voice;

   if ( GUS_Debug )
      {
      DB_printf( "GUS voice %d playing\n", VoiceNumber );
      }

   RestoreInterrupts( flags );

   return( voice->handle );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_PlayVOC

   Begins playback of digitized sound.
---------------------------------------------------------------------*/

int GUSWAVE_PlayVOC
   (
   char *sample,
   int   pitchoffset,
   int   angle,
   int   volume,
   int   priority,
   unsigned long callbackval
   )

   {
   int handle;
   int status;
   playbackstatus soundstatus;
   VoiceNode *voice;
   unsigned flags;

   // Make sure it's a valid VOC file.
   status = strncmp( sample, "Creative Voice File", 19 );
   if ( status != 0 )
      {
      // Tell multivoc that we had a bad VOC file
      MV_ErrorCode = MV_InvalidVOCFile;

      GUSWAVE_SetErrorCode( GUSWAVE_InvalidVOCFile );
      return( GUSWAVE_Error );
      }

   // Request a voice from the voice pool
   voice = GUSWAVE_AllocVoice( priority );
   if ( voice == NULL )
      {
      if ( GUS_Debug )
         {
         DB_printf( "No more voices.  Skipping sound.\n" );
         }
      GUSWAVE_SetErrorCode( GUSWAVE_NoVoices );
      return( GUSWAVE_Warning );
      }

   voice->NextBlock   = sample + *( unsigned short int * )( sample + 0x14 );
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->BlockLength = 0;
   voice->PitchScale  = PITCH_GetScale( pitchoffset );
   voice->wavetype    = VOC;
   voice->bits        = 8;
   voice->GetSound    = GUSWAVE_GetNextVOCBlock;
   voice->length      = 0;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->callbackval = callbackval;

   soundstatus = GUSWAVE_GetNextVOCBlock( voice );
   if ( soundstatus == SoundDone )
      {
      flags = DisableInterrupts();
      LL_AddToTail( VoiceNode, &VoicePool, voice );
      RestoreInterrupts( flags );

      if ( GUS_Debug )
         {
         DB_printf( "Voice ended before playback.\n" );
         }

      // Tell multivoc that we had a bad VOC file
      MV_ErrorCode = MV_InvalidVOCFile;

      GUSWAVE_SetErrorCode( GUSWAVE_InvalidVOCFile );
      return( GUSWAVE_Error );
      }

   handle = GUSWAVE_Play( voice, angle, volume, 1 );
   return( handle );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_PlayWAV

   Begins playback of digitized sound.
---------------------------------------------------------------------*/

int GUSWAVE_PlayWAV
   (
   char *sample,
   int   pitchoffset,
   int   angle,
   int   volume,
   int   priority,
   unsigned long callbackval
   )

   {
   VoiceNode *voice;
   int        handle;
   int        channels;
   int        bits;
   int        length;
   riff_header   *riff;
   format_header *format;
   data_header   *data;

   riff = ( riff_header * )sample;

   if ( ( strncmp( riff->RIFF, "RIFF", 4 ) != 0 ) ||
      ( strncmp( riff->WAVE, "WAVE", 4 ) != 0 ) ||
      ( strncmp( riff->fmt, "fmt ", 4) != 0 ) )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_InvalidWAVFile );
      return( GUSWAVE_Error );
      }

   format = ( format_header * )( riff + 1 );
   data   = ( data_header * )( ( ( char * )format ) + riff->format_size );

   if ( format->wFormatTag != 1 )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_InvalidWAVFile );
      return( GUSWAVE_Error );
      }

   channels = format->nChannels;
   if ( ( channels != 1 ) && ( channels != 2 ) )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_InvalidWAVFile );
      return( GUSWAVE_Error );
      }

   bits = format->nBitsPerSample;
   if ( ( bits != 8 ) && ( bits != 16 ) )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_InvalidWAVFile );
      return( GUSWAVE_Error );
      }

   if ( strncmp( data->DATA, "data", 4 ) != 0 )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_InvalidWAVFile );
      return( GUSWAVE_Error );
      }

   // Request a voice from the voice pool
   voice = GUSWAVE_AllocVoice( priority );
   if ( voice == NULL )
      {
      if ( GUS_Debug )
         {
         DB_printf( "No more voices.  Skipping sound.\n" );
         }
      GUSWAVE_SetErrorCode( GUSWAVE_NoVoices );
      return( GUSWAVE_Warning );
      }

   voice->wavetype    = WAV;
   voice->bits        = bits;
   voice->GetSound    = GUSWAVE_GetNextWAVBlock;

   length = data->size;

   voice->Playing     = TRUE;
   voice->DemandFeed  = NULL;
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->length      = min( length, 0x8000 );
   voice->BlockLength = length - voice->length;//  min( loopend + 1, data->size );
   voice->sound       = ( char * )( data + 1 );
   voice->NextBlock   = voice->sound + voice->length;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->callbackval = callbackval;
   voice->LoopStart   = NULL;// voice->NextBlock + loopstart;
   voice->LoopEnd     = NULL;//voice->NextBlock + min( loopend, data->size - 1 );
   voice->LoopSize    = 0;//( voice->LoopEnd - voice->LoopStart ) + 1;
   voice->PitchScale  = PITCH_GetScale( pitchoffset );
   voice->SamplingRate = format->nSamplesPerSec;
   voice->RateScale   = ( voice->SamplingRate * voice->PitchScale ) >> 16;

   handle = GUSWAVE_Play( voice, angle, volume, channels );

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_StartDemandFeedPlayback

   Begins playback of digitized sound.
---------------------------------------------------------------------*/

int GUSWAVE_StartDemandFeedPlayback
   (
   void ( *function )( char **ptr, unsigned long *length ),
   int   channels,
   int   bits,
   int   rate,
   int   pitchoffset,
   int   angle,
   int   volume,
   int   priority,
   unsigned long callbackval
   )

   {
   VoiceNode *voice;
   int        handle;

   // Request a voice from the voice pool
   voice = GUSWAVE_AllocVoice( priority );
   if ( voice == NULL )
      {
      if ( GUS_Debug )
         {
         DB_printf( "No more voices.  Skipping sound.\n" );
         }
      GUSWAVE_SetErrorCode( GUSWAVE_NoVoices );
      return( GUSWAVE_Warning );
      }

   voice->wavetype    = DemandFeed;
   voice->bits        = bits;
   voice->GetSound    = GUSWAVE_GetNextDemandFeedBlock;
   voice->Playing     = TRUE;
   voice->DemandFeed  = function;
   voice->LoopStart   = NULL;
   voice->LoopCount   = 0;
   voice->BlockLength = 0;
   voice->length      = 256;
   voice->sound       = ( bits == 8 ) ? GUS_Silence8 : ( char * )GUS_Silence16;
   voice->NextBlock   = NULL;
   voice->next        = NULL;
   voice->prev        = NULL;
   voice->priority    = priority;
   voice->callbackval = callbackval;
   voice->PitchScale  = PITCH_GetScale( pitchoffset );
   voice->SamplingRate = rate;
   voice->RateScale   = ( voice->SamplingRate * voice->PitchScale ) >> 16;

   handle = GUSWAVE_Play( voice, angle, volume, channels );

   return( handle );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_SetReverseStereo

   Set the orientation of the left and right channels.
---------------------------------------------------------------------*/

void GUSWAVE_SetReverseStereo
   (
   int setting
   )

   {
   GUSWAVE_SwapLeftRight = setting;
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_GetReverseStereo

   Returns the orientation of the left and right channels.
---------------------------------------------------------------------*/

int GUSWAVE_GetReverseStereo
   (
   void
   )

   {
   return( GUSWAVE_SwapLeftRight );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_InitVoices

   Begins playback of digitized sound.
---------------------------------------------------------------------*/

static int GUSWAVE_InitVoices
   (
   void
   )

   {
   int i;

   for( i = 0; i < MAX_VOICES; i++ )
      {
      VoiceStatus[ i ].playing = FALSE;
      VoiceStatus[ i ].Voice   = NULL;
      }

   VoicePool.start = NULL;
   VoicePool.end   = NULL;
   VoiceList.start = NULL;
   VoiceList.end   = NULL;

   for( i = 0; i < VOICES; i++ )
      {
      GUSWAVE_Voices[ i ].num      = -1;
      GUSWAVE_Voices[ i ].Active   = FALSE;
      GUSWAVE_Voices[ i ].GF1voice = -1;
      GUSWAVE_Voices[ i ].mem      = NULL;
      }

   for( i = 0; i < VOICES; i++ )
      {
      GUSWAVE_Voices[ i ].num      = i;
      GUSWAVE_Voices[ i ].Active   = FALSE;
      GUSWAVE_Voices[ i ].GF1voice = 0;

      GUSWAVE_Voices[ i ].mem = gf1_malloc( GF1BSIZE );
      if ( GUSWAVE_Voices[ i ].mem == NULL )
         {
         GUSWAVE_MaxVoices = i;
         if ( i < 1 )
            {
            if ( GUSMIDI_Installed )
               {
               GUSWAVE_SetErrorCode( GUSWAVE_UltraNoMemMIDI );
               }
            else
               {
               GUSWAVE_SetErrorCode( GUSWAVE_UltraNoMem );
               }
            return( GUSWAVE_Error );
            }

         break;
         }

      LL_AddToTail( VoiceNode, &VoicePool, &GUSWAVE_Voices[ i ] );
      }

   return( GUSWAVE_Ok );
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_SetCallBack

   Set the function to call when a voice stops.
---------------------------------------------------------------------*/

void GUSWAVE_SetCallBack
   (
   void ( *function )( unsigned long )
   )

   {
   GUSWAVE_CallBackFunc = function;
   }


/*---------------------------------------------------------------------
   Function: GUSWAVE_Init

   Initializes the Gravis Ultrasound for digitized sound playback.
---------------------------------------------------------------------*/

int GUSWAVE_Init
   (
   int numvoices
   )

   {
   int status;

   if ( GUSWAVE_Installed )
      {
      GUSWAVE_Shutdown();
      }

   GUSWAVE_SetErrorCode( GUSWAVE_Ok );

   status = GUS_Init();
   if ( status != GUS_Ok )
      {
      GUSWAVE_SetErrorCode( GUSWAVE_GUSError );
      return( GUSWAVE_Error );
      }

   GUS_Debug = USER_CheckParameter( "DEBUGGUS" );

   GUSWAVE_MaxVoices = min( numvoices, VOICES );
   GUSWAVE_MaxVoices = max( GUSWAVE_MaxVoices, 0 );

   status = GUSWAVE_InitVoices();
   if ( status != GUSWAVE_Ok )
      {
      GUS_Shutdown();
      return( status );
      }

   GUSWAVE_SetReverseStereo( FALSE );

   GUSWAVE_CallBackFunc = NULL;
   GUSWAVE_Installed = TRUE;

   return( GUSWAVE_Ok );
   }
