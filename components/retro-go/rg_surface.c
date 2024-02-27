#include "rg_system.h"
#include "rg_surface.h"
#include "lodepng.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define CHECK_SURFACE(surface, retval)                                                                     \
    if (!surface || !surface->data)                                                                        \
    {                                                                                                      \
        RG_LOGE("Invalid surface data");                                                                   \
        return retval;                                                                                     \
    }                                                                                                      \
    else if (surface->width < 1 || surface->width > 4096 || surface->height < 1 || surface->height > 4096) \
    {                                                                                                      \
        RG_LOGE("Invalid surface dimensions %d %d", surface->width, surface->height);                      \
        return retval;                                                                                     \
    }

rg_surface_t *rg_surface_create(int width, int height, uint32_t format, uint32_t alloc_flags)
{
    size_t stride = width * (format & RG_PIXEL_PALETTE ? 1 : (format == RG_PIXEL_888 ? 3 : 2));
    size_t size = sizeof(rg_surface_t) + (height * stride); // + (format & RG_PIXEL_PALETTE ? 512 : 0);
    rg_surface_t *surface = alloc_flags ? rg_alloc(size, alloc_flags) : malloc(size);
    surface->width = width;
    surface->height = height;
    surface->stride = stride;
    surface->offset = 0;
    surface->format = format;
    surface->data = (void*)surface + sizeof(rg_surface_t);
    // surface->palette = (void*)surface + size - 512;
    return surface;
}

void rg_surface_free(rg_surface_t *surface)
{
    free(surface);
}

bool rg_surface_copy(const rg_surface_t *source, const rg_rect_t *source_rect, rg_surface_t *dest, const rg_rect_t *dest_rect, bool scale)
{
    CHECK_SURFACE(source, false);
    CHECK_SURFACE(dest, false);

    int source_format = source->format & RG_PIXEL_FORMAT;
    int dest_format = dest->format & RG_PIXEL_FORMAT;
    int copy_width = dest->width;
    int copy_height = dest->height;
    int transparency = -1;

    // This function will eventually replace rg_gui_copy_buffer and rg_gui_draw_image and
    // and rg_surface_copy_resampled but not today!
    RG_ASSERT(dest_format == RG_PIXEL_565_LE || dest_format == RG_PIXEL_565_BE, "Bad format");
    scale = scale && (dest->width != source->width || dest->height != source->height);

    if (!scale && transparency == -1 && dest->stride == source->stride && dest_format == source_format)
    {
        memcpy(dest->data, source->data, dest->height * dest->stride);
    }
    else
    {
        float step_x = (float)source->width / copy_width;
        float step_y = (float)source->height / copy_height;

        // FIXME: Optimize this (float mult + 3 branches on each pixel is unacceptable)
        // but I don't want macro hell right now and this isn't used for game rendering
        #define COPY_PIXELS(SRC_PIXEL)                                                         \
            for (int y = 0; y < copy_height; ++y)                                              \
            {                                                                                  \
                int src_y = scale ? (y * step_y) : y;                                          \
                const uint8_t *src = source->data + source->offset + (src_y * source->stride); \
                uint16_t *dst = dest->data + dest->offset + (y * dest->stride);                \
                for (int x = 0; x < copy_width; ++x)                                           \
                {                                                                              \
                    int src_x = scale ? (int)(x * step_x) : x;                                 \
                    uint16_t pixel = SRC_PIXEL;                                                \
                    if ((int)pixel == transparency)                                            \
                        continue;                                                              \
                    if (dest_format == RG_PIXEL_565_BE)                                        \
                        dst[x] = (pixel << 8) | (pixel >> 8);                                  \
                    else                                                                       \
                        dst[x] = pixel;                                                        \
                }                                                                              \
            }

        if (source_format == RG_PIXEL_565_LE)
        {
            COPY_PIXELS(((uint16_t *)src)[src_x]);
        }
        else if (source_format == RG_PIXEL_565_BE)
        {
            COPY_PIXELS(((uint16_t *)src)[src_x]; pixel = (pixel << 8) | (pixel >> 8));
        }
        else if (source_format == RG_PIXEL_PAL565_LE)
        {
            COPY_PIXELS(source->palette[src[src_x]]);
        }
        else if (source_format == RG_PIXEL_PAL565_BE)
        {
            COPY_PIXELS(source->palette[src[src_x]]; pixel = (pixel << 8) | (pixel >> 8));
        }
        else if (source_format == RG_PIXEL_888)
        {
            COPY_PIXELS(RGB888_TO_RGB565(src[src_x * 3 + 0], src[src_x * 3 + 1], src[src_x * 3 + 2]));
        }
        else
        {
            RG_LOGE("Invalid source format?");
            return false;
        }
    }

    return true;
}

rg_surface_t *rg_surface_resize(const rg_surface_t *source, int new_width, int new_height)
{
    CHECK_SURFACE(source, NULL);

    if (new_width <= 0 && new_height <= 0)
    {
        new_width = source->width;
        new_height = source->height;
    }
    else if (new_width <= 0)
    {
        new_width = source->width * ((float)new_height / source->height);
    }
    else if (new_height <= 0)
    {
        new_height = source->height * ((float)new_width / source->width);
    }

    rg_surface_t *dest = rg_surface_create(new_width, new_height, RG_PIXEL_565_LE, 0);
    if (!dest)
    {
        RG_LOGW("Out of memory!\n");
        return NULL;
    }
    rg_surface_copy(source, NULL, dest, NULL, true);
    return dest;
}

rg_surface_t *rg_surface_load_image(const uint8_t *data, size_t data_len, uint32_t flags)
{
    RG_ASSERT(data && data_len >= 16, "bad param");

    if (memcmp(data, "\x89PNG", 4) == 0)
    {
        unsigned error, width, height;
        uint8_t *image = NULL;

        error = lodepng_decode24(&image, &width, &height, data, data_len);
        if (error)
        {
            RG_LOGE("PNG decoding failed: %d\n", error);
            return NULL;
        }

        rg_surface_t *img = rg_surface_create(width, height, RG_PIXEL_565_LE, 0);
        if (img)
        {
            size_t pixel_count = width * height;
            const uint8_t *src = image;
            uint16_t *dest = img->data;

            // RGB888 or RGBA8888 to RGB565
            for (size_t i = 0; i < pixel_count; ++i)
            {
                // TO DO: Properly scale values instead of discarding extra bits
                *dest++ = (((src[0] >> 3) & 0x1F) << 11)  // Red
                          | (((src[1] >> 2) & 0x3F) << 5) // Green
                          | (((src[2] >> 3) & 0x1F));     // Blue
                src += 3;
            }
        }
        free(image);
        return img;
    }
    else // RAW565 (uint16 width, uint16 height, uint16 data[])
    {
        int width = ((uint16_t *)data)[0];
        int height = ((uint16_t *)data)[1];
        int size_diff = (width * height * 2 + 4) - data_len;

        if (size_diff >= 0 && size_diff <= 100)
        {
            rg_surface_t *img = rg_surface_create(width, height, RG_PIXEL_565_LE, 0);
            if (img)
            {
                // Image is already RGB565 little endian, just copy it
                memcpy(img->data, data + 4, data_len - 4);
            }
            // else Maybe we could just return (rg_image_t *)buffer; ?
            return img;
        }
    }

    RG_LOGE("Image format not recognized!\n");
    return NULL;
}

rg_surface_t *rg_surface_load_image_file(const char *filename, uint32_t flags)
{
    RG_ASSERT(filename, "bad param");

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        RG_LOGE("Unable to open image file '%s'!\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    size_t data_len = ftell(fp);
    uint8_t *data = malloc(data_len);
    if (!data)
    {
        RG_LOGE("Memory allocation failed (%d bytes)!\n", (int)data_len);
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    fread(data, data_len, 1, fp);
    fclose(fp);

    rg_surface_t *img = rg_surface_load_image(data, data_len, flags);
    free(data);

    return img;
}

bool rg_surface_save_image_file(const rg_surface_t *source, const char *filename, int width, int height)
{
    CHECK_SURFACE(source, false);

    rg_surface_t *img = rg_surface_create(width, height, RG_PIXEL_565_LE, 0);
    if (!img)
        return false;

    rg_surface_copy(source, NULL, img, NULL, true);

    size_t pixel_count = img->width * img->height;
    uint8_t *image = malloc(pixel_count * 3);
    uint8_t *dest = image;
    unsigned error = -1;
    if (image)
    {
        // RGB565 to RGB888
        for (size_t i = 0; i < pixel_count; ++i)
        {
            uint16_t pixel = ((uint16_t *)img->data)[i];
            dest[0] = ((pixel >> 11) & 0x1F) << 3;
            dest[1] = ((pixel >> 5) & 0x3F) << 2;
            dest[2] = ((pixel & 0x1F) << 3);
            dest += 3;
        }
        error = lodepng_encode24_file(filename, image, img->width, img->height);
    }

    rg_surface_free(img);
    free(image);

    if (error == 0)
        return true;

    RG_LOGE("PNG encoding failed: %d\n", error);
    return false;
}
