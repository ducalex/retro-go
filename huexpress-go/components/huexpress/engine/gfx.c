// gfx.c - VDC/VCE Emulation
//
#include "pce.h"

static gfx_context_t gfx_context;
static int line_counter = 0;
static int last_line_counter = 0;

// Active screen buffer
static uint8_t* screen_buffer;

// Cache for linear tiles and sprites. This is basically a decoded VRAM
static uint32_t *OBJ_CACHE;
bool TILE_CACHE[2048];
bool SPR_CACHE[512];

//
int ScrollYDiff;

#define PAL(nibble) (PAL[(L >> ((nibble) * 4)) & 15])


/*
	Convert a PCE coded sprite into a linear one
*/
static inline void
spr2pixel(int no)
{
	uint8_t *C = (uint8_t *)(VRAM + no * 64);
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


/*
	Convert a PCE coded tile into a linear one
*/
static inline void
tile2pixel(int no)
{
	uint8_t *C = (uint8_t *)(VRAM + no * 16);
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


/*
	Draw background tiles between two lines
*/
static void // Do not inline
draw_tiles(int Y1, int Y2, int scroll_x, int scroll_y)
{
	const uint8_t _bg_w[] = { 32, 64, 128, 128 };
	const uint8_t _bg_h[] = { 32, 64 };

	uint8_t bg_w = _bg_w[(IO_VDC_REG[MWR].W >> 4) & 3]; // Bits 5-4 select the width
    uint8_t bg_h = _bg_h[(IO_VDC_REG[MWR].W >> 6) & 1]; // Bit 6 selects the height

	int XW, no, x, y, h, offset;
	uint8_t *PP, *PAL, *P, *C;
	uint32_t *C2;

	if (Y1 == 0) {
		TRACE_GFX("\n=================================================\n");
	}

	TRACE_GFX("Rendering lines %3d - %3d\tScroll: (%3d,%3d)\n", Y1, Y2, scroll_x, scroll_y);

	y = Y1 + scroll_y;
	offset = y & 7;
	h = 8 - offset;
	if (h > Y2 - Y1)
		h = Y2 - Y1;
	y >>= 3;

	PP = (screen_buffer + XBUF_WIDTH * Y1) - (scroll_x & 7);
	XW = IO_VDC_SCREEN_WIDTH / 8 + 1;

	for (int Line = Y1; Line < Y2; y++) {
		x = scroll_x / 8;
		y &= bg_h - 1;
		for (int X1 = 0; X1 < XW; X1++, x++, PP += 8) {
			x &= bg_w - 1;

			no = VRAM[x + y * bg_w];

			PAL = &Palette[(no >> 8) & 0x1F0];

			// PCE has max of 2048 tiles
			no &= 0x7FF;

			if (TILE_CACHE[no] == 0) {
				tile2pixel(no);
			}

			C2 = OBJ_CACHE + (no * 8 + offset);
			C = (uint8_t*)(VRAM + no * 16 + offset);
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


/*
	Draw sprite C to framebuffer P
*/
static void // Do not inline (take advantage of xtensa's windowed registers)
draw_sprite(uint8_t *P, uint16_t *C, uint32_t *C2, int height, uint16_t attr)
{
	uint8_t *PAL = &Palette[256 + ((attr & 0xF) << 4)];

	bool hflip = attr & H_FLIP;
	int inc = (attr & V_FLIP) ? -1 : 1;
	int inc2 = inc * 2;

	for (int i = 0; i < height; i++, C += inc, C2 += inc2, P += XBUF_WIDTH) {

		uint16_t J = C[0] | C[16] | C[32] | C[48];
		uint32_t L;

		if (!J)
			continue;

		if (hflip) {
			L = C2[1];
			if ((J & 0x8000)) P[15] = PAL(1);
			if ((J & 0x4000)) P[14] = PAL(3);
			if ((J & 0x2000)) P[13] = PAL(5);
			if ((J & 0x1000)) P[12] = PAL(7);
			if ((J & 0x0800)) P[11] = PAL(0);
			if ((J & 0x0400)) P[10] = PAL(2);
			if ((J & 0x0200)) P[9]  = PAL(4);
			if ((J & 0x0100)) P[8]  = PAL(6);

			L = C2[0];
			if ((J & 0x80)) P[7] = PAL(1);
			if ((J & 0x40)) P[6] = PAL(3);
			if ((J & 0x20)) P[5] = PAL(5);
			if ((J & 0x10)) P[4] = PAL(7);
			if ((J & 0x08)) P[3] = PAL(0);
			if ((J & 0x04)) P[2] = PAL(2);
			if ((J & 0x02)) P[1] = PAL(4);
			if ((J & 0x01)) P[0] = PAL(6);
		}
		else {
			L = C2[1];
			if ((J & 0x8000)) P[0] = PAL(1);
			if ((J & 0x4000)) P[1] = PAL(3);
			if ((J & 0x2000)) P[2] = PAL(5);
			if ((J & 0x1000)) P[3] = PAL(7);
			if ((J & 0x0800)) P[4] = PAL(0);
			if ((J & 0x0400)) P[5] = PAL(2);
			if ((J & 0x0200)) P[6] = PAL(4);
			if ((J & 0x0100)) P[7] = PAL(6);

			L = C2[0];
			if ((J & 0x80)) P[8]  = PAL(1);
			if ((J & 0x40)) P[9]  = PAL(3);
			if ((J & 0x20)) P[10] = PAL(5);
			if ((J & 0x10)) P[11] = PAL(7);
			if ((J & 0x08)) P[12] = PAL(0);
			if ((J & 0x04)) P[13] = PAL(2);
			if ((J & 0x02)) P[14] = PAL(4);
			if ((J & 0x01)) P[15] = PAL(6);
		}
	}
}


/*
	Draw sprites between two lines
*/
static void // Do not inline
draw_sprites(int Y1, int Y2, int priority)
{
	// NOTE: At this time we do not respect bg sprites priority over top sprites.
	// Example: Assume that sprite #2 is priority=0 and sprite #5 is priority=1. If they
	// overlap then sprite #5 shouldn't be drawn because #2 > #5. But currently it will.

	// We iterate sprites in reverse order because earlier sprites have
	// higher priority and therefore must overwrite later sprites.

	for (int n = 63; n >= 0; n--) {
		sprite_t *spr = (sprite_t *)SPRAM + n;
		uint16_t attr = spr->attr;

		if (((attr >> 7) & 1) != priority)
			continue;

		int y = (spr->y & 0x3FF) - 64;
		int x = (spr->x & 0x3FF) - 32;
		int cgx = (attr >> 8) & 1;
		int cgy = (attr >> 12) & 3;
		int inc = (attr & V_FLIP) ? -1 : 1;
		int no = (spr->no & 0x7FF);

		TRACE_GFX("Sprite 0x%02X : X = %d, Y = %d, attr = %d, no = %d\n", n, x, y, attr, no);

		cgy |= cgy >> 1;
		no = (no >> 1) & ~(cgy * 2 + cgx);

		// PCE has max of 512 sprites
		no &= 0x1FF;

		if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= IO_VDC_SCREEN_WIDTH || x + (cgx + 1) * 16 < 0) {
			continue;
		}

		for (int i = 0; i < cgy * 2 + cgx + 1; i++) {
			if (SPR_CACHE[no + i] == 0) {
				spr2pixel(no + i);
			}
			if (!cgx)
				i++;
		}

		uint8_t *P = screen_buffer + (XBUF_WIDTH * y + x);
		uint16_t *C = VRAM + (no * 64);
		uint32_t *C2 = OBJ_CACHE + (no * 32);

		cgy *= 16;

		if (attr & V_FLIP) {
			C += 15 * 2 + cgy * 8;
			C2 += 15 * 2 + cgy * 4;
		}

		for (int yy = 0; yy <= cgy; yy += 16) {
			int t = Y1 - y - yy;
			int h = 16;

			if (t > 0) {
				C += t * inc;
				C2 += t * inc * 2;
				h -= t;
				P += t * XBUF_WIDTH;
			}

			if (h > Y2 - y - yy)
				h = Y2 - y - yy;

			if (attr & H_FLIP) {
				for (int j = 0; j <= cgx; j++) {
					draw_sprite(P + (cgx - j) * 16, C + j * 64, C2 + j * 32, h, attr);
				}
			} else {
				for (int j = 0; j <= cgx; j++) {
					draw_sprite(P + j * 16, C + j * 64, C2 + j * 32, h, attr);
				}
			}

			P += h * XBUF_WIDTH;
			C += (h + 16 * 7) * inc;
			C2 += (h + 16) * (inc * 2);
		}
	}
}


/*
	Hit Check Sprite#0 and others
*/
static inline bool
sprite_hit_check(void)
{
	sprite_t *spr = (sprite_t *)SPRAM;
	int x0 = spr->x;
	int y0 = spr->y;
	int w0 = (((spr->attr >> 8) & 1) + 1) * 16;
	int h0 = (((spr->attr >> 12) & 3) + 1) * 16;

	spr++;

	for (int i = 1; i < 64; i++, spr++) {
		int x = spr->x;
		int y = spr->y;
		int w = (((spr->attr >> 8) & 1) + 1) * 16;
		int h = (((spr->attr >> 12) & 3) + 1) * 16;
		if ((x < x0 + w0) && (x + w > x0) && (y < y0 + h0) && (y + h > y0))
			return 1;
	}
	return 0;
}


IRAM_ATTR void
gfx_latch_context(int force)
{
	if (!gfx_context.latched || force) { // Context is already saved + we haven't render the line using it
		gfx_context.scroll_x = IO_VDC_REG[BXR].W;
		gfx_context.scroll_y = IO_VDC_REG[BYR].W - ScrollYDiff;
		gfx_context.control = IO_VDC_REG[CR].W;
		gfx_context.latched = 1;
	}
}


/*
	Render lines into the buffer from min_line to max_line (inclusive)
*/
static inline void
render_lines(int min_line, int max_line)
{
	gfx_context.latched = 0;

	if (!screen_buffer) {
		return;
	}

	// Sprites with priority 0 are drawn behind the tiles
	if (gfx_context.control & 0x40) {
		draw_sprites(min_line, max_line, 0);
	}

	// Draw the background tiles
	if (gfx_context.control & 0x80) {
		draw_tiles(min_line, max_line, gfx_context.scroll_x, gfx_context.scroll_y);
	}

	// Draw regular sprites
	if (gfx_context.control & 0x40) {
		draw_sprites(min_line, max_line, 1);
	}
}


int
gfx_init(void)
{
	OBJ_CACHE = osd_alloc(0x10000);

	gfx_clear_cache();

	osd_gfx_init();

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

	osd_gfx_shutdown();
}


/*
	Raises a VDC IRQ and/or process pending VDC IRQs.
	More than one interrupt can happen in a single line on real hardware and the cpu
	would usually receive them one by one. We use an uint32 as a 8 slot buffer.
*/
void
gfx_irq(int type)
{
	/* If IRQ, push it on the stack */
	if (type >= 0) {
		io.vdc_irq_queue <<= 4;
		io.vdc_irq_queue |= type & 0xF;
	}

	/* Pop the first pending vdc interrupt only if io.irq_status is clear */
	int pos = 28;
	while (!(io.irq_status & INT_IRQ1) && io.vdc_irq_queue) {
		if (io.vdc_irq_queue >> pos) {
			io.vdc_status |= 1 << (io.vdc_irq_queue >> pos);
			io.vdc_irq_queue &= ~(0xF << pos);
			io.irq_status |= INT_IRQ1; // Notify the CPU
		}
		pos -= 4;
	}
}


/*
	Process one scanline
*/
void
gfx_run(void)
{
	screen_buffer = osd_gfx_framebuffer();

	/* DMA Transfer in "progress" */
	if (io.vdc_satb > DMA_TRANSFER_COUNTER) {
		if (--io.vdc_satb == DMA_TRANSFER_COUNTER) {
			if (SATBIntON) {
				gfx_irq(VDC_STAT_DS);
			}
		}
	}

	/* Test raster hit */
	if (RasHitON) {
		uint raster_hit = (IO_VDC_REG[RCR].W & 0x3FF) - 64;
		uint current_line = Scanline - IO_VDC_MINLINE + 1;
		if (current_line == raster_hit && raster_hit < 263) {
			TRACE_GFX("\n-----------------RASTER HIT (%d)------------------\n", Scanline);
			gfx_irq(VDC_STAT_RR);
		}
	}

	/* Visible area */
	if (Scanline >= 14 && Scanline <= 255) {
		if (Scanline == IO_VDC_MINLINE) {
			gfx_latch_context(1);
		}

		if (Scanline >= IO_VDC_MINLINE && Scanline <= IO_VDC_MAXLINE) {
			if (gfx_context.latched) {
				render_lines(last_line_counter, line_counter);
				last_line_counter = line_counter;
			}
			line_counter++;
		}
	}
	/* V Blank trigger line */
	else if (Scanline == 256) {

		// Draw any lines left in the context
		gfx_latch_context(0);
		render_lines(last_line_counter, line_counter);

		// Trigger interrupts
		if (SpHitON && sprite_hit_check()) {
			gfx_irq(VDC_STAT_CR);
		}
		if (VBlankON) {
			gfx_irq(VDC_STAT_VD);
		}

		/* VRAM to SATB DMA */
		if (io.vdc_satb == DMA_TRANSFER_PENDING || IO_VDC_REG[DCR].W & 0x0010) {
			memcpy(SPRAM, VRAM + IO_VDC_REG[SATB].W, 512);
			io.vdc_satb = DMA_TRANSFER_COUNTER + 4;
		}

		/* Frame done, we can now process pending res change. */
		if (io.vdc_mode_chg && IO_VDC_REG[VCR].W != 0) {
			TRACE_GFX("Changing mode: VDS = %04x VSW = %04x VDW = %04x VCR = %04x\n",
				IO_VDC_REG[VPR].B.h, IO_VDC_REG[VPR].B.l,
				IO_VDC_REG[VDW].W, IO_VDC_REG[VCR].W);
			io.vdc_mode_chg = 0;
			osd_gfx_set_mode(IO_VDC_SCREEN_WIDTH, IO_VDC_SCREEN_HEIGHT);
		}
	}
	/* V Blank area */
	else {
		gfx_context.latched = 0;
		last_line_counter = 0;
		line_counter = 0;
		ScrollYDiff = 0;
	}

	/* Always call at least once (to handle pending IRQs) */
	gfx_irq(-1);
}
