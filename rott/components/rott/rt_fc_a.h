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
#ifndef _rt_fc_a_public
#define _rt_fc_a_public

//***************************************************************************
//
//    RT_FC_A.ASM - Maprow stuff for floor and ceiling
//
//***************************************************************************

void DrawSkyPost (byte * buf, byte * src, int height);
void DrawRow(int count, byte * dest, byte * src);
void DrawRotRow(int count, byte * dest, byte * src);
void DrawMaskedRotRow(int count, byte * dest, byte * src);

#if (defined __WATCOMC__)
#pragma aux DrawSkyPost parm [EDI] [ESI] [ECX] modify exact [eax ecx edx edi esi ebx]
#pragma aux DrawRow parm [ECX] [EDI] [ESI] modify exact [eax ebx ecx edx esi edi]
#pragma aux DrawRotRow parm [ECX] [EDI] [ESI] modify exact [eax ebx ecx edx esi edi]
#pragma aux DrawMaskedRotRow parm [ECX] [EDI] [ESI] modify exact [eax ebx ecx edx esi edi]
#endif

#endif

