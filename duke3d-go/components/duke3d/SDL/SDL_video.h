#ifndef SDL_TFT_H
#define SDL_TFT_H

#include "SDL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include <errno.h>

#include "driver/gpio.h"

#define SDL_SaveBMP(surface, file) {}
#define SDL_LoadBMP_RW(src, freesrc) {}

typedef struct{
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 unused;
} SDL_Color;

typedef struct{
  int ncolors;
  SDL_Color *colors;
} SDL_Palette;

typedef struct SDL_PixelFormat {
  SDL_Palette *palette;
  Uint8  BitsPerPixel;
  Uint8  BytesPerPixel;
  Uint8  Rloss, Gloss, Bloss, Aloss;
  Uint8  Rshift, Gshift, Bshift, Ashift;
  Uint32 Rmask, Gmask, Bmask, Amask;
  Uint32 colorkey;
  Uint8  alpha;
} SDL_PixelFormat;

typedef struct{
  Sint16 x, y;
  Uint16 w, h;
} SDL_Rect;

typedef struct SDL_Surface {
        Uint32 flags;                           /* Read-only */
        SDL_PixelFormat *format;                /* Read-only */
        int w, h;                               /* Read-only */
        Uint16 pitch;                           /* Read-only */
        void *pixels;                           /* Read-write */

        /* clipping information */
        SDL_Rect clip_rect;                     /* Read-only */

        /* Reference count -- used when freeing surface */
        int refcount;                           /* Read-mostly */

	/* This structure also contains private fields not shown here */
} SDL_Surface;

/* These are the currently supported flags for the SDL_surface */
/* Available for SDL_CreateRGBSurface() or SDL_SetVideoMode() */
#define SDL_SWSURFACE	0x00000000	/* Surface is in system memory */
#define SDL_HWSURFACE	0x00000001	/* Surface is in video memory */
#define SDL_ASYNCBLIT	0x00000004	/* Use asynchronous blits if possible */
/* Available for SDL_SetVideoMode() */
#define SDL_ANYFORMAT	0x10000000	/* Allow any video depth/pixel-format */
#define SDL_HWPALETTE	0x20000000	/* Surface has exclusive palette */
#define SDL_DOUBLEBUF	0x40000000	/* Set up double-buffered video mode */
#define SDL_FULLSCREEN	0x80000000	/* Surface is a full screen display */
#define SDL_OPENGL      0x00000002      /* Create an OpenGL rendering context */
#define SDL_OPENGLBLIT	0x0000000A	/* Create an OpenGL rendering context and use it for blitting */
#define SDL_RESIZABLE	0x00000010	/* This video mode may be resized */
#define SDL_NOFRAME	0x00000020	/* No window caption or edge frame */
/* Used internally (read-only) */
#define SDL_HWACCEL	0x00000100	/* Blit uses hardware acceleration */
#define SDL_SRCCOLORKEY	0x00001000	/* Blit uses a source color key */
#define SDL_RLEACCELOK	0x00002000	/* Private flag */
#define SDL_RLEACCEL	0x00004000	/* Surface is RLE encoded */
#define SDL_SRCALPHA	0x00010000	/* Blit uses source alpha blending */
#define SDL_PREALLOC	0x01000000	/* Surface uses preallocated memory */

/* Evaluates to true if the surface needs to be locked before access */
#define SDL_MUSTLOCK(S) (((S)->flags & SDL_RLEACCEL) != 0)


typedef struct{
  Uint32 hw_available:1;
  Uint32 wm_available:1;
  Uint32 blit_hw:1;
  Uint32 blit_hw_CC:1;
  Uint32 blit_hw_A:1;
  Uint32 blit_sw:1;
  Uint32 blit_sw_CC:1;
  Uint32 blit_sw_A:1;
  Uint32 blit_fill;
  Uint32 video_mem;
  SDL_PixelFormat *vfmt;
} SDL_VideoInfo;

typedef int SDLKey;

void SDL_WM_SetCaption(const char *title, const char *icon);
Uint32 SDL_WasInit(Uint32 flags);
int SDL_InitSubSystem(Uint32 flags);
int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
char *SDL_GetKeyName(SDLKey key);
Uint32 SDL_GetTicks(void);
SDL_Keymod SDL_GetModState(void);
SDL_Surface *SDL_GetVideoSurface(void);
Uint32 SDL_MapRGB(SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b);
int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
void SDL_FreeSurface(SDL_Surface *surface);
void SDL_QuitSubSystem(Uint32 flags);
int SDL_Flip(SDL_Surface *screen);
int SDL_VideoModeOK(int width, int height, int bpp, Uint32 flags);
int SDL_LockSurface(SDL_Surface *surface);
void SDL_UnlockSurface(SDL_Surface* surface);
void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags);
SDL_VideoInfo *SDL_GetVideoInfo(void);
char *SDL_VideoDriverName(char *namebuf, int maxlen);

extern SemaphoreHandle_t display_mutex;
void SDL_LockDisplay();
void SDL_UnlockDisplay();

typedef unsigned char  JE_byte;
//extern JE_byte ** allocateTwoDimenArrayOnHeapUsingMalloc(int row, int col);

#endif
