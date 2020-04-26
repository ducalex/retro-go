#pragma once

#include <stdint.h>

#define ODROID_SCREEN_WIDTH  320
#define ODROID_SCREEN_HEIGHT 240

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
} odroid_backlight_level;

typedef enum
{
    ODROID_DISPLAY_SCALING_NONE = 0,  // No scaling, center image on screen
    ODROID_DISPLAY_SCALING_FIT,       // Scale and preserve aspect ratio
    ODROID_DISPLAY_SCALING_FILL,      // Scale and stretch to fill screen
    ODROID_DISPLAY_SCALING_COUNT
} odroid_display_scaling;

typedef enum
{
    ODROID_DISPLAY_FILTER_NONE = 0x0,
    ODROID_DISPLAY_FILTER_LINEAR_X = 0x1,
    ODROID_DISPLAY_FILTER_LINEAR_Y = 0x2,
    ODROID_DISPLAY_FILTER_BILINEAR = 0x3,
    // ODROID_DISPLAY_FILTER_SCANLINE,
    ODROID_DISPLAY_FILTER_COUNT = 4,
} odroid_display_filter;

typedef struct {
    short left;
    short width;
    short repeat;
} odroid_line_diff;

typedef struct {
    short width;       // In px
    short height;      // In px
    short stride;      // In bytes
    short pixel_size;  // In bytes
    short pixel_mask;  // Put 0xFF if no palette
    short pixel_clear; // Set each pixel to this value after reading it (-1 to disable)
    void *buffer;      // Should match pixel_size
    uint16_t *palette; //
    uint8_t pal_shift_mask;
    odroid_line_diff diff[240];
} odroid_video_frame;

extern volatile int8_t displayScalingMode;
extern volatile int8_t displayFilterMode;
extern volatile int8_t forceVideoRefresh;

int8_t odroid_display_backlight_get();
void odroid_display_backlight_set(int8_t level);

void odroid_display_reset_scale(short width, short height);
void odroid_display_set_scale(short width, short height, float aspect_ratio);
void odroid_display_write_frame(odroid_video_frame *frame);
short odroid_display_queue_update(odroid_video_frame *frame, odroid_video_frame *previousFrame);

void odroid_display_init();
void odroid_display_deinit();
void odroid_display_drain_spi();
void odroid_display_write(short left, short top, short width, short height, uint16_t* bufferLE);
void odroid_display_clear(uint16_t colorLE);
void odroid_display_show_hourglass();

