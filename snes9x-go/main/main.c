#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "snes9x.h"
#include "soundux.h"
#include "memmap.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "cpuexec.h"
#include "srtc.h"
#include "save.h"

#include "keymap.h"

#define AUDIO_SAMPLE_RATE (32040)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_LOW_PASS_RANGE ((60 * 65536) / 100)

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];
static rg_app_t *app;

static bool apu_enabled = true;
static bool lowpass_filter = false;
static int frameskip = 4;

bool overclock_cycles = false;
int one_c = 4, slow_one_c = 5, two_c = 6;

static int keymap_id = 0;
static keymap_t keymap;

static const char *SETTING_FRAMESKIP = "frameskip";
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
    return rg_display_save_frame(filename, currentUpdate, width, height);
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

static rg_gui_event_t apu_toggle_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        apu_enabled = !apu_enabled;
        rg_settings_set_number(NS_APP, SETTING_APU_EMULATION, apu_enabled);
    }

    sprintf(option->value, "%s", apu_enabled ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t frameskip_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        frameskip += (event == RG_DIALOG_PREV) ? -1 : 1;
        frameskip = RG_MAX(frameskip, 1);
        rg_settings_set_number(NS_APP, SETTING_FRAMESKIP, frameskip);
    }

    sprintf(option->value, "%d", frameskip);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t lowpass_filter_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        lowpass_filter = !lowpass_filter;

    sprintf(option->value, "%s", lowpass_filter ? "On" : "Off");

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
        return RG_DIALOG_CLOSE;
    }
    else if (event == RG_DIALOG_ENTER || event == RG_DIALOG_ALT)
    {
        return RG_DIALOG_DISMISS;
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t menu_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        bool dismissed = false;

        while (!dismissed)
        {
            rg_gui_option_t options[20];
            rg_gui_option_t *option = options;
            char values[16][20];
            char profile[32];

            option->label = "Profile";
            option->value = strcat(strcat(strcpy(profile, "< "), keymap.name), " >");
            option->flags = RG_DIALOG_FLAG_NORMAL;
            option->update_cb = &change_keymap_cb;
            option++;

            option->label = "";
            option->value = NULL;
            option->flags = RG_DIALOG_FLAG_NORMAL;
            option->update_cb = &change_keymap_cb;
            option++;

            option->label = "snes9x  ";
            option->value = "handheld";
            option->flags = RG_DIALOG_FLAG_NORMAL;
            option->update_cb = &change_keymap_cb;
            option++;

            for (int i = 0; i < RG_COUNT(keymap.keys); i++)
            {
                int local_button = keymap.keys[i].local_mask;
                int mod_button = keymap.keys[i].mod_mask;
                int snes9x_button = log2(keymap.keys[i].snes9x_mask); // convert bitmask to bit number

                // Empty mapping
                if (snes9x_button < 4)
                    continue;

                // For now we don't display the D-PAD because it doesn't fit on large font
                if (local_button & (RG_KEY_UP|RG_KEY_DOWN|RG_KEY_LEFT|RG_KEY_RIGHT))
                    continue;

                if (keymap.keys[i].mod_mask)
                    sprintf(values[i], "%s + %s", rg_input_get_key_name(mod_button), rg_input_get_key_name(local_button));
                else
                    sprintf(values[i], "%s", rg_input_get_key_name(local_button));

                option->label = SNES_BUTTONS[snes9x_button];
                option->value = values[i];
                option->flags = RG_DIALOG_FLAG_NORMAL;
                option->update_cb = &change_keymap_cb;
                option++;
            }

            *option++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

            dismissed = rg_gui_dialog("Controls", options, 0) == RG_DIALOG_CANCELLED;
            rg_display_queue_update(currentUpdate, NULL);
            rg_display_sync();
        }
    }

    strcpy(option->value, keymap.name);

    return RG_DIALOG_VOID;
}

bool S9xInitDisplay(void)
{
    GFX.Pitch = SNES_WIDTH * 2;
    GFX.ZPitch = SNES_WIDTH;
    GFX.Screen = currentUpdate->buffer;
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
    rg_audio_sample_t mixbuffer[available_samples];
    S9xMixSamples(&mixbuffer, available_samples);
    rg_audio_submit(mixbuffer, available_samples >> 1);
}
#endif

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {2, "APU enable", (char *)"", 1, &apu_toggle_cb},
        {2, "LP Filter", (char*)"", 1, &lowpass_filter_cb},
        {2, "Frameskip", (char *)"", 1, &frameskip_cb},
        {2, "Controls", (char *)"", 1, &menu_keymap_cb},
        RG_DIALOG_CHOICE_LAST,
    };
    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    frameskip = rg_settings_get_number(NS_APP, SETTING_FRAMESKIP, frameskip);
    apu_enabled = rg_settings_get_number(NS_APP, SETTING_APU_EMULATION, 1);

    updates[0].buffer = malloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2);

    rg_display_set_source_format(SNES_WIDTH, SNES_HEIGHT, 0, 0, SNES_WIDTH * 2, RG_PIXEL_565_LE);

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

    if (!LoadROM(app->romPath))
        RG_PANIC("ROM loading failed!");

#ifdef USE_BLARGG_APU
    S9xSetSamplesAvailableCallback(S9xAudioCallback);
#else
    rg_audio_sample_t mixbuffer[AUDIO_BUFFER_LENGTH];
    S9xSetPlaybackRate(Settings.SoundPlaybackRate);
#endif

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    app->refreshRate = Memory.ROMFramesPerSecond;

    bool menuCancelled = false;
    bool menuPressed = false;
    int frames = 0;

    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (menuPressed && !(joystick & RG_KEY_MENU))
        {
            if (!menuCancelled)
            {
                usleep(50 * 1000);
                rg_gui_game_menu();
                rg_audio_set_sample_rate(app->sampleRate * app->speed);
            }
            menuCancelled = false;
        }
        else if (joystick & RG_KEY_OPTION)
        {
            rg_gui_options_menu();
            rg_audio_set_sample_rate(app->sampleRate * app->speed);
        }

        menuPressed = joystick & RG_KEY_MENU;

        if (menuPressed && joystick & ~RG_KEY_MENU)
        {
            menuCancelled = true;
        }

        int64_t startTime = rg_system_timer();

        IPPU.RenderThisFrame = (frames++ % frameskip) == 0;
        GFX.Screen = currentUpdate->buffer;

    #ifndef USE_BLARGG_APU
        // Fully disabling the APU isn't possible at this point.
        if (!apu_enabled)
            Settings.APUEnabled = false, IAPU.APUExecuting = false;
    #endif

        S9xMainLoop();

        if (IPPU.RenderThisFrame)
            rg_display_queue_update(currentUpdate, NULL);

    #ifndef USE_BLARGG_APU
        if (apu_enabled && lowpass_filter)
            S9xMixSamplesLowPass((void *)mixbuffer, AUDIO_BUFFER_LENGTH << 1, AUDIO_LOW_PASS_RANGE);
        else if (apu_enabled)
            S9xMixSamples((void *)mixbuffer, AUDIO_BUFFER_LENGTH << 1);
    #endif

        int elapsed = rg_system_timer() - startTime;

    #ifndef USE_BLARGG_APU
        if (apu_enabled)
            rg_audio_submit(mixbuffer, AUDIO_BUFFER_LENGTH);
    #endif

        rg_system_tick(elapsed);
    }
}
