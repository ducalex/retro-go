// gfx.c - VDC General emulation
//
#include "pce.h"
#include "config.h"

static gfx_context_t saved_gfx_context[4];

//
int ScrollYDiff;

static bool gfx_need_redraw = 0;
static bool sprite_usespbg = 0;
static int display_counter = 0;
static int last_display_counter = 0;

// Cache for linear tiles and sprites. This is basically a decoded VRAM
bool TILE_CACHE[2048];
bool SPR_CACHE[512];
uint32 *OBJ_CACHE;

// Sprite priority mask memory
static uchar *SPM, *SPM_ptr;

#define PAL(nibble) (PAL[(L >> (nibble * 4)) & 15]);


/*****************************************************************************

		Function: spr2pixel

		Description: convert a PCE coded sprite into a linear one
		Parameters: int no,the number of the sprite to convert
		Return: nothing

*****************************************************************************/
static inline void
spr2pixel(int no)
{
	uint32 M, L, i;
	uchar *C = VRAM + no * 128;
	uint32 *C2 = OBJ_CACHE + no * 32;
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
	SPR_CACHE[no] = 1;
}


/*****************************************************************************

		Function: tile2pixel

		Description: convert a PCE coded tile into a linear one
		Parameters: int no, the number of the tile to convert
		Return: nothing

*****************************************************************************/
static inline void
tile2pixel(int no)
{
	uint32 M, L, i;
	uchar *C = VRAM + no * 32;
	uint32 *C2 = OBJ_CACHE + no * 8;

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
	TILE_CACHE[no] = 1;
}


/*****************************************************************************

		Function: draw_sprite

		Description: draw sprite to framebuffer
		Parameters: int offset (offset in SPM/framebuffer memory to use)
					uint16 *C (sprite to draw)
					uint32 *C2 (cached linear sprite)
					uchar *PAL (address of the palette of this sprite)
					int h (the number of line to draw)
					int inc (the value to increment the sprite buffer)
					uchar *M (mask priority)
					uchar pr (priority to compare against M)
					bool hflip (flip tile horizontally)
					bool update_mask (update priority mask)
		Return: nothing

*****************************************************************************/
static inline void
draw_sprite(int offset, uint16 *C, uint32 *C2, uchar *PAL, int h, int inc,
	uchar pr, bool hflip, bool update_mask)
{
	uchar *P = osd_gfx_buffer + offset;
	uchar *M = SPM + offset;

	for (int i = 0; i < h; i++, C = (uint16*)((uchar*)C + inc),
			C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		uint16 J = C[0] | C[16] | C[32] | C[48];
		uint32 L;

		if (!J)
			continue;

		if (hflip) {
			L = C2[1];
			if ((J & 0x8000) && M[15] <= pr) P[15] = PAL(1);
			if ((J & 0x4000) && M[14] <= pr) P[14] = PAL(3);
			if ((J & 0x2000) && M[13] <= pr) P[13] = PAL(5);
			if ((J & 0x1000) && M[12] <= pr) P[12] = PAL(7);
			if ((J & 0x0800) && M[11] <= pr) P[11] = PAL(0);
			if ((J & 0x0400) && M[10] <= pr) P[10] = PAL(2);
			if ((J & 0x0200) && M[9] <= pr)  P[9]  = PAL(4);
			if ((J & 0x0100) && M[8] <= pr)  P[8]  = PAL(6);

			L = C2[0];
			if ((J & 0x80) && M[7] <= pr) P[7] = PAL(1);
			if ((J & 0x40) && M[6] <= pr) P[6] = PAL(3);
			if ((J & 0x20) && M[5] <= pr) P[5] = PAL(5);
			if ((J & 0x10) && M[4] <= pr) P[4] = PAL(7);
			if ((J & 0x08) && M[3] <= pr) P[3] = PAL(0);
			if ((J & 0x04) && M[2] <= pr) P[2] = PAL(2);
			if ((J & 0x02) && M[1] <= pr) P[1] = PAL(4);
			if ((J & 0x01) && M[0] <= pr) P[0] = PAL(6);
		}
		else {
			L = C2[1];
			if ((J & 0x8000) && M[0] <= pr) P[0] = PAL(1);
			if ((J & 0x4000) && M[1] <= pr) P[1] = PAL(3);
			if ((J & 0x2000) && M[2] <= pr) P[2] = PAL(5);
			if ((J & 0x1000) && M[3] <= pr) P[3] = PAL(7);
			if ((J & 0x0800) && M[4] <= pr) P[4] = PAL(0);
			if ((J & 0x0400) && M[5] <= pr) P[5] = PAL(2);
			if ((J & 0x0200) && M[6] <= pr) P[6] = PAL(4);
			if ((J & 0x0100) && M[7] <= pr) P[7] = PAL(6);

			L = C2[0];
			if ((J & 0x80) && M[8] <= pr)  P[8]  = PAL(1);
			if ((J & 0x40) && M[9] <= pr)  P[9]  = PAL(3);
			if ((J & 0x20) && M[10] <= pr) P[10] = PAL(5);
			if ((J & 0x10) && M[11] <= pr) P[11] = PAL(7);
			if ((J & 0x08) && M[12] <= pr) P[12] = PAL(0);
			if ((J & 0x04) && M[13] <= pr) P[13] = PAL(2);
			if ((J & 0x02) && M[14] <= pr) P[14] = PAL(4);
			if ((J & 0x01) && M[15] <= pr) P[15] = PAL(6);
		}

		if (update_mask) {
			if (hflip) {
				M[15] = (J & 0x8000) ? pr : 0;
				M[14] = (J & 0x4000) ? pr : 0;
				M[13] = (J & 0x2000) ? pr : 0;
				M[12] = (J & 0x1000) ? pr : 0;
				M[11] = (J & 0x0800) ? pr : 0;
				M[10] = (J & 0x0400) ? pr : 0;
				M[9]  = (J & 0x0200) ? pr : 0;
				M[8]  = (J & 0x0100) ? pr : 0;
				M[7]  = (J & 0x0080) ? pr : 0;
				M[6]  = (J & 0x0040) ? pr : 0;
				M[5]  = (J & 0x0020) ? pr : 0;
				M[4]  = (J & 0x0010) ? pr : 0;
				M[3]  = (J & 0x0008) ? pr : 0;
				M[2]  = (J & 0x0004) ? pr : 0;
				M[1]  = (J & 0x0002) ? pr : 0;
				M[0]  = (J & 0x0001) ? pr : 0;
			}
			else {
				M[0]  = (J & 0x8000) ? pr : 0;
				M[1]  = (J & 0x4000) ? pr : 0;
				M[2]  = (J & 0x2000) ? pr : 0;
				M[3]  = (J & 0x1000) ? pr : 0;
				M[4]  = (J & 0x0800) ? pr : 0;
				M[5]  = (J & 0x0400) ? pr : 0;
				M[6]  = (J & 0x0200) ? pr : 0;
				M[7]  = (J & 0x0100) ? pr : 0;
				M[8]  = (J & 0x0080) ? pr : 0;
				M[9]  = (J & 0x0040) ? pr : 0;
				M[10] = (J & 0x0020) ? pr : 0;
				M[11] = (J & 0x0010) ? pr : 0;
				M[12] = (J & 0x0008) ? pr : 0;
				M[13] = (J & 0x0004) ? pr : 0;
				M[14] = (J & 0x0002) ? pr : 0;
				M[15] = (J & 0x0001) ? pr : 0;
			}
		}
	}
}


/*****************************************************************************

		Function: draw_tiles

		Description: draw tiles on screen
		Parameters: int Y1,int Y2 (lines to draw between)
		Return: nothing

*****************************************************************************/
static inline void
draw_tiles(int Y1, int Y2)
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

			if (TILE_CACHE[no] == 0) {
				tile2pixel(no);
			}

			C2 = OBJ_CACHE + (no * 8 + offset);
			C = VRAM + (no * 32 + offset * 2);
			P = PP;
			for (i = 0; i < h; i++, P += XBUF_WIDTH, C2++, C += 2) {

				J = C[0] | C[1] | C[16] | C[17];

				if (!J)
					continue;

				L = C2[0];
				if (J & 0x80) P[0] = PAL(1);
				if (J & 0x40) P[1] = PAL(3);
				if (J & 0x20) P[2] = PAL(5);
				if (J & 0x10) P[3] = PAL(7);
				if (J & 0x08) P[4] = PAL(0);
				if (J & 0x04) P[5] = PAL(2);
				if (J & 0x02) P[6] = PAL(4);
				if (J & 0x01) P[7] = PAL(6);
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

		Function: draw_sprites

		Description: draw all sprites between two lines, with the normal method
		Parameters: int Y1, int Y2 (the 'ordonee' to draw between), uchar bg
			(do we draw fg or bg sprites)
		Return: absolutely nothing

*****************************************************************************/
static inline void
draw_sprites(int Y1, int Y2, uchar bg)
{
	int n, i, j, x, y, no, atr, inc, cgx, cgy;
	int pos, y_sum, h, t;
	bool spr_bg, hflip;
	sprite_t *spr;

	spr = (sprite_t *) SPRAM + 63;

	for (n = 0; n < 64; n++, spr--) {
		atr = spr->atr;
		spr_bg = (atr >> 7) & 1;

		if (spr_bg != bg)
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
			if (SPR_CACHE[no + i] == 0) {
				spr2pixel(no + i);
			}
			if (!cgx)
				i++;
		}

		uchar *R = &Palette[256 + ((atr & 15) << 4)];
		uchar *C = VRAM + (no * 128);
		uint32 *C2 = OBJ_CACHE + (no * 32);

		hflip = (atr & H_FLIP);
		pos = XBUF_WIDTH * y + x;
		y_sum = 0;
		inc = 2;

		if (atr & V_FLIP) {
			inc = -2;
			C += 15 * 2 + cgy * 256;
			C2 += (15 * 2 + cgy * 64);
		}

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

			uchar pri = (sprite_usespbg || !spr_bg) ? n : 0xFF;

			for (j = 0; j <= cgx; j++) {
				draw_sprite(
					pos + (hflip ? cgx - j : j) * 16,
					(uint16*)(C + j * 128),
					C2 + j * 32,
					R,
					h,
					inc,
					pri,
					hflip,
					!spr_bg
				);
			}

			if (!spr_bg)
				sprite_usespbg = 1;

			pos += h * XBUF_WIDTH;
			C += h * inc + 16 * 7 * inc;
			C2 += h * inc + 16 * inc;
			y_sum += 16;
		}
	}
}


/*
	Hit Check Sprite#0 and others
*/
static inline bool
sprite_hit_check(void)
{
	int i, x0, y0, w0, h0, x, y, w, h;
	sprite_t *spr;

	spr = (sprite_t *) SPRAM;
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

//! Computes the new screen height and eventually change the screen mode
void
gfx_change_video_mode()
{
	uint16 temp_vds = IO_VDC_REG[VPR].B.h;
	uint16 temp_vsw = IO_VDC_REG[VPR].B.l;
	uint16 temp_vdw = IO_VDC_REG[VDW].W;
	uint16 temp_vcr = IO_VDC_REG[VCR].W;

	uint16 min_display = temp_vds + temp_vsw;
	uint16 max_display = min_display;
	uint16 cur_display = min_display;

	TRACE_GFX("GFX: Changing pce screen mode\n"
		" VDS = %04x VSW = %04x VDW = %04x VCR = %04x\n",
		 temp_vds, temp_vsw, temp_vdw, temp_vcr);

	if (temp_vdw == 0)
		return;

	while (cur_display < 242 + 14) {
		cur_display += temp_vdw;
		max_display = cur_display;
		cur_display += 3 + temp_vcr;

		TRACE_GFX("GFX: Adding vdw to the height of graphics,"
			" cur_display = %d\n", cur_display);
	}

	min_display = MAX(min_display, 14);
	max_display = MIN(max_display, 242 + 14);

	io.vdc_minline = min_display;
	io.vdc_maxline = max_display;

	//! Number of lines to render
	io.screen_h = max_display - min_display + 1;

	//TRACE_GFX("GFX: %d lines to render\n", io.screen_h);

	osd_gfx_set_mode(io.screen_w, io.screen_h);
}


int
gfx_init()
{
	gfx_need_redraw = 0;

	OBJ_CACHE = rg_alloc(0x10000, MEM_FAST);
    SPM_ptr   = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
	SPM	      = SPM_ptr + XBUF_WIDTH * 64 + 32;

	gfx_clear_cache();

	osd_gfx_init();

	// Build palette
	osd_gfx_set_color(255, 0x3f, 0x3f, 0x3f);

	for (uchar i = 0; i < 255; i++) {
		osd_gfx_set_color(i, (i & 0x1C) << 1, (i & 0xe0) >> 2,
			(i & 0x03) << 4);
	}

	return 0;
}


void
gfx_clear_cache()
{
	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));
	memset(&TILE_CACHE, 0, sizeof(TILE_CACHE));
}


void
gfx_term()
{
	if (OBJ_CACHE) {
		free(OBJ_CACHE);
		OBJ_CACHE = NULL;
	}

	if (SPM_ptr) {
		free(SPM_ptr);
		SPM_ptr = NULL;
	}

	osd_gfx_shutdown();
}


IRAM_ATTR void
gfx_save_context(char slot_number)
{
	gfx_context_t *context;

	// assert(slot_number < 4);

	if (slot_number == 0) {
		if (gfx_need_redraw == 1) { // Context is already saved + we haven't render the line using it
			MESSAGE_DEBUG("Cancelled context saving as a previous one wasn't consumed yet\n");
			return;
		} else
			gfx_need_redraw = 1;
	}

	context = saved_gfx_context + slot_number;

	context->scroll_x = ScrollX;
	context->scroll_y = ScrollY;
	context->scroll_y_diff = ScrollYDiff;
	context->control = Control;

	TRACE_GFX("Saving context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


static inline void
gfx_load_context(char slot_number)
{
	gfx_context_t *context;

	// assert(slot_number < 4);

	context = saved_gfx_context + slot_number;

	ScrollX = context->scroll_x;
	ScrollY = context->scroll_y;
	ScrollYDiff = context->scroll_y_diff;
	Control = context->control;

	TRACE_GFX("Restoring context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


//! render_lines
/*
	render lines into the buffer from min_line to max_line (inclusive)
*/
static inline void
render_lines(int min_line, int max_line)
{
	if (osd_skipFrames == 0) // Check for frameskip
	{
		gfx_save_context(1);
		gfx_load_context(0);

		// To do: enforce host.options.bgEnabled and fgEnabled

		// Temp hack
		if (max_line == 239) max_line = 240;

		sprite_usespbg = 0;

		if (SpriteON)
			draw_sprites(min_line, max_line, 0); // max_line + 1

		draw_tiles(min_line, max_line); // max_line + 1

		if (SpriteON)
			draw_sprites(min_line, max_line, 1);  // max_line + 1

		gfx_load_context(1);
	}

	gfx_need_redraw = 0;
}


//! gfx_loop
/*
	process one scanline
*/
IRAM_ATTR uchar
gfx_loop()
{
	uchar return_value = INT_NONE;

	io.vdc_status &= ~(VDC_RasHit | VDC_SATBfinish);

	// Count dma delay
	if (io.vdc_satb_counter > 0) {
		// A dma is in progress
		if (--io.vdc_satb_counter == 0) {
			// dma has just finished
			if (SATBIntON) {
				io.vdc_status |= VDC_SATBfinish;
				return_value = INT_IRQ;
			}
		}
	}

	// Test raster hit
	if (RasHitON) {
		if (((IO_VDC_REG[RCR].W & 0x3FF) >= 0x40)
			&& ((IO_VDC_REG[RCR].W & 0x3FF) <= 0x146)) {
			uint16 temp_rcr = (uint16) ((IO_VDC_REG[RCR].W & 0x3FF) - 0x40);

			if (Scanline == (temp_rcr + IO_VDC_REG[VPR].B.l + IO_VDC_REG[VPR].B.h) % 263) {
				// printf("\n---------------------\nRASTER HIT (%d)\n----------------------\n", Scanline);
				io.vdc_status |= VDC_RasHit;
				return_value = INT_IRQ;
			}
		} else {
			// printf("Raster counter out of bounds (%d)\n", IO_VDC_REG[RCR].W);
		}
	}
	//  else
	//		printf("Raster disabled\n");

	/* Visible area */
	if (Scanline >= 14 && Scanline <= 255) {
		if (Scanline == 14) {
			last_display_counter = 0;
			display_counter = 0;
			ScrollYDiff = 0;

			// Signal that we've left the VBlank area
			io.vdc_status &= ~VDC_InVBlank;

			TRACE_GFX("Cleaning VBlank bit from vdc_status (now, 0x%02x)",
				io.vdc_status);
		}

		if (Scanline == io.vdc_minline) {
			gfx_need_redraw = 0;

			gfx_save_context(0);

			TRACE_GFX("GFX: FORCED SAVE OF GFX CONTEXT\n");
		}

		if (Scanline >= io.vdc_minline && Scanline <= io.vdc_maxline) {
			if (gfx_need_redraw) {
				// && Scanline > io.vdc_minline)
				// We got render things before being on the second line

				render_lines(last_display_counter, display_counter);

				last_display_counter = display_counter;
			}
			display_counter++;
		}
	}
	/* V Blank trigger line */
	else if (Scanline == 256) {

		if (osd_keyboard())
			return INT_QUIT;

		if (io.vdc_mode_chg) {
			gfx_change_video_mode();
			io.vdc_mode_chg = 0;
		}

		if (sprite_hit_check())
			io.vdc_status |= VDC_SpHit;
		else
			io.vdc_status &= ~VDC_SpHit;

		gfx_save_context(0);

		render_lines(last_display_counter, display_counter);

		if (ROM_CRC == 0xD00CA74F && last_display_counter == 0 && display_counter >= 239) {
			 printf("Splatterhouse hack, skipping blit, scroll = (%d,%d,%d), CR = 0x%02d\n",
				 ScrollX, ScrollY, ScrollYDiff, Control);
		} else {
			osd_gfx_blit();
		}

#if defined(ENABLE_NETPLAY)
		if (option.want_netplay != INTERNET_PROTOCOL) {
			/* When in internet protocol mode, it's the server which is in charge of throlling */
			osd_wait_next_vsync();
		}
#else
		osd_wait_next_vsync();
#endif							/* NETPLAY_ENABLE */

		/* VRAM to SATB DMA */
		if (io.vdc_satb == 1 || IO_VDC_REG[DCR].W & 0x0010) {
			memcpy(SPRAM, VRAM + IO_VDC_REG[SATB].W * 2, 64 * 8);
			// io.vdc_satb = 1;
			io.vdc_satb = 0;
			io.vdc_status &= ~VDC_SATBfinish;

			// Mark satb dma end interuption to happen in 4 scanlines
			io.vdc_satb_counter = 4;
		}

		if (return_value == INT_IRQ)
			io.vdc_pendvsync = 1;
		else {
			if (VBlankON) {
				io.vdc_status |= VDC_InVBlank;
				return_value = INT_IRQ;
			}
		}
	}
	/* V Blank area */
	else {
		gfx_need_redraw = 0;
	}

	Scanline++;

	if (Scanline > 262)
		Scanline = 0;

	if ((return_value != INT_IRQ) && io.vdc_pendvsync) {
		io.vdc_status |= VDC_InVBlank;
		return_value = INT_IRQ;
		io.vdc_pendvsync = 0;
	}

	if (return_value == INT_IRQ) {
		if (!(io.irq_mask & IRQ1)) {
			io.irq_status |= IRQ1;
			return return_value;
		}
	}
	return INT_NONE;
}
