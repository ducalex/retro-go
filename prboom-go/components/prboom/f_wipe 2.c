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
 *      Mission begin melt/wipe screen special effect.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"
#include "doomdef.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"

//
// SCREEN WIPE PACKAGE
//

// Parts re-written to support true-color video modes. Column-major
// formatting removed. - POPE

// CPhipps - macros for the source and destination screens
#define SRC_SCR 2
#define DEST_SCR 3

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;

static int y_lookup[MAX_SCREENWIDTH];


static int wipe_initMelt(int ticks)
{
  int i;

  // copy start screen to main screen
  for(i=0;i<SCREENHEIGHT;i++)
    memcpy(wipe_scr.data+i*wipe_scr.byte_pitch,
           wipe_scr_start.data+i*wipe_scr.byte_pitch,
           SCREENWIDTH*V_GetPixelDepth());

  // setup initial column positions (y<0 => not ready to scroll yet)
  y_lookup[0] = -(M_Random()%16);
  for (i=1;i<SCREENWIDTH;i++)
    {
      int r = (M_Random()%3) - 1;
      y_lookup[i] = y_lookup[i-1] + r;
      if (y_lookup[i] > 0)
        y_lookup[i] = 0;
      else
        if (y_lookup[i] == -16)
          y_lookup[i] = -15;
    }
  return 0;
}

static int wipe_doMelt(int ticks)
{
  boolean done = true;
  int i;
  const int depth = V_GetPixelDepth();

  while (ticks--) {
    for (i=0;i<(SCREENWIDTH);i++) {
      if (y_lookup[i]<0) {
        y_lookup[i]++;
        done = false;
        continue;
      }
      if (y_lookup[i] < SCREENHEIGHT) {
        byte *s, *d;
        int j, k, dy;

        /* cph 2001/07/29 -
          *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
          *  the whole screen, so make the melt rate depend on SCREENHEIGHT
          *  so it takes no longer in high res
          */
        dy = (y_lookup[i] < 16) ? y_lookup[i]+1 : SCREENHEIGHT/25;
        if (y_lookup[i]+dy >= SCREENHEIGHT)
          dy = SCREENHEIGHT - y_lookup[i];

        s = wipe_scr_end.data    + (y_lookup[i]*wipe_scr_end.byte_pitch+(i*depth));
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.byte_pitch+(i*depth));
        for (j=dy;j;j--) {
          for (k=0; k<depth; k++)
            d[k] = s[k];
          d += wipe_scr.byte_pitch;
          s += wipe_scr_end.byte_pitch;
        }
        y_lookup[i] += dy;
        s = wipe_scr_start.data  + (i*depth);
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.byte_pitch+(i*depth));
        for (j=SCREENHEIGHT-y_lookup[i];j;j--) {
          for (k=0; k<depth; k++)
            d[k] = s[k];
          d += wipe_scr.byte_pitch;
          s += wipe_scr_end.byte_pitch;
        }
        done = false;
      }
    }
  }
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory

static int wipe_exitMelt(int ticks)
{
  V_FreeScreen(&wipe_scr_start);
  wipe_scr_start.width = 0;
  wipe_scr_start.height = 0;
  V_FreeScreen(&wipe_scr_end);
  wipe_scr_end.width = 0;
  wipe_scr_end.height = 0;
  // Paranoia
  screens[SRC_SCR] = wipe_scr_start;
  screens[DEST_SCR] = wipe_scr_end;
  return 0;
}

int wipe_StartScreen(void)
{
  wipe_scr_start.width = SCREENWIDTH;
  wipe_scr_start.height = SCREENHEIGHT;
  wipe_scr_start.byte_pitch = screens[0].byte_pitch;
  wipe_scr_start.short_pitch = screens[0].short_pitch;
  wipe_scr_start.int_pitch = screens[0].int_pitch;
  wipe_scr_start.not_on_heap = false;
  V_AllocScreen(&wipe_scr_start);
  screens[SRC_SCR] = wipe_scr_start;
  V_CopyRect(0, 0, 0,       SCREENWIDTH, SCREENHEIGHT, 0, 0, SRC_SCR, VPT_NONE ); // Copy start screen to buffer
  return 0;
}

int wipe_EndScreen(void)
{
  wipe_scr_end.width = SCREENWIDTH;
  wipe_scr_end.height = SCREENHEIGHT;
  wipe_scr_end.byte_pitch = screens[0].byte_pitch;
  wipe_scr_end.short_pitch = screens[0].short_pitch;
  wipe_scr_end.int_pitch = screens[0].int_pitch;
  wipe_scr_end.not_on_heap = false;
  V_AllocScreen(&wipe_scr_end);
  screens[DEST_SCR] = wipe_scr_end;
  V_CopyRect(0, 0, 0,       SCREENWIDTH, SCREENHEIGHT, 0, 0, DEST_SCR, VPT_NONE); // Copy end screen to buffer
  V_CopyRect(0, 0, SRC_SCR, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0       , VPT_NONE); // restore start screen
  return 0;
}

// killough 3/5/98: reformatted and cleaned up
int wipe_ScreenWipe(int ticks)
{
  static boolean go;                               // when zero, stop the wipe
  if (!go)                                         // initial stuff
    {
      go = 1;
      wipe_scr = screens[0];
      wipe_initMelt(ticks);
    }
  // do a piece of wipe-in
  if (wipe_doMelt(ticks))     // final stuff
    {
      wipe_exitMelt(ticks);
      go = 0;
    }
  return !go;
}
