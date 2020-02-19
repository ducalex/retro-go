#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_heap_caps.h"
#include "odroid_system.h"

#include <string.h>
#include <stdint.h>
#include <noftypes.h>
#include <bitmap.h>
#include <nofconfig.h>
#include <event.h>
#include <gui.h>
#include <log.h>
#include <nes.h>
#include <nes_pal.h>
#include <nesinput.h>
#include <nes/nesstate.h>
#include <osd.h>
#include "sdkconfig.h"

#define AUDIO_SAMPLERATE   32000
#define AUDIO_FRAGSIZE     (AUDIO_SAMPLERATE/NES_REFRESH_RATE)

#define DEFAULT_WIDTH        NES_SCREEN_WIDTH
#define DEFAULT_HEIGHT       NES_VISIBLE_HEIGHT

#define NES_VERTICAL_OVERDRAW (NES_SCREEN_HEIGHT-NES_VISIBLE_HEIGHT)

#define PIXEL_MASK 0x3F

static char* romPath;
static char* romData;

static char fb[1];
static bitmap_t *myBitmap;
static uint16_t myPalette[64];

struct video_update {
    odroid_scanline diff[NES_VISIBLE_HEIGHT];
    uint8_t *buffer;
    uint16_t *palette;
    int stride;
};
static struct video_update update1 = {0,};
static struct video_update update2 = {0,};
static struct video_update *currentUpdate = &update1;

uint fullFrames = 0;

volatile QueueHandle_t videoTaskQueue;

extern nes_t* console_nes;
extern nes6502_context cpu;
// --- MAIN


/* File system interface */
void osd_fullname(char *fullname, const char *shortname)
{
   strncpy(fullname, shortname, PATH_MAX);
}

/* This gives filenames for storage of saves */
char *osd_newextension(char *string, char *ext)
{
   return string;
}

/* This gives filenames for storage of PCX snapshots */
int osd_makesnapname(char *filename, int len)
{
   return -1;
}

char *osd_getromdata()
{
	return (char*)romData;
}

// We use this well placed call to load our save game
int osd_installtimer(int frequency, void *func, int funcsize, void *counter, int countersize)
{
   return 0;
}

int osd_logprint(const char *string)
{
   return printf("%s", string);
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
      int16_t sample = audio_frame[i];
      audio_frame[i*2] = sample;
      audio_frame[i*2+1] = sample;
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

/* copy nes palette over to hardware */
static void set_palette(rgb_t *pal)
{
   for (int i = 0; i < 64; i++)
   {
      uint16_t c = (pal[i].b>>3) + ((pal[i].g>>2)<<5) + ((pal[i].r>>3)<<11);
      myPalette[i] = (c>>8) | ((c&0xff)<<8);
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

static void IRAM_ATTR custom_blit(bitmap_t *bmp, short interlace)
{
   if (!bmp) {
      printf("custom_blit called with NULL bitmap!\n");
      abort();
   }

   struct video_update *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;
   // Flip the update struct so we can keep track of the changes in the last
   // frame and fill in the new details (actually, these ought to always be
   // the same...)
   currentUpdate->buffer = bmp->line[NES_VERTICAL_OVERDRAW/2];
   currentUpdate->stride = bmp->pitch;

   // Note, the NES palette never changes during runtime and we assume that
   // there are no duplicate entries, so no need to pass the palette over to
   // the diff function.
   odroid_buffer_diff(currentUpdate->buffer, previousUpdate->buffer, NULL, NULL,
                      NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT, currentUpdate->stride,
                      1, PIXEL_MASK, 0, currentUpdate->diff);

   if (currentUpdate->diff[0].width && currentUpdate->diff[0].repeat == NES_VISIBLE_HEIGHT) {
      ++fullFrames;
   }

   xQueueSend(videoTaskQueue, &currentUpdate, portMAX_DELAY);

   currentUpdate = previousUpdate;
}

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

void osd_getvideoinfo(vidinfo_t *info)
{
   info->default_width = DEFAULT_WIDTH;
   info->default_height = DEFAULT_HEIGHT;
   info->driver = &sdlDriver;
}

static void videoTask(void *arg)
{
    videoTaskQueue = xQueueCreate(1, sizeof(void*));

    int8_t previousScalingMode = -1;
    struct video_update *update;

    while(1)
    {
        xQueuePeek(videoTaskQueue, &update, portMAX_DELAY);

        if (!update) break;

        bool redraw = previousScalingMode != scalingMode || forceRedraw;
        if (redraw)
        {
           ili9341_blank_screen();
           previousScalingMode = scalingMode;
           forceRedraw = false;

           if (scalingMode) {
               float aspect = (scalingMode == ODROID_SCALING_FILL) ? (8.f / 7.f) : 1.f;
               odroid_display_set_scale(NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT, aspect);
           } else {
               odroid_display_reset_scale(NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT);
           }
        }

        ili9341_write_frame_scaled(update->buffer, redraw ? NULL : update->diff,
                                   NES_SCREEN_WIDTH, NES_VISIBLE_HEIGHT, update->stride,
                                   1, PIXEL_MASK, myPalette);

        xQueueReceive(videoTaskQueue, &update, portMAX_DELAY);
    }

    videoTaskQueue = NULL;

    vTaskDelete(NULL);

    while (1) {}
}


/**
 * Input
 */
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
   }
   else if (joystick.values[ODROID_INPUT_VOLUME]) {
      odroid_overlay_game_settings_menu(NULL, 0);
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

void osd_getmouse(int *x, int *y, int *button)
{
}


/**
 * Init/Shutdown
 */

int osd_init()
{
   log_chain_logfunc(osd_logprint);

   osd_init_sound();

   xTaskCreatePinnedToCore(&videoTask, "videoTask", 2048, NULL, 5, NULL, 1);

   return 0;
}

int osd_main(int argc, char *argv[])
{
   config.filename = "n/a";

   return main_loop(argv[0], system_nes);
}

void osd_shutdown()
{
	osd_stopsound();
}


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

void QuitEmulator(bool save)
{
   printf("QuitEmulator: stopping tasks.\n");

   void *exitVideoTask = NULL;
   xQueueSend(videoTaskQueue, &exitVideoTask, portMAX_DELAY);
   while (videoTaskQueue) vTaskDelay(1);

   odroid_audio_terminate();
   ili9341_blank_screen();

   odroid_display_lock();
   odroid_display_show_hourglass();
   odroid_display_unlock();

   if (save) {
      printf("QuitEmulator: Saving state.\n");
      SaveState();
   }

   // Set menu application
   odroid_system_application_set(0);

   // Reset
   esp_restart();
}


void app_main(void)
{
	printf("nesemu (%s-%s).\n", COMPILEDATE, GITREV);

   odroid_system_init(2, 32000, &romPath);

   // Load ROM
   romData = heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
   if (!romData) abort();

   size_t fileSize = 0;

   if (strcasecmp(romPath + (strlen(romPath) - 4), ".zip") == 0)
   {
      printf("app_main ROM: Reading compressed file: %s\n", romPath);
      fileSize = odroid_sdcard_unzip_file_to_memory(romPath, romData, 1024 * 1024);
   }
   else
   {
      printf("app_main ROM: Reading file: %s\n", romPath);
      fileSize = odroid_sdcard_copy_file_to_memory(romPath, romData, 1024 * 1024);
   }

   printf("app_main ROM: fileSize=%d\n", fileSize);
   if (fileSize == 0)
   {
      odroid_display_show_error(ODROID_SD_ERR_BADFILE);
      odroid_system_halt();
   }

   printf("NoFrendo start!\n");
   char* args[1] = { strdup(odroid_sdcard_get_filename(romPath)) };
   nofrendo_main(1, args);

   printf("NoFrendo died.\n");
   abort();
}
