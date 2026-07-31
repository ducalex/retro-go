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
#ifndef _rt_rand_public
#define _rt_rand_public

#include "develop.h"

void  InitializeRNG ( void );
int   GetRandomSeed ( void );


#if RANDOMTEST

int   GameRNG ( char * string, int val );
#define GameRandomNumber(string,val)   GameRNG(string, val)

int   RNG ( char * string, int val );
#define RandomNumber(string,val)   RNG(string, val)

#else

int   GameRNG ( void );
#define GameRandomNumber(string,val)   GameRNG()

int   RNG ( void );
#define RandomNumber(string,val)   RNG()

#endif



void  SetRNGindex ( int i );
int   GetRNGindex ( void );
#endif
