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
#include "rt_view.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lumpy.h"
#include "rt_util.h"
#include "rt_vid.h"
#include "rt_game.h"
#include "rt_draw.h"
#include "rt_ted.h"
#include "isr.h"
#include "rt_rand.h"
#include "rt_sound.h"
#include "modexlib.h"
#include "rt_menu.h"

#ifdef DOS
#include <mem.h>
#endif

#include <stdlib.h>

#include "rt_main.h"
#include "rt_battl.h"
#include "rt_floor.h"
#include "rt_str.h"
#include "watcom.h"
#include "develop.h"
//MED
#include "memcheck.h"

#define LIGHTNINGLEVEL 4
#define MINLIGHTNINGLEVEL   2
#define MAXLIGHTNINGLEVEL   10

/*
=============================================================================

                               GLOBALS

=============================================================================
*/
extern int G_weaponscale;


int    StatusBar = 0;
int    lightninglevel=0;
boolean  lightning=false;
int    normalshade;
int    darknesslevel;
int    maxshade;
int    minshade;
int    baseminshade;
int    basemaxshade;
int    viewheight;
int    viewwidth;
longword   heightnumerator;
fixed  scale;
int    screenofs;
int    centerx;
int    centery;
int    centeryfrac;
int    fulllight;
int    weaponscale;
int    viewsize;
byte * colormap;
byte * redmap;
byte * greenmap;
byte * playermaps[MAXPLAYERCOLORS];
//short  pixelangle[MAXVIEWWIDTH];
#ifndef RETRO_GO
short  pixelangle[800];
#endif
byte   gammatable[GAMMAENTRIES];
int    gammaindex;
int    focalwidth=160;
int    yzangleconverter;
byte   uniformcolors[MAXPLAYERCOLORS]={
                                       25,
                                       222,
                                       29,
                                       206,
                                       52,
                                       6,
                                       155,
                                       16,
                                       90,
                                       129,
                                       109
                                       };

#ifdef DOS
byte    mapmasks1[4][9] = {
{1 ,3 ,7 ,15,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

byte    mapmasks2[4][9] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15,15} };

byte    mapmasks3[4][9] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3 ,7},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7 ,15} };
#endif


/*
=============================================================================

                               LOCAL

=============================================================================
*/

static char *YourComputerSucksString = "Buy a 486! :)";

static int viewsizes[MAXVIEWSIZES*2]={ 80,48,
                           128,72,
                           160,88,
                           192,104,
                           224,120,
                           256,136,
                           288,152,
                           320,168,
                           320,184,
                           320,200,
                           320,200};

static int ColorMapLoaded=0;

static int      lightningtime=0;
static int      lightningdelta=0;
static int      lightningdistance=0;
static int      lightningsoundtime=0;
static boolean  periodic=false;
static int      periodictime=0;

void SetViewDelta ( void );
void UpdatePeriodicLighting (void);

/*
====================
=
= ResetFocalWidth
=
====================
*/
void ResetFocalWidth ( void )
{
   focalwidth=iGLOBAL_FOCALWIDTH;//FOCALWIDTH;
   SetViewDelta();
}

/*
====================
=
= ChangeFocalWidth
=
====================
*/
void ChangeFocalWidth ( int amount )
{
   focalwidth=iGLOBAL_FOCALWIDTH+amount;//FOCALWIDTH+amount;
   SetViewDelta();
}


/*
====================
=
= SetViewDelta
=
====================
*/

void SetViewDelta ( void )
{
//iGLOBAL_SCREENHEIGHT
//iGLOBAL_SCREENWIDTH
//
// calculate scale value for vertical height calculations
// and sprite x calculations
//
	if ( iGLOBAL_SCREENWIDTH == 320) {
		scale = (centerx*focalwidth)/(160);
	}else if ( iGLOBAL_SCREENWIDTH == 640) {
		scale = (centerx*focalwidth)/(160);
	}else if ( iGLOBAL_SCREENWIDTH == 800) {
		scale = (centerx*focalwidth)/(160);
	}
//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>HEIGHTFRACTION
//
   heightnumerator = (((focalwidth/10)*centerx*4096)<<HEIGHTFRACTION);
}

/*
====================
=
= CalcProjection
=
====================
*/

void CalcProjection ( void )
{
    int   i;
    int   frac;
    int   intang;
    byte * table;
    byte * ptr;
    int   length;
    int  * pangle;



// Already being called in ResetFocalWidth
//    SetViewDelta();

//
// load in tables file
//


//Hey, isn't this stuff already loaded in?
//Why don't we make this a lump?
   table=W_CacheLumpName("tables",PU_STATIC, CvtNull, 1);
   ptr=table;

//
// get size of table
//

   memcpy(&length,ptr,sizeof(int));
   SwapIntelLong(&length);
   ptr+=sizeof(int);
   pangle=SafeMalloc(length*sizeof(int));
   memcpy(pangle,ptr,length*sizeof(int));

   frac=((length*65536/centerx))>>1;
   for (i=0;i<centerx;i++)
      {
      // start 1/2 pixel over, so viewangle bisects two middle pixels
      intang=pangle[frac>>16];
      SwapIntelLong(&intang);
      pixelangle[centerx-1-i] =(short) intang;
      pixelangle[centerx+i] =(short) -intang;
      frac+=(length*65536/centerx);
      }
   table=W_CacheLumpName("tables",PU_CACHE, CvtNull, 1);
   SafeFree(pangle);
}


/*
==========================
=
= SetViewSize
=
==========================
*/

void SetViewSize
   (
   int size
   )

   {
   int height;
   int maxheight;
   int screenx;
   int screeny;
   int topy;

	if ((size<0) || (size>=MAXVIEWSIZES)){//bna added
        printf("Illegal screen size = %d\n",size);
		size = 8;//set default value
		viewsize = 8;
	}

   //if ((size<0) || (size>=MAXVIEWSIZES))
   //   Error("Illegal screen size = %ld\n",size);

   viewwidth  = viewsizes[ size << 1 ];         // must be divisable by 16
   viewheight = viewsizes[ ( size << 1 ) + 1 ]; // must be even

   maxheight = iGLOBAL_SCREENHEIGHT;
   topy      = 0;

   // Only keep the kills flag
   StatusBar &= ~( BOTTOM_STATUS_BAR | TOP_STATUS_BAR |
      STATUS_PLAYER_STATS );

   if ( SHOW_KILLS() )
      {
      // Account for height of kill boxes
      maxheight -= 24;
      }

   if ( size < 9 )
      {
      StatusBar |= TOP_STATUS_BAR;

      // Account for height of top status bar
      maxheight -= 16;
      topy      += 16;
      }

//   if ( size == 7 ){maxheight -= 16;}//bna++
//   if ( size <= 6 ){topy -= 8;}//bna++

   if ( size < 8 )
      {
      // Turn on health and ammo bar
      StatusBar |= BOTTOM_STATUS_BAR;

      maxheight -= 16;

      }
   else if ( size < 10 )
      {
      // Turn on transparent health and ammo bar
      StatusBar |= STATUS_PLAYER_STATS;
      }
   //   SetTextMode (  );
   //   viewheight=viewheight;
   height = viewheight;
   if ( height > 168 )
   {
        // Prevent weapon from being scaled too big
	    height = 168;
	  	if ( iGLOBAL_SCREENWIDTH == 640) {
			height = 260;
		}else if ( iGLOBAL_SCREENWIDTH == 800) {
			height = 360;
		}
   }

   if ((G_weaponscale > 150)&&(G_weaponscale <600)){height = G_weaponscale;}
   if ( iGLOBAL_SCREENWIDTH == 320) {G_weaponscale=G_weaponscale/2;}
   weaponscale = ( height << 16 ) / 168;//( height << 16 ) = 170 * 65536

  
   centerx     = viewwidth >> 1;
   centery     = viewheight >> 1;
   centeryfrac = (centery << 16);
   yzangleconverter = ( 0xaf85 * viewheight ) / iGLOBAL_SCREENHEIGHT;

   // Center the view horizontally
   screenx = ( iGLOBAL_SCREENWIDTH - viewwidth ) >> 1;

   if ( viewheight >= maxheight )
      {
      screeny = topy;
      viewheight = maxheight;
      }
   else
      {
      // Center the view vertically
      screeny = ( ( maxheight - viewheight ) >> 1 ) + topy;
      }

   // Calculate offset of view window
#ifdef DOS
   screenofs = ( screenx >> 2 ) + ylookup[ screeny ];
#else
   screenofs = screenx + ylookup[ screeny ];
#endif

//
// calculate trace angles and projection constants
//

   ResetFocalWidth();


// Already being called in ResetFocalWidth
//   SetViewDelta();


     CalcProjection();

   }


//******************************************************************************
//
// DrawCPUJape ()
//
//******************************************************************************

void DrawCPUJape
   (
   void
   )

   {
   int width;
   int height;

   CurrentFont = tinyfont;
   VW_MeasurePropString( YourComputerSucksString, &width, &height );

   DrawGameString( 160 - width / 2, 100 + 48 / 2 + 2,
      YourComputerSucksString, true );
   }


/*
==========================
=
= SetupScreen
=
==========================
*/

void SetupScreen ( boolean flip )
{
   pic_t *shape;

   SetViewSize(viewsize);

   if ( viewsize < 7 )
      {
      shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );
      //DrawTiledRegion( 0, 16, 320, 200 - 32, 0, 16, shape );
      DrawTiledRegion( 0, 16, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT - 32, 0, 16, shape );//bna++
      }

   if ( viewsize == 0 )
      {
      DrawCPUJape();
      }

   DrawPlayScreen (true);
   if (flip==true)
      {
      ThreeDRefresh();
      VL_CopyDisplayToHidden();
      }
}



void LoadColorMap( void )
{
   int i,j;
   int lump, length;

   if (ColorMapLoaded==1)
      Error("Called LoadColorMap twice\n");
   ColorMapLoaded=1;
//
//   load in the light tables
//   256 byte align tables
//

	lump = W_GetNumForName("colormap");
	length = W_LumpLength (lump) + 256;
	colormap = SafeMalloc (length);
	byte *aligned_colormap = (byte *)( ((long)colormap + 255)&~0xff);
	W_ReadLump (lump, aligned_colormap);
    colormap = aligned_colormap;

// Fix fire colors in colormap

   for (i=31;i>=16;i--)
		for (j=0xea;j<0xf9;j++)
         colormap[i*256+j]=colormap[(((i-16)/4+16))*256+j];

// Get special maps

	lump = W_GetNumForName("specmaps");
	length = W_LumpLength (lump+1) + 256;
	redmap = SafeMalloc (length);
	byte *aligned_redmap = (byte *)( ((long)redmap + 255)&~0xff);
	W_ReadLump (lump+1, aligned_redmap);
    redmap = aligned_redmap;
   greenmap = redmap+(16*256);

// Get player colormaps

//   if (modemgame==true)
      {
      lump = W_GetNumForName("playmaps")+1;
      for (i=0;i<MAXPLAYERCOLORS;i++)
         {
	      length = W_LumpLength (lump+i) + 256;
	      playermaps[i] = SafeMalloc (length);
	      byte *aligned_playmap = (byte *)( ((long)playermaps[i] + 255)&~0xff);
	      W_ReadLump (lump+i, aligned_playmap);
          playermaps[i] = aligned_playmap;
         }
      }

   if (!quiet)
      printf("RT_VIEW: Colormaps Initialized\n");

}

/*
==========================
=
= SetupLightLevels
=
==========================
*/
#define LIGHTRATEBASE 252
#define LIGHTRATEEND  267
#define LIGHTLEVELBASE 216
#define LIGHTLEVELEND  223
void SetupLightLevels ( void )
{
   int glevel;

   periodic=false;
	fog=0;
	lightsource=0;

// Set up light level for level

	if (((word)MAPSPOT(2,0,1)>=104) && ((word)MAPSPOT(2,0,1)<=105))
		fog=(word)MAPSPOT(2,0,1)-104;
	else
		Error ("There is no Fog icon on map %d\n",gamestate.mapon);
	if ((word)MAPSPOT(3,0,1)==139)
      {
      if (fog==0)
         {
		   lightsource=1;
         lights=Z_Malloc(MAPSIZE*MAPSIZE*(sizeof(unsigned long)),PU_LEVEL,NULL);
         memset (lights,0,MAPSIZE*MAPSIZE*(sizeof(unsigned long)));
         }
      else
		   Error("You cannot use light sourcing on a level with fog on map %d\n",gamestate.mapon);
      }
	else if ((word)MAPSPOT(3,0,1))
		Error("You must use the lightsource icon or nothing at all at (3,0) in plane 1 on map %d\n",gamestate.mapon);
   if (((word)MAPSPOT(2,0,0)>=LIGHTLEVELBASE) && ((word)MAPSPOT(2,0,0)<=LIGHTLEVELEND))
      glevel=(MAPSPOT(2,0,0)-LIGHTLEVELBASE);
	else
		Error("You must specify a valid darkness level icon at (2,0) on map %d\n",gamestate.mapon);

   SetLightLevels ( glevel );

   if (((word)MAPSPOT(3,0,0)>=LIGHTRATEBASE) && ((word)MAPSPOT(3,0,0)<=LIGHTRATEEND))
      glevel=(MAPSPOT(3,0,0)-LIGHTRATEBASE);
	else
      {
//      Error("You must specify a valid darkness rate icon at (3,0) on map %ld\n",gamestate.mapon);
      glevel = 4;
      }

   SetLightRate ( glevel );
   lightningtime=0;
   lightningdistance=0;
   lightninglevel=0;
   lightningdelta=0;
   lightningsoundtime=0;
}

/*
==========================
=
= SetLightLevels
=
==========================
*/
void SetLightLevels ( int darkness )
{
   if (fog==0)
      {
      baseminshade=0x10+((7-darkness)>>1);
      basemaxshade=0x1f-(darkness>>1);
      }
   else
      {
      baseminshade=darkness;
      basemaxshade=0x10;
      }
   minshade=baseminshade;
   maxshade=basemaxshade;
   darknesslevel=darkness;
}

/*
==========================
=
= GetLightLevelTile
=
==========================
*/
int GetLightLevelTile ( void )
{
   if (fog==0)
      {
      return ((7-((baseminshade-0x10)<<1))+LIGHTLEVELBASE);
      }
   else
      {
      return (baseminshade+LIGHTLEVELBASE);
      }
}


/*
==========================
=
= SetLightRate
=
==========================
*/
void SetLightRate ( int rate )
{
   normalshade=(HEIGHTFRACTION+8)-rate;
   if (normalshade>14) normalshade=14;
   if (normalshade<3) normalshade=3;
}

/*
==========================
=
= GetLightRate
=
==========================
*/
int GetLightRate ( void )
{
   return ((HEIGHTFRACTION+8)-normalshade);
}

/*
==========================
=
= GetLightRateTile
=
==========================
*/
int GetLightRateTile ( void )
{
   return ((HEIGHTFRACTION+8)-normalshade+LIGHTRATEBASE);
}



/*
==========================
=
= UpdateLightLevel
=
==========================
*/
void UpdateLightLevel (int area)
{
   int numlights;
   int targetmin;
   int targetmax;
   int numtiles;

   if (fog==true)
      return;

   numtiles=(numareatiles[area]>>5)-2;
   numlights=(LightsInArea[area]-numtiles)>>1;

   if (numlights<0)
      numlights=0;
   if (numlights>GENERALNUMLIGHTS)
      numlights=GENERALNUMLIGHTS;
   targetmin=baseminshade+(GENERALNUMLIGHTS-numlights);
   targetmax=basemaxshade-numlights;
   if (targetmax<baseminshade)
      targetmax=baseminshade;
   if (targetmin>targetmax)
      targetmin=targetmax;

   if (minshade>targetmin)
      minshade-=1;
   else if (minshade<targetmin)
      minshade+=1;

   if (maxshade>targetmax)
      maxshade-=1;
   else if (maxshade<targetmax)
      maxshade+=1;

#if 0
   targetlevel=baseminshade+(GENERALNUMLIGHTS-numlights);
   if (abs(minshade-targetlevel)==1)
      minshade=targetlevel;
   else if (minshade>targetlevel)
      minshade-=2;
   else if (minshade<targetlevel)
      minshade+=2;
#endif
}

/*
==========================
=
= SetIllumination
=
= Postive value lightens
= Negative value darkens
=
==========================
*/
void SetIllumination (int level)
{
   if (fog)
      return;
	maxshade-=level;
   if (maxshade>31)
      maxshade=31;
   if (maxshade<0x10)
      maxshade=0x10;
	minshade-=level;
   if (minshade<0x10)
      minshade=0x10;
   if (minshade>31)
      minshade=31;
}

/*
==========================
=
= GetIlluminationDelta
=
==========================
*/
int GetIlluminationDelta (void)
{
   if (fog)
      return 0;
   else
      return maxshade-basemaxshade;
}

/*
==========================
=
= UpdateLightning
=
==========================
*/
void UpdateLightning (void)
{
   if (periodic==true)
      {
      UpdatePeriodicLighting();
      return;
      }
   if ((fog==1) || (lightning==false))
      return;

   if (lightningtime<=0)
      {
      if (lightningsoundtime>0)
         SD_Play3D (SD_LIGHTNINGSND, 0, lightningdistance);
      lightningtime=GameRandomNumber("UpdateLightning",0)<<1;
      lightningdistance=GameRandomNumber("UpdateLightning",0);
      lightninglevel=(255-lightningdistance)>>LIGHTNINGLEVEL;
      if (lightninglevel<MINLIGHTNINGLEVEL)
         lightninglevel=MINLIGHTNINGLEVEL;
      if (lightninglevel>MAXLIGHTNINGLEVEL)
         lightninglevel=MAXLIGHTNINGLEVEL;
      lightningdelta=lightninglevel>>1;
      lightningsoundtime=lightningdistance>>1;
      if (lightningdistance<100)
         {
         SetIllumination(lightninglevel);
         }
      }
   else if (lightninglevel>0)
      {
      lightninglevel-=lightningdelta;
      if (lightninglevel<=0)
         {
         lightninglevel=0;
         }
      }
   else
      lightningtime--;
   if (lightningsoundtime)
      {
      lightningsoundtime--;
      if (lightningsoundtime<=0)
         {
         int volume;

         volume=255-lightningdistance;
         if (volume>170) volume=170;
         SD_PlayPitchedSound ( SD_LIGHTNINGSND, volume,-(lightningdistance<<2));
         lightningsoundtime=0;
         }
      }
}

/*
==========================
=
= UpdatePeriodicLighting
=
==========================
*/
#define PERIODICMAG (6)
#define PERIODICSTEP (20)
#define PERIODICBASE (0x0f)
void UpdatePeriodicLighting (void)
{
   int val;

   val=FixedMul(PERIODICMAG,sintable[periodictime]);
   periodictime=(periodictime+PERIODICSTEP)&(FINEANGLES-1);
   basemaxshade=PERIODICBASE+(PERIODICMAG)+val;
   baseminshade=basemaxshade-GENERALNUMLIGHTS-1;
}

/*
==========================
=
= SetModemLightLevel
=
==========================
*/
void SetModemLightLevel ( int type )
{
   periodic=false;
   fulllight=false;
   switch (type)
      {
      case bo_light_dark:
         MAPSPOT(2,0,0)=(word)216;
         MAPSPOT(3,0,0)=(word)255;
         MAPSPOT(3,0,1)=(word)139;
         MAPSPOT(2,0,1)=(word)104;
         SetupLightLevels ();
         break;
      case bo_light_normal:
         break;
      case bo_light_bright:
         MAPSPOT(2,0,0)=(word)223;
         MAPSPOT(3,0,0)=(word)267;
         MAPSPOT(3,0,1)=(word)0;
         MAPSPOT(2,0,1)=(word)104;
         SetupLightLevels ();
         break;
      case bo_light_fog:
         MAPSPOT(2,0,0)=(word)219;
         MAPSPOT(3,0,0)=(word)259;
         MAPSPOT(2,0,1)=(word)105;
         MAPSPOT(3,0,1)=(word)0;
         SetupLightLevels ();
         break;
      case bo_light_periodic:
         fog=0;
         MAPSPOT(2,0,1)=(word)104;
         MAPSPOT(3,0,1)=(word)139;
         SetupLightLevels ();
         periodic=true;
         break;
      case bo_light_lightning:
         if (sky!=0)
            {
            MAPSPOT(2,0,0)=(word)222;
            MAPSPOT(3,0,0)=(word)255;
            MAPSPOT(3,0,1)=(word)139;
            MAPSPOT(2,0,1)=(word)104;
            SetupLightLevels ();
            lightning=true;
            }
         break;
      }
}

