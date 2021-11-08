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
 *      Rendering of moving objects, sprites.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_THINGS__
#define __R_THINGS__

#ifdef __GNUG__
#pragma interface
#endif

#include "r_draw.h"

/* Constant arrays used for psprite clipping and initializing clipping. */

extern int negonearray[MAX_SCREENWIDTH];       /* killough 2/8/98: */ // dropoff overflow
extern int screenheightarray[MAX_SCREENWIDTH]; /* change to MAX_*  */ // dropoff overflow

/* Vars for R_DrawMaskedColumn */

extern int     *mfloorclip;    // dropoff overflow
extern int     *mceilingclip;  // dropoff overflow
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t pspritescale;
extern fixed_t pspriteiscale;
/* proff 11/06/98: Added for high-res */
extern fixed_t pspriteyscale;

void R_DrawMaskedColumn(const rpatch_t *patch,
                        R_DrawColumn_f colfunc,
                        draw_column_vars_t *dcvars,
                        const rcolumn_t *column,
                        const rcolumn_t *prevcolumn,
                        const rcolumn_t *nextcolumn);
void R_SortVisSprites(void);
void R_AddSprites(subsector_t* subsec, int lightlevel);
void R_DrawPlayerSprites(void);
void R_InitSprites(const char * const * namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);

#endif
