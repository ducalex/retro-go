/*****************************************/
/* ESP32 (Odroid GO) Graphics Engine     */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*	ducalex                              */
/*****************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rg_system.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <osd.h>

#define AUDIO_SAMPLE_RATE 22050
// #define AUDIO_BUFFER_LENGTH  (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 / 5)

static int16_t audiobuffer[AUDIO_BUFFER_LENGTH * 2];
static uint16_t *mypalette;
static uint8_t *framebuffers[2];
static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];
static int current_height, current_width;
static bool overscan = false;
static int skipFrames = 0;

static rg_app_desc_t *app;

#ifdef ENABLE_NETPLAY
static bool netplay = false;
#endif

static const char *SETTING_AUDIOTYPE = "audiotype";
static const char *SETTING_OVERSCAN  = "overscan";
// --- MAIN


uint8_t *osd_gfx_framebuffer(void)
{
    if (skipFrames > 0)
        return NULL;
    return (uint8_t *)currentUpdate->my_arg;
}

void osd_gfx_init(void)
{
    framebuffers[0] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);
    framebuffers[1] = rg_alloc(XBUF_WIDTH * XBUF_HEIGHT, MEM_SLOW);

    overscan = rg_settings_get_app_int32(SETTING_OVERSCAN, 1);

    mypalette = PalettePCE(16);
    for (int i = 0; i < 256; i++) {
        mypalette[i] = (mypalette[i] << 8) | (mypalette[i] >> 8);
    }

    osd_gfx_set_mode(256, 240);
}

void osd_gfx_set_mode(int width, int height)
{
    const rg_display_t *display = rg_display_get_status();
    int crop_h = MAX(0, width - (int)display->screen.width);
    int crop_v = MAX(0, height - (int)display->screen.height) + (overscan ? 6 : 0);

    // We center the content vertically and horizontally to allow overflows all around
    int offset_center = (((XBUF_HEIGHT - height) / 2 + 16) * XBUF_WIDTH + (XBUF_WIDTH - width) / 2);
    int offset_cropping = (crop_v / 2) * XBUF_WIDTH + (crop_h / 2);

    current_width = width;
    current_height = height;

    RG_LOGI("Resolution: %dx%d / Cropping: H: %d V: %d\n", width, height, crop_h, crop_v);

    frames[0].flags = RG_PIXEL_PAL|RG_PIXEL_565|RG_PIXEL_BE;
    frames[0].width = width - crop_h;
    frames[0].height = height - crop_v;
    frames[0].stride = XBUF_WIDTH;
    frames[0].pixel_mask = 0xFF;
    frames[0].palette = mypalette;
    frames[1] = frames[0];

    frames[0].buffer = framebuffers[0] + offset_center + offset_cropping;
    frames[1].buffer = framebuffers[1] + offset_center + offset_cropping;

    frames[0].my_arg = framebuffers[0] + offset_center;
    frames[1].my_arg = framebuffers[1] + offset_center;

    currentUpdate = &frames[0];
}

void osd_gfx_blit(void)
{
    bool drawFrame = !skipFrames;

    if (drawFrame)
    {
        rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];
        rg_display_queue_update(currentUpdate, NULL);

        currentUpdate = previousUpdate;
    }

    // See if we need to skip a frame to keep up
    if (skipFrames == 0)
    {
        skipFrames++;

        if (app->speedupEnabled)
            skipFrames += app->speedupEnabled * 2.5;
    }
    else if (skipFrames > 0)
    {
        skipFrames--;
    }
}

void osd_gfx_shutdown(void)
{
    RG_LOGI("Goodbye...\n");
}

static dialog_return_t overscan_update_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        overscan = !overscan;
        rg_settings_set_app_int32(SETTING_OVERSCAN, overscan);
        osd_gfx_set_mode(current_width, current_height);
    }

    strcpy(option->value, overscan ? "On " : "Off");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t sampletype_update_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        host.sound.sample_uint8 ^= 1;
        rg_settings_set_app_int32(SETTING_AUDIOTYPE, host.sound.sample_uint8);
    }

    strcpy(option->value, host.sound.sample_uint8 ? "On " : "Off");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t advanced_settings_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        dialog_option_t options[] = {
            {2, "Overscan      ", "On ", 1, &overscan_update_cb},
            {3, "Unsigned audio", "Off", 1, &sampletype_update_cb},
            RG_DIALOG_CHOICE_LAST};
        rg_gui_dialog("Advanced", options, 0);
    }
    return RG_DIALOG_IGNORE;
}

int osd_input_init(void)
{
    return 0;
}

void osd_input_read(void)
{
    uint32_t joystick = rg_input_read_gamepad();
    uint32_t buttons = 0;

    // TO DO: We should pause the audio task when entering a menu...
    if (joystick & GAMEPAD_KEY_MENU)
    {
        rg_gui_game_menu();
    }
    else if (joystick & GAMEPAD_KEY_VOLUME)
    {
        dialog_option_t options[] = {
            {101, "More...", NULL, 1, &advanced_settings_cb},
            RG_DIALOG_CHOICE_LAST};
        rg_gui_game_settings_menu(options);
    }

    if (joystick & GAMEPAD_KEY_LEFT)   buttons |= JOY_LEFT;
    if (joystick & GAMEPAD_KEY_RIGHT)  buttons |= JOY_RIGHT;
    if (joystick & GAMEPAD_KEY_UP)     buttons |= JOY_UP;
    if (joystick & GAMEPAD_KEY_DOWN)   buttons |= JOY_DOWN;
    if (joystick & GAMEPAD_KEY_A)      buttons |= JOY_A;
    if (joystick & GAMEPAD_KEY_B)      buttons |= JOY_B;
    if (joystick & GAMEPAD_KEY_START)  buttons |= JOY_RUN;
    if (joystick & GAMEPAD_KEY_SELECT) buttons |= JOY_SELECT;

    PCE.Joypad.regs[0] = buttons;
}

static void audioTask(void *arg)
{
    RG_LOGI("task started.\n");

    while (1)
    {
        psg_update(audiobuffer, AUDIO_BUFFER_LENGTH);
        rg_audio_submit(audiobuffer, AUDIO_BUFFER_LENGTH);
    }

    vTaskDelete(NULL);
}

void osd_snd_init(void)
{
    host.sound.stereo = true;
    host.sound.sample_freq = AUDIO_SAMPLE_RATE;
    host.sound.sample_uint8 = rg_settings_get_app_int32(SETTING_AUDIOTYPE, 0);

    xTaskCreatePinnedToCore(&audioTask, "audioTask", 1024 * 2, NULL, 5, NULL, 1);
}

void osd_snd_shutdown(void)
{
    rg_audio_deinit();
}

void osd_log(int type, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

void osd_vsync(void)
{
    const int32_t frametime = get_frame_time(60);
    static int64_t lasttime, prevtime;

    int64_t curtime = get_elapsed_time();
    int32_t sleep = frametime - (curtime - lasttime);

    if (sleep > frametime)
    {
        RG_LOGE("Our vsync timer seems to have overflowed! (%dus)\n", sleep);
    }
    else if (sleep > 0)
    {
        usleep(sleep);
    }
    else if (sleep < -(frametime / 2))
    {
        skipFrames++;
    }

    rg_system_tick(curtime - prevtime);

    prevtime = get_elapsed_time();
    lasttime += frametime;

    if ((lasttime + frametime) < prevtime)
        lasttime = prevtime;
}

void *osd_alloc(size_t size)
{
    return rg_alloc(size, (size <= 0x10000) ? MEM_FAST : MEM_SLOW);
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    // We must use previous update because at this point current has been wiped.
    rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];
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
    rg_emu_proc_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers);

    InitPCE(app->romPath);

    if (app->startAction == RG_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

    RunPCE();

    RG_PANIC("PCE-GO died.");
}
