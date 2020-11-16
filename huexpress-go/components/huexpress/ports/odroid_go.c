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
#include <ctype.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <osd.h>

/**
 * Video
 */

static uint16_t mypalette[256];
static uint8_t *framebuffers[2];
static odroid_video_frame_t frames[2];
static odroid_video_frame_t *curFrame, *prevFrame;
static uint8_t current_fb = 0;
static bool gfx_init_done = false;

uint8_t* osd_gfx_buffer = NULL;
uint osd_skipFrames = 0;
uint osd_blitFrames = 0;
uint osd_fullFrames = 0;

extern QueueHandle_t audioSyncQueue;

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )

// #define USE_PARTIAL_FRAMES

static inline void set_current_fb(int i)
{
    current_fb = i & 1;
    osd_gfx_buffer = framebuffers[current_fb];
    prevFrame = curFrame;
    curFrame = &frames[current_fb];
}


void osd_gfx_init(void)
{
    framebuffers[0] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    framebuffers[1] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);

    // We never de-allocate them, so we don't care about the malloc pointer
    framebuffers[0] += XBUF_WIDTH * 64 + 32;
    framebuffers[1] += XBUF_WIDTH * 64 + 32;
}


void osd_gfx_set_mode(int width, int height)
{
    printf("%s: (%dx%d)\n", __func__, width, height);

    int crop_h = MAX(0, width - ODROID_SCREEN_WIDTH);
    int crop_v = MAX(0, height - ODROID_SCREEN_HEIGHT);

    // Should we consider vdc_minline/vdc_maxline to offset?

    printf("%s: Cropping H: %d V: %d\n", __func__, crop_h, crop_v);

	frames[0].width = width - crop_h;
	frames[0].height = height - crop_v;
	frames[0].stride = XBUF_WIDTH;
	frames[0].pixel_size = 1;
	frames[0].pixel_mask = 0xFF;
    frames[0].pixel_clear = -1;
	frames[0].palette = mypalette;
	frames[1] = frames[0];

	frames[0].buffer = framebuffers[0] + (crop_v / 2) * XBUF_WIDTH + (crop_h / 2);
	frames[1].buffer = framebuffers[1] + (crop_v / 2) * XBUF_WIDTH + (crop_h / 2);

    set_current_fb(0);

    odroid_display_force_refresh();
    gfx_init_done = true;
}


void osd_gfx_blit(void)
{
    if (!gfx_init_done) return;

    bool drawFrame = !osd_skipFrames;

    if (drawFrame)
    {
#ifndef USE_PARTIAL_FRAMES
        curFrame->pixel_clear = Palette[0];
        prevFrame = NULL;
#endif
        if (odroid_display_queue_update(curFrame, prevFrame) == SCREEN_UPDATE_FULL) {
            osd_fullFrames++;
        }
        set_current_fb(!current_fb);
        osd_blitFrames++;
    }

    // See if we need to skip a frame to keep up
    if (osd_skipFrames == 0)
    {
#ifndef USE_PARTIAL_FRAMES
        osd_skipFrames++;
#endif
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


void osd_gfx_set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t col = 0xffff;
    if (index != 255)
    {
        col = COLOR_RGB(r >> 2, g >> 2, b >> 2);
        col = (col << 8) | (col >> 8);
    }
    mypalette[index] = col;
}


/**
 * Keyboard
 */

int osd_input_init(void)
{
    return 0;
}

void osd_input_read(void)
{
    odroid_gamepad_state_t joystick;
    odroid_input_read_gamepad(&joystick);

	if (joystick.values[ODROID_INPUT_MENU]) {
		odroid_overlay_game_menu();
	}
	else if (joystick.values[ODROID_INPUT_VOLUME]) {
		odroid_overlay_game_settings_menu(NULL);
	}

    unsigned char rc = 0;
    if (joystick.values[ODROID_INPUT_LEFT]) rc |= JOY_LEFT;
    if (joystick.values[ODROID_INPUT_RIGHT]) rc |= JOY_RIGHT;
    if (joystick.values[ODROID_INPUT_UP]) rc |= JOY_UP;
    if (joystick.values[ODROID_INPUT_DOWN]) rc |= JOY_DOWN;

    if (joystick.values[ODROID_INPUT_A]) rc |= JOY_A;
    if (joystick.values[ODROID_INPUT_B]) rc |= JOY_B;
    if (joystick.values[ODROID_INPUT_START]) rc |= JOY_RUN;
    if (joystick.values[ODROID_INPUT_SELECT]) rc |= JOY_SELECT;

    io.JOY[0] = rc;
}


/**
 * Sound
 */

#define AUDIO_SAMPLE_RATE   (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)

static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];


static void
audioTask(void *arg)
{
    printf("%s: STARTED\n", __func__);

    while (1)
    {
        snd_update(audioBuffer, AUDIO_BUFFER_LENGTH);
        odroid_audio_submit(audioBuffer, AUDIO_BUFFER_LENGTH);
    }

    vTaskDelete(NULL);
}

void osd_snd_init(void)
{
    host.sound.stereo = true;
    host.sound.freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_size = 1;

    xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 2, NULL, 5, NULL, 1);
}

void osd_snd_shutdown(void)
{
    odroid_audio_stop();
}


/**
 * Misc
 */

void osd_log(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}


void osd_wait_next_vsync(void)
{
	const double deltatime = (1000000.0 / 60.0);
	static double lasttime, curtime, prevtime, sleep;
	struct timeval tp;

	gettimeofday(&tp, NULL);
	curtime = tp.tv_sec * 1000000.0 + tp.tv_usec;

	sleep = lasttime + deltatime - curtime;

	if (sleep > 0)
	{
		tp.tv_sec = 0;
		tp.tv_usec = sleep;
		select(1, NULL, NULL, NULL, &tp);
		usleep(sleep);
	}
	else if (sleep < -8333.0)
	{
		osd_skipFrames++;
	}

    odroid_system_tick(!osd_blitFrames, osd_fullFrames, (uint)(curtime - prevtime));
	osd_blitFrames = 0;
	osd_fullFrames = 0;

	gettimeofday(&tp, NULL);
	curtime = prevtime = tp.tv_sec * 1000000.0 + tp.tv_usec;
	lasttime += deltatime;

	if ((lasttime + deltatime) < curtime)
		lasttime = curtime;
}


void* osd_alloc(size_t size)
{
    return rg_alloc(size, (size <= 0x10000) ? MEM_FAST : MEM_SLOW);
}
