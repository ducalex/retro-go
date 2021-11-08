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
 *-----------------------------------------------------------------------------*/


#ifndef R_PATCH_H
#define R_PATCH_H

// Used to specify the sloping of the top and bottom of a column post
typedef enum {
  RDRAW_EDGESLOPE_TOP_UP   = (1<<0),
  RDRAW_EDGESLOPE_TOP_DOWN = (1<<1),
  RDRAW_EDGESLOPE_BOT_UP   = (1<<2),
  RDRAW_EDGESLOPE_BOT_DOWN = (1<<3),
  RDRAW_EDGESLOPE_TOP_MASK = 0x3,
  RDRAW_EDGESLOPE_BOT_MASK = 0xc,  
} edgeslope_t;

typedef struct {
  int topdelta;
  int length;
  edgeslope_t slope;
} rpost_t;

typedef struct {
  int numPosts;
  rpost_t *posts;
  unsigned char *pixels;
} rcolumn_t;

typedef struct {
  int width;
  int height;
  unsigned  widthmask;
    
  unsigned char isNotTileable;
  
  int leftoffset;
  int topoffset;
  
  // this is the single malloc'ed/free'd array 
  // for this patch
  unsigned char *data;
  
  // these are pointers into the data array
  unsigned char *pixels;
  rcolumn_t *columns;
  rpost_t *posts;

#ifdef TIMEDIAG
  int locktic;
#endif
  unsigned int locks;
} rpatch_t;


const rpatch_t *R_CachePatchNum(int id);
void R_UnlockPatchNum(int id);
#define R_CachePatchName(name) R_CachePatchNum(W_GetNumForName(name))
#define R_UnlockPatchName(name) R_UnlockPatchNum(W_GetNumForName(name))

const rpatch_t *R_CacheTextureCompositePatchNum(int id);
void R_UnlockTextureCompositePatchNum(int id);


// Size query funcs
int R_NumPatchWidth(int lump) ;
int R_NumPatchHeight(int lump);
#define R_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define R_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))


const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex);


// returns R_GetPatchColumnWrapped for square, non-holed textures
// and R_GetPatchColumnClamped otherwise
const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex);


void R_InitPatches();
void R_FlushAllPatches();

#endif
