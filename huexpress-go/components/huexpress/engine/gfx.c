// gfx.c - VDC General emulation
//
#include "pce.h"
#include "utils.h"
#include "config.h"

#undef TRACE
#if ENABLE_TRACING_GFX
#define TRACE(x...) printf("TraceGfx: " x)
#else
#define TRACE(x...)
#endif

static gfx_context saved_gfx_context[4];

//! Whether we need to draw pending lines
static bool gfx_need_redraw = 0;

//! Frame to skip before the next frame to render
int UCount = 0;

// Frameskip setting
int UPeriod = 0;

static int display_counter = 0;
static int last_display_counter = 0;


//! Computes the new screen height and eventually change the screen mode
void
gfx_change_video_mode()
{
	//! minimal theorical line where to begin drawing
	int min_display;

	//! maximal theorical line where to end drawing
	int max_display;

	int cur_display;

	int temp_vds = IO_VDC_REG[VPR].B.h;
	int temp_vsw = IO_VDC_REG[VPR].B.l;
	int temp_vdw = (int) IO_VDC_REG[VDW].W;
	int temp_vcr = (int) IO_VDC_REG[VCR].W;

#if ENABLE_TRACING_GFX
	TRACE("GFX: Changing pce screen mode\n"
		" VDS = %04x VSW = %04x VDW = %04x VCR = %04x\n",
		 temp_vds, temp_vsw, temp_vdw, temp_vcr);
	// getchar();
#endif

	if (temp_vdw == 0)
		return;

	min_display = temp_vds + temp_vsw;

	max_display = cur_display = min_display;
	while (cur_display < 242 + 14) {
		cur_display += temp_vdw;
		max_display = cur_display;
		cur_display += 3 + temp_vcr;

#if ENABLE_TRACING_GFX
		TRACE("GFX: Adding vdw to the height of graphics,"
			" cur_display = %d\n", cur_display);
#endif
	}

	min_display = (min_display > 14 ? min_display : 14);
	max_display = (max_display < 242 + 14 ? max_display : 242 + 14);

	io.vdc_minline = (uint16) min_display;
	io.vdc_maxline = (uint16) max_display;

	//! Number of lines to render
	io.screen_h = max_display - min_display + 1;

#if ENABLE_TRACING_GFX
	//TRACE("GFX: %d lines to render\n", io.screen_h);
#endif

	osd_gfx_set_mode(io.screen_w, io.screen_h);
}


int
gfx_init()
{
    gfx_need_redraw = 0;
    UCount = 0;

	// Sprite memory
	VRAMS = VRAMS ?: (uchar *)rg_alloc(VRAMSIZE, MEM_FAST);
	VRAM2 = VRAM2 ?: (uchar *)rg_alloc(VRAMSIZE, MEM_FAST);
    memset(&SPR_CACHE, 0, sizeof(SPR_CACHE));

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
gfx_term()
{
	if (VRAMS) free(VRAMS);
	if (VRAM2) free(VRAM2);

	VRAMS = VRAM2 = NULL;

	osd_gfx_shutdown();
}


IRAM_ATTR void
gfx_save_context(char slot_number)
{
	gfx_context *context;

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

	TRACE("Saving context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


IRAM_ATTR void
gfx_load_context(char slot_number)
{
	gfx_context *context;

	// assert(slot_number < 4);

	context = saved_gfx_context + slot_number;

	ScrollX = context->scroll_x;
	ScrollY = context->scroll_y;
	ScrollYDiff = context->scroll_y_diff;
	Control = context->control;

	TRACE("Restoring context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, Control);
}


//! Render lines
/*
	render lines into the buffer from min_line to max_line (inclusive)
*/
static inline void
render_lines(int min_line, int max_line)
{
	if (osd_skipFrames == 0 && UCount == 0) // Check for frameskip
	{
		gfx_save_context(1);
		gfx_load_context(0);

		// Temp hack
		if (max_line == 239) max_line = 240;

		if (SpriteON && SPONSwitch)
		{
			RefreshSpriteExact(min_line, max_line, 0); // max_line + 1
			RefreshLine(min_line, max_line); // max_line + 1
			RefreshSpriteExact(min_line, max_line, 1);  // max_line + 1
		}
		else
			RefreshLine(min_line, max_line); // max_line + 1

		gfx_load_context(1);
	}

	gfx_need_redraw = 0;
}


//! Rewritten version of Loop6502 from scratch, called when each line drawing should occur
/* TODO:
	 - sprite #0 collision checking (occur as soon as the sprite #0 is shown and overlap another sprite
	 - frame skipping to test
*/
IRAM_ATTR char
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
	//      printf("Raster disabled\n");

	// Rendering of tiles / sprites

	if (Scanline < 14) {
		gfx_need_redraw = 0;
	} else if (Scanline < 14 + 242) {
		if (Scanline == 14) {
			last_display_counter = 0;
			display_counter = 0;
			ScrollYDiff = 0;

			// Signal that we've left the VBlank area
			io.vdc_status &= ~VDC_InVBlank;

			TRACE("Cleaning VBlank bit from vdc_status (now, 0x%02x)",
				io.vdc_status);
		}

		if (Scanline == io.vdc_minline) {
			gfx_need_redraw = 0;

			gfx_save_context(0);

			TRACE("GFX: FORCED SAVE OF GFX CONTEXT\n");
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
	} else if (Scanline < 14 + 242 + 4) {
		if (Scanline == 14 + 242) {
			gfx_save_context(0);

			render_lines(last_display_counter, display_counter);

			if (io.vdc_mode_chg) {
				gfx_change_video_mode();
				io.vdc_mode_chg = 0;
			}

			if (osd_keyboard())
				return INT_QUIT;

			if (!UCount)
				RefreshScreen();

			if (CheckSprites())
				io.vdc_status |= VDC_SpHit;
			else
				io.vdc_status &= ~VDC_SpHit;

			if (!UCount) {
#if defined(ENABLE_NETPLAY)
				if (option.want_netplay != INTERNET_PROTOCOL) {
					/* When in internet protocol mode, it's the server which is in charge of throlling */
					osd_wait_next_vsync();
				}
#else
				osd_wait_next_vsync();
#endif							/* NETPLAY_ENABLE */
				UCount = UPeriod;
			} else {
				UCount--;
			}

			/* VRAM to SATB DMA */
			if (io.vdc_satb == 1 || IO_VDC_REG[DCR].W & 0x0010) {
#if defined(WORDS_BIGENDIAN)
				swab(VRAM + IO_VDC_REG[SATB].W * 2, SPRAM, 64 * 8);
#else
				memcpy(SPRAM, VRAM + IO_VDC_REG[SATB].W * 2, 64 * 8);
#endif
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

	} else {
		//Three last lines of ntsc scanlining
	}

	// Incrementing the scanline

	Scanline++;

	if (Scanline >= 263)
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
