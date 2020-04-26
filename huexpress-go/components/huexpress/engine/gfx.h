#ifndef _GFX_H_
#define _GFX_H_

#include "sprite.h"
#include "osd.h"

#define	WIDTH	(360+64)
#define	HEIGHT	256

// The extra 32's and 64's are linked to the way the sprite are blitted on screen, which can overlap to near memory
// If only one pixel is drawn in the screen, the whole sprite is written, which can eventually overlap unexpected area
// A sharper way of doing would probably reduce the amount of needed data from 220kb to 128kb (eventually smaller if restricting
// games with hi res to be launched)
// XBUF_WIDTH = 536 + 32 + 32
// XBUG_HEIGHT = 240 + 64 + 64

#define XBUF_WIDTH 	(536 + 32 + 32)
#define	XBUF_HEIGHT	(240 + 64 + 64)


#define textoutshadow(bmp,f,s,x,y,color1,color2,offsetx,offsety) { textout(bmp,f,s,x+offsetx,y+offsety,color2); textout(bmp,f,s,x,y,color1); }
// Just a little define to avoid too many keystrokes ;)


typedef struct {
	//typeof(IO_VDC_07_BXR.W) scroll_x;
	//typeof(IO_VDC_08_BYR.W) scroll_y;
	uint16 scroll_x;
	uint16 scroll_y;
	int scroll_y_diff;
	//typeof(IO_VDC_05_CR.W) cr;
	uint16 cr;
} gfx_context;

extern int gfx_need_video_mode_change;

void change_pce_screen_height();

#define MAX_GFX_CONTEXT_SLOT_NUMBER 2
extern gfx_context saved_gfx_context[MAX_GFX_CONTEXT_SLOT_NUMBER];

void save_gfx_context(int slot_number);
// void load_gfx_context(int slot_number);
void gfx_init();

extern int UCount;
extern int gfx_need_redraw;

uchar Loop6502();

#endif
