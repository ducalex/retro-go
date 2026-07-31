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
//***************************************************************************
//
//    RT_SCALE.H - Scale-o-rama
//
//***************************************************************************
#ifndef _rt_scale_public
#define _rt_scale_public

#include "rt_draw.h"

extern  int dc_texturemid;
extern  int dc_iscale;
extern  int dc_invscale;
extern  int centeryclipped;
extern  int sprtopoffset;
extern  int dc_yl;
extern  int dc_yh;
extern  byte * dc_source;
extern  int transparentlevel;

void ScaleShape (visobj_t * vis);
void ScaleWeapon (int xcent, int yoffset, int shapenum);
void DrawScreenSprite (int x, int y, int shapenum);
void SetLightLevel (int height);
void ScaleMaskedPost (byte * src, byte * buf);
void DrawScreenSizedSprite (int lump);
void ScaleTransparentPost (byte * src, byte * buf, int level);
void ScaleTransparentShape (visobj_t * sprite);
void ScaleSolidShape (visobj_t * sprite);
void DrawUnScaledSprite (int x, int y, int shapenum, int shade);
void DrawPositionedScaledSprite (int x, int y, int shapenum, int height, int type);
void DrawNormalSprite (int x, int y, int shapenum);

#endif
