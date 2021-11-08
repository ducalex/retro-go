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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __R_DRAW__
#define __R_DRAW__

#include "r_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

enum column_pipeline_e {
  RDC_PIPELINE_STANDARD,
  RDC_PIPELINE_TRANSLUCENT,
  RDC_PIPELINE_TRANSLATED,
  RDC_PIPELINE_FUZZ,
  RDC_PIPELINE_MAXPIPELINES,
};

// Used to specify what kind of filering you want
enum draw_filter_type_e {
  RDRAW_FILTER_NONE,
  RDRAW_FILTER_POINT,
  RDRAW_FILTER_LINEAR,
  RDRAW_FILTER_ROUNDED,
  RDRAW_FILTER_MAXFILTERS
};

// Used to specify what kind of column edge rendering to use on masked 
// columns. SQUARE = standard, SLOPED = slope the column edge up or down
// based on neighboring columns
enum sloped_edge_type_e {
  RDRAW_MASKEDCOLUMNEDGE_SQUARE,
  RDRAW_MASKEDCOLUMNEDGE_SLOPED
};

// Packaged into a struct - POPE
typedef struct {
  int                 x;
  int                 yl;
  int                 yh;
  fixed_t             z; // the current column z coord
  fixed_t             iscale;
  fixed_t             texturemid;
  int                 texheight;    // killough
  fixed_t             texu; // the current column u coord
  const byte          *source; // first pixel in a column
  const byte          *prevsource; // first pixel in previous column
  const byte          *nextsource; // first pixel in next column
  const lighttable_t  *colormap;
  const lighttable_t  *nextcolormap;
  const byte          *translation;
  int                 edgeslope; // OR'ed RDRAW_EDGESLOPE_*
  // 1 if R_DrawColumn* is currently drawing a masked column, otherwise 0
  int                 drawingmasked;
  enum sloped_edge_type_e edgetype;
} draw_column_vars_t;

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars);

void R_VideoErase(int x, int y, int count);

typedef struct {
  int                 y;
  int                 x1;
  int                 x2;
  fixed_t             z; // the current span z coord
  fixed_t             xfrac;
  fixed_t             yfrac;
  fixed_t             xstep;
  fixed_t             ystep;
  const byte          *source; // start of a 64*64 tile image
  const lighttable_t  *colormap;
  const lighttable_t  *nextcolormap;
} draw_span_vars_t;

typedef struct {
  byte           *byte_topleft;
  unsigned short *short_topleft;
  unsigned int   *int_topleft;
  int   byte_pitch;
  int   short_pitch;
  int   int_pitch;

  enum draw_filter_type_e filterwall;
  enum draw_filter_type_e filterfloor;
  enum draw_filter_type_e filtersprite;
  enum draw_filter_type_e filterz;
  enum draw_filter_type_e filterpatch;

  enum sloped_edge_type_e sprite_edges;
  enum sloped_edge_type_e patch_edges;

  // Used to specify an early-out magnification threshold for filtering.
  // If a texture is being minified (dcvars.iscale > rdraw_magThresh), then it
  // drops back to point filtering.
  fixed_t mag_threshold;
} draw_vars_t;

extern draw_vars_t drawvars;

extern byte playernumtotrans[MAXPLAYERS]; // CPhipps - what translation table for what player
extern byte       *translationtables;

typedef void (*R_DrawColumn_f)(draw_column_vars_t *dcvars);
R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type,
                                   enum draw_filter_type_e filter,
                                   enum draw_filter_type_e filterz);

// Span blitting for rows, floor/ceiling. No Spectre effect needed.
typedef void (*R_DrawSpan_f)(draw_span_vars_t *dsvars);
R_DrawSpan_f R_GetDrawSpanFunc(enum draw_filter_type_e filter,
                               enum draw_filter_type_e filterz);
void R_DrawSpan(draw_span_vars_t *dsvars);

void R_InitBuffer(int width, int height);

// Initialize color translation tables, for player rendering etc.
void R_InitTranslationTables(void);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);

// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
void R_ResetColumnBuffer(void);

#endif
