#ifndef _GFX_H_
#define _GFX_H_

//#include <SDL.h>

#include "sys_dep.h"


//#ifdef MY_VIDEO_MODE_SCANLINES
struct my_scanline {
    int YY1;
    int YY2;
    uint8_t* buffer;
};
//#endif

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

#include "sprite.h"

#define textoutshadow(bmp,f,s,x,y,color1,color2,offsetx,offsety) { textout(bmp,f,s,x+offsetx,y+offsety,color2); textout(bmp,f,s,x,y,color1); }
// Just a little define to avoid too many keystrokes ;)

/*
 * generic_rect - Used to keep calc_fullscreen_aspect gfx lib independant.  Currently
 *   used to remap values to an SDL_Rect structure.
 */
struct generic_rect {
	unsigned short start_x, start_y;
	unsigned short end_x, end_y;
};

typedef struct {
	//typeof(IO_VDC_07_BXR.W) scroll_x;
	//typeof(IO_VDC_08_BYR.W) scroll_y;
	uint16 scroll_x;
	uint16 scroll_y;
	int scroll_y_diff;
	//typeof(IO_VDC_05_CR.W) cr;
	uint16 cr;
} gfx_context;


//extern SDL_Surface *physical_screen;
//extern SDL_Rect physical_screen_rect;
//extern SDL_Color olay_cmap[256];

extern int video_dump_flag;
extern int gfx_need_video_mode_change;

void calc_fullscreen_aspect(unsigned short physical_screen_width,
							unsigned short physical_screen_height,
							struct generic_rect *rect,
							unsigned short pce_screen_width,
							unsigned short pce_screen_height);
void change_pce_screen_height();

#define MAX_GFX_CONTEXT_SLOT_NUMBER 2
extern gfx_context saved_gfx_context[MAX_GFX_CONTEXT_SLOT_NUMBER];

void save_gfx_context_(int slot_number);
// void load_gfx_context_(int slot_number);

#ifdef MY_INLINE_GFX
#define save_gfx_context(slot_number) do { \
    gfx_context *destination_context; \
    destination_context = saved_gfx_context + slot_number; \
    if (slot_number == 0) { \
        if (gfx_need_redraw == 0) \
            gfx_need_redraw = 1; \
        else { /* Context is already saved + we haven't render the line using it */ \
            TRACE("Canceled context saving as a previous one wasn't consumed yet\n"); \
            break;  \
        } \
    } \
    if (slot_number >= MAX_GFX_CONTEXT_SLOT_NUMBER) { \
        printf("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__, \
            __FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER); \
        Log("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__, \
            __FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER); \
        break; \
    } \
    destination_context->scroll_x = ScrollX; \
    destination_context->scroll_y = ScrollY; \
    destination_context->scroll_y_diff = ScrollYDiff; \
    destination_context->cr = IO_VDC_05_CR.W; \
    } while (false);

#define load_gfx_context(slot_number) \
    if (slot_number >= MAX_GFX_CONTEXT_SLOT_NUMBER) { \
        Log("Internal error in %s(%s), slot %d >= %d\n", __FUNCTION__, \
            __FILE__, slot_number, MAX_GFX_CONTEXT_SLOT_NUMBER); \
    } else {  \
    gfx_context *source_context; \
    source_context = saved_gfx_context + slot_number; \
    ScrollX = source_context->scroll_x; \
    ScrollY = source_context->scroll_y; \
    ScrollYDiff = source_context->scroll_y_diff; \
    IO_VDC_05_CR.W = source_context->cr; \
    TRACE("Restoring context %d, scroll = (%d,%d,%d), CR = 0x%02d\n", \
        slot_number, ScrollX, ScrollY, ScrollYDiff, IO_VDC_05_CR.W); \
    }

#define gfx_init() \
    UCount = 0; \
    gfx_need_video_mode_change = 0; \
    gfx_need_redraw = 0; \
    IO_VDC_05_CR.W = 0;

#else
#define save_gfx_context(slot_number) save_gfx_context_(slot_number)
#define load_gfx_context(slot_number) load_gfx_context_(slot_number)
void gfx_init();
#endif

int start_dump_video();
void stop_dump_video();
void dump_video_frame();

void dump_rgb_frame(char *output_buffer);

extern int UCount;
extern int gfx_need_redraw;

#ifdef MY_INLINE_GFX_Loop6502
#define GFX_Loop6502_Init \
    int video_dump_countdown = 0; \
    int display_counter = 0; \
    int last_display_counter = 0; \
    int satb_dma_counter = 0;

//uchar Loop6502_2();
#else
#define GFX_Loop6502_Init
uchar Loop6502();
#endif

#if ENABLE_TRACING_GFX
void gfx_debug_printf(char *format, ...);
#endif

#endif
