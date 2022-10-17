#include "rg_system.h"
#include "lodepng.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


rg_image_t *rg_image_load_from_file(const char *filename, uint32_t flags)
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
        RG_LOGE("Memory allocation failed (%d bytes)!\n", data_len);
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    fread(data, data_len, 1, fp);
    fclose(fp);

    rg_image_t *img = rg_image_load_from_memory(data, data_len, flags);
    free(data);

    return img;
}

rg_image_t *rg_image_load_from_memory(const uint8_t *data, size_t data_len, uint32_t flags)
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

        rg_image_t *img = rg_image_alloc(width, height);
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
            rg_image_t *img = rg_image_alloc(width, height);
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

bool rg_image_save_to_file(const char *filename, const rg_image_t *img, uint32_t flags)
{
    RG_ASSERT(filename && img, "bad param");

    size_t pixel_count = img->width * img->height;
    uint8_t *image = malloc(pixel_count * 3);
    uint8_t *dest = image;
    unsigned error;

    if (!image)
    {
        RG_LOGE("Memory alloc failed!\n");
        return false;
    }

    // RGB565 to RGB888
    for (int i = 0; i < pixel_count; ++i)
    {
        dest[0] = ((img->data[i] >> 11) & 0x1F) << 3;
        dest[1] = ((img->data[i] >> 5) & 0x3F) << 2;
        dest[2] = ((img->data[i] & 0x1F) << 3);
        dest += 3;
    }

    error = lodepng_encode24_file(filename, image, img->width, img->height);
    free(image);

    if (error)
    {
        RG_LOGE("PNG encoding failed: %d\n", error);
        return false;
    }

    return true;
}

rg_image_t *rg_image_copy_resampled(const rg_image_t *img, int new_width, int new_height, int new_format)
{
    RG_ASSERT(img, "bad param");

    if (new_width <= 0 && new_height <= 0)
    {
        new_width = img->width;
        new_height = img->height;
    }
    else if (new_width <= 0)
    {
        new_width = img->width * ((float)new_height / img->height);
    }
    else if (new_height <= 0)
    {
        new_height = img->height * ((float)new_width / img->width);
    }

    rg_image_t *new_img = rg_image_alloc(new_width, new_height);
    if (!new_img)
    {
        RG_LOGW("Out of memory!\n");
    }
    else if (new_width == img->width && new_height == img->height)
    {
        memcpy(new_img, img, (2 + new_width * new_height) * 2);
    }
    else
    {
        float step_x = (float)img->width / new_width;
        float step_y = (float)img->height / new_height;
        uint16_t *dst = new_img->data;
        for (int y = 0; y < new_height; y++)
        {
            for (int x = 0; x < new_width; x++)
            {
                *(dst++) = img->data[((int)(y * step_y) * img->width) + (int)(x * step_x)];
            }
        }
    }
    return new_img;
}

rg_image_t *rg_image_alloc(size_t width, size_t height)
{
    rg_image_t *img = malloc(sizeof(rg_image_t) + width * height * 2);
    if (!img)
    {
        RG_LOGE("Image alloc failed (%dx%d)\n", width, height);
        return NULL;
    }
    img->width = width;
    img->height = height;
    return img;
}

void rg_image_free(rg_image_t *img)
{
    if (img)
        free(img);
}
