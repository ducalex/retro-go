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
#include "watcom.h"
#include "engine.h"
#include "_engine.h"
#include "rt_draw.h"
#include "rt_door.h"
#include "rt_stat.h"
#include "rt_ted.h"
#include "rt_view.h"
#include <stdlib.h>
//MED
#include "memcheck.h"


/*
=============================================================================

Global Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/
/*
//wallcast_t posts[642];//bna++
wallcast_t posts[800+2];//bna++
//wallcast_t posts[321];
*/
//wallcast_t *posts;
int lasttilex;
int lasttiley;
/*
=============================================================================

Local Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/

static int xtilestep,ytilestep;
static int c_vx,c_vy;

void InitialCast ( void );
void Cast ( int curx );

void Interpolate (int x1, int x2)
{
   int i;
   int dtexture;
   int frac;
   int dheight;
   int hfrac;
   int dx;

   dx=x2-x1;
   dtexture=(((posts[x2].texture-posts[x1].texture)<<12)+0x800)/dx;
   dheight=(((posts[x2].wallheight-posts[x1].wallheight)<<8)+0x80)/dx;
   frac=dtexture+(posts[x1].texture<<12);
   hfrac=dheight+(posts[x1].wallheight<<8);
   for (i=x1+1;i<=x2-1;i++,frac+=dtexture,hfrac+=dheight)
      {
      posts[i].lump=posts[x1].lump;
      posts[i].posttype=posts[x1].posttype;
      posts[i].offset=posts[x1].offset;
      posts[i].alttile=posts[x1].alttile;
      posts[i].texture=(frac>>12);
      posts[i].wallheight=hfrac>>8;
      }
}

void Refresh ( void )
{
   int x;

// Cast Initial comb filter

   InitialCast();

   for (x=0;x<=viewwidth-4;x+=4)
      {
      if NOTSAMETILE(x,x+4)
         {
         Cast(x+2);
         if NOTSAMETILE(x,x+2)
            {
            Cast(x+1);
            }
         else
            Interpolate (x,x+2);
         if NOTSAMETILE(x+2,x+4)
            {
            Cast(x+3);
            }
         else
            Interpolate (x+2,x+4);
         }
      else
         Interpolate(x,x+4);
      }
}


void HitWall(int curx, int vertical, int xtile, int ytile)
{
   (void)curx;
   int num;

   posts[curx].offset=(xtile<<7)+ytile;
   posts[curx].lump = tilemap[xtile][ytile];
   posts[curx].alttile=0;
   posts[curx].posttype=0;

   if (vertical<0)
      {
      xintercept=xtile<<16;
      if (xtilestep<0)
         xintercept+=0xffff;
      yintercept=FixedScale(xintercept-viewx,c_vy,c_vx)+viewy;
      if (posts[curx].lump & 0x4000)
   	   {
         if (tilemap[xtile-(xtilestep>>7)][ytile]&0x8000)
            {
            num=tilemap[xtile-(xtilestep>>7)][ytile];
            if (num&0x4000)
               {
               if (maskobjlist[num&0x3ff]->sidepic)
   	    	      posts[curx].lump = maskobjlist[num&0x3ff]->sidepic;
               else
   	            posts[curx].lump &= 0x3ff;
               }
            else
   		 	   posts[curx].lump = doorobjlist[num&0x3ff]->sidepic;
            }
         else
            {
            if (posts[curx].lump&0x1000)
               posts[curx].lump=animwalls[posts[curx].lump&0x3ff].texture;
            else
   		      posts[curx].lump &= 0x3ff;
            }
       	}
      else if (posts[curx].lump & 0x2000)
         {
         if (IsWindow(xtile,ytile))
            posts[curx].alttile=-1;
         else
            posts[curx].alttile=(MAPSPOT(xtile,ytile,2))+1;
   		posts[curx].lump &= 0x3ff;
         }
      else if (posts[curx].lump & 0x1000)
         posts[curx].lump=animwalls[posts[curx].lump&0x3ff].texture;
      else if (posts[curx].lump & 0x800)
         {
         posts[curx].lump &= 0x3ff;
         posts[curx].posttype=2;
         }
      posts[curx].texture=yintercept-(ytile<<16);
//      posts[curx].texture&=0xffff;
      if (posts[curx].texture<0)
         posts[curx].texture=0;
      if (posts[curx].texture>65535)
         posts[curx].texture=65535;
      if (xtilestep<0)
         posts[curx].texture^=0xffff;
      posts[curx].posttype+=1;
//      posts[curx].texture=(posts[curx].texture+firstcoloffset)&65535;
      }
   else
      {
      yintercept=ytile<<16;
      if (ytilestep<0)
         yintercept+=0xffff;
      xintercept=FixedScale(yintercept-viewy,c_vx,c_vy)+viewx;
      if (posts[curx].lump & 0x4000)
	      {    // check for adjacent doors
         if (tilemap[xtile][ytile-ytilestep]&0x8000)
            {
            num=tilemap[xtile][ytile-ytilestep];
            if (num&0x4000)
               {
               if (maskobjlist[num&0x3ff]->sidepic)
		    	      posts[curx].lump = maskobjlist[num&0x3ff]->sidepic;
               else
			         posts[curx].lump &= 0x3ff;
               }
            else
		    	   posts[curx].lump = doorobjlist[num&0x3ff]->sidepic;
            }
         else
            {
            if (posts[curx].lump&0x1000)
               posts[curx].lump=animwalls[posts[curx].lump&0x3ff].texture;
            else
   		      posts[curx].lump &= 0x3ff;
            }
    		}
      else if (posts[curx].lump & 0x2000)
         {
         if (IsWindow(xtile,ytile))
            posts[curx].alttile=-1;
         else
            posts[curx].alttile=(MAPSPOT(xtile,ytile,2))+1;
   		posts[curx].lump &= 0x3ff;
         }
      else if (posts[curx].lump & 0x1000)
         posts[curx].lump=animwalls[posts[curx].lump&0x3ff].texture;
      else if (posts[curx].lump & 0x800)
         {
         posts[curx].lump &= 0x3ff;
         posts[curx].posttype=2;
         }
      posts[curx].texture=xintercept-(xtile<<16);
//      posts[curx].texture&=0xffff;
      if (posts[curx].texture<0)
         posts[curx].texture=0;
      if (posts[curx].texture>65535)
         posts[curx].texture=65535;
      if (ytilestep>0)
         posts[curx].texture^=0xffff;
//      posts[curx].posttype+=0;
//      posts[curx].texture=(posts[curx].texture+firstcoloffset)&65535;
      }
   posts[curx].wallheight=CalcHeight();
}

void InitialCast ( void )
{
   int snx,sny;
   int incr[2];
   int thedir[2];
   int cnt;
   int grid[2];
   int index;
   int curx;

   c_vx=c_startx;
   c_vy=c_starty;
   for (curx=0;curx<=viewwidth;curx+=4)
      {
      snx=viewx&0xffff;
      sny=viewy&0xffff;

      if (c_vx>0)
         {
         thedir[0]=1;
         xtilestep=0x80;
         snx^=0xffff;
         incr[1]=-c_vx;
         }
      else
         {
         thedir[0]=-1;
         xtilestep=-0x80;
         incr[1]=c_vx;
         }
      if (c_vy>0)
         {
         thedir[1]=1;
         ytilestep=1;
         sny^=0xffff;
         incr[0]=c_vy;
         }
      else
         {
         thedir[1]=-1;
         ytilestep=-1;
         incr[0]=-c_vy;
         }
      cnt=FixedMul(snx,incr[0])+FixedMul(sny,incr[1]);
      grid[0]=viewx>>16;
      grid[1]=viewy>>16;
      do
         {
         int tile;

         index=(cnt>=0);
         cnt+=incr[index];
         spotvis[grid[0]][grid[1]]=1;
         grid[index]+=thedir[index];

         if ((tile=tilemap[grid[0]][grid[1]])!=0)
            {
            if (tile&0x8000)
               {
               if ( (!(tile&0x4000)) && (doorobjlist[tile&0x3ff]->action==dr_closed))
                  {
                  spotvis[grid[0]][grid[1]]=1;
                  if (doorobjlist[tile&0x3ff]->flags&DF_MULTI)
                     MakeWideDoorVisible(tile&0x3ff);
                  do
                     {
                     index=(cnt>=0);
                     cnt+=incr[index];
                     grid[index]+=thedir[index];
                     if ((tilemap[grid[0]][grid[1]]!=0) &&
                        (!(tilemap[grid[0]][grid[1]]&0x8000)) )
                        break;
                     }
                  while (1);
                  break;
                  }
               else
                  continue;
               }
            else
               {
               mapseen[grid[0]][grid[1]]=1;
               break;
               }
            }
         }
      while (1);
      HitWall(curx, cnt-incr[index], grid[0], grid[1]);
      c_vx+=viewsin<<2;
      c_vy+=viewcos<<2;
      }
}


void Cast ( int curx )
{
   int snx,sny;
   int incr[2];
   int thedir[2];
   int cnt;
   int grid[2];
   int index;

   c_vx=c_startx+(curx*viewsin);
   c_vy=c_starty+(curx*viewcos);
   snx=viewx&0xffff;
   sny=viewy&0xffff;

   if (c_vx>0)
      {
      thedir[0]=1;
      xtilestep=0x80;
      snx^=0xffff;
      incr[1]=-c_vx;
      }
   else
      {
      thedir[0]=-1;
      xtilestep=-0x80;
      incr[1]=c_vx;
      }
   if (c_vy>0)
      {
      thedir[1]=1;
      ytilestep=1;
      sny^=0xffff;
      incr[0]=c_vy;
      }
   else
      {
      thedir[1]=-1;
      ytilestep=-1;
      incr[0]=-c_vy;
      }
   cnt=FixedMul(snx,incr[0])+FixedMul(sny,incr[1]);
   grid[0]=viewx>>16;
   grid[1]=viewy>>16;
   do
      {
      int tile;

      index=(cnt>=0);
      cnt+=incr[index];
      spotvis[grid[0]][grid[1]]=1;
      grid[index]+=thedir[index];

      if ((tile=tilemap[grid[0]][grid[1]])!=0)
         {
         if (tile&0x8000)
            {
            if ( (!(tile&0x4000)) && (doorobjlist[tile&0x3ff]->action==dr_closed))
               {
               spotvis[grid[0]][grid[1]]=1;
               if (doorobjlist[tile&0x3ff]->flags&DF_MULTI)
                  MakeWideDoorVisible(tile&0x3ff);
               do
                  {
                  index=(cnt>=0);
                  cnt+=incr[index];
                  grid[index]+=thedir[index];
                  if ((tilemap[grid[0]][grid[1]]!=0) &&
                     (!(tilemap[grid[0]][grid[1]]&0x8000)) )
                     break;
                  }
               while (1);
               break;
               }
            else
               continue;
            }
         else
            {
            mapseen[grid[0]][grid[1]]=1;
            break;
            }
         }
      }
   while (1);
   HitWall(curx, cnt-incr[index], grid[0], grid[1]);
}

