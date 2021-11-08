/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
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
 *      Handles in-memory caching of WAD lumps
 *
 *-----------------------------------------------------------------------------
 */
#if 0
// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"

static struct {
  void *cache;
#ifdef TIMEDIAG
  int locktic;
#endif
  unsigned int locks;
} *cachelump;

#ifdef HEAPDUMP
void W_PrintLump(FILE* fp, void* p) {
  int i;
  for (i=0; i<numlumps; i++)
    if (cachelump[i].cache == p) {
      fprintf(fp, " %8.8s %6u %2d %6d", lumpinfo[i].name,
	      W_LumpLength(i), cachelump[i].locks, gametic - cachelump[i].locktic);
      return;
    }
  fprintf(fp, " not found");
}
#endif

#ifdef TIMEDIAG
static void W_ReportLocks(void)
{
  int i;
  lprintf(LO_DEBUG, "W_ReportLocks:\nLump     Size   Locks  Tics\n");
  for (i=0; i<numlumps; i++) {
    if (cachelump[i].locks)
      lprintf(LO_DEBUG, "%8.8s %6u %2d   %6d\n", lumpinfo[i].name,
	      W_LumpLength(i), cachelump[i].locks, gametic - cachelump[i].locktic);
  }
}
#endif

/* W_InitCache
 *
 * cph 2001/07/07 - split from W_Init
 */
void W_InitCache(void)
{
  // set up caching
  cachelump = calloc(sizeof *cachelump, numlumps);
  if (!cachelump)
    I_Error ("W_Init: Couldn't allocate lumpcache");

#ifdef TIMEDIAG
  atexit(W_ReportLocks);
#endif
}

void W_DoneCache(void)
{
}

/* W_CacheLumpNum
 * killough 4/25/98: simplified
 * CPhipps - modified for new lump locking scheme
 *           returns a const*
 */

const void *W_CacheLumpNum(int lump)
{
  const int locks = 1;
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_CacheLumpNum: %i >= numlumps",lump);
#endif

  if (!cachelump[lump].cache)      // read the lump in
    W_ReadLump(lump, Z_Malloc(W_LumpLength(lump), PU_CACHE, &cachelump[lump].cache));

  /* cph - if wasn't locked but now is, tell z_zone to hold it */
  if (!cachelump[lump].locks && locks) {
    Z_ChangeTag(cachelump[lump].cache,PU_STATIC);
#ifdef TIMEDIAG
    cachelump[lump].locktic = gametic;
#endif
  }
  cachelump[lump].locks += locks;

#ifdef SIMPLECHECKS
  if (!((cachelump[lump].locks+1) & 0xf))
    lprintf(LO_DEBUG, "W_CacheLumpNum: High lock on %8s (%d)\n",
	    lumpinfo[lump].name, cachelump[lump].locks);
#endif

  return cachelump[lump].cache;
}

const void *W_LockLumpNum(int lump)
{
  return W_CacheLumpNum(lump);
}

/*
 * W_UnlockLumpNum
 *
 * CPhipps - this changes (should reduce) the number of locks on a lump
 */

void W_UnlockLumpNum(int lump)
{
  const int unlocks = 1;
#ifdef SIMPLECHECKS
  if ((signed short)cachelump[lump].locks < unlocks)
    lprintf(LO_DEBUG, "W_UnlockLumpNum: Excess unlocks on %8s (%d-%d)\n",
	    lumpinfo[lump].name, cachelump[lump].locks, unlocks);
#endif
  cachelump[lump].locks -= unlocks;
  /* cph - Note: must only tell z_zone to make purgeable if currently locked,
   * else it might already have been purged
   */
  if (unlocks && !cachelump[lump].locks)
    Z_ChangeTag(cachelump[lump].cache, PU_CACHE);
}

#endif