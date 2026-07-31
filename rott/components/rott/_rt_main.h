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
#ifndef _rt_main_private
#define _rt_main_private

#include "develop.h"

void GameLoop (void);
void PlayLoop (void);
void PollKeyboard(void);
void FixColorMap( void );

#define QUITTIMEINTERVAL ((35*6)-5)
boolean CheckForQuickLoad ( void );

#if SAVE_SCREEN

void WriteLBMfile (char *filename, byte *data, int width, int height);
void GetFileName (boolean saveLBM);
void DrawRottTitle ( void );

void WritePCX (char * file, byte * source);
int PutBytes (unsigned char *ptr, unsigned int bytes);

typedef struct
{
	unsigned short w,h;
	short          x,y;
	unsigned char  nPlanes;
	unsigned char  masking;
	unsigned char  compression;
	unsigned char  pad1;
	unsigned short transparentColor;
	unsigned char  xAspect,yAspect;
	short          pageWidth,pageHeight;
} bmhd_t;


typedef struct {
   unsigned char manufacturer;
   unsigned char version;
   unsigned char encoding;
   unsigned char bitsperpixel;
   unsigned short int  xmin, ymin, xmax, ymax;
   unsigned short int  hres, vres;
   unsigned char colormap[16][3];
   unsigned char reserved;
   unsigned char nplanes;
   unsigned short int  bytesperline;
} PCX_HEADER;

#define GAP_SIZE  (128 - sizeof (PCX_HEADER))


#endif

#endif
