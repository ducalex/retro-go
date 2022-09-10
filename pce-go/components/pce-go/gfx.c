// gfx.c - VDC/VCE Emulation
//
#include <stdlib.h>
#include <string.h>
#include "pce.h"
#include "gfx.h"

#define PAL(nibble) (PAL[(L >> ((nibble) * 4)) & 15])

#define V_FLIP  0x8000
#define H_FLIP  0x0800

static int last_line_counter = 0;
static int line_counter = 0;

static struct {
	int scroll_x;
	int scroll_y;
	int control;
	int latched;
} gfx_context;

static uint8_t *framebuffer_top, *framebuffer_bottom;

/*
	Draw background tiles between two lines
*/
static void
draw_tiles(uint8_t *screen_buffer, int Y1, int Y2, int scroll_x, int scroll_y)
{
	TRACE_GFX("Rendering tiles on lines %3d - %3d\tScroll: (%3d,%3d)\n", Y1, Y2, scroll_x, scroll_y);

	uint32_t _bg_w[] = { 32, 64, 128, 128 };
	uint32_t _bg_h[] = { 32, 64 };

	uint32_t bg_w = _bg_w[(IO_VDC_REG[MWR].W >> 4) & 3]; // Bits 5-4 select the width
	uint32_t bg_h = _bg_h[(IO_VDC_REG[MWR].W >> 6) & 1]; // Bit 6 selects the height

	int num_tiles = IO_VDC_SCREEN_WIDTH / 8 + 1;
	int x;
	int y = Y1 + scroll_y;
	int offset = y & 7;
	int h = MIN(8 - offset, Y2 - Y1);

	y >>= 3;

	uint8_t *PP = (screen_buffer + XBUF_WIDTH * Y1) - (scroll_x & 7);

	for (int line = Y1; line < Y2; y++) {
		x = scroll_x / 8;
		y &= bg_h - 1;
		for (int n = 0; n < num_tiles; n++, x++, PP += 8) {
			x &= bg_w - 1;

			int no = PCE.VRAM[x + y * bg_w];

			uint8_t *PAL = &PCE.Palette[(no >> 8) & 0x1F0];
			uint8_t *C = (uint8_t*)(PCE.VRAM + (no & 0x7FF) * 16 + offset);
			uint8_t *P = PP;

			for (int i = 0; i < h; i++, P += XBUF_WIDTH, C += 2) {
				uint32_t J, L, M;

				J = C[0] | C[1] | C[16] | C[17];

				if (!J)
					continue;

				if (P + 8 >= framebuffer_bottom) {
					MESSAGE_DEBUG("tile overflow!\n");
					break;
				} else if (P < framebuffer_top) {
					MESSAGE_DEBUG("tile underflow!\n");
					continue;
				}

				M = C[0];
				L = ((M & 0x88) >> 3) | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
				M = C[1];
				L |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16) | ((M & 0x11) << 25);
				M = C[16];
				L |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
				M = C[17];
				L |= ((M & 0x88) >> 0) | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);

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
		line += h;
		PP += XBUF_WIDTH * h - num_tiles * 8;
		offset = 0;
		h = MIN(8, Y2 - line);
	}
}


/*
	Draw sprite C to framebuffer P
*/
static void
draw_sprite(uint8_t *P, const uint16_t *C, int height, uint32_t attr)
{
	uint8_t *PAL = &PCE.Palette[256 + ((attr & 0xF) << 4)];

	bool hflip = attr & H_FLIP;
	int inc = 1; //(attr & V_FLIP) ? -1 : 1;

	if (attr & V_FLIP) {
		inc = -1;
		C = C + height - 1;
	}

	for (int i = 0; i < height; i++, C += inc, P += XBUF_WIDTH) {

		uint32_t J = C[0] | C[16] | C[32] | C[48];
		uint32_t L1, L2, L, M;

		if (!J)
			continue;

		// This will also need to be handled in draw_sprites... (it could adjust simply constrain the height)
		if (P + 16 >= framebuffer_bottom) {
			MESSAGE_DEBUG("sprite overflow %d!\n", i);
			break;
		} else if (P < framebuffer_top) {
			MESSAGE_DEBUG("sprite underflow %d!\n", i);
			continue;
		}

		M = C[0];
		L1 = ((M & 0x88) >> 3) | ((M & 0x44) << 6) | ((M & 0x22) << 15) | ((M & 0x11) << 24);
		L2 = ((M & 0x8800) >> 11) | ((M & 0x4400) >> 2) | ((M & 0x2200) << 7) | ((M & 0x1100) << 16);
		M = C[16];
		L1 |= ((M & 0x88) >> 2) | ((M & 0x44) << 7) | ((M & 0x22) << 16) | ((M & 0x11) << 25);
		L2 |= ((M & 0x8800) >> 10) | ((M & 0x4400) >> 1) | ((M & 0x2200) << 8) | ((M & 0x1100) << 17);
		M = C[32];
		L1 |= ((M & 0x88) >> 1) | ((M & 0x44) << 8) | ((M & 0x22) << 17) | ((M & 0x11) << 26);
		L2 |= ((M & 0x8800) >> 9) | ((M & 0x4400) >> 0) | ((M & 0x2200) << 9) | ((M & 0x1100) << 18);
		M = C[48];
		L1 |= ((M & 0x88) >> 0) | ((M & 0x44) << 9) | ((M & 0x22) << 18) | ((M & 0x11) << 27);
		L2 |= ((M & 0x8800) >> 8) | ((M & 0x4400) << 1) | ((M & 0x2200) << 10) | ((M & 0x1100) << 19);

		if (hflip) {
			L = L2;
			if ((J & 0x8000)) P[15] = PAL(1);
			if ((J & 0x4000)) P[14] = PAL(3);
			if ((J & 0x2000)) P[13] = PAL(5);
			if ((J & 0x1000)) P[12] = PAL(7);
			if ((J & 0x0800)) P[11] = PAL(0);
			if ((J & 0x0400)) P[10] = PAL(2);
			if ((J & 0x0200)) P[9]  = PAL(4);
			if ((J & 0x0100)) P[8]  = PAL(6);

			L = L1;
			if ((J & 0x80)) P[7] = PAL(1);
			if ((J & 0x40)) P[6] = PAL(3);
			if ((J & 0x20)) P[5] = PAL(5);
			if ((J & 0x10)) P[4] = PAL(7);
			if ((J & 0x08)) P[3] = PAL(0);
			if ((J & 0x04)) P[2] = PAL(2);
			if ((J & 0x02)) P[1] = PAL(4);
			if ((J & 0x01)) P[0] = PAL(6);
		} else {
			L = L2;
			if ((J & 0x8000)) P[0] = PAL(1);
			if ((J & 0x4000)) P[1] = PAL(3);
			if ((J & 0x2000)) P[2] = PAL(5);
			if ((J & 0x1000)) P[3] = PAL(7);
			if ((J & 0x0800)) P[4] = PAL(0);
			if ((J & 0x0400)) P[5] = PAL(2);
			if ((J & 0x0200)) P[6] = PAL(4);
			if ((J & 0x0100)) P[7] = PAL(6);

			L = L1;
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
draw_sprites(uint8_t *screen_buffer, int Y1, int Y2, int priority)
{
	TRACE_GFX("Rendering sprites on lines %3d - %3d\tPriority: %d\n", Y1, Y2, priority);

	// NOTE: At this time we do not respect bg sprites priority over top sprites.
	// Example: Assume that sprite #2 is priority=0 and sprite #5 is priority=1. If they
	// overlap then sprite #5 shouldn't be drawn because #2 > #5. But currently it will.

	// We iterate sprites in reverse order because earlier sprites have
	// higher priority and therefore must overwrite later sprites.

	for (int n = 63; n >= 0; n--) {
		const sprite_t *spr = &PCE.SPRAM[n];
		uint32_t attr = spr->attr;

		if (((attr >> 7) & 1) != priority) {
			continue;
		}

		int y = (spr->y & 0x3FF) - 64;
		int x = (spr->x & 0x3FF) - 32;
		int cgx = (attr >> 8) & 1;
		int cgy = (attr >> 12) & 3;
		int no = (spr->no & 0x7FF);

		cgy |= cgy >> 1;

		no = (no >> 1) & ~(cgy * 2 + cgx);
		no &= 0x1FF; // PCE has max of 512 sprites

		TRACE_SPR("Sprite 0x%02X : X = %d, Y = %d, attr = %d, no = %d\n", n, x, y, attr, no);

		// Sprite is completely outside our window, skip it
		if (y >= Y2 || y + (cgy + 1) * 16 < Y1 || x >= IO_VDC_SCREEN_WIDTH || x + (cgx + 1) * 16 < 0) {
			continue;
		}

		cgy *= 16;

		uint8_t *P = screen_buffer + ((attr & V_FLIP ? cgy + y : y) * XBUF_WIDTH) + x;
		uint16_t *C = PCE.VRAM + (no * 64);

		for (int yy = 0; yy <= cgy; yy += 16) {
			int height = 16;
			if (attr & V_FLIP) {
				height = MIN(16, Y2 - y - (cgy - yy));
			} else {
				int t = Y1 - y - yy;
				if (t > 0) {
					P += t * XBUF_WIDTH;
					C += t;
					height -= t;
				}
				height = MIN(height, Y2 - y - yy);
			}

			if (height > 0) {
				for (int j = 0; j <= cgx; j++) {
					draw_sprite(P + (attr & H_FLIP ? cgx - j : j) * 16, C + j * 64, height, attr);
				}
			} else {
				MESSAGE_DEBUG("negative sprite height!\n");
			}

			P += ((attr & V_FLIP) ? -height : height) * XBUF_WIDTH;
			C += height + 16 * 7;
		}
	}
}


/*
	Hit Check Sprite#0 and others
*/
static inline bool
sprite_hit_check(void)
{
	const sprite_t *spr = &PCE.SPRAM[0];
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
		gfx_context.scroll_y = IO_VDC_REG[BYR].W - PCE.ScrollYDiff;
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

	uint8_t *screen_buffer = osd_gfx_framebuffer(PCE.VDC.screen_width, PCE.VDC.screen_height);
	if (!screen_buffer) {
		return;
	}

	// Assume 16 columns of scratch area around our buffer.
	framebuffer_top = screen_buffer - 16;
	framebuffer_bottom = screen_buffer + PCE.VDC.screen_height * XBUF_WIDTH;

	// We must fill the region with color 0 first.
	size_t screen_width = IO_VDC_SCREEN_WIDTH;
	for (int y = min_line; y <= max_line; y++) {
		memset(screen_buffer + (y * XBUF_WIDTH), PCE.Palette[0], screen_width);
	}

	// Sprites with priority 0 are drawn behind the tiles
	if (gfx_context.control & 0x40) {
		draw_sprites(screen_buffer, min_line, max_line, 0);
	}

	// Draw the background tiles
	if (gfx_context.control & 0x80) {
		draw_tiles(screen_buffer, min_line, max_line, gfx_context.scroll_x, gfx_context.scroll_y);
	}

	// Draw regular sprites
	if (gfx_context.control & 0x40) {
		draw_sprites(screen_buffer, min_line, max_line, 1);
	}
}


int
gfx_init(void)
{
	gfx_reset(true);
	return 0;
}


void
gfx_reset(bool hard)
{
	last_line_counter = 0;
	line_counter = 0;
}


void
gfx_term(void)
{
	//
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
		PCE.VDC.pending_irqs <<= 4;
		PCE.VDC.pending_irqs |= type & 0xF;
	}

	/* Pop the first pending vdc interrupt only if CPU.irq_lines is clear */
	int pos = 28;
	while (!(CPU.irq_lines & INT_IRQ1) && PCE.VDC.pending_irqs) {
		if (PCE.VDC.pending_irqs >> pos) {
			PCE.VDC.status |= 1 << (PCE.VDC.pending_irqs >> pos);
			PCE.VDC.pending_irqs &= ~(0xF << pos);
			CPU.irq_lines |= INT_IRQ1; // Notify the CPU
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
	int scanline = PCE.Scanline;

	/* DMA Transfer in "progress" */
	if (PCE.VDC.satb > DMA_TRANSFER_COUNTER) {
		if (--PCE.VDC.satb == DMA_TRANSFER_COUNTER) {
			if (SATBIntON) {
				gfx_irq(VDC_STAT_DS);
			}
		}
	}

	/* Test raster hit */
	if (RasHitON) {
		if (IO_VDC_REG[RCR].W >= 0x40 && (IO_VDC_REG[RCR].W <= 0x146)) {
			uint16_t temp_rcr = (uint16_t)(IO_VDC_REG[RCR].W - 0x40);
			if (scanline == (temp_rcr + IO_VDC_MINLINE) % 263) {
				TRACE_GFX("\n-----------------RASTER HIT (%d)------------------\n", scanline);
				gfx_irq(VDC_STAT_RR);
			}
		}
	}

	/* Visible area */
	if (scanline >= 14 && scanline <= 255) {
		if (scanline == IO_VDC_MINLINE) {
			gfx_latch_context(1);
		}

		if (scanline >= IO_VDC_MINLINE && scanline <= IO_VDC_MAXLINE) {
			if (gfx_context.latched) {
				render_lines(last_line_counter, line_counter);
				last_line_counter = line_counter;
			}
			line_counter++;
		}
	}
	/* V Blank trigger line */
	else if (scanline == 256) {

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
		if (PCE.VDC.satb == DMA_TRANSFER_PENDING || AutoSATBON) {
			memcpy(PCE.SPRAM, PCE.VRAM + IO_VDC_REG[SATB].W, 512);
			PCE.VDC.satb = DMA_TRANSFER_COUNTER + 4;
		}
	}
	/* V Blank area */
	else {
		gfx_context.latched = 0;
		last_line_counter = 0;
		line_counter = 0;
		PCE.ScrollYDiff = 0;
	}

	/* Always call at least once (to handle pending IRQs) */
	gfx_irq(-1);
}
