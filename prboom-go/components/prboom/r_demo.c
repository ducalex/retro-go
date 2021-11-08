/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze, Andrey Budko
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
 *      Demo stuff
 *
 *---------------------------------------------------------------------
 */

#include "doomstat.h"
#include "r_demo.h"
#include "r_fps.h"

int demo_smoothturns = false;
int demo_smoothturnsfactor = 6;

static int smooth_playing_turns[SMOOTH_PLAYING_MAXFACTOR];
static int_64_t smooth_playing_sum;
static int smooth_playing_index;
static angle_t smooth_playing_angle;

void R_SmoothPlaying_Reset(player_t *player)
{
  if (demo_smoothturns && demoplayback) //  && players
  {
    if (!player)
      player = &players[displayplayer];

    if (player==&players[displayplayer])
    {
      smooth_playing_angle = players[displayplayer].mo->angle;
      memset(smooth_playing_turns, 0, sizeof(smooth_playing_turns[0]) * SMOOTH_PLAYING_MAXFACTOR);
      smooth_playing_sum = 0;
      smooth_playing_index = 0;
    }
  }
}

void R_SmoothPlaying_Add(int delta)
{
  if (demo_smoothturns && demoplayback)
  {
    smooth_playing_sum -= smooth_playing_turns[smooth_playing_index];
    smooth_playing_turns[smooth_playing_index] = delta;
    smooth_playing_index = (smooth_playing_index + 1)%(demo_smoothturnsfactor);
    smooth_playing_sum += delta;
    smooth_playing_angle += (int)(smooth_playing_sum/(demo_smoothturnsfactor));
  }
}

angle_t R_SmoothPlaying_Get(angle_t defangle)
{
  if (demo_smoothturns && demoplayback)
    return smooth_playing_angle;
  else
    return defangle;
}

void R_ResetAfterTeleport(player_t *player)
{
  R_ResetViewInterpolation();
  R_SmoothPlaying_Reset(player);
}
