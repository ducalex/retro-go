/*****************************************/
/* ESP32 (Odroid GO) Graphics Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*	ducalex                              */
/*****************************************/

#include <odroid_system.h>
#include "osd_gfx.h"
#include "utils.h"

uint16_t mypalette[256];
uint8_t *framebuffers[2];
uint8_t current_fb;

uchar* XBuf = NULL;
uchar* osd_gfx_buffer = NULL;

static uint totalElapsedTime = 0;
static uint emulatedFrames = 0;
static uint skippedFrames = 0;
static uint fullFrames = 0;
static uint frameTime = 0;

bool skipFrame;

static odroid_video_frame frame1;
static odroid_video_frame frame2;

static bool gfx_init_done = false;

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )


static inline void set_current_fb(int i)
{
    current_fb = i & 1;
    XBuf = framebuffers[current_fb];
    osd_gfx_buffer = XBuf + 32 + 64 * XBUF_WIDTH;
}


static void init_gfx()
{
    printf("%s: (%dx%d)\n", __func__, io.screen_w, io.screen_h);

    forceVideoRefresh = true;
    frameTime = get_frame_time(60);

    if (!framebuffers[0]) framebuffers[0] = calloc(1, XBUF_WIDTH * XBUF_HEIGHT);
    if (!framebuffers[1]) framebuffers[1] = calloc(1, XBUF_WIDTH * XBUF_HEIGHT);

    assert(framebuffers[0] && framebuffers[1]);

	frame1.width = io.screen_w;
	frame1.height = io.screen_h;
	frame1.stride = XBUF_WIDTH;
	frame1.pixel_size = 1;
	frame1.pixel_mask = 0xFF;
	frame1.palette = mypalette;
	frame2 = frame1;

	frame1.buffer = framebuffers[0] + 32 + 64 * XBUF_WIDTH;
	frame2.buffer = framebuffers[1] + 32 + 64 * XBUF_WIDTH;

    set_current_fb(0);
    SetPalette();

    gfx_init_done = true;
}


int osd_gfx_init(void)
{
    init_gfx();
    return true;
}


int osd_gfx_init_normal_mode(void)
{
    init_gfx();
    return true;
}


void osd_gfx_put_image_normal(void)
{
    if (!gfx_init_done) return;

    uint startTime = get_elapsed_time();

    if (skipFrame)
    {
        ++skippedFrames;
    }
    else
    {
        short ret;

		if (current_fb == 0)
        {
    		ret = odroid_display_queue_update(&frame1, &frame2);
            set_current_fb(1);
        }
		else
        {
	    	ret = odroid_display_queue_update(&frame2, &frame1);
            set_current_fb(0);
        }

        if (ret == SCREEN_UPDATE_FULL) ++fullFrames;
    }

    skipFrame = !skipFrame && get_elapsed_time_since(startTime) > frameTime;

    // odroid_audio_submit(pcm.buf, pcm.pos >> 1);

    totalElapsedTime += get_elapsed_time_since(startTime);
    ++emulatedFrames;

    if (emulatedFrames == 60)
    {
        odroid_system_print_stats(totalElapsedTime, emulatedFrames, skippedFrames, fullFrames);

        emulatedFrames = skippedFrames = fullFrames = 0;
        totalElapsedTime = 0;
    }
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
