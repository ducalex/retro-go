/*****************************************/
/* ESP32 (Odroid GO) Graphics Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*	ducalex                              */
/*****************************************/

#include <odroid_system.h>
#include <string.h>
#include "osd_gfx.h"
#include "utils.h"

static uint16_t mypalette[256];
static uint8_t *framebuffers[2];
static odroid_video_frame frames[2];
static uint8_t current_fb = 0;
static bool gfx_init_done = false;

uchar* XBuf = NULL;
uchar* osd_gfx_buffer = NULL;

static uint frameTime = 0;
uint skipFrames = 0;

extern uchar *SPM_raw, *SPM;
extern uchar *Pal;

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )

static inline void set_current_fb(int i)
{
    current_fb = i & 1;
    XBuf = framebuffers[current_fb];
    osd_gfx_buffer = frames[current_fb].buffer;

    for (int i = 0; i < 240; i++) {
        memset(osd_gfx_buffer + i * XBUF_WIDTH, Pal[0], 336);
        memset(SPM + i * XBUF_WIDTH, 0, 336);
    }
}

int osd_gfx_init(void)
{
    printf("%s: (%dx%d)\n", __func__, io.screen_w, io.screen_h);

    forceVideoRefresh = true;
    frameTime = get_frame_time(60);

    framebuffers[0] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    framebuffers[1] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    SPM_raw         = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    SPM = SPM_raw + XBUF_WIDTH * 64 + 32;

    return true;
}


int osd_gfx_init_normal_mode(void)
{
	frames[0].width = io.screen_w;
	frames[0].height = io.screen_h;
	frames[0].stride = XBUF_WIDTH;
	frames[0].pixel_size = 1;
	frames[0].pixel_mask = 0xFF;
	frames[0].palette = mypalette;
	frames[1] = frames[0];

	frames[0].buffer = framebuffers[0] + 32 + 64 * XBUF_WIDTH;
	frames[1].buffer = framebuffers[1] + 32 + 64 * XBUF_WIDTH;

    set_current_fb(0);
    SetPalette();

    gfx_init_done = true;
    return true;
}


void osd_gfx_put_image_normal(void)
{
    if (!gfx_init_done) return;

    uint startTime = get_elapsed_time();
    bool drawFrame = !skipFrames;
    bool fullFrame = false;

    if (drawFrame)
    {
        fullFrame = odroid_display_queue_update(&frames[current_fb], NULL) == SCREEN_UPDATE_FULL;
        set_current_fb(!current_fb);
    }

    // See if we need to skip a frame to keep up
    if (skipFrames == 0)
    {
        skipFrames = 1;
        if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
        if (speedupEnabled) skipFrames += speedupEnabled * 2.5;
    }
    else if (skipFrames > 0)
    {
        skipFrames--;
    }

    // odroid_audio_submit(pcm.buf, pcm.pos >> 1);

    odroid_system_stats_tick(!drawFrame, fullFrame);
}


void osd_gfx_shut_normal_mode(void)
{
    printf("%s: \n", __func__);
}


void osd_gfx_set_color(uchar index, uchar r, uchar g, uchar b)
{
    uint16_t col;
    if (index == 255)
    {
        col = 0xffff;
    }
    else
    {
        col = COLOR_RGB(r >> 2, g >> 2, b >> 2);
        col = (col << 8) | (col >> 8);
    }
    mypalette[index] = col;
}


osd_gfx_driver osd_gfx_driver_list[1] = {
    {
        osd_gfx_init,
        osd_gfx_init_normal_mode,
        osd_gfx_put_image_normal,
        osd_gfx_shut_normal_mode
    }
};
