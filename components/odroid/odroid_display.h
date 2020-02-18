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
    uint8_t top;
    uint8_t repeat;
    short left;
    short width;
} odroid_scanline;

void ili9341_init();
void ili9341_poweroff();
void ili9341_write_frame(uint16_t* buffer);
void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer);
void ili9341_write_frame_scaled(void* buffer, odroid_scanline* diff, short width, short height, short stride,
                                short pixel_width, uint8_t pixel_mask, uint16_t* palette);
void ili9341_fill_screen(uint16_t color);
void ili9341_blank_screen();

int odroid_display_backlight_get();
void odroid_display_backlight_set(int level);

void odroid_display_reset_scale(short width, short height);
void odroid_display_set_scale(short width, short height, float aspect);

void odroid_display_drain_spi();
void odroid_display_lock();
void odroid_display_unlock();
void odroid_display_show_error(int errNum);
void odroid_display_show_hourglass();

void odroid_buffer_diff(void *buffer,
                        void *old_buffer,
                        uint16_t *palette,
                        uint16_t *old_palette,
                        short width, short height, short stride,
                        short pixel_width, uint8_t pixel_mask,
                        uint8_t palette_shift_mask,
                        odroid_scanline *out_diff);
