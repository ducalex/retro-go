#pragma once

#include <stdint.h>

enum ODROID_SYS_ERROR {
    ODROID_SD_ERR_BADFILE = 1,
    ODROID_SD_ERR_NOCARD,
    ODROID_SD_ERR_NOBIOS,
    ODROID_EMU_ERR_CRASH,
};

typedef enum
{
    ODROID_BACKLIGHT_LEVEL0 = 0,
    ODROID_BACKLIGHT_LEVEL1 = 1,
    ODROID_BACKLIGHT_LEVEL2 = 2,
    ODROID_BACKLIGHT_LEVEL3 = 3,
    ODROID_BACKLIGHT_LEVEL4 = 4,
    ODROID_BACKLIGHT_LEVEL_COUNT = 5,
} odroid_backlight_level;

typedef struct __attribute__((__packed__)) {
    short left;
    short width;
    short repeat;
} odroid_scanline;

void ili9341_init();
void ili9341_poweroff();
void ili9341_write_frame(uint16_t* buffer);
void ili9341_write_frame_gb(uint16_t* buffer, int scale);
void ili9341_write_frame_8bit(uint8_t* buffer, odroid_scanline* diff, int width, int height, int stride, uint8_t pixel_mask, uint16_t* palette);
void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer);
void ili9341_clear(uint16_t color);
void ili9341_blank_screen();

void display_tasktonotify_set(int value);

int odroid_display_backlight_get();
void odroid_display_backlight_set(int level);

void odroid_display_reset_scale(int width, int height);
void odroid_display_set_scale(int width, int height, float aspect);

void odroid_display_drain_spi();
void odroid_display_lock();
void odroid_display_unlock();
void odroid_display_show_error(int errNum);
void odroid_display_show_hourglass();
void odroid_buffer_diff(uint8_t *buffer,
                        uint8_t *old_buffer,
                        uint16_t *palette,
                        uint16_t *old_palette,
                        int width, int height, int stride,
                        uint8_t pixel_mask,
                        uint8_t palette_shift_mask,
                        odroid_scanline *out_diff);
void odroid_buffer_diff_interlaced(uint8_t *buffer,
                                   uint8_t *old_buffer,
                                   uint16_t *palette,
                                   uint16_t *old_palette,
                                   int width, int height, int stride,
                                   uint8_t pixel_mask,
                                   uint8_t palette_shift_mask,
                                   int field,
                                   odroid_scanline *out_diff,
                                   odroid_scanline *old_diff);

int odroid_buffer_diff_count(odroid_scanline *diff, int height);
