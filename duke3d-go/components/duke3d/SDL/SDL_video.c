#include "SDL_video.h"
#include "rg_display.h"
#include "rg_system.h"
#include "build.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_memory_utils.h"

static rg_surface_t screen_surface;

SDL_Surface* primary_surface;
static size_t primary_pixel_capacity;
static bool primary_uses_fb_pool;
static bool full_sync_next_flip;

int SDL_LockSurface(SDL_Surface *surface)
{
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
    info.vfmt = &vfmt;
    info.vfmt->BitsPerPixel = 8;
    return &info;
}

char *SDL_VideoDriverName(char *namebuf, int maxlen)
{
    return "RETRO-GO";
}


SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags)
{
    static SDL_Rect modes_data[] = {
        {0, 0, 320, 240},
        {0, 0, 240, 160},
        {0, 0, 0, 0}
    };
    static SDL_Rect *modes[] = { &modes_data[0], &modes_data[1], NULL };
    return modes;
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{

}

char *SDL_GetKeyName(SDLKey key)
{
    return (char *)"";
}

SDL_Keymod SDL_GetModState(void)
{
    return (SDL_Keymod)0;
}

IRAM_ATTR Uint32 SDL_GetTicks(void)
{
    return esp_timer_get_time() / 1000;
}

Uint32 SDL_WasInit(Uint32 flags)
{
	return 0;
}

#define MAX_SUBMITTED_SURFACES 2
#define PREFERRED_RENDER_HEIGHT 200
typedef struct {
    rg_surface_t surface;
    uint8_t *pixels;
} fb_t;

static fb_t fb_pool[MAX_SUBMITTED_SURFACES];
static int current_fb_idx = 0;
static bool frame_has_3d_view;
static int64_t frame_busy_start;

int SDL_InitSubSystem(Uint32 flags)
{
    if(flags & SDL_INIT_VIDEO)
    {
        for (int i = 0; i < MAX_SUBMITTED_SURFACES; i++) {
            fb_pool[i].pixels = rg_alloc(INTERNAL_RES_W * INTERNAL_RES_H, MEM_SLOW);
            fb_pool[i].surface.palette = rg_alloc(256 * 2, MEM_SLOW);
            if (!fb_pool[i].pixels || !fb_pool[i].surface.palette) {
                RG_LOGE("Failed to pre-allocate FB %d", i);
                return -1;
            }
        }
        // Duke normally selects 320x200. The primary SDL surface is attached
        // to these preallocated 320x240 buffers, so either supported height
        // can be selected without allocating or copying another framebuffer.
        SDL_CreateRGBSurface(0, INTERNAL_RES_W, PREFERRED_RENDER_HEIGHT, 8, 0,0,0,0);
    }
    return 0; // 0 = OK, -1 = Error
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    SDL_Surface *surface = (SDL_Surface *)rg_alloc(sizeof(SDL_Surface), MEM_SLOW);
    SDL_Rect rect = { .x=0, .y=0, .w=width, .h=height};
    
    SDL_PixelFormat* pf = (SDL_PixelFormat*)rg_alloc(sizeof(SDL_PixelFormat), MEM_SLOW);
    pf->palette = (SDL_Palette *)rg_alloc(sizeof(SDL_Palette), MEM_SLOW);
    pf->palette->ncolors = 256;
    pf->palette->colors = (SDL_Color *)rg_alloc(256 * sizeof(SDL_Color), MEM_SLOW);
    
	pf->BitsPerPixel = depth;
	pf->BytesPerPixel = depth / 8;
	pf->Rloss = 0; pf->Gloss = 0; pf->Bloss = 0; pf->Aloss = 0,
	pf->Rshift = 0; pf->Gshift = 0; pf->Bshift = 0; pf->Ashift = 0;
	pf->Rmask = Rmask; pf->Gmask = Gmask; pf->Bmask = Bmask; pf->Amask = Amask;
	pf->colorkey = 0;
	pf->alpha = 255;

    surface->flags = flags;
    surface->format = pf;
    surface->w = width;
    surface->h = height;
    surface->pitch = width*(depth/8);
    surface->clip_rect = rect;
    surface->refcount = 1;
    const size_t pixel_bytes = width * height * (depth / 8);
    const size_t pool_pixel_capacity = INTERNAL_RES_W * INTERNAL_RES_H;
    if (depth == 8 && pixel_bytes <= pool_pixel_capacity && fb_pool[0].pixels) {
        // Render directly into the same two buffers submitted to Retro-Go.
        // This removes the former 64KB PSRAM-to-PSRAM copy on every frame.
        current_fb_idx = 0;
        surface->pixels = fb_pool[current_fb_idx].pixels;
        primary_pixel_capacity = pool_pixel_capacity;
        primary_uses_fb_pool = true;
        RG_LOGI("Direct double buffering enabled: 2 x %u-byte indexed buffers",
                (unsigned)pool_pixel_capacity);
    } else {
        surface->pixels = rg_alloc(pixel_bytes, MEM_FAST | MEM_NOPANIC);
        primary_pixel_capacity = pixel_bytes;
        primary_uses_fb_pool = false;
    }
    if (!surface->pixels) {
        RG_LOGE("Failed to allocate %u-byte render framebuffer", (unsigned)pixel_bytes);
        free(pf->palette->colors);
        free(pf->palette);
        free(pf);
        free(surface);
        return NULL;
    }
    RG_LOGI("Render framebuffer: %u bytes in %s RAM",
            (unsigned)pixel_bytes,
            esp_ptr_internal(surface->pixels) ? "internal" : "external");
    extern uint8_t* frameplace;
    extern uint8_t* frameoffset;
    frameoffset = frameplace = (uint8_t*)surface->pixels;

    if(primary_surface == NULL)
    	primary_surface = surface;
    return surface;
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    if(dst)//|| dst->sprite == NULL)
    {
    	if(dstrect != NULL)
    	{
			for(int y = dstrect->y; y < dstrect->y + dstrect->h;y++)
				memset((unsigned char *)dst->pixels + y*dst->w + dstrect->x, (unsigned char)color, dstrect->w);
    	} else {
    		memset(dst->pixels, (unsigned char)color, dst->pitch*dst->h);
    	}
    }
    return 0;
}

SDL_Surface *SDL_GetVideoSurface(void)
{
    return primary_surface;
}

Uint32 SDL_MapRGB(SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b)
{
    if(fmt->BitsPerPixel == 16)
    {
        uint16_t bb = (b >> 3) & 0x1f;
        uint16_t gg = ((g >> 2) & 0x3f) << 5;
        uint16_t rr = ((r >> 3) & 0x1f) << 11;
        return (Uint32) (rr | gg | bb);
    }
    return (Uint32)0;
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors)
{
    if (surface == NULL) {
        return 0;
    }
    if (!screen_surface.palette) {
        screen_surface.palette = rg_alloc(256 * 2, MEM_SLOW);
    }
    if (!screen_surface.palette) return 0;

	for(int i = firstcolor; i < firstcolor+ncolors; i++)
	{
        if (i >= 256) break;
        uint16_t r = colors[i].r;
        uint16_t g = colors[i].g;
        uint16_t b = colors[i].b;
        uint16_t v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
#if RG_SCREEN_PIXEL_FORMAT == 0 /* 565_BE */
        v = (v >> 8) | (v << 8);
#endif
		screen_surface.palette[i] = v;
    }
    return 1;
}

// Convert a Duke3D-style 6-bit palette (packed as B,G,R,unused per entry) to RGB565.
//
// Brightness/gamma is handled upstream in setbrightness() via the britable[] table
// (loaded from tables.dat), which applies the same power-curve correction as eDuke32.
// This function's job is purely precision-correct channel conversion:
//
//   1. Fill-expand 6-bit to 8-bit: v8 = (v6 << 2) | (v6 >> 4)  (0->0, 63->255)
//   2. Ceiling-round to 5-bit (R,B) with clamp: min(31, (v8 + 4) >> 3)
//      Ceiling-round to 6-bit (G) with clamp:   min(63, (v8 + 2) >> 2)
//      Ceiling rounding preserves tiny values (e.g. b6=1 -> b5=1, not b5=0).
//      Clamping prevents overflow at v8=255: (255+4)>>3=32 -> clamped to 31.
//      NOTE: Using min() instead of & mask because 32 & 0x1F = 0, not 31!
int SDL_SetPalette565(const uint8_t *pal6)
{
    if (!screen_surface.palette) {
        screen_surface.palette = rg_alloc(256 * 2, MEM_SLOW);
    }
    if (!screen_surface.palette) return 0;

    for (int i = 0; i < 256; i++) {
        // palettebuffer layout per entry: [B, G, R, unused] (each 6-bit, 0-63)
        uint16_t r8 = (pal6[i*4+2] << 2) | (pal6[i*4+2] >> 4);
        uint16_t g8 = (pal6[i*4+1] << 2) | (pal6[i*4+1] >> 4);
        uint16_t b8 = (pal6[i*4+0] << 2) | (pal6[i*4+0] >> 4);
        // Ceiling-round with proper clamping (not masking which wraps 32->0)
        uint16_t r5_raw = (r8 + 4) >> 3;
        uint16_t g6_raw = (g8 + 2) >> 2;
        uint16_t b5_raw = (b8 + 4) >> 3;
        uint16_t r5 = r5_raw < 31 ? r5_raw : 31;
        uint16_t g6 = g6_raw < 63 ? g6_raw : 63;
        uint16_t b5 = b5_raw < 31 ? b5_raw : 31;
        uint16_t v = (r5 << 11) | (g6 << 5) | b5;
#if RG_SCREEN_PIXEL_FORMAT == 0 /* 565_BE */
        v = (v >> 8) | (v << 8);
#endif
        screen_surface.palette[i] = v;
    }
    return 1;
}

void SDL_PresentPalette(void)
{
    // Gameplay palette effects run near the end of displayrest(). Swapping a
    // direct render buffer there would split one frame across two buffers.
    // The normal nextpage() immediately presents the new palette safely.
    if (primary_uses_fb_pool && frame_has_3d_view)
        return;

    SDL_Flip(primary_surface);
}

void SDL_MarkFrame3D(void)
{
    frame_has_3d_view = true;
}

void SDL_RequestFullFrameSync(void)
{
    full_sync_next_flip = true;
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    if (primary_surface) {
        const size_t required_bytes = (size_t)width * height *
                                      (primary_surface->format->BitsPerPixel / 8);
        if (required_bytes > primary_pixel_capacity) {
            void *candidate = rg_alloc(required_bytes, MEM_FAST | MEM_NOPANIC);
            const bool candidate_internal = candidate && esp_ptr_internal(candidate);

            // A larger mode must use the new allocation even when it falls
            // back to PSRAM.
            if (candidate) {
                memcpy(candidate, primary_surface->pixels, primary_pixel_capacity);
                if (!primary_uses_fb_pool)
                    free(primary_surface->pixels);
                primary_surface->pixels = candidate;
                primary_pixel_capacity = required_bytes;
                primary_uses_fb_pool = false;
                RG_LOGW("Render framebuffer resized: %u bytes in %s RAM",
                        (unsigned)required_bytes,
                        candidate_internal ? "internal" : "external");
            }
        }

        if (required_bytes > primary_pixel_capacity) {
            RG_LOGE("Unable to resize render framebuffer to %ux%u",
                    (unsigned)width, (unsigned)height);
            return NULL;
        }

        primary_surface->w = width;
        primary_surface->h = height;
        primary_surface->pitch = width * (primary_surface->format->BitsPerPixel / 8);

        extern uint8_t* frameplace;
        extern uint8_t* frameoffset;
        frameoffset = frameplace = (uint8_t *)primary_surface->pixels;
        return primary_surface;
    }
	return SDL_GetVideoSurface();
}

void SDL_FreeSurface(SDL_Surface *surface)
{
    if (surface) {
        if (surface->pixels && !(surface == primary_surface && primary_uses_fb_pool))
            free(surface->pixels);
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

void SDL_QuitSubSystem(Uint32 flags)
{

}

static void frame_limit_timer_cb(void *arg)
{
    xSemaphoreGive((SemaphoreHandle_t)arg);
}

static void limit_fps(int fps)
{
    static SemaphoreHandle_t wait_sem;
    static esp_timer_handle_t wait_timer;
    static int64_t last_frame_us;

    if (!wait_sem) {
        wait_sem = xSemaphoreCreateBinary();
        if (wait_sem) {
            const esp_timer_create_args_t timer_args = {
                .callback = frame_limit_timer_cb,
                .arg = wait_sem,
                .name = "duke_fps",
            };
            if (esp_timer_create(&timer_args, &wait_timer) != ESP_OK) {
                vSemaphoreDelete(wait_sem);
                wait_sem = NULL;
            }
        }
    }

    const int64_t frame_time_us = (1000000 + fps - 1) / fps;
    int64_t now = rg_system_timer();

    if (last_frame_us > 0) {
        const int64_t remaining_us = frame_time_us - (now - last_frame_us);
        if (remaining_us > 0) {
            if (wait_timer) {
                xSemaphoreTake(wait_sem, 0);
                if (esp_timer_start_once(wait_timer, remaining_us) == ESP_OK) {
                    xSemaphoreTake(wait_sem, portMAX_DELAY);
                } else {
                    rg_usleep((uint32_t)remaining_us);
                }
            } else {
                rg_usleep((uint32_t)remaining_us);
            }
        }
    }

    last_frame_us = rg_system_timer();
}

IRAM_ATTR int SDL_Flip(SDL_Surface *screen)
{
    if (!screen || !screen->pixels) {
        return -1;
    }

    const bool force_full_sync = full_sync_next_flip;
    full_sync_next_flip = false;

    const int64_t flip_start = rg_system_timer();
    const uint32_t render_us = frame_busy_start > 0
        ? (uint32_t)(flip_start - frame_busy_start) : 0;
    const bool had_3d_view = frame_has_3d_view;
    frame_has_3d_view = false;

    fb_t *fb = &fb_pool[current_fb_idx];

    fb->surface.width = screen->w;
    fb->surface.height = screen->h;
    fb->surface.stride = screen->w;
    fb->surface.format = RG_PIXEL_PAL565_BE;
    fb->surface.data = fb->pixels;


    // Pixels normally already live in this stable submission buffer. Retain
    // the copy fallback for an unexpected mode larger than the pool.
    const int64_t copy_start = rg_system_timer();
    if (!primary_uses_fb_pool) {
        memcpy(fb->pixels, screen->pixels, (size_t)screen->w * screen->h);
    }
    if (screen_surface.palette) {
        memcpy(fb->surface.palette, screen_surface.palette, 256 * 2);
    }
    uint32_t copy_us = (uint32_t)(rg_system_timer() - copy_start);

    rg_display_submit(&fb->surface, 0);

    // A blocking submit to Retro-Go's depth-one queue guarantees that the
    // alternate buffer is no longer queued. Point every Duke framebuffer
    // alias at it before the engine starts drawing the next frame.
    if (primary_uses_fb_pool) {
        const int next_fb_idx =
            (current_fb_idx + 1) % MAX_SUBMITTED_SURFACES;
        uint8_t *next_pixels = fb_pool[next_fb_idx].pixels;
        const int64_t sync_start = rg_system_timer();

        if (force_full_sync || !had_3d_view) {
            // Splash screens, fades and other 2D paths are incremental.
            // Seed the alternate buffer completely before it is reused.
            memcpy(next_pixels, fb->pixels, (size_t)screen->w * screen->h);
        } else {
            // The 3D viewport is fully redrawn. Preserve only the incremental
            // status bar, borders and other pixels outside that rectangle.
            int x1 = windowx1;
            int x2 = windowx2;
            int y1 = windowy1;
            int y2 = windowy2;
            if (x1 < 0) x1 = 0;
            if (x1 > screen->w) x1 = screen->w;
            if (x2 < -1) x2 = -1;
            if (x2 >= screen->w) x2 = screen->w - 1;
            if (y1 < 0) y1 = 0;
            if (y1 > screen->h) y1 = screen->h;
            if (y2 < -1) y2 = -1;
            if (y2 >= screen->h) y2 = screen->h - 1;
            const size_t stride = (size_t)screen->w;

            if (y1 > 0)
                memcpy(next_pixels, fb->pixels, stride * y1);
            if (y2 + 1 < screen->h)
                memcpy(next_pixels + stride * (y2 + 1),
                       fb->pixels + stride * (y2 + 1),
                       stride * (screen->h - y2 - 1));
            if (x1 > 0 || x2 + 1 < screen->w) {
                for (int y = y1; y <= y2; y++) {
                    uint8_t *dst = next_pixels + stride * y;
                    const uint8_t *src = fb->pixels + stride * y;
                    if (x1 > 0)
                        memcpy(dst, src, x1);
                    if (x2 + 1 < screen->w)
                        memcpy(dst + x2 + 1, src + x2 + 1,
                               screen->w - x2 - 1);
                }
            }
        }
        copy_us += (uint32_t)(rg_system_timer() - sync_start);

        current_fb_idx = next_fb_idx;
        screen->pixels = fb_pool[current_fb_idx].pixels;
        extern uint8_t *frameplace;
        extern uint8_t *frameoffset;
        frameoffset = frameplace = (uint8_t *)screen->pixels;
    } else {
        current_fb_idx = (current_fb_idx + 1) % MAX_SUBMITTED_SURFACES;
    }

    rg_system_tick(render_us + copy_us);

    limit_fps(60);
    frame_busy_start = rg_system_timer();

	return 0;
}

int SDL_VideoModeOK(int width, int height, int bpp, Uint32 flags)
{
	if(bpp == 8)
		return 1;
	return 0;
}

void SDL_LockDisplay()
{
}

void SDL_UnlockDisplay()
{
}
