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
//***************************************************************************
//
// Public header for RT_STR.C.
//
//***************************************************************************

#ifndef _rt_str_public
#define _rt_str_public

#include "lumpy.h"


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern int fontcolor;


//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************

typedef  struct
{
   int   x,y;
} Point;

typedef  struct
{
   int   x, y,
         w, h,
         px, py;
} WindowRec;            // Record used to save & restore screen windows

typedef  struct
{
	Point ul,lr;
} Rect;


//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

//
// String rtns
//

void VW_DrawClippedString (int x, int y, const char *string);
void US_ClippedPrint (int x, int y, const char *s);

void VWB_DrawPropString  (const char *string);
void VW_MeasurePropString (const char *string, int *width, int *height);

void US_MeasureStr (int *width, int *height, const char * s, ...) __attribute__((format(printf,3,4)));

void VW_DrawPropString (const char *string);

void US_SetPrintRoutines (void (*measure)(const char *, int *, int *, font_t *),
                          void (*print)(const char *));
void US_Print (const char *s);
void US_BufPrint (const char *s);
void US_PrintUnsigned (unsigned long int n);
void US_PrintSigned (long int n);
void USL_PrintInCenter (const char *s, Rect r);
void US_PrintCentered (const char *s);
void US_CPrintLine (const char *s);
void US_CPrint (const char *s);


//
// Input rtns
//

boolean US_LineInput (int x, int y, char *buf, const char *def, boolean escok,
                      int maxchars, int maxwidth, int color);
boolean US_lineinput (int x, int y, char *buf, const char *def, boolean escok,
                      int maxchars, int maxwidth, int color);
int CalibrateJoystick(void);

//
// Window rtns
//

void US_DrawWindow (int x, int y, int w, int h);
void US_CenterWindow (int w, int h);

//
// Intensity font rtns
//

void DrawIString (unsigned short int x, unsigned short int y, const char *string, int flags);
void DrawIntensityString (unsigned short int x, unsigned short int y, const char *string, int color);
void VW_MeasureIntensityPropString (const char *string, int *width, int *height);
byte GetIntensityColor (byte pix);

#include "myprint.h"

#endif
