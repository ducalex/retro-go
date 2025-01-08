#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_DISPLAY_SCALING_OFF = 0, // No scaling, center image on screen
    RG_DISPLAY_SCALING_FIT,     // Scale and preserve aspect ratio
    RG_DISPLAY_SCALING_FULL,    // Scale and stretch to fill screen
    RG_DISPLAY_SCALING_ZOOM,  // Custom zoom and preserve aspect ratio
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

enum
{
    RG_DISPLAY_WRITE_NOSYNC = (1 << 0),
    RG_DISPLAY_WRITE_NOSWAP = (1 << 1),
};

typedef struct
{
    display_rotation_t rotation;
    display_scaling_t scaling;
    display_filter_t filter;
    display_backlight_t backlight;
    char *border_file;
    double custom_zoom;
} rg_display_config_t;

typedef struct
{
    int32_t totalFrames;
    int32_t fullFrames;
    int32_t partFrames;
    int64_t blockTime;
    int64_t busyTime;
} rg_display_counters_t;

typedef struct
{
    const char *name;                                               // Driver name
    bool (*init)(void);                                             // Init the display
    bool (*deinit)(void);                                           // Deinit the display
    bool (*sync)(void);                                             // Pause until all data has been flushed
    bool (*set_backlight)(float percent);                           // Set backlight 0.0 - 1.0
    bool (*set_window)(int left, int top, int width, int height);   // Set draw window
    uint16_t *(*get_buffer)(size_t length);                         // Get a DMA-capable buffer to write pixels to and send via send_buffer
    bool (*send_buffer)(uint16_t *buffer, size_t length);           // Send data to display (buffer MUST be acquired via get_buffer)
    // bool (*write)(int left, int top, int width, int height, int pitch, const uint16_t data);
} rg_display_driver_t;

typedef struct
{
    struct
    {
        int real_width, real_height; // Real physical resolution
        int margin_top, margin_bottom, margin_left, margin_right;
        int width, height; // Visible resolution (minus margins)
        int format;
    } screen;
    struct
    {
        int top, left;
        int width, height;
        float step_x, step_y;
        bool filter_x, filter_y;
    } viewport;
    struct
    {
        int width, height;
    } source;
    bool changed;
} rg_display_t;

#include "rg_surface.h"

void rg_display_init(void);
void rg_display_deinit(void);
void rg_display_write_rect(int left, int top, int width, int height, int stride, const uint16_t *buffer, uint32_t flags);
void rg_display_clear_rect(int left, int top, int width, int height, uint16_t color_le);
void rg_display_clear_except(int left, int top, int width, int height, uint16_t color_le);
void rg_display_clear(uint16_t color_le);
bool rg_display_sync(bool block);
void rg_display_force_redraw(void);
void rg_display_submit(const rg_surface_t *update, uint32_t flags);

rg_display_counters_t rg_display_get_counters(void);
const rg_display_t *rg_display_get_info(void);
int rg_display_get_width(void);
int rg_display_get_height(void);

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
void rg_display_set_custom_zoom(double factor);
double rg_display_get_custom_zoom(void);
