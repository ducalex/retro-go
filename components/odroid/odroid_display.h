#pragma once

#include <stdint.h>

enum ODROID_SYS_ERROR {
    ODROID_SD_ERR_BADFILE = 1,
    ODROID_SD_ERR_NOCARD,
    ODROID_SD_ERR_NOBIOS,
    ODROID_EMU_ERR_CRASH,
};

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
    ODROID_DISPLAY_SCALING_NONE = 0, // No scaling, center image on screen
    ODROID_DISPLAY_SCALING_SCALE,    // Scale and preserve aspect ratio
    ODROID_DISPLAY_SCALING_FILL,     // Scale and stretch to fill screen
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
    int16_t width;       // In px
    int16_t height;      // In px
    int16_t stride;      // In bytes
    uint8_t pixel_size;  // In bytes
    uint8_t pixel_mask;  // Put 0xFF if no palette
    void *buffer;        // Should match pixel_size
    uint16_t *palette;   //
    uint8_t pal_shift_mask;
    odroid_line_diff diff[240];
} odroid_video_frame;

typedef struct {
    odroid_video_frame *frame;
    odroid_line_diff diff[240];
    int8_t use_diff;
} odroid_video_update;

extern volatile int8_t displayScalingMode;
extern volatile int8_t displayFilterMode;
extern volatile int8_t forceVideoRefresh;

void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer);

int8_t odroid_display_backlight_get();
void odroid_display_backlight_set(int8_t level);

void odroid_display_reset_scale(short width, short height);
void odroid_display_set_scale(short width, short height, float aspect_ratio);
void odroid_display_write_frame(odroid_video_frame *frame);
short odroid_display_queue_update(odroid_video_frame *frame, odroid_video_frame *previousFrame);

void odroid_display_init();
void odroid_display_deinit();
void odroid_display_lock();
void odroid_display_unlock();
void odroid_display_drain_spi();
void odroid_display_clear(uint16_t color);
void odroid_display_show_error(int errNum);
void odroid_display_show_hourglass();
