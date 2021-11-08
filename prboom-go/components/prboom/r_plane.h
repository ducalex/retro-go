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
 *      Refresh, visplane stuff (floor, ceilings).
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_PLANE__
#define __R_PLANE__

#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif

/* killough 10/98: special mask indicates sky flat comes from sidedef */
#define PL_SKYFLAT (0x80000000)

/* Visplane related. */
extern int *lastopening; // dropoff overflow

extern int floorclip[], ceilingclip[]; // dropoff overflow
extern fixed_t yslope[], distscale[];

void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_DrawPlanes (void);

visplane_t *R_FindPlane(
                        fixed_t height,
                        int picnum,
                        int lightlevel,
                        fixed_t xoffs,  /* killough 2/28/98: add x-y offsets */
                        fixed_t yoffs
                       );

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop);

#endif
