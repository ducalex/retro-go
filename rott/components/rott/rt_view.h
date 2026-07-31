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
//    RT_VIEW.H
//
//***************************************************************************
#ifndef _rt_view_public
#define _rt_view_public

#include "modexlib.h"

#define HEIGHTFRACTION 6
#define MAXVIEWSIZES   11

//#define FOCALWIDTH 160//160
//#define FPFOCALWIDTH 160.0//160.0
 

#define NUMGAMMALEVELS 8
#define GAMMAENTRIES (64*8)

#define GENERALNUMLIGHTS (5)

extern int StatusBar;

// Kill count
#define STATUS_KILLS        0x1
// Transparent health bar
#define STATUS_PLAYER_STATS 0x2
// Bottom status bar
#define BOTTOM_STATUS_BAR   0x4
// Top status bar
#define TOP_STATUS_BAR      0x8

#define SHOW_KILLS()             ( StatusBar & STATUS_KILLS )
#define SHOW_PLAYER_STATS()      ( StatusBar & STATUS_PLAYER_STATS )
#define SHOW_BOTTOM_STATUS_BAR() ( StatusBar & BOTTOM_STATUS_BAR )
#define SHOW_TOP_STATUS_BAR()    ( StatusBar & TOP_STATUS_BAR )

#define YOURCPUSUCKS_Y      ( 100 + 48 / 2 + 2 )
#define YOURCPUSUCKS_HEIGHT 8

#define MAXPLAYERCOLORS (11)
typedef enum
   {
   pc_gray,
   pc_brown,
   pc_black,
   pc_tan,
   pc_red,
   pc_olive,
   pc_blue,
   pc_white,
   pc_green,
   pc_purple,
   pc_orange
   } playercolors;

extern byte * playermaps[MAXPLAYERCOLORS];
//extern short  pixelangle[MAXVIEWWIDTH];
extern short  *pixelangle;
extern byte   gammatable[GAMMAENTRIES];
extern int    gammaindex;
extern byte   uniformcolors[MAXPLAYERCOLORS];

extern  byte mapmasks1[4][9];                   // Map Mask for post scaling
extern  byte mapmasks2[4][9];                   // Map Mask for post scaling
extern  byte mapmasks3[4][9];                   // Map Mask for post scaling

extern  int normalshade;                        // Normal shading level for stuff
extern  int maxshade;                           // max shading level
extern  int minshade;                           // min shading level
extern  int baseminshade;
extern  int basemaxshade;
extern  int    viewheight;
extern  int    viewwidth;
extern  longword heightnumerator;
extern  fixed  scale;
extern  int    screenofs;
extern  int    centerx;
extern  int centery;
extern  int centeryfrac;
extern  int fulllight;
extern  byte * colormap;
extern  byte * greenmap;
extern  byte * redmap;
extern  int weaponscale;
extern  int viewsize;
extern  int focalwidth;
extern  int yzangleconverter;
extern  int lightninglevel;
extern  boolean  lightning;
extern  int    darknesslevel;

void DrawCPUJape( void );
void SetupScreen ( boolean flip );
void ResetFocalWidth ( void );
void ChangeFocalWidth ( int amount );
void SetViewSize ( int size );
void LoadColorMap( void );
void UpdateLightLevel (int area);
void SetIllumination (int level);
int  GetIlluminationDelta (void);
void UpdateLightning (void);
void SetLightLevels ( int darkness );
void SetupLightLevels ( void );
void SetLightRate ( int rate );
int GetLightRate ( void );
void SetModemLightLevel ( int type );
int GetLightRateTile ( void );
int GetLightLevelTile ( void );

#endif
