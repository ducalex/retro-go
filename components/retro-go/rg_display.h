#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    RG_SCREEN_UPDATE_EMPTY,
    RG_SCREEN_UPDATE_FULL,
    RG_SCREEN_UPDATE_PARTIAL,
    RG_SCREEN_UPDATE_ERROR,
} screen_update_t;

typedef enum
{
    RG_BACKLIGHT_LEVEL0 = 0,
    RG_BACKLIGHT_LEVEL1 = 1,
    RG_BACKLIGHT_LEVEL2 = 2,
    RG_BACKLIGHT_LEVEL3 = 3,
    RG_BACKLIGHT_LEVEL4 = 4,
    RG_BACKLIGHT_LEVEL_COUNT = 5,
} display_backlight_t;

typedef enum
{
    RG_DISPLAY_SCALING_OFF = 0,  // No scaling, center image on screen
    RG_DISPLAY_SCALING_FIT,       // Scale and preserve aspect ratio
    RG_DISPLAY_SCALING_FILL,      // Scale and stretch to fill screen
    RG_DISPLAY_SCALING_COUNT
} display_scaling_t;

typedef enum
{
    RG_DISPLAY_FILTER_OFF = 0x0,
    RG_DISPLAY_FILTER_LINEAR_X = 0x1,
    RG_DISPLAY_FILTER_LINEAR_Y = 0x2,
    RG_DISPLAY_FILTER_BILINEAR = 0x3,
    // RG_DISPLAY_FILTER_SCANLINE,
    RG_DISPLAY_FILTER_COUNT = 4,
} display_filter_t;

typedef enum
{
   RG_DISPLAY_ROTATION_OFF = 0,
   RG_DISPLAY_ROTATION_AUTO,
   RG_DISPLAY_ROTATION_LEFT,
   RG_DISPLAY_ROTATION_RIGHT,
   RG_DISPLAY_ROTATION_COUNT,
} display_rotation_t;

enum
{
    RG_PIXEL_565 = 0b0000, // 16bit 565
    RG_PIXEL_555 = 0b0010, // 16bit 555
    RG_PIXEL_PAL = 0b0001, // Use palette
    RG_PIXEL_BE  = 0b0000, // big endian
    RG_PIXEL_LE  = 0b0100, // little endian
};

typedef struct {
    short left;
    short width;
    short repeat;
} rg_line_diff_t;

typedef struct {
    uint32_t flags;         // bitwise of RG_PIXEL_*
    uint32_t width;         // In px
    uint32_t height;        // In px
    uint32_t stride;        // In bytes
    uint32_t pixel_mask;    // Used only with palette
    void *buffer;           // Should be at least height*stride bytes
    void *palette;          //
    void *my_arg;           // Reserved for user usage
    rg_line_diff_t diff[256];
} rg_video_frame_t;

typedef void (*display_callback_t)(rg_video_frame_t *arg);

void rg_display_init();
void rg_display_deinit();
void rg_display_drain_spi();
void rg_display_write(int left, int top, int width, int height, int stride, const uint16_t* buffer);
void rg_display_clear(uint16_t colorLE);
void rg_display_force_refresh(void);
void rg_display_set_scale(int width, int height, double aspect_ratio);
void rg_display_show_info(const char *text, int timeout_ms);
bool rg_display_save_frame(const char *filename, const rg_video_frame_t *frame, int width, int height);
screen_update_t rg_display_queue_update(rg_video_frame_t *frame, rg_video_frame_t *previousFrame);

display_backlight_t rg_display_get_backlight();
void rg_display_set_backlight(display_backlight_t level);

display_scaling_t rg_display_get_scaling_mode(void);
void rg_display_set_scaling_mode(display_scaling_t mode);

display_filter_t rg_display_get_filter_mode(void);
void rg_display_set_filter_mode(display_filter_t mode);

display_rotation_t rg_display_get_rotation(void);
void rg_display_set_rotation(display_rotation_t rotation);
