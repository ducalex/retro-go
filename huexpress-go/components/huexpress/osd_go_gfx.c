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

static uint frameTime = 0;
uint skipFrames = 0;

extern uchar *SPM_raw, *SPM;

static odroid_video_frame frame1;
static odroid_video_frame frame2;

static bool gfx_init_done = false;

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )

#if 0
#define PCENGINE_GAME_WIDTH  352
#define PCENGINE_GAME_HEIGHT 240
#define PCENGINE_REMOVE_X 16
#define XBUF_WIDTH  (536 + 32 + 32)

extern uchar *Pal;
extern uchar *SPM;

void ili9341_write_frame_pcengine_mode0(uint8_t* buffer, uint16_t* pal)
{
    // ili9341_write_frame_rectangleLE(0,0,300,240, buffer -32);
#if 0
    uint8_t* framePtr = buffer + PCENGINE_REMOVE_X;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer;
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          //int index = i * displayWidth;
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            //line_buffer[index++] = value1;
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
    //memset(buffer, Pal[0], 240 * XBUF_WIDTH);
    //memset(SPM, 0, 240 * XBUF_WIDTH);
#endif

    uint8_t* framePtr = buffer + PCENGINE_REMOVE_X;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer;
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
          sPtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
    //memset(buffer, Pal[0], 240 * XBUF_WIDTH);
    //memset(SPM, 0, 240 * XBUF_WIDTH);

    //#define MISSING ( 240 * XBUF_WIDTH - PCENGINE_GAME_HEIGHT*320)
    //memset(sPtr, 0, MISSING);
}
#endif

static inline void set_current_fb(int i)
{
    current_fb = i & 1;
    XBuf = framebuffers[current_fb];
    osd_gfx_buffer = XBuf + 32 + 64 * XBUF_WIDTH;
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
        short ret;

		if (current_fb == 0)
        {
    		ret = odroid_display_queue_update(&frame1, NULL);
            set_current_fb(1);
        }
		else
        {
	    	ret = odroid_display_queue_update(&frame2, NULL);
            set_current_fb(0);
        }

        fullFrame = (ret == SCREEN_UPDATE_FULL);
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
