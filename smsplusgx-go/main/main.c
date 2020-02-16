#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"

#include "../components/smsplus/shared.h"
#include "odroid_system.h"

#define AUDIO_SAMPLE_RATE (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)


#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

static ODROID_START_ACTION startAction = 0;
static char* romPath = NULL;

uint32_t* audioBuffer;

uint8_t* framebuffers[2];
int currentBuffer = 0;

struct video_update {
    odroid_scanline diff[SMS_HEIGHT];
    uint8_t *buffer;
    uint16_t palette[PALETTE_SIZE];
    short width;
    short height;
    short stride;
};
static struct video_update update1 = {0,};
static struct video_update update2 = {0,};
static struct video_update *currentUpdate = &update1;

int8_t scaling_mode = ODROID_SCALING_FILL;
bool force_redraw = false;
bool speedup_enabled = false;
bool skipFrame = false;

QueueHandle_t videoQueue;
// --- MAIN


volatile bool videoTaskIsRunning = false;
static void videoTask(void *arg)
{
    videoTaskIsRunning = true;

    scaling_mode = odroid_settings_Scaling_get();

    int8_t previous_scaling_mode = -1;
    struct video_update* update;

    while(1)
    {
        xQueuePeek(videoQueue, &update, portMAX_DELAY);

        if (!update) break;

        bool redraw = previous_scaling_mode != scaling_mode || force_redraw;
        if (redraw)
        {
            ili9341_blank_screen();
            previous_scaling_mode = scaling_mode;
            force_redraw = false;

            if (scaling_mode) {
                float aspect = (sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS) &&
                               scaling_mode == ODROID_SCALING_FILL ? 1.2f : 1.f;
                odroid_display_set_scale(update->width, update->height, aspect);
            } else {
                odroid_display_reset_scale(update->width, update->height);
            }
        }

        ili9341_write_frame_scaled(update->buffer, redraw ? NULL : update->diff,
                                   update->width, update->height, update->stride,
                                   1, PIXEL_MASK, update->palette);

        xQueueReceive(videoQueue, &update, portMAX_DELAY);
    }

    videoTaskIsRunning = false;
    vTaskDelete(NULL);

    while (1) {}
}

void SaveState()
{
    // Save sram
    odroid_input_battery_monitor_enabled_set(0);
    odroid_system_led_set(1);

    odroid_display_lock();
    odroid_display_drain_spi();

    char* pathName = odroid_sdcard_get_savefile_path(romPath);
    if (!pathName) abort();

    FILE* f = fopen(pathName, "w");

    if (f == NULL)
    {
        printf("SaveState: fopen save failed\n");
        odroid_overlay_alert("Save failed");
    }
    else
    {
        system_save_state(f);
        fclose(f);

        printf("SaveState: system_save_state OK.\n");
    }

    odroid_display_unlock();

    free(pathName);

    odroid_system_led_set(0);
    odroid_input_battery_monitor_enabled_set(1);
}

void LoadState(const char* cartName)
{
    odroid_display_lock();
    odroid_display_drain_spi();

    char* pathName = odroid_sdcard_get_savefile_path(romPath);
    if (!pathName) abort();

    FILE* f = fopen(pathName, "r");
    if (f == NULL)
    {
        printf("LoadState: fopen load failed\n");
    }
    else
    {
        system_load_state(f);
        fclose(f);

        printf("LoadState: loadstate OK.\n");
    }

    odroid_display_unlock();

    free(pathName);
}

void QuitEmulator(bool save)
{
    printf("QuitEmulator: stopping tasks.\n");

    void *exitVideoTask = NULL;
    xQueueSend(videoQueue, &exitVideoTask, portMAX_DELAY);
    while (videoTaskIsRunning) vTaskDelay(1);

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

void system_manage_sram(uint8 *sram, int slot, int mode)
{
    // printf("system_manage_sram\n");
    // sram_load();
}

void app_main(void)
{
    printf("smsplusgx (%s-%s).\n", COMPILEDATE, GITREV);

    // Do before odroid_system_init to make sure we get the caps requested
    framebuffers[0] = heap_caps_malloc(SMS_WIDTH * SMS_HEIGHT, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    framebuffers[1] = heap_caps_malloc(SMS_WIDTH * SMS_HEIGHT, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    audioBuffer     = heap_caps_calloc(AUDIO_BUFFER_LENGTH * 2, 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);

    // Init all the console hardware
    odroid_system_init(3, AUDIO_SAMPLE_RATE, &romPath, &startAction);

    assert(framebuffers[0] && framebuffers[1]);
    assert(audioBuffer);

    videoQueue = xQueueCreate(1, sizeof(uint16_t*));
    xTaskCreatePinnedToCore(&videoTask, "videoTask", 4096, NULL, 5, NULL, 1);


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
    bitmap.data = framebuffers[0];

    // cart.pages = (cartSize / 0x4000);
    // cart.rom = romAddress;


    //system_init2(AUDIO_SAMPLE_RATE);
    set_option_defaults();

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    if (startAction == ODROID_START_ACTION_RESUME)
    {
        LoadState(romPath);
    }


    odroid_gamepad_state previousJoystickState;
    odroid_input_gamepad_read(&previousJoystickState);

    uint startTime;
    uint stopTime;
    uint elapsedTime;
    uint totalElapsedTime = 0;
    uint emulatedFrames = 0;
    uint skippedFrames = 0;
    uint muteFrameCount = 0;

    uint8_t refresh = (sms.display == DISPLAY_NTSC) ? 60 : 50;
    const int frameTime = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000 / refresh;

    while (true)
    {
        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
            force_redraw = true;
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_overlay_game_settings_menu(NULL, 0);
            force_redraw = true;
        }

        startTime = xthal_get_ccount();

        int smsButtons = 0;
        int smsSystem = 0;

    	if (joystick.values[ODROID_INPUT_UP]) smsButtons |= INPUT_UP;
    	if (joystick.values[ODROID_INPUT_DOWN]) smsButtons |= INPUT_DOWN;
    	if (joystick.values[ODROID_INPUT_LEFT]) smsButtons |= INPUT_LEFT;
    	if (joystick.values[ODROID_INPUT_RIGHT]) smsButtons |= INPUT_RIGHT;
    	if (joystick.values[ODROID_INPUT_A]) smsButtons |= INPUT_BUTTON2;
    	if (joystick.values[ODROID_INPUT_B]) smsButtons |= INPUT_BUTTON1;

		if (sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2)
		{
			if (joystick.values[ODROID_INPUT_START]) smsSystem |= INPUT_PAUSE;
			if (joystick.values[ODROID_INPUT_SELECT]) smsSystem |= INPUT_START;
		}
		else
		{
			if (joystick.values[ODROID_INPUT_START]) smsSystem |= INPUT_START;
			if (joystick.values[ODROID_INPUT_SELECT]) smsSystem |= INPUT_PAUSE;
		}

    	input.pad[0] = smsButtons;
        input.system = smsSystem;


        if (sms.console == CONSOLE_COLECO)
        {
            input.system = 0;
            coleco.keypad[0] = 0xff;
            coleco.keypad[1] = 0xff;

            // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, *, #
            switch (cart.crc)
            {
                case 0x798002a2:    // Frogger
                case 0x32b95be0:    // Frogger
                case 0x9cc3fabc:    // Alcazar
                case 0x964db3bc:    // Fraction Fever
                    if (joystick.values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 10; // *
                    }

                    if (previousJoystickState.values[ODROID_INPUT_SELECT] &&
                        !joystick.values[ODROID_INPUT_SELECT])
                    {
                        system_reset();
                    }
                    break;

                case 0x1796de5e:    // Boulder Dash
                case 0x5933ac18:    // Boulder Dash
                case 0x6e5c4b11:    // Boulder Dash
                    if (joystick.values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 11; // #
                    }

                    if (joystick.values[ODROID_INPUT_START] &&
                        joystick.values[ODROID_INPUT_LEFT])
                    {
                        coleco.keypad[0] = 1;
                    }

                    if (previousJoystickState.values[ODROID_INPUT_SELECT] &&
                        !joystick.values[ODROID_INPUT_SELECT])
                    {
                        system_reset();
                    }
                    break;
                case 0x109699e2:    // Dr. Seuss's Fix-Up The Mix-Up Puzzler
                case 0x614bb621:    // Decathlon
                    if (joystick.values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 1;
                    }
                    if (joystick.values[ODROID_INPUT_START] &&
                        joystick.values[ODROID_INPUT_LEFT])
                    {
                        coleco.keypad[0] = 10; // *
                    }
                    break;

                default:
                    if (joystick.values[ODROID_INPUT_START])
                    {
                        coleco.keypad[0] = 1;
                    }

                    if (previousJoystickState.values[ODROID_INPUT_SELECT] &&
                        !joystick.values[ODROID_INPUT_SELECT])
                    {
                        system_reset();
                    }
                    break;
            }
        }

        previousJoystickState = joystick;

        system_frame(skipFrame);

        if (skipFrame)
        {
            ++skippedFrames;
        }
        else
        {
            currentUpdate->width  = bitmap.viewport.w;
            currentUpdate->height = bitmap.viewport.h;
            currentUpdate->buffer = bitmap.data + bitmap.viewport.x;
            currentUpdate->stride = bitmap.pitch;
            render_copy_palette(currentUpdate->palette);

            struct video_update *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

            // Diff buffer
            odroid_buffer_diff(currentUpdate->buffer, previousUpdate->buffer,
                               currentUpdate->palette, previousUpdate->palette,
                               currentUpdate->width, currentUpdate->height,
                               currentUpdate->stride, 1, PIXEL_MASK, PAL_SHIFT_MASK,
                               currentUpdate->diff);

            // Send update data to video queue on other core
            xQueueSend(videoQueue, &currentUpdate, portMAX_DELAY);

            // Flip the update struct so we don't start writing into it while
            // the second core is still updating the screen.
            currentUpdate = previousUpdate;

            // Swap buffers
            currentBuffer = 1 - currentBuffer;
            bitmap.data = framebuffers[currentBuffer];
        }

        if (speedup_enabled)
        {
            skipFrame = emulatedFrames % 4;
            snd.enabled = false;
        }
        else
        {
            snd.enabled = true;

            // See if we need to skip a frame to keep up
            stopTime = xthal_get_ccount();
            elapsedTime = (stopTime > startTime) ?
                (stopTime - startTime) :
                ((uint64_t)stopTime + (uint64_t)0xffffffff) - (startTime);

            skipFrame = (!skipFrame && elapsedTime > frameTime);

            // Process audio
            for (int x = 0; x < snd.sample_count; x++)
            {
                // When the emulator starts, audible popping is generated.
                // Audio should be disabled during this startup period.
                if (muteFrameCount < 60 * 2)
                {
                    ++muteFrameCount;
                    audioBuffer[x] = 0;
                }
                else
                {
                    audioBuffer[x] = (snd.output[0][x] << 16) + snd.output[1][x];
                }
            }

            odroid_audio_submit((short*)audioBuffer, snd.sample_count);
        }


        stopTime = xthal_get_ccount();
        elapsedTime = (stopTime > startTime) ?
            (stopTime - startTime) :
            ((uint64_t)stopTime + (uint64_t)0xffffffff) - (startTime);

        totalElapsedTime += elapsedTime;
        ++emulatedFrames;

        if (emulatedFrames == refresh)
        {
            float seconds = totalElapsedTime / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000.0f);
            float fps = emulatedFrames / seconds;

            odroid_battery_state battery;
            odroid_input_battery_level_read(&battery);

            printf("HEAP:%d, FPS:%f, SKIP:%d, BATTERY:%d [%d]\n",
                esp_get_free_heap_size() / 1024, fps, skippedFrames,
                battery.millivolts, battery.percentage);

            emulatedFrames = 0;
            skippedFrames = 0;
            totalElapsedTime = 0;
        }
    }
}
