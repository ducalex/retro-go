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
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#ifndef _develop_public
#define _develop_public
#ifdef __cplusplus
extern "C" {
#endif

#define DEVELOPMENT          0
#define SHAREWARE            0
#define LOCATIONINFO         1
#define SOFTWAREERROR        1
#define MEMORYCORRUPTIONTEST 1
#define PRECACHETEST         0
#define DATACORRUPTIONTEST   0
#define RANDOMNUMBERTEST     0


#if ( LOCATIONINFO == 1 )

#define funcstart() \
   { \
   SoftError( "funcstart : module '%s' at line %d.\n", __FILE__, __LINE__ );\
   }

#define funcend() \
   { \
   SoftError( "  funcend : module '%s' at line %d.\n", __FILE__, __LINE__ );\
   }

#else

#define funcstart()
#define funcend()

#endif

#ifdef __cplusplus
};
#endif
#endif
