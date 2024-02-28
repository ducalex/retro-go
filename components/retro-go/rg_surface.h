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
    RG_PIXEL_FORMAT = 0xFF,

    // Additional misc flags
    RG_PIXEL_NOSYNC = 0b100000000,
} rg_pixel_flags_t;

#define RG_PIXEL_GET_SIZE(format) ((format & RG_PIXEL_PALETTE) ? 1 : (((format) & RG_PIXEL_FORMAT) == RG_PIXEL_888 ? 3 : 2))

// TO DO: Properly scale values instead of discarding extra bits
#define RGB888_TO_RGB565(r, g, b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | (((b) & 0x1F)))

typedef struct
{
    int top, left;
    int width, height;
} rg_rect_t;

typedef struct
{
    int width, height;
    int stride; // offset, pixlen
    uint32_t format;
    uint16_t *palette;
    union {
        void *buffer;
        void *data;
    };
} rg_surface_t;

// rg_image_t always contains a RG_PIXEL_565_LE surface
typedef rg_surface_t rg_image_t;

rg_surface_t *rg_surface_create(int width, int height, uint32_t format, uint32_t alloc_flags);
rg_surface_t *rg_surface_load_image(const uint8_t *data, size_t data_len, uint32_t flags);
rg_surface_t *rg_surface_load_image_file(const char *filename, uint32_t flags);
void rg_surface_free(rg_surface_t *surface);
bool rg_surface_copy(const rg_surface_t *source, const rg_rect_t *source_rect, rg_surface_t *dest,
                     const rg_rect_t *dest_rect, bool scale);
rg_surface_t *rg_surface_resize(const rg_surface_t *source, int new_width, int new_height);
bool rg_surface_save_image_file(const rg_surface_t *source, const char *filename, int width, int height);
