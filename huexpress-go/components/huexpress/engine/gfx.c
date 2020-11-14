// gfx.c - VDC General emulation
//
#include "pce.h"

static gfx_context_t saved_gfx_context[2];
static bool gfx_need_redraw = 0;
static bool sprite_usespbg = 0;
static int display_counter = 0;
static int last_display_counter = 0;

// Sprite priority mask memory
static uint8_t *SPM, *SPM_ptr;

// Cache for linear tiles and sprites. This is basically a decoded VRAM
static uint32_t *OBJ_CACHE;
bool TILE_CACHE[2048];
bool SPR_CACHE[512];

//
int ScrollYDiff;

#define PAL(nibble) (PAL[(L >> ((nibble) * 4)) & 15]);


/*****************************************************************************

		Function: spr2pixel

		Description: convert a PCE coded sprite into a linear one
		Parameters: int no,the number of the sprite to convert
		Return: nothing

*****************************************************************************/
FAST_INLINE void
spr2pixel(int no)
{
	uint8_t *C = VRAM + no * 128;
	uint32_t *C2 = OBJ_CACHE + no * 32;
	// 2 longs -> 16 nibbles => 32 loops for a 16*16 spr

	TRACE_SPR("Planing sprite %d\n", no);
	for (int i = 0; i < 32; i++, C++, C2++) {
		uint32_t M, L;
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
FAST_INLINE void
tile2pixel(int no)
{
	uint8_t *C = VRAM + no * 32;
	uint32_t *C2 = OBJ_CACHE + no * 8;

	TRACE_SPR("Planing tile %d\n", no);
	for (int i = 0; i < 8; i++, C += 2, C2++) {
		uint32_t M, L;
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
					uint16_t *C (sprite to draw)
					uint32_t *C2 (cached linear sprite)
					uint8_t *PAL (address of the palette of this sprite)
					int h (the number of line to draw)
					int inc (the value to increment the sprite buffer)
					int pr (priority to compare against M)
					bool hflip (flip tile horizontally)
					bool update_mask (update priority mask)
		Return: nothing

*****************************************************************************/
FAST_INLINE void
draw_sprite(int offset, uint16_t *C, uint32_t *C2, uint8_t *PAL, int h, int inc,
	uint8_t pr, bool hflip, bool update_mask)
{
	uint8_t *P = osd_gfx_buffer + offset;
	uint8_t *M = SPM + offset;

	for (int i = 0; i < h; i++, C = (uint16_t*)((uint8_t*)C + inc),
			C2 += inc, P += XBUF_WIDTH, M += XBUF_WIDTH) {

		uint16_t J = C[0] | C[16] | C[32] | C[48];
		uint32_t L;

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
FAST_INLINE void
draw_tiles(int Y1, int Y2)
{
	int XW, no, x, y, h, offset;
	uint8_t *PP, *PAL, *P, *C;
	uint32_t *C2;

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

	for (int Line = Y1; Line < Y2; y++) {
		x = ScrollX / 8;
		y &= io.bg_h - 1;
		for (int X1 = 0; X1 < XW; X1++, x++, PP += 8) {
			x &= io.bg_w - 1;

			no = ((uint16_t*)VRAM)[x + y * io.bg_w];

			PAL = &Palette[(no >> 8) & 0x1F0];

			// PCE has max of 2048 tiles
			no &= 0x7FF;

			if (TILE_CACHE[no] == 0) {
				tile2pixel(no);
			}

			C2 = OBJ_CACHE + (no * 8 + offset);
			C = VRAM + (no * 32 + offset * 2);
			P = PP;
			for (int i = 0; i < h; i++, P += XBUF_WIDTH, C2++, C += 2) {
				uint32_t J, L;

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
		Parameters: int Y1, int Y2 (the 'ordonee' to draw between), bool bg
			(do we draw fg or bg sprites)
		Return: absolutely nothing

*****************************************************************************/
FAST_INLINE void
draw_sprites(int Y1, int Y2, bool bg)
{
	sprite_t *spr = (sprite_t *)SPRAM + 63;

	for (int n = 0; n < 64; n++, spr--) {
		int spr_bg = (spr->atr >> 7) & 1;

		if (spr_bg != bg)
			continue;

		int y = (spr->y & 1023) - 64;
		int x = (spr->x & 1023) - 32;
		int cgx = (spr->atr >> 8) & 1;
		int cgy = (spr->atr >> 12) & 3;
		int no = (spr->no & 2047); // 4095 for supergraphx
		int pos = XBUF_WIDTH * y + x;
		int inc = 2;

		TRACE_GFX("Sprite 0x%02X : X = %d, Y = %d, atr = 0x%04X, no = 0x%03X\n",
					n, x, y, spr->atr, (unsigned int)no);

		cgy |= cgy >> 1;
		no = (no >> 1) & ~(cgy * 2 + cgx);

		// PCE has max of 512 sprites
		no &= 0x1FF;

		if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= io.screen_w || x + (cgx + 1) * 16 < 0) {
			continue;
		}

		for (int i = 0; i < cgy * 2 + cgx + 1; i++) {
			if (SPR_CACHE[no + i] == 0) {
				spr2pixel(no + i);
			}
			if (!cgx)
				i++;
		}

		uint8_t *PAL = &Palette[256 + ((spr->atr & 15) << 4)];
		uint8_t *C = VRAM + (no * 128);
		uint32_t *C2 = OBJ_CACHE + (no * 32);

		if (spr->atr & V_FLIP) {
			inc = -2;
			C += 15 * 2 + cgy * 256;
			C2 += (15 * 2 + cgy * 64);
		}

		for (int i = 0, y_sum = 0; i <= cgy; i++, y_sum += 16) {
			int h = 16;
			int t = Y1 - y - y_sum;
			int pri = (sprite_usespbg || !spr_bg) ? n : 0xFF;
			int hflip = (spr->atr & H_FLIP) ? 1 : 0;

			if (t > 0) {
				C += t * inc;
				C2 += (t * inc);
				h -= t;
				pos += t * XBUF_WIDTH;
			}

			if (h > Y2 - y - y_sum)
				h = Y2 - y - y_sum;

			for (int j = 0; j <= cgx; j++) {
				draw_sprite(
					pos + (hflip ? cgx - j : j) * 16,
					(uint16_t*)(C + j * 128),
					C2 + j * 32,
					PAL,
					h,
					inc,
					pri,
					hflip,
					!spr_bg
				);
			}

			if (!bg)
				sprite_usespbg = 1;

			pos += h * XBUF_WIDTH;
			C += h * inc + 16 * 7 * inc;
			C2 += h * inc + 16 * inc;
		}
	}
}


/*
	Hit Check Sprite#0 and others
*/
FAST_INLINE bool
sprite_hit_check(void)
{
	sprite_t *spr = (sprite_t *)SPRAM;
	int x0 = spr->x;
	int y0 = spr->y;
	int w0 = (((spr->atr >> 8) & 1) + 1) * 16;
	int h0 = (((spr->atr >> 12) & 3) + 1) * 16;

	spr++;

	for (int i = 1; i < 64; i++, spr++) {
		int x = spr->x;
		int y = spr->y;
		int w = (((spr->atr >> 8) & 1) + 1) * 16;
		int h = (((spr->atr >> 12) & 3) + 1) * 16;
		if ((x < x0 + w0) && (x + w > x0) && (y < y0 + h0) && (y + h > y0))
			return 1;
	}
	return 0;
}


/*
	Computes the new screen height and eventually change the screen mode
*/
FAST_INLINE void
change_video_mode(void)
{
	uint16_t temp_vds = IO_VDC_REG[VPR].B.h;
	uint16_t temp_vsw = IO_VDC_REG[VPR].B.l;
	uint16_t temp_vdw = IO_VDC_REG[VDW].W;
	uint16_t temp_vcr = IO_VDC_REG[VCR].W;

	TRACE_GFX("GFX: Changing pce screen mode\n"
		" VDS = %04x VSW = %04x VDW = %04x VCR = %04x\n",
		 temp_vds, temp_vsw, temp_vdw, temp_vcr);

	if (temp_vdw == 0)
		return;

	io.vdc_minline = ((temp_vsw & 0x1F)+1+temp_vds+2); //temp_vds + temp_vsw;;
	io.vdc_maxline = io.vdc_minline + temp_vdw; // + 1
	io.screen_h = io.vdc_maxline - io.vdc_minline + 1;

	//TRACE_GFX("GFX: %d lines to render\n", io.screen_h);

	osd_gfx_set_mode(io.screen_w, io.screen_h);
}


IRAM_ATTR void
gfx_save_context(int slot_number)
{
	gfx_context_t *context = &saved_gfx_context[slot_number];

	if (slot_number == 0) {
		if (gfx_need_redraw == 1) { // Context is already saved + we haven't render the line using it
			// MESSAGE_DEBUG("Cancelled context saving as a previous one wasn't consumed yet\n");
			return;
		}
		gfx_need_redraw = 1;
	}

	context->scroll_x = ScrollX;
	context->scroll_y = ScrollY;
	context->scroll_y_diff = ScrollYDiff;
	context->control = Control;

	TRACE_GFX("Saving context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


FAST_INLINE void
gfx_load_context(int slot_number)
{
	gfx_context_t *context = &saved_gfx_context[slot_number];

	ScrollX = context->scroll_x;
	ScrollY = context->scroll_y;
	ScrollYDiff = context->scroll_y_diff;
	Control = context->control;

	TRACE_GFX("Restoring context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


/*
	render lines into the buffer from min_line to max_line (inclusive)
*/
FAST_INLINE void
render_lines(int min_line, int max_line)
{
	if (osd_skipFrames == 0) // Check for frameskip
	{
		gfx_save_context(1);
		gfx_load_context(0);

		sprite_usespbg = 0;

		if (SpriteON)
			draw_sprites(min_line, max_line, false); // max_line + 1

		draw_tiles(min_line, max_line); // max_line + 1

		if (SpriteON)
			draw_sprites(min_line, max_line, true);  // max_line + 1

		gfx_load_context(1);
	}

	gfx_need_redraw = 0;
}


int
gfx_init(void)
{
	gfx_need_redraw = 0;

	OBJ_CACHE = rg_alloc(0x10000, MEM_FAST);
    SPM_ptr   = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
	SPM	      = SPM_ptr + XBUF_WIDTH * 64 + 32;

	gfx_clear_cache();

	osd_gfx_init();

	// Build palette
	for (int i = 0; i < 255; i++) {
		osd_gfx_set_color(i, (i & 0x1C) << 1, (i & 0xe0) >> 2, (i & 0x03) << 4);
	}
	osd_gfx_set_color(255, 0x3f, 0x3f, 0x3f);

	return 0;
}


void
gfx_clear_cache(void)
{
	memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));
	memset(&TILE_CACHE, 0, sizeof(TILE_CACHE));
}


void
gfx_term(void)
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


/*
	process one scanline
*/
IRAM_ATTR void
gfx_run(void)
{
	// Count dma delay
	if (io.vdc_satb_counter > 0) {
		// A dma is in progress
		if (--io.vdc_satb_counter == 0) {
			// dma has just finished
			if (SATBIntON) {
				io.vdc_status |= VDC_STAT_DS;
				io.irq_status |= INT_IRQ1;
			}
		}
	}

	// Test raster hit
	if (RasHitON) {
		uint raster_hit = (IO_VDC_REG[RCR].W & 0x3FF) - 64;
		uint current_line = Scanline - io.vdc_minline + 1;
		if (current_line == raster_hit && raster_hit < 263) {
			TRACE_GFX("\n-----------------RASTER HIT (%d)------------------\n", Scanline);
			io.vdc_status |= VDC_STAT_RR;
			io.irq_status |= INT_IRQ1;
		}
	}

	/* Visible area */
	if (Scanline >= 14 && Scanline <= 255) {
		if (Scanline == 14) {
			last_display_counter = 0;
			display_counter = 0;
			ScrollYDiff = 0;
		}

		if (Scanline == io.vdc_minline) {
			gfx_need_redraw = 0;
			gfx_save_context(0);
			// MESSAGE_DEBUG("GFX: FORCED SAVE OF GFX CONTEXT\n");
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
		if (io.vdc_mode_chg) {
			change_video_mode();
			io.vdc_mode_chg = 0;
		}

		if (SpHitON && sprite_hit_check()) {
			io.vdc_status |= VDC_STAT_CR;
			io.irq_status |= INT_IRQ1;
		}

		gfx_save_context(0);

		render_lines(last_display_counter, display_counter);

		/* VRAM to SATB DMA */
		if (io.vdc_satb == 1 || IO_VDC_REG[DCR].W & 0x0010) {
			memcpy(SPRAM, VRAM + IO_VDC_REG[SATB].W * 2, 512);
			io.vdc_satb = 0;
			io.vdc_satb_counter = 4; // Transfer ends in 4 scanlines
		}

		if (VBlankON) {
			io.vdc_status |= VDC_STAT_VD;
			io.irq_status |= INT_IRQ1;
		}
	}
	/* V Blank area */
	else {
		gfx_need_redraw = 0;
	}
}
