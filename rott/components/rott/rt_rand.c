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
#include "rt_def.h"
#include "_rt_rand.h"
#include "rt_rand.h"
#include "develop.h"
#include "rt_util.h"
#include <time.h>

#if (DEVELOPMENT == 1)
#include "rt_main.h"
#endif
//MED
#include "memcheck.h"

//****************************************************************************
//
// GLOBALS
//
//****************************************************************************

static int rndindex;
static int sndindex = 0;
// Pointer defined in rg_memory.c


//****************************************************************************
//
// GetRandomSeed ()
//
//****************************************************************************

int GetRandomSeed ( void )
{
   return ( time (NULL) % (SIZE_OF_RANDOM_TABLE) );
}

//****************************************************************************
//
// InitializeRNG ()
//
//****************************************************************************

void  InitializeRNG ( void )
{
   SetRNGindex(GetRandomSeed());
   sndindex=GetRandomSeed();
}

//****************************************************************************
//
// SetRNGindex ()
//
//****************************************************************************

void  SetRNGindex ( int i )
{
   rndindex=i;
//#if (DEVELOPMENT == 1)
   SoftError("RNG index set at %d\n",i);
//#endif
}

//****************************************************************************
//
// GetRNGindex ()
//
//****************************************************************************

int GetRNGindex ( void )
{
   return rndindex;
}


#if (RANDOMTEST==1)
//****************************************************************************
//
// int GameRNG ( char * string, int val )
//
//****************************************************************************
int   GameRNG ( char * string, int val )
{
   rndindex = (rndindex+1)&(SIZE_OF_RANDOM_TABLE-1);
   SoftError("RNG - num=%3d called from=%s val=%d\n",RandomTable[rndindex],string,val);
   return RandomTable[rndindex];
}
#else
//****************************************************************************
//
// int GameRNG (void)
//
//****************************************************************************
int   GameRNG ( void )
{
   rndindex = (rndindex+1)&(SIZE_OF_RANDOM_TABLE-1);

   return RandomTable[rndindex];
}
#endif



#if (RANDOMTEST==1)
//****************************************************************************
//
// int RNG ( char * string, int val )
//
//****************************************************************************

int   RNG ( char * string, int val )
{
   sndindex = (sndindex+1)&(SIZE_OF_RANDOM_TABLE-1);
//   SoftError("SRNG - num=%3ld called from=%s val=%ld\n",RandomTable[sndindex],string,val);
   return RandomTable[sndindex];
}
#else
//****************************************************************************
//
// int RNG (void)
//
//****************************************************************************

int   RNG( void )
{
   sndindex = (sndindex+1)&(SIZE_OF_RANDOM_TABLE-1);

   return RandomTable[sndindex];
}
#endif

