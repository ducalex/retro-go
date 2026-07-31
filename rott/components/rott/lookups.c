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
#include "rt_util.h"
#include "rt_view.h"
#include <math.h>
#include <dos.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <stdlib.h>
#include <sys\stat.h>

//MED
#include "memcheck.h"

#define PANGLES 512
#define NUMSINANGLES FINEANGLES+FINEANGLEQUAD+1

#ifndef RETRO_GO
fixed  pangle[PANGLES];
long   sintable[NUMSINANGLES];
short  tantable[FINEANGLES];
byte   gammatable[GAMMAENTRIES];
#endif

extern  int      _argc;
extern  char **  _argv;

/*
=================
=
= Error
=
= For abnormal program terminations
=
=================
*/

void Error (char *error, ...)
{
	va_list	argptr;

	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	exit (1);
}


int SafeOpenWrite (char *_filename)
{
	int	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}


void SafeWrite (int handle, void *buffer, long count)
{
	unsigned	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,iocount) != iocount)
			Error ("File write failure");
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
}




void CalcPixelAngles ( void )
{
    int   i;
    long  intang;
    double  angle;
    double  tang;

    const   double radtoint = (double)FINEANGLES/2/PI;


    for (i=0;i<PANGLES;i++)
       {
       // start 1/2 pixel over, so viewangle bisects two middle pixels
       //tang = ((((double)i*160.0)+80.0)/(FPFOCALWIDTH*(double)PANGLES));
       tang = ((((double)i*160.0)+80.0)/(dGLOBAL_FPFOCALWIDTH*(double)PANGLES));
       angle = atan(tang);
       intang = ((long)(angle*radtoint));
       pangle[i] = intang;
       }
}



void BuildSinTable (void)
{
   int   i;
   double  angle,anglestep;
   double  sinangle;
   fixed  value;

   angle = 0;
   anglestep = (double)(PI/2/FINEANGLEQUAD);
   for (i=0;i<=FINEANGLEQUAD;i++)
      {
      sinangle=sin(angle);
      value=(fixed)((double)GLOBAL1*sinangle);
      sintable[i]     =
      sintable[i+FINEANGLES]  =
      sintable[FINEANGLES/2-i] = value;
      sintable[FINEANGLES-i] = -value;
      sintable[FINEANGLES/2+i] = -value;
      angle += anglestep;
      }
}

void BuildTanTable (void)
{
   int   i;
   double  angle,anglestep;
   double  tanangle;
   fixed  value;

   angle = 0;
   anglestep = (double)(PI*2/FINEANGLES);
   for (i=0;i<FINEANGLES;i++)
      {
      tanangle=tan(angle);
      value=(fixed)((double)GLOBAL1*tanangle);
      tantable[i] =(short) (value>>1);
      angle += anglestep;
      }
}

void BuildGammaTable (void)
{
   int     l, i, inf;
   int     j;
   int     gGamma=0x100;
   j=0;
   for (l=0 ; l<NUMGAMMALEVELS ; l++,gGamma+=32)
      {
      double nGamma = (double)256 / gGamma;
      double nScale = (double)63  / pow(63, nGamma);

      for ( i = 0; i < 64; i++ )
         {
         inf = pow(i, nGamma) * nScale;
         if (inf < 0)
            inf = 0;
         if (inf > 63)
            inf = 63;
         gammatable[j++]=inf;
         }
      }
}

void main ()
{
   int handle;
   int size;

   if (_argc!=2)
      {
      printf("LOOKUPS -- Apogee Software (c) 1994\n");
      printf("\n USAGE:    lookups <name.dat>\n");
      exit(0);
      }
   handle=SafeOpenWrite (_argv[1]);
   CalcPixelAngles();
   BuildSinTable();
   BuildTanTable();
   BuildGammaTable();
   size=PANGLES;
   SafeWrite(handle,&size,sizeof(int));
   SafeWrite(handle,&pangle[0],sizeof(fixed)*size);
   size=NUMSINANGLES;
   SafeWrite(handle,&size,sizeof(int));
   SafeWrite(handle,&sintable[0],sizeof(fixed)*size);
   size=FINEANGLES;
   SafeWrite(handle,&size,sizeof(int));
   SafeWrite(handle,&tantable[0],sizeof(short)*size);
   size=GAMMAENTRIES;
   SafeWrite(handle,&size,sizeof(int));
   SafeWrite(handle,&gammatable[0],sizeof(byte)*size);
   close (handle);
}
