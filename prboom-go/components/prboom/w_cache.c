/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 2001 by
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
 *      Transparent access to data in WADs using mmap or caching
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"

typedef struct
{
  void *ptr;
  short lump;
  short locks;
} segment_t;

#define MAX_CACHE_HANDLES 192
static segment_t segments[MAX_CACHE_HANDLES];
static int cursor = 0;

void W_InitCache(void)
{
  //
}

void W_DoneCache(void)
{
  //
}

const void *W_CacheLumpNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error("W_CacheLumpNum: %d >= numlumps(%d)", lump, numlumps);
#endif

  for (int i = 0; i < MAX_CACHE_HANDLES; i++)
  {
    if (segments[i].lump == lump && segments[i].ptr)
    {
      if (++segments[i].locks <= 1)
      {
        Z_ChangeTag(segments[i].ptr, PU_STATIC);
        segments[i].locks = 1;
      }
      return segments[i].ptr;
    }
  }

  int n = MAX_CACHE_HANDLES;
  while (segments[cursor].locks > 0)
  {
    if (++cursor == MAX_CACHE_HANDLES)
      cursor = 0;
    if (--n == 0)
      I_Error("Out of cache handles!");
  }

  int handle = cursor++;

  if (cursor == MAX_CACHE_HANDLES)
    cursor = 0;

  if (segments[handle].ptr)
    Z_Free(segments[handle].ptr);

  W_ReadLump(lump, Z_Malloc(W_LumpLength(lump), PU_STATIC, &segments[handle].ptr));

  segments[handle].lump = lump;
  segments[handle].locks = 1;

  return segments[handle].ptr;
}

void W_UnlockLumpNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error("W_UnlockLumpNum: %d >= numlumps(%d)", lump, numlumps);
#endif

  for (int i = 0; i < MAX_CACHE_HANDLES; i++)
  {
    if (segments[i].lump == lump && segments[i].ptr)
    {
      if (--segments[i].locks == 0)
      {
        Z_ChangeTag(segments[i].ptr, PU_CACHE);
        return;
      }
    }
  }
}

#ifdef HEAPDUMP
void W_PrintLump(FILE *fp, void *p)
{
  //
}
#endif
