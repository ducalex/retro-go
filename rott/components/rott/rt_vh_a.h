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
#ifndef _rt_vh_a_public
#define _rt_vh_a_public

//***************************************************************************
//
// Public header for RT_VH_A.ASM.
//
//***************************************************************************

void VH_UpdateScreen (void);
void JoyStick_Vals (void);

#ifdef __WATCOMC__
#pragma aux JoyStick_Vals modify exact [eax ebx ecx edx esi edi]
#endif

#endif
