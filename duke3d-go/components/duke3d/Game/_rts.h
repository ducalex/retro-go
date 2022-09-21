//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#ifndef __rts_private__
#define __rts_private__

//===============
//   TYPES
//===============

typedef struct
   {
   char  name[8];
   int32 handle,position,size;
   } lumpinfo_t;

typedef struct
   {
   char  identification[4];              // should be IWAD
   int32 numlumps;
   int32 infotableofs;
   } wadinfo_t;

typedef struct
   {
   int32 filepos;
   int32 size;
   char  name[8];
   } filelump_t;

#endif
