/*****************************************/
/* ESP32 (retro-go) Graphics Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*	ducalex                              */
/*****************************************/

#include <rg_system.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <pce-go.h>
#include <psg.h>

#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 / 5)

static int current_height = 0;
static int current_width = 0;
static int overscan = false;
static int downsample = false;
static int skipFrames = 0;
static uint8_t *framebuffers[2];

static bool emulationPaused = false; // This should probably be a mutex
static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];
static rg_app_t *app;

static const char *SETTING_AUDIOTYPE = "audiotype";
static const char *SETTING_OVERSCAN  = "overscan";
// --- MAIN


static rg_gui_event_t overscan_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        overscan = !overscan;
        rg_settings_set_number(NS_APP, SETTING_OVERSCAN, overscan);
        current_width = current_height = 0;
    }

    strcpy(option->value, overscan ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t sampletype_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        downsample ^= 1;
        rg_settings_set_number(NS_APP, SETTING_AUDIOTYPE, downsample);
    }

    strcpy(option->value, downsample ? "On " : "Off");

    return RG_DIALOG_VOID;
}

uint8_t *osd_gfx_framebuffer(int width, int height)
{
    if (width != current_width || height != current_height)
    {
        RG_LOGI("Resolution changed to: %dx%d\n", width, height);

        // PCE-GO needs 16 columns of scratch space + horizontally center
        int offset_center = 16 + ((XBUF_WIDTH - width) / 2);
        updates[0].buffer = framebuffers[0] + offset_center;
        updates[1].buffer = framebuffers[1] + offset_center;

        rg_display_set_source_format(width, height, 0, 0, XBUF_WIDTH, RG_PIXEL_PAL565_BE);

        current_width = width;
        current_height = height;
    }
    return skipFrames ? NULL : currentUpdate->buffer;
}

void osd_vsync(void)
{
    static int64_t lasttime, prevtime;

    if (skipFrames == 0)
    {
        rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
        rg_display_queue_update(currentUpdate, NULL);
        currentUpdate = previousUpdate;
    }

    // See if we need to skip a frame to keep up
    if (skipFrames == 0)
    {
        if (app->speed > 1.f)
            skipFrames = app->speed * 2.5f;
        else
            skipFrames = 1;
    }
    else if (skipFrames > 0)
    {
        skipFrames--;
    }

    int32_t frameTime = 1000000 / 60 / app->speed;
    int64_t curtime = rg_system_timer();
    int32_t sleep = frameTime - (curtime - lasttime);

    if (sleep > frameTime)
    {
        RG_LOGE("Our vsync timer seems to have overflowed! (%dus)\n", sleep);
    }
    else if (sleep > 0)
    {
        usleep(sleep);
    }
    else if (sleep < -(frameTime / 2))
    {
        skipFrames++;
    }

    rg_system_tick(curtime - prevtime);

    prevtime = rg_system_timer();
    lasttime += frameTime;

    if ((lasttime + frameTime) < prevtime)
        lasttime = prevtime;
}

void osd_input_read(uint8_t joypads[8])
{
    uint32_t joystick = rg_input_read_gamepad();
    uint32_t buttons = 0;

    if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
    {
        emulationPaused = true;
        if (joystick & RG_KEY_MENU)
            rg_gui_game_menu();
        else
            rg_gui_options_menu();
        rg_audio_set_sample_rate(app->sampleRate * app->speed);
        emulationPaused = false;
    }

    if (joystick & RG_KEY_LEFT)   buttons |= JOY_LEFT;
    if (joystick & RG_KEY_RIGHT)  buttons |= JOY_RIGHT;
    if (joystick & RG_KEY_UP)     buttons |= JOY_UP;
    if (joystick & RG_KEY_DOWN)   buttons |= JOY_DOWN;
    if (joystick & RG_KEY_A)      buttons |= JOY_A;
    if (joystick & RG_KEY_B)      buttons |= JOY_B;
    if (joystick & RG_KEY_START)  buttons |= JOY_RUN;
    if (joystick & RG_KEY_SELECT) buttons |= JOY_SELECT;

    joypads[0] = buttons;
}

static void audioTask(void *arg)
{
    rg_audio_sample_t audiobuffer[AUDIO_BUFFER_LENGTH];
    RG_LOGI("task started.\n");

    while (1)
    {
        // TODO: Clearly we need to add a better way to remain in sync with the main task...
        while (emulationPaused)
            usleep(20 * 1000);
        psg_update((void*)audiobuffer, AUDIO_BUFFER_LENGTH, downsample);
        rg_audio_submit(audiobuffer, AUDIO_BUFFER_LENGTH);
    }

    rg_task_delete(NULL);
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    // We must use previous update because at this point current has been wiped.
    rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
    return rg_display_save_frame(filename, previousUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
    return SaveState(filename) == 0;
}

static bool load_state_handler(const char *filename)
{
    if (LoadState(filename) != 0)
    {
        ResetPCE(false);
        return false;
    }
    return true;
}

static bool reset_handler(bool hard)
{
    ResetPCE(hard);
    return true;
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {2, "Overscan      ", "On ", 1, &overscan_update_cb},
        {3, "Unsigned audio", "Off", 1, &sampletype_update_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    framebuffers[0] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_FAST);
    framebuffers[1] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_FAST);

    overscan = rg_settings_get_number(NS_APP, SETTING_OVERSCAN, 1);
    downsample = rg_settings_get_number(NS_APP, SETTING_AUDIOTYPE, 0);

    uint16_t *palette = PalettePCE(16);
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = (palette[i] << 8) | (palette[i] >> 8);
        updates[0].palette[i] = color;
        updates[1].palette[i] = color;
    }
    free(palette);

    InitPCE(AUDIO_SAMPLE_RATE, true, app->romPath);

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    rg_task_create("pce_sound", &audioTask, NULL, 3 * 1024, 5, 1);

    RunPCE();

    RG_PANIC("PCE-GO died.");
}
