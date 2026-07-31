#ifndef _RG_SDL_H_
#define _RG_SDL_H_

#include <stdint.h>

/* Basic SDL types */
typedef uint8_t  Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;

typedef struct SDL_Rect {
    int16_t x, y;
    uint16_t w, h;
} SDL_Rect;

typedef struct SDL_Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 unused;
} SDL_Color;

typedef struct SDL_Palette {
    int ncolors;
    SDL_Color *colors;
} SDL_Palette;

typedef struct SDL_PixelFormat {
    SDL_Palette *palette;
    Uint8  BitsPerPixel;
    Uint8  BytesPerPixel;
    Uint32 Rmask, Gmask, Bmask, Amask;
    Uint8  Rshift, Gshift, Bshift, Ashift;
    Uint8  Rloss, Gloss, Bloss, Aloss;
} SDL_PixelFormat;

typedef struct SDL_Surface {
    Uint32 flags;
    SDL_PixelFormat *format;
    int w, h;
    int pitch;
    void *pixels;
    SDL_Rect clip_rect;
    int refcount;
} SDL_Surface;

/* SDL Flags */
#define SDL_SWSURFACE   0x00000000
#define SDL_HWSURFACE   0x00000001
#define SDL_ASYNCBLIT   0x00000004
#define SDL_ANYFORMAT   0x10000000
#define SDL_HWPALETTE   0x20000000
#define SDL_DOUBLEBUF   0x40000000
#define SDL_FULLSCREEN  0x80000000
#define SDL_OPENGL      0x00000002
#define SDL_OPENGLBLIT  0x0000000A

#define SDL_INIT_VIDEO    0x00000020
#define SDL_INIT_AUDIO    0x00000010
#define SDL_INIT_JOYSTICK 0x00000200

#define SDL_PRESSED  1
#define SDL_RELEASED 0
#define SDL_ENABLE   1
#define SDL_DISABLE  0

typedef enum {
    SDLK_UNKNOWN = 0,
    SDLK_BACKSPACE = 8,
    SDLK_TAB = 9,
    SDLK_RETURN = 13,
    SDLK_ESCAPE = 27,
    SDLK_SPACE = 32,
    SDLK_QUOTE = 39,
    SDLK_COMMA = 44,
    SDLK_MINUS = 45,
    SDLK_PERIOD = 46,
    SDLK_SLASH = 47,
    SDLK_0 = 48,
    SDLK_1 = 49,
    SDLK_2 = 50,
    SDLK_3 = 51,
    SDLK_4 = 52,
    SDLK_5 = 53,
    SDLK_6 = 54,
    SDLK_7 = 55,
    SDLK_8 = 56,
    SDLK_9 = 57,
    SDLK_SEMICOLON = 59,
    SDLK_EQUALS = 61,
    SDLK_LEFTBRACKET = 91,
    SDLK_BACKSLASH = 92,
    SDLK_RIGHTBRACKET = 93,
    SDLK_BACKQUOTE = 96,
    SDLK_a = 97,
    SDLK_b = 98,
    SDLK_c = 99,
    SDLK_d = 100,
    SDLK_e = 101,
    SDLK_f = 102,
    SDLK_g = 103,
    SDLK_h = 104,
    SDLK_i = 105,
    SDLK_j = 106,
    SDLK_k = 107,
    SDLK_l = 108,
    SDLK_m = 109,
    SDLK_n = 110,
    SDLK_o = 111,
    SDLK_p = 112,
    SDLK_q = 113,
    SDLK_r = 114,
    SDLK_s = 115,
    SDLK_t = 116,
    SDLK_u = 117,
    SDLK_v = 118,
    SDLK_w = 119,
    SDLK_x = 120,
    SDLK_y = 121,
    SDLK_z = 122,
    SDLK_DELETE = 127,
    SDLK_KP0 = 256,
    SDLK_KP1 = 257,
    SDLK_KP2 = 258,
    SDLK_KP3 = 259,
    SDLK_KP4 = 260,
    SDLK_KP5 = 261,
    SDLK_KP6 = 262,
    SDLK_KP7 = 263,
    SDLK_KP8 = 264,
    SDLK_KP9 = 265,
    SDLK_KP_PERIOD = 266,
    SDLK_KP_DIVIDE = 267,
    SDLK_KP_MULTIPLY = 268,
    SDLK_KP_MINUS = 269,
    SDLK_KP_PLUS = 270,
    SDLK_KP_ENTER = 271,
    SDLK_UP = 273,
    SDLK_DOWN = 274,
    SDLK_RIGHT = 275,
    SDLK_LEFT = 276,
    SDLK_INSERT = 277,
    SDLK_HOME = 278,
    SDLK_END = 279,
    SDLK_PAGEUP = 280,
    SDLK_PAGEDOWN = 281,
    SDLK_F1 = 282,
    SDLK_F2 = 283,
    SDLK_F3 = 284,
    SDLK_F4 = 285,
    SDLK_F5 = 286,
    SDLK_F6 = 287,
    SDLK_F7 = 288,
    SDLK_F8 = 289,
    SDLK_F9 = 290,
    SDLK_F10 = 291,
    SDLK_F11 = 292,
    SDLK_F12 = 293,
    SDLK_NUMLOCK = 300,
    SDLK_CAPSLOCK = 301,
    SDLK_SCROLLOCK = 302,
    SDLK_RSHIFT = 303,
    SDLK_LSHIFT = 304,
    SDLK_RCTRL = 305,
    SDLK_LCTRL = 306,
    SDLK_RALT = 307,
    SDLK_LALT = 308,
    SDLK_MODE = 313,
    SDLK_WORLD_63 = 323,
    SDLK_PAUSE = 324,
    SDLK_LAST
} SDLKey;

typedef struct SDL_keysym {
    uint8_t scancode;
    SDLKey sym;
    uint32_t mod;
    uint16_t unicode;
} SDL_keysym;

typedef struct SDL_KeyboardEvent {
    uint8_t type;
    uint8_t state;
    SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent {
    uint8_t type;
    uint8_t state;
    uint16_t x, y;
    int16_t xrel, yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_JoyBallEvent {
    uint8_t type;
    uint8_t which;
    uint8_t ball;
    int16_t xrel, yrel;
} SDL_JoyBallEvent;

#define SDL_KEYUP 2
#define SDL_KEYDOWN 3
#define SDL_JOYBALLMOTION 4
#define SDL_MOUSEMOTION 5
#define SDL_MOUSEBUTTONUP 6
#define SDL_MOUSEBUTTONDOWN 7
#define SDL_QUIT 8

typedef struct SDL_Event {
    uint8_t type;
    union {
        SDL_KeyboardEvent key;
        SDL_MouseMotionEvent motion;
        SDL_JoyBallEvent jball;
    };
} SDL_Event;

/* SDL Functions */
int SDL_Init(Uint32 flags);
int SDL_InitSubSystem(Uint32 flags);
void SDL_Quit(void);
void SDL_QuitSubSystem(Uint32 flags);
Uint32 SDL_WasInit(Uint32 flags);
char *SDL_GetError(void);

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
SDL_Surface *SDL_GetVideoSurface(void);
int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors);
int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
int SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);
void SDL_FreeSurface(SDL_Surface *surface);
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
int SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void SDL_GetClipRect(SDL_Surface *surface, SDL_Rect *rect);
void SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect);

typedef struct SDL_VideoInfo {
    uint32_t wm_available : 1;
} SDL_VideoInfo;
const SDL_VideoInfo *SDL_GetVideoInfo(void);

void SDL_WM_SetCaption(const char *title, const char *icon);
int SDL_ShowCursor(int toggle);
void SDL_Delay(Uint32 ms);
Uint32 SDL_GetTicks(void);
int SDL_WM_ToggleFullScreen(SDL_Surface *surface);

typedef enum {
    SDL_GRAB_QUERY = -1,
    SDL_GRAB_OFF = 0,
    SDL_GRAB_ON = 1
} SDL_GrabMode;
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode);

int SDL_PollEvent(SDL_Event *event);
int kbhit(void);
char getch(void);

/* Joystick stubs */
typedef struct _SDL_Joystick SDL_Joystick;
int SDL_NumJoysticks(void);
SDL_Joystick *SDL_JoystickOpen(int device_index);
void SDL_JoystickClose(SDL_Joystick *joystick);
int SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis);
int SDL_JoystickEventState(int state);

/* Mouse stubs */
Uint8 SDL_GetMouseState(int *x, int *y);
#define SDL_BUTTON_LMASK  0x01
#define SDL_BUTTON_MMASK  0x02
#define SDL_BUTTON_RMASK  0x04

/* Modifiers */
#define KMOD_NONE  0x0000
#define KMOD_LSHIFT 0x0001
#define KMOD_RSHIFT 0x0002
#define KMOD_LCTRL  0x0040
#define KMOD_RCTRL  0x0080
#define KMOD_LALT   0x0100
#define KMOD_RALT   0x0200
#define KMOD_CTRL  (KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT   (KMOD_LALT|KMOD_RALT)

/* Audio (if needed by dsl.c) */
typedef struct SDL_AudioSpec {
    int freq;
    Uint16 format;
    Uint8 channels;
    Uint8 silence;
    Uint16 samples;
    Uint32 size;
    void (*callback)(void *userdata, Uint8 *stream, int len);
    void *userdata;
} SDL_AudioSpec;

#define AUDIO_U8     0x0008
#define AUDIO_S16SYS 0x8010

#endif
