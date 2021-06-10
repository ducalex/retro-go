#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    RG_UPDATE_EMPTY = 0,
    RG_UPDATE_FULL,
    RG_UPDATE_PARTIAL,
    RG_UPDATE_ERROR,
} rg_update_t;

typedef enum
{
    RG_DISPLAY_BACKLIGHT_0 = 0,
    RG_DISPLAY_BACKLIGHT_1,
    RG_DISPLAY_BACKLIGHT_2,
    RG_DISPLAY_BACKLIGHT_3,
    RG_DISPLAY_BACKLIGHT_4,
    RG_DISPLAY_BACKLIGHT_5,
    RG_DISPLAY_BACKLIGHT_MIN = RG_DISPLAY_BACKLIGHT_0,
    RG_DISPLAY_BACKLIGHT_MAX = RG_DISPLAY_BACKLIGHT_5,
    RG_DISPLAY_BACKLIGHT_DEFAULT = RG_DISPLAY_BACKLIGHT_3,
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
    RG_DISPLAY_FILTER_OFF = 0,
    RG_DISPLAY_FILTER_HORIZ,
    RG_DISPLAY_FILTER_VERT,
    RG_DISPLAY_FILTER_BOTH,
    RG_DISPLAY_FILTER_COUNT,
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
    display_backlight_t backlight; // _level
    display_rotation_t rotation; // _mode
    display_scaling_t scaling; // _mode
    display_filter_t filter; // _mode
    struct {
        uint32_t width;
        uint32_t height;
    } screen;
    struct {
        uint32_t width;
        uint32_t height;
        uint32_t x, y;
    } viewport;
    struct {
        uint32_t width;
        uint32_t height;
        uint32_t x, y;
    } source;
    bool changed;
} rg_display_t;

typedef struct {
    short left;     // uint32_t left:10;
    short width;    // uint32_t width:10;
    short repeat;   // uint32_t repeat:10;
} rg_line_diff_t;

typedef struct {
    uint32_t flags;         // bitwise of RG_PIXEL_*
    uint32_t width;         // In px
    uint32_t height;        // In px
    uint32_t stride;        // In bytes
    uint32_t pixel_mask;    // Used only with palette
    void *buffer;           // Should be at least height*stride bytes. expects uint8_t * | uint16_t *
    void *palette;          // rg_video_palette_t expects uint16_t * of size pixel_mask
    void *my_arg;           // Reserved for user usage
    rg_line_diff_t diff[256];
} rg_video_frame_t;

void rg_display_init(void);
void rg_display_deinit(void);
void rg_display_drain_spi(void);
void rg_display_write(int left, int top, int width, int height, int stride, const uint16_t* buffer);
void rg_display_clear(uint16_t colorLE);
void rg_display_reset_config(void);
void rg_display_show_info(const char *text, int timeout_ms);
bool rg_display_save_frame(const char *filename, rg_video_frame_t *frame, int width, int height);
rg_update_t rg_display_queue_update(rg_video_frame_t *frame, rg_video_frame_t *previousFrame);
rg_display_t rg_display_get_status(void);


void rg_display_set_scaling(display_scaling_t scaling);
display_scaling_t rg_display_get_scaling(void);
void rg_display_set_filter(display_filter_t filter);
display_filter_t rg_display_get_filter(void);
void rg_display_set_rotation(display_rotation_t rotation);
display_rotation_t rg_display_get_rotation(void);
void rg_display_set_backlight(display_backlight_t backlight);
display_backlight_t rg_display_get_backlight(void);
