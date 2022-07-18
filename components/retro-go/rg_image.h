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

rg_image_t *rg_image_load_from_file(const char *filename, uint32_t flags);
rg_image_t *rg_image_load_from_memory(const uint8_t *data, size_t data_len, uint32_t flags);
rg_image_t *rg_image_alloc(size_t width, size_t height);
rg_image_t *rg_image_copy_resampled(const rg_image_t *img, int new_width, int new_height, int new_format);
bool rg_image_save_to_file(const char *filename, const rg_image_t *img, uint32_t flags);
void rg_image_free(rg_image_t *img);
