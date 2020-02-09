// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "driver/rtc_io.h"
#include "driver/i2s.h"

#include "odroid_system.h"

//Nes stuff wants to define this as well...
#undef false
#undef true
#undef bool

#include <math.h>
#include <string.h>
#include <noftypes.h>
#include <bitmap.h>
#include <nofconfig.h>
#include <event.h>
#include <gui.h>
#include <log.h>
#include <nes.h>
#include <nes_pal.h>
#include <nesinput.h>
#include <osd.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "../nofrendo/nes/nesstate.h"

#define AUDIO_SAMPLERATE   32000
#define AUDIO_FRAGSIZE     (AUDIO_SAMPLERATE/NES_REFRESH_RATE)
//#define  AUDIO_FRAGSIZE

#define  DEFAULT_WIDTH        256
#define  DEFAULT_HEIGHT       NES_VISIBLE_HEIGHT

#define PIXEL_MASK 0x3F

struct update_meta {
    odroid_scanline diff[NES_VISIBLE_HEIGHT];
    uint8_t *buffer;
    int stride;
};

int8_t scaling_mode = ODROID_SCALING_FILL;
QueueHandle_t vidQueue;


//Seemingly, this will be called only once. Should call func with a freq of frequency,
int osd_installtimer(int frequency, void *func, int funcsize, void *counter, int countersize)
{
   return 0;
}


/*
** Audio
*/
static void (*audio_callback)(void *buffer, int length) = NULL;

static int16_t *audio_frame;

void do_audio_frame()
{
   audio_callback(audio_frame, AUDIO_FRAGSIZE); //get audio data

   //16 bit mono -> 32-bit (16 bit r+l)
   for (int i = AUDIO_FRAGSIZE - 1; i >= 0; --i)
   {
      int sample = (int)audio_frame[i];

      audio_frame[i*2] = (short)sample;
      audio_frame[i*2+1] = (short)sample;
   }

   odroid_audio_submit(audio_frame, AUDIO_FRAGSIZE);
}

void osd_setsound(void (*playfunc)(void *buffer, int length))
{
   //Indicates we should call playfunc() to get more data.
   audio_callback = playfunc;
}

static void osd_stopsound(void)
{
   audio_callback = NULL;
}


static int osd_init_sound(void)
{
   audio_frame = malloc(4 * AUDIO_FRAGSIZE);
	audio_callback = NULL;
	return 0;
}

void osd_getsoundinfo(sndinfo_t *info)
{
   info->sample_rate = AUDIO_SAMPLERATE;
   info->bps = 16;
}

/*
** Video
*/

static int init(int width, int height);
static void shutdown(void);
static int set_mode(int width, int height);
static void set_palette(rgb_t *pal);
static void clear(uint8 color);
static bitmap_t *lock_write(void);
static void free_write(int num_dirties, rect_t *dirty_rects);
static void custom_blit(bitmap_t *bmp, short interlace);
static char fb[1]; //dummy

viddriver_t sdlDriver =
{
   "Simple DirectMedia Layer",         /* name */
   init,          /* init */
   shutdown,      /* shutdown */
   set_mode,      /* set_mode */
   set_palette,   /* set_palette */
   clear,         /* clear */
   lock_write,    /* lock_write */
   free_write,    /* free_write */
   custom_blit,   /* custom_blit */
   false          /* invalidate flag */
};


bitmap_t *myBitmap;

void osd_getvideoinfo(vidinfo_t *info)
{
   info->default_width = DEFAULT_WIDTH;
   info->default_height = DEFAULT_HEIGHT;
   info->driver = &sdlDriver;
}

/* flip between full screen and windowed */
void osd_togglefullscreen(int code)
{
}

/* initialise video */
static int init(int width, int height)
{
	return 0;
}

static void shutdown(void)
{
}

/* set a video mode */
static int set_mode(int width, int height)
{
   return 0;
}

uint16 myPalette[64];

/* copy nes palette over to hardware */
static void set_palette(rgb_t *pal)
{
   for (int i = 0; i < 64; i++)
   {
      uint16_t c=(pal[i].b>>3)+((pal[i].g>>2)<<5)+((pal[i].r>>3)<<11);
      myPalette[i]=(c>>8)|((c&0xff)<<8);
      //myPalette[i]= c;
   }

}

/* clear all frames to a particular color */
static void clear(uint8 color)
{
//   SDL_FillRect(mySurface, 0, color);
}



/* acquire the directbuffer for writing */
static bitmap_t *lock_write(void)
{
//   SDL_LockSurface(mySurface);
   myBitmap = bmp_createhw((uint8*)fb, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_WIDTH*2);
   return myBitmap;
}

/* release the resource */
static void free_write(int num_dirties, rect_t *dirty_rects)
{
   bmp_destroy(&myBitmap);
}

static struct update_meta update1 = {0,};
static struct update_meta update2 = {0,};
static struct update_meta *update = &update2;
#define NES_VERTICAL_OVERDRAW (NES_SCREEN_HEIGHT-NES_VISIBLE_HEIGHT)
#define INTERLACE_THRESHOLD ((NES_SCREEN_WIDTH*NES_VISIBLE_HEIGHT)/2)

static void IRAM_ATTR custom_blit(bitmap_t *bmp, short interlace)
{
   if (!bmp) {
      printf("custom_blit called with NULL bitmap!\n");
      abort();
   }

   uint8_t *old_buffer = update->buffer;
   odroid_scanline *old_diff = update->diff;

   // Flip the update struct so we can keep track of the changes in the last
   // frame and fill in the new details (actually, these ought to always be
   // the same...)
   update = (update == &update1) ? &update2 : &update1;
   update->buffer = bmp->line[NES_VERTICAL_OVERDRAW/2];
   update->stride = bmp->pitch;

   // Note, the NES palette never changes during runtime and we assume that
   // there are no duplicate entries, so no need to pass the palette over to
   // the diff function.
   if (interlace >= 0) {
      odroid_buffer_diff_interlaced(update->buffer, old_buffer,
                                    NULL, NULL,
                                    NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT,
                                    update->stride, PIXEL_MASK, 0, interlace,
                                    update->diff, old_diff);
   } else {
      odroid_buffer_diff(update->buffer, old_buffer,
                         NULL, NULL,
                         NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT,
                         update->stride, PIXEL_MASK, 0, update->diff);
   }

   xQueueSend(vidQueue, &update, portMAX_DELAY);
}


//This runs on core 1.
volatile bool videoTaskIsRunning = false;
static void videoTask(void *arg)
{
    videoTaskIsRunning = true;

    int8_t previous_scaling_mode = ODROID_SCALING_UNKNOWN;

    while(1)
    {
        struct update_meta *update = NULL;
        xQueuePeek(vidQueue, &update, portMAX_DELAY);

        if (!update) break;

        bool scale_changed = (previous_scaling_mode != scaling_mode);
        if (scale_changed)
        {
           // Clear display
           ili9341_blank_screen();
           previous_scaling_mode = scaling_mode;
           if (scaling_mode) {
               float aspect = (scaling_mode == ODROID_SCALING_FILL) ? (8.f / 7.f) : 1.f;
               odroid_display_set_scale(NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT, aspect);
           } else {
               odroid_display_reset_scale(NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT);
           }
        }

        ili9341_write_frame_8bit(update->buffer,
                                 scale_changed ? NULL : update->diff,
                                 NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT,
                                 update->stride, PIXEL_MASK,
                                 myPalette);

        xQueueReceive(vidQueue, &update, portMAX_DELAY);
    }


    odroid_display_lock();
    odroid_display_show_hourglass();
    odroid_display_unlock();

    videoTaskIsRunning = false;

    vTaskDelete(NULL);

    while(1){}
}


/*
** Input
*/

void SaveState()
{
    printf("Saving state.\n");

    odroid_input_battery_monitor_enabled_set(0);
    odroid_system_led_set(1);

    save_sram();

    odroid_system_led_set(0);
    odroid_input_battery_monitor_enabled_set(1);

    printf("Saving state OK.\n");
}

void PowerDown()
{
    // Stop tasks
    printf("PowerDown: stopping tasks.\n");

    // Clear audio to prevent stuttering
    odroid_audio_terminate();

    void *exitVideoTask = NULL;
    xQueueSend(vidQueue, &exitVideoTask, portMAX_DELAY);
    while (videoTaskIsRunning) { vTaskDelay(10); }

    // state
    printf("PowerDown: Saving state.\n");
    SaveState();

    // LCD
    printf("PowerDown: Powerdown LCD panel.\n");
    ili9341_poweroff();

    printf("PowerDown: Entering deep sleep.\n");
    odroid_system_sleep();

    // Should never reach here
    abort();
}

void QuitEmulator(bool save)
{
   odroid_audio_terminate();

   ili9341_blank_screen();

   void *exitVideoTask = NULL;
   xQueueSend(vidQueue, &exitVideoTask, portMAX_DELAY);
   while (videoTaskIsRunning) { vTaskDelay(10); }

   if (save) {
      SaveState();
   }

   // Set menu application
   odroid_system_application_set(0);

   // Reset
   esp_restart();
}

void RedrawScreen()
{
   ili9341_write_frame_8bit(update->buffer,
                           NULL,
                           NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT,
                           update->stride, PIXEL_MASK,
                           myPalette);
}

extern nes_t* console_nes;
extern nes6502_context cpu;

void osd_getinput(void)
{
	const int event[16] = {
      event_joypad1_select,0,0,event_joypad1_start,event_joypad1_up,event_joypad1_right,event_joypad1_down,event_joypad1_left,
      0,0,0,0,event_soft_reset,event_joypad1_a,event_joypad1_b,event_hard_reset
	};

	event_t evh;
   static int previous = 0xffff;
   int b = 0;
   int changed = b ^ previous;

   odroid_gamepad_state joystick;
   odroid_input_gamepad_read(&joystick);

   if (joystick.values[ODROID_INPUT_MENU]) {
      odroid_overlay_game_menu();
      RedrawScreen();
   }
   else if (joystick.values[ODROID_INPUT_VOLUME]) {
      odroid_overlay_settings_menu(NULL, 0);
      RedrawScreen();
   }

	// A
	if (!joystick.values[ODROID_INPUT_A])
		b |= (1 << 13);

	// B
	if (!joystick.values[ODROID_INPUT_B])
		b |= (1 << 14);

	// select
	if (!joystick.values[ODROID_INPUT_SELECT])
		b |= (1 << 0);

	// start
	if (!joystick.values[ODROID_INPUT_START])
		b |= (1 << 3);

	// right
	if (!joystick.values[ODROID_INPUT_RIGHT])
		b |= (1 << 5);

	// left
	if (!joystick.values[ODROID_INPUT_LEFT])
		b |= (1 << 7);

	// up
	if (!joystick.values[ODROID_INPUT_UP])
		b |= (1 << 4);

	// down
	if (!joystick.values[ODROID_INPUT_DOWN])
		b |= (1 << 6);

   previous = b;

	for (int x = 0; x < 16; x++) {
		if (changed & 1) {
			evh = event_get(event[x]);
			if (evh) evh((b & 1) ? INP_STATE_BREAK : INP_STATE_MAKE);
		}
		changed >>= 1;
		b >>= 1;
	}
}

static void osd_freeinput(void)
{
}

void osd_getmouse(int *x, int *y, int *button)
{
}

/*
** Shutdown
*/

/* this is at the bottom, to eliminate warnings */
void osd_shutdown()
{
	osd_stopsound();
	osd_freeinput();
}

static int logprint(const char *string)
{
   return printf("%s", string);
}

/*
** Startup
*/
// Boot state overrides
bool forceConsoleReset = false;

int osd_init()
{
   log_chain_logfunc(logprint);

   osd_init_sound();

   scaling_mode = odroid_settings_Scaling_get();

   vidQueue = xQueueCreate(1, sizeof(struct update_meta *));
   xTaskCreatePinnedToCore(&videoTask, "videoTask", 2048, NULL, 5, NULL, 1);

   return 0;
}
