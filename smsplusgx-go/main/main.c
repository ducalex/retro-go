#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"

#include "../components/smsplus/shared.h"
#include "odroid_system.h"

#define APP_ID 30

#define AUDIO_SAMPLE_RATE (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

static int8_t startAction;
static char* romPath;

static uint32_t* audioBuffer;

static odroid_video_frame update1;
static odroid_video_frame update2;
static odroid_video_frame *currentUpdate = &update1;

static uint totalElapsedTime = 0;
static uint emulatedFrames = 0;
static uint skippedFrames = 0;
static uint fullFrames = 0;

static bool skipFrame = false;

static bool consoleIsGG = false;
static bool consoleIsSMS = false;

static odroid_gamepad_state joystick1;
static odroid_gamepad_state joystick2;
static odroid_gamepad_state *localJoystick = &joystick1;
static odroid_gamepad_state *remoteJoystick = &joystick2;

// --- MAIN


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

void system_manage_sram(uint8 *sram, int slot, int mode)
{
    // printf("system_manage_sram\n");
    // sram_load();
}

void app_main(void)
{
    printf("smsplusgx (%s-%s).\n", COMPILEDATE, GITREV);

    // Do before odroid_system_init to make sure we get the caps requested
    update1.buffer = heap_caps_malloc(SMS_WIDTH * SMS_HEIGHT, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    update2.buffer = heap_caps_malloc(SMS_WIDTH * SMS_HEIGHT, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    audioBuffer     = heap_caps_calloc(AUDIO_BUFFER_LENGTH * 2, 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);

    // Init all the console hardware
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&romPath, &startAction, &LoadState, &SaveState);

    // Do the check after screen init so we can display an error
    if (!update1.buffer || !update2.buffer || !audioBuffer)
    {
        odroid_system_panic("Buffer allocation failed.");
    }

    // Load ROM
    load_rom(romPath);

    sms.use_fm = 0;

// #if 1
// 	sms.country = TYPE_OVERSEAS;
// #else
//     sms.country = TYPE_DOMESTIC;
// #endif

	//sms.dummy = framebuffer[0]; //A normal cart shouldn't access this memory ever. Point it to vram just in case.
	// sms.sram = malloc(SRAM_SIZE);
    // if (!sms.sram)
    //     abort();
    //
    // memset(sms.sram, 0xff, SRAM_SIZE);

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    //bitmap.depth = 8;
    bitmap.data = update1.buffer;

    set_option_defaults();

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    consoleIsSMS = sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2;
    consoleIsGG  = sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS;

    // if (consoleIsSMS) odroid_system_set_app_id(APP_ID + 1);
    // if (consoleIsGG)  odroid_system_set_app_id(APP_ID + 2);

    if (startAction == ODROID_START_ACTION_RESUME)
    {
        odroid_system_load_state(0);
    }

    update1.width  = update2.width  = bitmap.viewport.w;
    update1.height = update2.height = bitmap.viewport.h;
    update1.stride  = update2.stride  = bitmap.pitch;
    update1.pixel_size = update2.pixel_size = 1;
    update1.pixel_mask = update2.pixel_mask = PIXEL_MASK;
    update1.pal_shift_mask = update2.pal_shift_mask = PAL_SHIFT_MASK;
    update1.palette = malloc(64); update2.palette = malloc(64);
    update1.buffer += bitmap.viewport.x;
    update2.buffer += bitmap.viewport.x;

    uint8_t refresh = (sms.display == DISPLAY_NTSC) ? 60 : 50;
    const int frameTime = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000 / refresh;

    while (true)
    {
        odroid_input_gamepad_read(localJoystick);

        if (localJoystick->values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (localJoystick->values[ODROID_INPUT_VOLUME]) {
            odroid_overlay_game_settings_menu(NULL);
        }

        uint startTime = xthal_get_ccount();

        input.pad[0] = 0;
        input.system = 0;

    	if (localJoystick->values[ODROID_INPUT_UP])    input.pad[0] |= INPUT_UP;
    	if (localJoystick->values[ODROID_INPUT_DOWN])  input.pad[0] |= INPUT_DOWN;
    	if (localJoystick->values[ODROID_INPUT_LEFT])  input.pad[0] |= INPUT_LEFT;
    	if (localJoystick->values[ODROID_INPUT_RIGHT]) input.pad[0] |= INPUT_RIGHT;
    	if (localJoystick->values[ODROID_INPUT_A])     input.pad[0] |= INPUT_BUTTON2;
    	if (localJoystick->values[ODROID_INPUT_B])     input.pad[0] |= INPUT_BUTTON1;

		if (consoleIsSMS)
		{
			if (localJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_PAUSE;
			if (localJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_START;
		}
		else if (consoleIsGG)
		{
			if (localJoystick->values[ODROID_INPUT_START])  input.system |= INPUT_START;
			if (localJoystick->values[ODROID_INPUT_SELECT]) input.system |= INPUT_PAUSE;
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

        system_frame(skipFrame);

        if (skipFrame)
        {
            ++skippedFrames;
        }
        else
        {
            odroid_video_frame *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

            render_copy_palette(currentUpdate->palette);

            if (odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL)
            {
                ++fullFrames;
            }

            // Swap buffers
            currentUpdate = previousUpdate;
            bitmap.data = currentUpdate->buffer - bitmap.viewport.x;
        }

        if (speedupEnabled)
        {
            skipFrame = emulatedFrames % (2 + speedupEnabled);
            snd.enabled = speedupEnabled < 2;
        }
        else
        {
            snd.enabled = true;

            // See if we need to skip a frame to keep up
            skipFrame = (!skipFrame && get_elapsed_time_since(startTime) > frameTime);

            // Process audio
            for (short i = 0; i < snd.sample_count; i++)
            {
                audioBuffer[i] = snd.output[0][i] << 16 | snd.output[1][i];
            }

            odroid_audio_submit((short*)audioBuffer, snd.sample_count);
        }


        totalElapsedTime += get_elapsed_time_since(startTime);
        ++emulatedFrames;

        if (emulatedFrames == refresh)
        {
            float seconds = totalElapsedTime / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000.0f);
            float fps = emulatedFrames / seconds;

            odroid_battery_state battery;
            odroid_input_battery_level_read(&battery);

            printf("HEAP:%d, FPS:%f, SKIP:%d, FULL:%d, BATTERY:%d [%d]\n",
                esp_get_free_heap_size() / 1024, fps, skippedFrames, fullFrames,
                battery.millivolts, battery.percentage);

            emulatedFrames = 0;
            skippedFrames = 0;
            fullFrames = 0;
            totalElapsedTime = 0;
        }
    }
}
