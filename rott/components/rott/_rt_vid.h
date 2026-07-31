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
#ifndef _rt_vid_private
#define _rt_vid_private

//******************************************************************************
//
// Private header for RT_VID.C
//
//******************************************************************************


//******************************************************************************
//
// DEFINES
//
//******************************************************************************

#define PIXTOBLOCK         4

#define VW_Hlin(x,z,y,c)   VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)   VL_Vlin(x,y,(z)-(y)+1,c)

#define VW_THlin(x,z,y,up)    VL_THlin(x,y,(z)-(x)+1, up)
#define VW_TVlin(y,z,x,up)    VL_TVlin(x,y,(z)-(y)+1, up)


#endif
