// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//  System interface for music.
//
//-----------------------------------------------------------------------------


#ifndef OPLPLAYER_H
#define OPLPLAYER_H

typedef struct
{
  // descriptive name of the player, such as "OPL2 Synth"
  const char *(*name)(void);

  // samplerate is in hz.  return is 1 for success
  int (*init)(int samplerate);

  // deallocate structures, cleanup, ...
  void (*shutdown)(void);

  // set volume, 0 = off, 15 = max
  void (*setvolume)(int v);

  // pause currently running song.
  void (*pause)(void);

  // undo pause
  void (*resume)(void);

  // return a player-specific handle, or NULL on failure.
  // data does not belong to player, but it will persist as long as unregister is not called
  const void *(*registersong)(const void *data, unsigned len);

  // deallocate structures, etc.  data is no longer valid
  void (*unregistersong)(const void *handle);

  void (*play)(const void *handle, int looping);

  // stop
  void (*stop)(void);

  // s16 stereo, with samplerate as specified in init.  player needs to be able to handle
  // just about anything for nsamp.  render can be called even during pause+stop.
  void (*render)(void *dest, unsigned nsamp);
} music_player_t;



extern const music_player_t opl_synth_player;


#endif
