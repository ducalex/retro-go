/*
  Custom GBA BIOS replacement
  Copyright (c) 2002-2006 VBA Development Team
  Copyright (c) 2006-2013 VBA-M Development Team
  Copyright (c) 2013 Normmatt
	
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <tonc.h>

u32 umul3232H32(u32 val, u32 val2)
{
	register u32 a __asm ("r2");
	register u32 b __asm ("r3");
	__asm ("umull %0, %1, %2, %3" :
	     "=r"(a), "=r"(b) :
		 "r"(val), "r"(val2)
		 );
	return b;
}