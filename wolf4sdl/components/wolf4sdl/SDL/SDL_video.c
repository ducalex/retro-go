#include "SDL_video.h"
#include "SDL.h"
#include "SDL_blit.h"
#include <rg_system.h>
#include <string.h>
#include <stdlib.h>

SDL_Surface* primary_surface = NULL;
static rg_surface_t *rg_screen = NULL;
static uint16_t shadow_palette[256];
static int64_t last_frame_time = 0;
static uint32_t frame_wait_us;

static bool surface_uses_rg_buffer(const SDL_Surface *surface)
{
    return surface && rg_screen && surface->pixels == rg_screen->data;
}

static uint32_t wait_for_display(void)
{
    int64_t start = rg_system_timer();
    while (rg_display_is_busy()) {
        rg_task_yield();
    }
    return (uint32_t)(rg_system_timer() - start);
}

static uint32_t wait_for_rg_buffer(const SDL_Surface *surface)
{
    if (!surface_uses_rg_buffer(surface)) return 0;

    uint32_t elapsed = wait_for_display();
    frame_wait_us += elapsed;
    return elapsed;
}

void SDL_RG_SetSurface(rg_surface_t *surf)
{
    rg_screen = surf;
}

void SDL_RG_ResetTiming(void)
{
    last_frame_time = rg_system_timer();
    frame_wait_us = 0;
}

int SDL_LockSurface(SDL_Surface *surface)
{
    // Retro-Go consumes its submitted surface asynchronously. Wolf's engine
    // keeps a separate backbuffer, so wait immediately before that buffer is
    // copied or a transition writes directly into the display surface.
    wait_for_rg_buffer(surface);

    return 0;
}

void SDL_UnlockSurface(SDL_Surface* surface)
{
}

void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
    SDL_Flip(screen);
}

SDL_VideoInfo *SDL_GetVideoInfo(void)
{
    static SDL_VideoInfo info;
    static SDL_PixelFormat vfmt;
    
    memset(&info, 0, sizeof(info));
    memset(&vfmt, 0, sizeof(vfmt));
    
    info.video_mem = 1024;
    info.vfmt = &vfmt;
    vfmt.BitsPerPixel = 8;
    vfmt.BytesPerPixel = 1;
    
    return &info;
}

char *SDL_VideoDriverName(char *namebuf, int maxlen)
{
    strncpy(namebuf, "Retro-Go", maxlen);
    return namebuf;
}

SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags)
{
    static SDL_Rect modes_list[] = {
        {0, 0, 320, 240},
        {0, 0, 320, 200},
        {0, 0, 0, 0}
    };
    static SDL_Rect *modes[] = {&modes_list[0], &modes_list[1], NULL};
    return modes;
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{
}

char *SDL_GetKeyName(SDLKey key)
{
    return (char *)"";
}

Uint32 SDL_GetTicks(void)
{
    return (Uint32)(rg_system_timer() / 1000);
}

Uint32 SDL_WasInit(Uint32 flags)
{
    return flags & SDL_INIT_VIDEO;
}

int SDL_InitSubSystem(Uint32 flags)
{
    return 0;
}

static SDL_Surface *create_rgb_surface(Uint32 flags, int width, int height, int depth, void *pixels)
{
    SDL_Surface *surface = (SDL_Surface *)calloc(1, sizeof(SDL_Surface));
    if (!surface) return NULL;

    surface->format = (SDL_PixelFormat *)calloc(1, sizeof(SDL_PixelFormat));
    if (!surface->format) {
        free(surface);
        return NULL;
    }

    surface->flags = flags | (pixels ? SDL_PREALLOC : 0);
    surface->format->BitsPerPixel = (Uint8)depth;
    surface->format->BytesPerPixel = (Uint8)(depth / 8);
    surface->w = width;
    surface->h = height;
    surface->pitch = (Uint16)(width * surface->format->BytesPerPixel);
    surface->pixels = pixels ? pixels : malloc(surface->pitch * height);
    if (!surface->pixels) {
        free(surface->format);
        free(surface);
        return NULL;
    }
    surface->refcount = 1;

    surface->map = (SDL_BlitMap *)calloc(1, sizeof(SDL_BlitMap));
    if (!surface->map) {
        if (!(surface->flags & SDL_PREALLOC)) free(surface->pixels);
        free(surface->format);
        free(surface);
        return NULL;
    }
    surface->map->sw_blit = SDL_SoftBlit;
    surface->map->sw_data = (struct private_swaccel *)calloc(1, sizeof(pub_swaccel));
    if (!surface->map->sw_data) {
        free(surface->map);
        if (!(surface->flags & SDL_PREALLOC)) free(surface->pixels);
        free(surface->format);
        free(surface);
        return NULL;
    }
    surface->map->sw_data->blit = SDL_BlitCopy;
    
    return surface;
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return create_rgb_surface(flags, width, height, depth, NULL);
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    if (!dst) return -1;
    
    int x = dstrect ? dstrect->x : 0;
    int y = dstrect ? dstrect->y : 0;
    int w = dstrect ? dstrect->w : dst->w;
    int h = dstrect ? dstrect->h : dst->h;
    
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > dst->w) w = dst->w - x;
    if (y + h > dst->h) h = dst->h - y;
    if (w <= 0 || h <= 0) return 0;

    for (int i = y; i < y + h; i++) {
        memset((uint8_t*)dst->pixels + i * dst->pitch + x, (int)color, (size_t)w);
    }
    return 0;
}

SDL_Surface *SDL_GetVideoSurface(void)
{
    return primary_surface;
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors)
{
    for (int i = 0; i < ncolors; i++) {
        int idx = firstcolor + i;
        if (idx >= 256) break;
        
        uint16_t r = colors[i].r >> 3;
        uint16_t g = colors[i].g >> 2;
        uint16_t b = colors[i].b >> 3;
        uint16_t color = (uint16_t)((r << 11) | (g << 5) | b);
        
        if (rg_screen && rg_screen->format == RG_PIXEL_PAL565_BE) {
            color = (color << 8) | (color >> 8);
        }
        shadow_palette[idx] = color;
    }
    return 1;
}

int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors)
{
    return SDL_SetColors(surface, colors, firstcolor, ncolors);
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    if (!primary_surface) {
        void *pixels = NULL;
        if (rg_screen && bpp == 8 && width == rg_screen->width && height == rg_screen->height) {
            pixels = rg_screen->data;
        }

        // SDL's hardware flag remains disabled. Retro-Go output buffering is
        // managed separately from Wolf's engine backbuffer.
        flags &= ~(SDL_HWSURFACE | SDL_DOUBLEBUF);
        primary_surface = create_rgb_surface(flags, width, height, bpp, pixels);
    }
    return primary_surface;
}

void SDL_FreeSurface(SDL_Surface *surface)
{
    if (surface) {
        if (surface == primary_surface) primary_surface = NULL;
        if (surface->pixels && !(surface->flags & SDL_PREALLOC)) free(surface->pixels);
        if (surface->format) free(surface->format);
        if (surface->map) {
            if (surface->map->sw_data) free(surface->map->sw_data);
            free(surface->map);
        }
        free(surface);
    }
}

int SDL_UpperBlit (SDL_Surface *src, SDL_Rect *srcrect,
                  SDL_Surface *dst, SDL_Rect *dstrect)
{
        SDL_Rect fulldst;
       int srcx, srcy, w, h;

       /* Make sure the surfaces aren't locked */
       if ( ! src || ! dst ) {
               return(-1);
       }

       if ( src->locked || dst->locked ) {
               return(-1);
       }

       /* If the destination rectangle is NULL, use the entire dest surface */
       if ( dstrect == NULL ) {
               fulldst.x = fulldst.y = 0;
               dstrect = &fulldst;
       }

       /* clip the source rectangle to the source surface */
       if(srcrect) {
               int maxw, maxh;

               srcx = srcrect->x;
               w = srcrect->w;
               if(srcx < 0) {
                       w += srcx;
                       dstrect->x -= srcx;
                       srcx = 0;
               }
               maxw = src->w - srcx;
               if(maxw < w)
                       w = maxw;

               srcy = srcrect->y;
               h = srcrect->h;
               if(srcy < 0) {
                       h += srcy;
                       dstrect->y -= srcy;
                       srcy = 0;
               }
               maxh = src->h - srcy;
               if(maxh < h)
                       h = maxh;

       } else {
               srcx = srcy = 0;
               w = src->w;
               h = src->h;
       }

       /* clip the destination rectangle against the clip rectangle */
       {
               SDL_Rect clip = {0, 0, dst->w, dst->h};
               int dx, dy;

               dx = clip.x - dstrect->x;
               if(dx > 0) {
                       w -= dx;
                       dstrect->x += dx;
                       srcx += dx;
               }
               dx = dstrect->x + w - clip.x - clip.w;
               if(dx > 0)
                       w -= dx;

               dy = clip.y - dstrect->y;
               if(dy > 0) {
                       h -= dy;
                       dstrect->y += dy;
                       srcy += dy;
               }
               dy = dstrect->y + h - clip.y - clip.h;
               if(dy > 0)
                       h -= dy;
       }

       if(w > 0 && h > 0) {
               SDL_Rect sr;
               sr.x = srcx;
               sr.y = srcy;
               sr.w = dstrect->w = w;
               sr.h = dstrect->h = h;
               // The primary SDL surface aliases Retro-Go's asynchronous
               // submission buffer. Acquire it before writing into it.
               wait_for_rg_buffer(dst);

               int result = SDL_SoftBlit(src, &sr, dst, dstrect);
               return result;
       }
       dstrect->w = dstrect->h = 0;
       return 0;
}

Uint32 SDL_MapRGB(SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b)
{
    if (format->BitsPerPixel == 8) return (Uint32)r; // Dummy for Wolf
    uint16_t color = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    return (Uint32)color;
}

int SDL_Flip(SDL_Surface *screen)
{
    if (!rg_screen || !screen) return -1;
    
    // Report active work separately from display wait time.
    int64_t now = rg_system_timer();
    if (last_frame_time == 0) last_frame_time = now;
    int64_t elapsed_us = now - last_frame_time;
    int busy_us = elapsed_us > frame_wait_us
            ? (int)(elapsed_us - frame_wait_us) : 0;
    rg_system_tick(busy_us);

    // Frame limiter (70 FPS = 14285 microseconds)
    int64_t target = last_frame_time + 14285;
    while (rg_system_timer() < target) {
        rg_task_yield();
    }
    last_frame_time = rg_system_timer();

    // Wait for display to be ready
    frame_wait_us += wait_for_display();

    // Update palette from shadow
    memcpy(rg_screen->palette, shadow_palette, 256 * 2);

    if (screen->pixels != rg_screen->data) {
        memcpy(rg_screen->data, screen->pixels, (size_t)(screen->w * screen->h));
    }
    
    rg_display_submit(rg_screen, 0);

    frame_wait_us = 0;

    return 0;
}
