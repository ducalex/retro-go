#include "shared.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>

#include <gw_system.h>
#include <gw_romloader.h>

/* access to internals for debug purpose */
#include <sm510.h>

unsigned char *ROM_DATA;
unsigned int ROM_DATA_LENGTH;

/* keys inpus (hw & sw) */
static bool softkey_time_pressed = 0;
static bool softkey_alarm_pressed = 0;
static bool softkey_A_pressed = 0;
static bool softkey_only = 0;
static int softkey_duration = 0;

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;

static void gw_set_time()
{
    // Get time. According to STM docs, both functions need to be called at once.
    time_t time_sec = time(NULL);
    struct tm *rtc = localtime(&time_sec);
    gw_time_t time;

    time.hours = rtc->tm_hour;
    time.minutes = rtc->tm_min;
    time.seconds = rtc->tm_sec;

    // set time of the emulated system
    gw_system_set_time(time);
    printf("Set time done!\n");
}

static void gw_check_time()
{
    static unsigned int is_gw_time_sync = 0;

    // Update time before we can set it
    time_t time_sec = time(NULL);
    struct tm *rtc = localtime(&time_sec);
    gw_time_t time = {0};

    // Set times
    time.hours = rtc->tm_hour;
    time.minutes = rtc->tm_min;
    time.seconds = rtc->tm_sec;

    // update time every 30s
    if ((time.seconds == 30) || (is_gw_time_sync == 0))
    {
        is_gw_time_sync = 1;
        gw_system_set_time(time);
    }
}
static bool gw_system_SaveState(const char *pathName)
{
    printf("Saving state...\n");
    gw_state_t state_save_buffer = {0};
    gw_state_save(&state_save_buffer);
    FILE *fp = fopen(pathName, "wb");
    if (!fp)
        return false;
    bool success = fwrite(&state_save_buffer, sizeof(gw_state_t), 1, fp);
    fclose(fp);
    return success;
}

static bool gw_system_LoadState(const char *pathName)
{
    printf("Loading state...\n");
    FILE *fp = fopen(pathName, "rb");
    if (!fp)
        return false;
    gw_state_t state_save_buffer;
    bool success = fread(&state_save_buffer, sizeof(gw_state_t), 1, fp)
        && gw_state_load(&state_save_buffer);
    fclose(fp);
    return success;
}

/* callback to get buttons state */
unsigned int gw_get_buttons()
{
    uint32_t hw_buttons = 0;
    if (!softkey_only)
    {
        uint32_t joystick = rg_input_read_gamepad();
        if (joystick & RG_KEY_LEFT) hw_buttons |= GW_BUTTON_LEFT;
        if (joystick & RG_KEY_UP) hw_buttons |= GW_BUTTON_UP;
        if (joystick & RG_KEY_RIGHT) hw_buttons |= GW_BUTTON_RIGHT;
        if (joystick & RG_KEY_DOWN) hw_buttons |= GW_BUTTON_DOWN;
        if (joystick & RG_KEY_A) hw_buttons |= GW_BUTTON_A;
        if (joystick & RG_KEY_B) hw_buttons |= GW_BUTTON_B;
        if (joystick & RG_KEY_SELECT) hw_buttons |= GW_BUTTON_TIME;
        if (joystick & RG_KEY_START) hw_buttons |= GW_BUTTON_GAME;
    }

    // software keys
    hw_buttons |= ((unsigned int)softkey_A_pressed) << 4;
    hw_buttons |= ((unsigned int)softkey_time_pressed) << 10;
    hw_buttons |= ((unsigned int)softkey_alarm_pressed) << 11;

    return hw_buttons;
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

void gw_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &gw_system_LoadState,
        .saveState = &gw_system_SaveState,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        RG_DIALOG_END,
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);
    app->tickRate = GW_REFRESH_RATE;

    updates[0] = rg_surface_create(GW_SCREEN_WIDTH, GW_SCREEN_HEIGHT, RG_PIXEL_565_LE, MEM_FAST);
    currentUpdate = updates[0];

    FILE *fp = fopen(app->romPath, "rb");
    if (!fp)
        RG_PANIC("Rom load failed");
    ROM_DATA = malloc(400000);
    // fseek(fp, 0x15, SEEK_SET);
    ROM_DATA_LENGTH = fread(ROM_DATA, 1, 400000, fp);
    fclose(fp);

    unsigned previous_m_halt = 2;

    /*** load ROM  */
    bool rom_status = gw_system_romload();

    if (!rom_status)
        RG_PANIC("gw_system_romload failed!");

    /*** Clear audio buffer */
    gw_system_sound_init();
    printf("Sound initialized\n");

    /* clear soft keys */
    softkey_time_pressed = 0;
    softkey_alarm_pressed = 0;
    softkey_duration = 0;

    /*** Configure the emulated system */
    gw_system_config();
    printf("G&W configured\n");

    /*** Start and Reset the emulated system */
    gw_system_start();
    printf("G&W start\n");

    gw_system_reset();
    printf("G&W reset\n");

    /* check if we have to load state */
    bool LoadState_done = false;
    if (app->bootFlags & RG_BOOT_RESUME)
    {
        LoadState_done = rg_emu_load_state(app->saveSlot);
    }

    /* emulate watch mode */
    if (!LoadState_done)
    {
        softkey_time_pressed = 0;
        softkey_alarm_pressed = 0;
        softkey_A_pressed = 0;

        // disable user keys
        softkey_only = 1;

        printf("G&W emulate watch mode\n");

        gw_system_reset();

        // From reset state : run
        gw_system_run(GW_AUDIO_FREQ * 2);

        // press TIME to exit TIME settings mode
        softkey_time_pressed = 1;
        gw_system_run(GW_AUDIO_FREQ / 2);
        softkey_time_pressed = 0;
        gw_system_run(GW_AUDIO_FREQ * 2);

        // synchronize G&W with RTC and run
        gw_check_time();
        gw_set_time();
        gw_system_run(GW_AUDIO_FREQ);

        // press A required by some game
        softkey_A_pressed = 1;
        gw_system_run(GW_AUDIO_FREQ / 2);
        softkey_A_pressed = 0;
        gw_system_run(GW_AUDIO_FREQ);

        // enable user keys
        softkey_only = 0;
    }

    /*** Main emulator loop */
    printf("Main emulator loop start\n");

    while (true)
    {
        /* refresh internal G&W timer on emulated CPU state transition */
        if (previous_m_halt != m_halt)
            gw_check_time();

        previous_m_halt = m_halt;

        // hardware keys
        uint32_t joystick = rg_input_read_gamepad();

        if (joystick & RG_KEY_MENU)
            rg_gui_game_menu();
        else if (joystick & RG_KEY_OPTION)
            rg_gui_options_menu();

        // soft keys emulation
        if (softkey_duration > 0)
            softkey_duration--;

        if (softkey_duration == 0)
        {
            softkey_time_pressed = 0;
            softkey_alarm_pressed = 0;
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = true;

        /* Emulate and Blit */
        // Call the emulator function with number of clock cycles
        // to execute on the emulated device
        gw_system_run(GW_SYSTEM_CYCLES);

        // Our refresh rate is 128Hz, which is way too fast for our display
        // so make sure the previous frame is done sending before queuing a new one
        if (rg_display_sync(false) && drawFrame)
        {
            gw_system_blit(currentUpdate->data);
            rg_display_submit(currentUpdate, 0);
        }
        /****************************************************************************/

        // Tick before submitting audio/syncing
        rg_system_tick(rg_system_timer() - startTime);

        /* copy audio samples for DMA */
        rg_audio_sample_t mixbuffer[GW_AUDIO_BUFFER_LENGTH];
        for (size_t i = 0; i < GW_AUDIO_BUFFER_LENGTH; i++)
        {
            mixbuffer[i].left = gw_audio_buffer[i] << 13;
            mixbuffer[i].right = gw_audio_buffer[i] << 13;
        }
        rg_audio_submit(mixbuffer, GW_AUDIO_BUFFER_LENGTH);
        gw_audio_buffer_copied = true;
    } // end of loop
}
