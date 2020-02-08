#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"

#include "../components/smsplus/shared.h"
#include "odroid_system.h"

#define AUDIO_SAMPLE_RATE (32000)

#define FRAME_CHECK 10
#if 0
#define INTERLACE_ON_THRESHOLD 8
#define INTERLACE_OFF_THRESHOLD 10
#elif 0
// All interlaced updates
#define INTERLACE_ON_THRESHOLD (FRAME_CHECK+1)
#define INTERLACE_OFF_THRESHOLD (FRAME_CHECK+1)
#else
// All progressive updates
#define INTERLACE_ON_THRESHOLD 0
#define INTERLACE_OFF_THRESHOLD 0
#endif

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

bool forceConsoleReset = false;
static char* romPath = NULL;

uint8_t* framebuffers[2];
int currentBuffer = 0;

uint32_t* audioBuffer = NULL;
int audioBufferCount = 0;

QueueHandle_t videoQueue;
TaskHandle_t videoTaskHandle;

struct bitmap_meta {
    odroid_scanline diff[SMS_HEIGHT];
    uint8_t *buffer;
    uint16 palette[PALETTE_SIZE*2];
    int width;
    int height;
    int stride;
};
static struct bitmap_meta update1 = {0,};
static struct bitmap_meta update2 = {0,};
static struct bitmap_meta *update = &update1;

int8_t scaling_mode = ODROID_SCALING_FIT;
int8_t force_redraw = false;

volatile bool videoTaskIsRunning = false;
static void videoTask(void *arg)
{
    struct bitmap_meta* meta;

    videoTaskIsRunning = true;

    int8_t previous_scaling_mode = ODROID_SCALING_UNKNOWN;
    float aspect = 1;

    while(1)
    {
        xQueuePeek(videoQueue, &meta, portMAX_DELAY);

        if (!meta) break;

        bool redraw = (previous_scaling_mode != scaling_mode) || force_redraw;
        if (redraw)
        {
            ili9341_blank_screen();
            previous_scaling_mode = scaling_mode;
            force_redraw = false;
            if (scaling_mode) {
                aspect = (sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS) ? 1.2f : 1.f;
                odroid_display_set_scale(meta->width, meta->height, aspect);
            } else {
                odroid_display_reset_scale(meta->width, meta->height);
            }
        }

        ili9341_write_frame_8bit(meta->buffer,
                                 redraw ? NULL : meta->diff,
                                 meta->width, meta->height,
                                 meta->stride, PIXEL_MASK, meta->palette);

        xQueueReceive(videoQueue, &meta, portMAX_DELAY);
    }

    odroid_display_lock();

    // Draw hourglass
    odroid_display_show_hourglass();

    odroid_display_unlock();

    videoTaskIsRunning = false;
    vTaskDelete(NULL);

    while (1) {}
}

//Read an unaligned byte.
char unalChar(const unsigned char *adr) {
	//See if the byte is in memory that can be read unaligned anyway.
	if (!(((int)adr)&0x40000000)) return *adr;
	//Nope: grab a word and distill the byte.
	int *p=(int *)((int)adr&0xfffffffc);
	int v=*p;
	int w=((int)adr&3);
	if (w==0) return ((v>>0)&0xff);
	if (w==1) return ((v>>8)&0xff);
	if (w==2) return ((v>>16)&0xff);
	if (w==3) return ((v>>24)&0xff);

    abort();
    return 0;
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

void PowerDown()
{
    // Stop tasks
    printf("PowerDown: stopping tasks.\n");

    // Clear audio to prevent studdering
    printf("PowerDown: stopping audio.\n");
    odroid_audio_terminate();

    void *exitVideoTask = NULL;
    xQueueSend(videoQueue, &exitVideoTask, portMAX_DELAY);
    while (videoTaskIsRunning) { vTaskDelay(10); }


    // state
    printf("PowerDown: Saving state.\n");
    SaveState();

    // LCD
    printf("PowerDown: Powerdown LCD panel.\n");
    ili9341_poweroff();

    odroid_system_sleep();

    // Should never reach here
    abort();
}

void QuitEmulator(bool save)
{
    esp_err_t err;

    // Stop tasks
    printf("PowerDown: stopping tasks.\n");

    // Clear audio to prevent studdering
    printf("PowerDown: stopping audio.\n");
    odroid_audio_terminate();

    ili9341_blank_screen();

    void *exitVideoTask = NULL;
    xQueueSend(videoQueue, &exitVideoTask, portMAX_DELAY);
    while (videoTaskIsRunning) { vTaskDelay(10); }

    if (save) {
        // state
        printf("PowerDown: Saving state.\n");
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

    // Init all the console hardware
    odroid_system_init(3, AUDIO_SAMPLE_RATE, &romPath, &forceConsoleReset);

    assert(framebuffers[0] && framebuffers[1]);

    scaling_mode = odroid_settings_Scaling_get();

    videoQueue = xQueueCreate(1, sizeof(uint16_t*));
    xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 1);


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

    if (!forceConsoleReset)
    {
        LoadState(romPath);
    }


    odroid_gamepad_state previousJoystickState;
    odroid_input_gamepad_read(&previousJoystickState);

    uint startTime;
    uint stopTime;
    uint totalElapsedTime = 0;
    int frame = 0;
    uint16_t muteFrameCount = 0;

    int refresh = (sms.display == DISPLAY_NTSC) ? 60 : 50;
    const int frameTime = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000 / refresh;
    int skipFrame = 0;
    int skippedFrames = 0;
    int renderedFrames = 0;
    int interlacedFrames = 0;
    int interlace = -1;

    while (true)
    {
        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
            force_redraw = true;
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_overlay_settings_menu(NULL, 0);
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

        if (!skipFrame)
        {
            system_frame(0, interlace);

            // Store buffer data
            if (sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS) {
                update->buffer = bitmap.data + (SMS_WIDTH - GG_WIDTH) / 2;
                update->width = GG_WIDTH;
                update->height = GG_HEIGHT;
            } else {
                update->buffer = bitmap.data;
                update->width = SMS_WIDTH;
                update->height = SMS_HEIGHT;
            }
            update->stride = bitmap.pitch;
            render_copy_palette(update->palette);

            struct bitmap_meta *old_update = (update == &update1) ? &update2 : &update1;

            // Diff buffer
            if (interlace >= 0) {
                ++interlacedFrames;
                interlace = 1 - interlace;
                odroid_buffer_diff_interlaced(update->buffer, old_update->buffer,
                                              update->palette, old_update->palette,
                                              update->width, update->height,
                                              update->stride,
                                              PIXEL_MASK, PAL_SHIFT_MASK,
                                              interlace,
                                              update->diff, old_update->diff);
            } else {
                odroid_buffer_diff(update->buffer, old_update->buffer,
                                   update->palette, old_update->palette,
                                   update->width, update->height,
                                   update->stride, PIXEL_MASK, PAL_SHIFT_MASK,
                                   update->diff);
            }

#if 1
            // Send update data to video queue on other core
            void *arg = (void*)update;
            xQueueSend(videoQueue, &arg, portMAX_DELAY);
#endif

            // Flip the update struct so we don't start writing into it while
            // the second core is still updating the screen.
            update = old_update;

            // Swap buffers
            currentBuffer = 1 - currentBuffer;
            bitmap.data = framebuffers[currentBuffer];
            ++renderedFrames;
        }
        else
        {
            system_frame(1, -1);
            ++skippedFrames;
        }

        // See if we need to skip a frame to keep up
        stopTime = xthal_get_ccount();
        int elapsedTime = (stopTime > startTime) ?
            (stopTime - startTime) :
            ((uint64_t)stopTime + (uint64_t)0xffffffff) - (startTime);
#if 1
        skipFrame = (!skipFrame && elapsedTime > frameTime);

        // Use interlacing if we drop too many frames
        if ((frame % FRAME_CHECK) == 0) {
            if (renderedFrames <= INTERLACE_ON_THRESHOLD && interlace == -1) {
                interlace = 0;
            }
            if (renderedFrames >= INTERLACE_OFF_THRESHOLD) {
                interlace = -1;
            }
            renderedFrames = 0;
        }
#endif

        // Create a buffer for audio if needed
        if (!audioBuffer || audioBufferCount < snd.sample_count)
        {
            if (audioBuffer)
                free(audioBuffer);

            size_t bufferSize = snd.sample_count * 2 * sizeof(int16_t);
            //audioBuffer = malloc(bufferSize);
            audioBuffer = heap_caps_malloc(bufferSize, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
            if (!audioBuffer)
                abort();

            audioBufferCount = snd.sample_count;

            printf("app_main: Created audio buffer (%d bytes).\n", bufferSize);
        }

        // Process audio
        for (int x = 0; x < snd.sample_count; x++)
        {
            uint32_t sample;

            if (muteFrameCount < 60 * 2)
            {
                // When the emulator starts, audible popping is generated.
                // Audio should be disabled during this startup period.
                sample = 0;
                ++muteFrameCount;
            }
            else
            {
                sample = (snd.output[0][x] << 16) + snd.output[1][x];
            }

            audioBuffer[x] = sample;
        }

        // send audio

        odroid_audio_submit((short*)audioBuffer, snd.sample_count);


        stopTime = xthal_get_ccount();

        elapsedTime = (stopTime > startTime) ?
            (stopTime - startTime) :
            ((uint64_t)stopTime + (uint64_t)0xffffffff) - (startTime);

        totalElapsedTime += elapsedTime;
        ++frame;

        if (frame == 60)
        {
            float seconds = totalElapsedTime / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000.0f);
            float fps = (frame / seconds);

            odroid_battery_state battery;
            odroid_input_battery_level_read(&battery);

            printf("HEAP:%d, FPS:%f, INT:%d, SKIP:%d, BATTERY:%d [%d]\n",
                esp_get_free_heap_size() / 1024, fps, interlacedFrames, skippedFrames,
                battery.millivolts, battery.percentage);

            frame = 0;
            totalElapsedTime = 0;
            skippedFrames = 0;
            interlacedFrames = 0;
        }
    }
}
