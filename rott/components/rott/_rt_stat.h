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
#ifndef _rt_stat_private
#define _rt_stat_private

#include "rt_stat.h"

void        AddStatic(statobj_t*);
void        AddAnimStatic(statobj_t*);
void        PreCacheStaticSounds(int);

#define SOLIDCOLORTICTIME   1
#define SOLIDCOLORINCREMENT 1
#define MAXFIRECOLOR      248
#define INITIALFIRECOLOR  246
#define IsLight(x,y)          ( (x>=0) && (x<=127) && (y>=0) && (y<=127) &&   \
										  (sprites[x][y]) && (sprites[x][y]->flags & FL_LIGHT) )


typedef struct sas
{
  int          x,y,z;
  int          flags;
  signed char  ticcount;
  int          hitpoints;
  short int    shapenum;
  signed char  ammo;
  signed char  count;
  signed char  itemnumber;
  short int    areanumber;
  short int    whichstat;
  byte         numanims;
  int          linked_to;
}saved_stat_type;

#endif
