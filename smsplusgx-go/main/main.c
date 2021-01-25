#include <rg_system.h>

#include "../components/smsplus/shared.h"

#define APP_ID 30

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

static int16_t audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static uint16_t palettes[2][32];
static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static long refreshRate = 60;
static long skipFrames = 0;

static bool consoleIsGG = false;
static bool consoleIsSMS = false;

static gamepad_state_t joystick1;
static gamepad_state_t *localJoystick = &joystick1;

#ifdef ENABLE_NETPLAY
static gamepad_state_t joystick2;
static gamepad_state_t *remoteJoystick = &joystick2;

static bool netplay = false;
#endif
// --- MAIN


static void netplay_callback(netplay_event_t event, void *arg)
{
#ifdef ENABLE_NETPLAY
   bool new_netplay;

   switch (event)
   {
      case NETPLAY_EVENT_STATUS_CHANGED:
         new_netplay = (rg_netplay_status() == NETPLAY_STATUS_CONNECTED);

         if (netplay && !new_netplay)
         {
            rg_gui_alert("Netplay", "Connection lost!");
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

   if (netplay && rg_netplay_mode() == NETPLAY_MODE_GUEST)
   {
      localJoystick = &joystick2;
      remoteJoystick = &joystick1;
   }
   else
   {
      localJoystick = &joystick1;
      remoteJoystick = &joystick2;
   }
#endif
}

static bool SaveState(char *pathName)
{
    FILE* f = fopen(pathName, "w");
    if (f)
    {
        system_save_state(f);
        fclose(f);
        char *filename = rg_emu_get_path(EMU_PATH_SCREENSHOT, 0);
        if (filename)
        {
            rg_display_save_frame(filename, currentUpdate, 160.f / currentUpdate->width);
            rg_free(filename);
        }
        return true;
    }
    return false;
}

static bool LoadState(char *pathName)
{
    FILE* f = fopen(pathName, "r");
    if (f)
    {
        system_load_state(f);
        fclose(f);
        return true;
    }
    system_reset();
    return false;
}

void app_main(void)
{
    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(&LoadState, &SaveState, &netplay_callback);

    frames[0].flags = RG_PIXEL_PAL|RG_PIXEL_565|RG_PIXEL_BE;
    frames[0].pixel_mask = PIXEL_MASK;
    frames[1] = frames[0];

    frames[0].buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);
    frames[1].buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);

    frames[0].palette = (uint16_t*)&palettes[0];
    frames[1].palette = (uint16_t*)&palettes[1];

    // Load ROM
    rg_app_desc_t *app = rg_system_get_app();

    system_reset_config();

    if (!load_rom(app->romPath))
    {
        RG_PANIC("ROM file loading failed!");
    }

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    //bitmap.depth = 8;
    bitmap.data = currentUpdate->buffer;

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    consoleIsSMS = sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2;
    consoleIsGG  = sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS;
    refreshRate = sms.display == DISPLAY_NTSC ? 60 : 50;

    app->refreshRate = refreshRate;

    // if (consoleIsSMS) rg_system_set_app_id(APP_ID + 1);
    // if (consoleIsGG)  rg_system_set_app_id(APP_ID + 2);

    frames[0].width  = frames[1].width  = bitmap.viewport.w;
    frames[0].height = frames[1].height = bitmap.viewport.h;
    frames[0].stride  = frames[1].stride  = bitmap.pitch;
    frames[0].buffer += bitmap.viewport.x;
    frames[1].buffer += bitmap.viewport.x;

    if (app->startAction == EMU_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

    long frameTime = get_frame_time(refreshRate);
    bool fullFrame = false;

    while (true)
    {
        *localJoystick = rg_input_read_gamepad();

        if (localJoystick->values[GAMEPAD_KEY_MENU]) {
            rg_gui_game_menu();
        }
        else if (localJoystick->values[GAMEPAD_KEY_VOLUME]) {
            rg_gui_game_settings_menu(NULL);
        }

        int64_t startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        input.pad[0] = 0x00;
        input.pad[1] = 0x00;
        input.system = 0x00;

        #ifdef ENABLE_NETPLAY
        if (netplay)
        {
            rg_netplay_sync(localJoystick, remoteJoystick, sizeof(gamepad_state_t));
            if (remoteJoystick->values[GAMEPAD_KEY_UP])    input.pad[1] |= INPUT_UP;
            if (remoteJoystick->values[GAMEPAD_KEY_DOWN])  input.pad[1] |= INPUT_DOWN;
            if (remoteJoystick->values[GAMEPAD_KEY_LEFT])  input.pad[1] |= INPUT_LEFT;
            if (remoteJoystick->values[GAMEPAD_KEY_RIGHT]) input.pad[1] |= INPUT_RIGHT;
            if (remoteJoystick->values[GAMEPAD_KEY_A])     input.pad[1] |= INPUT_BUTTON2;
            if (remoteJoystick->values[GAMEPAD_KEY_B])     input.pad[1] |= INPUT_BUTTON1;
            if (consoleIsSMS)
            {
                if (remoteJoystick->values[GAMEPAD_KEY_START])  input.system |= INPUT_PAUSE;
                if (remoteJoystick->values[GAMEPAD_KEY_SELECT]) input.system |= INPUT_START;
            }
            else if (consoleIsGG)
            {
                if (remoteJoystick->values[GAMEPAD_KEY_START])  input.system |= INPUT_START;
                if (remoteJoystick->values[GAMEPAD_KEY_SELECT]) input.system |= INPUT_PAUSE;
            }
        }
        #endif

    	if (localJoystick->values[GAMEPAD_KEY_UP])    input.pad[0] |= INPUT_UP;
    	if (localJoystick->values[GAMEPAD_KEY_DOWN])  input.pad[0] |= INPUT_DOWN;
    	if (localJoystick->values[GAMEPAD_KEY_LEFT])  input.pad[0] |= INPUT_LEFT;
    	if (localJoystick->values[GAMEPAD_KEY_RIGHT]) input.pad[0] |= INPUT_RIGHT;
    	if (localJoystick->values[GAMEPAD_KEY_A])     input.pad[0] |= INPUT_BUTTON2;
    	if (localJoystick->values[GAMEPAD_KEY_B])     input.pad[0] |= INPUT_BUTTON1;

		if (consoleIsSMS)
		{
			if (localJoystick->values[GAMEPAD_KEY_START])  input.system |= INPUT_PAUSE;
			if (localJoystick->values[GAMEPAD_KEY_SELECT]) input.system |= INPUT_START;
		}
		else if (consoleIsGG)
		{
			if (localJoystick->values[GAMEPAD_KEY_START])  input.system |= INPUT_START;
			if (localJoystick->values[GAMEPAD_KEY_SELECT]) input.system |= INPUT_PAUSE;
		}
        else // Coleco
        {
            coleco.keypad[0] = 0xff;
            coleco.keypad[1] = 0xff;

            if (localJoystick->values[GAMEPAD_KEY_SELECT])
            {
                rg_input_wait_for_key(GAMEPAD_KEY_SELECT, false);
                system_reset();
            }

            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, *, #
            switch (cart.crc)
            {
                case 0x798002a2:    // Frogger
                case 0x32b95be0:    // Frogger
                case 0x9cc3fabc:    // Alcazar
                case 0x964db3bc:    // Fraction Fever
                    if (localJoystick->values[GAMEPAD_KEY_START])
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                case 0x1796de5e:    // Boulder Dash
                case 0x5933ac18:    // Boulder Dash
                case 0x6e5c4b11:    // Boulder Dash
                    if (localJoystick->values[GAMEPAD_KEY_START])
                    {
                        coleco.keypad[0] = 11; // #
                    }

                    if (localJoystick->values[GAMEPAD_KEY_START] &&
                        localJoystick->values[GAMEPAD_KEY_LEFT])
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
                case 0x109699e2:    // Dr. Seuss's Fix-Up The Mix-Up Puzzler
                case 0x614bb621:    // Decathlon
                    if (localJoystick->values[GAMEPAD_KEY_START])
                    {
                        coleco.keypad[0] = 1;
                    }
                    if (localJoystick->values[GAMEPAD_KEY_START] &&
                        localJoystick->values[GAMEPAD_KEY_LEFT])
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                default:
                    if (localJoystick->values[GAMEPAD_KEY_START])
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
            }
        }

        system_frame(!drawFrame);

        if (drawFrame)
        {
            rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];

            if (render_copy_palette(currentUpdate->palette))
                fullFrame = rg_display_queue_update(currentUpdate, NULL);
            else
                fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_SCREEN_UPDATE_FULL;

            // Swap buffers
            currentUpdate = previousUpdate;
            bitmap.data = currentUpdate->buffer - bitmap.viewport.x;
        }

        long elapsed = get_elapsed_time_since(startTime);

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            if (app->speedupEnabled)
                skipFrames = app->speedupEnabled * 2.5;
            else if (elapsed >= frameTime) // Frame took too long
                skipFrames = 1;
            else if (drawFrame && fullFrame) // This could be avoided when scaling != full
                skipFrames = 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        rg_system_tick(!drawFrame, fullFrame, elapsed);

        if (!app->speedupEnabled)
        {
            size_t length = snd.sample_count;
            for (size_t i = 0, out = 0; i < length; i++, out += 2)
            {
                audioBuffer[out] = snd.stream[0][i] * 2.75f;
                audioBuffer[out + 1] = snd.stream[1][i] * 2.75f;
            }
            rg_audio_submit(audioBuffer, length);
        }
    }
}
