#ifndef _GFX_H_
#define _GFX_H_

/* Sprite structure
 * y,x = pos
 * no = address of the pattern in the Video ram
 * atr = attributes
 * bit 0-4 : number of the palette to be used
 * bit 7 : background sprite
 *          0 -> must be drawn behind tiles
 *          1 -> must be drawn in front of tiles
 * bit 8 : width
 *          0 -> 16 pixels
 *          1 -> 32 pixels
 * bit 11 : horizontal flip
 *          0 -> normal shape
 *          1 -> must be draw horizontally flipped
 * bit 13-12 : height
 *          00 -> 16 pixels
 *          01 -> 32 pixels
 *          10 -> 48 pixels
 *          11 -> 64 pixels
 * bit 15 : vertical flip
 *          0 -> normal shape
 *          1 -> must be drawn vertically flipped
 */
typedef struct {
	short y, x, no, atr;
} sprite_t;

typedef struct {
	int16 scroll_x;
	int16 scroll_y;
	int16 scroll_y_diff;
	int16 control;
} gfx_context_t;

extern bool TILE_CACHE[4096];
extern bool SPR_CACHE[1024];

#define V_FLIP  0x8000
#define H_FLIP  0x0800

// The extra 32's and 64's are linked to the way the sprite are blitted on screen, which can overlap to near memory
// If only one pixel is drawn in the screen, the whole sprite is written, which can eventually overlap unexpected area
// This could be fixed at the cost of performance but we don't need the extra memory
#define XBUF_WIDTH 	(512 + 32 + 32) // Most games only need 360 but as of rg 1.20 we have memory to spare
#define	XBUF_HEIGHT	(242 + 64 + 64)

int  gfx_init();
void gfx_term();
void gfx_clear_cache();
void gfx_change_video_mode();
void gfx_save_context(char slot_number);
// void gfx_load_context(char slot_number);
uchar gfx_loop();

extern int ScrollYDiff;

#endif
