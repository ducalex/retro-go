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
 *      The actual span/column drawing functions.
 *      Here find the main potential for optimization,
 *       e.g. inline assembly, different algorithms.
 *
 *-----------------------------------------------------------------------------*/
// This is required because of macro hell
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_filter.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "am_map.h"
#include "lprintf.h"

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

byte *viewimage;
int  viewwidth;
int  scaledviewwidth;
int  viewheight;
int  viewwindowx;
int  viewwindowy;

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

// CPhipps - made const*'s
const byte *tranmap;          // translucency filter maps 256x256   // phares
const byte *main_tranmap;     // killough 4/11/98

//
// R_DrawColumn
// Source is the top of the column to scale.
//

// SoM: OPTIMIZE for ANYRES
typedef enum
{
   COL_NONE,
   COL_OPAQUE,
   COL_TRANS,
   COL_FLEXTRANS,
   COL_FUZZ,
   COL_FLEXADD
} columntype_e;

static int    temp_x = 0;
static int    tempyl[4], tempyh[4];
static byte           byte_tempbuf[MAX_SCREENHEIGHT * 4];
#ifndef NOTRUECOLOR
static unsigned short short_tempbuf[MAX_SCREENHEIGHT * 4];
static unsigned int   int_tempbuf[MAX_SCREENHEIGHT * 4];
#endif
static int    startx = 0;
static int    temptype = COL_NONE;
static int    commontop, commonbot;
static const byte *temptranmap = NULL;
// SoM 7-28-04: Fix the fuzz problem.
static const byte   *tempfuzzmap;

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50
// proff 08/17/98: Changed for high-res
//#define FUZZOFF (SCREENWIDTH)
#define FUZZOFF 1

static const int fuzzoffset_org[FUZZTABLE] = {
  FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

static int fuzzoffset[FUZZTABLE];

static int fuzzpos = 0;

// render pipelines
#define RDC_STANDARD      1
#define RDC_TRANSLUCENT   2
#define RDC_TRANSLATED    4
#define RDC_FUZZ          8
// no color mapping
#define RDC_NOCOLMAP     16
// filter modes
#define RDC_DITHERZ      32
#define RDC_BILINEAR     64
#define RDC_ROUNDED     128

draw_vars_t drawvars = {
  NULL, // byte_topleft
  NULL, // short_topleft
  NULL, // int_topleft
  0, // byte_pitch
  0, // short_pitch
  0, // int_pitch
  RDRAW_FILTER_POINT, // filterwall
  RDRAW_FILTER_POINT, // filterfloor
  RDRAW_FILTER_POINT, // filtersprite
  RDRAW_FILTER_POINT, // filterz
  RDRAW_FILTER_POINT, // filterpatch

  RDRAW_MASKEDCOLUMNEDGE_SQUARE, // sprite_edges
  RDRAW_MASKEDCOLUMNEDGE_SQUARE, // patch_edges

  // 49152 = FRACUNIT * 0.75
  // 81920 = FRACUNIT * 1.25
  49152 // mag_threshold
};

//
// Error functions that will abort if R_FlushColumns tries to flush
// columns without a column type.
//

static void R_FlushWholeError(void)
{
   I_Error("R_FlushWholeColumns called without being initialized.\n");
}

static void R_FlushHTError(void)
{
   I_Error("R_FlushHTColumns called without being initialized.\n");
}

static void R_QuadFlushError(void)
{
   I_Error("R_FlushQuadColumn called without being initialized.\n");
}

static void (*R_FlushWholeColumns)(void) = R_FlushWholeError;
static void (*R_FlushHTColumns)(void)    = R_FlushHTError;
static void (*R_FlushQuadColumn)(void) = R_QuadFlushError;

static void R_FlushColumns(void)
{
   if(temp_x != 4 || commontop >= commonbot)
      R_FlushWholeColumns();
   else
   {
      R_FlushHTColumns();
      R_FlushQuadColumn();
   }
   temp_x = 0;
}

//
// R_ResetColumnBuffer
//
// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
//
void R_ResetColumnBuffer(void)
{
   // haleyjd 10/06/05: this must not be done if temp_x == 0!
   if(temp_x)
      R_FlushColumns();
   temptype = COL_NONE;
   R_FlushWholeColumns = R_FlushWholeError;
   R_FlushHTColumns    = R_FlushHTError;
   R_FlushQuadColumn   = R_QuadFlushError;
}

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL8
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawflush.inl"

#ifndef NOTRUECOLOR
#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL32
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz32
#include "r_drawflush.inl"
#endif
//
// R_DrawColumn
//

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//

byte playernumtotrans[MAXPLAYERS];
byte translationtables[3 * 256];

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"

#ifndef NOTRUECOLOR
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawcolpipeline.inl"
#endif

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLUCENT

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL8
#include "r_drawcolpipeline.inl"

#ifndef NOTRUECOLOR
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL32
#include "r_drawcolpipeline.inl"
#endif

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLATED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"

#ifndef NOTRUECOLOR
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawcolpipeline.inl"
#endif

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawcolpipeline.inl"

#ifndef NOTRUECOLOR
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz32
#include "r_drawcolpipeline.inl"
#endif

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[VID_MODEMAX][RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS][RDC_PIPELINE_MAXPIPELINES] = {
  {
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn8_PointUV,
       R_DrawTLColumn8_PointUV,
       R_DrawTranslatedColumn8_PointUV,
       R_DrawFuzzColumn8_PointUV,},
      {R_DrawColumn8_LinearUV,
       R_DrawTLColumn8_LinearUV,
       R_DrawTranslatedColumn8_LinearUV,
       R_DrawFuzzColumn8_LinearUV,},
      {R_DrawColumn8_RoundedUV,
       R_DrawTLColumn8_RoundedUV,
       R_DrawTranslatedColumn8_RoundedUV,
       R_DrawFuzzColumn8_RoundedUV,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn8_PointUV_PointZ,
       R_DrawTLColumn8_PointUV_PointZ,
       R_DrawTranslatedColumn8_PointUV_PointZ,
       R_DrawFuzzColumn8_PointUV_PointZ,},
      {R_DrawColumn8_LinearUV_PointZ,
       R_DrawTLColumn8_LinearUV_PointZ,
       R_DrawTranslatedColumn8_LinearUV_PointZ,
       R_DrawFuzzColumn8_LinearUV_PointZ,},
      {R_DrawColumn8_RoundedUV_PointZ,
       R_DrawTLColumn8_RoundedUV_PointZ,
       R_DrawTranslatedColumn8_RoundedUV_PointZ,
       R_DrawFuzzColumn8_RoundedUV_PointZ,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn8_PointUV_LinearZ,
       R_DrawTLColumn8_PointUV_LinearZ,
       R_DrawTranslatedColumn8_PointUV_LinearZ,
       R_DrawFuzzColumn8_PointUV_LinearZ,},
      {R_DrawColumn8_LinearUV_LinearZ,
       R_DrawTLColumn8_LinearUV_LinearZ,
       R_DrawTranslatedColumn8_LinearUV_LinearZ,
       R_DrawFuzzColumn8_LinearUV_LinearZ,},
      {R_DrawColumn8_RoundedUV_LinearZ,
       R_DrawTLColumn8_RoundedUV_LinearZ,
       R_DrawTranslatedColumn8_RoundedUV_LinearZ,
       R_DrawFuzzColumn8_RoundedUV_LinearZ,},
    },
  },
#ifndef NOTRUECOLOR
  {
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn15_PointUV,
       R_DrawTLColumn15_PointUV,
       R_DrawTranslatedColumn15_PointUV,
       R_DrawFuzzColumn15_PointUV,},
      {R_DrawColumn15_LinearUV,
       R_DrawTLColumn15_LinearUV,
       R_DrawTranslatedColumn15_LinearUV,
       R_DrawFuzzColumn15_LinearUV,},
      {R_DrawColumn15_RoundedUV,
       R_DrawTLColumn15_RoundedUV,
       R_DrawTranslatedColumn15_RoundedUV,
       R_DrawFuzzColumn15_RoundedUV,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn15_PointUV_PointZ,
       R_DrawTLColumn15_PointUV_PointZ,
       R_DrawTranslatedColumn15_PointUV_PointZ,
       R_DrawFuzzColumn15_PointUV_PointZ,},
      {R_DrawColumn15_LinearUV_PointZ,
       R_DrawTLColumn15_LinearUV_PointZ,
       R_DrawTranslatedColumn15_LinearUV_PointZ,
       R_DrawFuzzColumn15_LinearUV_PointZ,},
      {R_DrawColumn15_RoundedUV_PointZ,
       R_DrawTLColumn15_RoundedUV_PointZ,
       R_DrawTranslatedColumn15_RoundedUV_PointZ,
       R_DrawFuzzColumn15_RoundedUV_PointZ,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn15_PointUV_LinearZ,
       R_DrawTLColumn15_PointUV_LinearZ,
       R_DrawTranslatedColumn15_PointUV_LinearZ,
       R_DrawFuzzColumn15_PointUV_LinearZ,},
      {R_DrawColumn15_LinearUV_LinearZ,
       R_DrawTLColumn15_LinearUV_LinearZ,
       R_DrawTranslatedColumn15_LinearUV_LinearZ,
       R_DrawFuzzColumn15_LinearUV_LinearZ,},
      {R_DrawColumn15_RoundedUV_LinearZ,
       R_DrawTLColumn15_RoundedUV_LinearZ,
       R_DrawTranslatedColumn15_RoundedUV_LinearZ,
       R_DrawFuzzColumn15_RoundedUV_LinearZ,},
    },
  },
  {
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn16_PointUV,
       R_DrawTLColumn16_PointUV,
       R_DrawTranslatedColumn16_PointUV,
       R_DrawFuzzColumn16_PointUV,},
      {R_DrawColumn16_LinearUV,
       R_DrawTLColumn16_LinearUV,
       R_DrawTranslatedColumn16_LinearUV,
       R_DrawFuzzColumn16_LinearUV,},
      {R_DrawColumn16_RoundedUV,
       R_DrawTLColumn16_RoundedUV,
       R_DrawTranslatedColumn16_RoundedUV,
       R_DrawFuzzColumn16_RoundedUV,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn16_PointUV_PointZ,
       R_DrawTLColumn16_PointUV_PointZ,
       R_DrawTranslatedColumn16_PointUV_PointZ,
       R_DrawFuzzColumn16_PointUV_PointZ,},
      {R_DrawColumn16_LinearUV_PointZ,
       R_DrawTLColumn16_LinearUV_PointZ,
       R_DrawTranslatedColumn16_LinearUV_PointZ,
       R_DrawFuzzColumn16_LinearUV_PointZ,},
      {R_DrawColumn16_RoundedUV_PointZ,
       R_DrawTLColumn16_RoundedUV_PointZ,
       R_DrawTranslatedColumn16_RoundedUV_PointZ,
       R_DrawFuzzColumn16_RoundedUV_PointZ,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn16_PointUV_LinearZ,
       R_DrawTLColumn16_PointUV_LinearZ,
       R_DrawTranslatedColumn16_PointUV_LinearZ,
       R_DrawFuzzColumn16_PointUV_LinearZ,},
      {R_DrawColumn16_LinearUV_LinearZ,
       R_DrawTLColumn16_LinearUV_LinearZ,
       R_DrawTranslatedColumn16_LinearUV_LinearZ,
       R_DrawFuzzColumn16_LinearUV_LinearZ,},
      {R_DrawColumn16_RoundedUV_LinearZ,
       R_DrawTLColumn16_RoundedUV_LinearZ,
       R_DrawTranslatedColumn16_RoundedUV_LinearZ,
       R_DrawFuzzColumn16_RoundedUV_LinearZ,},
    },
  },
  {
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn32_PointUV,
       R_DrawTLColumn32_PointUV,
       R_DrawTranslatedColumn32_PointUV,
       R_DrawFuzzColumn32_PointUV,},
      {R_DrawColumn32_LinearUV,
       R_DrawTLColumn32_LinearUV,
       R_DrawTranslatedColumn32_LinearUV,
       R_DrawFuzzColumn32_LinearUV,},
      {R_DrawColumn32_RoundedUV,
       R_DrawTLColumn32_RoundedUV,
       R_DrawTranslatedColumn32_RoundedUV,
       R_DrawFuzzColumn32_RoundedUV,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn32_PointUV_PointZ,
       R_DrawTLColumn32_PointUV_PointZ,
       R_DrawTranslatedColumn32_PointUV_PointZ,
       R_DrawFuzzColumn32_PointUV_PointZ,},
      {R_DrawColumn32_LinearUV_PointZ,
       R_DrawTLColumn32_LinearUV_PointZ,
       R_DrawTranslatedColumn32_LinearUV_PointZ,
       R_DrawFuzzColumn32_LinearUV_PointZ,},
      {R_DrawColumn32_RoundedUV_PointZ,
       R_DrawTLColumn32_RoundedUV_PointZ,
       R_DrawTranslatedColumn32_RoundedUV_PointZ,
       R_DrawFuzzColumn32_RoundedUV_PointZ,},
    },
    {
      {NULL, NULL, NULL, NULL,},
      {R_DrawColumn32_PointUV_LinearZ,
       R_DrawTLColumn32_PointUV_LinearZ,
       R_DrawTranslatedColumn32_PointUV_LinearZ,
       R_DrawFuzzColumn32_PointUV_LinearZ,},
      {R_DrawColumn32_LinearUV_LinearZ,
       R_DrawTLColumn32_LinearUV_LinearZ,
       R_DrawTranslatedColumn32_LinearUV_LinearZ,
       R_DrawFuzzColumn32_LinearUV_LinearZ,},
      {R_DrawColumn32_RoundedUV_LinearZ,
       R_DrawTLColumn32_RoundedUV_LinearZ,
       R_DrawTranslatedColumn32_RoundedUV_LinearZ,
       R_DrawFuzzColumn32_RoundedUV_LinearZ,},
    },
  },
#endif
};

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type,
                                   enum draw_filter_type_e filter,
                                   enum draw_filter_type_e filterz) {
  R_DrawColumn_f result = drawcolumnfuncs[V_GetMode()][filterz][filter][type];
  if (result == NULL)
    I_Error("R_GetDrawColumnFunc: undefined function (%d, %d, %d)",
            type, filter, filterz);
  return result;
}

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars) {
  dcvars->x = dcvars->yl = dcvars->yh = dcvars->z = 0;
  dcvars->iscale = dcvars->texturemid = dcvars->texheight = dcvars->texu = 0;
  dcvars->source = dcvars->prevsource = dcvars->nextsource = NULL;
  dcvars->colormap = dcvars->nextcolormap = colormaps[0];
  dcvars->translation = NULL;
  dcvars->edgeslope = dcvars->drawingmasked = 0;
  dcvars->edgetype = drawvars.sprite_edges;
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//

void R_InitTranslationTables (void)
{
  int i, j;
#define MAXTRANS 3
  byte transtocolour[MAXTRANS];

  // killough 5/2/98:
  // Remove dependency of colormaps aligned on 256-byte boundary

  for (i=0; i<MAXTRANS; i++) transtocolour[i] = 255;

  for (i=0; i<MAXPLAYERS; i++) {
    byte wantcolour = mapcolor_plyr[i];
    playernumtotrans[i] = 0;
    if (wantcolour != 0x70) // Not green, would like translation
      for (j=0; j<MAXTRANS; j++)
  if (transtocolour[j] == 255) {
    transtocolour[j] = wantcolour; playernumtotrans[i] = j+1; break;
  }
  }

  // translate just the 16 green colors
  for (i=0; i<256; i++)
    if (i >= 0x70 && i<= 0x7f)
      {
  // CPhipps - configurable player colours
        translationtables[i] = colormaps[0][((i&0xf)<<9) + transtocolour[0]];
        translationtables[i+256] = colormaps[0][((i&0xf)<<9) + transtocolour[1]];
        translationtables[i+512] = colormaps[0][((i&0xf)<<9) + transtocolour[2]];
      }
    else  // Keep all other colors as is.
      translationtables[i]=translationtables[i+256]=translationtables[i+512]=i;
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#ifndef NOTRUECOLOR
#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"
#endif

static R_DrawSpan_f drawspanfuncs[VID_MODEMAX][RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS] = {
  {
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
    {
      NULL,
      R_DrawSpan8_PointUV_PointZ,
      R_DrawSpan8_LinearUV_PointZ,
      R_DrawSpan8_RoundedUV_PointZ,
    },
    {
      NULL,
      R_DrawSpan8_PointUV_LinearZ,
      R_DrawSpan8_LinearUV_LinearZ,
      R_DrawSpan8_RoundedUV_LinearZ,
    },
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
  },
#ifndef NOTRUECOLOR
  {
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
    {
      NULL,
      R_DrawSpan15_PointUV_PointZ,
      R_DrawSpan15_LinearUV_PointZ,
      R_DrawSpan15_RoundedUV_PointZ,
    },
    {
      NULL,
      R_DrawSpan15_PointUV_LinearZ,
      R_DrawSpan15_LinearUV_LinearZ,
      R_DrawSpan15_RoundedUV_LinearZ,
    },
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
  },
  {
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
    {
      NULL,
      R_DrawSpan16_PointUV_PointZ,
      R_DrawSpan16_LinearUV_PointZ,
      R_DrawSpan16_RoundedUV_PointZ,
    },
    {
      NULL,
      R_DrawSpan16_PointUV_LinearZ,
      R_DrawSpan16_LinearUV_LinearZ,
      R_DrawSpan16_RoundedUV_LinearZ,
    },
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
  },
  {
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
    {
      NULL,
      R_DrawSpan32_PointUV_PointZ,
      R_DrawSpan32_LinearUV_PointZ,
      R_DrawSpan32_RoundedUV_PointZ,
    },
    {
      NULL,
      R_DrawSpan32_PointUV_LinearZ,
      R_DrawSpan32_LinearUV_LinearZ,
      R_DrawSpan32_RoundedUV_LinearZ,
    },
    {
      NULL,
      NULL,
      NULL,
      NULL,
    },
  },
#endif
};

R_DrawSpan_f R_GetDrawSpanFunc(enum draw_filter_type_e filter,
                               enum draw_filter_type_e filterz) {
  R_DrawSpan_f result = drawspanfuncs[V_GetMode()][filterz][filter];
  if (result == NULL)
    I_Error("R_GetDrawSpanFunc: undefined function (%d, %d)",
            filter, filterz);
  return result;
}

void R_DrawSpan(draw_span_vars_t *dsvars) {
  R_GetDrawSpanFunc(drawvars.filterfloor, drawvars.filterz)(dsvars);
}

//
// R_InitBuffer
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_InitBuffer(int width, int height)
{
  int i=0;
  // Handle resize,
  //  e.g. smaller view windows
  //  with border and/or status bar.

  viewwindowx = (SCREENWIDTH-width) >> 1;

  // Same with base row offset.

  viewwindowy = width==SCREENWIDTH ? 0 : (SCREENHEIGHT-(ST_SCALED_HEIGHT-1)-height)>>1;

  drawvars.byte_topleft = screens[0].data + viewwindowy*screens[0].byte_pitch + viewwindowx;
  drawvars.short_topleft = (unsigned short *)(screens[0].data) + viewwindowy*screens[0].short_pitch + viewwindowx;
  drawvars.int_topleft = (unsigned int *)(screens[0].data) + viewwindowy*screens[0].int_pitch + viewwindowx;
  drawvars.byte_pitch = screens[0].byte_pitch;
  drawvars.short_pitch = screens[0].short_pitch;
  drawvars.int_pitch = screens[0].int_pitch;

  if (V_GetMode() == VID_MODE8) {
    for (i=0; i<FUZZTABLE; i++)
      fuzzoffset[i] = fuzzoffset_org[i]*screens[0].byte_pitch;
  }
#ifndef NOTRUECOLOR
  else if ((V_GetMode() == VID_MODE15) || (V_GetMode() == VID_MODE16)) {
    for (i=0; i<FUZZTABLE; i++)
      fuzzoffset[i] = fuzzoffset_org[i]*screens[0].short_pitch;
  } else if (V_GetMode() == VID_MODE32) {
    for (i=0; i<FUZZTABLE; i++)
      fuzzoffset[i] = fuzzoffset_org[i]*screens[0].int_pitch;
  }
#endif
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// CPhipps - patch drawing updated

void R_FillBackScreen (void)
{
  int     x,y;

  if (scaledviewwidth == SCREENWIDTH)
    return;

  V_DrawBackground(gamemode == commercial ? "GRNROCK" : "FLOOR7_2", 1);

  for (x=0; x<scaledviewwidth; x+=8)
    V_DrawNamePatch(viewwindowx+x,viewwindowy-8,1,"brdr_t", CR_DEFAULT, VPT_NONE);

  for (x=0; x<scaledviewwidth; x+=8)
    V_DrawNamePatch(viewwindowx+x,viewwindowy+viewheight,1,"brdr_b", CR_DEFAULT, VPT_NONE);

  for (y=0; y<viewheight; y+=8)
    V_DrawNamePatch(viewwindowx-8,viewwindowy+y,1,"brdr_l", CR_DEFAULT, VPT_NONE);

  for (y=0; y<viewheight; y+=8)
    V_DrawNamePatch(viewwindowx+scaledviewwidth,viewwindowy+y,1,"brdr_r", CR_DEFAULT, VPT_NONE);

  // Draw beveled edge.
  V_DrawNamePatch(viewwindowx-8,viewwindowy-8,1,"brdr_tl", CR_DEFAULT, VPT_NONE);

  V_DrawNamePatch(viewwindowx+scaledviewwidth,viewwindowy-8,1,"brdr_tr", CR_DEFAULT, VPT_NONE);

  V_DrawNamePatch(viewwindowx-8,viewwindowy+viewheight,1,"brdr_bl", CR_DEFAULT, VPT_NONE);

  V_DrawNamePatch(viewwindowx+scaledviewwidth,viewwindowy+viewheight,1,"brdr_br", CR_DEFAULT, VPT_NONE);
}

//
// Copy a screen buffer.
//

void R_VideoErase(int x, int y, int count)
{
  memcpy(screens[0].data+y*screens[0].byte_pitch+x*V_GetPixelDepth(),
          screens[1].data+y*screens[1].byte_pitch+x*V_GetPixelDepth(),
          count*V_GetPixelDepth());   // LFB copy.
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//

void R_DrawViewBorder(void)
{
  int top, side, i;

  if ((SCREENHEIGHT != viewheight) ||
      ((automapmode & am_active) && ! (automapmode & am_overlay)))
  {
    // erase left and right of statusbar
    side= ( SCREENWIDTH - ST_SCALED_WIDTH ) / 2;

    if (side > 0) {
      for (i = (SCREENHEIGHT - ST_SCALED_HEIGHT); i < SCREENHEIGHT; i++)
      {
        R_VideoErase (0, i, side);
        R_VideoErase (ST_SCALED_WIDTH+side, i, side);
      }
    }
  }

  if ( viewheight >= ( SCREENHEIGHT - ST_SCALED_HEIGHT ))
    return; // if high-res, don�t go any further!

  top = ((SCREENHEIGHT-ST_SCALED_HEIGHT)-viewheight)/2;
  side = (SCREENWIDTH-scaledviewwidth)/2;

  // copy top
  for (i = 0; i < top; i++)
    R_VideoErase (0, i, SCREENWIDTH);

  // copy sides
  for (i = top; i < (top+viewheight); i++) {
    R_VideoErase (0, i, side);
    R_VideoErase (viewwidth+side, i, side);
  }

  // copy bottom
  for (i = top+viewheight; i < (SCREENHEIGHT - ST_SCALED_HEIGHT); i++)
    R_VideoErase (0, i, SCREENWIDTH);
}
