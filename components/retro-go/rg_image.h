#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// rg_image_t contains an RGB565 (LE) image
typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t data[];
} rg_image_t;

// rg_rgb_t contains a 16 or 24 bit RGB color
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rg_rgb_t;

// rg_palette_t contains a 16 or 24bit color palette
typedef struct
{
    rg_rgb_t colors[256];
    size_t size;
    uint8_t format;
} rg_palette_t;


rg_image_t *rg_image_load_from_file(const char *filename, uint32_t flags);
rg_image_t *rg_image_load_from_memory(const uint8_t *data, size_t data_len, uint32_t flags);
rg_image_t *rg_image_alloc(size_t width, size_t height);
bool rg_image_build_palette(rg_palette_t *out, const rg_image_t *img);
bool rg_image_save_to_file(const char *filename, const rg_image_t *img, uint32_t flags);
bool rg_image_save_to_memory(const uint8_t *data, size_t data_len, const rg_image_t *img, uint32_t flags);
void rg_image_free(rg_image_t *img);
