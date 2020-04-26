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


#include "sprite.h"

#include "utils.h"


#undef TRACE
#if ENABLE_TRACING_SPRITE
#define TRACE(x...) printf("TraceSprite: " x)
#else
#define TRACE(x...)
#endif


uchar BGONSwitch = 1;
// do we have to draw background ?

uchar SPONSwitch = 1;
// Do we have to draw sprites ?

// uint32 spr_init_pos[1024];
// cooked initial position of sprite

void (*RefreshSprite) (int Y1, int Y2, uchar bg);

int ScrollYDiff;
int oldScrollX;
int oldScrollY;
int oldScrollYDiff;

// Actual memory area where the gfx functions are drawing sprites and tiles
uchar *SPM_raw;//[XBUF_WIDTH * XBUF_HEIGHT];
/*static*/ uchar *SPM;// = SPM_raw + XBUF_WIDTH * 64 + 32;

int frame = 0;
// number of frame displayed

/*
	Hit Chesk Sprite#0 and others
*/
int32
CheckSprites(void)
{
	int i, x0, y0, w0, h0, x, y, w, h;
	SPR *spr;

	spr = (SPR *) SPRAM;
	x0 = spr->x;
	y0 = spr->y;
	w0 = (((spr->atr >> 8) & 1) + 1) * 16;
	h0 = (((spr->atr >> 12) & 3) + 1) * 16;
	spr++;
	for (i = 1; i < 64; i++, spr++) {
		x = spr->x;
		y = spr->y;
		w = (((spr->atr >> 8) & 1) + 1) * 16;
		h = (((spr->atr >> 12) & 3) + 1) * 16;
		if ((x < x0 + w0) && (x + w > x0) && (y < y0 + h0) && (y + h > y0))
			return 1;
	}
	return 0;
}


/*****************************************************************************

        Function:   sp2pixel

        Description:convert a PCE coded sprite into a linear one
        Parameters:int no,the number of the sprite to convert
        Return:nothing but update VRAMS

*****************************************************************************/
void
sp2pixel(int no)
{
    uint32 M;
    uchar *C;
    uchar *C2;

    C = &VRAM[no * 128];
    C2 = &VRAMS[no * 32 * 4];
    // 2 longs -> 16 nibbles => 32 loops for a 16*16 spr

    int16 i;
    for (i = 0; i < 32; i++, C++, C2 += 4) {
        uint32 L;
        M = C[0];
        L = ((M & 0x88) >> 3)
            | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
        M = C[32];
        L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16)
            | ((M & 0x11) << 25);
        M = C[64];
        L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17)
            | ((M & 0x11) << 26);
        M = C[96];
        L |= ((M & 0x88)) | ((M & 0x44) << 9) | ((M & 0x22) << 18)
            | ((M & 0x11) << 27);
        /* C2[0] = L; */
        C2[0] = L & 0xff;
        C2[1] = (L >> 8) & 0xff;
        C2[2] = (L >> 16) & 0xff;
        C2[3] = (L >> 24) & 0xff;
    }
}


void
PutSpriteHflipMakeMask(uchar * P, uchar * C, uchar * C2, uchar * R,
    int16 h, int16 inc, uchar * M, uchar pr)
{
    uint16 J;
    uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
        P += XBUF_WIDTH, M += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
            | (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif
        if (!J)
            continue;
        L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);   //sp2pixel(C+1);
        if (J & 0x8000) {
            P[15] = PAL((L >> 4) & 15);
            M[15] = pr;
        }
        if (J & 0x4000) {
            P[14] = PAL((L >> 12) & 15);
            M[14] = pr;
        }
        if (J & 0x2000) {
            P[13] = PAL((L >> 20) & 15);
            M[13] = pr;
        }
        if (J & 0x1000) {
            P[12] = PAL((L >> 28));
            M[12] = pr;
        }
        if (J & 0x0800) {
            P[11] = PAL((L) & 15);
            M[11] = pr;
        }
        if (J & 0x0400) {
            P[10] = PAL((L >> 8) & 15);
            M[10] = pr;
        }
        if (J & 0x0200) {
            P[9] = PAL((L >> 16) & 15);
            M[9] = pr;
        }
        if (J & 0x0100) {
            P[8] = PAL((L >> 24) & 15);
            M[8] = pr;
        }
        /* L = C2[0];                        *///sp2pixel(C);
        L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
        if (J & 0x80) {
            P[7] = PAL((L >> 4) & 15);
            M[7] = pr;
        }
        if (J & 0x40) {
            P[6] = PAL((L >> 12) & 15);
            M[6] = pr;
        }
        if (J & 0x20) {
            P[5] = PAL((L >> 20) & 15);
            M[5] = pr;
        }
        if (J & 0x10) {
            P[4] = PAL((L >> 28));
            M[4] = pr;
        }
        if (J & 0x08) {
            P[3] = PAL((L) & 15);
            M[3] = pr;
        }
        if (J & 0x04) {
            P[2] = PAL((L >> 8) & 15);
            M[2] = pr;
        }
        if (J & 0x02) {
            P[1] = PAL((L >> 16) & 15);
            M[1] = pr;
        }
        if (J & 0x01) {
            P[0] = PAL((L >> 24) & 15);
            M[0] = pr;
        }
    }
}


void
PutSpriteHflipM(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
                int16 inc, uchar * M, uchar pr)
{
    uint16 J;
    uint32 L;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
        P += XBUF_WIDTH, M += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8)) | (C[64]
            + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif
        if (!J)
            continue;

        /* L = C2[1];        *///sp2pixel(C+1);
        L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);
        if ((J & 0x8000) && M[15] <= pr)
            P[15] = PAL((L >> 4) & 15);
        if ((J & 0x4000) && M[14] <= pr)
            P[14] = PAL((L >> 12) & 15);
        if ((J & 0x2000) && M[13] <= pr)
            P[13] = PAL((L >> 20) & 15);
        if ((J & 0x1000) && M[12] <= pr)
            P[12] = PAL((L >> 28));
        if ((J & 0x0800) && M[11] <= pr)
            P[11] = PAL((L) & 15);
        if ((J & 0x0400) && M[10] <= pr)
            P[10] = PAL((L >> 8) & 15);
        if ((J & 0x0200) && M[9] <= pr)
            P[9] = PAL((L >> 16) & 15);
        if ((J & 0x0100) && M[8] <= pr)
            P[8] = PAL((L >> 24) & 15);
        /* L = C2[0];        *///sp2pixel(C);
        L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
        if ((J & 0x80) && M[7] <= pr)
            P[7] = PAL((L >> 4) & 15);
        if ((J & 0x40) && M[6] <= pr)
            P[6] = PAL((L >> 12) & 15);
        if ((J & 0x20) && M[5] <= pr)
            P[5] = PAL((L >> 20) & 15);
        if ((J & 0x10) && M[4] <= pr)
            P[4] = PAL((L >> 28));
        if ((J & 0x08) && M[3] <= pr)
            P[3] = PAL((L) & 15);
        if ((J & 0x04) && M[2] <= pr)
            P[2] = PAL((L >> 8) & 15);
        if ((J & 0x02) && M[1] <= pr)
            P[1] = PAL((L >> 16) & 15);
        if ((J & 0x01) && M[0] <= pr)
            P[0] = PAL((L >> 24) & 15);
    }
}


void
PutSpriteHflip(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
    int16 inc)
{
    uint16 J;
    uint32 L;

    // TODO: This is a hack, see issue #1
    // the graphics are mis-aligned on flipped sprites when we go
    // with the uchar that everything else uses.
    uint32* C2r = (uint32*)C2;

    int16 i;
    for (i = 0; i < h; i++, C += inc, C2r += inc, P += XBUF_WIDTH) {
        J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
            | (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
        J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
            | ((uint16 *) C)[48];
#endif

        if (!J)
            continue;
        L = C2r[1];             //sp2pixel(C+1);
        if (J & 0x8000)
            P[15] = PAL((L >> 4) & 15);
        if (J & 0x4000)
            P[14] = PAL((L >> 12) & 15);
        if (J & 0x2000)
            P[13] = PAL((L >> 20) & 15);
        if (J & 0x1000)
            P[12] = PAL((L >> 28));
        if (J & 0x0800)
            P[11] = PAL((L) & 15);
        if (J & 0x0400)
            P[10] = PAL((L >> 8) & 15);
        if (J & 0x0200)
            P[9] = PAL((L >> 16) & 15);
        if (J & 0x0100)
            P[8] = PAL((L >> 24) & 15);
        L = C2r[0];             //sp2pixel(C);
        if (J & 0x80)
            P[7] = PAL((L >> 4) & 15);
        if (J & 0x40)
            P[6] = PAL((L >> 12) & 15);
        if (J & 0x20)
            P[5] = PAL((L >> 20) & 15);
        if (J & 0x10)
            P[4] = PAL((L >> 28));
        if (J & 0x08)
            P[3] = PAL((L) & 15);
        if (J & 0x04)
            P[2] = PAL((L >> 8) & 15);
        if (J & 0x02)
            P[1] = PAL((L >> 16) & 15);
        if (J & 0x01)
            P[0] = PAL((L >> 24) & 15);
    }
}


/*****************************************************************************

        Function:   plane2pixel

        Description: convert a PCE coded tile into a linear one
        Parameters:int no, the number of the tile to convert
        Return:nothing, but updates VRAM2

*****************************************************************************/
void
plane2pixel(int no)
{
    uint32 M;
    uchar *C = VRAM + no * 32;
    uchar *C2 = VRAM2 + no * 8 * 4;
    uint32 L;

    int16 i;
    TRACE("Planing tile %d\n", no);
    for (i = 0; i < 8; i++, C += 2, C2 += 4) {
        M = C[0];
        TRACE("C[0]=%02X\n", M);
        L = ((M & 0x88) >> 3)
            | ((M & 0x44) << 6) | ((M & 0x22) << 15)
            | ((M & 0x11) << 24);
        M = C[1];
        TRACE("C[1]=%02X\n", M);
        L |= ((M & 0x88) >> 2)
            | ((M & 0x44) << 7) | ((M & 0x22) << 16)
            | ((M & 0x11) << 25);
        M = C[16];
        TRACE("C[16]=%02X\n", M);
        L |= ((M & 0x88) >> 1)
            | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
        M = C[17];
        TRACE("C[17]=%02X\n", M);
        L |= ((M & 0x88))
            | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);
        /* C2[0] = L;                        //37261504 */
        C2[0] = L & 0xff;
        C2[1] = (L >> 8) & 0xff;
        C2[2] = (L >> 16) & 0xff;
        C2[3] = (L >> 24) & 0xff;
        TRACE("L=%04X\n", L);
    }
}


/*****************************************************************************

        Function: RefreshScreen

        Description: refresh screen
        Parameters: none
        Return: nothing

*****************************************************************************/
void
RefreshScreen(void)
{
    frame += UPeriod + 1;
    /*
       {
       char* spr = SPRAM;
       uint32 CRC = -1;
       int index;
       for (index = 0; index < 64 * sizeof(SPR); index++) {
       spr[index] ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[spr[index]];
       }
       Log("frame %d : CRC = %X\n", frame, CRC);

       {
       char* spr = ((uint16*)VRAM) + 2048;
       uint32 CRC = -1;
       int index;
       for (index = 0; index < 64 * sizeof(SPR); index++) {
       char tmp = spr[index];
       tmp ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[tmp];
       }
       Log("frame %d : CRC VRAM[2048-] = %X\n", frame, CRC);
       }
     */

    (*osd_gfx_driver_list[video_driver].draw) ();

#if ENABLE_TRACING_GFX
    /*
       Log("VRAM: %02x%02x%02x%02x %02x%02x%02x%02x\n",
       VRAM[0],
       VRAM[1],
       VRAM[2],
       VRAM[3],
       VRAM[4],
       VRAM[5],
       VRAM[6],
       VRAM[7]);
     */
    {
        int index;
        uchar tmp_data;
        uint32 CRC = 0xFFFFFFFF;

        for (index = 0; index < 0x10000; index++) {
            tmp_data = VRAM2[index];
            tmp_data ^= CRC;
            CRC >>= 8;
            CRC ^= TAB_CONST[tmp_data];
        }
        Log("VRAM2 CRC = %08x\n", ~CRC);

        for (index = 0; index < 0x10000; index++) {
            if (VRAM2[index] != 0)
                Log("%04X:%08X\n", index, VRAM2[index]);
        }
    }

    /*
       {
       int index;
       uchar tmp_data;
       unsigned int CRC = 0xFFFFFFFF;

       for (index = 0; index < 256; index++) {
       tmp_data = zp_base[index];
       tmp_data ^= CRC;
       CRC >>= 8;
       CRC ^= TAB_CONST[tmp_data];
       }
       Log("ZP CRC = %08x\n", ~CRC);
       }
     */
#endif
    //memset(osd_gfx_buffer, Pal[0], 240 * XBUF_WIDTH);
    // We don't clear the part out of reach of the screen blitter
    //memset(SPM, 0, 240 * XBUF_WIDTH);
}


/*****************************************************************************

		Function: RefreshLine

		Description: draw tiles on screen
		Parameters: int Y1,int Y2 (lines to draw between)
		Return: nothing

*****************************************************************************/
void
RefreshLine(int Y1, int Y2)
{
    ODROID_DEBUG_PERF_START2(debug_perf_part_refr)
    int X1, XW, Line;
    int x, y, h, offset;

    uchar *PP;
    Y2++;

#if ENABLE_TRACING_GFX
    if (Y1 == 0) {
        TRACE("\n= %d =================================================\n",
            frame);
    }
    TRACE("Rendering lines %3d - %3d\tScroll: (%3d,%3d,%3d)\n",
        Y1, Y2, ScrollX, ScrollY, ScrollYDiff);
#endif

    PP = osd_gfx_buffer + XBUF_WIDTH * Y1;

    if (ScreenON && BGONSwitch) {
        y = Y1 + ScrollY - ScrollYDiff;
        offset = y & 7;
        h = 8 - offset;
        if (h > Y2 - Y1)
            h = Y2 - Y1;
        y >>= 3;
        PP -= ScrollX & 7;
        XW = io.screen_w / 8 + 1;

        for (Line = Y1; Line < Y2; y++) {
            x = ScrollX / 8;
            y &= io.bg_h - 1;
            for (X1 = 0; X1 < XW; X1++, x++, PP += 8) {
                uchar *R, *P, *C;
                uchar *C2;
                int no, i;
                x &= io.bg_w - 1;

#if defined(WORDS_BIGENDIAN)
                no = VRAM[(x + y * io.bg_w) << 1]
                    + (VRAM[((x + y * io.bg_w) << 1) + 1] << 8);
#else
                no = ((uint16 *) VRAM)[x + y * io.bg_w];
#endif

                R = &Pal[(no >> 12) * 16];

#if ENABLE_TRACING_GFX
                // Old code was only no &= 0xFFF
                if ((no & 0xFFF) > 0x800) {
                    TRACE("GFX: Access to an invalid VRAM area "
                        "(tile pattern 0x%04x).\n", no);
                }
#endif
                no &= 0x7FF;

                if (vchange[no]) {
                    vchange[no] = 0;
                    plane2pixel(no);
                }
                C2 = (VRAM2 + (no * 8 + offset) * 4);
                C = VRAM + (no * 32 + offset * 2);
                P = PP;
                for (i = 0; i < h; i++, P += XBUF_WIDTH, C2 += 4, C += 2) {
                    uint32 L;
                    uint16 J;
                    J = (C[0] | C[1] | C[16] | C[17]);
                    if (!J)
                        continue;

                    L = C2[0] + (C2[1] << 8) + (C2[2] << 16)
                        + (C2[3] << 24);
                    if (J & 0x80)
                        P[0] = PAL((L >> 4) & 15);
                    if (J & 0x40)
                        P[1] = PAL((L >> 12) & 15);
                    if (J & 0x20)
                        P[2] = PAL((L >> 20) & 15);
                    if (J & 0x10)
                        P[3] = PAL((L >> 28));
                    if (J & 0x08)
                        P[4] = PAL((L) & 15);
                    if (J & 0x04)
                        P[5] = PAL((L >> 8) & 15);
                    if (J & 0x02)
                        P[6] = PAL((L >> 16) & 15);
                    if (J & 0x01)
                        P[7] = PAL((L >> 24) & 15);
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
    ODROID_DEBUG_PERF_INCR2(debug_perf_part_refr, ODROID_DEBUG_PERF_SPRITE_RefreshLine)
}


#define	SPBG	0x80
#define	CGX		0x100


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
void
PutSprite(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h, int16 inc)
{
	uint16 J;
	uint32 L;

	int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc * 4, P += XBUF_WIDTH) {
#if defined(WORDS_BIGENDIAN)
		J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
			| (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));
#else
		J = ((uint16 *) C)[0] | ((uint16 *) C)[16]
			| ((uint16 *) C)[32] | ((uint16 *) C)[48];
#endif

		if (!J)
			continue;

		L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);	//sp2pixel(C+1);
		if (J & 0x8000)
			P[0] = PAL((L >> 4) & 15);
		if (J & 0x4000)
			P[1] = PAL((L >> 12) & 15);
		if (J & 0x2000)
			P[2] = PAL((L >> 20) & 15);
		if (J & 0x1000)
			P[3] = PAL((L >> 28));
		if (J & 0x0800)
			P[4] = PAL((L) & 15);
		if (J & 0x0400)
			P[5] = PAL((L >> 8) & 15);
		if (J & 0x0200)
			P[6] = PAL((L >> 16) & 15);
		if (J & 0x0100)
			P[7] = PAL((L >> 24) & 15);

		L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);	//sp2pixel(C);
		if (J & 0x80)
			P[8] = PAL((L >> 4) & 15);
		if (J & 0x40)
			P[9] = PAL((L >> 12) & 15);
		if (J & 0x20)
			P[10] = PAL((L >> 20) & 15);
		if (J & 0x10)
			P[11] = PAL((L >> 28));
		if (J & 0x08)
			P[12] = PAL((L) & 15);
		if (J & 0x04)
			P[13] = PAL((L >> 8) & 15);
		if (J & 0x02)
			P[14] = PAL((L >> 16) & 15);
		if (J & 0x01)
			P[15] = PAL((L >> 24) & 15);
	}
}


void
PutSpriteHandleFull(uchar * P, uchar * C, uchar * C2, uchar * R,
	int16 h, int16 inc)
{
	uint16 J;
	uint32 L;

	int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc, P += XBUF_WIDTH) {
		J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8)) | (C[64]
			+ (C[65] << 8)) | (C[96] + (C[97] << 8));
#if 0
		J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
			| ((uint16 *) C)[48];
#endif
		if (!J)
			continue;
		if (J == 65535) {
			L = C2[1];			//sp2pixel(C+1);

			P[0] = PAL((L >> 4) & 15);
			P[1] = PAL((L >> 12) & 15);
			P[2] = PAL((L >> 20) & 15);
			P[3] = PAL((L >> 28));
			P[4] = PAL((L) & 15);
			P[5] = PAL((L >> 8) & 15);
			P[6] = PAL((L >> 16) & 15);
			P[7] = PAL((L >> 24) & 15);
			L = C2[0];			//sp2pixel(C);
			P[8] = PAL((L >> 4) & 15);
			P[9] = PAL((L >> 12) & 15);
			P[10] = PAL((L >> 20) & 15);
			P[11] = PAL((L >> 28));
			P[12] = PAL((L) & 15);
			P[13] = PAL((L >> 8) & 15);
			P[14] = PAL((L >> 16) & 15);
			P[15] = PAL((L >> 24) & 15);

			return;
		}

		L = C2[1];				//sp2pixel(C+1);
		if (J & 0x8000)
			P[0] = PAL((L >> 4) & 15);
		if (J & 0x4000)
			P[1] = PAL((L >> 12) & 15);
		if (J & 0x2000)
			P[2] = PAL((L >> 20) & 15);
		if (J & 0x1000)
			P[3] = PAL((L >> 28));
		if (J & 0x0800)
			P[4] = PAL((L) & 15);
		if (J & 0x0400)
			P[5] = PAL((L >> 8) & 15);
		if (J & 0x0200)
			P[6] = PAL((L >> 16) & 15);
		if (J & 0x0100)
			P[7] = PAL((L >> 24) & 15);
		L = C2[0];				//sp2pixel(C);
		if (J & 0x80)
			P[8] = PAL((L >> 4) & 15);
		if (J & 0x40)
			P[9] = PAL((L >> 12) & 15);
		if (J & 0x20)
			P[10] = PAL((L >> 20) & 15);
		if (J & 0x10)
			P[11] = PAL((L >> 28));
		if (J & 0x08)
			P[12] = PAL((L) & 15);
		if (J & 0x04)
			P[13] = PAL((L >> 8) & 15);
		if (J & 0x02)
			P[14] = PAL((L >> 16) & 15);
		if (J & 0x01)
			P[15] = PAL((L >> 24) & 15);
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
void
PutSpriteM(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h, int16 inc,
	uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

	int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
		P += XBUF_WIDTH, M += XBUF_WIDTH) {
		J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
			| (C[64] + (C[65] << 8)) | (C[96] + (C[97] << 8));

#if 0
		J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
			| ((uint16 *) C)[48];
#endif
		// fprintf(stderr, "Masked : %lX\n", J);
		if (!J)
			continue;

		/* L = C2[1];        *///sp2pixel(C+1);
		L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);

		if ((J & 0x8000) && M[0] <= pr)
			P[0] = PAL((L >> 4) & 15);
		if ((J & 0x4000) && M[1] <= pr)
			P[1] = PAL((L >> 12) & 15);
		if ((J & 0x2000) && M[2] <= pr)
			P[2] = PAL((L >> 20) & 15);
		if ((J & 0x1000) && M[3] <= pr)
			P[3] = PAL((L >> 28));
		if ((J & 0x0800) && M[4] <= pr)
			P[4] = PAL((L) & 15);
		if ((J & 0x0400) && M[5] <= pr)
			P[5] = PAL((L >> 8) & 15);
		if ((J & 0x0200) && M[6] <= pr)
			P[6] = PAL((L >> 16) & 15);
		if ((J & 0x0100) && M[7] <= pr)
			P[7] = PAL((L >> 24) & 15);
		/* L = C2[0];        *///sp2pixel(C);
		L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
		if ((J & 0x80) && M[8] <= pr)
			P[8] = PAL((L >> 4) & 15);
		if ((J & 0x40) && M[9] <= pr)
			P[9] = PAL((L >> 12) & 15);
		if ((J & 0x20) && M[10] <= pr)
			P[10] = PAL((L >> 20) & 15);
		if ((J & 0x10) && M[11] <= pr)
			P[11] = PAL((L >> 28));
		if ((J & 0x08) && M[12] <= pr)
			P[12] = PAL((L) & 15);
		if ((J & 0x04) && M[13] <= pr)
			P[13] = PAL((L >> 8) & 15);
		if ((J & 0x02) && M[14] <= pr)
			P[14] = PAL((L >> 16) & 15);
		if ((J & 0x01) && M[15] <= pr)
			P[15] = PAL((L >> 24) & 15);
	}
}

void
PutSpriteMakeMask(uchar * P, uchar * C, uchar * C2, uchar * R, int16 h,
	int16 inc, uchar * M, uchar pr)
{
	uint16 J;
	uint32 L;

	int16 i;
	for (i = 0; i < h; i++, C += inc, C2 += inc * 4,
		P += XBUF_WIDTH, M += XBUF_WIDTH) {

#if 0
		J = ((uint16 *) C)[0] | ((uint16 *) C)[16] | ((uint16 *) C)[32]
			| ((uint16 *) C)[48];
#endif

		J = (C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8)) | (C[64]
			+ (C[65] << 8)) | (C[96] + (C[97] << 8));

#if 0
		if ((uint16) J !=
			(uint16) ((C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8))
			| (C[64] + (C[65] << 8)) | (C[92] + (C[93] << 8)))) {
			Log("J != ... ( 0x%x != 0x%x )\n", J,
				(C[0] + (C[1] << 8)) | (C[32] + (C[33] << 8)) | (C[64]
				+ (C[65] << 8)) | (C[92] + (C[93] << 8)));
			Log("((uint16 *) C)[0] = %x\t(C[0] + (C[1] << 8)) = %x\n",
				((uint16 *) C)[0], (C[0] + (C[1] << 8)));
			Log("((uint16 *) C)[16] = %x\t(C[32] + (C[33] << 8)) = %x\n",
				((uint16 *) C)[16], (C[32] + (C[33] << 8)));
			Log("((uint16 *) C)[32] = %x\t(C[64] + (C[65] << 8)) = %x\n",
				((uint16 *) C)[32], (C[64] + (C[65] << 8)));
			Log("((uint16 *) C)[48] = %x\t(C[92] + (C[93] << 8)) = %x\n",
				((uint16 *) C)[48], (C[92] + (C[93] << 8)));
			Log("& ((uint16 *) C)[48] = %p\t&C[92] = %p\n",
				&((uint16 *) C)[48], &C[92]);
		}
#endif

		if (!J)
			continue;
		/* L = C2[1];        *///sp2pixel(C+1);
		L = C2[4] + (C2[5] << 8) + (C2[6] << 16) + (C2[7] << 24);
		if (J & 0x8000) {
			P[0] = PAL((L >> 4) & 15);
			M[0] = pr;
		}
		if (J & 0x4000) {
			P[1] = PAL((L >> 12) & 15);
			M[1] = pr;
		}
		if (J & 0x2000) {
			P[2] = PAL((L >> 20) & 15);
			M[2] = pr;
		}
		if (J & 0x1000) {
			P[3] = PAL((L >> 28));
			M[3] = pr;
		}
		if (J & 0x0800) {
			P[4] = PAL((L) & 15);
			M[4] = pr;
		}
		if (J & 0x0400) {
			P[5] = PAL((L >> 8) & 15);
			M[5] = pr;
		}
		if (J & 0x0200) {
			P[6] = PAL((L >> 16) & 15);
			M[6] = pr;
		}
		if (J & 0x0100) {
			P[7] = PAL((L >> 24) & 15);
			M[7] = pr;
		}
		/* L = C2[0];        *///sp2pixel(C);
		L = C2[0] + (C2[1] << 8) + (C2[2] << 16) + (C2[3] << 24);
		if (J & 0x80) {
			P[8] = PAL((L >> 4) & 15);
			M[8] = pr;
		}
		if (J & 0x40) {
			P[9] = PAL((L >> 12) & 15);
			M[9] = pr;
		}
		if (J & 0x20) {
			P[10] = PAL((L >> 20) & 15);
			M[10] = pr;
		}
		if (J & 0x10) {
			P[11] = PAL((L >> 28));
			M[11] = pr;
		}
		if (J & 0x08) {
			P[12] = PAL((L) & 15);
			M[12] = pr;
		}
		if (J & 0x04) {
			P[13] = PAL((L >> 8) & 15);
			M[13] = pr;
		}
		if (J & 0x02) {
			P[14] = PAL((L >> 16) & 15);
			M[14] = pr;
		}
		if (J & 0x01) {
			P[15] = PAL((L >> 24) & 15);
			M[15] = pr;
		}
	}
}


/*****************************************************************************

		Function: RefreshSpriteExact

		Description: draw all sprites between two lines, with the normal method
		Parameters: int Y1, int Y2 (the 'ordonee' to draw between), uchar bg
			(do we draw fg or bg sprites)
		Return: absolutly nothing

*****************************************************************************/
void
RefreshSpriteExact(int Y1, int Y2, uchar bg)
{
    ODROID_DEBUG_PERF_START2(debug_perf_part1)
    int n;
    SPR *spr;

    /* TEST */
    Y2++;

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

#if ENABLE_TRACING_GFX
        /*
           Log("Sprite 0x%02X : X = %d, Y = %d, atr = 0x%04X, no = 0x%03X\n",
           n,
           x,
           y,
           atr,
           (unsigned long)no);
         */
#endif

        cgx = (atr >> 8) & 1;
        cgy = (atr >> 12) & 3;
        cgy |= cgy >> 1;
        no = (no >> 1) & ~(cgy * 2 + cgx);
        if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= FC_W
            || x + (cgx + 1) * 16 < 0) {
            continue;
        }

        uchar* R = &SPal[(atr & 15) * 16];
        for (i = 0; i < cgy * 2 + cgx + 1; i++) {
            if (vchanges[no + i]) {
                vchanges[no + i] = 0;
                sp2pixel(no + i);
            }
            if (!cgx)
                i++;
        }
        uchar* C = VRAM + (no * 128);
        uchar* C2 = VRAMS + (no * 32) * 4;  /* TEST */
        pos = XBUF_WIDTH * (y + 0) + x;
        inc = 2;
        if (atr & V_FLIP) {
            inc = -2;
            C += 15 * 2 + cgy * 256;
            C2 += (15 * 2 + cgy * 64) * 4;
        }
        y_sum = 0;

        for (i = 0; i <= cgy; i++) {
            t = Y1 - y - y_sum;
            h = 16;
            if (t > 0) {
                C += t * inc;
                C2 += (t * inc) * 4;
                h -= t;
                pos += t * XBUF_WIDTH;
            }
            if (h > Y2 - y - y_sum)
                h = Y2 - y - y_sum;
            if (spbg == 0) {
                sprite_usespbg = 1;
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipMakeMask(osd_gfx_buffer + pos
                            + (cgx - j) * 16, C + j * 128, C2 + j * 32 * 4, R,
                            h, inc, SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteMakeMask(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else if (sprite_usespbg) {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflipM(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R,
                            h, inc, SPM + pos + (cgx - j) * 16, n);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteM(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc,
                            SPM + pos + j * 16, n);
                    }
                }
            } else {
                if (atr & H_FLIP) {
                    for (j = 0; j <= cgx; j++) {
                        PutSpriteHflip(osd_gfx_buffer + pos + (cgx - j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc);
                    }
                } else {
                    for (j = 0; j <= cgx; j++) {
                        PutSprite(osd_gfx_buffer + pos + (j) * 16,
                            C + j * 128, C2 + j * 32 * 4, R, h, inc);
                    }
                }
            }
            pos += h * XBUF_WIDTH;
            C += h * inc + 16 * 7 * inc;
            C2 += (h * inc + 16 * inc) * 4;
            y_sum += 16;
        }
    }
    ODROID_DEBUG_PERF_INCR2(debug_perf_part1, ODROID_DEBUG_PERF_SPRITE_RefreshSpriteExact)
}


char tmp_str[10];
extern int vheight;
extern char *sbuf[];
int sprite_usespbg = 0;


