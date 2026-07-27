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

#ifndef __mouse_h
#define __mouse_h
#ifdef __cplusplus
extern "C" {
#endif

#define LEFT_MOUSE   1
#define RIGHT_MOUSE  2
#define MIDDLE_MOUSE 4
#define LEFT_MOUSE_PRESSED( button ) ( ( ( button ) & LEFT_MOUSE ) != 0 )
#define RIGHT_MOUSE_PRESSED( button ) ( ( ( button ) & RIGHT_MOUSE ) != 0 )
#define MIDDLE_MOUSE_PRESSED( button ) ( ( ( button ) & MIDDLE_MOUSE ) != 0 )

boolean MOUSE_Init( void );
void    MOUSE_Shutdown( void );
void    MOUSE_ShowCursor( void );
void    MOUSE_HideCursor( void );
int32   MOUSE_GetButtons( void );
void    MOUSE_GetPosition( int32*x, int32*y  );
void    MOUSE_GetDelta( int32*x, int32*y  );

#ifdef __cplusplus
};
#endif
#endif /* __mouse_h */
