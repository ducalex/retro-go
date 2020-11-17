#pragma once

#include <stdint.h>
#include "odroid_colors.h"

typedef enum {
    SCREEN_UPDATE_EMPTY,
    SCREEN_UPDATE_FULL,
    SCREEN_UPDATE_PARTIAL,
    SCREEN_UPDATE_ERROR,
} screen_update_t;

typedef enum
{
    ODROID_BACKLIGHT_LEVEL0 = 0,
    ODROID_BACKLIGHT_LEVEL1 = 1,
    ODROID_BACKLIGHT_LEVEL2 = 2,
    ODROID_BACKLIGHT_LEVEL3 = 3,
    ODROID_BACKLIGHT_LEVEL4 = 4,
    ODROID_BACKLIGHT_LEVEL_COUNT = 5,
} odroid_display_backlight_t;

typedef enum
{
    ODROID_DISPLAY_SCALING_OFF = 0,  // No scaling, center image on screen
    ODROID_DISPLAY_SCALING_FIT,       // Scale and preserve aspect ratio
    ODROID_DISPLAY_SCALING_FILL,      // Scale and stretch to fill screen
    ODROID_DISPLAY_SCALING_COUNT
} odroid_display_scaling_t;

typedef enum
{
    ODROID_DISPLAY_FILTER_OFF = 0x0,
    ODROID_DISPLAY_FILTER_LINEAR_X = 0x1,
    ODROID_DISPLAY_FILTER_LINEAR_Y = 0x2,
    ODROID_DISPLAY_FILTER_BILINEAR = 0x3,
    // ODROID_DISPLAY_FILTER_SCANLINE,
    ODROID_DISPLAY_FILTER_COUNT = 4,
} odroid_display_filter_t;

typedef enum
{
   ODROID_PIXEL_FORMAT_565_BE = 0,  // 16bit 565 big endian (prefered for our lcd)
   ODROID_PIXEL_FORMAT_565_LE,      // 16bit 565 little endian
} odroid_pixel_format_t;

typedef enum
{
   ODROID_DISPLAY_ROTATION_OFF = 0,
   ODROID_DISPLAY_ROTATION_AUTO,
   ODROID_DISPLAY_ROTATION_LEFT,
   ODROID_DISPLAY_ROTATION_RIGHT,
   ODROID_DISPLAY_ROTATION_COUNT,
} odroid_display_rotation_t;

typedef struct {
    short left;
    short width;
    short repeat;
} odroid_line_diff_t;

typedef struct {
    int width;          // In px
    int height;         // In px
    int stride;         // In bytes
    int pixel_size;     // In bytes
    int pixel_mask;     // Unused if no palette
    int pixel_clear;    // Clear each pixel to this value after reading it (-1 to disable)
    void *buffer;       // Should be at least height*stride bytes
    void *palette;      //
    uint8_t pal_shift_mask;
    odroid_line_diff_t diff[256];
} odroid_video_frame_t;

void odroid_display_init();
void odroid_display_deinit();
void odroid_display_drain_spi();
void odroid_display_write(short left, short top, short width, short height, const uint16_t* bufferLE);
void odroid_display_write_rect(short left, short top, short width, short height, short stride, const uint16_t* bufferLE);
void odroid_display_clear(uint16_t colorLE);
void odroid_display_show_hourglass();
void odroid_display_force_refresh(void);
void odroid_display_set_scale(short width, short height, double aspect_ratio);
short odroid_display_queue_update(odroid_video_frame_t *frame, odroid_video_frame_t *previousFrame);

odroid_display_backlight_t odroid_display_get_backlight();
void odroid_display_set_backlight(odroid_display_backlight_t level);

odroid_display_scaling_t odroid_display_get_scaling_mode(void);
void odroid_display_set_scaling_mode(odroid_display_scaling_t mode);

odroid_display_filter_t odroid_display_get_filter_mode(void);
void odroid_display_set_filter_mode(odroid_display_filter_t mode);

odroid_display_rotation_t odroid_display_get_rotation(void);
void odroid_display_set_rotation(odroid_display_rotation_t rotation);
