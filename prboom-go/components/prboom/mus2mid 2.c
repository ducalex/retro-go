/*
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  This file supports conversion of MUS format music in memory
 *  to MIDI format 1 music in memory.
 *
 *  Much of the code here is thanks to S. Bacquet's source for QMUS2MID.C
 *
 */

#include "doomtype.h"
#include "doomdef.h"
#include "m_swap.h"
#include "z_zone.h"
#include "lprintf.h"
#include "mus2mid.h"

#define MIDI_TRACKS 16

// MIDI track
typedef struct
{
  unsigned velocity;
  unsigned deltaT;
  size_t length;
  byte *data;
} track_t;

// event types
typedef enum
{
  RELEASE_NOTE,
  PLAY_NOTE,
  BEND_NOTE,
  SYS_EVENT,
  CNTL_CHANGE,
  UNKNOWN_EVENT1,
  SCORE_END,
  UNKNOWN_EVENT2,
} mus_event_t;

// MUS format header structure
typedef struct PACKEDATTR
{
  unsigned char ID[4];        // identifier "MUS"0x1A
  unsigned short ScoreLength; // length of music portion
  unsigned short ScoreStart;  // offset of music portion
  unsigned short channels;    // count of primary channels
  unsigned short SecChannels; // count of secondary channels
  unsigned short InstrCnt;    // number of instruments
} MUSheader;

// lookup table MUS -> MID controls
static const byte MUS2MIDcontrol[15] =
{
    0,    // Program change - not a MIDI control change
    0x00, // Bank select
    0x01, // Modulation pot
    0x07, // Volume
    0x0A, // Pan pot
    0x0B, // Expression pot
    0x5B, // Reverb depth
    0x5D, // Chorus depth
    0x40, // Sustain pedal
    0x43, // Soft pedal
    0x78, // All sounds off
    0x7B, // All notes off
    0x7E, // Mono
    0x7F, // Poly
    0x79, // Reset all controllers
};

#define WriteByte(track, byte) {track_t *t = (track);if (t->data)t->data[t->length] = (byte);t->length++;}

static mus_err_t mus2mid_internal(track_t *tracks, const byte *mus, size_t muslen)
{
  const MUSheader *header = (MUSheader *)mus;
  int MIDIchan2track[MIDI_TRACKS];
  int MUS2MIDchannel[MIDI_TRACKS];

  for (int i = 0; i < MIDI_TRACKS; i++) // init the track structure's tracks
  {
    MUS2MIDchannel[i] = -1; // flag for channel not used yet
    tracks[i].velocity = 64;
    tracks[i].deltaT = 0;
    tracks[i].length = 0;
  }

  // the first track is a special tempo/key track
  const byte track0[] = {
    0x00, 0xff, 0x59, 0x02, 0x00, 0x00,       // Key (C major)
    0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a, // Tempo
  };
  for (int i = 0; i < sizeof(track0); i++)
    WriteByte(&tracks[0], track0[i]);

  const byte *musptr = mus + header->ScoreStart;
  size_t numtracks = 1;

  // process the MUS events in the MUS buffer
  while (1)
  {
    byte MUSevent = *musptr++;
    byte MUSeventType = (MUSevent & 0x7F) >> 4;
    byte MUSchannel = MUSevent & 0x0F;
    int data;

    if (MUSeventType == SCORE_END)
      break;

    if (MUS2MIDchannel[MUSchannel] == -1)
    {
      if (MUSchannel == 15)
      {
        // MUS channel 15 is always assigned to MIDI channel 9 (percussion)
        MUS2MIDchannel[MUSchannel] = 9;
      }
      else
      {
        int max = -1;
        for (int i = 0; i < 15; i++)
          max = MAX(max, MUS2MIDchannel[i]);
        MUS2MIDchannel[MUSchannel] = (max == 8 ? 10 : max + 1);
      }
      MIDIchan2track[MUS2MIDchannel[MUSchannel]] = numtracks++;
    }

    int MIDIchannel = MUS2MIDchannel[MUSchannel];
    int MIDItrack = MIDIchan2track[MIDIchannel];

    unsigned value = tracks[MIDItrack].deltaT;
    unsigned buffer = value & 0x7f;

    while ((value >>= 7)) // terminates because value unsigned
    {
      buffer <<= 8;   // note first value shifted in has bit 8 clear
      buffer |= 0x80 | (value & 0x7f);
    }

    while (1) // write bytes out in opposite order
    {
      WriteByte(&tracks[MIDItrack], buffer & 0xff); // insure buffer masked
      if (buffer & 0x80)
        buffer >>= 8;
      else // terminate on the byte with bit 8 clear
        break;
    }
    tracks[MIDItrack].deltaT = 0;

    switch (MUSeventType)
    {
    case RELEASE_NOTE:
      WriteByte(&tracks[MIDItrack], 0x90 | MIDIchannel);
      data = *musptr++;
      WriteByte(&tracks[MIDItrack], (byte)(data & 0x7F));
      WriteByte(&tracks[MIDItrack], 0);
      break;

    case PLAY_NOTE:
      WriteByte(&tracks[MIDItrack], 0x90 | MIDIchannel);
      data = *musptr++;
      WriteByte(&tracks[MIDItrack], (byte)(data & 0x7F));
      if (data & 0x80)
        tracks[MIDItrack].velocity = (*musptr++) & 0x7f;
      WriteByte(&tracks[MIDItrack], tracks[MIDItrack].velocity);
      break;

    case BEND_NOTE:
      WriteByte(&tracks[MIDItrack], 0xE0 | MIDIchannel);
      data = *musptr++;
      WriteByte(&tracks[MIDItrack], (byte)((data & 1) << 6));
      WriteByte(&tracks[MIDItrack], (byte)(data >> 1));
      break;

    case SYS_EVENT:
      WriteByte(&tracks[MIDItrack], 0x80 | MIDIchannel);
      data = *musptr++;
      if (data < 10 || data > 14)
        return MUS_BADEVENT;
      WriteByte(&tracks[MIDItrack], MUS2MIDcontrol[data]);
      if (data == 12)
      {
        WriteByte(&tracks[MIDItrack], (byte)(header->channels + 1));
      }
      else WriteByte(&tracks[MIDItrack], 0);
      break;

    case CNTL_CHANGE:
      data = *musptr++;
      if (data > 9)
        return MUS_BADEVENT;
      if (data)
      {
        WriteByte(&tracks[MIDItrack], 0x80 | MIDIchannel);
        WriteByte(&tracks[MIDItrack], MUS2MIDcontrol[data]);
      }
      else
      {
        WriteByte(&tracks[MIDItrack], 0xC0 | MIDIchannel);
      }
      data = *musptr++;
      WriteByte(&tracks[MIDItrack], (byte)(data & 0x7F));
      break;

    case SCORE_END:
      break;

    default:            // meaning not known
      return MUS_BADEVENT; // exit with error
    }

    if (MUSevent & 0x80) // Last event
    {
      unsigned byte, DeltaTime = 0;
      do // shift each byte read up in the result until a byte with bit 8 clear
      {
        byte = *musptr++;
        DeltaTime = (DeltaTime << 7) + (byte & 0x7F);
      } while (byte & 0x80);
      for (int i = 0; i < MIDI_TRACKS; i++)
        tracks[i].deltaT += DeltaTime; //whether allocated yet or not
    }

    if (MUSeventType == SCORE_END)
      break;

    if ((musptr - mus) >= muslen)
      return MUS_BADLENGTH;
  }

  return MUS_OK;
}

//
// mus2mid()
//
// Converts a memory buffer containing MUS data to an MIDI 1 structure
//
// Returns 0 if successful, otherwise an error code (see mus2mid.h).
//
mus_err_t mus2mid(const byte *mus, size_t muslen, byte **mid, size_t *midlen, int division)
{
  const MUSheader *header = (MUSheader *)mus;
  track_t tracks[MIDI_TRACKS] = {0};
  byte  *mididata = NULL;
  size_t totalsize = 14;
  size_t numtracks = 0;
  size_t length = 0;
  mus_err_t ret = MUS_OK;

  if (!mus || memcmp(mus, "MUS\x1A", 4))
    return MUS_INVALID;

  lprintf(LO_INFO, "mus2mid: Length: %d, Start: %d, Channels: %d, Instruments: %d.\n",
          header->ScoreStart, header->ScoreLength, header->channels, header->InstrCnt);

  if (!muslen)
    muslen = header->ScoreStart + header->ScoreLength;

  if (header->channels > 15) // MUSchannels + drum channel > 16
    return MUS_TOOMCHAN;

  if (header->ScoreStart + header->ScoreLength > muslen)
    return MUS_BADLENGTH;

  if (header->ScoreLength == 0)
    return MUS_BADLENGTH;

  // We do a dry run to see how large our final buffer needs to be
  // instead of doing a dozen mallocs/reallocs that fragments our heap.
  if ((ret = mus2mid_internal(tracks, mus, muslen)))
    return ret;

  for (int i = 0; i < MIDI_TRACKS; i++)
    if ((length = tracks[i].length))
    {
      totalsize += 8 + length + 4;
      numtracks++;
    }

  if (!(mididata = malloc(totalsize)))
    return MUS_MEMALLOC;

  byte *midiptr = mididata;

  // write the midi header
  *midiptr++ = 'M';
  *midiptr++ = 'T';
  *midiptr++ = 'h';
  *midiptr++ = 'd';
  *midiptr++ = 0;
  *midiptr++ = 0;
  *midiptr++ = 0;
  *midiptr++ = 6; // Length
  *midiptr++ = 0;
  *midiptr++ = 1; // Format
  *midiptr++ = 0;
  *midiptr++ = numtracks;
  *midiptr++ = (division >> 8) & 0x7f;
  *midiptr++ = (division) & 0xff;

  for (int i = 0; i < MIDI_TRACKS; i++) // init the track structure's tracks
  {
    if ((length = tracks[i].length))
    {
      // header
      *midiptr++ = 'M';
      *midiptr++ = 'T';
      *midiptr++ = 'r';
      *midiptr++ = 'k';
      *midiptr++ = (length >> 24) & 0xff;
      *midiptr++ = (length >> 16) & 0xff;
      *midiptr++ = (length >> 8) & 0xff;
      *midiptr++ = (length) & 0xff;

      // data
      tracks[i].data = midiptr;
      midiptr += tracks[i].length;

      // footer / end of track
      *midiptr++ = 0x00;
      *midiptr++ = 0xFF;
      *midiptr++ = 0x2F;
      *midiptr++ = 0x00;
    }
  }

  if ((ret = mus2mid_internal(tracks, mus, muslen)) == 0)
    *midlen = (midiptr - mididata), *mid = mididata;
  else
    free(mididata);

  return ret;
}
