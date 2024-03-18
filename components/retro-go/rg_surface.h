#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum
{
    RG_PIXEL_565_BE = 0x01, // 16bit RGB
    RG_PIXEL_565_LE = 0x02, // 16bit RGB
    RG_PIXEL_888    = 0x03, // 24bit RGB

    RG_PIXEL_PALETTE = 0x80, // Use palette
    RG_PIXEL_PAL565_BE = RG_PIXEL_565_BE | RG_PIXEL_PALETTE,
    RG_PIXEL_PAL565_LE = RG_PIXEL_565_LE | RG_PIXEL_PALETTE,
    RG_PIXEL_PAL888 = RG_PIXEL_888 | RG_PIXEL_PALETTE,

    // Masks
    RG_PIXEL_FORMAT = 0xFFFF,
} rg_pixel_format_t;

#define RG_PIXEL_GET_SIZE(format) ((format & RG_PIXEL_PALETTE) ? 1 : (((format) & RG_PIXEL_FORMAT) == RG_PIXEL_888 ? 3 : 2))

// color must accept 0-0xFFFF and -1 (transparent)
typedef int rg_color_t;

typedef struct
{
    int left, top;
    int width, height;
} rg_rect_t;

typedef struct
{
    int width, height;
    int stride, offset; // , pixlen
    int format;
    uint16_t *palette;
    void *data;
    bool free_data;
    bool free_palette;
} rg_surface_t;

// rg_image_t always contains a RG_PIXEL_565_LE surface
typedef rg_surface_t rg_image_t;

// Special surface that draws directly to screen. It is write only.
// extern const rg_surface_t SCREEN_SURFACE;

rg_surface_t *rg_surface_create(int width, int height, int format, uint32_t alloc_flags);
rg_surface_t *rg_surface_load_image(const uint8_t *data, size_t data_len, uint32_t flags);
rg_surface_t *rg_surface_load_image_file(const char *filename, uint32_t flags);
void rg_surface_free(rg_surface_t *surface);
bool rg_surface_copy(const rg_surface_t *source, const rg_rect_t *source_rect, rg_surface_t *dest,
                     const rg_rect_t *dest_rect, bool scale);
bool rg_surface_fill(rg_surface_t *dest, const rg_rect_t *rect, rg_color_t color);
rg_surface_t *rg_surface_convert(const rg_surface_t *source, int new_width, int new_height, int new_format);
#define rg_surface_resize(source, new_width, new_height) rg_surface_convert(source, new_width, new_height, RG_PIXEL_565_LE)
bool rg_surface_save_image_file(const rg_surface_t *source, const char *filename, int width, int height);
