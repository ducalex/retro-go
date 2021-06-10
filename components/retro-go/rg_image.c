#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lupng.h>
// #include <gifdec.h>
// #include <gifenc.h>

#include "rg_system.h"
#include "rg_image.h"


static inline void copy_rgb565_to_rgb888(uint8_t *dest, const uint16_t *src, size_t pixel_count)
{
    RG_ASSERT(dest && src, "bad param");

    for (int i = 0; i < pixel_count; ++i)
    {
        dest[0] = ((src[i] >> 11) & 0x1F) << 3;
        dest[1] = ((src[i] >> 5) & 0x3F) << 2;
        dest[2] = ((src[i] & 0x1F) << 3);
        dest += 3;
    }
}

static inline void copy_rgb888_to_rgb565(uint16_t *dest, const uint8_t *src, size_t pixel_count)
{
    RG_ASSERT(dest && src, "bad param");

    for (int i = 0; i < pixel_count; ++i)
    {
        dest[i] = (((src[0] >> 3) & 0x1F) << 11)
                | (((src[1] >> 2) & 0x3F) << 5)
                | (((src[2] >> 3) & 0x1F));
        src += 3;
    }
}

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
    if (!data || data_len < 16)
        return NULL;

    if (memcmp(data, "\x89PNG", 4) == 0)
    {
        LuImage *png = luPngReadMem(data, data_len);
        if (!png)
        {
            RG_LOGE("PNG parsing failed!\n");
            return NULL;
        }

        rg_image_t *img = rg_image_alloc(png->width, png->height);
        if (img)
        {
            copy_rgb888_to_rgb565(img->data, png->data, img->width * img->height);
        }
        luImageRelease(png, NULL);
        return img;
    }
    else if (memcmp(data, "GIF89a", 6) == 0)
    {
    #if 0
        gd_GIF *gif = gd_open_gif(filename);
        if (!gif)
        {
            RG_LOGE("GIF parsing failed!\n");
            return NULL;
        }

        rg_image_t *img = rg_image_alloc(gif->width, gif->height);
        void *buffer = malloc(gif->width * gif->height * 3);
        if (img && buffer && gd_get_frame(gif))
        {
            gd_render_frame(gif, buffer);
            copy_rgb888_to_rgb565(img->data, buffer, img->width * img->height);
        }
        gd_close_gif(gif);
        free(buffer);
        return img;
    #endif
    }
    else // RAW565 (uint16 width, uint16 height, uint16 data[])
    {
        int img_width = ((uint16_t *)data)[0];
        int img_height = ((uint16_t *)data)[1];
        int size_diff = (img_width * img_height * 2 + 4) - data_len;

        if (size_diff >= 0 && size_diff <= 100)
        {
            rg_image_t *img = rg_image_alloc(img_width, img_height);
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

    LuImage *png = luImageCreate(img->width, img->height, 3, 8, 0, 0);
    if (!png)
    {
        RG_LOGE("LuImage allocation failed!\n");
        return false;
    }

    copy_rgb565_to_rgb888(png->data, img->data, img->width * img->height);

    if (luPngWriteFile(filename, png))
    {
        luImageRelease(png, 0);
        return true;
    }

    luImageRelease(png, 0);
    return false;
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
    free(img);
}
