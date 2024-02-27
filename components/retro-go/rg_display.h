#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_DISPLAY_SCALING_OFF = 0, // No scaling, center image on screen
    RG_DISPLAY_SCALING_FIT,     // Scale and preserve aspect ratio
    RG_DISPLAY_SCALING_FULL,    // Scale and stretch to fill screen
    RG_DISPLAY_SCALING_CUSTOM,  // Custom width and height
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

typedef enum
{
    RG_DISPLAY_BACKLIGHT_MIN = 1,
    RG_DISPLAY_BACKLIGHT_MAX = 100,
} display_backlight_t;

typedef struct
{
    display_rotation_t rotation;
    display_scaling_t scaling;
    display_filter_t filter;
    display_backlight_t backlight;
    char *border_file;
    int custom_width, custom_height;
} rg_display_config_t;

typedef struct
{
    int32_t totalFrames;
    int32_t fullFrames;
    int32_t partFrames;
    int32_t delayedFrames;
    int64_t busyTime;
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
        bool ready;
    } source;
    bool changed;
} rg_display_t;

#include "rg_surface.h"

typedef rg_surface_t rg_video_update_t;

void rg_display_init(void);
void rg_display_deinit(void);
void rg_display_write(int left, int top, int width, int height, int stride, const uint16_t *buffer,
                      rg_pixel_flags_t flags);
void rg_display_clear(uint16_t color_le);
bool rg_display_sync(bool block);
void rg_display_force_redraw(void);
bool rg_display_save_frame(const char *filename, const rg_surface_t *frame, int width, int height);
void rg_display_set_source_format(int width, int height, int crop_h, int crop_v, int stride, rg_pixel_flags_t format);
void rg_display_submit(const rg_surface_t *update, uint32_t flags);

rg_display_counters_t rg_display_get_counters(void);
const rg_display_t *rg_display_get_info(void);

void rg_display_set_scaling(display_scaling_t scaling);
display_scaling_t rg_display_get_scaling(void);
void rg_display_set_filter(display_filter_t filter);
display_filter_t rg_display_get_filter(void);
void rg_display_set_rotation(display_rotation_t rotation);
display_rotation_t rg_display_get_rotation(void);
void rg_display_set_backlight(display_backlight_t percent);
display_backlight_t rg_display_get_backlight(void);
void rg_display_set_border(const char *filename);
char *rg_display_get_border(void);
void rg_display_set_custom_width(int width);
int rg_display_get_custom_width(void);
void rg_display_set_custom_height(int height);
int rg_display_get_custom_height(void);
