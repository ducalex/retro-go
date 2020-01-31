#pragma once

#include <stdint.h>

enum ODROID_SD_ERR {
    ODROID_SD_ERR_BADFILE = 0x01,
    ODROID_SD_ERR_NOCARD = 0x02
};

typedef struct __attribute__((__packed__)) {
    short left;
    short width;
    short repeat;
} odroid_scanline;

void ili9341_init();
void ili9341_poweroff();
void ili9341_prepare();
void send_reset_drawing(int left, int top, int width, int height);
void send_continue_wait();
void send_continue_line(uint16_t *line, int width, int lineCount);

void ili9341_write_frame_gb(uint16_t* buffer, int scale);

void backlight_percentage_set(int value);
//void ili9341_write_frame(uint16_t* buffer);
void odroid_display_reset_scale(int width, int height);
void odroid_display_set_scale(int width, int height, float aspect);
void ili9341_write_frame_8bit(uint8_t* buffer, odroid_scanline* diff, int width, int height, int stride, uint8_t pixel_mask, uint16_t* palette);
void ili9341_write_frame_rectangle(short left, short top, short width, short height, uint16_t* buffer);
void ili9341_clear(uint16_t color);
void ili9341_blank_screen();
void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer);
void display_tasktonotify_set(int value);

int is_backlight_initialized();
void odroid_display_show_splash();
void odroid_display_drain_spi();
void odroid_display_lock();
void odroid_display_unlock();
void odroid_display_show_sderr(int errNum);
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
