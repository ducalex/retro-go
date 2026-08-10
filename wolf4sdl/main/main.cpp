/*
 * WOLF4SDL FOR RETRO-GO
 * 
 * To build for Spear of Destiny (SOD) instead of Wolfenstein 3D:
 * 1. Open "components/wolf4sdl/wl_main/version.h"
 * 2. Uncomment the line: #define SPEAR
 */

#include <rg_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSIONALREADYCHOSEN

#include "wl_def.h"
#include "wl_main.h"

#define AUDIO_SAMPLE_RATE 22050
#define SYSTEM_MENU_HOLD_US 500000
#define NATIVE_TAP_MIN_US 1000

#if RG_SCREEN_PIXEL_FORMAT == 0
#define FB_PIXEL_FORMAT RG_PIXEL_PAL565_BE
#else
#define FB_PIXEL_FORMAT RG_PIXEL_PAL565_LE
#endif

static rg_surface_t *update;
static rg_app_t *app;

extern "C" void SDL_RG_SetSurface(rg_surface_t *surf);
extern "C" void SDL_RG_ResetTiming();

// Standard Retro-Go button mapping
typedef struct {
    int rg_key;
    SDL_Scancode scancode;
    SDL_Keycode keycode;
} rg_bind_t;

static rg_bind_t binds[] = {
    {RG_KEY_UP, SDL_SCANCODE_UP, SDLK_UP},
    {RG_KEY_DOWN, SDL_SCANCODE_DOWN, SDLK_DOWN},
    {RG_KEY_LEFT, SDL_SCANCODE_LEFT, SDLK_LEFT},
    {RG_KEY_RIGHT, SDL_SCANCODE_RIGHT, SDLK_RIGHT},
    {RG_KEY_A, SDL_SCANCODE_LCTRL, SDLK_LCTRL},   // Fire
    {RG_KEY_B, SDL_SCANCODE_LALT, SDLK_LALT},     // Strafe
    {RG_KEY_X, SDL_SCANCODE_SPACE, SDLK_SPACE},   // Open/Use
    {RG_KEY_Y, SDL_SCANCODE_Y, SDLK_y},           // Weapon Cycle
    {RG_KEY_L, SDL_SCANCODE_COMMA, SDLK_COMMA},   // Strafe left
    {RG_KEY_R, SDL_SCANCODE_PERIOD, SDLK_PERIOD}, // Strafe right
    {RG_KEY_START, SDL_SCANCODE_SPACE, SDLK_SPACE}, // Open/Use
    {RG_KEY_SELECT, SDL_SCANCODE_Y, SDLK_y},      // Weapon Cycle
    {0, SDL_SCANCODE_UNKNOWN, (SDLKey)0}
};

static uint32_t last_input = 0;
static uint32_t suppressed_input = 0;
static int64_t menu_press_time = 0;
static int64_t option_press_time = 0;
static bool menu_pressed = false;
static bool option_pressed = false;
static bool pending_menu_keyup = false;
static int64_t menu_keydown_time = 0;

static void set_key_event(SDL_Event *event, bool pressed,
                          SDL_Scancode scancode, SDL_Keycode keycode)
{
    event->type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event->key.keysym.scancode = scancode;
    event->key.keysym.sym = keycode;
    event->key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
}

static void clear_native_input()
{
    memset((void *)Keyboard, 0, SDLK_LAST * sizeof(Keyboard[0]));
    LastScan = sc_None;
    LastASCII = key_None;
    last_input = 0;
    menu_pressed = false;
    option_pressed = false;
    pending_menu_keyup = false;
}

static void open_system_menu(bool options)
{
    // Retro-Go's dialogs are blocking. Release every native key before
    // entering, then discard any buttons still held when the dialog closes.
    clear_native_input();
    if (options)
        rg_gui_options_menu();
    else
        rg_gui_game_menu();

    clear_native_input();
    suppressed_input = rg_input_read_gamepad();
    lasttimecount = GetTimeCount();
    SDL_RG_ResetTiming();
}

extern "C" int SDL_RG_PollEvent(SDL_Event *event)
{
    uint32_t raw_input = rg_input_read_gamepad();
    suppressed_input &= raw_input;
    uint32_t current_input = raw_input & ~suppressed_input;
    int64_t now = rg_system_timer();

    // A short MENU press remains Wolf3D Escape. Defer it until release so a
    // long hold cannot open both Wolf's menu and Retro-Go's game menu.
    if (pending_menu_keyup) {
        if (now - menu_keydown_time < NATIVE_TAP_MIN_US)
            return 0;

        pending_menu_keyup = false;
        set_key_event(event, false, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE);
        return 1;
    }

    if (current_input & RG_KEY_MENU) {
        if (!menu_pressed) {
            menu_pressed = true;
            menu_press_time = now;
        } else if (now - menu_press_time >= SYSTEM_MENU_HOLD_US) {
            open_system_menu(false);
            return 0;
        }
    } else if (menu_pressed) {
        menu_pressed = false;
        pending_menu_keyup = true;
        menu_keydown_time = now;
        set_key_event(event, true, SDL_SCANCODE_ESCAPE, SDLK_ESCAPE);
        return 1;
    }

    // OPTION is Wolf3D's hold-to-run key, so assert Shift immediately. If it
    // becomes a long hold, clear Shift before opening Retro-Go's options.
    if (current_input & RG_KEY_OPTION) {
        if (!option_pressed) {
            option_pressed = true;
            option_press_time = now;
            set_key_event(event, true, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT);
            return 1;
        } else if (now - option_press_time >= SYSTEM_MENU_HOLD_US) {
            open_system_menu(true);
            return 0;
        }
    } else if (option_pressed) {
        option_pressed = false;
        set_key_event(event, false, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT);
        return 1;
    }

    uint32_t changed = current_input ^ last_input;
    
    if (changed) {
        for (int i = 0; binds[i].rg_key; i++) {
            if (changed & binds[i].rg_key) {
                set_key_event(event, current_input & binds[i].rg_key,
                              binds[i].scancode, binds[i].keycode);
                
                last_input ^= binds[i].rg_key;
                return 1;
            }
        }
    }
    return 0;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_surface_save_image_file(update, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    rg_gui_alert("Not implemented", "Please use the in-game menu");
    return false;
}

static bool load_state_handler(const char *filename)
{
    rg_gui_alert("Not implemented", "Please use the in-game menu");
    return false;
}

static bool reset_handler(bool hard)
{
    (void)hard;
    rg_gui_alert("Not implemented", "Please use the in-game menu");
    return false;
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(update, 0);
    }
}

static void options_handler(rg_gui_option_t *dest)
{
    *dest++ = (rg_gui_option_t)RG_DIALOG_END;
}

static bool wolf_data_present(const char *directory)
{
    static const char *headers[] = {
        "vgahead.wl6",
        "vgahead.wl1",
        "vgahead.sod",
    };
    char path[400];

    for (size_t i = 0; i < sizeof(headers) / sizeof(headers[0]); ++i)
    {
        int written = snprintf(path, sizeof(path), "%s%s",
                               directory, headers[i]);
        if (written < 0 || (size_t)written >= sizeof(path))
            continue;
        if (rg_storage_exists(path))
            return true;
    }
    return false;
}

static bool select_wolf_data_directory(char *destination, size_t size,
                                       const char *base_directory)
{
    char base[350];
    char data[350];
    size_t length;
    int written;

    if (!destination || size == 0 || !base_directory || !base_directory[0])
        return false;

    written = snprintf(base, sizeof(base), "%s", base_directory);
    if (written < 0 || (size_t)written >= sizeof(base))
        return false;

    length = strlen(base);
    if (length > 0 && base[length - 1] != '/')
    {
        if (length + 1 >= sizeof(base))
            return false;
        base[length++] = '/';
        base[length] = 0;
    }

    // Prefer the tidy layout, but retain compatibility with data files stored
    // directly in the game's root directory.
    written = snprintf(data, sizeof(data), "%sdata/", base);
    if (written < 0 || (size_t)written >= sizeof(data))
        return false;

    if (wolf_data_present(data))
    {
        length = strlen(data) + 1;
        if (length > size)
            return false;
        memcpy(destination, data, length);
        return true;
    }
    if (wolf_data_present(base))
    {
        length = strlen(base) + 1;
        if (length > size)
            return false;
        memcpy(destination, base, length);
        return true;
    }
    return false;
}

extern int wolf_main(int argc, char *argv[]);

extern "C" void app_main()
{
    rg_config_t config;
    memset(&config, 0, sizeof(config));
    config.sampleRate = AUDIO_SAMPLE_RATE;
    config.frameRate = 70; 
    config.storageRequired = true;
    config.romRequired = false;
    config.handlers.loadState = &load_state_handler;
    config.handlers.saveState = &save_state_handler;
    config.handlers.reset = &reset_handler;
    config.handlers.screenshot = &screenshot_handler;
    config.handlers.event = &event_handler;
    config.handlers.options = &options_handler;
    
    app = rg_system_init(&config);

    // Constant internal resolution for consistency.
    int width = 320;
    int height = 240;

    // Use full scaling as the initial preset, while preserving the user's
    // selection from Retro-Go's Options menu on subsequent launches.
    if (!rg_settings_exists(NS_APP, "DispScaling"))
        rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);

    update = rg_surface_create(width, height, FB_PIXEL_FORMAT, MEM_FAST);

    SDL_RG_SetSurface(update);

    char current_datadir[350] = {0};
    bool data_found = false;

    const char *romPath = app->romPath;
    if (romPath && romPath[0])
    {
        char baseDir[350];
        rg_stat_t rom_stat = rg_storage_stat(romPath);

        snprintf(baseDir, sizeof(baseDir), "%s", romPath);
        if (!rom_stat.is_dir)
        {
            char *lastSlash = strrchr(baseDir, '/');
            if (lastSlash)
                *(lastSlash + 1) = 0;
            else
                baseDir[0] = 0;
        }

        data_found = select_wolf_data_directory(
                current_datadir, sizeof(current_datadir), baseDir);
    }

    // Direct .bin launches have no launcher ROM path. Also use this as a
    // fallback if a supplied boot path does not lead to valid game data.
    if (!data_found)
    {
        data_found = select_wolf_data_directory(
                current_datadir, sizeof(current_datadir),
                RG_BASE_PATH_ROMS "/wolf3d/");
    }

    if (!data_found)
    {
        rg_gui_alert("Wolf3D data not found",
                     "Place the game files in\n/wolf3d/data/ or /wolf3d/.");
        rg_system_exit();
    }

    RG_LOGI("Using game data from: %s", current_datadir);

    // Prepare arguments for Wolf4SDL
    char arg0[] = "wolf4sdl";
    char arg1[] = "--res";
    char arg2[] = "320";
    char arg3[] = "240";
    char arg4[] = "--samplerate";
    char arg5[] = "11025";
    char arg6[] = "--configdir";
    char arg7[256];
    strcpy(arg7, RG_BASE_PATH_CONFIG "/wolf3d/");
    char arg8[] = "--savedir";
    char arg9[256];
    strcpy(arg9, RG_BASE_PATH_SAVES "/wolf3d/");
    char arg10[] = "--datadir";
    char arg11[350];
    strcpy(arg11, current_datadir);
    
    char *argv[] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, NULL };
    int argc = 12;

    // Call Wolf4SDL main
    wolf_main(argc, argv);

    rg_system_exit();
}
