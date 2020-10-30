/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU Library General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *	Authors:
 *		Zeograd (original author)
 *		Alexander von Gluck IV, kallisti5@unixzen.com
 */

// sprice.c - VDC Sprite emulation
//
#include "sprite.h"
#include "pce.h"

// These are arrays to cache the result of the linearisation of PCE sprites and tiles
object_cache_t OBJ_CACHE;

// Actual memory area where the gfx functions are drawing sprites and tiles
uchar *SPM_raw;//[XBUF_WIDTH * XBUF_HEIGHT];
uchar *SPM;// = SPM_raw + XBUF_WIDTH * 64 + 32;

static bool sprite_usespbg = 0;

int ScrollYDiff;


/*****************************************************************************

        Function:   spr2pixel

        Description:convert a PCE coded sprite into a linear one
        Parameters:int no,the number of the sprite to convert
        Return:nothing but update OBJ_CACHE.data

*****************************************************************************/
static inline void
spr2pixel(int no)
{
	uint32 M, L, i;
    uchar *C = VRAM + no * 128;
	uint32 *C2 = (uint32*)(OBJ_CACHE.data + no * 128);
    // 2 longs -> 16 nibbles => 32 loops for a 16*16 spr

    TRACE_SPR("Planing sprite %d\n", no);
    for (i = 0; i < 32; i++, C++, C2++) {
        M = C[0];
        L = ((M & 0x88) >> 3) | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
        M = C[32];
        L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16) | ((M & 0x11) << 25);
        M = C[64];
        L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
        M = C[96];
        L |= ((M & 0x88)) | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);
        C2[0] = L;
    }
}


/*****************************************************************************

        Function:   tile2pixel

        Description: convert a PCE coded tile into a linear one
        Parameters:int no, the number of the tile to convert
        Return:nothing, but updates OBJ_CACHE.data

*****************************************************************************/
static inline void
tile2pixel(int no)
{
	uint32 M, L, i;
    uchar *C = VRAM + no * 32;
	uint32 *C2 = (uint32*)(OBJ_CACHE.data + no * 32);

    TRACE_SPR("Planing tile %d\n", no);
    for (i = 0; i < 8; i++, C += 2, C2++) {
        M = C[0];
        L = ((M & 0x88) >> 3) | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
        M = C[1];
        L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16) | ((M & 0x11) << 25);
        M = C[16];
        L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
        M = C[17];
        L |= ((M & 0x88)) | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);
        C2[0] = L;
    }
    OBJ_CACHE.tile_valid[no] = 1;
}


static inline void
PutSpriteHflipMakeMask(uchar * P, uchar * C, uint32 * C2, uchar * PAL,
    int16 h, int16 inc, uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

        if (!J)
            continue;

        L = C2[1];
        if (J & 0x8000) {
            P[15] = PAL[(L >> 4) & 15];
            M[15] = pr;
        }
        if (J & 0x4000) {
            P[14] = PAL[(L >> 12) & 15];
            M[14] = pr;
        }
        if (J & 0x2000) {
            P[13] = PAL[(L >> 20) & 15];
            M[13] = pr;
        }
        if (J & 0x1000) {
            P[12] = PAL[(L >> 28)];
            M[12] = pr;
        }
        if (J & 0x0800) {
            P[11] = PAL[(L) & 15];
            M[11] = pr;
        }
        if (J & 0x0400) {
            P[10] = PAL[(L >> 8) & 15];
            M[10] = pr;
        }
        if (J & 0x0200) {
            P[9] = PAL[(L >> 16) & 15];
            M[9] = pr;
        }
        if (J & 0x0100) {
            P[8] = PAL[(L >> 24) & 15];
            M[8] = pr;
        }

        L = C2[0];
        if (J & 0x80) {
            P[7] = PAL[(L >> 4) & 15];
            M[7] = pr;
        }
        if (J & 0x40) {
            P[6] = PAL[(L >> 12) & 15];
            M[6] = pr;
        }
        if (J & 0x20) {
            P[5] = PAL[(L >> 20) & 15];
            M[5] = pr;
        }
        if (J & 0x10) {
            P[4] = PAL[(L >> 28)];
            M[4] = pr;
        }
        if (J & 0x08) {
            P[3] = PAL[(L) & 15];
            M[3] = pr;
        }
        if (J & 0x04) {
            P[2] = PAL[(L >> 8) & 15];
            M[2] = pr;
        }
        if (J & 0x02) {
            P[1] = PAL[(L >> 16) & 15];
            M[1] = pr;
        }
        if (J & 0x01) {
            P[0] = PAL[(L >> 24) & 15];
            M[0] = pr;
        }
    }
}


static inline void
PutSpriteHflipM(uchar * P, uchar * C, uint32 * C2, uchar * PAL, int16 h,
                int16 inc, uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

        if (!J)
            continue;

        L = C2[1];
        if ((J & 0x8000) && M[15] <= pr)
            P[15] = PAL[(L >> 4) & 15];
        if ((J & 0x4000) && M[14] <= pr)
            P[14] = PAL[(L >> 12) & 15];
        if ((J & 0x2000) && M[13] <= pr)
            P[13] = PAL[(L >> 20) & 15];
        if ((J & 0x1000) && M[12] <= pr)
            P[12] = PAL[(L >> 28)];
        if ((J & 0x0800) && M[11] <= pr)
            P[11] = PAL[(L) & 15];
        if ((J & 0x0400) && M[10] <= pr)
            P[10] = PAL[(L >> 8) & 15];
        if ((J & 0x0200) && M[9] <= pr)
            P[9] = PAL[(L >> 16) & 15];
        if ((J & 0x0100) && M[8] <= pr)
            P[8] = PAL[(L >> 24) & 15];

        L = C2[0];
        if ((J & 0x80) && M[7] <= pr)
            P[7] = PAL[(L >> 4) & 15];
        if ((J & 0x40) && M[6] <= pr)
            P[6] = PAL[(L >> 12) & 15];
        if ((J & 0x20) && M[5] <= pr)
            P[5] = PAL[(L >> 20) & 15];
        if ((J & 0x10) && M[4] <= pr)
            P[4] = PAL[(L >> 28)];
        if ((J & 0x08) && M[3] <= pr)
            P[3] = PAL[(L) & 15];
        if ((J & 0x04) && M[2] <= pr)
            P[2] = PAL[(L >> 8) & 15];
        if ((J & 0x02) && M[1] <= pr)
            P[1] = PAL[(L >> 16) & 15];
        if ((J & 0x01) && M[0] <= pr)
            P[0] = PAL[(L >> 24) & 15];
    }
}


static inline void
PutSpriteHflip(uchar * P, uchar * C, uint32 * C2, uchar * PAL, int16 h,
    int16 inc)
{
	uint16 J;
	uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

        if (!J)
            continue;

        L = C2[1];
        if (J & 0x8000)
            P[15] = PAL[(L >> 4) & 15];
        if (J & 0x4000)
            P[14] = PAL[(L >> 12) & 15];
        if (J & 0x2000)
            P[13] = PAL[(L >> 20) & 15];
        if (J & 0x1000)
            P[12] = PAL[(L >> 28)];
        if (J & 0x0800)
            P[11] = PAL[(L) & 15];
        if (J & 0x0400)
            P[10] = PAL[(L >> 8) & 15];
        if (J & 0x0200)
            P[9] = PAL[(L >> 16) & 15];
        if (J & 0x0100)
            P[8] = PAL[(L >> 24) & 15];

        L = C2[0];
        if (J & 0x80)
            P[7] = PAL[(L >> 4) & 15];
        if (J & 0x40)
            P[6] = PAL[(L >> 12) & 15];
        if (J & 0x20)
            P[5] = PAL[(L >> 20) & 15];
        if (J & 0x10)
            P[4] = PAL[(L >> 28)];
        if (J & 0x08)
            P[3] = PAL[(L) & 15];
        if (J & 0x04)
            P[2] = PAL[(L >> 8) & 15];
        if (J & 0x02)
            P[1] = PAL[(L >> 16) & 15];
        if (J & 0x01)
            P[0] = PAL[(L >> 24) & 15];
    }
}


/*****************************************************************************

		Function: PutSprite

		Description: convert a sprite from VRAM to normal format
		Parameters: uchar *P (the place where to draw i.e. XBuf[...])
								uchar *C (the buffer toward the sprite to draw)
								uchar *C2 (the buffer of precalculated sprite)
								uchar *R (address of the palette of this sprite [in PAL] )
								int h (the number of line to draw)
								int inc (the value to increment the sprite buffer)
		Return: nothing

*****************************************************************************/
static inline void
PutSprite(uchar * P, uchar * C, uint32 * C2, uchar * PAL, int16 h, int16 inc)
{
	uint16 J;
	uint32 L;

    int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

		if (!J)
			continue;

		L = C2[1];
		if (J & 0x8000)
			P[0] = PAL[(L >> 4) & 15];
		if (J & 0x4000)
			P[1] = PAL[(L >> 12) & 15];
		if (J & 0x2000)
			P[2] = PAL[(L >> 20) & 15];
		if (J & 0x1000)
			P[3] = PAL[(L >> 28)];
		if (J & 0x0800)
			P[4] = PAL[(L) & 15];
		if (J & 0x0400)
			P[5] = PAL[(L >> 8) & 15];
		if (J & 0x0200)
			P[6] = PAL[(L >> 16) & 15];
		if (J & 0x0100)
			P[7] = PAL[(L >> 24) & 15];

		L = C2[0];
		if (J & 0x80)
			P[8] = PAL[(L >> 4) & 15];
		if (J & 0x40)
			P[9] = PAL[(L >> 12) & 15];
		if (J & 0x20)
			P[10] = PAL[(L >> 20) & 15];
		if (J & 0x10)
			P[11] = PAL[(L >> 28)];
		if (J & 0x08)
			P[12] = PAL[(L) & 15];
		if (J & 0x04)
			P[13] = PAL[(L >> 8) & 15];
		if (J & 0x02)
			P[14] = PAL[(L >> 16) & 15];
		if (J & 0x01)
			P[15] = PAL[(L >> 24) & 15];
	}
}


/*****************************************************************************

		Function:	PutSpriteM

		Description: Display a sprite considering priority
		Parameters:
			uchar *P : A Pointer in the buffer where we got to draw the sprite
			uchar *C : A pointer in the video mem where data are available
			uchar *C2 : A pointer in the VRAMS mem
			uchar *R	: A pointer to the current palette
			int h : height of the sprite
			int inc : value of the incrementation for the data
			uchar* M :
			Return:

*****************************************************************************/
static inline void
PutSpriteM(uchar * P, uchar * C, uint32 * C2, uchar * PAL, int16 h, int16 inc,
	uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

    int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

		if (!J)
			continue;

		L = C2[1];
		if ((J & 0x8000) && M[0] <= pr)
			P[0] = PAL[(L >> 4) & 15];
		if ((J & 0x4000) && M[1] <= pr)
			P[1] = PAL[(L >> 12) & 15];
		if ((J & 0x2000) && M[2] <= pr)
			P[2] = PAL[(L >> 20) & 15];
		if ((J & 0x1000) && M[3] <= pr)
			P[3] = PAL[(L >> 28) & 15];
		if ((J & 0x0800) && M[4] <= pr)
			P[4] = PAL[(L) & 15];
		if ((J & 0x0400) && M[5] <= pr)
			P[5] = PAL[(L >> 8) & 15];
		if ((J & 0x0200) && M[6] <= pr)
			P[6] = PAL[(L >> 16) & 15];
		if ((J & 0x0100) && M[7] <= pr)
			P[7] = PAL[(L >> 24) & 15];

		L = C2[0];
		if ((J & 0x80) && M[8] <= pr)
			P[8] = PAL[(L >> 4) & 15];
		if ((J & 0x40) && M[9] <= pr)
			P[9] = PAL[(L >> 12) & 15];
		if ((J & 0x20) && M[10] <= pr)
			P[10] = PAL[(L >> 20) & 15];
		if ((J & 0x10) && M[11] <= pr)
			P[11] = PAL[(L >> 28) & 15];
		if ((J & 0x08) && M[12] <= pr)
			P[12] = PAL[(L) & 15];
		if ((J & 0x04) && M[13] <= pr)
			P[13] = PAL[(L >> 8) & 15];
		if ((J & 0x02) && M[14] <= pr)
			P[14] = PAL[(L >> 16) & 15];
		if ((J & 0x01) && M[15] <= pr)
			P[15] = PAL[(L >> 24) & 15];
	}
}


static inline void
PutSpriteMakeMask(uchar * P, uchar * C, uint32 * C2, uchar * PAL, int16 h,
	int16 inc, uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

	int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];

		if (!J)
			continue;

		L = C2[1];
		if (J & 0x8000) {
			P[0] = PAL[(L >> 4) & 15];
			M[0] = pr;
		}
		if (J & 0x4000) {
			P[1] = PAL[(L >> 12) & 15];
			M[1] = pr;
		}
		if (J & 0x2000) {
			P[2] = PAL[(L >> 20) & 15];
			M[2] = pr;
		}
		if (J & 0x1000) {
			P[3] = PAL[(L >> 28) & 15];
			M[3] = pr;
		}
		if (J & 0x0800) {
			P[4] = PAL[(L) & 15];
			M[4] = pr;
		}
		if (J & 0x0400) {
			P[5] = PAL[(L >> 8) & 15];
			M[5] = pr;
		}
		if (J & 0x0200) {
			P[6] = PAL[(L >> 16) & 15];
			M[6] = pr;
		}
		if (J & 0x0100) {
			P[7] = PAL[(L >> 24) & 15];
			M[7] = pr;
		}

		L = C2[0];
		if (J & 0x80) {
			P[8] = PAL[(L >> 4) & 15];
			M[8] = pr;
		}
		if (J & 0x40) {
			P[9] = PAL[(L >> 12) & 15];
			M[9] = pr;
		}
		if (J & 0x20) {
			P[10] = PAL[(L >> 20) & 15];
			M[10] = pr;
		}
		if (J & 0x10) {
			P[11] = PAL[(L >> 28) & 15];
			M[11] = pr;
		}
		if (J & 0x08) {
			P[12] = PAL[(L) & 15];
			M[12] = pr;
		}
		if (J & 0x04) {
			P[13] = PAL[(L >> 8) & 15];
			M[13] = pr;
		}
		if (J & 0x02) {
			P[14] = PAL[(L >> 16) & 15];
			M[14] = pr;
		}
		if (J & 0x01) {
			P[15] = PAL[(L >> 24) & 15];
			M[15] = pr;
		}
	}
}


/*****************************************************************************

        Function: RefreshScreen

        Description: refresh screen
        Parameters: none
        Return: nothing

*****************************************************************************/
IRAM_ATTR void
RefreshScreen(void)
{
    osd_gfx_blit();

    // We don't clear the parts out of reach of the screen blitter
    // memset(osd_gfx_buffer, Pal[0], 240 * XBUF_WIDTH);
    // memset(SPM, 0, 240 * XBUF_WIDTH);
}


/*****************************************************************************

		Function: RefreshLine

		Description: draw tiles on screen
		Parameters: int Y1,int Y2 (lines to draw between)
		Return: nothing

*****************************************************************************/
IRAM_ATTR void
RefreshLine(int Y1, int Y2)
{
    int X1, XW, Line;
    int x, y, h, offset;

    uchar *PP;

    if (Y1 == 0) {
        TRACE_GFX("\n=================================================\n");
    }

    TRACE_GFX("Rendering lines %3d - %3d\tScroll: (%3d,%3d,%3d)\n",
        Y1, Y2, ScrollX, ScrollY, ScrollYDiff);

    if (!ScreenON || Y1 == Y2) {
        return;
    }

    y = Y1 + ScrollY - ScrollYDiff;
    offset = y & 7;
    h = 8 - offset;
    if (h > Y2 - Y1)
        h = Y2 - Y1;
    y >>= 3;

    PP = (osd_gfx_buffer + XBUF_WIDTH * Y1) - (ScrollX & 7);
    XW = io.screen_w / 8 + 1;

    for (Line = Y1; Line < Y2; y++) {
        x = ScrollX / 8;
        y &= io.bg_h - 1;
        for (X1 = 0; X1 < XW; X1++, x++, PP += 8) {
            uchar *PAL, *P, *C;
        	uint32 *C2;
        	uint32 J, L, no, i;
            x &= io.bg_w - 1;

            no = ((uint16 *) VRAM)[x + y * io.bg_w];

            PAL = &Palette[(no >> 8) & 0x1F0];

            if (no > 0x800) {
                //  MESSAGE_DEBUG("GFX: Access to an invalid VRAM area (tile pattern 0x%04x).\n", no);
            }

            no &= 0x7FF;

            if (OBJ_CACHE.tile_valid[no] == 0) {
                tile2pixel(no);
            }

            C2 = (uint32*)(OBJ_CACHE.data + (no * 8 + offset) * 4);
            C = VRAM + (no * 32 + offset * 2);
            P = PP;
            for (i = 0; i < h; i++, P += XBUF_WIDTH, C2++, C += 2) {

                J = (C[0] | C[1] | C[16] | C[17]);

                if (!J)
                    continue;

                L = C2[0];
                if (J & 0x80)
                    P[0] = PAL[(L >> 4) & 15];
                if (J & 0x40)
                    P[1] = PAL[(L >> 12) & 15];
                if (J & 0x20)
                    P[2] = PAL[(L >> 20) & 15];
                if (J & 0x10)
                    P[3] = PAL[(L >> 28) & 15];
                if (J & 0x08)
                    P[4] = PAL[(L) & 15];
                if (J & 0x04)
                    P[5] = PAL[(L >> 8) & 15];
                if (J & 0x02)
                    P[6] = PAL[(L >> 16) & 15];
                if (J & 0x01)
                    P[7] = PAL[(L >> 24) & 15];
            }
        }
        Line += h;
        PP += XBUF_WIDTH * h - XW * 8;
        offset = 0;
        h = Y2 - Line;
        if (h > 8)
            h = 8;
    }
}


/*****************************************************************************

		Function: RefreshSpriteExact

		Description: draw all sprites between two lines, with the normal method
		Parameters: int Y1, int Y2 (the 'ordonee' to draw between), uchar bg
			(do we draw fg or bg sprites)
		Return: absolutely nothing

*****************************************************************************/
IRAM_ATTR void
RefreshSpriteExact(int Y1, int Y2, uchar bg)
{
    int n;
    SPR *spr;

    spr = (SPR *) SPRAM + 63;

    if (bg == 0)
        sprite_usespbg = 0;

    for (n = 0; n < 64; n++, spr--) {
        int x, y, no, atr, inc, cgx, cgy;
        int pos;
        int h, t, i, j;
        int y_sum;
        int spbg;
        atr = spr->atr;
        spbg = (atr >> 7) & 1;
        if (spbg != bg)
            continue;
        y = (spr->y & 1023) - 64;
        x = (spr->x & 1023) - 32;

        no = spr->no & 2047;
        // 4095 is for supergraphx only
        // no = (unsigned int)(spr->no & 4095);

        TRACE_GFX("Sprite 0x%02X : X = %d, Y = %d, atr = 0x%04X, no = 0x%03X\n",
                    n, x,y, atr, (unsigned long)no);

        cgx = (atr >> 8) & 1;
        cgy = (atr >> 12) & 3;
        cgy |= cgy >> 1;
        no = (no >> 1) & ~(cgy * 2 + cgx);
        if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= io.screen_w
            || x + (cgx + 1) * 16 < 0) {
            continue;
        }

        for (i = 0; i < cgy * 2 + cgx + 1; i++) {
            if (OBJ_CACHE.sprite_valid[no + i] == 0) {
                spr2pixel(no + i);
            }
            if (!cgx)
                i++;
        }

        uchar* R = &Palette[256 + ((atr & 15) << 4)];
        uchar* C = VRAM + (no * 128);
    	uint32* C2 = (uint32 *)(OBJ_CACHE.data + (no * 32) * 4);

        pos = XBUF_WIDTH * (y + 0) + x;
        inc = 2;
        if (atr & V_FLIP) {
            inc = -2;
            C += 15 * 2 + cgy * 256;
            C2 += (15 * 2 + cgy * 64);
        }
        y_sum = 0;

        for (i = 0; i <= cgy; i++) {
            t = Y1 - y - y_sum;
            h = 16;
            if (t > 0) {
                C += t * inc;
                C2 += (t * inc);
                h -= t;
                pos += t * XBUF_WIDTH;
            }

            if (h > Y2 - y - y_sum)
                h = Y2 - y - y_sum;

            if (spbg == 0) {
                sprite_usespbg = 1;
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipMakeMask(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32, R, h, inc,
                            SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteMakeMask(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else if (sprite_usespbg) {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipM(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32, R,
                            h, inc, SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteM(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflip(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32, R, h, inc);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSprite(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32, R, h, inc);
                    }
                }
            }
            pos += h * XBUF_WIDTH;
            C += h * inc + 16 * 7 * inc;
            C2 += h * inc + 16 * inc;
            y_sum += 16;
        }
    }
}
