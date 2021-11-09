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
  void *file;
  void *ptr;
  size_t offset;
  size_t len;
  int locks;
} segment_t;

#define NO_MMAP_HANDLES 128
static segment_t segments[NO_MMAP_HANDLES];
static int cursor = 0;
// static byte *lumpmap;

//
// RG: Using a lumpmap would avoid scanning segments[] on every W_CacheLumpNum and
// W_UnlockLumpNum calls. But it is causing some issues at the moment so we'll live
// with the small delay. Which is still less than what it was in the previous implementation!
//
void W_InitCache(void)
{
  // lumpmap = malloc(numlumps);
  // memset(lumpmap, 0xFF, numlumps);
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

  FILE *fd = lumpinfo[lump].wadfile->handle;
  size_t length = lumpinfo[lump].size;
  off_t offset = lumpinfo[lump].position;

  for (int i = 0; i < NO_MMAP_HANDLES; i++)
  {
    if (segments[i].file == fd && segments[i].offset == offset && segments[i].len >= length && segments[i].ptr)
    {
      segments[i].locks++;

      if (segments[i].locks <= 1)
      {
        Z_ChangeTag(segments[i].ptr, PU_STATIC);
        segments[i].locks = 1;
      }
      return segments[i].ptr;
    }
  }

  int n = NO_MMAP_HANDLES;
  while (segments[cursor].locks > 0)
  {
    if (++cursor == NO_MMAP_HANDLES)
      cursor = 0;
    if (--n == 0)
      I_Error("Out of Mmap handles!");
  }

  int handle = cursor++;

  if (cursor == NO_MMAP_HANDLES)
    cursor = 0;

  if (segments[handle].ptr)
    Z_Free(segments[handle].ptr);

  void *block = Z_Malloc(length, PU_STATIC, &segments[handle].ptr);

  segments[handle].file = fd;
  segments[handle].offset = offset;
  segments[handle].len = length;
  segments[handle].locks = 1;

  fseek(fd, offset, SEEK_SET);
  fread(block, length, 1, fd);

  return block;
}

void W_UnlockLumpNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error("W_UnlockLumpNum: %d >= numlumps(%d)", lump, numlumps);
#endif

  size_t length = lumpinfo[lump].size;
  void *fd = lumpinfo[lump].wadfile->handle;
  off_t offset = lumpinfo[lump].position;

  for (int i = 0; i < NO_MMAP_HANDLES; i++)
  {
    if (segments[i].file == fd && segments[i].offset == offset && segments[i].len >= length && segments[i].ptr)
    {
      segments[i].locks--;
      if (segments[i].locks == 0)
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
