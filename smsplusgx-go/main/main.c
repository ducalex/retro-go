#include <rg_system.h>

#include "../components/smsplus/shared.h"

#define AUDIO_SAMPLE_RATE   (32000)

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];

static rg_app_t *app;

static uint32_t joystick1;
static uint32_t *localJoystick = &joystick1;

#ifdef RG_ENABLE_NETPLAY
static uint32_t joystick2;
static uint32_t *remoteJoystick = &joystick2;

static bool netplay = false;
#endif
// --- MAIN


static void event_handler(int event, void *arg)
{
#ifdef RG_ENABLE_NETPLAY
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

static bool screenshot_handler(const char *filename, int width, int height)
{
	return rg_display_save_frame(filename, currentUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
    FILE* f = fopen(filename, "w");
    if (f)
    {
        system_save_state(f);
        fclose(f);
        return true;
    }
    return false;
}

static bool load_state_handler(const char *filename)
{
    FILE* f = fopen(filename, "r");
    if (f)
    {
        system_load_state(f);
        fclose(f);
        return true;
    }
    system_reset();
    return false;
}

static bool reset_handler(bool hard)
{
    system_reset();
    return true;
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .event = &event_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, NULL);

    updates[0].buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);
    updates[1].buffer = rg_alloc(SMS_WIDTH * SMS_HEIGHT, MEM_FAST);

    system_reset_config();

    if (!load_rom(app->romPath))
    {
        RG_PANIC("ROM file loading failed!");
    }

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    bitmap.data = currentUpdate->buffer;

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_poweron();

    app->refreshRate = (sms.display == DISPLAY_NTSC) ? FPS_NTSC : FPS_PAL;

    updates[0].buffer += bitmap.viewport.x;
    updates[1].buffer += bitmap.viewport.x;

    rg_display_set_source_format(bitmap.viewport.w, bitmap.viewport.h, 0, 0, bitmap.pitch, RG_PIXEL_PAL565_BE);

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    int skipFrames = 0;
    int copyPalette = 0;

    while (true)
    {
        *localJoystick = rg_input_read_gamepad();

        if (*localJoystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
            if (*localJoystick & RG_KEY_MENU)
                rg_gui_game_menu();
            else
                rg_gui_options_menu();
            rg_audio_set_sample_rate(app->sampleRate * app->speed);
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = !skipFrames;
        bool fullFrame = true;

        input.pad[0] = 0x00;
        input.pad[1] = 0x00;
        input.system = 0x00;

        #ifdef RG_ENABLE_NETPLAY
        if (netplay)
        {
            rg_netplay_sync(localJoystick, remoteJoystick, sizeof(*localJoystick));

            uint32_t joystick = *remoteJoystick;

            if (joystick & RG_KEY_UP)    input.pad[1] |= INPUT_UP;
            if (joystick & RG_KEY_DOWN)  input.pad[1] |= INPUT_DOWN;
            if (joystick & RG_KEY_LEFT)  input.pad[1] |= INPUT_LEFT;
            if (joystick & RG_KEY_RIGHT) input.pad[1] |= INPUT_RIGHT;
            if (joystick & RG_KEY_A)     input.pad[1] |= INPUT_BUTTON2;
            if (joystick & RG_KEY_B)     input.pad[1] |= INPUT_BUTTON1;
            if (IS_SMS)
            {
                if (joystick & RG_KEY_START)  input.system |= INPUT_PAUSE;
                if (joystick & RG_KEY_SELECT) input.system |= INPUT_START;
            }
            else if (IS_GG)
            {
                if (joystick & RG_KEY_START)  input.system |= INPUT_START;
                if (joystick & RG_KEY_SELECT) input.system |= INPUT_PAUSE;
            }
        }
        #endif

        uint32_t joystick = *localJoystick;

        if (joystick & RG_KEY_UP)    input.pad[0] |= INPUT_UP;
        if (joystick & RG_KEY_DOWN)  input.pad[0] |= INPUT_DOWN;
        if (joystick & RG_KEY_LEFT)  input.pad[0] |= INPUT_LEFT;
        if (joystick & RG_KEY_RIGHT) input.pad[0] |= INPUT_RIGHT;
        if (joystick & RG_KEY_A)     input.pad[0] |= INPUT_BUTTON2;
        if (joystick & RG_KEY_B)     input.pad[0] |= INPUT_BUTTON1;

        if (IS_SMS)
        {
            if (joystick & RG_KEY_START)  input.system |= INPUT_PAUSE;
            if (joystick & RG_KEY_SELECT) input.system |= INPUT_START;
        }
        else if (IS_GG)
        {
            if (joystick & RG_KEY_START)  input.system |= INPUT_START;
            if (joystick & RG_KEY_SELECT) input.system |= INPUT_PAUSE;
        }
        else // Coleco
        {
            coleco.keypad[0] = 0xff;
            coleco.keypad[1] = 0xff;

            if (joystick & RG_KEY_SELECT)
            {
                rg_input_wait_for_key(RG_KEY_SELECT, false);
                system_reset();
            }

            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, *, #
            switch (cart.crc)
            {
                case 0x798002a2:    // Frogger
                case 0x32b95be0:    // Frogger
                case 0x9cc3fabc:    // Alcazar
                case 0x964db3bc:    // Fraction Fever
                    if (joystick & RG_KEY_START)
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                case 0x1796de5e:    // Boulder Dash
                case 0x5933ac18:    // Boulder Dash
                case 0x6e5c4b11:    // Boulder Dash
                    if (joystick & RG_KEY_START)
                    {
                        coleco.keypad[0] = 11; // #
                    }

                    if ((joystick & RG_KEY_START) && (joystick & RG_KEY_LEFT))
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
                case 0x109699e2:    // Dr. Seuss's Fix-Up The Mix-Up Puzzler
                case 0x614bb621:    // Decathlon
                    if (joystick & RG_KEY_START)
                    {
                        coleco.keypad[0] = 1;
                    }
                    if ((joystick & RG_KEY_START) && (joystick & RG_KEY_LEFT))
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                default:
                    if (joystick & RG_KEY_START)
                    {
                        coleco.keypad[0] = 1;
                    }
                    break;
            }
        }

        system_frame(!drawFrame);

        if (drawFrame)
        {
            rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
            if (render_copy_palette(currentUpdate->palette))
            {
                previousUpdate = NULL;
                copyPalette = true;
            }
            else if (copyPalette)
            {
                memcpy(currentUpdate->palette, previousUpdate->palette, 512);
                copyPalette = false;
            }
            fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_UPDATE_FULL;
            currentUpdate = &updates[currentUpdate == &updates[0]]; // Swap
            bitmap.data = currentUpdate->buffer - bitmap.viewport.x;
        }

        int elapsed = rg_system_timer() - startTime;

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (app->refreshRate * app->speed);
            if (elapsed > frameTime - 2000) // It takes about 2ms to copy the audio buffer
                skipFrames = (elapsed + frameTime / 2) / frameTime;
            else if (drawFrame && fullFrame) // This could be avoided when scaling != full
                skipFrames = 1;
            if (app->speed > 1.f) // This is a hack until we account for audio speed...
                skipFrames += (int)app->speed;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        rg_system_tick(elapsed);

        // The emulator's sound buffer isn't in a very convenient format, we must remix it.
        size_t sample_count = snd.sample_count;
        rg_audio_sample_t mixbuffer[sample_count];
        for (size_t i = 0; i < sample_count; i++)
        {
            mixbuffer[i].left = snd.stream[0][i] * 2.75f;
            mixbuffer[i].right = snd.stream[1][i] * 2.75f;
        }

        // Audio is used to pace emulation :)
        rg_audio_submit(mixbuffer, sample_count);
    }
}
