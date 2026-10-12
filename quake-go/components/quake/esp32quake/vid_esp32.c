#include "rg_system.h"
#include "quakedef.h"
#include "d_local.h"
#include "esp_attr.h"

// viddef_t vid; // Defined in screen.c

#if RG_SCREEN_PIXEL_FORMAT == 0
#define QUAKE_PIXEL_FORMAT RG_PIXEL_PAL565_BE
#else
#define QUAKE_PIXEL_FORMAT RG_PIXEL_PAL565_LE
#endif

// The S3 uses Quake's native 320x200 mode for its substantial renderer saving;
// the faster P4 retains a sharper native-height 320x240 framebuffer.
#if defined(CONFIG_IDF_TARGET_ESP32)
#define BASEWIDTH 160
#define BASEHEIGHT 120
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
#define BASEWIDTH 320
#define BASEHEIGHT 240
#else
#define BASEWIDTH 320
#define BASEHEIGHT 200
#endif

#if defined(CONFIG_IDF_TARGET_ESP32)
static DRAM_ATTR int16_t zbuffer[BASEWIDTH * BASEHEIGHT] __attribute__((aligned(16)));
#else
static EXT_RAM_BSS_ATTR int16_t zbuffer[BASEWIDTH * BASEHEIGHT] __attribute__((aligned(16)));
#endif

// surfcache is large. For original ESP32, keep it in internal DRAM to save PSRAM address space.
#if defined(CONFIG_IDF_TARGET_ESP32P4)
#define SURFCACHE_SIZE (640 * 1024)
static EXT_RAM_BSS_ATTR uint8_t surfcache[SURFCACHE_SIZE] __attribute__((aligned(16)));
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define SURFCACHE_SIZE (640 * 1024)
static EXT_RAM_BSS_ATTR uint8_t surfcache[SURFCACHE_SIZE] __attribute__((aligned(16)));
#else
#define SURFCACHE_SIZE (32 * 1024)
static uint8_t surfcache[SURFCACHE_SIZE] __attribute__((aligned(16)));
#endif

const unsigned short *const d_8to16table = NULL;
const unsigned *const d_8to24table = NULL;

static rg_surface_t *rg_surfaces[2];
static int current_surface = 0;
static rg_surface_t *last_complete_surface = NULL;
static uint8_t current_palette[256 * 3];
static bool palette_dirty[2] = {false, false};
static bool skip_screen_update = false;
static bool display_was_late = false;
static uint32_t display_submit_us = 0;

void VID_SetPalette(unsigned char *palette)
{
    memcpy(current_palette, palette, 256 * 3);
    palette_dirty[0] = true;
    palette_dirty[1] = true;
}

void VID_ShiftPalette(unsigned char *p)
{
    VID_SetPalette(p);
}

void VID_Init(unsigned char *palette)
{
    // Create two surfaces for double-buffering. Use internal RAM for original ESP32.
    for (int i = 0; i < 2; i++) {
#if defined(CONFIG_IDF_TARGET_ESP32)
        rg_surfaces[i] = rg_surface_create(BASEWIDTH, BASEHEIGHT, QUAKE_PIXEL_FORMAT, MEM_FAST);
#else
        rg_surfaces[i] = rg_surface_create(BASEWIDTH, BASEHEIGHT, QUAKE_PIXEL_FORMAT, MEM_SLOW);
#endif
        if (!rg_surfaces[i]) {
            Sys_Error("VID_Init: Could not create surface %d", i);
        }

#if !defined(CONFIG_IDF_TARGET_ESP32)
        // rg_display resolves every indexed pixel through this table. Keep a
        // private palette per queued surface, but place those hot 512 bytes in
        // internal RAM instead of alongside the pixel data in PSRAM.
        uint16_t *fast_palette = rg_alloc(256 * sizeof(uint16_t), MEM_FAST);
        if (!fast_palette) {
            Sys_Error("VID_Init: Could not create fast palette %d", i);
        }
        rg_surfaces[i]->palette = fast_palette;
        rg_surfaces[i]->free_palette = true;
#endif
    }

    vid.width = vid.conwidth = BASEWIDTH;
    vid.height = vid.conheight = BASEHEIGHT;
    vid.rowbytes = vid.conrowbytes = BASEWIDTH;
    // Quake's aspect is the source pixel aspect relative to a 4:3 display.
    // This is 1.0 at 320x240 (P4) and 160x120 (ESP32), but 5:6 at
    // 320x200 (S3), matching the non-square pixels of the original mode.
    vid.aspect = ((float)BASEHEIGHT / (float)BASEWIDTH) * (320.0f / 240.0f);
    vid.numpages = 2;
    vid.colormap = host_colormap;
    vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
    vid.buffer = vid.conbuffer = rg_surfaces[current_surface]->data;
    vid.maxwarpwidth = WARP_WIDTH;
    vid.maxwarpheight = WARP_HEIGHT;
    vid.recalc_refdef = 1;

    d_pzbuffer = (short *)zbuffer;
    D_InitCaches(surfcache, sizeof(surfcache));
    
    // Use fullscreen as the first-launch default without overwriting a mode
    // the user has selected in Retro-Go's display options.
    if (!rg_settings_exists(NS_APP, "DispScaling")) {
        rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);
    }

    unsigned surface_cache_kib = (unsigned)(sizeof(surfcache) / 1024);
    RG_LOGI("VID_Init: %dx%d, double-buffering, surface cache %u KiB, zbuffer at %p, surfaces at %p and %p, palettes at %p and %p",
            BASEWIDTH, BASEHEIGHT, surface_cache_kib, zbuffer,
            rg_surfaces[0]->data, rg_surfaces[1]->data,
            rg_surfaces[0]->palette, rg_surfaces[1]->palette);
}

void VID_Shutdown(void)
{
    // A submitted surface remains owned by the display task until it has been
    // removed from the queue after write_update() finishes reading it.
    while (rg_display_is_busy()) {
        rg_task_yield();
    }

    for (int i = 0; i < 2; i++) {
        if (rg_surfaces[i]) {
            rg_surface_free(rg_surfaces[i]);
            rg_surfaces[i] = NULL;
        }
    }
    last_complete_surface = NULL;
}

void VID_Update(vrect_t *rects)
{
    (void)rects;
    rg_surface_t *surf = rg_surfaces[current_surface];

    if (palette_dirty[current_surface])
    {
        for (int i = 0; i < 256; i++)
        {
            uint8_t r = current_palette[i * 3];
            uint8_t g = current_palette[i * 3 + 1];
            uint8_t b = current_palette[i * 3 + 2];
            uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
#if RG_SCREEN_PIXEL_FORMAT == 0
            surf->palette[i] = (color << 8) | (color >> 8);
#else
            surf->palette[i] = color;
#endif
        }
        palette_dirty[current_surface] = false;
    }

    // The display queue retains this surface until write_update() has finished
    // reading it. With strict two-buffer alternation, the next submit provides
    // any required back-pressure while this frame is displayed in parallel.
    display_was_late = rg_display_is_busy();
    int64_t submit_start = rg_system_timer();
    rg_display_submit(surf, 0);
    display_submit_us += (uint32_t)(rg_system_timer() - submit_start);
    last_complete_surface = surf;

    // Swap to the other surface for next frame
    current_surface = 1 - current_surface;
    vid.buffer = vid.conbuffer = rg_surfaces[current_surface]->data;
}

void VID_SetSkipFrame(qboolean skip)
{
    skip_screen_update = skip;
}

qboolean VID_ShouldSkipFrame(void)
{
    return skip_screen_update;
}

qboolean VID_ConsumeDisplayLate(void)
{
    qboolean late = display_was_late;
    display_was_late = false;
    return late;
}

uint32_t VID_ConsumeDisplayTime(void)
{
    uint32_t elapsed = display_submit_us;
    display_submit_us = 0;
    return elapsed;
}

void VID_Redraw(void)
{
    if (last_complete_surface) {
        rg_display_submit(last_complete_surface, 0);
    }
}

bool VID_SaveScreenshot(const char *filename, int width, int height)
{
    return last_complete_surface &&
           rg_surface_save_image_file(last_complete_surface, filename, width, height);
}

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect(int x, int y, int width, int height)
{
}
