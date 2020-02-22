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

typedef enum
{
    ODROID_DISPLAY_UPDATE_AUTO = 0,
    ODROID_DISPLAY_UPDATE_FULL,
    ODROID_DISPLAY_UPDATE_PARTIAL,
    ODROID_DISPLAY_UPDATE_COUNT,
} odroid_display_update_mode;

typedef struct __attribute__((__packed__)) {
    uint8_t top;
    uint8_t height;
    short left;
    short width;
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
    odroid_line_diff diff[224];
} odroid_video_frame;

typedef struct {
    odroid_video_frame *frame;
    odroid_line_diff diff[224];
    int8_t use_diff;
} odroid_video_update;

extern volatile int8_t scalingMode;
extern volatile int8_t displayUpdateMode;
extern volatile int8_t forceVideoRefresh;

void ili9341_write_frame_rectangleLE(short left, short top, short width, short height, uint16_t* buffer);

int8_t odroid_display_backlight_get();
void odroid_display_backlight_set(int8_t level);

void odroid_display_reset_scale(short width, short height);
void odroid_display_set_scale(short width, short height, float aspect_ratio);
void odroid_display_write_frame(odroid_video_frame *frame);
void odroid_display_queue_update(odroid_video_frame *frame, odroid_video_frame *previousFrame);

void odroid_display_init();
void odroid_display_deinit();
void odroid_display_lock();
void odroid_display_unlock();
void odroid_display_drain_spi();
void odroid_display_clear(uint16_t color);
void odroid_display_show_error(int errNum);
void odroid_display_show_hourglass();
