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
#ifndef _rt_dmand_public
#define _rt_dmand_public

typedef enum {
   rs_nodata,
   rs_newsound,
   rs_endsound,
   rs_data
} recordstate;

//***************************************************************************
//
// SD_StartIncomingSound - Setup to receive an incoming sound in chunks
//
//***************************************************************************

void SD_StartIncomingSound ( void );

//***************************************************************************
//
// SD_StopIncomingSound - Stop receiving an incoming sound and playback
//
//***************************************************************************

void SD_StopIncomingSound ( void );

//***************************************************************************
//
// SD_UpdateIncomingSound - Update an incoming sound
//
//***************************************************************************

void SD_UpdateIncomingSound ( byte * data, word length );

//***************************************************************************
//
// SD_GetSoundData - Returns next piece of sound data, returns:
//
//                   nodata if no sound data is ready
//                   newsound if it is the start of a new sound
//                            data is also returned;
//                   data if data is ready
//
//***************************************************************************

recordstate SD_GetSoundData ( byte * data, word length );

//***************************************************************************
//
// SD_SoundDataReady - Returns true if data is ready
//
//***************************************************************************

boolean SD_SoundDataReady ( void );

//***************************************************************************
//
// SD_SetRecordingActive - Set the recording active flag
//
//***************************************************************************

void SD_SetRecordingActive ( void );

//***************************************************************************
//
// SD_ClearRecordingActive - Clear the recording active flag
//
//***************************************************************************

void SD_ClearRecordingActive ( void );

//***************************************************************************
//
// SD_RecordingActive - Check if recording is active on some system
//
//***************************************************************************

boolean SD_RecordingActive ( void );

//***************************************************************************
//
// SD_StartRecordingSound - Start recording a sound in chunks
//
//***************************************************************************

boolean SD_StartRecordingSound ( void );

//***************************************************************************
//
// SD_StopRecordingSound - Stop recording a sound
//
//***************************************************************************
void SD_StopRecordingSound ( void );

#endif
