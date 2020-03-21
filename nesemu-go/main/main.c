#include <freertos/FreeRTOS.h>
#include "odroid_system.h"

#include <string.h>
#include <stdint.h>
#include <noftypes.h>
#include <bitmap.h>
#include <event.h>
#include <gui.h>
#include <log.h>
#include <nes.h>
#include <nesinput.h>
#include <nes/nesstate.h>
#include <osd.h>
#include "sdkconfig.h"

#define APP_ID 10

#define AUDIO_SAMPLE_RATE   32000

#define PIXEL_MASK 0x3F

#define NVS_KEY_LIMIT_SPRITES "limitspr"

static int8_t startAction;
static char* romPath;
static char* romData;
static size_t romSize;

static uint16_t myPalette[64];

static odroid_video_frame update1 = {NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 0, 1, PIXEL_MASK, NULL, myPalette, 0};
static odroid_video_frame update2 = {NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 0, 1, PIXEL_MASK, NULL, myPalette, 0};
static odroid_video_frame *currentUpdate = &update1;

static short overscan = -1;

uint fullFrames = 0;

static void (*audio_callback)(void *buffer, int length) = NULL;
static int16_t *audioBuffer;
// --- MAIN


bool SaveState(char *pathName)
{
   return state_save(pathName) >= 0;
}

bool LoadState(char *pathName)
{
   if (state_load(pathName) < 0)
   {
      nes_reset(HARD_RESET);
      return false;
   }
   return true;
}


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

size_t osd_getromdata(unsigned char **data)
{
	*data = (unsigned char*)romData;
   return romSize;
}

void osd_loadstate()
{
   if (startAction == ODROID_START_ACTION_RESUME)
   {
      odroid_system_load_state(0);
   }

   ppu_limitsprites(odroid_settings_int32_get(NVS_KEY_LIMIT_SPRITES, 1));
   ppu_setnpal(nes_getcontextptr()->ppu, odroid_settings_Palette_get());
}

int osd_logprint(const char *string)
{
   return printf("%s", string);
}


/*
** Audio
*/
void osd_audioframe(int audioSamples)
{
   audio_callback(audioBuffer, audioSamples); //get audio data

   //16 bit mono -> 32-bit (16 bit r+l)
   for (int i = audioSamples - 1; i >= 0; --i)
   {
      int16_t sample = audioBuffer[i];
      audioBuffer[i*2] = sample;
      audioBuffer[i*2+1] = sample;
   }

   odroid_audio_submit(audioBuffer, audioSamples);
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

void osd_getsoundinfo(sndinfo_t *info)
{
   info->sample_rate = AUDIO_SAMPLE_RATE;
   info->bps = 16;
}


/*
** Video
*/
void osd_setpalette(rgb_t *pal)
{
   for (int i = 0; i < 64; i++)
   {
      uint16_t c = (pal[i].b>>3) + ((pal[i].g>>2)<<5) + ((pal[i].r>>3)<<11);
      myPalette[i] = (c>>8) | ((c&0xff)<<8);
   }
   forceVideoRefresh = true;
}

void IRAM_ATTR osd_blitscreen(bitmap_t *bmp)
{
   if (overscan < 0)
   {
      overscan = nes_getcontextptr()->overscan;
      update1.height = update2.height = NES_SCREEN_HEIGHT - (overscan * 2);
   }

   odroid_video_frame *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

   currentUpdate->buffer = bmp->line[overscan];
   currentUpdate->stride = bmp->pitch;

   if (odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL)
   {
      ++fullFrames;
   }

   currentUpdate = previousUpdate;
}


static bool sprite_limit_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   int val = odroid_settings_int32_get(NVS_KEY_LIMIT_SPRITES, 1);

   if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
      val = val ? 0 : 1;
      odroid_settings_int32_set(NVS_KEY_LIMIT_SPRITES, val);
      ppu_limitsprites(val);
   }

   strcpy(option->value, val ? "On " : "Off");

   return event == ODROID_DIALOG_ENTER;
}

static bool region_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   int val = odroid_settings_Region_get();
   int max = 2;

   if (event == ODROID_DIALOG_PREV) val = val > 0 ? val - 1 : max;
   if (event == ODROID_DIALOG_NEXT) val = val < max ? val + 1 : 0;

   if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
      odroid_settings_Region_set(val);
   }

   if (val == ODROID_REGION_AUTO) strcpy(option->value, "Auto");
   if (val == ODROID_REGION_NTSC) strcpy(option->value, "NTSC");
   if (val == ODROID_REGION_PAL)  strcpy(option->value, "PAL ");

   return event == ODROID_DIALOG_ENTER;
}

static bool advanced_settings_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   if (event == ODROID_DIALOG_ENTER) {
      odroid_dialog_choice_t options[] = {
         {1, "Region", "Auto", 1, &region_update_cb},
         {2, "Sprite limit", "On ", 1, &sprite_limit_cb},
         // {4, "", "", 1, NULL},
         //{0, "Reset all", "", 1, NULL},
      };
      odroid_overlay_dialog("Advanced", options, sizeof(options) / sizeof(options[0]), 0);
   }
   return false;
}

static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   int pal = odroid_settings_Palette_get();
   int max = PPU_PAL_COUNT - 1;

   if (event == ODROID_DIALOG_PREV) pal = pal > 0 ? pal - 1 : max;
   if (event == ODROID_DIALOG_NEXT) pal = pal < max ? pal + 1 : 0;

   if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
      odroid_settings_Palette_set(pal);
      ppu_setnpal(nes_getcontextptr()->ppu, pal);
      // This is less than ideal, but it works for now
      odroid_display_unlock();
      odroid_display_queue_update(currentUpdate, NULL);
      odroid_display_queue_update(currentUpdate, NULL);
      odroid_display_lock();
   }

   sprintf(option->value, "%.7s", ppu_getnpal(pal)->name);
   return event == ODROID_DIALOG_ENTER;
}


void osd_getinput(void)
{
	const int events[8] = {
      event_joypad1_start, event_joypad1_select, event_joypad1_up, event_joypad1_right,
      event_joypad1_down, event_joypad1_left, event_joypad1_a, event_joypad1_b
	};

   static uint8 previous = 0xff;
   uint8 b = 0;
   uint8 changed = b ^ previous;

   odroid_gamepad_state joystick;
   odroid_input_gamepad_read(&joystick);

   if (joystick.values[ODROID_INPUT_MENU]) {
      odroid_overlay_game_menu();
   }
   else if (joystick.values[ODROID_INPUT_VOLUME]) {
      odroid_dialog_choice_t options[] = {
            {100, "Palette", "Default", 1, &palette_update_cb},
            // {101, "", "", -1, NULL},
            {101, "More...", "", 1, &advanced_settings_cb},
      };
      odroid_overlay_game_settings_menu(options, sizeof(options) / sizeof(options[0]));
   }

	if (!joystick.values[ODROID_INPUT_START])  b |= (1 << 0);
	if (!joystick.values[ODROID_INPUT_SELECT]) b |= (1 << 1);
	if (!joystick.values[ODROID_INPUT_UP])     b |= (1 << 2);
	if (!joystick.values[ODROID_INPUT_RIGHT])  b |= (1 << 3);
	if (!joystick.values[ODROID_INPUT_DOWN])   b |= (1 << 4);
	if (!joystick.values[ODROID_INPUT_LEFT])   b |= (1 << 5);
	if (!joystick.values[ODROID_INPUT_A])      b |= (1 << 6);
	if (!joystick.values[ODROID_INPUT_B])      b |= (1 << 7);

   previous = b;

	for (int x = 0; x < 8; x++) {
		if (changed & 1) {
         event_raise(events[x], (b & 1) ? INP_STATE_BREAK : INP_STATE_MAKE);
		}
		changed >>= 1;
		b >>= 1;
	}
}


/**
 * Init/Shutdown
 */

int osd_init()
{
   log_chain_logfunc(osd_logprint);

   return 0;
}

void osd_shutdown()
{
	osd_stopsound();
}


void app_main(void)
{
	printf("nesemu (%s-%s).\n", COMPILEDATE, GITREV);

   odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
   odroid_system_emu_init(&romPath, &startAction);

   audioBuffer = calloc(AUDIO_SAMPLE_RATE / 50, 4);
   assert(audioBuffer != NULL);

   // Load ROM
   romData = malloc(1024 * 1024);
   assert(romData != NULL);

   if (strcasecmp(romPath + (strlen(romPath) - 4), ".zip") == 0)
   {
      printf("app_main ROM: Reading compressed file: %s\n", romPath);
      romSize = odroid_sdcard_unzip_file_to_memory(romPath, romData, 1024 * 1024);
   }
   else
   {
      printf("app_main ROM: Reading file: %s\n", romPath);
      romSize = odroid_sdcard_copy_file_to_memory(romPath, romData, 1024 * 1024);
   }

   printf("app_main ROM: romSize=%d\n", romSize);
   if (romSize == 0)
   {
      odroid_system_panic("ROM read failed");
   }

   int region = NES_NTSC;
   switch(odroid_settings_Region_get())
   {
      case ODROID_REGION_AUTO: region = NES_AUTO; break;
      case ODROID_REGION_NTSC: region = NES_NTSC; break;
      case ODROID_REGION_PAL:  region = NES_PAL;  break;
   }

   printf("NoFrendo start!\n");
   nofrendo_start(romPath, region);

   printf("NoFrendo died.\n");
   abort();
}
