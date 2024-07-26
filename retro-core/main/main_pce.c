#include "shared.h"

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <pce-go.h>
#include <psg.h>

#undef AUDIO_SAMPLE_RATE
#define AUDIO_SAMPLE_RATE 22050

static bool emulationPaused = false; // This should probably be a mutex
static int overscan = false;
static int skipFrames = 0;
static bool drawFrame = true;
static bool slowFrame = false;

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;

static const char *SETTING_OVERSCAN  = "overscan";
// --- MAIN


static rg_gui_event_t overscan_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        overscan = !overscan;
        rg_settings_set_number(NS_APP, SETTING_OVERSCAN, overscan);
        return RG_DIALOG_REDRAW;
    }

    strcpy(option->value, overscan ? "On " : "Off");

    return RG_DIALOG_VOID;
}

uint8_t *osd_gfx_framebuffer(int width, int height)
{
    if (width > 0 && height > 0)
    {
        currentUpdate->width = width;
        currentUpdate->height = height;
    }
    return drawFrame ? currentUpdate->data : NULL;
}

void osd_vsync(void)
{
    static int64_t lasttime, prevtime;

    if (drawFrame)
    {
        slowFrame = !rg_display_sync(false);
        rg_display_submit(currentUpdate, 0);
        currentUpdate = updates[currentUpdate == updates[0]];
    }

    // See if we need to skip a frame to keep up
    if (skipFrames == 0)
    {
        if (app->frameskip > 0)
            skipFrames = app->frameskip;
        else if (drawFrame && slowFrame)
            skipFrames = 1;
    }
    else if (skipFrames > 0)
    {
        skipFrames--;
    }

    int64_t curtime = rg_system_timer();
    int frameTime = 1000000 / (app->tickRate * app->speed);
    int sleep = frameTime - (curtime - lasttime);

    if (sleep > frameTime)
    {
        RG_LOGE("Our vsync timer seems to have overflowed! (%dus)", sleep);
    }
    else if (sleep > 0)
    {
        rg_usleep(sleep);
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

    drawFrame = (skipFrames == 0);
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
    const size_t numSamples = 62; // TODO: Find the best value

    RG_LOGI("task started. numSamples=%d.", (int)numSamples);
    while (1)
    {
        // TODO: Clearly we need to add a better way to remain in sync with the main task...
        while (emulationPaused)
            rg_task_yield();
        psg_update((int16_t *)audioBuffer, numSamples, 0xFF);
        rg_audio_submit(audioBuffer, numSamples);
    }
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        // We must use previous update because at this point current has been wiped.
        rg_display_submit(updates[currentUpdate == updates[0]], 0);
    }
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    // We must use previous update because at this point current has been wiped.
    return rg_surface_save_image_file(updates[currentUpdate == updates[0]], filename, width, height);
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

void pce_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Overscan", "-", RG_DIALOG_FLAG_NORMAL, &overscan_update_cb},
        RG_DIALOG_END
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);
    overscan = rg_settings_get_number(NS_APP, SETTING_OVERSCAN, 1);

    updates[0] = rg_surface_create(XBUF_WIDTH, XBUF_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    updates[1] = rg_surface_create(XBUF_WIDTH, XBUF_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    currentUpdate = updates[0];

    updates[0]->data += 16;
    updates[0]->width -= 16;
    updates[1]->data += 16;
    updates[1]->width -= 16;

    uint16_t *palette = PalettePCE(16);
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = (palette[i] << 8) | (palette[i] >> 8);
        updates[0]->palette[i] = color;
        updates[1]->palette[i] = color;
    }
    free(palette);

    emulationPaused = true;
    rg_task_create("pce_sound", &audioTask, NULL, 2 * 1024, RG_TASK_PRIORITY_2, 1);

    InitPCE(app->sampleRate, true);

    if (rg_extension_match(app->romPath, "zip"))
    {
        void *data;
        size_t size;
        if (!rg_storage_unzip_file(app->romPath, NULL, &data, &size, RG_FILE_ALIGN_8KB))
            RG_PANIC("ROM file unzipping failed!");
        if (LoadCard(data, size) != 0)
            RG_PANIC("ROM loading failed");
    }
    else if (LoadFile(app->romPath) != 0)
    {
        RG_PANIC("ROM loading failed");
    }

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    app->tickRate = 60;
    app->frameskip = 1;

    emulationPaused = false;
    RunPCE();

    RG_PANIC("PCE-GO died.");
}
