//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// graphics.cpp: implementation of the graphics class.
//
//////////////////////////////////////////////////////////////////////

//Flavor - Convert from DirectDraw to SDL

//Flavor #define WIN32_LEAN_AND_MEAN

#include "StdAfx.h"
#include "main.h"
#include "graphics.h"
#include "memory.h"

//
// 16 bit graphics buffers
// At the moment there's no system which uses more than 16 bit
// colors.
// A grpaphics buffer holds number referencing to the color from
// the "total palette" for that system.
//
unsigned short totalpalette[32*32*32];

unsigned short *drawBuffer;

// standard VRAM table adresses
unsigned char *sprite_table   = get_address(0x00008800);
unsigned char *pattern_table   = get_address(0x0000A000);
unsigned short*patterns = (unsigned short*)pattern_table;
unsigned short *tile_table_front  = (unsigned short *)get_address(0x00009000);
unsigned short *tile_table_back  = (unsigned short *)get_address(0x00009800);
unsigned short *palette_table   = (unsigned short *)get_address(0x00008200);
unsigned char *bw_palette_table  = get_address(0x00008100);
unsigned char *sprite_palette_numbers = get_address(0x00008C00);
// VDP registers
//
// wher is the vdp rendering now on the lcd display?
//unsigned char *scanlineX  = get_address(0x00008008);
unsigned char *scanlineY  = get_address(0x00008009);
// frame 0/1 priority registers
unsigned char *frame0Pri  = get_address(0x00008000);
unsigned char *frame1Pri  = get_address(0x00008030);
// windowing registers
unsigned char *wndTopLeftX = get_address(0x00008002);
unsigned char *wndTopLeftY = get_address(0x00008003);
unsigned char *wndSizeX  = get_address(0x00008004);
unsigned char *wndSizeY  = get_address(0x00008005);
// scrolling registers
unsigned char *scrollSpriteX = get_address(0x00008020);
unsigned char *scrollSpriteY = get_address(0x00008021);
unsigned char *scrollFrontX = get_address(0x00008032);
unsigned char *scrollFrontY = get_address(0x00008033);
unsigned char *scrollBackX = get_address(0x00008034);
unsigned char *scrollBackY = get_address(0x00008035);
// background color selection register and table
unsigned char *bgSelect  = get_address(0x00008118);
unsigned short *bgTable  = (unsigned short *)get_address(0x000083E0);
unsigned char *oowSelect  = get_address(0x00008012);
unsigned short *oowTable  = (unsigned short *)get_address(0x000083F0);
// machine constants
unsigned char *color_switch = get_address(0x00006F91);


// Defines
#define ZeroStruct(x) ZeroMemory(&x, sizeof(x)); x.dwSize = sizeof(x);
#define SafeRelease(x) if(x) { x->Release(); x = NULL; }

//Flavor - For speed testing, you can turn off screen updates
//#define NO_SCREEN_OUTPUT  //seems to give about 4-6FPS on GP2X

// flip buffers and indicate that one of the buffers is ready
// for blitting to the screen.
void graphicsBlitEnd()
{
    //displayDirty=1;
}

extern void graphics_paint();


//////////////////////////////////////////////////////////////////////////////
//
// Palette Initialization
//
//////////////////////////////////////////////////////////////////////////////

void palette_init16(DWORD dwRBitMask, DWORD dwGBitMask, DWORD dwBBitMask)
{
    //dbg_print("in palette_init16(0x%X, 0x%X, 0x%X)\n", dwRBitMask, dwGBitMask, dwBBitMask);

    char RShiftCount = 0, GShiftCount = 0, BShiftCount = 0;
    char RBitCount = 0, GBitCount = 0, BBitCount = 0;
    int  r,g,b;
    DWORD i;

    i = dwRBitMask;
    while (!(i&1))
    {
        i = i >> 1;
        RShiftCount++;
    }
    while (i&1)
    {
        i = i >> 1;
        RBitCount++;
    }
    i = dwGBitMask;
    while (!(i&1))
    {
        i = i >> 1;
        GShiftCount++;
    }
    while (i&1)
    {
        i = i >> 1;
        GBitCount++;
    }
    i = dwBBitMask;
    while (!(i&1))
    {
        i = i >> 1;
        BShiftCount++;
    }
    while (i&1)
    {
        i = i >> 1;
        BBitCount++;
    }

    for (b=0; b<16; b++)
        for (g=0; g<16; g++)
            for (r=0; r<16; r++)
                totalpalette[b*256+g*16+r] =
                    (((b<<(BBitCount-4))+(b>>(4-(BBitCount-4))))<<BShiftCount) +
                    (((g<<(GBitCount-4))+(g>>(4-(GBitCount-4))))<<GShiftCount) +
                    (((r<<(RBitCount-4))+(r>>(4-(RBitCount-4))))<<RShiftCount);
}

//////////////////////////////////////////////////////////////////////////////
//
// Neogeo Pocket & Neogeo Pocket color rendering
//
//////////////////////////////////////////////////////////////////////////////

//static const bwTable[8] = { 0x0000, 0x0333, 0x0555, 0x0777, 0x0999, 0x0BBB, 0x0DDD, 0x0FFF };
static const unsigned short bwTable[8] =
    {
        0x0FFF, 0x0DDD, 0x0BBB, 0x0999, 0x0777, 0x0555, 0x0333, 0x0000
    };

#define OFFSETX 0
#define OFFSETY 0

//
// THOR'S GRAPHIC CORE
//

typedef struct
{
	unsigned char flip;
	unsigned char x;
	unsigned char pal;
} MYSPRITE;

typedef struct
{
	unsigned short tile;
	unsigned char id;
} MYSPRITEREF;

typedef struct
{
	unsigned char count[152];
	MYSPRITEREF refs[152][64];
} MYSPRITEDEF;

MYSPRITEDEF *mySprPri40,*mySprPri80,*mySprPriC0;
MYSPRITE *mySprites = NULL;
unsigned short myPalettes[192];

void sortSprites(unsigned int bw)
{
    unsigned int spriteCode;
    unsigned char x, y, prevx=0, prevy=0;
    unsigned int i, j;
    unsigned short tile;
    MYSPRITEREF *spr;

    if (mySprites == NULL)
    {
        mySprites = (MYSPRITE*)calloc(64, sizeof(MYSPRITE));
        mySprPri40 = (MYSPRITEDEF*)calloc(1, sizeof(MYSPRITEDEF));
        mySprPri80 = (MYSPRITEDEF*)calloc(1, sizeof(MYSPRITEDEF));
        mySprPriC0 = (MYSPRITEDEF*)calloc(1, sizeof(MYSPRITEDEF));
    }
    // initialize the number of sprites in each structure
    memset(mySprPri40->count, 0, 152);
    memset(mySprPri80->count, 0, 152);
    memset(mySprPriC0->count, 0, 152);

    for (i=0;i<64;i++)
    {
        spriteCode = *((unsigned short *)(sprite_table+4*i));

        prevx = (spriteCode & 0x0400 ? prevx : 0) + *(sprite_table+4*i+2);
        x = prevx + *scrollSpriteX;

        prevy = (spriteCode & 0x0200 ? prevy : 0) + *(sprite_table+4*i+3);
        y = prevy + *scrollSpriteY;

        if ((x>167 && x<249) || (y>151 && y<249) || (spriteCode<=0xff) || ((spriteCode & 0x1800)==0))
            continue;

		mySprites[i].x = x;
		mySprites[i].pal = bw ? ((spriteCode>>11)&0x04) : ((sprite_palette_numbers[i]&0xf)<<2);
		mySprites[i].flip = spriteCode>>8;
		tile = (spriteCode & 0x01ff)<<3;

        for (j = 0; j < 8; ++j,++y)
        {
        	if (y>151)
        		continue;
            switch (spriteCode & 0x1800)
            {
                case 0x1800:
                spr = &mySprPriC0->refs[y][mySprPriC0->count[y]++];
                break;
                case 0x1000:
                spr = &mySprPri80->refs[y][mySprPri80->count[y]++];
                break;
                case 0x0800:
                spr = &mySprPri40->refs[y][mySprPri40->count[y]++];
                break;
                default: continue;
            }
            spr->id = i;
            spr->tile = tile + (spriteCode&0x4000 ? 7-j : j);
        }
    }
}

void drawSprites(unsigned short* draw,
				 MYSPRITEREF *sprites,int count,
				 int x0,int x1)
{
	unsigned short*pal;
	unsigned int pattern,pix,cnt;
	MYSPRITE *spr;
	int i,cx;

    if (mySprites == NULL)
    {
        mySprites = (MYSPRITE*)calloc(64, sizeof(MYSPRITE));
    }

	for (i=count-1;i>=0;--i)
	{
		pattern = patterns[sprites[i].tile];
		if (pattern==0)
			continue;

		spr = &mySprites[sprites[i].id];

		if (spr->x>248)
			cx = spr->x-256;
        else
			cx = spr->x;

		if (cx+8<=x0 || cx>=x1)
			continue;

		pal = &myPalettes[spr->pal];

        if (cx<x0)
        {
			cnt = 8-(x0-cx);
			if (spr->flip&0x80)
			{
                pattern>>=((8-cnt)<<1);
                for (cx=x0;pattern;++cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
                pattern &= (0xffff>>((8-cnt)<<1));
                for (cx = x0+cnt-1;pattern;--cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
        }
        else if (cx+7<x1)
        {
			if (spr->flip&0x80)
			{
                for (;pattern;++cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
                for (cx+=7;pattern;--cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
        }
        else
        {
			cnt = x1-cx;
			if (spr->flip&0x80)
			{
                pattern &= (0xffff>>((8-cnt)<<1));
                for (;pattern;++cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
                pattern>>=((8-cnt)<<1);
                for (cx+=cnt-1;pattern;--cx)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[cx] = pal[pix];
                    pattern>>=2;
                }
			}
        }
	}
}

void drawScrollPlane(unsigned short* draw,
					 unsigned short* tile_table,int scrpal,
					 unsigned char dx,unsigned char dy,
					 int x0,int x1,unsigned int bw)
{
	unsigned short*tiles;
	unsigned short*pal;
	unsigned int pattern;
	unsigned int j,count,pix,idy,tile;
	int i,x2;

	dx+=x0;
	tiles = tile_table+((dy>>3)<<5)+(dx>>3);

	count = 8-(dx&0x7);
	dx &= 0xf8;
	dy &= 0x7;
	idy = 7-dy;

	i = x0;

    if (count<8)
    {
		tile = *(tiles++);
		pattern = patterns[(((tile&0x1ff))<<3) + (tile&0x4000 ? idy:dy)];
		if (pattern)
		{
			pal = &myPalettes[scrpal + (bw ? (tile&0x2000 ? 4 : 0) : ((tile>>7)&0x3c))];
			if (tile&0x8000)
			{
                pattern>>=((8-count)<<1);
                for (j=i;pattern;++j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
                pattern &= (0xffff>>((8-count)<<1));
                for (j=i+count-1;pattern;--j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
		}
        i+=count;
		dx+=8;
		if (dx==0)
			tiles-=32;
    }

    x2 = i+((x1-i)&0xf8);

	for (;i<x2;i+=8)
	{
		tile = *(tiles++);
		pattern = patterns[(((tile&0x1ff))<<3) + (tile&0x4000 ? idy:dy)];
		if (pattern)
		{
			pal = &myPalettes[scrpal + (bw ? (tile&0x2000 ? 4 : 0) : ((tile>>7)&0x3c))];
			if (tile&0x8000)
			{
                for (j=i;pattern;++j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
                for (j=i+7;pattern;--j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
		}
		dx+=8;
		if (dx==0)
			tiles-=32;
	}

	if (x2!=x1)
	{
        count = x1-x2;
		tile = *(tiles++);
		pattern = patterns[(((tile&0x1ff))<<3) + (tile&0x4000 ? idy:dy)];
		if (pattern)
		{
			pal = &myPalettes[scrpal + (bw ? (tile&0x2000 ? 4 : 0) : ((tile>>7)&0x3c))];
			if (tile&0x8000)
			{
                pattern &= (0xffff>>((8-count)<<1));
                for (j=i;pattern;++j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
			else
			{
			    pattern>>=((8-count)<<1);
                for (j=i+count-1;pattern;--j)
                {
                    pix = pattern & 0x3;
                    if (pix)
                        draw[j] = pal[pix];
                    pattern>>=2;
                }
			}
		}
	}
}

void myGraphicsBlitLine(unsigned char render)
{
    render = true;
	int i,x0,x1;
    if (*scanlineY < 152)
    {
        if(render)
        {
			unsigned short* draw = &drawBuffer[*scanlineY*NGPC_SIZEX];
			unsigned short bgcol;
            unsigned int bw = (m_emuInfo.machine == NGP);
            unsigned short OOWCol = NGPC_TO_SDL16(oowTable[*oowSelect & 0x07]);
            unsigned short* pal;
            unsigned short* mempal;

			if (*scanlineY==0)
				sortSprites(bw);
			if (*scanlineY<*wndTopLeftY || *scanlineY>*wndTopLeftY+*wndSizeY || *wndSizeX==0 || *wndSizeY==0)
			{

				for (i=0;i<NGPC_SIZEX;i++)
					draw[i] = OOWCol;
			}
			else
			{

				if (((*scanlineY)&7) == 0)
				{
		            if (bw)
    		        {
        		        for(i=0;i<4;i++)
            		    {
	                	    myPalettes[i]     = NGPC_TO_SDL16(bwTable[bw_palette_table[i]    & 0x07]);
		                    myPalettes[4+i]   = NGPC_TO_SDL16(bwTable[bw_palette_table[4+i]  & 0x07]);
    		                myPalettes[64+i]  = NGPC_TO_SDL16(bwTable[bw_palette_table[8+i]  & 0x07]);
        		            myPalettes[68+i]  = NGPC_TO_SDL16(bwTable[bw_palette_table[12+i] & 0x07]);
            		        myPalettes[128+i] = NGPC_TO_SDL16(bwTable[bw_palette_table[16+i] & 0x07]);
                		    myPalettes[132+i] = NGPC_TO_SDL16(bwTable[bw_palette_table[20+i] & 0x07]);
		                }
    		        }
        		    else
					{
						pal = myPalettes;
						mempal = palette_table;

						for (i=0;i<192;i++)
							*(pal++) = NGPC_TO_SDL16(*(mempal++));
        		    }
				}


	            if(*bgSelect & 0x80)
    	            bgcol = NGPC_TO_SDL16(bgTable[*bgSelect & 0x07]);
        	    else if(bw)
	                bgcol = NGPC_TO_SDL16(bwTable[0]);
    	        else
        	        bgcol = NGPC_TO_SDL16(bgTable[0]);//maybe 0xFFF?

				x0 = *wndTopLeftX;
				x1 = x0+*wndSizeX;
				if (x1>NGPC_SIZEX)
					x1 = NGPC_SIZEX;

				for (i=x0;i<x1;i++)
					draw[i] = bgcol;

				if (mySprPri40->count[*scanlineY])
					drawSprites(draw,mySprPri40->refs[*scanlineY],mySprPri40->count[*scanlineY],x0,x1);

	            if (*frame1Pri & 0x80)
	            {
        			drawScrollPlane(draw,tile_table_front,64,*scrollFrontX,*scrollFrontY+*scanlineY,x0,x1,bw);
	            	if (mySprPri80->count[*scanlineY])
						drawSprites(draw,mySprPri80->refs[*scanlineY],mySprPri80->count[*scanlineY],x0,x1);
		        	drawScrollPlane(draw,tile_table_back,128,*scrollBackX,*scrollBackY+*scanlineY,x0,x1,bw);
	            }
	            else
	            {
		        	drawScrollPlane(draw,tile_table_back,128,*scrollBackX,*scrollBackY+*scanlineY,x0,x1,bw);
					if (mySprPri80->count[*scanlineY])
						drawSprites(draw,mySprPri80->refs[*scanlineY],mySprPri80->count[*scanlineY],x0,x1);
	    	    	drawScrollPlane(draw,tile_table_front,64,*scrollFrontX,*scrollFrontY+*scanlineY,x0,x1,bw);
	            }

				if (mySprPriC0->count[*scanlineY])
					drawSprites(draw,mySprPriC0->refs[*scanlineY],mySprPriC0->count[*scanlineY],x0,x1);

				for (i=0;i<x0;i++)
					draw[i] = OOWCol;
				for (i=x1;i<NGPC_SIZEX;i++)
					draw[i] = OOWCol;

	        }
        }
        if (*scanlineY == 151)
        {
            // start VBlank period
            tlcsMemWriteB(0x00008010,tlcsMemReadB(0x00008010) | 0x40);
            if (render)
                graphics_paint();
        }
        *scanlineY+= 1;
    }
    else if (*scanlineY == 198)
    {
        // stop VBlank period
        tlcsMemWriteB(0x00008010,tlcsMemReadB(0x00008010) & ~0x40);

	    if(render)
            graphics_paint();

        *scanlineY = 0;
    }
    else
        *scanlineY+= 1;
}


//////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
//////////////////////////////////////////////////////////////////////////////

const char ErrDirectDrawCreate[] = "DirectDrawCreate Failed";
const char ErrQueryInterface[]  = "QueryInterface Failed";
const char ErrSetCooperativeLevel[] = "SetCooperativeLevel Failed";
const char ErrCreateSurface[]  = "CreateSurface Failed";

#define DDOK(x) if (hRet != DD_OK) { LastErr = x; return FALSE; }

BOOL graphics_init(void)
{
    // palette_init16(0x001f,0x03e0,0x7c00);

    palette_init16(0xf800,0x07c0,0x003e);

    if (m_emuInfo.machine == NGPC)
    {
        bgTable  = (unsigned short *)get_address(0x000083E0);
        oowTable  = (unsigned short *)get_address(0x000083F0);
    }
    else
    {
        bgTable  = (unsigned short *)bwTable;
        oowTable = (unsigned short *)bwTable;
    }

    sprite_table   = get_address(0x00008800);
    pattern_table   = get_address(0x0000A000);
    patterns = (unsigned short*)pattern_table;
    tile_table_front  = (unsigned short *)get_address(0x00009000);
    tile_table_back  = (unsigned short *)get_address(0x00009800);
    palette_table   = (unsigned short *)get_address(0x00008200);
    bw_palette_table  = get_address(0x00008100);
    sprite_palette_numbers = get_address(0x00008C00);
    scanlineY  = get_address(0x00008009);
    frame0Pri  = get_address(0x00008000);
    frame1Pri  = get_address(0x00008030);
    wndTopLeftX = get_address(0x00008002);
    wndTopLeftY = get_address(0x00008003);
    wndSizeX  = get_address(0x00008004);
    wndSizeY  = get_address(0x00008005);
    scrollSpriteX = get_address(0x00008020);
    scrollSpriteY = get_address(0x00008021);
    scrollFrontX = get_address(0x00008032);
    scrollFrontY = get_address(0x00008033);
    scrollBackX = get_address(0x00008034);
    scrollBackY = get_address(0x00008035);
    bgSelect  = get_address(0x00008118);
    bgTable  = (unsigned short *)get_address(0x000083E0);
    oowSelect  = get_address(0x00008012);
    oowTable  = (unsigned short *)get_address(0x000083F0);
    color_switch = get_address(0x00006F91);

    *scanlineY = 0;

    return TRUE;
}

void graphics_cleanup()
{}

