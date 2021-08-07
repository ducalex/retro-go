#pragma once

#include <stdbool.h>

// The extra 32's are linked to the way the sprite are drawn on screen, which can overlap to near memory
// If only one pixel is drawn in the screen, the whole sprite is written, which can eventually overlap unexpected area
// This could be fixed at the cost of performance but we don't need the extra memory
#define XBUF_WIDTH 	(352 + 32)
#define	XBUF_HEIGHT	(242 + 32)

int gfx_init(void);
void gfx_run(void);
void gfx_term(void);
void gfx_irq(int type);
void gfx_reset(bool hard);
void gfx_latch_context(int force);
void gfx_obj_cache_invalidate(int num);
