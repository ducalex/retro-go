/*

This program implements the graphics rendering for the different variation
of SM510 family microcomputer.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#include <string.h>

#include "gw_type_defs.h"
#include "gw_graphic.h"
#include "gw_system.h"
#include "gw_romloader.h"
#include "sm510.h"

//  RGB565 16bits pixel format to 32bits pixel format
/*
            RRRRRGGGGGGBBBBB
			RGB16 bits to 32 bits
            RRRRRGGGGGGBBBBBRRRRRGGGGGGBBBBB
            00000111111000001111100000011111
        0x     ...0...7...e...0...f...8...1....f
*/
#define GW_RGB565_32BITS 0x07E0F81F

// RGB565 16bits pixel format mask:  per color
#define GW_MASK_RGB565_R 0xF800
#define GW_MASK_RGB565_G 0x07E0
#define GW_MASK_RGB565_B 0x001F

static uint16 *gw_graphic_framebuffer = 0;

static uint16 *source_mixer = 0;

static uint8 SEG_TRANSPARENT_COLOR = 0;
#define SEG_WHITE_COLOR 0xff
#define SEG_BLACK_COLOR 0x0

/*** Basic Pixel functions ***/
/*
  RGB multiplicative blending between background and segment 
  bg: background RGB565 16bits pixel format
  sg: segment is 8bits Green of RGB565 16 pixel format
*/

static inline uint16 rgb_multiply_8bits(uint32 bg, uint32 sg)
{

	/* Separate each colors */
	uint32 bg_r = (bg & GW_MASK_RGB565_R) >> 11;
	uint32 bg_g = (bg & GW_MASK_RGB565_G) >> 5;
	uint32 bg_b = (bg & GW_MASK_RGB565_B);

	/* RGB multiply and normalize */
	bg_r = (bg_r * sg) >> 8;
	bg_g = (bg_g * sg) >> 8;
	bg_b = (bg_b * sg) >> 8;

	/* return in RGB565 format */
	return (uint16)(bg_r << 11) | (bg_g << 5) | bg_b;
}

/************************ Generic function to display a segment *****************/
void (*update_segment)(uint8 segment_nb, bool segment_state);

/* Segment to framebuffer graphic function */
/* segments are stored from 0 to 255 in memory*/

/* Deviation 2bits segment resolution function */
__attribute__((optimize("unroll-loops"))) static inline void update_segment_2bits(uint8 segment_nb, bool segment_state)
{

	/* segment first pixel corner (up/left) */
	uint32 segment = gw_segments_offset[segment_nb];

	uint8 cur_pixel,cur_pixelr;
	int idx = 0;

	/* nothing to do for this segment */
	if (segment_state == 0)
		return;

	/* 4bits data size access */
	idx = segment & 0x3;
	segment = segment >> 2;

	uint8 *pixel;
	pixel = &gw_segments[segment];

	/* get segment coordinates */
	uint16 segments_x = gw_segments_x[segment_nb];
	uint16 segments_y = gw_segments_y[segment_nb];
	uint16 segments_width = gw_segments_width[segment_nb];
	uint16 segments_height = gw_segments_height[segment_nb];

	for (int line = segments_y; line < segments_height + segments_y; line++)
	{
		for (int x = segments_x; x < segments_width + segments_x; x++)
		{

			cur_pixelr = (pixel[idx >> 2] >> 2*(idx & 0x3)) &0x3;

			idx++;

			cur_pixel = cur_pixelr | cur_pixelr << 2 | cur_pixelr << 4 | cur_pixelr << 6;


			/* Check if there something to mix
			if the segment pixel is transparent nothing to do. */
			if (cur_pixel != SEG_TRANSPARENT_COLOR) {
			
			// change black color to get transparency effect
			if ( cur_pixel == 0 ) cur_pixel = 39;
			gw_graphic_framebuffer[line * GW_SCREEN_WIDTH + x] = rgb_multiply_8bits(source_mixer[line * GW_SCREEN_WIDTH + x], cur_pixel);
			}

		}
	}
}
/* Deviation 4bits segment resolution function */
__attribute__((optimize("unroll-loops"))) static inline void update_segment_4bits(uint8 segment_nb, bool segment_state)
{

	/* segment first pixel corner (up/left) */
	uint32 segment = gw_segments_offset[segment_nb];

	uint8 cur_pixel;
	int idx = 0;

	/* nothing to do for this segment */
	if (segment_state == 0)
		return;

	/* 4bits data size access */
	idx = segment & 0x1;
	segment = segment >> 1;

	uint8 *pixel;
	pixel = &gw_segments[segment];

	/* get segment coordinates */
	uint16 segments_x = gw_segments_x[segment_nb];
	uint16 segments_y = gw_segments_y[segment_nb];
	uint16 segments_width = gw_segments_width[segment_nb];
	uint16 segments_height = gw_segments_height[segment_nb];

	for (int line = segments_y; line < segments_height + segments_y; line++)
	{
		for (int x = segments_x; x < segments_width + segments_x; x++)
		{

			if ((idx & 0x1) == 0)
				cur_pixel = pixel[idx >> 1] & 0xF0;
			else
				cur_pixel = pixel[idx >> 1] << 4;

			cur_pixel |= cur_pixel >> 4;
			idx++;

			/* Check if there something to mix
			if the segment pixel is transparent nothing to do. */
			if (cur_pixel != SEG_TRANSPARENT_COLOR)
				gw_graphic_framebuffer[line * GW_SCREEN_WIDTH + x] = rgb_multiply_8bits(source_mixer[line * GW_SCREEN_WIDTH + x], cur_pixel);
		}
	}
}

/* Deviation 8bits segment resolution function */
__attribute__((optimize("unroll-loops"))) static inline void update_segment_8bits(uint8 segment_nb, bool segment_state)
{

	/* segment first pixel corner (up/left) */
	uint32 segment = gw_segments_offset[segment_nb];

	uint8 cur_pixel;
	int idx = 0;

	/* nothing to do for this segment */
	if (segment_state == 0)
		return;

	uint8 *pixel;
	pixel = &gw_segments[segment];

	/* get segment coordinates */
	uint16 segments_x = gw_segments_x[segment_nb];
	uint16 segments_y = gw_segments_y[segment_nb];
	uint16 segments_width = gw_segments_width[segment_nb];
	uint16 segments_height = gw_segments_height[segment_nb];

	for (int line = segments_y; line < segments_height + segments_y; line++)
	{
		for (int x = segments_x; x < segments_width + segments_x; x++)
		{

			cur_pixel = pixel[idx];
			idx++;

			/* Check if there something to mix
				if the segment pixel is black, it's considered transparent.
				it prevents from mixing 2 overlapping segments
				'gw_background' is the colour sheet. segment colour is white. */
			if (cur_pixel != SEG_TRANSPARENT_COLOR)
				gw_graphic_framebuffer[line * GW_SCREEN_WIDTH + x] = rgb_multiply_8bits(source_mixer[line * GW_SCREEN_WIDTH + x], cur_pixel);
		}
	}
}

/* Specific functions to pool segments status */

/* Flicker filter enable flag */
static bool deflicker_enabled = false;

/* SM510 RAM based LCD controller */
__attribute__((optimize("unroll-loops"))) inline void gw_gfx_sm510_rendering(uint16 *framebuffer)
{
	/*
#SM51X series: output to x.y.z, where:
	# x = group a/b/bs/c (0/1/2/3)
	# y = segment 1-16 (0-15)
	# z = common H1-H4 (0-3)
            
#segment position   = 64*x + 4*y + z

    CPU RAM display 
	0X50.. 0X5F : c1..c16
	0X60.. 0X6F : a1..a16
	0X70.. 0X7F : b1..b16
*/

	uint8 HxA, HxB, HxC; // RAM segment 4bits value H4..H1

	uint8 segment_position;
	uint8 segment_state;

	gw_graphic_framebuffer = framebuffer;

	if (gw_head.flags & FLAG_RENDERING_LCD_INVERTED)
	{

		memset(framebuffer, 0, GW_SCREEN_WIDTH * GW_SCREEN_HEIGHT * 2);
		SEG_TRANSPARENT_COLOR = SEG_BLACK_COLOR;
		source_mixer = gw_background;
	}
	else
	{
		memcpy(framebuffer, gw_background, GW_SCREEN_WIDTH * GW_SCREEN_HEIGHT * 2);
		SEG_TRANSPARENT_COLOR = SEG_WHITE_COLOR;
		source_mixer = framebuffer;
	}

	//scan group a1..a16,b1..b16,c11..c16
	for (int seg_y = 0; seg_y < NB_SEGS_ROW; seg_y++)
	{

		if (deflicker_enabled)
		{
			HxA = gw_ram_state[ADD_SEGA_BASE + seg_y] & 0xf;
			HxB = gw_ram_state[ADD_SEGB_BASE + seg_y] & 0xf;
			HxC = gw_ram_state[ADD_SEGC_BASE + seg_y] & 0xf;
		}
		else
		{
			HxA = gw_ram[ADD_SEGA_BASE + seg_y] & 0xf;
			HxB = gw_ram[ADD_SEGB_BASE + seg_y] & 0xf;
			HxC = gw_ram[ADD_SEGC_BASE + seg_y] & 0xf;
		}

		//scan H1..H4
		for (int seg_z = 0; seg_z < NB_SEGS_COL; seg_z++)
		{

			segment_position = 4 * seg_y + seg_z;

			//segment a
			segment_state = m_bc || !m_bp ? 0 : (HxA & (1 << seg_z)) != 0;
			update_segment(segment_position, segment_state);

			//segment b
			segment_state = m_bc || !m_bp ? 0 : (HxB & (1 << seg_z)) != 0;
			update_segment(segment_position + 64, segment_state);

			//segment c
			segment_state = m_bc || !m_bp ? 0 : (HxC & (1 << seg_z)) != 0;
			update_segment(segment_position + 192, segment_state);
		}
	}

	/* bs1,b2 output from L/X and Y regs */

	//scan H1..H4 BS segment
	for (int seg_z = 0; seg_z < NB_SEGS_COL; seg_z++)
	{

		/* Blinking mask using m_y and m_div as timer */
		uint8 blink = (m_div & 0x4000) ? m_y : 0;

		/* bs1 is derived from ml */
		uint8 seg = (m_l & ~blink);
		segment_state = (m_bc || !m_bp) ? 0 : seg;

		update_segment(128 + seg_z, ((segment_state & (1 << seg_z)) != 0));

		/* bs2 is derived from mx */
		seg = (m_x & ~blink);
		segment_state = (m_bc || !m_bp) ? 0 : seg;

		update_segment(132 + seg_z, ((segment_state & (1 << seg_z)) != 0));
	}
}

/* SM500 I/O based LCD controller */
__attribute__((optimize("unroll-loops"))) inline void gw_gfx_sm500_rendering(uint16 *framebuffer)
{
	/*
# SM500/SM5A series: output to x.y.z, where:
	# x = O group (0-*)
	# y = O segment 1-4 (0-3)
	# z = common H1/H2 (0/1)
            
#segment position  = 8*x + 2*y + z    
	 56 segments
	m_ox[9];  W' latch, max 9
	m_o[9];    W latch
*/
	uint8 seg;

	gw_graphic_framebuffer = framebuffer;

	if (gw_head.flags & FLAG_RENDERING_LCD_INVERTED)
	{

		memset(framebuffer, 0, GW_SCREEN_WIDTH * GW_SCREEN_HEIGHT * 2);
		SEG_TRANSPARENT_COLOR = SEG_BLACK_COLOR;
		source_mixer = gw_background;
	}
	else
	{
		memcpy(framebuffer, gw_background, GW_SCREEN_WIDTH * GW_SCREEN_HEIGHT * 2);
		SEG_TRANSPARENT_COLOR = SEG_WHITE_COLOR;
		source_mixer = framebuffer;
	}

	// 2 columns z
	for (int h = 0; h < 2; h++)
	{
		// number of W,W' output registers (m_o_pins= 7 or 9)
		for (int o = 0; o < m_o_pins; o++)
		{
			// 4 segments per group
			// check if LCD deflicker is activated
			if (deflicker_enabled)
				seg = h ? m_ox_state[o] : m_o_state[o];
			else
				seg = h ? m_ox[o] : m_o[o];

			// 8x+2y+z with x=o, y=2,4,6,8, z=h (72 segments max.)
			update_segment(8 * o + 0 + h, m_bp ? ((seg & 0x1) != 0) : 0); // 0,1 8,9 16,17 24,25 32,33 40,41 48,49 56,57 64,65
			update_segment(8 * o + 2 + h, m_bp ? ((seg & 0x2) != 0) : 0); // 2,3
			update_segment(8 * o + 4 + h, m_bp ? ((seg & 0x4) != 0) : 0); // 4,5
			update_segment(8 * o + 6 + h, m_bp ? ((seg & 0x8) != 0) : 0); // 6,7
		}
	}
}
void gw_gfx_init()
{

	/* init LCD deflicker level */
	// for emulated cpus side
	flag_lcd_deflicker_level = (gw_head.flags & FLAG_LCD_DEFLICKER_MASK) >> 6;

	// for segments rendering side
	deflicker_enabled = (flag_lcd_deflicker_level != 0);

	/* determine which API to use for segments rendering */
	update_segment = update_segment_8bits;

	if (gw_head.flags & FLAG_SEGMENTS_4BITS)
		update_segment = update_segment_4bits;
	if (gw_head.flags & FLAG_SEGMENTS_2BITS)
		update_segment = update_segment_2bits;

}
