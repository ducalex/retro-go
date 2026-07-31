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
// RT_BUILD.C

#include "rt_def.h"
#include <string.h>
#include "watcom.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef DOS
#include <dos.h>
#include <conio.h>
#endif

#include "rt_build.h"
#include "_rt_buil.h"
#include "rt_dr_a.h"
#include "rt_draw.h"
#include "rt_scale.h"
#include "rt_menu.h"
#include "rt_main.h"
#include "isr.h"
#include "rt_util.h"
#include "engine.h"
#include "lumpy.h"
#include "rt_fc_a.h"
#include "z_zone.h"
#include "w_wad.h"
#include "rt_view.h"
#include "rt_cfg.h"
#include "rt_vid.h"

#include "rt_sound.h"
#include "modexlib.h"
#include "rt_str.h"
//MED
#include "memcheck.h"

byte * intensitytable;


// LOCAL VARIABLES

static byte * menubuf;
static byte * menubuffers[2];
static char menutitles[2][40];
static int alternatemenubuf=0;
static int titleshade=16;
static int titleshadedir=1;
static int titleyoffset=0;
static char titlestring[40]="\0";
static int readytoflip;
static boolean MenuBufStarted=false;
static int mindist=0x2700;
static boolean BackgroundDrawn=false;

static plane_t planelist[MAXPLANES],*planeptr;

static int StringShade=16;

extern void (*USL_MeasureString)(const char *, int *, int *, font_t *);

static char strbuf[MaxString];

//******************************************************************************
//
// DrawRotPost
//
//******************************************************************************

void DrawRotPost ( int height, byte * src, byte * buf, int origheight)
{
   int y1;
   int y2;

   hp_srcstep=(origheight<<18)/height;
   y1 = (((centery<<8)-(height<<5)+(MENUOFFY<<8)));
   y2 = (((height<<6)+y1)>>8);

   if (((y1>>8)>=200) || (y2<0))
      return;
   if (y1<0)
      {
      hp_startfrac=FixedMulShift(-y1,hp_srcstep,8);
      y2 = (((height<<6)+y1)>>8);
      y1=0;
      }
   else
      {
      hp_startfrac=FixedMulShift(255-(y1&0xff),hp_srcstep,8);
      y1>>=8;
      }
   if (y2>200)
      {
      DrawMenuPost(200-y1, src, buf+ylookup[y1]);
      }
   else
      {
      DrawMenuPost(y2-y1, src, buf+ylookup[y1]);
      }
}



/*
========================
=
= GetPoint
=
========================
*/

void GetPoint (int x1, int y1, int px, int py, int * screenx, int * height, int angle)
{

  fixed gxt,gyt,nx,ny;
  fixed gxtt,gytt;
  int gx,gy;


//
// translate point to view centered coordinates
//
  gx = x1-px;
  gy = y1-py;

//
// calculate newx
//
  gxt = FixedMul(gx,costable[angle]);
  gyt = FixedMul(gy,sintable[angle]);
  nx =gxt-gyt;

  if (nx<mindist)
	  nx=mindist;


//
// calculate newy
//
  gxtt = FixedMul(gx,sintable[angle]);
  gytt = FixedMul(gy,costable[angle]);
  ny = gytt+gxtt;

// too close, don't overflow the divid'


  *screenx = 160 + ((ny*NORMALWIDTHMULTIPLIER)/nx);            // DEBUG: use assembly divide

  *height = NORMALHEIGHTDIVISOR/nx;

  if (*screenx<0) *screenx=0;

}


/*
========================
=
= InterpolatePlane
=
========================
*/

void InterpolatePlane (visobj_t * plane)
{
   int d1,d2;
   int top;
   int topinc;
   int bot;
   int botinc;
   int i;
   int texture;
   int dh;
   int dx;
   int height;


   dx=(plane->x2-plane->x1+1);
   if (plane->h1<=0 || plane->h2<=0 || (dx==0))
      return;
   d1=65536/plane->h1;
   d2=65536/plane->h2;
   dh=((plane->h2-plane->h1)<<8)/dx;
   top=0;
   topinc=(d1)*((plane->textureend-plane->texturestart)>>4);
   bot=d2*dx;
   botinc=d1-d2;
   height=(plane->h1<<8);
   if (plane->x1>=viewwidth)
      return;
   for (i=plane->x1;i<=plane->x2;i++)
      {
      if ((i>=0 && i<viewwidth)&&(posts[i].wallheight<(height>>8)))
         {
         if (bot)
            {
            texture=((top/bot)+(plane->texturestart>>4))>>6;
            posts[i].texture=texture*plane->viewx;
            posts[i].lump=plane->shapenum;
            posts[i].wallheight=(height>>8);
            posts[i].offset=plane->viewx;
            }
         }
      top+=topinc;
      bot+=botinc;
      height+=dh;
      }
}

//******************************************************************************
//
// DrawPlanePosts
//
//******************************************************************************

void   DrawPlanePosts (void)
{
   int height;
   char * buf;
   byte * shape;
   int lastwall=-2;
   int plane;
   int i;

   shadingtable=colormap+(16<<8);

#ifdef DOS
   for (plane=0;plane<4;plane++)
#endif

      {
      VGAWRITEMAP(plane);
      buf=(byte *)(bufferofs);

#ifdef DOS
      for (i=plane;i<viewwidth;i+=4,buf++)
#else
      for (i=0;i<viewwidth;i++,buf++)
#endif
         {
         height=(posts[i].wallheight);
         if (height<=4)
            continue;
         if (lastwall!=posts[i].lump)
            {
            lastwall=posts[i].lump;
            if (lastwall==-1)
               shape=menubuf;
            else
               shape=W_CacheLumpNum(lastwall,PU_CACHE, Cvt_patch_t, 1);
            }
         DrawRotPost (height,shape+posts[i].texture,buf,posts[i].offset);
         }
      }
}

//******************************************************************************
//
// NextPlaneptr
//
//******************************************************************************

void NextPlaneptr ( void )
{
   if (planeptr < &planelist[MAXPLANES-1]) // don't let it overflo'
  		planeptr++;
}

//******************************************************************************
//
// RestPlaneptr
//
//******************************************************************************

void ResetPlaneptr ( void )
{
   planeptr = &planelist[0];
}

//******************************************************************************
//
// NextVisptr
//
//******************************************************************************

void NextVisptr ( void )
{
   if (visptr < &vislist[MAXVISIBLE-1]) // don't let it overflo'
  		visptr++;
}

//******************************************************************************
//
// ResetVisptr
//
//******************************************************************************

void ResetVisptr ( void )
{
   visptr = &vislist[0];
}

//******************************************************************************
//
// SetupPlanes
//
//******************************************************************************

void SetupPlanes ( void )
{
   int i;

   if (!posts) return;

   for (i=0;i<320;i++)
      posts[i].wallheight=-1;
}



//******************************************************************************
//
// CalcPlanes
//
//******************************************************************************

void CalcPlanes ( int px, int py, int angle )
{
   plane_t * pptr;

   ResetVisptr();
   for (pptr = &planelist[0]; pptr<planeptr; pptr++)
      {
      if (SideOfLine(pptr->x1,pptr->y1,pptr->x2,pptr->y2,px,py)<0)
         {
         GetPoint (pptr->x1,pptr->y1,px,py,&visptr->x1,&visptr->h1,angle);
         GetPoint (pptr->x2,pptr->y2,px,py,&visptr->x2,&visptr->h2,angle);
         visptr->textureend=0x0;
         visptr->texturestart=pptr->texturewidth;
         }
      else
         {
         GetPoint (pptr->x2,pptr->y2,px,py,&visptr->x1,&visptr->h1,angle);
         GetPoint (pptr->x1,pptr->y1,px,py,&visptr->x2,&visptr->h2,angle);
         visptr->texturestart=0x0;
         visptr->textureend=pptr->texturewidth;
         }
      visptr->shapenum=pptr->texture;
      visptr->viewx=pptr->origheight;
      visptr->viewheight=MAX(visptr->h1,visptr->h2);
      NextVisptr();
      }
}


//******************************************************************************
//
// DrawTransformedPlanes
//
//******************************************************************************

void DrawTransformedPlanes ( void )
{
   int numvisible;
   int greatest;
   int height;
   int i;
   visobj_t * closest;

   numvisible = visptr-&vislist[0];
   if (!numvisible)
      return;                                     // no visible objects
   for (i = 0; i<numvisible; i++)
      {
      greatest = 0;
      for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
         {
         height = visstep->viewheight;
         if (height > greatest)
            {
            greatest = height;
            closest = visstep;
            }
         }
      InterpolatePlane(closest);
      closest->viewheight = 0;
      }
}


//******************************************************************************
//
// ClearMenuBuf
//
//******************************************************************************

void ClearMenuBuf ( void )
{
   byte * shape;

   if (MenuBufStarted==false)
      Error("Called ClearMenuBuf without menubuf started\n");

   shape=W_CacheLumpName(MENUBACKNAME,PU_CACHE, Cvt_patch_t, 1);
   shape+=8;
   memcpy (menubuf,shape,TEXTUREW*TEXTUREHEIGHT);
}

//******************************************************************************
//
// ShutdownMenuBuf
//
//******************************************************************************

void ShutdownMenuBuf ( void )
{
   if (MenuBufStarted==false)
      return;
   MenuBufStarted=false;
   SafeFree(menubuffers[0]);
   SafeFree(menubuffers[1]);
   if (loadedgame==false)
      SetViewSize(viewsize);
}

//******************************************************************************
//
// SetupMenuBuf
//
//******************************************************************************

void SetupMenuBuf ( void )
{
#define SRCH 148
#define SRCW 258
#define PLANEX1 (-0x24000)
#define PLANEX2 (0x23fff)
#define PLANEW (16<<10)
#define PLANEY (0x40000)
#define PLANEW2 (0x5a827)

   if (MenuBufStarted==true)
      return;
   MenuBufStarted=true;

   // No top offsets like in game

   centery=100;
   centeryfrac=centery<<16;

   strcpy(titlestring,menutitles[0]);

   screenofs=0;
   viewwidth=320;
   viewheight=200;

   alternatemenubuf=0;

   ResetPlaneptr();
   planeptr->texture=-1;
   planeptr->y1=PLANEX1;
   planeptr->x1=PLANEW;
   planeptr->y2=PLANEX2;
   planeptr->x2=PLANEW;
   planeptr->origheight=TEXTUREHEIGHT;
   planeptr->texturewidth=TEXTUREWIDTH;
   NextPlaneptr();
   planeptr->texture=-1;
   planeptr->y1=PLANEX1;
   planeptr->x1=-PLANEW;
   planeptr->y2=PLANEX2;
   planeptr->x2=-PLANEW;
   planeptr->origheight=TEXTUREHEIGHT;
   planeptr->texturewidth=TEXTUREWIDTH;
   NextPlaneptr();
   planeptr->texture=W_GetNumForName(MENUBACKNAME);
   planeptr->y1=PLANEX1;
   planeptr->x1=PLANEW;
   planeptr->y2=PLANEX1;
   planeptr->x2=-PLANEW;
   planeptr->origheight=TEXTUREHEIGHT;
   planeptr->texturewidth=TEXTUREWIDTH;
   NextPlaneptr();
   planeptr->texture=W_GetNumForName(MENUBACKNAME);
   planeptr->y1=PLANEX2;
   planeptr->x1=PLANEW;
   planeptr->y2=PLANEX2;
   planeptr->x2=-PLANEW;
   planeptr->origheight=TEXTUREHEIGHT;
   planeptr->texturewidth=TEXTUREWIDTH;
   NextPlaneptr();

   menubuffers[0]=SafeMalloc(TEXTUREW*TEXTUREHEIGHT);
   menubuffers[1]=SafeMalloc(TEXTUREW*TEXTUREHEIGHT);
   menubuf=menubuffers[0];
   ClearMenuBuf();
   BackgroundDrawn=false;
}


//******************************************************************************
//
// PositionMenuBuf
//
//******************************************************************************

void PositionMenuBuf( int angle, int distance, boolean drawbackground )
{
   int px,py;
   font_t * oldfont;
   int width,height;


   if (MenuBufStarted==false)
      Error("Called PositionMenuBuf without menubuf started\n");
   CalcTics();
   SetupPlanes();
   if ((drawbackground==true) || (BackgroundDrawn==false))
      {
      VL_DrawPostPic (W_GetNumForName("trilogo"));
      }
   px=FixedMulShift(distance,costable[angle&(FINEANGLES-1)],16);
   py=FixedMulShift(-distance,sintable[angle&(FINEANGLES-1)],16);
   CalcPlanes(px,py,(angle+ANG180)&(FINEANGLES-1));
   DrawTransformedPlanes();
   DrawPlanePosts();
   oldfont=CurrentFont;
   CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
   US_MeasureStr (&width, &height, "%s", titlestring);
   US_ClippedPrint ((320-width)>>1, MENUTITLEY-titleyoffset, titlestring);
   CurrentFont=oldfont;
   FlipPage();
   titleshade+=titleshadedir;
   if (abs(titleshade-16)>6)
      titleshadedir=-titleshadedir;
   if (BackgroundDrawn==false)
      {
      VL_CopyDisplayToHidden();
      BackgroundDrawn=true;
      }
}

//******************************************************************************
//
// RefreshMenuBuf
//
//******************************************************************************

void RefreshMenuBuf( int time )
{
   int i;

   if (MenuBufStarted==false)
      Error("Called RefreshMenuBuf without menubuf started\n");

   if (readytoflip)
      return;

   for (i=0;i<=time;i+=tics)
      {
      //PositionMenuBuf (0,NORMALVIEW,false);
      PositionMenuBuf (0,NORMALVIEW,true);//bna++ in not true bg in menu is no redrawn
      }
}

//******************************************************************************
//
// ScaleMenuBufPost
//
//******************************************************************************

void ScaleMenuBufPost (byte * src, int topoffset, byte * buf)
{
   int  d;
   int  offset;
   int  length;
   int  s;


   while (1)
      {
      offset=*(src++);
      if (offset==0xff)
         return;
      else
         {
         d=offset-topoffset;
         length=*(src++);
         for (s=0;s<length;s++,d++)
            *(buf+d)=*(src+s);
         src+=length;
         }
      }
}

//******************************************************************************
//
// SetAlternateMenuBuf
//
//******************************************************************************

void SetAlternateMenuBuf ( void )
{
  if (MenuBufStarted==false)
     Error("Called SetAlternateMenuBuf without menubuf started\n");

  alternatemenubuf^=1;
  readytoflip=1;
  menubuf=menubuffers[alternatemenubuf];
}

//******************************************************************************
//
// SetMenuTitle
//
//******************************************************************************

void SetMenuTitle ( const char * menutitle )
{
  if (MenuBufStarted==false)
     Error("Called SetMenuTitle without menubuf started\n");
  strcpy(menutitles[alternatemenubuf],menutitle);
  if (readytoflip==0)
     strcpy(titlestring,menutitle);
}

//******************************************************************************
//
// DrawMenuBufPicture
//
//******************************************************************************

void DrawMenuBufPicture (int x, int y, const byte * pic, int w, int h)
{
   byte *buffer;
   int i;

   if (MenuBufStarted==false)
      Error("Called DrawMenuBufPictoure without menubuf started\n");

   if ((x<0) || (x+w>=TEXTUREW))
      Error ("DrawMenuBufPicture: x is out of range\n");
   if ((y<0) || (y+h>=TEXTUREHEIGHT))
      Error ("DrawMenuBufPicture: y is out of range\n");

   for (i=0;i<w;i++)
      {
      buffer = (byte*)menubuf+y+((x+i)*TEXTUREHEIGHT);
      memcpy(buffer,pic,h);
      pic+=h;
      }
}

//******************************************************************************
//
// DrawMenuBufItem
//
//******************************************************************************

void DrawMenuBufItem (int x, int y, int shapenum)
{
   byte *buffer;
   int cnt;
   byte *shape;
   patch_t *p;

   if (MenuBufStarted==false)
      Error("Called DrawMenuBufItem without menubuf started\n");

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_patch_t, 1);
   p = (patch_t *)shape;

   if (((x-p->leftoffset)<0) || ((x-p->leftoffset+p->width)>=TEXTUREW))
      Error ("DrawMenuBufItem: x is out of range\n");
   if (((y-p->topoffset)<0) || ((y-p->topoffset+p->height)>=TEXTUREHEIGHT))
      Error ("DrawMenuBufItem: y is out of range\n");

   buffer = (byte*)menubuf+y+((x-p->leftoffset)*TEXTUREHEIGHT);

   for (cnt = 0; cnt < p->width; cnt++,buffer+=TEXTUREHEIGHT)
      ScaleMenuBufPost ((byte *)(p->collumnofs[cnt]+shape),
                        p->topoffset, buffer);
}

//******************************************************************************
//
// IScaleMenuBufPost
//
//******************************************************************************

void IScaleMenuBufPost (byte * src, int topoffset, byte * buf, int color)
{
   int  d;
   int  offset;
   int  length;
   int  s;


   while (1)
      {
      offset=*(src++);
      if (offset==0xff)
         return;
      else
         {
         d=offset-topoffset;
         length=*(src++);
         for (s=0;s<length;s++,d++)
            *(buf+d)=*(intensitytable+((*(src+s))<<8)+color);
         src+=length;
         }
      }
}


//******************************************************************************
//
// DrawIMenuBufItem
//
//******************************************************************************

void DrawIMenuBufItem (int x, int y, int shapenum, int color)
{
   byte *buffer;
   int cnt;
   byte *shape;
   patch_t *p;


   if (MenuBufStarted==false)
      Error("Called DrawIMenuBufItem without menubuf started\n");

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_patch_t, 1);
   p = (patch_t *)shape;

   if (((x-p->leftoffset)<0) || ((x-p->leftoffset+p->width)>=TEXTUREW))
      Error ("DrawIMenuBufItem: x is out of range\n");
   if (((y-p->topoffset)<0) || ((y-p->topoffset+p->height)>=TEXTUREHEIGHT))
      Error ("DrawIMenuBufItem: y is out of range\n");

   buffer = (byte*)menubuf+y+((x-p->leftoffset)*TEXTUREHEIGHT);

   for (cnt = 0; cnt < p->width; cnt++,buffer+=TEXTUREHEIGHT)
      IScaleMenuBufPost ((byte *)(p->collumnofs[cnt]+shape),
                        p->topoffset, buffer, color);
}


//******************************************************************************
//
// TScaleMenuBufPost
//
//******************************************************************************
void TScaleMenuBufPost (byte * src, int topoffset, byte * buf)
{
   int  d;
   int  offset;
   int  length;
   byte pixel;
   int  s;


   while (1)
      {
      offset=*(src++);
      if (offset==0xff)
         return;
      else
         {
         d=offset-topoffset;
         length=*(src++);
         for (s=0;s<length;s++,d++)
            {
            pixel = *(buf+d);
            pixel = *(shadingtable+pixel);
            *(buf+d) = pixel;
            }
         src+=length;
         }
      }
}


//******************************************************************************
//
// CScaleMenuBufPost
//
//******************************************************************************
void CScaleMenuBufPost (byte * src, int topoffset, byte * buf)
{
   int  d;
   int  offset;
   int  length;
   byte pixel;
   int  s;


   while (1)
      {
      offset=*(src++);
      if (offset==0xff)
         return;
      else
         {
         d=offset-topoffset;
         length=*(src++);
         for (s=0;s<length;s++,d++)
            {
            pixel = *(src+s);
            pixel = *(shadingtable+pixel);
            *(buf+d) = pixel;
            }
         src+=length;
         }
      }
}


//******************************************************************************
//
// EraseMenuBufRegion
//
//******************************************************************************

void EraseMenuBufRegion (int x, int y, int width, int height)
{
   byte *buffer;
   int xx,yy;
   byte * shape;

   if (MenuBufStarted==false)
      Error("Called EraseMenuBufRegion without menubuf started\n");

   if ((x<0) || (x+width>=TEXTUREW))
      Error ("EraseMenuBufRegion: x is out of range\n");
   if ((y<0) || (y+height>=TEXTUREHEIGHT))
      Error ("EraseMenuBufRegion: y is out of range\n");

   shape=W_CacheLumpName(MENUBACKNAME,PU_CACHE, Cvt_patch_t, 1);
   shape+=8;
   shape+=(x*TEXTUREHEIGHT)+y;
   buffer = (byte*)menubuf+(x*TEXTUREHEIGHT)+y;

   for (xx = 0; xx < width; xx++)
      {
      for (yy = 0; yy < height; yy++)
         *(buffer+yy)=*(shape+yy);
      buffer+=TEXTUREHEIGHT;
      shape+=TEXTUREHEIGHT;
      }
}


//******************************************************************************
//
// DrawTMenuBufPic
//
//******************************************************************************

void DrawTMenuBufPic (int x, int y, int shapenum)
{
   byte *buffer;
   byte *buf;
   int xx,yy;
   int plane;
   int pixel;
   byte *shape;
   byte *src;
   pic_t *p;

   if (MenuBufStarted==false)
      Error("Called DrawTMenuBufPic without menubuf started\n");

   shadingtable=colormap+(25<<8);

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_pic_t, 1);
   p = (pic_t *)shape;

   if ((x<0) || ((x+(p->width<<2))>=TEXTUREW))
      Error ("DrawTMenuBufPic: x is out of range\n");
   if ((y<0) || ((y+p->height)>=TEXTUREHEIGHT))
      Error ("DrawTMenuBufPic: y is out of range\n");

   buffer = (byte*)menubuf+(x*TEXTUREHEIGHT)+y;

   src=(byte *)&p->data;
   for (plane=0;plane<4;plane++,buffer+=TEXTUREHEIGHT)
      {
      for (yy = 0; yy < p->height; yy++)
         {
         buf=buffer+yy;
         for (xx = 0; xx < p->width; xx++,buf+=TEXTUREHEIGHT<<2)
            {
            pixel = *(buf);
            pixel = *(shadingtable+pixel);
            *(buf) = pixel;
            }
         }
      }
}


//******************************************************************************
//
// DrawTMenuBufItem
//
//******************************************************************************

void DrawTMenuBufItem (int x, int y, int shapenum)
{
   byte *buffer;
   int cnt;
   byte *shape;
   patch_t *p;


   if (MenuBufStarted==false)
      Error("Called DrawTMenuBufItem without menubuf started\n");

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_patch_t, 1);
   p = (patch_t *)shape;

   shadingtable=colormap+(25<<8);

   if (((x-p->leftoffset)<0) || ((x-p->leftoffset+p->width)>=TEXTUREW))
      Error ("DrawTMenuBufItem: x is out of range\n");
   if (((y-p->topoffset)<0) || ((y-p->topoffset+p->height)>=TEXTUREHEIGHT))
      Error ("DrawTMenuBufItem: y is out of range\n");

   buffer = (byte*)menubuf+y+((x-p->leftoffset)*TEXTUREHEIGHT);

   for (cnt = 0; cnt < p->width; cnt++,buffer+=TEXTUREHEIGHT)
      TScaleMenuBufPost ((byte *)(p->collumnofs[cnt]+shape),
                        p->topoffset, buffer);
}

//******************************************************************************
//
// DrawColoredMenuBufItem
//
//******************************************************************************

void DrawColoredMenuBufItem (int x, int y, int shapenum, int color)
{
   byte *buffer;
   int cnt;
   byte *shape;
   patch_t *p;


   if (MenuBufStarted==false)
      Error("Called DrawColoredMenuBufItem without menubuf started\n");

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_patch_t, 1);
   p = (patch_t *)shape;

   shadingtable=playermaps[color]+(16<<8);

   if (((x-p->leftoffset)<0) || ((x-p->leftoffset+p->width)>=TEXTUREW))
      Error ("DrawColoredMenuBufItem: x is out of range\n");
   if (((y-p->topoffset)<0) || ((y-p->topoffset+p->height)>=TEXTUREHEIGHT))
      Error ("DrawColoredMenuBufItem: y is out of range\n");

   buffer = (byte*)menubuf+y+((x-p->leftoffset)*TEXTUREHEIGHT);

   for (cnt = 0; cnt < p->width; cnt++,buffer+=TEXTUREHEIGHT)
      CScaleMenuBufPost ((byte *)(p->collumnofs[cnt]+shape),
                        p->topoffset, buffer);
}

//******************************************************************************
//
// DrawMenuBufPic
//
//******************************************************************************

void DrawMenuBufPic (int x, int y, int shapenum)
{
   byte *buffer;
   byte *buf;
   int xx,yy;
   int plane;
   byte *shape;
   byte *src;
   pic_t *p;

   if (MenuBufStarted==false)
      Error("Called DrawMenuBufPic without menubuf started\n");

   shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_pic_t, 1);
   p = (pic_t *)shape;

   if ((x<0) || ((x+(p->width<<2))>=TEXTUREW))
      Error ("DrawTMenuBufPic: x is out of range\n");
   if ((y<0) || ((y+p->height)>=TEXTUREHEIGHT))
      Error ("DrawTMenuBufPic: y is out of range\n");


   buffer = (byte*)menubuf+(x*TEXTUREHEIGHT)+y;

   src=(byte *)&p->data;
   for (plane=0;plane<4;plane++,buffer+=TEXTUREHEIGHT)
      {
      for (yy = 0; yy < p->height; yy++)
         {
         buf=buffer+yy;
         for (xx = 0; xx < p->width; xx++,buf+=TEXTUREHEIGHT<<2)
            *(buf)=*(src++);
         }
      }
}




//******************************************************************************
//
// DrawTMenuBufBox
//
//******************************************************************************


void DrawTMenuBufBox ( int x, int y, int width, int height )
   {
   byte *buffer;
   int   xx;
   int   yy;
   int   pixel;

   if (MenuBufStarted==false)
      Error("Called DrawTMenuBufBox without menubuf started\n");

   shadingtable = colormap + ( 25 << 8 );

   if ( ( x < 0 ) || ( ( x + width ) >= TEXTUREW ) )
      Error ("DrawTMenuBar : x is out of range\n");
   if ( ( y < 0 ) || ( y + height ) >= TEXTUREHEIGHT )
      Error ("DrawTMenuBar : y is out of range\n");

   buffer = ( byte * )menubuf + ( x * TEXTUREHEIGHT ) + y;

   for ( xx = 0; xx < width; xx++ )
      {
      for ( yy = 0; yy < height; yy++ )
         {
         pixel = *( buffer + yy );
         pixel = *( shadingtable + pixel );
         *( buffer + yy ) = pixel;
         }

      buffer += TEXTUREHEIGHT;
      }
   }


//******************************************************************************
//
// DrawTMenuBufHLine
//
//******************************************************************************

void DrawTMenuBufHLine (int x, int y, int width, boolean up)
{
   byte *buffer;
   byte *buf;
   int xx;
   int plane;
   int pixel;
   int w = width>>2;
   int ww = w;

   if ((x<0) || ((x+(width))>=TEXTUREW))
      Error ("DrawTMenuBufBox: x is out of range\n");
   if (y<0)
      Error ("DrawTMenuBufBox: y is out of range\n");

   buffer = (byte*)menubuf+(x*TEXTUREHEIGHT)+y;

   if (up)
      shadingtable=colormap+(13<<8);
   else
      shadingtable=colormap+(25<<8);

   for (plane = 0; plane < 4; plane++, buffer += TEXTUREHEIGHT)
   {
      w = ww;
      switch (plane)
      {
         case 0:
            if (width % 4)
               w++;
         break;
         case 1:
            if ((width % 4) > 1)
               w++;
         break;
         case 2:
            if ((width % 4) > 2)
               w++;
         break;
      }


      buf = buffer;
      for (xx = 0; xx < w; xx++, buf += (TEXTUREHEIGHT<<2))
      {
         pixel = *(buf);
         pixel = *(shadingtable+pixel);
         *(buf) = pixel;
      }
   }
}

//******************************************************************************
//
// DrawTMenuBufVLine
//
//******************************************************************************

void DrawTMenuBufVLine (int x, int y, int height, boolean up)
{
   byte *buffer;
   byte *buf;
   int yy;
   int pixel;

   if (x<0)
      Error ("DrawTMenuBufBox: x is out of range\n");
   if ((y<0) || ((y+height)>=TEXTUREHEIGHT))
      Error ("DrawTMenuBufBox: y is out of range\n");

   buffer = (byte*)menubuf+(x*TEXTUREHEIGHT)+y;

   if (up)
      shadingtable=colormap+(13<<8);
   else
      shadingtable=colormap+(25<<8);

   for (yy = 0; yy < height; yy++)
   {
      buf = buffer+yy;
      pixel = *(buf);
      pixel = *(shadingtable+pixel);
      *(buf) = pixel;
   }
}

//******************************************************************************
//******************************************************************************
//
// STRING ROUTINES
//
//******************************************************************************
//******************************************************************************


//******************************************************************************
//
// DrawMenuBufPropString ()
//
//******************************************************************************

void DrawMenuBufPropString (int px, int py, const char *string)
{
   byte  pix;
   int   width,height,ht;
   byte  *source, *dest, *origdest;
   int   ch;


   if (MenuBufStarted==false)
      Error("Called DrawMenuBufPropString without menubuf started\n");

   ht = CurrentFont->height;
   dest = origdest = (byte*)menubuf+(px*TEXTUREHEIGHT)+py;

   while ((ch = (unsigned char)*string++)!=0)
   {
      ch -= 31;
      width = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
      {
         height = ht;
         while (height--)
         {
            pix = *source;
            if (pix)
               *dest = pix;

            source++;
            dest ++;
         }

         PrintX++;
         origdest+=TEXTUREHEIGHT;
         dest = origdest;
      }
   }

}


//******************************************************************************
//
// DrawMenuBufIString ()
//
//******************************************************************************

void DrawMenuBufIString (int px, int py, const char *string, int color)
{
   byte  pix;
   int   width,height,ht;
   byte  *source, *dest, *origdest;
   int   ch;

   if (MenuBufStarted==false)
      Error("Called DrawMenuBufPropString without menubuf started\n");

   if ( ( color < 0 ) || ( color > 255 ) )
      {
      Error( "Intensity Color out of range\n" );
      }

   ht = IFont->height;
   dest = origdest = (byte*)menubuf+(px*TEXTUREHEIGHT)+py;

   PrintX = px;
   PrintY = py;

   while ((ch = (unsigned char)*string++)!=0)
   {
      // Tab
      if ( ch == '\x9' )
         {
         int offset;

         PrintX   -= px;
         offset    = 4 * 5 - PrintX % ( 4 * 5 );
         PrintX   += offset + px;
         origdest += offset * TEXTUREHEIGHT;
         dest      = origdest;
         continue;
         }

      ch -= 31;
      width = IFont->width[ ch ];

      source = ( ( byte * )IFont ) + IFont->charofs[ ch ];

      while (width--)
      {
         height = ht;
         while (height--)
         {
            pix = *source;
            if ( pix != 0xFE )
               {
               *dest = ( ( byte )intensitytable[ ( pix << 8 ) + color ] );
               GetIntensityColor( pix );
               }

            source++;
            dest ++;
         }

         PrintX++;
         origdest+=TEXTUREHEIGHT;
         dest = origdest;
      }
   }

}


//******************************************************************************
//
// DrawTMenuBufPropString ()
//
// Draws a string at px, py to bufferofs
//
//******************************************************************************

void DrawTMenuBufPropString (int px, int py, const char *string)
{
   byte  pix;
   int   width,height,ht;
   byte  *source, *dest, *origdest;
   int   ch;


   if (MenuBufStarted==false)
      Error("Called DrawTMenuBufPropString without menubuf started\n");

   ht = CurrentFont->height;
   dest = origdest = (byte*)menubuf+(px*TEXTUREHEIGHT)+py;

   shadingtable=colormap+(StringShade<<8);
   while ((ch = (unsigned char)*string++)!=0)
   {
      ch -= 31;
      width = CurrentFont->width[ch];
      source = ((byte *)CurrentFont)+CurrentFont->charofs[ch];
      while (width--)
      {
         height = ht;
         while (height--)
         {
            pix = *source;
            if (pix)
               {
               pix = *dest;
               pix = *(shadingtable+pix);
               *dest = pix;
               }
            source++;
            dest ++;
         }

         PrintX++;
         origdest+=TEXTUREHEIGHT;
         dest = origdest;
      }
   }
}


//******************************************************************************
//
// MenuBufCPrintLine() - Prints a string centered on the current line and
//    advances to the next line. Newlines are not supported.
//
//******************************************************************************

void MenuBufCPrintLine (const char *s)
{
   int w, h;

   USL_MeasureString (s, &w, &h, CurrentFont);

   if (w > WindowW)
      Error("MenuBufCPrintLine() - String exceeds width");

   PrintX = WindowX + ((WindowW - w) / 2);
   DrawMenuBufPropString (PrintX, PrintY, s);
   PrintY += h;
}

//******************************************************************************
//
// MenuBufCPrint() - Prints a string in the current window. Newlines are
//    supported.
//
//******************************************************************************

void MenuBufCPrint (const char *string)
{
   char  c,
         *se,
         *s;

    /* !!! FIXME: this is lame. */
    if (strlen(string) >= sizeof (strbuf))
    {
        fprintf(stderr, "buffer overflow!\n");
        return;
    }

    /* prevent writing to literal strings... ( MenubufCPrint("feh"); ) */
    strcpy(strbuf, string);
    s = strbuf;

   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      MenuBufCPrintLine(s);

      s = se;
      if (c)
      {
         *se = c;
         s++;
      }
   }
}

//******************************************************************************
//
// MenuBufPrintLine() - Prints a string on the current line and
//    advances to the next line. Newlines are not supported.
//
//******************************************************************************

void MenuBufPrintLine (const char *s)
{
   int w, h;

   USL_MeasureString (s, &w, &h, CurrentFont);

   if (w > WindowW)
      Error("MenuBufCPrintLine() - String exceeds width");

   PrintX = WindowX;
   DrawMenuBufPropString (PrintX, PrintY, s);
   PrintY += h;
}

//******************************************************************************
//
// MenuBufPrint() - Prints a string in the current window. Newlines are
//    supported.
//
//******************************************************************************

void MenuBufPrint (const char *string)
{
   char  c,
         *se,
         *s;

   strcpy(strbuf, string);
   s = strbuf;
   
   while (*s)
   {
      se = s;
      while ((c = *se) && (c != '\n'))
         se++;
      *se = '\0';

      MenuBufPrintLine(s);

      s = se;
      if (c)
      {
         *se = c;
         s++;
      }
   }
}

//******************************************************************************
//
// MenuTBufPrintLine() - Prints a string on the current line and
//    advances to the next line. Newlines are not supported.
//
//******************************************************************************

void MenuTBufPrintLine (const char *s, int shade)
{
   int w, h;

   USL_MeasureString (s, &w, &h, CurrentFont);

   if (w > WindowW)
      Error("MenuBufCPrintLine() - String exceeds width");

   PrintX = WindowX;
   StringShade=shade;
   DrawTMenuBufPropString (PrintX, PrintY, s);
   PrintY += h;
}

//******************************************************************************
//
// FlipMenuBuf
//
//******************************************************************************

void FlipMenuBuf ( void )
{
   int i;
   int dh;
   int h;
   int y;
   int dy;
   int time;
   int flip;

   if (MenuBufStarted==false)
      Error("Called FlipMenuBuf without menubuf started\n");

   if (!readytoflip)
      Error("FlipMenuBuf called without SetAlternateMenuBuf beforehand");
   readytoflip=0;
   if (Menuflipspeed<=5)
      {
      menubuf=menubuffers[alternatemenubuf];
      strcpy(titlestring,menutitles[alternatemenubuf]);
      RefreshMenuBuf(0);
      }
   else
      {
      menubuf=menubuffers[alternatemenubuf^1];
      strcpy(titlestring,menutitles[alternatemenubuf^1]);
      time=Menuflipspeed-5;
      dh=(1024<<8)/time;
      h=0;
      dy=((MENUTITLEY*6)<<8)/time;
      y=0;
      flip=0;
      titleyoffset=0;
      for (i=0;i<time;i+=tics)
         {
         PositionMenuBuf(h>>8,NORMALVIEW,true);
         h+=dh*tics;
         y+=dy*tics;
         titleyoffset=y>>8;
         if ((h>=512<<8) && (flip==0))
            {
            MN_PlayMenuSnd (SD_MENUFLIP);
            h=1536<<8;
            dy=-dy;
            menubuf=menubuffers[alternatemenubuf];
            strcpy(titlestring,menutitles[alternatemenubuf]);
            flip=1;
            }
         }
      }
   titleyoffset=0;
   BackgroundDrawn=false;
}



//******************************************************************************
//
// RotatePlane
//
//******************************************************************************

void RotatePlane ( void )
{
   SetupMenuBuf();

   while (!(Keyboard[0x01]))
      {
      RefreshMenuBuf(100);
      SetAlternateMenuBuf();
      ClearMenuBuf();
      DrawMenuBufPic  (0,0,W_GetNumForName("newg11"));
      DrawMenuBufItem (0,0,W_GetNumForName("apogee"));
      FlipMenuBuf();
      EraseMenuBufRegion(30,30,30,30);
      RefreshMenuBuf(100);
      SetAlternateMenuBuf();
      ClearMenuBuf();
      FlipMenuBuf();
      }
   ShutdownMenuBuf();
}
