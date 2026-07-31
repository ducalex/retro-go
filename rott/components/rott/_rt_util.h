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
#ifndef _rt_util_private
#define _rt_util_private

#ifdef DOS
#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA        0x3c9
#define PEL_MASK        0x3c6
#endif

#define ERRORROW        2
#define ERRORCOL        11

#define ERRORFILE       ("rotterr.txt")
#define SOFTERRORFILE   ("error.txt")
#define DEBUGFILE       ("rott.dbg")
#define MAPDEBUGFILE    ("mapinfo.txt")

#define SGN(x)          ((x>0) ? (1) : ((x==0) ? (0) : (-1)))

#define SLASHES         ('\\')
#define MAXCHARS        8

#define WeightR  3
#define WeightG  5
#define WeightB  2

#endif
