/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef _rt_build_private
#define _rt_build_private

#define MAX(x,y)               ((x>y) ? (x) : (y))
#define MAXPLANES 10

// Should be 10 with titles
#define MENUOFFY     (10)
#define MENUBACKNAME ("plane")
#define MENUTITLEY 10
#define TEXTUREW     (288)
#define TEXTUREWIDTH ((TEXTUREW*1024)-1)
#define TEXTUREHEIGHT (158)
#define NORMALVIEW   (0x40400L)
#define NORMALHEIGHTDIVISOR   (156000000)
#define NORMALWIDTHMULTIPLIER (241)
#define FLIPTIME     20//60


typedef struct
{
   int   x1, y1;
   int   x2, y2;
   int   texturewidth;
   int   texture;
   int   origheight;
} plane_t;

#endif
