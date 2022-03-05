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

#include "doomtype.h"
#include "r_filter.h"
#include "z_zone.h"

#define DMR 16
byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM] = {
   {0*DMR, 14*DMR,  3*DMR, 13*DMR}, {11*DMR,  5*DMR, 8*DMR,  6*DMR},
  {12*DMR,  2*DMR, 15*DMR,  1*DMR},  {7*DMR,  9*DMR, 4*DMR, 10*DMR}
};

byte *filter_roundedUVMap;
byte filter_roundedRowMap[4*16];

void R_FilterInit(void) {
  int i,j,s,t;

	
	filter_roundedUVMap=malloc(FILTER_UVDIM*FILTER_UVDIM);

  // scale2x takes the following source:
  // A B C
  // D E F
  // G H I
  //
  // and doubles the size of E to produce:
  // E0 E1
  // E2 E3
  //
  //  E0 = D == B && B != F && D != H ? D : E;
  //  E1 = B == F && B != D && F != H ? F : E;
  //  E2 = D == H && D != B && H != F ? D : E;
  //  E3 = H == F && D != H && B != F ? F : E;
  //
  // to make this comparison regimen faster, we encode source color
  // equivalency into a single byte with the getCode() macro
  //
  // #define getCode(b,f,h,d) ( (b == f)<<0 | (f == h)<<1 | (h == d)<<2 | (d == b)<<3 )

  // encode the scale2x conditionals into a lookup code
  for (i=0; i<16; i++) {
    //  E0 = D == B && B != F && D != H ? D : E; // 10-0 => 1000 or 1010 => 8 or A
    filter_roundedRowMap[0*16+i] = (i == 0x8 || i == 0xA) ? 0 : 1;
    //  E1 = B == F && B != D && F != H ? F : E; // 0-01 => 0101 or 0001 => 5 or 1
    filter_roundedRowMap[1*16+i] = (i == 0x5 || i == 0x1) ? 2 : 1;
    //  E2 = D == H && D != B && H != F ? D : E; // 010- => 0101 or 0100 => 5 or 4
    filter_roundedRowMap[2*16+i] = (i == 0x4 || i == 0x5) ? 0 : 1;
    //  E3 = H == F && D != H && B != F ? F : E; // -010 => 1010 or 0010 => A or 2
    filter_roundedRowMap[3*16+i] = (i == 0xA || i == 0x2) ? 2 : 1;
  }

  // fill the uvMap. this will return:
  // 0/\1x
  // /4 \x
  // \  /x
  // 2\/3x
  // .. based on the uv coordinates
  for (i=0; i<FILTER_UVDIM; i++) {
    for (j=0; j<FILTER_UVDIM; j++) {
      s = (FILTER_UVDIM/2) - i;
      t = (FILTER_UVDIM/2) - j;
      if (s>=0 && t>=0)      filter_roundedUVMap[i*FILTER_UVDIM+j] = (s+t > FILTER_UVDIM/2) ? 0 : 4;
      else if (s>=0 && t<=0) filter_roundedUVMap[i*FILTER_UVDIM+j] = (s-t > FILTER_UVDIM/2) ? 2 : 4;
      else if (s<=0 && t>=0) filter_roundedUVMap[i*FILTER_UVDIM+j] = (-s+t > FILTER_UVDIM/2) ? 1 : 4;
      else if (s<=0 && t<=0) filter_roundedUVMap[i*FILTER_UVDIM+j] = (-s-t > FILTER_UVDIM/2) ? 3 : 4;
      else filter_roundedUVMap[i*FILTER_UVDIM+j] = 4;
    }
  }
}

byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d) {
  // A B C
  // D E F
  // G H I
  // perform the Scale2x algorithm (quickly) to get the new quad to represent E
  static byte quad[5];
  static byte rowColors[3];
  int code;
  
  rowColors[0] = d;
  rowColors[1] = e;
  rowColors[2] = f;
  
  #define getCode(b,f,h,d) ( (b == f)<<0 | (f == h)<<1 | (h == d)<<2 | (d == b)<<3 )

  code = getCode(b,f,h,d);
  quad[0] = rowColors[filter_roundedRowMap[0*16+code]];
  quad[1] = rowColors[filter_roundedRowMap[1*16+code]];
  quad[2] = rowColors[filter_roundedRowMap[2*16+code]];
  quad[3] = rowColors[filter_roundedRowMap[3*16+code]];
  quad[4] = e;
  
  return quad;
}
