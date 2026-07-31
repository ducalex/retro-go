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
#ifndef _rt_draw_private
#define _rt_draw_private

#include "develop.h"

#define MINZ                            0x2700
#define MAXBOB                          0x9000

#define GOLOWER  0x38000
#define GOHIGHER 0x20000

#define MAXVISIBLEDOORS 30

#define DHEIGHTFRACTION 8

#define MINZ                            0x2700
#define MAXDRAWNTICS                    40

#define W_CHANGE  (WEAPONUPTICS || WEAPONDOWNTICS)

#if (SHAREWARE == 0)
 #define NUMWEAPGRAPHICS 16
#else
 #define NUMWEAPGRAPHICS 9
#endif

#define HFRACTION (6+HEIGHTFRACTION)

#define FIXEDTRANSLEVEL (30)

typedef struct
{
   int x;
   int y;
   int angle;
   int scale;
   int dx;
   int dy;
   int dangle;
   int dscale;
   int phase;
   int time;
   int pausetime;
   int pausex;
   int pausey;
} screensaver_t;

void  DrawPlayerWeapon(void);
boolean TransformPlane (int x1, int y1, int x2, int y2, visobj_t * plane);
int   CalcRotate (objtype *ob);
void  DrawScaleds (void);
void  FixOfs (void);
void SetSpriteLightLevel (int x, int y, visobj_t * sprite, int dir, int fullbright);

#endif
