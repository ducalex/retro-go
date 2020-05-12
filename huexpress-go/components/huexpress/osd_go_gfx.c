/*****************************************/
/* ESP32 (Odroid GO) Graphics Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*	ducalex                              */
/*****************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <odroid_system.h>
#include <string.h>
#include "utils.h"
#include "osd.h"

static uint16_t mypalette[256];
static uint8_t *framebuffers[2];
static odroid_video_frame frames[2];
static odroid_video_frame *curFrame;
static uint8_t current_fb = 0;
static bool gfx_init_done = false;

uchar* osd_gfx_buffer = NULL;
uint osd_skipFrames = 0;
uint osd_blitFrames = 0;

extern uchar *SPM_raw, *SPM;

static QueueHandle_t videoTaskQueue;
extern QueueHandle_t audioSyncQueue;

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )

static inline void set_current_fb(int i)
{
    current_fb = i & 1;
    osd_gfx_buffer = frames[current_fb].buffer;
    curFrame = &frames[current_fb];

    for (int i = 0; i < 240; i++) {
        // memset(osd_gfx_buffer + i * XBUF_WIDTH, Palette[0], io.screen_w);
        memset(SPM + i * XBUF_WIDTH, 0, io.screen_w);
    }
}


static void videoTask(void *arg)
{
    videoTaskQueue = xQueueCreate(1, sizeof(void*));
    odroid_video_frame *frame;

    while(1)
    {
        xQueueReceive(videoTaskQueue, &frame, portMAX_DELAY);
        frame->pixel_clear = Palette[0];
        // odroid_display_queue_update(frame, frame == &frames[0] ? &frames[1] : &frames[0]);
        odroid_display_queue_update(frame, NULL);
    }

    videoTaskQueue = NULL;

    vTaskDelete(NULL);

    while (1) {}
}


void osd_gfx_init(void)
{
    framebuffers[0] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    framebuffers[1] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    SPM_raw         = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    SPM = SPM_raw + XBUF_WIDTH * 64 + 32;

    // UPeriod = 1;

    // xTaskCreatePinnedToCore(&videoTask, "videoTask", 3072, NULL, 5, NULL, 1);
}


void osd_gfx_set_mode(short width, short height)
{
    printf("%s: (%dx%d)\n", __func__, width, height);

    int crop_w = MAX(0, width - ODROID_SCREEN_WIDTH);

    // Should we consider vdc_minline/vdc_maxline to offset?

	frames[0].width = width - crop_w;
	frames[0].height = height;
	frames[0].stride = XBUF_WIDTH;
	frames[0].pixel_size = 1;
	frames[0].pixel_mask = 0xFF;
    frames[0].pixel_clear = 0;
	frames[0].palette = mypalette;
	frames[1] = frames[0];

	frames[0].buffer = framebuffers[0] + 32 + 64 * XBUF_WIDTH + (crop_w / 2);
	frames[1].buffer = framebuffers[1] + 32 + 64 * XBUF_WIDTH + (crop_w / 2);

    set_current_fb(0);

    forceVideoRefresh = true;
    gfx_init_done = true;
}


void osd_gfx_blit(void)
{
    if (!gfx_init_done) return;

    bool drawFrame = !osd_skipFrames;
    bool fullFrame = true;

    if (drawFrame)
    {
        // xQueueSend(videoTaskQueue, &curFrame, portMAX_DELAY);
        curFrame->pixel_clear = Palette[0];
        odroid_display_queue_update(curFrame, NULL);
        set_current_fb(!current_fb);
        osd_blitFrames++;
    }

    // See if we need to skip a frame to keep up
    if (osd_skipFrames == 0)
    {
        osd_skipFrames++;

        if (speedupEnabled) osd_skipFrames += speedupEnabled * 2.5;
    }
    else if (osd_skipFrames > 0)
    {
        osd_skipFrames--;
    }
}


void osd_gfx_shutdown(void)
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
