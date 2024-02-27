#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum
{
    // These are legacy flags, don't use them. Still needed for now
    RG_PIXEL_565 = 0b0000, // 16bit 565
    RG_PIXEL_555 = 0b0010, // 16bit 555
    RG_PIXEL_PAL = 0b0001, // Use palette
    RG_PIXEL_BE = 0b0000,  // big endian
    RG_PIXEL_LE = 0b0100,  // little endian

    // These are the ones that should be used by applications:
    RG_PIXEL_565_BE = RG_PIXEL_565 | RG_PIXEL_BE,
    RG_PIXEL_565_LE = RG_PIXEL_565 | RG_PIXEL_LE,
    RG_PIXEL_PAL565_BE = RG_PIXEL_565 | RG_PIXEL_BE | RG_PIXEL_PAL,
    RG_PIXEL_PAL565_LE = RG_PIXEL_565 | RG_PIXEL_LE | RG_PIXEL_PAL,

    // Additional misc flags
    RG_PIXEL_NOSYNC = 0b100000000,

    // Masks
    RG_PIXEL_FORMAT = 0x00FF,
    RG_PIXEL_FLAGS = 0xFF00,

    RG_PIXEL_NO_CHANGE = -1,
} rg_pixel_flags_t;

typedef struct
{
    int top, left;
    int width, height;
} rg_rect_t;

typedef struct
{
    int width, height;
    int stride, offset;
    uint32_t format;
    uint16_t palette[256];
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
                     const rg_rect_t *dest_rect, bool resample);
rg_surface_t *rg_surface_resize(const rg_surface_t *source, int new_width, int new_height);
bool rg_surface_save_image_file(const rg_surface_t *source, const char *filename, int width, int height);
