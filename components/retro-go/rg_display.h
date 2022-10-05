#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_UPDATE_EMPTY = 0,
    RG_UPDATE_FULL,
    RG_UPDATE_PARTIAL,
    RG_UPDATE_ERROR,
} rg_update_t;

typedef enum
{
    RG_DISPLAY_UPDATE_PARTIAL = 0,
    RG_DISPLAY_UPDATE_FULL,
    // RG_DISPLAY_UPDATE_INTERLACE,
    // RG_DISPLAY_UPDATE_SMART,
    RG_DISPLAY_UPDATE_COUNT,
} display_update_t;

typedef enum
{
    RG_DISPLAY_SCALING_OFF = 0, // No scaling, center image on screen
    RG_DISPLAY_SCALING_FIT,     // Scale and preserve aspect ratio
    RG_DISPLAY_SCALING_FILL,    // Scale and stretch to fill screen
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
    // These are legacy flags, don't use them. Still needed for now
    RG_PIXEL_565 = 0b0000, // 16bit 565
    RG_PIXEL_555 = 0b0010, // 16bit 555
    RG_PIXEL_PAL = 0b0001, // Use palette
    RG_PIXEL_BE = 0b0000,  // big endian
    RG_PIXEL_LE = 0b0100,  // little endian

    // These are the ones that should be used by applications:
    RG_PIXEL_565_BE = RG_PIXEL_565 | RG_PIXEL_BE,
    RG_PIXEL_565_LE = RG_PIXEL_565 | RG_PIXEL_LE,
    RG_PIXEL_PAL565_BE = RG_PIXEL_565 | RG_PIXEL_BE | RG_PIXEL_PAL,
    RG_PIXEL_PAL565_LE = RG_PIXEL_565 | RG_PIXEL_LE | RG_PIXEL_PAL,
};

typedef struct
{
    display_rotation_t rotation;
    display_scaling_t scaling;
    display_filter_t filter;
    display_update_t update_mode;
    int backlight;
} rg_display_config_t;

typedef struct
{
    int32_t totalFrames;
    int32_t fullFrames;
    int64_t busyTime; // This is only time spent blocking the main task
} rg_display_counters_t;

typedef struct
{
    struct
    {
        int width;
        int height;
        int format;
    } screen;
    struct
    {
        int width;
        int height;
        int x_pos;
        int y_pos;
        int x_inc;
        int y_inc;
    } viewport;
    struct
    {
        int width;
        int height;
        int stride;
        int offset;
        int pixlen;
        int crop_h;
        int crop_v;
        int format;
    } source;
    bool changed;
} rg_display_t;

typedef struct
{
    short left;   // int32_t left:10;
    short width;  // int32_t width:10;
    short repeat; // int32_t repeat:10;
} rg_line_diff_t;

typedef struct
{
    rg_update_t type;
    void *buffer;          // Should be at least height*stride bytes. expects uint8_t * | uint16_t *
    uint16_t palette[256]; // Used in RG_PIXEL_PAL is set
    rg_line_diff_t diff[256];
} rg_video_update_t;

void rg_display_init(void);
void rg_display_deinit(void);
void rg_display_write(int left, int top, int width, int height, int stride,
                      const uint16_t *buffer); // , bool little_endian);
void rg_display_clear(uint16_t color_le);
void rg_display_sync(void);
bool rg_display_is_busy(void);
void rg_display_force_redraw(void);
bool rg_display_save_frame(const char *filename, const rg_video_update_t *frame, int width, int height);
void rg_display_set_source_format(int width, int height, int crop_h, int crop_v, int stride, int format);

rg_update_t rg_display_submit(/*const*/ rg_video_update_t *update, const rg_video_update_t *previousUpdate);
#define rg_display_queue_update rg_display_submit

rg_display_counters_t rg_display_get_counters(void);
rg_display_config_t rg_display_get_config(void);
const rg_display_t *rg_display_get_info(void);

void rg_display_set_scaling(display_scaling_t scaling);
display_scaling_t rg_display_get_scaling(void);
void rg_display_set_filter(display_filter_t filter);
display_filter_t rg_display_get_filter(void);
void rg_display_set_rotation(display_rotation_t rotation);
display_rotation_t rg_display_get_rotation(void);
void rg_display_set_backlight(int percent);
int rg_display_get_backlight(void);
void rg_display_set_update_mode(display_update_t update);
display_update_t rg_display_get_update_mode(void);
