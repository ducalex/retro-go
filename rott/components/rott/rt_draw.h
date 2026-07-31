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
#ifndef _rt_draw_public
#define _rt_draw_public

//***************************************************************************
//
//    RT_DRAW.C - Draw-o-rama
//
//***************************************************************************


#define MAXVISIBLE              256

extern int whereami;

extern byte * shadingtable;            // Shading table for DrawPost

typedef struct
{
int viewheight;
int viewx;
int shapenum;
int altshapenum;
int shapesize;
int x1,x2,h1,h2;
int texturestart;
int textureend;
byte * colormap;

} visobj_t;


extern  word             (*tilemap)[MAPSIZE];    // wall values only
extern  byte             (*spotvis)[MAPSIZE];

extern int tics;
extern int wstart;
extern int fandc;

//
// math tables
//
extern short    *tantable;
extern fixed    *sintable;
extern fixed    *costable;

//
// refresh variables
//

extern fixed   viewx,viewy;             // the focal point
extern int     viewangle;
extern fixed   viewsin,viewcos;

//
// ray tracing variables
//

extern visobj_t *vislist;
extern visobj_t *visptr,*visstep,*farthest;

extern long     xintercept,yintercept;
extern byte     (*mapseen)[MAPSIZE];
extern unsigned long * lights;

extern int hp_startfrac;
extern int hp_srcstep;
extern int levelheight;
extern int maxheight;
extern int nominalheight;

extern int actortime;
extern int drawtime;
extern int c_startx;
extern int c_starty;

extern const int dirangle8[9];
extern const int dirangle16[16];
extern  int firstcoloffset;

//=========================== macros =============================

#define LightSourceAt(x,y)    (*(lights+((x)<<7)+(y)))
#define SetLight(x,y,level)   (LightSourceAt((x),(y))|=(unsigned long)(level))

//=========================== functions =============================

void  BuildTables (void);
void  CalcTics (void);
void  ThreeDRefresh (void);
void  FlipPage ( void );
void  TurnShakeOff( void );
void  AdaptDetail ( void );
int   CalcHeight (void);
void  DoLoadGameSequence( void );
void RotateBuffer (int startangle, int endangle, int startscale, int endscale, int time);
void ApogeeTitle (void);
void DopefishTitle (void);
void RotationFun (void);
void GetRainBoundingBox (int * xmin, int * xmax, int * ymin, int * ymax);
void StartupScreenSaver ( void );
void ShutdownScreenSaver ( void );
void UpdateScreenSaver ( void );
void DoEndCinematic ( void );
void DoCreditScreen ( void );
void DoMicroStoryScreen ( void );
void DoInBetweenCinematic (int yoffset, int lump, int delay, char * string );

#endif
