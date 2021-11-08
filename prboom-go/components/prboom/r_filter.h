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

#ifndef R_FILTER_H
#define R_FILTER_H

#define DITHER_DIM 4

extern byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM];
#define FILTER_UVBITS 6
#define FILTER_UVDIM (1<<FILTER_UVBITS)
extern byte *filter_roundedUVMap;
//extern byte filter_roundedUVMap[FILTER_UVDIM*FILTER_UVDIM];
extern byte filter_roundedRowMap[4*16];

void R_FilterInit(void);

// Use the dither matrix to determine whether a pixel is on or off based
// on the overall intensity we're trying to simulate
#define filter_getDitheredPixelLevel(x, y, intensity) \
  ((filter_ditherMatrix[(y)&(DITHER_DIM-1)][(x)&(DITHER_DIM-1)] < (intensity)) ? 1 : 0)

// Choose current pixel or next pixel down based on dither of the fractional
// texture V coord. texV is not a true fractional texture coord, so it
// has to be converted using ((texV) - dcvars.yl) >> 8), which was empirically
// derived. the "-dcvars.yl" is apparently required to offset some minor
// shaking in coordinate y-axis and prevents dithering seams
#define FILTER_GETV(x,y,texV,nextRowTexV) \
  (filter_getDitheredPixelLevel(x, y, (((texV) - yl) >> 8)&0xff) ? ((nextRowTexV)>>FRACBITS) : ((texV)>>FRACBITS))

// Choose current column or next column to the right based on dither of the 
// fractional texture U coord
#define filter_getDitheredForColumn(x, y, texV, nextRowTexV) \
  dither_sources[(filter_getDitheredPixelLevel(x, y, filter_fracu))][FILTER_GETV(x,y,texV,nextRowTexV)]

#define filter_getRoundedForColumn(texV, nextRowTexV) \
  filter_getScale2xQuadColors( \
    source[      ((texV)>>FRACBITS)              ], \
    source[      (MAX(0, ((texV)>>FRACBITS)-1))  ], \
    nextsource[  ((texV)>>FRACBITS)              ], \
    source[      ((nextRowTexV)>>FRACBITS)       ], \
    prevsource[  ((texV)>>FRACBITS)              ] \
  ) \
    [ filter_roundedUVMap[ \
      ((filter_fracu>>(8-FILTER_UVBITS))<<FILTER_UVBITS) + \
      ((((texV)>>8) & 0xff)>>(8-FILTER_UVBITS)) \
    ] ]

#define filter_getRoundedForSpan(texU, texV) \
  filter_getScale2xQuadColors( \
    source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)            ], \
    source[ (((texU)>>16)&0x3f) | ((((texV)-FRACUNIT)>>10)&0xfc0) ], \
    source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0) ], \
    source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0) ], \
    source[ ((((texU)-FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0) ] \
  ) \
    [ filter_roundedUVMap[ \
      (((((texU)>>8) & 0xff)>>(8-FILTER_UVBITS))<<FILTER_UVBITS) + \
      ((((texV)>>8) & 0xff)>>(8-FILTER_UVBITS)) \
    ] ]

byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d);

// This is the horrendous macro version of the function commented out of 
// r_filter.c. It does a bilinear blend on the four source texels for a 
// given u and v
#define filter_getFilteredForColumn32(depthmap, texV, nextRowTexV) ( \
  VID_PAL32( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

// The 16 bit method of the filtering doesn't really maintain enough 
// accuracy for discerning viewers, but the alternative requires converting
// from 32 bit, which is slow and requires both the intPalette and the 
// shortPalette to be in memory at the same time.
#define filter_getFilteredForColumn16(depthmap, texV, nextRowTexV) ( \
  VID_PAL16( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

#define filter_getFilteredForColumn15(depthmap, texV, nextRowTexV) ( \
  VID_PAL15( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

// Same as for column but wrapping at 64
#define filter_getFilteredForSpan32(depthmap, texU, texV) ( \
  VID_PAL32( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

// Use 16 bit addition here since it's a little faster and the defects from
// such low-accuracy blending are less visible on spans
#define filter_getFilteredForSpan16(depthmap, texU, texV) ( \
  VID_PAL16( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

#define filter_getFilteredForSpan15(depthmap, texU, texV) ( \
  VID_PAL15( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

// do red and blue at once for slight speedup

#define GETBLENDED15_5050(col1, col2) \
  ((((col1&0x7c1f)+(col2&0x7c1f))>>1)&0x7c1f) | \
  ((((col1&0x03e0)+(col2&0x03e0))>>1)&0x03e0)

#define GETBLENDED16_5050(col1, col2) \
  ((((col1&0xf81f)+(col2&0xf81f))>>1)&0xf81f) | \
  ((((col1&0x07e0)+(col2&0x07e0))>>1)&0x07e0)

#define GETBLENDED32_5050(col1, col2) \
  ((((col1&0xff00ff)+(col2&0xff00ff))>>1)&0xff00ff) | \
  ((((col1&0x00ff00)+(col2&0x00ff00))>>1)&0x00ff00)

#define GETBLENDED15_3268(col1, col2) \
  ((((col1&0x7c1f)*5+(col2&0x7c1f)*11)>>4)&0x7c1f) | \
  ((((col1&0x03e0)*5+(col2&0x03e0)*11)>>4)&0x03e0)

#define GETBLENDED16_3268(col1, col2) \
  ((((col1&0xf81f)*5+(col2&0xf81f)*11)>>4)&0xf81f) | \
  ((((col1&0x07e0)*5+(col2&0x07e0)*11)>>4)&0x07e0)

#define GETBLENDED32_3268(col1, col2) \
  ((((col1&0xff00ff)*5+(col2&0xff00ff)*11)>>4)&0xff00ff) | \
  ((((col1&0x00ff00)*5+(col2&0x00ff00)*11)>>4)&0x00ff00)

#define GETBLENDED15_9406(col1, col2) \
  ((((col1&0x7c1f)*15+(col2&0x7c1f))>>4)&0x7c1f) | \
  ((((col1&0x03e0)*15+(col2&0x03e0))>>4)&0x03e0)

#define GETBLENDED16_9406(col1, col2) \
  ((((col1&0xf81f)*15+(col2&0xf81f))>>4)&0xf81f) | \
  ((((col1&0x07e0)*15+(col2&0x07e0))>>4)&0x07e0)

#define GETBLENDED32_9406(col1, col2) \
  ((((col1&0xff00ff)*15+(col2&0xff00ff))>>4)&0xff00ff) | \
  ((((col1&0x00ff00)*15+(col2&0x00ff00))>>4)&0x00ff00)

#endif
