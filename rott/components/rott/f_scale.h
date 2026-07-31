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
#ifndef _f_scale_public
#define _f_scale_public

extern int cin_yl;
extern int cin_yh;
extern int cin_ycenter;
extern int cin_iscale;
extern int cin_texturemid;
extern byte * cin_source;

void R_DrawFilmColumn (byte * buf);
void DrawFilmPost (byte * buf, byte * src, int height);

#if (defined __WATCOMC__)
#pragma aux R_DrawFilmColumn parm [EDI] modify exact [eax ebx ecx edx esi edi]
#pragma aux DrawFilmPost parm [EDI] [ESI] [ECX] modify exact [eax ecx edx edi esi ebx]
#endif

#endif
