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
 */

/********************************************************************/
/*					 Generic Graphic source file					*/
/*																	*/
/*			 Adapted by Zeograd (Olivier Jolly) for using Allegro	*/
/*																	*/
/********************************************************************/


#include "pce.h"
#include "utils.h"
#include "config.h"

#undef TRACE
#if ENABLE_TRACING_GFX
#define TRACE(x...) printf("TraceGfx: " x)
#else
#define TRACE(x...)
#endif

extern int skipFrames;

typedef struct {
	uchar r, g, b;
} rgb_map_struct;

rgb_map_struct rgb_map[256];

void
SetPalette(void)
{
	uchar i;

	osd_gfx_set_color(255, 0x3f, 0x3f, 0x3f);
	rgb_map[255].r = 255;
	rgb_map[255].b = 255;
	rgb_map[255].g = 255;

	for (i = 0; i < 255; i++) {
		osd_gfx_set_color(i, (i & 0x1C) << 1, (i & 0xe0) >> 2,
			(i & 0x03) << 4);
		rgb_map[i].r = (i & 0x1C) << 3;
		rgb_map[i].g = (i & 0xe0);
		rgb_map[i].b = (i & 0x03) << 6;
	}
}


/*!
 * calc_fullscreen_aspect:
 * Generic utility that takes the width x height of the current output screen
 * and sets up a gfx lib independent struct generic_rect (gfx.h) with the
 * aspect-correct scaling values, provided option.want_fullscreen_aspect
 * has been set in the config or via the command line.
 */
void
calc_fullscreen_aspect(unsigned short physical_screen_width,
	unsigned short physical_screen_height, struct generic_rect *rect,
	unsigned short pce_screen_width, unsigned short pce_screen_height)
{
	/*
	 * Routine not called often so extra sanity checks pose no penalty.
	 */
	if (physical_screen_height == 0) {
		printf
			("calc_fullscreen_aspect: physical_screen_height is 0!	Aborting . . .\n");
		exit(0);
	}

	if (pce_screen_width == 0) {
		printf
			("calc_fullscreen_aspect: pce_screen_width is 0!	Aborting . . .\n");
		exit(0);
	}

	if (host.want_fullscreen_aspect) {
		float physical_screen_ratio, pce_ratio;
		int new_size;

		physical_screen_ratio
			= (float) physical_screen_width / physical_screen_height;

		pce_ratio
			= (pce_screen_width / physical_screen_ratio) / pce_screen_height;

		if (pce_ratio < 1.0) {
			new_size = (int) (physical_screen_width * pce_ratio);

			(*rect).start_x
				= (unsigned short int)((physical_screen_width - new_size) / 2);
			(*rect).start_y = 0;
			(*rect).end_x = (unsigned short int) new_size;
			(*rect).end_y = physical_screen_height;
		} else {
			new_size = (int) (physical_screen_height / pce_ratio);

			(*rect).start_x = 0;
			(*rect).start_y
				= (unsigned short int)((physical_screen_height - new_size) / 2);
			(*rect).end_x = physical_screen_width;
			(*rect).end_y = (unsigned short int) new_size;
		}
	} else {
		(*rect).start_x = (*rect).start_y = 0;
		(*rect).end_x = physical_screen_width;
		(*rect).end_y = physical_screen_height;
	}
}

//! Computes the new screen height and eventually change the screen mode
void
change_pce_screen_height()
{
	//! minimal theorical line where to begin drawing
	int min_display;

	//! maximal theorical line where to end drawing
	int max_display;

	int cur_display;

	int temp_vds = IO_VDC_0C_VPR.B.h;
	int temp_vsw = IO_VDC_0C_VPR.B.l;
	int temp_vdw = (int) IO_VDC_0D_VDW.W;
	int temp_vcr = (int) IO_VDC_0E_VCR.W;

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

	io.vdc_min_display = (uint16) min_display;
	io.vdc_max_display = (uint16) max_display;

	//! Number of lines to render
	io.screen_h = max_display - min_display + 1;

#if ENABLE_TRACING_GFX
	//TRACE("GFX: %d lines to render\n", io.screen_h);
#endif

	(*osd_gfx_driver_list[host.video_driver].mode) ();
}

gfx_context saved_gfx_context[MAX_GFX_CONTEXT_SLOT_NUMBER];

//! Whether we need to draw pending lines
int gfx_need_redraw;

//! Frame to skip before the next frame to render
int UCount = 0;

//! Whether we should change video mode after drawing the current frame
int gfx_need_video_mode_change = 0;

void gfx_init()
{
    UCount = 0;
    gfx_need_video_mode_change = 0;
    gfx_need_redraw = 0;
}

inline void
save_gfx_context(int slot_number)
{
	gfx_context *destination_context;

	destination_context = saved_gfx_context + slot_number;

	if (slot_number == 0) {
		if (gfx_need_redraw == 0)
			gfx_need_redraw = 1;
		else {					// Context is already saved + we haven't render the line using it
			TRACE("Canceled context saving as a previous one wasn't "
				"consumed yet\n");
			return;
		}
	}
	if (slot_number >= MAX_GFX_CONTEXT_SLOT_NUMBER) {
		TRACE("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__,
			__FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER);
		Log("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__,
			__FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER);
		return;
	}

	TRACE("Saving context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, IO_VDC_05_CR.W);

	destination_context->scroll_x = ScrollX;
	destination_context->scroll_y = ScrollY;
	destination_context->scroll_y_diff = ScrollYDiff;
	destination_context->cr = IO_VDC_05_CR.W;
}


static inline void
load_gfx_context(int slot_number)
{
	gfx_context *source_context;

	if (slot_number >= MAX_GFX_CONTEXT_SLOT_NUMBER) {
		Log("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__,
			__FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER);
		return;
	}

	source_context = saved_gfx_context + slot_number;

	ScrollX = source_context->scroll_x;
	ScrollY = source_context->scroll_y;
	ScrollYDiff = source_context->scroll_y_diff;
	IO_VDC_05_CR.W = source_context->cr;

	TRACE("Restoring context %d, scroll = (%d,%d,%d), CR = 0x%02d\n",
		slot_number, ScrollX, ScrollY, ScrollYDiff, IO_VDC_05_CR.W);
}

//! render lines
/*
	render lines into the buffer from min_line to max_line, inclusive
				Refresh* draw things from min to max line given, with max exclusive
*/
static inline void
render_lines(int min_line, int max_line)
{
	if (skipFrames == 0 && UCount == 0)  // Either we're in frameskip = 0 or we're in the frame to draw
	{
		save_gfx_context(1);
		load_gfx_context(0);

		if (SpriteON && SPONSwitch)
		{
			RefreshSpriteExact(min_line, max_line - 1, 0);
			RefreshLine(min_line, max_line - 1);
			RefreshSpriteExact(min_line, max_line - 1, 1);
		}
		else
			RefreshLine(min_line, max_line - 1);

		load_gfx_context(1);
	}

	gfx_need_redraw = 0;
}


//! Rewritten version of Loop6502 from scratch, called when each line drawing should occur
/* TODO:
	 - sprite #0 collision checking (occur as soon as the sprite #0 is shown and overlap another sprite
	 - frame skipping to test
*/
inline uchar
Loop6502()
{
	static int display_counter = 0;
	static int last_display_counter = 0;
	static int satb_dma_counter = 0;
	uchar return_value = INT_NONE;

	io.vdc_status &= ~(VDC_RasHit | VDC_SATBfinish);

	// Count dma delay

	if (satb_dma_counter > 0) {
		// A dma is in progress
		satb_dma_counter--;

		if (!satb_dma_counter) {
			// dma has just finished
			if (SATBIntON) {
				io.vdc_status |= VDC_SATBfinish;
				return_value = INT_IRQ;
			}
		}
	}
	// Test raster hit
	if (RasHitON) {
		if (((IO_VDC_06_RCR.W & 0x3FF) >= 0x40)
			&& ((IO_VDC_06_RCR.W & 0x3FF) <= 0x146)) {
			uint16 temp_rcr = (uint16) ((IO_VDC_06_RCR.W & 0x3FF) - 0x40);

			if (scanline
				== (temp_rcr + IO_VDC_0C_VPR.B.l + IO_VDC_0C_VPR.B.h) % 263) {
				// printf("\n---------------------\nRASTER HIT (%d)\n----------------------\n", scanline);
				io.vdc_status |= VDC_RasHit;
				return_value = INT_IRQ;
			}
		} else {
			// printf("Raster counter out of bounds (%d)\n", IO_VDC_06_RCR.W);
		}
	}
	//  else
	//      printf("Raster disabled\n");

	// Rendering of tiles / sprites

	if (scanline < 14) {
		gfx_need_redraw = 0;
	} else if (scanline < 14 + 242) {
		if (scanline == 14) {
			last_display_counter = 0;
			display_counter = 0;
			ScrollYDiff = 0;
			oldScrollYDiff = 0;

			// Signal that we've left the VBlank area
			io.vdc_status &= ~VDC_InVBlank;

			TRACE("Cleaning VBlank bit from vdc_status (now, 0x%02x)",
				io.vdc_status);
		}

		if (scanline == io.vdc_min_display) {
			gfx_need_redraw = 0;

			save_gfx_context(0);

			TRACE("GFX: FORCED SAVE OF GFX CONTEXT\n");
		}

		if ((scanline >= io.vdc_min_display)
			&& (scanline <= io.vdc_max_display)) {
			if (gfx_need_redraw) {
				// && scanline > io.vdc_min_display)
				// We got render things before being on the second line

				render_lines(last_display_counter, display_counter);

				last_display_counter = display_counter;
			}
			display_counter++;
		}
	} else if (scanline < 14 + 242 + 4) {
		if (scanline == 14 + 242) {
			save_gfx_context(0);

			render_lines(last_display_counter, display_counter);

			if (gfx_need_video_mode_change) {
				gfx_need_video_mode_change = 0;
				change_pce_screen_height();
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
					wait_next_vsync();
				}
#else
				wait_next_vsync();
#endif							/* NETPLAY_ENABLE */
				UCount = UPeriod;
			} else {
				UCount--;
			}

			/* VRAM to SATB DMA */
			if (io.vdc_satb == 1 || IO_VDC_0F_DCR.W & 0x0010) {
#if defined(WORDS_BIGENDIAN)
				swab(VRAM + IO_VDC_13_SATB.W * 2, SPRAM, 64 * 8);
#else
				memcpy(SPRAM, VRAM + IO_VDC_13_SATB.W * 2, 64 * 8);
#endif
				io.vdc_satb = 1;
				io.vdc_status &= ~VDC_SATBfinish;

				// Mark satb dma end interuption to happen in 4 scanlines
				satb_dma_counter = 4;
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

	scanline++;

	if (scanline >= 263)
		scanline = 0;

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
