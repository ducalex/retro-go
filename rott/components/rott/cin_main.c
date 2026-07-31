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
#ifdef DOS
#include <malloc.h>
#include <dos.h>
#include <io.h>
#include <conio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "cin_glob.h"
#include "scriplib.h"
#include "watcom.h"
#include "z_zone.h"
#include "w_wad.h"
#include "cin_main.h"
#include "modexlib.h"
#include "lumpy.h"

#include "cin_actr.h"
#include "cin_evnt.h"
#include "cin_efct.h"
//MED
#include "memcheck.h"

boolean cinematicdone;
static int cinematictime;
static int cinematictics;
static int cinematictictime;
static int profiletics=-1;

/*
================
=
= ProfileMachine
=
================
*/
void ProfileMachine ( void )
{
   int i;
   int time;
   int endtime;

   if (profiletics>0)
      return;
   time=GetCinematicTime();
   for (i=0;i<4;i++)
      {
      ProfileDisplay();
      }
   endtime=GetCinematicTime();

   profiletics = (endtime-time)>>2;
   if (profiletics<1)
      profiletics=1;
}

/*
================
=
= StartupCinematic
=
================
*/
void StartupCinematic ( void )
{
   StartupEvents ();
   StartupCinematicActors ();
   cinematicdone=false;
   cinematictime=0;
   GetCinematicTics ();
   ClearCinematicAbort();
   ProfileMachine();
}


/*
================
=
= ShutdownCinematic
=
================
*/
void ShutdownCinematic ( void )
{
   ShutdownEvents ();
   ShutdownCinematicActors ();
}


/*
================
=
= ParseCinematicScript
=
================
*/

void ParseCinematicScript (void)
{
   int time;

   time=0;
   do
      {
      //
      // get next command time
      //
      GetToken (true);
      if (endofscript)
         break;
      time+=ParseNum(token);
      ParseEvent ( time );
      }
   while (script_p < scriptend_p);
}


/*
==============
=
= CacheScriptFile
=
==============
*/

void CacheScriptFile (char *filename)
{
	long            size;
   int lump;

   lump=W_GetNumForName(filename);

   scriptbuffer=W_CacheLumpNum(lump,PU_CACHE, CvtNull, 1);
	size = W_LumpLength(lump);

	script_p = scriptbuffer;
	scriptend_p = script_p + size;
	scriptline = 1;
	endofscript = false;
	tokenready = false;
}


/*
=================
=
= GrabCinematicScript
=
=================
*/

void GrabCinematicScript (char const *basename, boolean uselumpy)
{
   char script[256];

//
// read in the script file
//
   strcpy (script, basename);
   strcat (script,".ms");
   if (uselumpy==false)
      LoadScriptFile (script);
   else
      CacheScriptFile ((char *)basename);

   ParseCinematicScript ();
}

/*
==============
=
= GetCinematicTics
=
==============
*/

void GetCinematicTics ( void )
{
   int time;

   time=GetCinematicTime();
   while (time==cinematictictime)
      {
      time=GetCinematicTime();
      }
   cinematictics=(time-cinematictictime);
   cinematictictime=time;
   // Use actual elapsed tics instead of inaccurate profiletics
   // cinematictics=profiletics;
}


void PlayMovie ( char * name, boolean uselumpy )
{
   int i;

   StartupCinematic ( );
   GrabCinematicScript (name, uselumpy);

   PrecacheCinematic ( );
   GetCinematicTics();
   while (cinematicdone==false)
      {
      cinematicdone=CinematicAbort();
#if DUMP
      printf("time=%ld\n",cinematictime);
#endif
      for (i=0;i<cinematictics;i++)
         {
         UpdateCinematicEvents ( cinematictime );
         UpdateCinematicActors ( );
         cinematictime++;
         }
      DrawCinematicActors ();
      GetCinematicTics();
      }

   ShutdownCinematic ();
}

#ifndef DOS
int cin_iscale;
byte *cin_source;
int cin_texturemid;
int cin_ycenter;
int cin_yh;
int cin_yl;

/* f_scale.asm */

void R_DrawFilmColumn (byte * buf)
{
	// This is *NOT* 100% correct - DDOI
	int count;
	int frac, fracstep;
	byte *dest;

	count = cin_yh - cin_yl;
	if (count < 0) return;

	dest = buf + ylookup[cin_yl];

	fracstep = cin_iscale;
	frac = cin_texturemid + (cin_yl-cin_ycenter)*fracstep;

	while (count--) {
		*dest = cin_source[(frac>>SFRACBITS)];
		dest += iGLOBAL_SCREENWIDTH;
		frac += fracstep;
	}
}

void DrawFilmPost (byte * buf, byte * src, int height)
{
	while (height--) { 
		*buf = *src;
		
		src++;
		
		buf += linewidth;
	}
}
#endif

