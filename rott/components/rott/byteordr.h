/*
Copyright (C) 2002 John R. Hall

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
//    byteordr.c - Byte order conversion routines.
//
//***************************************************************************

#ifndef BYTEORDER_H
#define BYTEORDER_H

typedef void (*converter_t)(void *, int);

#define DECLARE_CONVERTER(type) void Cvt_##type(void *lmp, int num);

DECLARE_CONVERTER(pic_t);
DECLARE_CONVERTER(lpic_t);
DECLARE_CONVERTER(font_t);
DECLARE_CONVERTER(lbm_t);
DECLARE_CONVERTER(patch_t);
DECLARE_CONVERTER(transpatch_t);
DECLARE_CONVERTER(cfont_t);
void CvtNull(void *lmp, int num);
converter_t CvtForType(int type);

#endif
