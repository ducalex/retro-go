#include <rg_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "StellaDS.h"
#include "Console.hxx"
#include "TIA.hxx"
#include "Cart.hxx"
#include "Event.hxx"
#include "StellaEvent.hxx"
#include "savestate.h"
#include "stella_config.h"

#define AUDIO_SAMPLE_RATE (22050)
#define AUDIO_BUFFER_SIZE (AUDIO_SAMPLE_RATE / 50) // Max samples for PAL

#if RG_SCREEN_PIXEL_FORMAT == 0
#define STELLA_PIXEL_FORMAT RG_PIXEL_PAL565_BE
#else
#define STELLA_PIXEL_FORMAT RG_PIXEL_PAL565_LE
#endif

static rg_app_t *app;
extern "C" rg_surface_t *screen;
static uint16_t *audio_buffer;
DRAM_ATTR static uint16_t screen_palette[256];
static int32_t paddle_resistance = 500000; // Start at center

extern Console* theConsole;
extern TIA theTIA;
extern Event myStellaEvent;
extern void Tia_process_wave(void);
extern uint16_t *aptr;
extern FICA2600 *vcsromlist;
extern uint8_t *BG_GFX;

static bool load_state_handler(const char *filename) {
    LoadState(filename);
    return true;
}

static bool save_state_handler(const char *filename) {
    SaveState(filename);
    return true;
}

static bool reset_handler(bool hard) {
    if (theConsole) theConsole->system().reset();
    return true;
}

static bool screenshot_handler(const char *filename, int width, int height) {
    if (width <= 0) width = 160;
    if (height <= 0) height = 240;
    return rg_surface_save_image_file(screen, filename, width, height);
}

static void event_handler(int event, void *data) {
    if (event == RG_EVENT_REDRAW) {
        rg_display_submit(screen, 0);
    }
}

#define SETTING_P1_DIFF "P1Diff"

static rg_gui_event_t p1_diff_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int diff = rg_settings_get_number(NS_APP, SETTING_P1_DIFF, 0);
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        diff = !diff;
        rg_settings_set_number(NS_APP, SETTING_P1_DIFF, diff);
    }
    myStellaEvent.set(Event::ConsoleLeftDifficultyA, diff == 1);
    myStellaEvent.set(Event::ConsoleLeftDifficultyB, diff == 0);
    strcpy(option->value, diff ? "A (Pro)" : "B (Novice)");
    return RG_DIALOG_VOID;
}

static void options_handler(rg_gui_option_t *dest) {
    *dest = (rg_gui_option_t){0};
    dest->label = "P1 Difficulty";
    dest->value = (char *)"-";
    dest->flags = RG_DIALOG_FLAG_NORMAL;
    dest->update_cb = &p1_diff_update_cb;
    dest++;

    *dest = (rg_gui_option_t)RG_DIALOG_END;
}


extern "C" void app_main(void) {
    rg_config_t config;
    memset(&config, 0, sizeof(config));
    config.sampleRate = AUDIO_SAMPLE_RATE;
    config.frameRate = 60;
    config.storageRequired = true;
    config.romRequired = true;
    config.handlers.loadState = &load_state_handler;
    config.handlers.saveState = &save_state_handler;
    config.handlers.reset = &reset_handler;
    config.handlers.screenshot = &screenshot_handler;
    config.handlers.event = &event_handler;
    config.handlers.options = &options_handler;

    app = rg_system_init(&config);

    if (!rg_settings_exists(NS_APP, "DispScaling")) {
        rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);
    }

    RG_LOGI("Stella initialized.");

    // Late allocation of massive structures in PSRAM
    if (vcsromlist == NULL) {
        vcsromlist = (FICA2600*)rg_alloc(MAX_ROMS_PER_DIRECTORY * sizeof(FICA2600), MEM_SLOW);
    }
    if (allConfigs_ptr == NULL) {
        allConfigs_ptr = (AllConfig_t*)rg_alloc(sizeof(AllConfig_t), MEM_SLOW);
        memset(allConfigs_ptr, 0, sizeof(AllConfig_t));
    }

    RG_LOGI("Loading ROM: %s\n", app->romPath);

    size_t rom_size;
    void *rom_data;

    if (rg_extension_match(app->romPath, "zip")) {
        if (!rg_storage_unzip_file(app->romPath, NULL, &rom_data, &rom_size, 0)) {
            RG_PANIC("ROM unzip failed!");
        }
    } else {
        if (!rg_storage_read_file(app->romPath, &rom_data, &rom_size, 0)) {
            RG_PANIC("ROM load failed!");
        }
    }

    RG_LOGI("Creating Console...\n");
    theConsole = new Console((const uInt8*)rom_data, rom_size, "noname");
    
    RG_LOGI("Initializing Audio...\n");
    dsInstallSoundEmuFIFO();
    
    RG_LOGI("Creating Display Surface (160x240)...\n");
    screen = rg_surface_create(160, 240, STELLA_PIXEL_FORMAT, MEM_SLOW);
    screen->palette = screen_palette;
    
    // Build the palette in the byte order consumed by the target display so
    // the scaler does not have to swap every output pixel.
    const uInt32* gamePalette = theTIA.palette();
    for (int i = 0; i < 256; i++) {
        uint32_t c = gamePalette[i];
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;
        uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        if (STELLA_PIXEL_FORMAT == RG_PIXEL_PAL565_BE) {
            color = (color << 8) | (color >> 8);
        }
        screen_palette[i] = color;
    }

    audio_buffer = (uint16_t*)rg_alloc(AUDIO_BUFFER_SIZE * 2 * sizeof(uint16_t), MEM_SLOW);

    // Ensure sound is enabled for the current session
    myCartInfo.soundQuality = SOUND_WAVE;

    if (app->bootFlags & RG_BOOT_RESUME) {
        rg_emu_load_state(app->saveSlot);
    }

    // Initialize difficulty settings from stored configuration
    int p1_diff = rg_settings_get_number(NS_APP, SETTING_P1_DIFF, 0);
    myStellaEvent.set(Event::ConsoleLeftDifficultyA, p1_diff == 1);
    myStellaEvent.set(Event::ConsoleLeftDifficultyB, p1_diff == 0);

    RG_LOGI("Emulation loop starting...\n");
    int frameCount = 0;
    int target_fps = 60;

    while (true) {
        int64_t startTime = rg_system_timer();

        uint32_t joy = rg_input_read_gamepad();
        if (joy & (RG_KEY_MENU | RG_KEY_OPTION)) {
            if (joy & RG_KEY_MENU) rg_gui_game_menu();
            else rg_gui_options_menu();
            continue;
        }

        myStellaEvent.set(Event::JoystickZeroUp,    joy & RG_KEY_UP);
        myStellaEvent.set(Event::JoystickZeroDown,  joy & RG_KEY_DOWN);
        myStellaEvent.set(Event::JoystickZeroLeft,  joy & RG_KEY_LEFT);
        myStellaEvent.set(Event::JoystickZeroRight, joy & RG_KEY_RIGHT);
        myStellaEvent.set(Event::JoystickZeroFire,  joy & (RG_KEY_A | RG_KEY_B));
        myStellaEvent.set(Event::ConsoleSelect,     joy & RG_KEY_SELECT);
        myStellaEvent.set(Event::ConsoleReset,      joy & RG_KEY_START);

        // Paddle Support: Map D-pad and A/B buttons to Paddle 0
        // Typical range is 0 to 1,000,000 ohms.
        if (joy & RG_KEY_RIGHT) paddle_resistance = (paddle_resistance < 15000) ? 0 : paddle_resistance - 15000;
        if (joy & RG_KEY_LEFT)  paddle_resistance = (paddle_resistance > 1000000 - 15000) ? 1000000 : paddle_resistance + 15000;
        
        myStellaEvent.set(Event::PaddleZeroResistance, paddle_resistance);
        myStellaEvent.set(Event::PaddleZeroFire, (joy & (RG_KEY_A | RG_KEY_B)) ? 1 : 0);
        
        if (theConsole) {
            target_fps = (myCartInfo.tv_type == PAL) ? 50 : 60;
            // Point screen to current TIA buffer
            screen->data = theTIA.myCurrentFrameBuffer[theTIA.myCurrentFrame];
            // StellaDS logic uses BG_GFX as the render target
            BG_GFX = (uint8_t *)screen->data;

            // Update surface height to match game's active lines for scaling
            screen->height = myCartInfo.displayNumScalines;

            theConsole->update();
        }

        rg_display_submit(screen, 0);

        if (myCartInfo.soundQuality) {
            int samples_per_frame = AUDIO_SAMPLE_RATE / target_fps;
            static rg_audio_frame_t frames[1024]; 
            for (int i = 0; i < samples_per_frame; i++) {
                Tia_process_wave();
                // Stella produces 0..3840 unsigned. Center at 1920, no amplification.
                int16_t sample = (int16_t)((int32_t)(*aptr) - 1920);
                frames[i].left = sample;
                frames[i].right = sample;
            }
            rg_audio_submit(frames, samples_per_frame);
        }

        rg_system_tick(rg_system_timer() - startTime);
        
        int64_t target_frame_time = 1000000 / target_fps;
        int64_t frameTime = rg_system_timer() - startTime;
        if (frameTime < target_frame_time) {
            rg_usleep(target_frame_time - frameTime);
        }

        if (++frameCount % 60 == 0) {
            // RG_LOGI("Emulation: %d frames\n", frameCount);
        }
    }
}
