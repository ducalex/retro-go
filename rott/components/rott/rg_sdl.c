#include "rt_def.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <rg_system.h>
#include <rg_display.h>
#include <rg_gui.h>
#include <rg_surface.h>
#include "rt_main.h"
#include "rt_util.h"
#include <stdlib.h>
#include <string.h>

static SDL_Surface *sdl_screen = NULL;
static rg_surface_t *rg_surface = NULL;

#if RG_SCREEN_PIXEL_FORMAT == 0
#define ROTT_DISPLAY_FORMAT RG_PIXEL_PAL565_BE
#else
#define ROTT_DISPLAY_FORMAT RG_PIXEL_PAL565_LE
#endif

extern volatile int *Keyboard; 
extern volatile int *Keystate;
extern volatile int LastScan;
extern volatile int *KeyboardQueue;
extern volatile int Keyhead;
extern volatile int Keytail;

#ifndef KEYQMAX
#define KEYQMAX 256
#endif

static const struct {
    int rg_key;
    int scancode;
} keymap[] = {
    {RG_KEY_UP,     0x48}, 
    {RG_KEY_DOWN,   0x50}, 
    {RG_KEY_LEFT,   0x4b}, 
    {RG_KEY_RIGHT,  0x4d}, 
    {RG_KEY_A,      0x1d}, // Ctrl (Attack)
    {RG_KEY_A,      0x1c}, // Enter (Menu Select)
    {RG_KEY_B,      0x36}, // RShift (Run)
    {RG_KEY_B,      0x38}, // Alt (Strafe)
    {RG_KEY_X,      0x39}, // Space (Use)
    {RG_KEY_Y,      0x1c}, // Enter (Weapon Swap)
    {RG_KEY_SELECT, 0x1c}, // Enter (Weapon Swap)
    {RG_KEY_START,  0x39}, // Space (Use)
    {RG_KEY_MENU,   0x01}, // Esc (Menu)
    {RG_KEY_OPTION, 0x0f}, // Tab (Map)
    {RG_KEY_L,      0x33}, // , (Strafe Left)
    {RG_KEY_R,      0x34}, // . (Strafe Right)
};

static uint32_t last_joystick = 0;

typedef struct {
    bool down;
    bool long_press;
    bool release_pending;
    int64_t pressed_at;
    int64_t release_at;
} system_key_state_t;

static system_key_state_t menu_key;
static system_key_state_t option_key;
static uint32_t suppressed_keys;

#define SYSTEM_MENU_HOLD_US 1000000
#define ENGINE_TAP_US       35000

static void set_engine_key(int scancode, bool pressed) {
    Keystate[scancode] = pressed;
    Keyboard[scancode] = pressed;

    if (pressed)
        LastScan = scancode;

    KeyboardQueue[Keytail] = pressed ? scancode : (scancode | 0x80);
    Keytail = (Keytail + 1) & (KEYQMAX - 1);
}

static void start_engine_tap(int scancode, system_key_state_t *state,
                             int64_t now) {
    set_engine_key(scancode, true);
    state->release_pending = true;
    state->release_at = now + ENGINE_TAP_US;
}

static void finish_engine_tap(int scancode, system_key_state_t *state,
                              int64_t now) {
    if (state->release_pending && now >= state->release_at) {
        set_engine_key(scancode, false);
        state->release_pending = false;
    }
}

static void clear_engine_input(void) {
    memset((void *)Keyboard, 0, 128 * sizeof(*Keyboard));
    memset((void *)Keystate, 0, 128 * sizeof(*Keystate));
    LastScan = 0;
    Keyhead = Keytail = 0;
}

static uint32_t handle_system_key(uint32_t joystick, uint32_t key,
                                  int scancode, system_key_state_t *state,
                                  void (*open_menu)(void)) {
    bool pressed = (joystick & key) != 0;
    int64_t now = rg_system_timer();

    if (pressed && !state->down) {
        state->down = true;
        state->long_press = false;
        state->pressed_at = now;
        Keystate[scancode] = 0;
        Keyboard[scancode] = 0;
    }

    if (pressed && !state->long_press &&
        now - state->pressed_at >= SYSTEM_MENU_HOLD_US) {
        state->long_press = true;

        int64_t paused_at = rg_system_timer();
        ROTT_SuspendForSystemMenu();
        open_menu();
        ROTT_ResumeFromSystemMenu(rg_system_timer() - paused_at);

        // Discard stale engine events and suppress every button still held as
        // the dialog closes until that physical button has been released.
        clear_engine_input();
        joystick = rg_input_read_gamepad();
        suppressed_keys |= joystick;
        joystick &= ~suppressed_keys;
        pressed = false;
    }

    if (!pressed && state->down) {
        if (!state->long_press)
            start_engine_tap(scancode, state, now);

        state->down = false;
        state->long_press = false;
    }

    return joystick;
}

static void update_input(void) {
    int64_t now = rg_system_timer();
    finish_engine_tap(0x01, &menu_key, now);
    finish_engine_tap(0x0f, &option_key, now);

    uint32_t raw_joystick = rg_input_read_gamepad();
    suppressed_keys &= raw_joystick;
    uint32_t joystick = raw_joystick & ~suppressed_keys;

    // Defer these engine keys until release. A short press remains the native
    // ROTT action; holding for half a second opens the corresponding Retro-Go
    // menu without first opening ROTT's menu or minimap.
    joystick = handle_system_key(joystick, RG_KEY_MENU, 0x01,
                                 &menu_key, rg_gui_game_menu);
    joystick = handle_system_key(joystick, RG_KEY_OPTION, 0x0f,
                                 &option_key, rg_gui_options_menu);

    uint32_t changed = joystick ^ last_joystick;

    for (int i = 0; i < sizeof(keymap)/sizeof(keymap[0]); i++) {
        int key = keymap[i].rg_key;
        int scancode = keymap[i].scancode;

        if (key == RG_KEY_MENU || key == RG_KEY_OPTION)
            continue;
        
        if (joystick & key) {
            Keystate[scancode] = 1;
            Keyboard[scancode] = 1;
            if (changed & key) {
                LastScan = scancode;
                // Add to queue
                KeyboardQueue[Keytail] = scancode;
                Keytail = (Keytail + 1) & (KEYQMAX - 1);
            }
        } else {
            Keystate[scancode] = 0;
            Keyboard[scancode] = 0;
            if (changed & key) {
                // Key up
                KeyboardQueue[Keytail] = scancode | 0x80;
                Keytail = (Keytail + 1) & (KEYQMAX - 1);
            }
        }
    }
    last_joystick = joystick;
}

int SDL_Init(Uint32 flags) { return 0; }
int SDL_InitSubSystem(Uint32 flags) { return 0; }
void SDL_Quit(void) {}
void SDL_QuitSubSystem(Uint32 flags) {}
Uint32 SDL_WasInit(Uint32 flags) { return flags; }
char *SDL_GetError(void) { return "Unknown error"; }

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
    if (sdl_screen) return sdl_screen;
    sdl_screen = rg_alloc(sizeof(SDL_Surface), MEM_SLOW);
    memset(sdl_screen, 0, sizeof(SDL_Surface));
    sdl_screen->w = width;
    sdl_screen->h = height;
    sdl_screen->pitch = width * (bpp / 8);
    sdl_screen->format = rg_alloc(sizeof(SDL_PixelFormat), MEM_SLOW);
    memset(sdl_screen->format, 0, sizeof(SDL_PixelFormat));
    sdl_screen->format->palette = rg_alloc(sizeof(SDL_Palette), MEM_SLOW);
    sdl_screen->format->palette->ncolors = 256;
    sdl_screen->format->palette->colors = rg_alloc(256 * sizeof(SDL_Color), MEM_SLOW);
    rg_surface = rg_surface_create(width, height, ROTT_DISPLAY_FORMAT, MEM_FAST);


    sdl_screen->pixels = rg_surface->data;
    return sdl_screen;
}

SDL_Surface *SDL_GetVideoSurface(void) { return sdl_screen; }

int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
    if (!surface || !surface->format || !surface->format->palette) return 0;

    /*
     * Retro-Go consumes paletted surfaces asynchronously.  The palette is
     * part of the submitted surface, so it must remain unchanged until the
     * display task releases the surface.  ROTT also expects physical palette
     * changes to update an already displayed indexed frame (notably during
     * cinematic fades), so submit the surface again after changing it.
     */
    const bool present_palette = surface == sdl_screen && rg_surface;
    if (present_palette) {
        while (rg_display_is_busy())
            rg_task_yield();
    }

    for (int i = 0; i < ncolors; i++) {
        int idx = i + firstcolor;
        if (idx >= 256) break;
        surface->format->palette->colors[idx] = colors[i];
        uint16_t r = colors[i].r >> 3;
        uint16_t g = colors[i].g >> 2;
        uint16_t b = colors[i].b >> 3;
        uint16_t color = (r << 11) | (g << 5) | b;
        if (rg_surface->format == RG_PIXEL_PAL565_BE)
            color = (color << 8) | (color >> 8);
        rg_surface->palette[idx] = color;
    }

    if (present_palette)
        rg_display_submit(rg_surface, 0);

    return 1;
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
    return SDL_SetPalette(surface, 0, colors, firstcolor, ncolors);
}

int SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h) {
    update_input();
    if (rg_surface) rg_display_submit(rg_surface, 0);
    return 0;
}

int SDL_Flip(SDL_Surface *screen) {
    return SDL_UpdateRect(screen, 0, 0, 0, 0);
}

bool rg_sdl_save_screenshot(const char *filename, int width, int height)
{
    return rg_surface &&
           rg_surface_save_image_file(rg_surface, filename, width, height);
}

void rg_sdl_redraw(void)
{
    if (rg_surface)
        rg_display_submit(rg_surface, 0);
}

void SDL_FreeSurface(SDL_Surface *surface) {
    if (surface == sdl_screen) return;
    if (surface) {
        if (surface->pixels) free(surface->pixels);
        if (surface->format) {
            if (surface->format->palette) {
                if (surface->format->palette->colors) free(surface->format->palette->colors);
                free(surface->format->palette);
            }
            free(surface->format);
        }
        free(surface);
    }
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
    SDL_Surface *surface = rg_alloc(sizeof(SDL_Surface), MEM_SLOW);
    memset(surface, 0, sizeof(SDL_Surface));
    surface->w = width;
    surface->h = height;
    surface->pitch = width * (depth / 8);
    surface->pixels = rg_alloc(surface->pitch * height, MEM_SLOW);
    surface->format = rg_alloc(sizeof(SDL_PixelFormat), MEM_SLOW);
    memset(surface->format, 0, sizeof(SDL_PixelFormat));
    surface->format->BitsPerPixel = depth;
    surface->format->BytesPerPixel = depth / 8;
    return surface;
}

int SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) { return 0; }
void SDL_GetClipRect(SDL_Surface *surface, SDL_Rect *rect) { if (surface && rect) *rect = surface->clip_rect; }
void SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect) { if (surface && rect) surface->clip_rect = *rect; }
const SDL_VideoInfo *SDL_GetVideoInfo(void) { static SDL_VideoInfo info = { .wm_available = 1 }; return &info; }
void SDL_WM_SetCaption(const char *title, const char *icon) {}
int SDL_ShowCursor(int toggle) { return 0; }
void SDL_Delay(Uint32 ms) { rg_usleep(ms * 1000); }
Uint32 SDL_GetTicks(void) { return (Uint32)(rg_system_timer() / 1000); }
int SDL_WM_ToggleFullScreen(SDL_Surface *surface) { return 1; }
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode) { return mode; }

int SDL_PollEvent(SDL_Event *event) {
    update_input();
    return 0; 
}

int kbhit(void) {
    update_input();
    return 0;
}

int SDL_NumJoysticks(void) { return 1; }
SDL_Joystick *SDL_JoystickOpen(int device_index) { return (SDL_Joystick *)1; }
void SDL_JoystickClose(SDL_Joystick *joystick) {}
int SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis) { return 0; }
int SDL_JoystickEventState(int state) { return state; }
Uint8 SDL_GetMouseState(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; return 0; }

/* SDL_mixer stubs - we keep them but return success */
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize) { return 0; }
void Mix_CloseAudio(void) {}
int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels) { return 0; }
Mix_Music *Mix_LoadMUS(const char *file) { return (Mix_Music *)1; }
void Mix_FreeMusic(Mix_Music *music) {}
int Mix_PlayMusic(Mix_Music *music, int loops) { return 0; }
int Mix_HaltMusic(void) { return 0; }
int Mix_VolumeMusic(int volume) { return 0; }
int Mix_PauseMusic(void) { return 0; }
int Mix_ResumeMusic(void) { return 0; }
int Mix_PausedMusic(void) { return 0; }
int Mix_PlayingMusic(void) { return 0; }
int Mix_FadeOutMusic(int ms) { return 0; }
int Mix_FadingMusic(void) { return 0; }
Mix_Chunk *Mix_LoadWAV(const char *file) { return (Mix_Chunk *)1; }
void Mix_FreeChunk(Mix_Chunk *chunk) {}
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops) { return 0; }
int Mix_HaltChannel(int channel) { return 0; }
int Mix_Volume(int channel, int volume) { return 0; }

/* ROTT Music is now handled by opl_music.c */

/* Missing signals and other stubs */
typedef void (*__sighandler_t)(int);
__sighandler_t signal(int sig, __sighandler_t func) { return (__sighandler_t)0; }
void outp(int port, int val) {}
int TS_LockMemory(void) { return 0; }
int TS_UnlockMemory(void) { return 0; }
