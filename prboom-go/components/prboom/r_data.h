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
 *  Refresh module, data I/O, caching, retrieval of graphics
 *  by name.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"
#include "r_patch.h"

#ifdef __GNUG__
#pragma interface
#endif

// A single patch from a texture definition, basically
// a rectangular area within the texture rectangle.
typedef struct
{
  int originx, originy;  // Block origin, which has already accounted
  int patch;             // for the internal origin of the patch.
} texpatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//

typedef struct
{
  char  name[8];         // Keep name for switch changing, etc.
  int   next, index;     // killough 1/31/98: used in hashing algorithm
  // CPhipps - moved arrays with per-texture entries to elements here
  unsigned  widthmask;
  // CPhipps - end of additions
  short width, height;
  short patchcount;      // All the patches[patchcount] are drawn
  texpatch_t patches[1]; // back-to-front into the cached texture.
} texture_t;

extern int numtextures;
extern texture_t **textures;


const byte *R_GetTextureColumn(const rpatch_t *texpatch, int col);


// I/O, setting up the stuff.
void R_InitData (void);
void R_PrecacheLevel (void);


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName (const char* name);   // killough -- const added


// R_*TextureNumForName returns the texture number for the texture name, or NO_TEXTURE if 
//  there is no texture (i.e. "-") specified.
/* cph 2006/07/23 - defined value for no-texture marker (texture "-" in the WAD file) */
#define NO_TEXTURE 0
int PUREFUNC R_TextureNumForName (const char *name);    // killough -- const added; cph - now PUREFUNC
int PUREFUNC R_SafeTextureNumForName (const char *name, int snum);
int PUREFUNC R_CheckTextureNumForName (const char *name);

void R_InitTranMap(int);      // killough 3/6/98: translucency initialization
int R_ColormapNumForName(const char *name);      // killough 4/4/98
/* cph 2001/11/17 - new func to do lighting calcs and get suitable colour map */
const lighttable_t* R_ColourMap(int lightlevel, fixed_t spryscale);

extern const byte *main_tranmap, *tranmap;

/* Proff - Added for OpenGL - cph - const char* param */
void R_SetPatchNum(patchnum_t *patchnum, const char *name);

#endif
