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

#include "../components/huexpress/engine/osd.h"

#define APP_ID 40

static uint16_t mypalette[256];
static uint8_t *framebuffers[2];
static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];
static int current_height, current_width;
static bool overscan = false;

static int skipFrames = 0;
static int blitFrames = 0;
static int fullFrames = 0;

#define NVS_KEY_SAMPLE_TYPE "audiotype"

#define AUDIO_SAMPLE_RATE 22050
// #define AUDIO_BUFFER_LENGTH  (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 / 5)

static short audiobuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_app_desc_t *app;

#ifdef ENABLE_NETPLAY
static bool netplay = false;
#endif
// --- MAIN

static inline void clear_buffer(rg_video_frame_t *update)
{
    void *buffer = update->buffer;
    for (int i = 0; i < update->height; ++i)
    {
        memset(buffer, PCE.Palette[0], update->width);
        buffer += update->stride;
    }
}

static inline void set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
    #define COLOR_RGB(r, g, b) ((((r) << 12) & 0xf800) + (((g) << 7) & 0x07e0) + (((b) << 1) & 0x001f))

    uint16_t col = 0xffff;
    if (index != 255)
    {
        col = COLOR_RGB(r >> 2, g >> 2, b >> 2);
        col = (col << 8) | (col >> 8);
    }
    mypalette[index] = col;
}

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

    overscan = rg_settings_DisplayOverscan_get();

    // Build palette
    for (int i = 0; i < 255; i++)
    {
        set_color(i, (i & 0x1C) << 1, (i & 0xe0) >> 2, (i & 0x03) << 4);
    }
    set_color(255, 0x3f, 0x3f, 0x3f);

    osd_gfx_set_mode(256, 240);
}

void osd_gfx_set_mode(int width, int height)
{
    int crop_h = MAX(0, width - RG_SCREEN_WIDTH);
    int crop_v = MAX(0, height - RG_SCREEN_HEIGHT) + (overscan ? 6 : 0);

    // We center the content vertically and horizontally to allow overflows all around
    int offset_center = (((XBUF_HEIGHT - height) / 2 + 16) * XBUF_WIDTH + (XBUF_WIDTH - width) / 2);
    int offset_cropping = (crop_v / 2) * XBUF_WIDTH + (crop_h / 2);

    current_width = width;
    current_height = height;

    MESSAGE_INFO("[GFX] Resolution: %dx%d / Cropping: H: %d V: %d\n", width, height, crop_h, crop_v);

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
    clear_buffer(currentUpdate);
}

void osd_gfx_blit(void)
{
    bool drawFrame = !skipFrames;

    if (drawFrame)
    {
        rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];
        if (rg_display_queue_update(currentUpdate, NULL) == RG_SCREEN_UPDATE_FULL)
        {
            fullFrames++;
        }
        blitFrames++;

        currentUpdate = previousUpdate;
        clear_buffer(currentUpdate);
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
    MESSAGE_INFO("Goodbye...\n");
}

static bool overscan_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        overscan = !overscan;
        rg_settings_DisplayOverscan_set(overscan);
        osd_gfx_set_mode(current_width, current_height);
    }

    strcpy(option->value, overscan ? "On " : "Off");

    return event == RG_DIALOG_ENTER;
}

static bool sampletype_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        host.sound.sample_uint8 ^= 1;
        rg_settings_app_int32_set(NVS_KEY_SAMPLE_TYPE, host.sound.sample_uint8);
    }

    strcpy(option->value, host.sound.sample_uint8 ? "On " : "Off");

    return event == RG_DIALOG_ENTER;
}

static bool advanced_settings_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        dialog_choice_t options[] = {
            {2, "Overscan      ", "On ", 1, &overscan_update_cb},
            {3, "Unsigned audio", "Off", 1, &sampletype_update_cb},
            RG_DIALOG_CHOICE_LAST};
        rg_gui_dialog("Advanced", options, 0);
    }
    return false;
}

int osd_input_init(void)
{
    return 0;
}

void osd_input_read(void)
{
    gamepad_state_t joystick = rg_input_read_gamepad();

    if (joystick.values[GAMEPAD_KEY_MENU])
    {
        rg_gui_game_menu();
    }
    else if (joystick.values[GAMEPAD_KEY_VOLUME])
    {
        dialog_choice_t options[] = {
            {101, "More...", "", 1, &advanced_settings_cb},
            RG_DIALOG_CHOICE_LAST};
        rg_gui_game_settings_menu(options);
    }

    unsigned char rc = 0;
    if (joystick.values[GAMEPAD_KEY_LEFT])   rc |= JOY_LEFT;
    if (joystick.values[GAMEPAD_KEY_RIGHT])  rc |= JOY_RIGHT;
    if (joystick.values[GAMEPAD_KEY_UP])     rc |= JOY_UP;
    if (joystick.values[GAMEPAD_KEY_DOWN])   rc |= JOY_DOWN;
    if (joystick.values[GAMEPAD_KEY_A])      rc |= JOY_A;
    if (joystick.values[GAMEPAD_KEY_B])      rc |= JOY_B;
    if (joystick.values[GAMEPAD_KEY_START])  rc |= JOY_RUN;
    if (joystick.values[GAMEPAD_KEY_SELECT]) rc |= JOY_SELECT;

    PCE.Joypad.regs[0] = rc;
}

static void audioTask(void *arg)
{
    MESSAGE_INFO("[PSG] task started.\n");

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
    host.sound.sample_uint8 = rg_settings_app_int32_get(NVS_KEY_SAMPLE_TYPE, 0);

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
        MESSAGE_ERROR("Our vsync timer seems to have overflowed! (%dus)\n", sleep);
    }
    else if (sleep > 0)
    {
        usleep(sleep);
    }
    else if (sleep < -(frametime / 2))
    {
        skipFrames++;
    }

    rg_system_tick(blitFrames == 0, fullFrames > 0, curtime - prevtime);
    blitFrames = 0;
    fullFrames = 0;

    prevtime = get_elapsed_time();
    lasttime += frametime;

    if ((lasttime + frametime) < prevtime)
        lasttime = prevtime;
}

void *osd_alloc(size_t size)
{
    return rg_alloc(size, (size <= 0x10000) ? MEM_FAST : MEM_SLOW);
}

static bool save_state(char *pathName)
{
    if (SaveState(pathName) == 0)
    {
        char *filename = rg_emu_get_path(EMU_PATH_SCREENSHOT, 0);
        if (filename)
        {
            // We must use previous update because at this point current has been wiped.
            rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];
            rg_display_save_frame(filename, previousUpdate, 160, 0);
            rg_free(filename);
        }
        return true;
    }

    return false;
}

static bool load_state(char *pathName)
{
    if (LoadState(pathName) != 0)
    {
        ResetPCE(false);
        return false;
    }
    return true;
}

static bool reset_emulation(bool hard)
{
    ResetPCE(hard);
    return true;
}

void app_main(void)
{
    rg_emu_proc_t handlers = {
        .loadState = &load_state,
        .saveState = &save_state,
        .reset = &reset_emulation,
    };

    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(handlers);

    // Clearing the buffer on the display core is somewhat faster, but it
    // prevents us from doing partial updates and screenshots.
    // rg_display_set_callback(clear_buffer);

    app = rg_system_get_app();

    InitPCE(app->romPath);

    if (app->startAction == EMU_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

    RunPCE();

    RG_PANIC("HuExpress died.");
}
