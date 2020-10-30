#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "cleantypes.h"

typedef struct {
	short y, x, no, atr;
} SPR;
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

typedef struct
{
	bool sprite_valid[512];
	bool tile_valid[2048];
	uchar *data;
} object_cache_t;

extern object_cache_t OBJ_CACHE;

extern uchar *SPM;

extern int ScrollYDiff;

#define V_FLIP  0x8000
#define H_FLIP  0x0800

// The true refreshing function
extern void RefreshLine(int Y1, int Y2);
extern void RefreshSpriteExact(int Y1, int Y2, uchar bg);
extern void RefreshScreen(void);

#endif
