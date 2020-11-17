#include <odroid_system.h>

#include "../components/smsplus/shared.h"

#define APP_ID 30

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

static uint32_t audioBuffer[AUDIO_BUFFER_LENGTH];

static uint16_t palettes[2][32];
static odroid_video_frame_t update1;
static odroid_video_frame_t update2;
static odroid_video_frame_t *currentUpdate = &update1;

static uint skipFrames = 0;

static bool netplay = false;

static bool consoleIsGG = false;
static bool consoleIsSMS = false;

static odroid_gamepad_state_t joystick1;
static odroid_gamepad_state_t joystick2;
static odroid_gamepad_state_t *localJoystick = &joystick1;
static odroid_gamepad_state_t *remoteJoystick = &joystick2;

// --- MAIN

#if 0
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
            system_reset();
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
#endif

static bool SaveState(char *pathName)
{
    FILE* f = fopen(pathName, "w");
    if (f == NULL)
        return false;

    system_save_state(f);
    fclose(f);

    return true;
}

static bool LoadState(char *pathName)
{
    FILE* f = fopen(pathName, "r");
    if (f == NULL)
    {
        system_reset();
        return false;
    }

    system_load_state(f);
    fclose(f);

    return true;
}

void app_main(void)
{
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    update1.buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);
    update2.buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);

    // Load ROM
    rg_app_desc_t *app = odroid_system_get_app();

    load_rom(app->romPath);

    system_reset_config();

    sms.use_fm = 0;

	// sms.dummy = framebuffer[0]; //A normal cart shouldn't access this memory ever. Point it to vram just in case.
	// sms.sram = malloc(SRAM_SIZE);

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    //bitmap.depth = 8;
    bitmap.data = update1.buffer;

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    consoleIsSMS = sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2;
    consoleIsGG  = sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS;

    // if (consoleIsSMS) odroid_system_set_app_id(APP_ID + 1);
    // if (consoleIsGG)  odroid_system_set_app_id(APP_ID + 2);

    if (app->startAction == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    update1.width  = update2.width  = bitmap.viewport.w;
    update1.height = update2.height = bitmap.viewport.h;
    update1.stride  = update2.stride  = bitmap.pitch;
    update1.pixel_size = update2.pixel_size = 1;
    update1.pixel_mask = update2.pixel_mask = PIXEL_MASK;
    update1.pixel_clear = update2.pixel_clear = -1;
    update1.pal_shift_mask = update2.pal_shift_mask = PAL_SHIFT_MASK;
    update1.palette = (uint16_t*)&palettes[0];
    update2.palette = (uint16_t*)&palettes[1];
    update1.buffer += bitmap.viewport.x;
    update2.buffer += bitmap.viewport.x;

    const int refresh_rate = (sms.display == DISPLAY_NTSC) ? FPS_NTSC : FPS_PAL;
    const int frameTime = get_frame_time(refresh_rate);
    bool fullFrame = false;

    while (true)
    {
        odroid_input_read_gamepad(localJoystick);

        if (localJoystick->values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (localJoystick->values[ODROID_INPUT_VOLUME]) {
            odroid_overlay_game_settings_menu(NULL);
        }

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        if (netplay)
        {
            odroid_netplay_sync(localJoystick, remoteJoystick, sizeof(odroid_gamepad_state_t));
        }

        input.pad[0] = 0x00;
        input.pad[1] = 0x00;
        input.system = 0x00;

    	if (localJoystick->values[ODROID_INPUT_UP])    input.pad[0] |= INPUT_UP;
    	if (localJoystick->values[ODROID_INPUT_DOWN])  input.pad[0] |= INPUT_DOWN;
    	if (localJoystick->values[ODROID_INPUT_LEFT])  input.pad[0] |= INPUT_LEFT;
    	if (localJoystick->values[ODROID_INPUT_RIGHT]) input.pad[0] |= INPUT_RIGHT;
    	if (localJoystick->values[ODROID_INPUT_A])     input.pad[0] |= INPUT_BUTTON2;
    	if (localJoystick->values[ODROID_INPUT_B])     input.pad[0] |= INPUT_BUTTON1;
    	if (remoteJoystick->values[ODROID_INPUT_UP])    input.pad[1] |= INPUT_UP;
    	if (remoteJoystick->values[ODROID_INPUT_DOWN])  input.pad[1] |= INPUT_DOWN;
    	if (remoteJoystick->values[ODROID_INPUT_LEFT])  input.pad[1] |= INPUT_LEFT;
    	if (remoteJoystick->values[ODROID_INPUT_RIGHT]) input.pad[1] |= INPUT_RIGHT;
    	if (remoteJoystick->values[ODROID_INPUT_A])     input.pad[1] |= INPUT_BUTTON2;
    	if (remoteJoystick->values[ODROID_INPUT_B])     input.pad[1] |= INPUT_BUTTON1;

		if (consoleIsSMS)
		{
			if (localJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_PAUSE;
			if (localJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_START;
			if (remoteJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_PAUSE;
			if (remoteJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_START;
		}
		else if (consoleIsGG)
		{
			if (localJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_START;
			if (localJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_PAUSE;
			if (remoteJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_START;
			if (remoteJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_PAUSE;
		}
        else // Coleco
        {
            coleco.keypad[0] = 0xff;
            coleco.keypad[1] = 0xff;

            if (localJoystick->values[ODROID_INPUT_SELECT])
            {
                odroid_input_wait_for_key(ODROID_INPUT_SELECT, false);
                system_reset();
            }

            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, *, #
            switch (cart.crc)
            {
                case 0x798002a2:    // Frogger
                case 0x32b95be0:    // Frogger
                case 0x9cc3fabc:    // Alcazar
                case 0x964db3bc:    // Fraction Fever
                    if (localJoystick->values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                case 0x1796de5e:    // Boulder Dash
                case 0x5933ac18:    // Boulder Dash
                case 0x6e5c4b11:    // Boulder Dash
                    if (localJoystick->values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 11; // #
                    }

                    if (localJoystick->values[ODROID_INPUT_START] &&
                        localJoystick->values[ODROID_INPUT_LEFT])
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
                case 0x109699e2:    // Dr. Seuss's Fix-Up The Mix-Up Puzzler
                case 0x614bb621:    // Decathlon
                    if (localJoystick->values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 1;
                    }
                    if (localJoystick->values[ODROID_INPUT_START] &&
                        localJoystick->values[ODROID_INPUT_LEFT])
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                default:
                    if (localJoystick->values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
            }
        }

        system_frame(!drawFrame);

        if (drawFrame)
        {
            odroid_video_frame_t *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

            render_copy_palette(currentUpdate->palette);

            fullFrame = odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL;

            // Swap buffers
            currentUpdate = previousUpdate;
            bitmap.data = currentUpdate->buffer - bitmap.viewport.x;
        }

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
            if (app->speedupEnabled) skipFrames += app->speedupEnabled * 2.5;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (!app->speedupEnabled)
        {
            // Process audio
            for (short i = 0; i < snd.sample_count; i++)
            {
                audioBuffer[i] = snd.output[0][i] << 16 | snd.output[1][i];
            }

            odroid_audio_submit((short*)audioBuffer, snd.sample_count);
        }
    }
}
