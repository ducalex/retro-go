#include <rg_system.h>
#include <string.h>
#include <nofrendo.h>
#include <bitmap.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nes_input.h>
#include <osd.h>

#define APP_ID 10

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

#define NVS_KEY_AUTOCROP "autocrop"

static char* romData;
static uint romCRC32;
static size_t romSize;

static uint16_t myPalette[64];
static odroid_video_frame_t update1 = {NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 0, 1, 0x3F, -1, NULL, myPalette, 0, {}};
static odroid_video_frame_t update2 = {NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, 0, 1, 0x3F, -1, NULL, myPalette, 0, {}};
static odroid_video_frame_t *currentUpdate = &update1;
static odroid_video_frame_t *previousUpdate = NULL;

static int16_t audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static gamepad_state_t joystick1;
static gamepad_state_t joystick2;
static gamepad_state_t *localJoystick = &joystick1;
static gamepad_state_t *remoteJoystick = &joystick2;

static uint samplesPerFrame = 0;

static bool overscan = true;
static uint autocrop = false;
static bool netplay  = false;

static bool fullFrame = 0;
static uint frameTime = 0;
static nes_t *nes;

static rg_app_desc_t *app;
// --- MAIN


static void netplay_callback(netplay_event_t event, void *arg)
{
   bool new_netplay;

   switch (event)
   {
      case NETPLAY_EVENT_STATUS_CHANGED:
         new_netplay = (odroid_netplay_status() == NETPLAY_STATUS_CONNECTED);

         if (netplay && !new_netplay)
         {
            odroid_overlay_alert("Connection lost!");
         }
         else if (!netplay && new_netplay)
         {
            // displayScalingMode = RG_DISPLAY_SCALING_FILL;
            // displayFilterMode = RG_DISPLAY_FILTER_NONE;
            // forceVideoRefresh = true;
            nes_reset(ZERO_RESET);
         }

         netplay = new_netplay;
         break;

      default:
         break;
   }

   if (netplay && odroid_netplay_mode() == NETPLAY_MODE_GUEST)
   {
      localJoystick = &joystick2;
      remoteJoystick = &joystick1;
   }
   else
   {
      localJoystick = &joystick1;
      remoteJoystick = &joystick2;
   }
}


static bool SaveState(char *pathName)
{
   return state_save(pathName) >= 0;
}

static bool LoadState(char *pathName)
{
   if (state_load(pathName) < 0)
   {
      nes_reset(HARD_RESET);
      return false;
   }
   return true;
}


static bool sprite_limit_cb(dialog_choice_t *option, dialog_event_t event)
{
   int val = odroid_settings_SpriteLimit_get();

   if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
      val = val ? 0 : 1;
      odroid_settings_SpriteLimit_set(val);
      ppu_setopt(PPU_LIMIT_SPRITES, val);
   }

   strcpy(option->value, val ? "On " : "Off");

   return event == RG_DIALOG_ENTER;
}

static bool overscan_update_cb(dialog_choice_t *option, dialog_event_t event)
{
   if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
      overscan = !overscan;
      odroid_settings_DisplayOverscan_set(overscan);
   }

   strcpy(option->value, overscan ? "Auto" : "Off ");

   return event == RG_DIALOG_ENTER;
}

static bool autocrop_update_cb(dialog_choice_t *option, dialog_event_t event)
{
   int val = autocrop;
   int max = 2;

   if (event == RG_DIALOG_PREV) val = val > 0 ? val - 1 : max;
   if (event == RG_DIALOG_NEXT) val = val < max ? val + 1 : 0;

   if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
      autocrop = val;
      odroid_settings_app_int32_set(NVS_KEY_AUTOCROP, autocrop);
   }

   if (val == 0) strcpy(option->value, "Never ");
   if (val == 1) strcpy(option->value, "Auto  ");
   if (val == 2) strcpy(option->value, "Always");

   return event == RG_DIALOG_ENTER;
}

static bool region_update_cb(dialog_choice_t *option, dialog_event_t event)
{
   int val = odroid_settings_Region_get();
   int max = 2;

   if (event == RG_DIALOG_PREV) val = val > 0 ? val - 1 : max;
   if (event == RG_DIALOG_NEXT) val = val < max ? val + 1 : 0;

   if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
      odroid_settings_Region_set(val);
   }

   if (val == EMU_REGION_AUTO) strcpy(option->value, "Auto");
   if (val == EMU_REGION_NTSC) strcpy(option->value, "NTSC");
   if (val == EMU_REGION_PAL)  strcpy(option->value, "PAL ");

   return event == RG_DIALOG_ENTER;
}

static bool advanced_settings_cb(dialog_choice_t *option, dialog_event_t event)
{
   if (event == RG_DIALOG_ENTER) {
      dialog_choice_t options[] = {
         {1, "Region      ", "Auto ", 1, &region_update_cb},
         {2, "Overscan    ", "Auto ", 1, &overscan_update_cb},
         {3, "Crop sides  ", "Never", 1, &autocrop_update_cb},
         {4, "Sprite limit", "On   ", 1, &sprite_limit_cb},
         RG_DIALOG_CHOICE_LAST
      };
      odroid_overlay_dialog("Advanced", options, 0);
   }
   return false;
}

static bool palette_update_cb(dialog_choice_t *option, dialog_event_t event)
{
   int pal = ppu_getopt(PPU_PALETTE_RGB);
   int max = PPU_PAL_COUNT - 1;

   if (event == RG_DIALOG_PREV) pal = pal > 0 ? pal - 1 : max;
   if (event == RG_DIALOG_NEXT) pal = pal < max ? pal + 1 : 0;

   if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
      odroid_settings_Palette_set(pal);
      ppu_setopt(PPU_PALETTE_RGB, pal);
      odroid_display_queue_update(currentUpdate, NULL);
      odroid_display_queue_update(currentUpdate, NULL);
   }

   sprintf(option->value, "%.7s", ppu_getpalette(pal)->name);
   return event == RG_DIALOG_ENTER;
}

size_t osd_getromdata(unsigned char **data)
{
	*data = (unsigned char*)romData;
   return romSize;
}

uint osd_getromcrc()
{
   return romCRC32;
}

void osd_loadstate()
{
   if (odroid_system_get_app()->startAction == ODROID_START_ACTION_RESUME)
   {
      odroid_system_emu_load_state(0);
   }

   ppu_setopt(PPU_LIMIT_SPRITES, odroid_settings_SpriteLimit_get());
   ppu_setopt(PPU_PALETTE_RGB, odroid_settings_Palette_get());
   overscan = odroid_settings_DisplayOverscan_get();
   autocrop = odroid_settings_app_int32_get(NVS_KEY_AUTOCROP, 0);

   nes = nes_getptr();
   frameTime = get_frame_time(nes->refresh_rate);
   samplesPerFrame = AUDIO_SAMPLE_RATE / nes->refresh_rate;
}

void osd_logprint(int type, char *string)
{
   printf("%s", string);
}

int osd_init()
{
   input_connect(INP_JOYPAD0);
   input_connect(INP_JOYPAD1);
   return 0;
}

void osd_shutdown()
{
   //
}

// We've reached vsync. We need to process audio and sleep if we ran too fast
void osd_vsync()
{
   static uint skipFrames = 0;
   static uint lastSyncTime = 0;

   uint elapsed = get_elapsed_time_since(lastSyncTime);

   if (skipFrames == 0)
   {
      if (elapsed > frameTime) skipFrames = 1;
      if (app->speedupEnabled) skipFrames += app->speedupEnabled * 2;
   }
   else if (skipFrames > 0)
   {
      skipFrames--;
   }

   // Tick before submitting audio/syncing
   odroid_system_tick(!nes->drawframe, fullFrame, elapsed);

   nes->drawframe = (skipFrames == 0);

   // Use audio to throttle emulation
   if (!app->speedupEnabled)
   {
      apu_process(audioBuffer, samplesPerFrame, true);
      odroid_audio_submit(audioBuffer, samplesPerFrame);
   }

   lastSyncTime = get_elapsed_time();
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
   odroid_display_force_refresh();
}

void osd_blitscreen(bitmap_t *bmp)
{
   int crop_v = (overscan) ? nes->overscan : 0;
   int crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;

   currentUpdate->buffer = bmp->line[crop_v] + crop_h;
   currentUpdate->stride = bmp->pitch;
   currentUpdate->width  = bmp->width - (crop_h * 2);
   currentUpdate->height = bmp->height - (crop_v * 2);

   fullFrame = odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL;

   previousUpdate = currentUpdate;
   currentUpdate = (currentUpdate == &update1) ? &update2 : &update1;
}

void osd_getinput(void)
{
   uint16 pad0 = 0, pad1 = 0;

   *localJoystick = odroid_input_read_gamepad();

   if (localJoystick->values[GAMEPAD_KEY_MENU])
   {
      odroid_overlay_game_menu();
   }
   else if (localJoystick->values[GAMEPAD_KEY_VOLUME])
   {
      dialog_choice_t options[] = {
            {100, "Palette", "Default", 1, &palette_update_cb},
            {101, "More...", "", 1, &advanced_settings_cb},
            RG_DIALOG_CHOICE_LAST
      };
      odroid_overlay_game_settings_menu(options);
   }

   if (netplay)
   {
      odroid_netplay_sync(localJoystick, remoteJoystick, sizeof(gamepad_state_t));
      if (joystick2.values[GAMEPAD_KEY_START])  pad1 |= INP_PAD_START;
      if (joystick2.values[GAMEPAD_KEY_SELECT]) pad1 |= INP_PAD_SELECT;
      if (joystick2.values[GAMEPAD_KEY_UP])     pad1 |= INP_PAD_UP;
      if (joystick2.values[GAMEPAD_KEY_RIGHT])  pad1 |= INP_PAD_RIGHT;
      if (joystick2.values[GAMEPAD_KEY_DOWN])   pad1 |= INP_PAD_DOWN;
      if (joystick2.values[GAMEPAD_KEY_LEFT])   pad1 |= INP_PAD_LEFT;
      if (joystick2.values[GAMEPAD_KEY_A])      pad1 |= INP_PAD_A;
      if (joystick2.values[GAMEPAD_KEY_B])      pad1 |= INP_PAD_B;
      input_update(INP_JOYPAD1, pad1);
   }

	if (joystick1.values[GAMEPAD_KEY_START])  pad0 |= INP_PAD_START;
	if (joystick1.values[GAMEPAD_KEY_SELECT]) pad0 |= INP_PAD_SELECT;
	if (joystick1.values[GAMEPAD_KEY_UP])     pad0 |= INP_PAD_UP;
	if (joystick1.values[GAMEPAD_KEY_RIGHT])  pad0 |= INP_PAD_RIGHT;
	if (joystick1.values[GAMEPAD_KEY_DOWN])   pad0 |= INP_PAD_DOWN;
	if (joystick1.values[GAMEPAD_KEY_LEFT])   pad0 |= INP_PAD_LEFT;
	if (joystick1.values[GAMEPAD_KEY_A])      pad0 |= INP_PAD_A;
	if (joystick1.values[GAMEPAD_KEY_B])      pad0 |= INP_PAD_B;

   input_update(INP_JOYPAD0, pad0);
}


void app_main(void)
{
   heap_caps_malloc_extmem_enable(64 * 1024);
   odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
   odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

   app = odroid_system_get_app();

   romData = rg_alloc(0x200000, MEM_ANY);

   const char *romPath = app->romPath;

   // Load ROM
   if (strcasecmp(romPath + (strlen(romPath) - 4), ".zip") == 0)
   {
      printf("app_main ROM: Reading compressed file: %s\n", romPath);
      romSize = odroid_sdcard_unzip_file(romPath, romData, 0x200000);
   }
   else
   {
      printf("app_main ROM: Reading file: %s\n", romPath);
      romSize = odroid_sdcard_read_file(romPath, romData, 0x200000);
   }

   printf("app_main ROM: romSize=%d\n", romSize);
   if (romSize <= 0)
   {
      RG_PANIC("ROM file loading failed!");
   }

   romCRC32 = crc32_le(0, (const uint8_t*)(romData + 16), romSize - 16);

   int region, ret;

   switch(odroid_settings_Region_get())
   {
      case EMU_REGION_AUTO: region = NES_AUTO; break;
      case EMU_REGION_NTSC: region = NES_NTSC; break;
      case EMU_REGION_PAL:  region = NES_PAL;  break;
      default: region = NES_NTSC; break;
   }

   printf("Nofrendo start!\n");

   ret = nofrendo_start(romPath, region, AUDIO_SAMPLE_RATE);

   switch (ret)
   {
      case -1: RG_PANIC("Init failed.");
      case -2: RG_PANIC("Unsupported ROM.");
      default: RG_PANIC("Nofrendo died!");
   }
}
