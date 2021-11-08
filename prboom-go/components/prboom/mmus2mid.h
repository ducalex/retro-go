/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
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
 *  mmus2mid.c supports conversion of MUS format music in memory
 *  to MIDI format 1 music in memory.
 */

#if !defined( MMUS2MID_H )
#define MMUS2MID_H

// error codes

typedef enum
{
  MUSDATACOR,    // MUS data corrupt
  TOOMCHAN,      // Too many channels
  MEMALLOC,      // Memory allocation error
  MUSDATAMT,     // MUS file empty
  BADMUSCTL,     // MUS event 5 or 7 found
  BADSYSEVT,     // MUS system event not in 10-14 range
  BADCTLCHG,     // MUS control change larger than 9
  TRACKOVF,      // MIDI track exceeds allocation
  BADMIDHDR,     // bad midi header detected
} error_code_t;

// some names for integers of various sizes, all unsigned
typedef unsigned char UBYTE;  // a one-byte int
typedef unsigned short UWORD; // a two-byte int
// proff: changed from unsigned int to unsigned long to avoid warning
typedef unsigned long ULONG;   // a four-byte int (assumes int 4 bytes)

#ifndef MSDOS /* proff: This is from allegro.h */
#define MIDI_TRACKS           32

typedef struct MIDI                    /* a midi file */
{
   int divisions;                      /* number of ticks per quarter note */
   struct {
      unsigned char *data;             /* MIDI message stream */
      int len;                         /* length of the track data */
   } track[MIDI_TRACKS];
} MIDI;
#endif /* !MSDOS */

extern int mmus2mid(const UBYTE *mus,MIDI *mid, UWORD division, int nocomp);
extern void free_mididata(MIDI *mid);
extern int MIDIToMidi(MIDI *mididata,UBYTE **mid,int *midlen);
extern int MidiToMIDI(UBYTE *mid,MIDI *mididata);

#endif
