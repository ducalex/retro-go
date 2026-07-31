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
#ifndef _rt_map_private
#define _rt_map_private

#if defined(__WATCOM__)
#pragma aux FastFill =  \
        "rep stosb"       \
        parm    [edi] [eax] [ecx] \
        modify exact [ecx]
#endif
void FastFill(byte * buf, int color, int count);


#define MAP_PLAYERCOLOR 4
#define MAP_MWALLCOLOR 13
#define MAP_PWALLCOLOR 8
#define MAP_AWALLCOLOR 9
#define MAP_WALLCOLOR 7
#define MAP_DOORCOLOR 3
#define MAP_SPRITECOLOR 2
#define MAP_ACTORCOLOR 15
#define MAP_SKYCOLOR 11

#define FULLMAP_SCALE 5

#endif
