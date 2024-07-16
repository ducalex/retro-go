#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "../components/snes9x/snes9x.h"
#include "../components/snes9x/soundux.h"
#include "../components/snes9x/memmap.h"
#include "../components/snes9x/apu.h"
#include "../components/snes9x/display.h"
#include "../components/snes9x/gfx.h"
#include "../components/snes9x/cpuexec.h"
#include "../components/snes9x/srtc.h"
#include "../components/snes9x/save.h"

#include "keymap_snes.h"
#include "shared.h"

#define AUDIO_LOW_PASS_RANGE ((60 * 65536) / 100)

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;

static bool apu_enabled = true;
static bool lowpass_filter = false;

static int keymap_id = 0;
static keymap_t keymap;

static const char *SETTING_KEYMAP = "keymap";
static const char *SETTING_APU_EMULATION = "apu";
// --- MAIN

static void update_keymap(int id)
{
    keymap_id = id % KEYMAPS_COUNT;
    keymap = KEYMAPS[keymap_id];
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    return S9xSaveState(filename);
}

static bool load_state_handler(const char *filename)
{
    return S9xLoadState(filename);
}

static bool reset_handler(bool hard)
{
    S9xReset();
    return true;
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static rg_gui_event_t apu_toggle_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        apu_enabled = !apu_enabled;
        rg_settings_set_number(NS_APP, SETTING_APU_EMULATION, apu_enabled);
    }

    strcpy(option->value, apu_enabled ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t lowpass_filter_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        lowpass_filter = !lowpass_filter;

    strcpy(option->value, lowpass_filter ? "On" : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t change_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        if (event == RG_DIALOG_PREV && --keymap_id < 0)
            keymap_id = KEYMAPS_COUNT - 1;
        if (event == RG_DIALOG_NEXT && ++keymap_id > KEYMAPS_COUNT - 1)
            keymap_id = 0;
        update_keymap(keymap_id);
        rg_settings_set_number(NS_APP, SETTING_KEYMAP, keymap_id);
        return RG_DIALOG_REDRAW;
    }

    if (event == RG_DIALOG_ENTER)
    {
        return RG_DIALOG_CANCEL;
    }

    if (option->arg == -1)
    {
        strcat(strcat(strcpy(option->value, "< "), keymap.name), " >");
    }
    else if (option->arg >= 0)
    {
        int local_button = keymap.keys[option->arg].local_mask;
        int mod_button = keymap.keys[option->arg].mod_mask;
        int snes9x_button = log2(keymap.keys[option->arg].snes9x_mask); // convert bitmask to bit number

        if (snes9x_button < 4 || (local_button & (RG_KEY_UP|RG_KEY_DOWN|RG_KEY_LEFT|RG_KEY_RIGHT)))
        {
            option->flags = RG_DIALOG_FLAG_HIDDEN;
            return RG_DIALOG_VOID;
        }

        if (keymap.keys[option->arg].mod_mask)
            sprintf(option->value, "%s + %s", rg_input_get_key_name(mod_button), rg_input_get_key_name(local_button));
        else
            sprintf(option->value, "%s", rg_input_get_key_name(local_button));

        option->label = SNES_BUTTONS[snes9x_button];
        option->flags = RG_DIALOG_FLAG_NORMAL;
    }

    return RG_DIALOG_VOID;
}

static rg_gui_event_t menu_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[20] = {
            {-1, "Profile", "<profile name>", RG_DIALOG_FLAG_NORMAL, &change_keymap_cb},
            {-2, "", NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
            {-3, "snes9x  ", "handheld", RG_DIALOG_FLAG_MESSAGE, NULL},
            {0, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {1, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {2, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {3, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {4, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {5, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {6, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {7, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {8, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {9, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {10, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {11, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {12, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {13, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {14, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {15, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            RG_DIALOG_END,
        };
        rg_gui_dialog("Controls", options, 0);
        return RG_DIALOG_REDRAW;
    }

    strcpy(option->value, keymap.name);
    return RG_DIALOG_VOID;
}

bool S9xInitDisplay(void)
{
    GFX.Pitch = SNES_WIDTH * 2;
    GFX.ZPitch = SNES_WIDTH;
    GFX.Screen = currentUpdate->data;
    GFX.SubScreen = malloc(GFX.Pitch * SNES_HEIGHT_EXTENDED);
    GFX.ZBuffer = malloc(GFX.ZPitch * SNES_HEIGHT_EXTENDED);
    GFX.SubZBuffer = malloc(GFX.ZPitch * SNES_HEIGHT_EXTENDED);
    return GFX.Screen && GFX.SubScreen && GFX.ZBuffer && GFX.SubZBuffer;
}

void S9xDeinitDisplay(void)
{
}

uint32_t S9xReadJoypad(int32_t port)
{
    if (port != 0)
        return 0;

    uint32_t joystick = rg_input_read_gamepad();
    uint32_t joypad = 0;

    for (int i = 0; i < RG_COUNT(keymap.keys); ++i)
    {
        uint32_t bitmask = keymap.keys[i].local_mask | keymap.keys[i].mod_mask;
        if (bitmask && bitmask == (joystick & bitmask))
        {
            joypad |= keymap.keys[i].snes9x_mask;
        }
    }

    return joypad;
}

bool S9xReadMousePosition(int32_t which1, int32_t *x, int32_t *y, uint32_t *buttons)
{
    return false;
}

bool S9xReadSuperScopePosition(int32_t *x, int32_t *y, uint32_t *buttons)
{
    return false;
}

bool JustifierOffscreen(void)
{
    return true;
}

void JustifierButtons(uint32_t *justifiers)
{
    (void)justifiers;
}

#ifdef USE_BLARGG_APU
static void S9xAudioCallback(void)
{
    S9xFinalizeSamples();
    size_t available_samples = S9xGetSampleCount();
    S9xMixSamples((void *)audioBuffer, available_samples);
    rg_audio_submit(audioBuffer, available_samples >> 1);
}
#endif

void snes_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Audio enable", "-", RG_DIALOG_FLAG_NORMAL, &apu_toggle_cb},
        {0, "Audio filter", "-", RG_DIALOG_FLAG_NORMAL, &lowpass_filter_cb},
        {0, "Controls    ", "-", RG_DIALOG_FLAG_NORMAL, &menu_keymap_cb},
        RG_DIALOG_END,
    };
    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);

    apu_enabled = rg_settings_get_number(NS_APP, SETTING_APU_EMULATION, 1);

    updates[0] = rg_surface_create(SNES_WIDTH, SNES_HEIGHT_EXTENDED, RG_PIXEL_565_LE, 0);
    updates[0]->height = SNES_HEIGHT;
    currentUpdate = updates[0];

    update_keymap(rg_settings_get_number(NS_APP, SETTING_KEYMAP, 0));

    Settings.CyclesPercentage = 100;
    Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.ControllerOption = SNES_JOYPAD;
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
    Settings.SoundPlaybackRate = AUDIO_SAMPLE_RATE;
    Settings.DisableSoundEcho = false;
    Settings.InterpolatedSound = true;
#ifdef USE_BLARGG_APU
    Settings.SoundInputRate = AUDIO_SAMPLE_RATE;
#endif

    if (!S9xInitDisplay())
        RG_PANIC("Display init failed!");

    if (!S9xInitMemory())
        RG_PANIC("Memory init failed!");

    if (!S9xInitAPU())
        RG_PANIC("APU init failed!");

    if (!S9xInitSound(0, 0))
        RG_PANIC("Sound init failed!");

    if (!S9xInitGFX())
        RG_PANIC("Graphics init failed!");

    const char *filename = app->romPath;

    if (rg_extension_match(filename, "zip"))
    {
        free(Memory.ROM); // Would be nice to reuse it directly...
        if (!rg_storage_unzip_file(filename, NULL, (void **)&Memory.ROM, &Memory.ROM_Size))
            RG_PANIC("ROM file unzipping failed!");
        filename = NULL;
    }

    if (!LoadROM(filename))
        RG_PANIC("ROM loading failed!");

#ifdef USE_BLARGG_APU
    S9xSetSamplesAvailableCallback(S9xAudioCallback);
#else
    S9xSetPlaybackRate(Settings.SoundPlaybackRate);
#endif

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    app->tickRate = Memory.ROMFramesPerSecond;
    app->frameskip = 3;

    bool menuCancelled = false;
    bool menuPressed = false;
    int skipFrames = 0;

    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (menuPressed && !(joystick & RG_KEY_MENU))
        {
            if (!menuCancelled)
            {
                rg_task_delay(50);
                rg_gui_game_menu();
            }
            menuCancelled = false;
        }
        else if (joystick & RG_KEY_OPTION)
        {
            rg_gui_options_menu();
        }

        menuPressed = joystick & RG_KEY_MENU;

        if (menuPressed && joystick & ~RG_KEY_MENU)
        {
            menuCancelled = true;
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = (skipFrames == 0);
        bool slowFrame = false;

        IPPU.RenderThisFrame = drawFrame;
        GFX.Screen = currentUpdate->data;

        S9xMainLoop();

        if (drawFrame)
        {
            slowFrame = !rg_display_sync(false);
            rg_display_submit(currentUpdate, 0);
        }

    #ifndef USE_BLARGG_APU
        if (apu_enabled && lowpass_filter)
            S9xMixSamplesLowPass((void *)audioBuffer, AUDIO_BUFFER_LENGTH << 1, AUDIO_LOW_PASS_RANGE);
        else if (apu_enabled)
            S9xMixSamples((void *)audioBuffer, AUDIO_BUFFER_LENGTH << 1);
    #endif

        rg_system_tick(rg_system_timer() - startTime);

    #ifndef USE_BLARGG_APU
        if (apu_enabled)
            rg_audio_submit(audioBuffer, AUDIO_BUFFER_LENGTH);
    #endif

        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (app->tickRate * app->speed);
            int elapsed = rg_system_timer() - startTime;
            if (app->frameskip > 0)
                skipFrames = app->frameskip;
            else if (elapsed > frameTime + 1500) // Allow some jitter
                skipFrames = 1; // (elapsed / frameTime)
            else if (drawFrame && slowFrame)
                skipFrames = 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }
    }
}
