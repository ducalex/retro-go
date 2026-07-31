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

#include "rt_def.h"
#ifdef DOS
#include <mem.h>
#endif

#include "rt_util.h"
#include "rt_sound.h"
#include "rt_net.h"
#include "rt_dmand.h"
#include "_rt_dman.h"
#include "fx_man.h"
#include "develop.h"
//MED
#include "memcheck.h"

static boolean Recording=false;
static boolean Feeder=false;
static byte * RecordingBuffer;
static int Playingvoice;
static int RecordingPointer;
static int FeederPointer;
static boolean Playback=false;
static boolean Playing=false;
static byte * PlaybackBuffer;
static int PlaybackPointer;
static int PlayingPointer;
static boolean RecordingSemaphore=false;

//#define FX_StartDemandFeedPlayback MV_StartDemandFeedPlayback
//#define FX_StartRecording          MV_StartRecording
//#define FX_StopRecord              MV_StopRecord
//#include "multivoc.h"

//***************************************************************************
//
// SD_UpdatePlaybackSound - Update playback of a sound in chunks
//
//***************************************************************************
void SD_UpdatePlaybackSound ( char ** ptr, unsigned long * length )
   {
   if ( Playing==false )
      {
      *ptr = NULL;
      *length = 0;
      return;
      }
   if (PlayingPointer==PlaybackPointer)
      {
      *ptr = NULL;
      *length = 0;
      if (Playback==false)
         {
         FX_StopSound( Playingvoice );
         SafeFree ( PlaybackBuffer );
         Playing=false;
         }
      return;
      }

   *length=PLAYBACKDELTASIZE;

   if (PlayingPointer==-1)
      {
      *ptr = NULL;
      *length = 0;
      return;
      }

   *ptr=&PlaybackBuffer[PlayingPointer];

   PlayingPointer = (PlayingPointer + *length) &
                      (PLAYBACKBUFFERSIZE - 1);
   }

//***************************************************************************
//
// SD_StartIncomingSound - Setup to receive an incoming sound in chunks
//
//***************************************************************************

void SD_StartIncomingSound ( void )
{
   if (SD_Started==false)
      return;
   if ( ( Recording==true ) || ( Playback==true ) )
      {
      return;
      }

   Playback=true;
   PlaybackBuffer = SafeMalloc (PLAYBACKBUFFERSIZE);
   Playing = false;
   PlayingPointer = -1;
   PlaybackPointer = 0;

   Playingvoice = FX_StartDemandFeedPlayback ( SD_UpdatePlaybackSound,
                  RECORDINGSAMPLERATE,
                  0, 255, 255, 255, 255, -1);
   if (Playingvoice==0)
      {
      SafeFree(PlaybackBuffer);
      Playback=false;
      }
}

//***************************************************************************
//
// SD_StopIncomingSound - Stop receiving an incoming sound and playback
//
//***************************************************************************

void SD_StopIncomingSound ( void )
{
   if (SD_Started==false)
      return;
   Playback=false;
}


//***************************************************************************
//
// SD_UpdateIncomingSound - Update an incoming sound
//
//***************************************************************************

void SD_UpdateIncomingSound ( byte * ptr, word length )
{
   int amount;

   if (SD_Started==false)
      return;

   if ( Playback==false )
      {
      return;
      }
   amount=length;
   if (PlaybackPointer+length > PLAYBACKBUFFERSIZE)
      amount=PLAYBACKBUFFERSIZE-PlaybackPointer;
   memcpy ( &PlaybackBuffer[PlaybackPointer],
            ptr, amount);
   PlaybackPointer = (PlaybackPointer + amount) &
                      (PLAYBACKBUFFERSIZE - 1);

   ptr+=amount;

   if (length!=amount)
      {
      amount=length-amount;
      memcpy ( &PlaybackBuffer[PlaybackPointer],
               ptr, amount);
      PlaybackPointer = (PlaybackPointer + amount) &
                         (PLAYBACKBUFFERSIZE - 1);
      }

   if (PlayingPointer==-1)
      {
      Playing=true;
      PlayingPointer=0;
      }
   if (PlaybackPointer==PlayingPointer)
      {
      Playback=false;
      }
}

//***************************************************************************
//
// SD_UpdateRecordingSound - Update recording a sound in chunks
//
//***************************************************************************
extern int whereami;
void SD_UpdateRecordingSound ( char * ptr, int length )
   {
   int amount;

   whereami = 69;
   if ( Recording==false )
      {
      return;
      }
   whereami = 70;
   amount=length;
   if (RecordingPointer+length > RECORDINGBUFFERSIZE)
      amount=RECORDINGBUFFERSIZE-RecordingPointer;
   memcpy ( &RecordingBuffer[RecordingPointer],
            ptr, amount);
   whereami = 71;
   RecordingPointer = (RecordingPointer + amount) &
                      (RECORDINGBUFFERSIZE - 1);

   if (length!=amount)
      {
      ptr += amount;
      amount=length-amount;
      memcpy ( &RecordingBuffer[RecordingPointer],
               ptr, amount);
      RecordingPointer = (RecordingPointer + amount) &
                         (RECORDINGBUFFERSIZE - 1);
      }
   whereami = 72;
   if (Feeder == false)
      {
      Feeder = true;
      }

   whereami = 73;
   if (RecordingPointer==FeederPointer)
      {
      Recording=false;
      }
   whereami = 74;
   }

//***************************************************************************
//
// SD_StartRecordingSound - Start recording a sound in chunks
//
//***************************************************************************

boolean SD_StartRecordingSound ( void )
{
   int status;

   if (SD_Started==false)
      return false;
   if (remoteridicule == false)
      return false;
   if ( ( Recording==true ) || ( Playback==true ) || (Feeder==true))
      {
      return false;
      }
   Recording=true;
   RecordingBuffer = SafeMalloc (RECORDINGBUFFERSIZE);
   Feeder = false;
   FeederPointer = -1;
   RecordingPointer = 0;

   status=FX_StartRecording( RECORDINGSAMPLERATE, SD_UpdateRecordingSound);

   if (status!=FX_Ok)
      {
      Recording=false;
      SafeFree(RecordingBuffer);
      return false;
      }

   return true;
}

//***************************************************************************
//
// SD_StopRecordingSound - Stop recording a sound
//
//***************************************************************************

void SD_StopRecordingSound ( void )
{
   if (SD_Started==false)
      return;
   if (Recording == true)
      {
      FX_StopRecord();
      Recording=false;
      }
}

//***************************************************************************
//
// SD_SetRecordingActive - Set the recording active flag
//
//***************************************************************************

void SD_SetRecordingActive ( void )
{
   RecordingSemaphore=true;
}

//***************************************************************************
//
// SD_ClearRecordingActive - Clear the recording active flag
//
//***************************************************************************

void SD_ClearRecordingActive ( void )
{
   RecordingSemaphore=false;
}

//***************************************************************************
//
// SD_RecordingActive - Check if recording is active on some system
//
//***************************************************************************

boolean SD_RecordingActive ( void )
{
   return RecordingSemaphore;
}

//***************************************************************************
//
// SD_GetSoundData - Returns next piece of sound data, returns:
//
//                   nodata if no sound data is ready
//                   newsound if it is the start of a new sound
//                            data is also returned;
//                   endsound if the sound is finished
//                   data if data is ready
//
//***************************************************************************

recordstate SD_GetSoundData ( byte * data, word length )
{
   recordstate status=rs_data;
   int amount;

   if (SD_Started==false)
      return rs_nodata;

   if (Feeder==false)
      return rs_nodata;

   if (FeederPointer==RecordingPointer)
      {
      if (Recording==false)
         {
         SafeFree(RecordingBuffer);
         Feeder=false;
         return rs_endsound;
         }
      else
         {
         return rs_nodata;
         }
      }

   if (FeederPointer==-1)
      {
      status=rs_newsound;
      FeederPointer=0;
      }

   amount=length;

   if (FeederPointer+length > RECORDINGBUFFERSIZE)
      amount=RECORDINGBUFFERSIZE-FeederPointer;
   memcpy ( data, &RecordingBuffer[FeederPointer], amount);

   FeederPointer = (FeederPointer + amount) &
                      (RECORDINGBUFFERSIZE - 1);

   data += amount;

   if (length!=amount)
      {
      amount=length-amount;
      memcpy ( data, &RecordingBuffer[FeederPointer], amount);
      FeederPointer = (FeederPointer + amount) &
                        (RECORDINGBUFFERSIZE - 1);
      }

   return status;
}

//***************************************************************************
//
// SD_SoundDataReady - Returns true if data is ready
//
//***************************************************************************

boolean SD_SoundDataReady ( void )
{
   if (SD_Started==false)
      return false;
   return Feeder;
}



