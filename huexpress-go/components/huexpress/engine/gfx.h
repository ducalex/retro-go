#ifndef _GFX_H_
#define _GFX_H_

typedef struct {
	/* Vertical position */
	uint16_t y;

	/* Horizontal position */
	uint16_t x;

	/* Offset in VRAM */
	uint16_t no;

	/* Attributes */
	uint16_t attr;
	/*
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
} sprite_t;

typedef struct {
	short scroll_x;
	short scroll_y;
	short control;
	short latched;
} gfx_context_t;

extern bool TILE_CACHE[2048];
extern bool SPR_CACHE[512];

extern int ScrollYDiff;

#define OBJ_CACHE_INVALIDATE(x) { \
	TILE_CACHE[((x) / 16) & 0x7FF] = 0; \
    SPR_CACHE[((x) / 64) & 0x1FF] = 0; \
}

#define V_FLIP  0x8000
#define H_FLIP  0x0800

// The extra 32's are linked to the way the sprite are drawn on screen, which can overlap to near memory
// If only one pixel is drawn in the screen, the whole sprite is written, which can eventually overlap unexpected area
// This could be fixed at the cost of performance but we don't need the extra memory
#define XBUF_WIDTH 	(480 + 32)
#define	XBUF_HEIGHT	(242 + 32)

int gfx_init(void);
void gfx_run(void);
void gfx_term(void);
void gfx_irq(int type);
void gfx_clear_cache(void);
void gfx_latch_context(int slot_number);
void gfx_set_scroll_diff(void);

#endif
