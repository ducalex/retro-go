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
 *          1 -> must be draw horizontaly flipped
 * bit 13-12 : heigth
 *          00 -> 16 pixels
 *          01 -> 32 pixels
 *          10 -> 48 pixels
 *          11 -> 64 pixels
 * bit 15 : vertical flip
 *          0 -> normal shape
 *          1 -> must be drawn verticaly flipped
 */

typedef struct
{
	bool Planes[2048];
	bool Sprites[512];
} sprite_cache_t;

extern sprite_cache_t SPR_CACHE;

extern uchar *SPM;

extern uchar *VRAM2, *VRAMS;
/* these contain linear representations that we can draw */

extern int ScrollYDiff;

#define V_FLIP  0x8000
#define H_FLIP  0x0800

// The true refreshing function
extern void RefreshLine(int Y1, int Y2);
extern void RefreshSpriteExact(int Y1, int Y2, uchar bg);
extern void RefreshScreen(void);

#endif
