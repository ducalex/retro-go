#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "pce.h"
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

extern uchar *VRAM2, *VRAMS;
/* these contain linear representations that we can draw */

extern uchar BGONSwitch;
/* do we have to draw background ? */

extern uchar SPONSwitch;
/* Do we have to draw sprites ? */

#define	PAL(c)	R[c]
#define	SPal	(Palette+256)

#define	MinLine	io.minline
#define	MaxLine	io.maxline

//#define ScrollX   io.scroll_x
//#define ScrollY io.scroll_y
#define	ScrollX	IO_VDC_07_BXR.W
#define	ScrollY	IO_VDC_08_BYR.W

extern int ScrollYDiff;

#define FC_W     io.screen_w
#define FC_H     256
#define V_FLIP  0x8000
#define H_FLIP  0x0800
extern uchar *SPM;

// The true refreshing function
extern void RefreshLine(int Y1, int Y2);
extern void RefreshSpriteExact(int Y1, int Y2, uchar bg);
extern int32 CheckSprites(void);
extern void RefreshScreen(void);

#endif
