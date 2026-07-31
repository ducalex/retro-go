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
// RT_FLOOR.C

#include "rt_def.h"
#ifdef DOS
#include <conio.h>
#endif

#include "watcom.h"
#include "rt_floor.h"
#include "rt_fc_a.h"
#include "_rt_floo.h"
#include "rt_draw.h"
#include "rt_util.h"
#include "engine.h"
#include "rt_main.h"
#include "w_wad.h"
#include "z_zone.h"
#include "rt_view.h"
#include "rt_ted.h"
#include "rt_cfg.h"
#include "rt_actor.h"
#include <string.h>
#include "isr.h"
#include "modexlib.h"
#include "rt_playr.h"
#include "rt_sound.h"
#include "rt_rand.h"
//MED
#include "memcheck.h"

/*
=============================================================================

Global Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/

int      sky;
int		mr_rowofs;
int		mr_count;
int		mr_xstep;
int		mr_ystep;
int		mr_xfrac;
int		mr_yfrac;
byte *   mr_dest;
byte *   mr_src;

/*
==================
=
= Local Variables
=
==================
*/

static byte     *floor;
static byte     *ceiling;
//static int xstarts[MAXVIEWHEIGHT];
#ifndef RETRO_GO
static int xstarts[600];//set to max hight res
#else
extern int *xstarts;
#endif
static byte * skysegs[MAXSKYSEGS];
static byte * skydata[MAXSKYDATA];
static int      horizonheight;
static int      centerskypost;
static int      oldsky=-1;


//bna fixit skyerror by 800x600 clouds not big enough

void DrawSky( void )
{

   byte * src;
   int dest;
//   int plane;
   int height;
//   int height2;
   int ang;
   int angle;
   int ofs;

   angle=viewangle;

   if ((fog==0) && (lightning==true))
      shadingtable=colormap+((basemaxshade-6-lightninglevel)<<8);
   else
      shadingtable=colormap+(1<<12);

   ofs=(((maxheight)-(player->z))>>3)+(centery-(viewheight>>1));
   if (ofs>centerskypost)
      {
      ofs=centerskypost;
      }
   else if (((centerskypost-ofs)+viewheight)>1799)
      {
      ofs=-(1799-(centerskypost+viewheight));
      }
//ofs=centerskypost;
#ifdef DOS
   if (doublestep>0)
      {
#ifdef DOS
      for (plane=0;plane<4;plane+=2)
#endif
         {
#ifdef DOS
         VGAMAPMASK((1<<plane)+(1<<(plane+1)));
         for (dest=plane;dest<viewwidth;dest+=4)
#else
         for (dest=0;dest<viewwidth;dest+=2)
#endif
            {
            height=posts[dest].ceilingclip;
            height2=posts[dest+1].ceilingclip;
            if (height<height2)
               height=height2;
            if (height<=0)
               continue;
            ang=(angle+pixelangle[dest])&(FINEANGLES-1);
            src=skysegs[ang]-ofs;
#ifdef DOS
            DrawSkyPost((byte *)bufferofs + (dest>>2),src,height);
#else
	/* TODO: this isn't right since it's not really optimized */
            DrawSkyPost((byte *)bufferofs + dest,src,height);
            DrawSkyPost((byte *)bufferofs + dest + 1,src,height);
#endif
            }
         }
      }
   else
#endif
      {
#ifdef DOS
      for (plane=0;plane<4;plane++)
#endif
         {
#ifdef DOS
         VGAWRITEMAP(plane);
         for (dest=plane;dest<viewwidth;dest+=4)
#else
         for (dest=0;dest<viewwidth;dest++)
#endif
            {
            if ((height=posts[dest].ceilingclip)<=0)
               continue;
            ang=(angle+pixelangle[dest])&(FINEANGLES-1);
            src=skysegs[ang]-ofs;
#ifdef DOS
            DrawSkyPost((byte *)bufferofs + (dest>>2),src,height);
#else
            DrawSkyPost((byte *)bufferofs + dest,src,height);
#endif
            }
         }
      }
}

/*
===================
=
= DrawFullSky
=
===================
*/
void DrawFullSky( void )
{

   byte * src;
   int dest;
   int plane;
   int ang;
   int angle;
   int ofs;

   angle=viewangle;

   if ((fog==0) && (lightning==true))
      shadingtable=colormap+((basemaxshade-5-lightninglevel)<<8);
   else
      shadingtable=colormap+(1<<12);

   ofs=(((maxheight)-(player->z))>>3)+(centery-(viewheight>>1));
   if (ofs>centerskypost)
      {
      ofs=centerskypost;
      }
   else if (((centerskypost-ofs)+viewheight)>599)
      {
      ofs=-(599-(centerskypost+viewheight));
      }

   bufferofs+=screenofs;

#ifdef DOS
   for (plane=0;plane<4;plane++)
#endif
      {
#ifdef DOS
      VGAWRITEMAP(plane);
      for (dest=plane;dest<viewwidth;dest+=4)
#else
      for (dest=0;dest<viewwidth;dest++)
#endif
         {
         ang=(angle+pixelangle[dest])&(FINEANGLES-1);
         src=skysegs[ang]-ofs;
#ifdef DOS
         DrawSkyPost((byte *)bufferofs + (dest>>2),src,viewheight);
#else
         DrawSkyPost((byte *)bufferofs + dest,src,viewheight);
#endif
         }
      }

   bufferofs-=screenofs;
}

/*
===================
=
= MakeSkyTile
=
===================
*/
void MakeSkyTile (byte * tile)
{
   int i,j;
   int srcstep;
   int src;

   srcstep=200<<10;
   for (i=0;i<64;i++)
      {
      src=0;
      for (j=0;j<64;j++)
         {
         *(tile + (i<<6) + j)=*(skysegs[(i<<2)]+(src>>16));
         src+=srcstep;
         }
      }
}

/*
===================
=
= MakeSkyData
=
===================
*/
void MakeSkyData ( void )
{
   byte * temp;
   byte * ptr;
   int c;

   temp=SafeMalloc(256*400);

   ptr=temp;

   for (c=0;c<256;c++)
      {

      memcpy(ptr,skydata[1]+(c*200),200);
      ptr+=200;
 
      memcpy(ptr,skydata[0]+(c*200),200);
      ptr+=200;

      //memcpy(ptr,skydata[1]+(c*200),200);
      //ptr+=200;
      //memcpy(ptr,skydata[0]+(c*200),200);
      //ptr+=200;
      }
   skydata[0]=temp;
}

/*
===================
=
= GetFloorCeilingLump
=
===================
*/

int GetFloorCeilingLump ( int num )
{
   int lump;

   switch (num)
      {
      case 1:
         lump=W_GetNumForName("FLRCL1\0");
         break;
      case 2:
         lump=W_GetNumForName("FLRCL2\0");
         break;
      case 3:
         lump=W_GetNumForName("FLRCL3\0");
         break;
      case 4:
         lump=W_GetNumForName("FLRCL4\0");
         break;
      case 5:
         lump=W_GetNumForName("FLRCL5\0");
         break;
      case 6:
         lump=W_GetNumForName("FLRCL6\0");
         break;
      case 7:
         lump=W_GetNumForName("FLRCL7\0");
         break;
      case 8:
         lump=W_GetNumForName("FLRCL8\0");
         break;
      case 9:
         lump=W_GetNumForName("FLRCL9\0");
         break;
      case 10:
         lump=W_GetNumForName("FLRCL10\0");
         break;
      case 11:
         lump=W_GetNumForName("FLRCL11\0");
         break;
      case 12:
         lump=W_GetNumForName("FLRCL12\0");
         break;
      case 13:
         lump=W_GetNumForName("FLRCL13\0");
         break;
      case 14:
         lump=W_GetNumForName("FLRCL14\0");
         break;
      case 15:
         lump=W_GetNumForName("FLRCL15\0");
         break;
      case 16:
         lump=W_GetNumForName("FLRCL16\0");
         break;
      default:
         Error("Illegal Floor/Ceiling Tile = %d\n",num);
         break;
      }
   return lump;
}

/*
===================
=
= SkyExists
=
===================
*/

boolean SkyExists (void)
{
   if (MAPSPOT(1,0,0) >= 234)
      {
      return true;
      }
   else
      {
      return false;
      }

}

/*
===================
=
= SetPlaneViewSize
=
===================
*/

void SetPlaneViewSize (void)
{
   int      x;
   int      i;
   int      s;
   int      floornum;
   int      ceilingnum;
   int      skytop;
   int      skybottom;

   sky=0;

   if (oldsky>0)
      {
      SafeFree(skydata[0]);
      oldsky=-1;
      }

   lightning=false;

	if (MAPSPOT(1,0,0) >= 234)
    {
    word crud;
	 sky = (MAPSPOT(1,0,0) - 233);
    if ((sky<1) || (sky>6))
       Error("Illegal Sky Tile = %d\n",sky);
    ceilingnum=1;
	 crud=(word)MAPSPOT(1,0,1);
	 if ((crud>=90) && (crud<=97))
	    horizonheight=crud-89;
    else if ((crud>=450) && (crud<=457))
		 horizonheight=crud-450+9;
	 else
       Error("You must specify a valid horizon height sprite icon over the sky at (2,0) on map %d\n",gamestate.mapon);

    // Check for lightnign icon

	 crud=(word)MAPSPOT(4,0,1);
	 if (crud==377)
       lightning=true;
    }
	else
	 ceilingnum = MAPSPOT(1,0,0)-197;

	floornum = MAPSPOT(0,0,0)-(179);

   floornum = GetFloorCeilingLump ( floornum );
   //ceilingnum = GetFloorCeilingLump ( ceilingnum );

   floor = W_CacheLumpNum(floornum,PU_LEVELSTRUCT, Cvt_patch_t, 1);
   floor +=8;

   if (sky==0)  // Don't cache in if not used
      {
      ceilingnum = GetFloorCeilingLump ( ceilingnum );
      ceiling = W_CacheLumpNum(ceilingnum,PU_LEVELSTRUCT, Cvt_patch_t, 1);
      ceiling +=8;
      } else {
      	ceiling = NULL;
      }

	s = W_GetNumForName("SKYSTART");

   switch (sky)
      {
      case 1:
         skytop=s+1;
         skybottom=s+2;
         break;
      case 2:
         skytop=s+3;
         skybottom=s+4;
         break;
      case 3:
         skytop=s+5;
         skybottom=s+6;
         break;
      case 4:
         skytop=s+7;
         skybottom=s+8;
         break;
      case 5:
         skytop=s+9;
         skybottom=s+10;
         break;
      case 6:
         skytop=s+11;
         skybottom=s+12;
         break;
      }
      if (sky!=0)
         {
         skydata[0]=W_CacheLumpNum(skytop,PU_STATIC, CvtNull, 1);
         skydata[1]=W_CacheLumpNum(skybottom,PU_STATIC, CvtNull, 1);
         centerskypost=MINSKYHEIGHT-(horizonheight*6);
         oldsky=sky;
         MakeSkyData();
         W_CacheLumpNum(skytop,PU_CACHE, CvtNull, 1);
         W_CacheLumpNum(skybottom,PU_CACHE, CvtNull, 1);
         x=511;
         for (i=0;i<MAXSKYSEGS;i++)
            {
            skysegs[i]=skydata[0]+((x>>1)*400)+centerskypost;
            x--;
            if (x==-1)
               {
               x=511;
               }
            } /* endfor */
         }
}

/*
==========================
=
= SetFCLightLevel
=
==========================
*/
void SetFCLightLevel (int height)
{
   int i;

	if (MISCVARS->GASON==1)
      {
		shadingtable=greenmap+(MISCVARS->gasindex<<8);
      return;
      }
   if (fulllight)
      {
      shadingtable=colormap+(1<<12);
      return;
      }
   if (fog)
      {
      i=(height>>normalshade)+minshade;
      if (i>maxshade) i=maxshade;
      shadingtable=colormap+(i<<8);
      }
   else
      {
      i=maxshade-(height>>normalshade);
      if (i<minshade) i=minshade;
      shadingtable=colormap+(i<<8);
      }
}

void DrawHLine (int xleft, int xright, int yp)
{
   int plane;
   byte * buf;
   byte * dest;
   int startxfrac;
   int startyfrac;
   int height;
//   int length;
   int ofs;

   if (yp==centery)
      return;
   if (yp>centery)
      {
      int hd;

      buf=floor;
      hd=yp-centery;
      height=(hd<<13)/(maxheight-pheight+32);
      }
   else
      {
      int hd;

      /* ROTT bug? It'd draw when there was no ceiling. - SBF */
      if (ceiling == NULL) return;
      
      buf=ceiling;
      
      hd=centery-yp;
      height=(hd<<13)/pheight;
      }
   SetFCLightLevel(height>>(8-HEIGHTFRACTION-1));
	mr_xstep = ((viewsin<<8)/(height));
	mr_ystep = ((viewcos<<8)/(height));

	startxfrac = ((viewx>>1) + FixedMulShift(mr_ystep,scale,2))-
                FixedMulShift(mr_xstep,(centerx-xleft),2);

	startyfrac = ((viewy>>1) - FixedMulShift(mr_xstep,scale,2))-
                FixedMulShift(mr_ystep,(centerx-xleft),2);

   dest=(byte *)bufferofs+ylookup[yp];

/* TODO: horizontal isn't as easy as vertical in packed */
#ifdef DOS
   if (doublestep>0)
      {
      if (xleft&1)
         xleft--;

#ifdef DOS
      for (plane=xleft;plane<xleft+4;plane+=2)
#endif
         {

#ifdef DOS
         mr_dest=dest+(plane>>2);
#else
         mr_dest=dest+xleft;
#endif

         mr_xfrac = startxfrac;
		   mr_yfrac = startyfrac;

#ifdef DOS
         startxfrac+=mr_xstep>>1;
         startyfrac+=mr_ystep>>1;

         mr_count=((xright-plane)>>2)+1;
#else
         mr_count = xright - xleft;
#endif

         if (mr_count)
            {
#ifdef DOS
            int p;
            ofs=((plane&3)<<3)+(plane&3)+1;
//          VGAMAPMASK(*((byte *)mapmasks1+ofs));
            p=plane&3;
            VGAMAPMASK((1<<p) + (1<<(p+1)));
#endif
            DrawRow(mr_count,mr_dest,buf);

#if 0
            ofs=(byte)*((byte *)mapmasks2+ofs);
            if (ofs!=0)
               {
               VGAMAPMASK(ofs);
               DrawRow(mr_count,mr_dest+1,buf);
               }
#endif
            }
         }
      }
   else
#endif
      {
#ifdef DOS
      for (plane=xleft;plane<xleft+4;plane++)
#endif
         {
#ifdef DOS
         mr_dest=dest+(plane>>2);
         VGAWRITEMAP(plane&3);
#else
         mr_dest=dest+xleft;
#endif

         mr_xfrac = startxfrac;
         mr_yfrac = startyfrac;

#ifdef DOS
         startxfrac+=mr_xstep>>2;
         startyfrac+=mr_ystep>>2;

         mr_count=((xright-plane)>>2)+1;
#else
        // back off the pixel increment (orig. is 4x)
        mr_xstep >>= 2;
        mr_ystep >>= 2;
        
         mr_count = xright-xleft+1;
#endif

         if (mr_count)
            DrawRow(mr_count,mr_dest,buf);
         }
      }

}

void DrawPlanes( void )
{
   int x,y;
   int twall;
   int bwall;

   if (sky)
      DrawSky();
   else
      {
      y=0;
      for (x=0;x<viewwidth;x++)
         {
         twall=posts[x].ceilingclip;
         while (y<twall)
            {
            xstarts[y]=x;
            y++;
            }
         while (y>twall)
            {
            y--;
            DrawHLine(xstarts[y],x-1,y);
            }
         }
      while (y>0)
         {
         y--;
         DrawHLine(xstarts[y],viewwidth-1,y);
         }
      }
   y=viewheight-1;
   for (x=0;x<viewwidth;x++)
      {
      bwall=posts[x].floorclip;
      while (y>bwall)
         {
         xstarts[y]=x;
         y--;
         }
      while (y<bwall)
         {
         y++;
         DrawHLine(xstarts[y],x-1,y);
         }
      }
   while (y<viewheight-1)
      {
      y++;
      DrawHLine(xstarts[y],viewwidth-1,y);
      }
}

#ifndef DOS
void DrawRow(int count, byte * dest, byte * src)
{
	unsigned frac, fracstep;
	int coord;

	frac = (mr_yfrac<<16) + (mr_xfrac&0xffff);
	fracstep = (mr_ystep<<16) + (mr_xstep&0xffff);

	while (count--) {
		/* extract the x/y coordinates */
		coord = ((frac >> (32-7)) | ((frac >> (32-23)) << 7)) & 16383;
		
		*dest++ = shadingtable[src[coord]];
		frac += fracstep;
	}
}
#endif
