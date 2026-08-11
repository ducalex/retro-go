#include "rg_system.h"
#include "rg_display.h"

#include <stdlib.h>
#include <string.h>

#define LCD_BUFFER_LENGTH (RG_SCREEN_WIDTH * 4) // In pixels

// static rg_display_driver_t driver;
static rg_task_t *display_task_queue;
static rg_display_counters_t counters;
static rg_display_config_t config;
static rg_surface_t *border;
static rg_display_t display;
static int16_t map_viewport_to_source_x[RG_SCREEN_WIDTH + 1];
static int16_t map_viewport_to_source_y[RG_SCREEN_HEIGHT + 1];
static uint32_t screen_line_checksum[RG_SCREEN_HEIGHT + 1];

#define LINE_IS_REPEATED(Y) (map_viewport_to_source_y[(Y)] == map_viewport_to_source_y[(Y) - 1])
// This is to avoid flooring a number that is approximated to .9999999 and be explicit about it
#define FLOAT_TO_INT(x) ((int)((x) + 0.1f))

static const char *SETTING_BACKLIGHT = "DispBacklight";
static const char *SETTING_SCALING = "DispScaling";
static const char *SETTING_FILTER = "DispFilter";
static const char *SETTING_ROTATION = "DispRotation";
static const char *SETTING_BORDER = "DispBorder";
static const char *SETTING_CUSTOM_ZOOM = "DispCustomZoom";
static const char *SETTING_TOUCH_OVERLAY = "DispTouchOverlay";

static void lcd_init(void);
static void lcd_deinit(void);
static void lcd_sync(void);
static void lcd_set_rotation(int rotation);
static void lcd_set_backlight(float percent);
static void lcd_set_window(int left, int top, int width, int height);
static inline uint16_t *lcd_get_buffer(size_t length);
static inline void lcd_send_buffer(uint16_t *buffer, size_t length);

#if RG_SCREEN_DRIVER == 0 || RG_SCREEN_DRIVER == 1 /* ILI9341/ST7789 */
#include "drivers/display/ili9341.h"
#elif RG_SCREEN_DRIVER == 99
#include "drivers/display/sdl2.h"
#else
#include "drivers/display/dummy.h"
#endif

#ifdef RG_GAMEPAD_TOUCH_MAP
// Ghost artwork for the RG_GAMEPAD_TOUCH_MAP zones: a D-pad cross with arrowheads,
// lettered A/B buttons and labelled pills for the menu keys. Two intensities: zone
// geometry is faint (25% white), glyphs and labels are strong (50% white). Pixels
// inside the emulator viewport are blended into the frame at submit time (see
// write_update); pixels on the letterbox bars are painted once per viewport change.
#define TOUCH_OVERLAY_FAINT_LE  0x4208 // matches blending 25% white over black
#define TOUCH_OVERLAY_STRONG_LE 0x7BEF // matches blending 50% white over black

static struct {
    uint8_t *faint;   // 1bpp, panel coordinates: zone geometry
    uint8_t *strong;  // 1bpp, panel coordinates: glyphs and labels
    uint8_t *button;  // the always-visible eye toggle (RG_TOUCH_OVERLAY_TOGGLE_ZONE)
    uint8_t *slash;   // strike-through over the eye, shown while the overlay is off
    uint8_t *row_any; // per-row occupancy of faint|strong, for fast skipping
    uint8_t *row_btn; // per-row occupancy of button|slash
    int stride;
} touch_overlay;

// 5x7 glyphs (rows top to bottom, bit 4 = leftmost), only the letters we label with
static const struct { char ch; uint8_t rows[7]; } touch_overlay_font[] = {
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
};

static void touch_overlay_set_pixel(uint8_t *mask, int x, int y)
{
    if (x >= 0 && y >= 0 && x < display.screen.real_width && y < display.screen.real_height)
    {
        mask[y * touch_overlay.stride + (x >> 3)] |= 1 << (x & 7);
        touch_overlay.row_any[y] = 1;
    }
}

static void touch_overlay_text(int cx, int cy, const char *text, int scale)
{
    const int len = (int)strlen(text);
    const int y0 = cy - (7 * scale) / 2;
    int x0 = cx - (len * 6 * scale - scale) / 2;
    for (int c = 0; c < len; ++c, x0 += 6 * scale)
    {
        for (size_t g = 0; g < RG_COUNT(touch_overlay_font); ++g)
        {
            if (touch_overlay_font[g].ch != text[c])
                continue;
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (touch_overlay_font[g].rows[row] & (0x10 >> col))
                        for (int sy = 0; sy < scale; ++sy)
                            for (int sx = 0; sx < scale; ++sx)
                                touch_overlay_set_pixel(touch_overlay.strong,
                                    x0 + col * scale + sx, y0 + row * scale + sy);
            break;
        }
    }
}

// Filled triangle pointing in (dx,dy), one of which must be 0. i=0 is the tip.
static void touch_overlay_arrow(int cx, int cy, int dx, int dy, int size)
{
    for (int i = 0; i < size; ++i)
        for (int j = -i; j <= i; ++j)
            touch_overlay_set_pixel(touch_overlay.strong,
                cx + (dx ? dx * (size / 2 - i) : j),
                cy + (dy ? dy * (size / 2 - i) : j));
}

static bool touch_overlay_in_rrect(int x, int y, int x1, int y1, int x2, int y2, int r)
{
    if (x < x1 || x > x2 || y < y1 || y > y2)
        return false;
    const int dx = RG_MAX(RG_MAX(x1 + r - x, x - (x2 - r)), 0);
    const int dy = RG_MAX(RG_MAX(y1 + r - y, y - (y2 - r)), 0);
    return dx * dx + dy * dy <= r * r;
}

static void touch_overlay_rrect(int x1, int y1, int x2, int y2, int r)
{
    for (int y = y1; y <= y2; ++y)
        for (int x = x1; x <= x2; ++x)
        {
            if (!touch_overlay_in_rrect(x, y, x1, y1, x2, y2, r))
                continue;
            // ~2px border: pixel is inside, but its 2-neighborhood isn't fully inside
            if (!touch_overlay_in_rrect(x - 2, y, x1, y1, x2, y2, r) ||
                !touch_overlay_in_rrect(x + 2, y, x1, y1, x2, y2, r) ||
                !touch_overlay_in_rrect(x, y - 2, x1, y1, x2, y2, r) ||
                !touch_overlay_in_rrect(x, y + 2, x1, y1, x2, y2, r))
                touch_overlay_set_pixel(touch_overlay.faint, x, y);
        }
}

static void touch_overlay_circle(int cx, int cy, int r)
{
    for (int y = cy - r; y <= cy + r; ++y)
        for (int x = cx - r; x <= cx + r; ++x)
        {
            const int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            if (d2 <= r * r && d2 >= (r - 2) * (r - 2))
                touch_overlay_set_pixel(touch_overlay.faint, x, y);
        }
}

static void touch_overlay_pill(const rg_keymap_touch_t *z, const char *label)
{
    const int x1 = z->x1 + 2, y1 = z->y1 + 2, x2 = z->x2 - 2, y2 = z->y2 - 2;
    touch_overlay_rrect(x1, y1, x2, y2, RG_MIN(x2 - x1, y2 - y1) / 3);
    touch_overlay_text((x1 + x2) / 2, (y1 + y2) / 2, label, 1);
}

#ifdef RG_TOUCH_OVERLAY_TOGGLE_ZONE
static void touch_overlay_btn_pixel(uint8_t *mask, int x, int y)
{
    if (x >= 0 && y >= 0 && x < display.screen.real_width && y < display.screen.real_height)
    {
        mask[y * touch_overlay.stride + (x >> 3)] |= 1 << (x & 7);
        touch_overlay.row_btn[y] = 1;
    }
}

// The eye button: an elliptical eye with a pupil (always shown), plus a diagonal
// strike-through that only renders while the overlay is disabled.
static void touch_overlay_build_button(void)
{
    static const struct { int x1, y1, x2, y2; } tz = RG_TOUCH_OVERLAY_TOGGLE_ZONE;
    const int cx = (tz.x1 + tz.x2) / 2, cy = (tz.y1 + tz.y2) / 2;
    const int a = 13, b = 7, edge = a * a * b * b;
    for (int dy = -b; dy <= b; ++dy)
        for (int dx = -a; dx <= a; ++dx)
        {
            const int v = dx * dx * b * b + dy * dy * a * a;
            if ((v <= edge && v >= edge * 5 / 8) || (dx * dx + dy * dy <= 3 * 3))
                touch_overlay_btn_pixel(touch_overlay.button, cx + dx, cy + dy);
        }
    for (int dx = -14; dx <= 14; ++dx)
        for (int t = -1; t <= 1; ++t)
            touch_overlay_btn_pixel(touch_overlay.slash, cx + dx, cy + dx * 10 / 14 + t);
}
#endif

// D-pad cross geometry: bar positions derived from the gap between the four zones
static struct { int x1, y1, x2, y2, vx1, vx2, hy1, hy2; } touch_overlay_cross;

static bool touch_overlay_in_cross(int x, int y)
{
    if (x < touch_overlay_cross.x1 || x > touch_overlay_cross.x2 ||
        y < touch_overlay_cross.y1 || y > touch_overlay_cross.y2)
        return false;
    return (x >= touch_overlay_cross.vx1 && x <= touch_overlay_cross.vx2) ||
           (y >= touch_overlay_cross.hy1 && y <= touch_overlay_cross.hy2);
}

static void touch_overlay_dpad(const rg_keymap_touch_t *up, const rg_keymap_touch_t *down,
                               const rg_keymap_touch_t *left, const rg_keymap_touch_t *right)
{
    touch_overlay_cross.x1 = RG_MIN(up->x1, RG_MIN(down->x1, RG_MIN(left->x1, right->x1)));
    touch_overlay_cross.y1 = RG_MIN(up->y1, RG_MIN(down->y1, RG_MIN(left->y1, right->y1)));
    touch_overlay_cross.x2 = RG_MAX(up->x2, RG_MAX(down->x2, RG_MAX(left->x2, right->x2)));
    touch_overlay_cross.y2 = RG_MAX(up->y2, RG_MAX(down->y2, RG_MAX(left->y2, right->y2)));
    touch_overlay_cross.vx1 = left->x2 + 1;
    touch_overlay_cross.vx2 = right->x1 - 1;
    touch_overlay_cross.hy1 = up->y2 + 1;
    touch_overlay_cross.hy2 = down->y1 - 1;

    if (touch_overlay_cross.vx1 >= touch_overlay_cross.vx2 ||
        touch_overlay_cross.hy1 >= touch_overlay_cross.hy2)
    {
        // Unexpected layout: fall back to plain outlines
        const rg_keymap_touch_t *zones[] = {up, down, left, right};
        for (size_t i = 0; i < 4; ++i)
            touch_overlay_rrect(zones[i]->x1 + 2, zones[i]->y1 + 2, zones[i]->x2 - 2, zones[i]->y2 - 2, 6);
        return;
    }

    for (int y = touch_overlay_cross.y1; y <= touch_overlay_cross.y2; ++y)
        for (int x = touch_overlay_cross.x1; x <= touch_overlay_cross.x2; ++x)
        {
            if (!touch_overlay_in_cross(x, y))
                continue;
            if (!touch_overlay_in_cross(x - 2, y) || !touch_overlay_in_cross(x + 2, y) ||
                !touch_overlay_in_cross(x, y - 2) || !touch_overlay_in_cross(x, y + 2))
                touch_overlay_set_pixel(touch_overlay.faint, x, y);
        }

    const int arrow = 8;
    touch_overlay_arrow((up->x1 + up->x2) / 2, (up->y1 + up->y2) / 2, 0, -1, arrow);
    touch_overlay_arrow((down->x1 + down->x2) / 2, (down->y1 + down->y2) / 2, 0, 1, arrow);
    touch_overlay_arrow((left->x1 + left->x2) / 2, (left->y1 + left->y2) / 2, -1, 0, arrow);
    touch_overlay_arrow((right->x1 + right->x2) / 2, (right->y1 + right->y2) / 2, 1, 0, arrow);
}

static void touch_overlay_build_mask(void)
{
    size_t zones = 0;
    const rg_keymap_touch_t *map = rg_input_get_touch_map(&zones);
    touch_overlay.stride = (display.screen.real_width + 7) / 8;
    touch_overlay.faint = calloc(display.screen.real_height, touch_overlay.stride);
    touch_overlay.strong = calloc(display.screen.real_height, touch_overlay.stride);
    touch_overlay.button = calloc(display.screen.real_height, touch_overlay.stride);
    touch_overlay.slash = calloc(display.screen.real_height, touch_overlay.stride);
    touch_overlay.row_any = calloc(display.screen.real_height, 1);
    touch_overlay.row_btn = calloc(display.screen.real_height, 1);
    if (!map || !touch_overlay.faint || !touch_overlay.strong || !touch_overlay.button ||
        !touch_overlay.slash || !touch_overlay.row_any || !touch_overlay.row_btn)
    {
        free(touch_overlay.faint), touch_overlay.faint = NULL;
        free(touch_overlay.strong), touch_overlay.strong = NULL;
        free(touch_overlay.button), touch_overlay.button = NULL;
        free(touch_overlay.slash), touch_overlay.slash = NULL;
        free(touch_overlay.row_any), touch_overlay.row_any = NULL;
        free(touch_overlay.row_btn), touch_overlay.row_btn = NULL;
        return;
    }

    const rg_keymap_touch_t *up = NULL, *down = NULL, *left = NULL, *right = NULL;

    for (size_t i = 0; i < zones; ++i)
    {
        const rg_keymap_touch_t *z = &map[i];
        const int cx = (z->x1 + z->x2) / 2, cy = (z->y1 + z->y2) / 2;
        switch (z->key)
        {
            case RG_KEY_UP:    up = z;    break; // the d-pad is drawn as a group below
            case RG_KEY_DOWN:  down = z;  break;
            case RG_KEY_LEFT:  left = z;  break;
            case RG_KEY_RIGHT: right = z; break;
            case RG_KEY_A:
            case RG_KEY_B:
                touch_overlay_circle(cx, cy, RG_MIN(z->x2 - z->x1, z->y2 - z->y1) / 2 - 3);
                touch_overlay_text(cx, cy, z->key == RG_KEY_A ? "A" : "B", 2);
                break;
            case RG_KEY_SELECT: touch_overlay_pill(z, "SELECT"); break;
            case RG_KEY_START:  touch_overlay_pill(z, "START");  break;
            case RG_KEY_MENU:   touch_overlay_pill(z, "MENU");   break;
            case RG_KEY_OPTION: touch_overlay_pill(z, "OPT");    break;
            default:
                touch_overlay_rrect(z->x1 + 2, z->y1 + 2, z->x2 - 2, z->y2 - 2, 6);
                break;
        }
    }

    if (up && down && left && right)
        touch_overlay_dpad(up, down, left, right);
    else
    {
        const rg_keymap_touch_t *zones_left[] = {up, down, left, right};
        for (size_t i = 0; i < 4; ++i)
            if (zones_left[i])
                touch_overlay_rrect(zones_left[i]->x1 + 2, zones_left[i]->y1 + 2,
                                    zones_left[i]->x2 - 2, zones_left[i]->y2 - 2, 6);
    }

#ifdef RG_TOUCH_OVERLAY_TOGGLE_ZONE
    touch_overlay_build_button();
#endif
}

static void touch_overlay_draw_run(int x1, int x2, int y, uint16_t color_le)
{
    if (x1 <= x2)
        rg_display_clear_rect(x1 - display.screen.margins.left, y - display.screen.margins.top,
                              x2 - x1 + 1, 1, color_le);
}

static void touch_overlay_draw_static_mask(const uint8_t *mask, const uint8_t *rows, uint16_t color_le,
                                           int vp_x1, int vp_y1, int vp_x2, int vp_y2)
{
    for (int y = 0; y < display.screen.real_height; ++y)
    {
        if (!rows[y])
            continue;
        const uint8_t *mrow = &mask[y * touch_overlay.stride];
        const bool row_in_vp = (y >= vp_y1 && y <= vp_y2);
        int run = -1;
        for (int x = 0; x <= display.screen.real_width; ++x)
        {
            bool set = x < display.screen.real_width && (mrow[x >> 3] & (1 << (x & 7)));
            if (set && run < 0)
                run = x;
            if (!set && run >= 0)
            {
                if (!row_in_vp) // whole run sits on the top/bottom bars
                {
                    touch_overlay_draw_run(run, x - 1, y, color_le);
                }
                else // only the parts hanging over the side bars; write_update blends the rest
                {
                    touch_overlay_draw_run(run, RG_MIN(x - 1, vp_x1 - 1), y, color_le);
                    touch_overlay_draw_run(RG_MAX(run, vp_x2 + 1), x - 1, y, color_le);
                }
                run = -1;
            }
        }
    }
}

static void touch_overlay_draw_static(void)
{
    if (!touch_overlay.row_any)
        return;
    const int vp_x1 = display.screen.margins.left + display.viewport.left;
    const int vp_y1 = display.screen.margins.top + display.viewport.top;
    const int vp_x2 = vp_x1 + display.viewport.width - 1;
    const int vp_y2 = vp_y1 + display.viewport.height - 1;
    if (config.touch_overlay)
    {
        touch_overlay_draw_static_mask(touch_overlay.faint, touch_overlay.row_any, TOUCH_OVERLAY_FAINT_LE, vp_x1, vp_y1, vp_x2, vp_y2);
        touch_overlay_draw_static_mask(touch_overlay.strong, touch_overlay.row_any, TOUCH_OVERLAY_STRONG_LE, vp_x1, vp_y1, vp_x2, vp_y2);
    }
    touch_overlay_draw_static_mask(touch_overlay.button, touch_overlay.row_btn, TOUCH_OVERLAY_STRONG_LE, vp_x1, vp_y1, vp_x2, vp_y2);
    if (!config.touch_overlay)
        touch_overlay_draw_static_mask(touch_overlay.slash, touch_overlay.row_btn, TOUCH_OVERLAY_STRONG_LE, vp_x1, vp_y1, vp_x2, vp_y2);
}
#endif

static int draw_on_screen_display(int region_start, int region_end)
{
    static unsigned int area_dirty = 0;
    rg_margins_t margins = rg_gui_get_safe_area();
    int left = display.screen.width - margins.right - 28;
    int top = margins.top + 4;
    int border = 3;
    int width = 20;
    int height = 14;

    if (region_end < top + height)
        return top + height;

    // Low battery indicator
    if (rg_system_get_indicator(RG_INDICATOR_POWER_LOW) && ((counters.totalFrames / 20) & 1))
    {
        rg_display_clear_rect(left, top, width, height, C_RED); // Main body
        rg_display_clear_rect(left + width, top + height / 4, border, height / 2, C_RED); // The tab
        rg_display_clear_rect(left + border, top + border, width - border * 2, height - border * 2, C_BLACK); // The fill
        // memset(&screen_line_checksum[top], 0, sizeof(uint32_t) * height);
        area_dirty |= (1 << RG_INDICATOR_POWER_LOW);
    }
    else if (area_dirty)
    {
        if (display.viewport.width < display.screen.width || display.viewport.height < display.screen.height)
            rg_display_clear_rect(left, top, width + border, height, C_BLACK);
        memset(&screen_line_checksum[top], 0, sizeof(uint32_t) * height);
        area_dirty = 0;
    }
    return 0;
}

static inline unsigned blend_pixels(unsigned a, unsigned b)
{
    // Fast path (taken 80-90% of the time)
    if (a == b)
        return a;

    // Not the original author, but a good explanation is found at:
    // https://medium.com/@luc.trudeau/fast-averaging-of-high-color-16-bit-pixels-cb4ac7fd1488
    a = (a << 8) | (a >> 8);
    b = (b << 8) | (b >> 8);
    unsigned s = a ^ b;
    unsigned v = ((s & 0xF7DEU) >> 1) + (a & b) + (s & 0x0821U);
    return (v << 8) | (v >> 8);

    // This is my attempt at averaging two 565BE values without swapping bytes (3x the speed of the code above)
    // return (((a ^ b) & 0b1101111011110110U) >> 1) + (a & b);
}

static inline void write_update(const rg_surface_t *update)
{
    const int64_t time_start = rg_system_timer();

    bool filter_x = display.viewport.filter_x;
    bool filter_y = display.viewport.filter_y;
    int draw_left = display.viewport.left;
    int draw_top = display.viewport.top;
    int draw_width = display.viewport.width;
    int draw_height = display.viewport.height;

    int crop_left = 0;
    int crop_top = 0;

    if (draw_left < 0)
    {
        crop_left += -draw_left * display.viewport.step_x;
        draw_width += draw_left * 2;
        draw_left = 0;
    }

    if (draw_top < 0)
    {
        crop_top += -draw_top * display.viewport.step_y;
        draw_height += draw_top * 2;
        draw_top = 0;
    }

    const int format = update->format;
    const int stride = update->stride;
    const void *data = update->data + update->offset + (crop_top * stride) + (crop_left * RG_PIXEL_GET_SIZE(format));
    const uint16_t *palette = update->palette;

    const bool partial_update = RG_SCREEN_PARTIAL_UPDATES;

    int lines_per_buffer = LCD_BUFFER_LENGTH / draw_width;
    int lines_remaining = draw_height;
    int lines_updated = 0;
    int window_top = -1;
    int osd_next_call = 20;

    for (int y = 0; y < draw_height;)
    {
        int lines_to_copy = RG_MIN(lines_per_buffer, lines_remaining);

        if (lines_to_copy < 1)
            break;

        // The vertical filter requires a block to start and end with unscaled lines
        if (filter_y)
        {
            while (lines_to_copy > 1 && (LINE_IS_REPEATED(y + lines_to_copy - 1) ||
                                         LINE_IS_REPEATED(y + lines_to_copy)))
                --lines_to_copy;
        }

        uint16_t *line_buffer = lcd_get_buffer(LCD_BUFFER_LENGTH);
        uint16_t *line_buffer_ptr = line_buffer;

        uint32_t checksum = 0xFFFFFFFF;
        bool need_update = !partial_update;

        for (int i = 0; i < lines_to_copy; ++i)
        {
            if (i > 0 && LINE_IS_REPEATED(y))
            {
                memcpy(line_buffer_ptr, line_buffer_ptr - draw_width, draw_width * 2);
                line_buffer_ptr += draw_width;
            }
            else
            {
                #define RENDER_LINE(PTR_TYPE, PIXEL) { \
                    PTR_TYPE *buffer = (PTR_TYPE *)(data + map_viewport_to_source_y[y] * stride);\
                    for (int xx = 0; xx < draw_width; ++xx) { \
                        int x = map_viewport_to_source_x[xx]; \
                        *line_buffer_ptr++ = (PIXEL); \
                    } \
                }
                if (format & RG_PIXEL_PALETTE)
                    RENDER_LINE(uint8_t, palette[buffer[x]])
                else if (format == RG_PIXEL_565_LE)
                    RENDER_LINE(uint16_t, (buffer[x] << 8) | (buffer[x] >> 8))
                else
                    RENDER_LINE(uint16_t, buffer[x])

                if (partial_update)
                {
                    checksum = rg_hash((void*)(line_buffer_ptr - draw_width), draw_width * 2);
                }
            }

            if (screen_line_checksum[draw_top + y] != checksum)
            {
                screen_line_checksum[draw_top + y] = checksum;
                need_update = true;
            }

            ++y;
        }

        if (filter_x && need_update)
        {
            for (int i = 0; i < lines_to_copy; ++i)
            {
                uint16_t *buffer = line_buffer + i * draw_width;
                for (int x = 1; x < draw_width - 1; ++x)
                {
                    if (map_viewport_to_source_x[x] == map_viewport_to_source_x[x - 1])
                    {
                        buffer[x] = blend_pixels(buffer[x - 1], buffer[x + 1]);
                    }
                }
            }
        }

        if (filter_y && need_update)
        {
            int top = y - lines_to_copy;
            for (int i = 1; i < lines_to_copy - 1; ++i)
            {
                if (LINE_IS_REPEATED(top + i))
                {
                    uint16_t *lineA = line_buffer + (i - 1) * draw_width;
                    uint16_t *lineB = line_buffer + (i + 0) * draw_width;
                    uint16_t *lineC = line_buffer + (i + 1) * draw_width;
                    for (size_t x = 0; x < draw_width; ++x)
                    {
                        lineB[x] = blend_pixels(lineA[x], lineC[x]);
                    }
                }
            }
        }

#ifdef RG_GAMEPAD_TOUCH_MAP
        if (touch_overlay.row_any && need_update)
        {
            // Ghost the touch overlay into the outgoing block. This happens after the
            // checksums, so partial updates keep working; toggling the overlay goes
            // through display.changed which resets the checksums anyway. The eye button
            // (and its strike-through when disabled) renders even when the art doesn't.
            const int block_top = display.screen.margins.top + draw_top + y - lines_to_copy;
            const int panel_left = display.screen.margins.left + draw_left;
            for (int i = 0; i < lines_to_copy; ++i)
            {
                const int py = block_top + i;
                const bool art = config.touch_overlay && touch_overlay.row_any[py];
                const bool btn = touch_overlay.row_btn[py];
                if (!art && !btn)
                    continue;
                const uint8_t *frow = &touch_overlay.faint[py * touch_overlay.stride];
                const uint8_t *srow = &touch_overlay.strong[py * touch_overlay.stride];
                const uint8_t *brow = &touch_overlay.button[py * touch_overlay.stride];
                const uint8_t *xrow = &touch_overlay.slash[py * touch_overlay.stride];
                uint16_t *line = line_buffer + i * draw_width;
                for (int xx = 0; xx < draw_width; ++xx)
                {
                    const int px = panel_left + xx;
                    const int byi = px >> 3, bit = 1 << (px & 7);
                    if ((btn && ((brow[byi] & bit) || (!config.touch_overlay && (xrow[byi] & bit)))) ||
                        (art && (srow[byi] & bit)))
                        line[xx] = blend_pixels(line[xx], 0xFFFF); // 50% white
                    else if (art && (frow[byi] & bit))
                        line[xx] = blend_pixels(line[xx], blend_pixels(line[xx], 0xFFFF)); // 25% white
                }
            }
        }
#endif

        if (need_update)
        {
            int left = display.screen.margins.left + draw_left;
            int top = display.screen.margins.top + draw_top + y - lines_to_copy;
            if (top != window_top)
                lcd_set_window(left, top, draw_width, lines_remaining);
            lcd_send_buffer(line_buffer, draw_width * lines_to_copy);
            window_top = top + lines_to_copy;
            lines_updated += lines_to_copy;
        }
        else
        {
            // Return unused buffer
            lcd_send_buffer(line_buffer, 0);
        }

        // Drawing the OSD as we progress reduces flicker compared to doing it once at the end
        if (osd_next_call && draw_top + y >= osd_next_call)
        {
            osd_next_call = draw_on_screen_display(0, draw_top + y);
            window_top = -1;
        }

        lines_remaining -= lines_to_copy;
    }

    if (lines_updated > draw_height * 0.80f)
        counters.fullFrames++;
    else
        counters.partFrames++;
    counters.busyTime += rg_system_timer() - time_start;
}

static void update_viewport_scaling(void)
{
    int screen_width = display.screen.width;
    int screen_height = display.screen.height;
    int src_width = display.source.width;
    int src_height = display.source.height;
    int new_width = src_width;
    int new_height = src_height;

    if (config.scaling == RG_DISPLAY_SCALING_FULL)
    {
        new_width = screen_width;
        new_height = screen_height;
    }
    else if (config.scaling == RG_DISPLAY_SCALING_FIT)
    {
        new_width = FLOAT_TO_INT(screen_height * ((float)src_width / src_height));
        new_height = screen_height;
        if (new_width > screen_width) {
            new_width = screen_width;
            new_height = FLOAT_TO_INT(screen_width * ((float)src_height / src_width));
        }
    }
    else if (config.scaling == RG_DISPLAY_SCALING_ZOOM)
    {
        new_width = FLOAT_TO_INT(src_width * config.custom_zoom);
        new_height = FLOAT_TO_INT(src_height * config.custom_zoom);
    }

    // Everything works better when we use even dimensions!
    new_width &= ~1;
    new_height &= ~1;

    display.viewport.left = (screen_width - new_width) / 2;
    display.viewport.top = (screen_height - new_height) / 2;
    display.viewport.width = new_width;
    display.viewport.height = new_height;

    display.viewport.step_x = (float)src_width / display.viewport.width;
    display.viewport.step_y = (float)src_height / display.viewport.height;

    display.viewport.filter_x = (config.filter == RG_DISPLAY_FILTER_HORIZ || config.filter == RG_DISPLAY_FILTER_BOTH) &&
                                (config.scaling && (display.viewport.width % src_width) != 0);
    display.viewport.filter_y = (config.filter == RG_DISPLAY_FILTER_VERT || config.filter == RG_DISPLAY_FILTER_BOTH) &&
                                (config.scaling && (display.viewport.height % src_height) != 0);

    memset(screen_line_checksum, 0, sizeof(screen_line_checksum));

    for (int x = 0; x < screen_width; ++x)
        map_viewport_to_source_x[x] = FLOAT_TO_INT(x * display.viewport.step_x);
    for (int y = 0; y < screen_height; ++y)
        map_viewport_to_source_y[y] = FLOAT_TO_INT(y * display.viewport.step_y);

    RG_LOGI("%dx%d@%.3f => %dx%d@%.3f left:%d top:%d step_x:%.2f step_y:%.2f", src_width, src_height,
            (float)src_width / src_height, new_width, new_height, (float)new_width / new_height,
            display.viewport.left, display.viewport.top, display.viewport.step_x, display.viewport.step_y);
}

static bool load_border_file(const char *filename)
{
    RG_LOGI("Loading border file: %s", filename ?: "(none)");

    free(border), border = NULL;
    display.changed = true;

    if (filename && (border = rg_surface_load_image_file(filename, 0)))
    {
        if (border->width != rg_display_get_width() || border->height != rg_display_get_height())
        {
            rg_surface_t *resized = rg_surface_resize(border, rg_display_get_width(), rg_display_get_height());
            if (resized)
            {
                rg_surface_free(border);
                border = resized;
            }
        }
        return true;
    }
    return false;
}

IRAM_ATTR
static void display_task(void *arg)
{
    rg_task_msg_t msg;

    while (rg_task_peek(&msg))
    {
        // Received a shutdown request!
        if (msg.type == RG_TASK_MSG_STOP)
            break;

        if (display.changed)
        {
            update_viewport_scaling();
            // Clear the screen if the viewport doesn't cover the entire screen because garbage could remain on the sides
            if (display.viewport.width < display.screen.width || display.viewport.height < display.screen.height)
            {
                if (border)
                    rg_display_write_rect(0, 0, border->width, border->height, 0, border->data, RG_DISPLAY_WRITE_NOSYNC);
                else
                    rg_display_clear_except(display.viewport.left, display.viewport.top, display.viewport.width, display.viewport.height, C_BLACK);
            }
            #ifdef RG_GAMEPAD_TOUCH_MAP
            touch_overlay_draw_static();
            #endif
            display.changed = false;
        }

        write_update(msg.dataPtr);
        // draw_on_screen_display(0, display.screen.height);
        rg_task_receive(&msg);

        lcd_sync();
    }
}

#ifdef RG_GAMEPAD_TOUCH_MAP
void rg_display_set_touch_overlay(bool enabled)
{
    config.touch_overlay = enabled;
    rg_settings_set_number(NS_GLOBAL, SETTING_TOUCH_OVERLAY, enabled);
    display.changed = true; // next submit resets the checksums and repaints the bars
}

bool rg_display_get_touch_overlay(void)
{
    return config.touch_overlay;
}
#endif

void rg_display_force_redraw(void)
{
    display.changed = true;
    // memset(screen_line_checksum, 0, sizeof(screen_line_checksum));
    rg_system_event(RG_EVENT_REDRAW, NULL);
    rg_display_sync(true);
}

const rg_display_t *rg_display_get_info(void)
{
    return &display;
}

rg_display_counters_t rg_display_get_counters(void)
{
    return counters;
}

int rg_display_get_width(void)
{
    // return display.screen.real_width - (display.screen.margins.left + display.screen.margins.right);
    return display.screen.width;
}

int rg_display_get_height(void)
{
    // return display.screen.real_height - (display.screen.margins.top + display.screen.margins.bottom);
    return display.screen.height;
}

void rg_display_set_scaling(display_scaling_t scaling)
{
    config.scaling = RG_MIN(RG_MAX(0, scaling), RG_DISPLAY_SCALING_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_SCALING, config.scaling);
    display.changed = true;
}

display_scaling_t rg_display_get_scaling(void)
{
    return config.scaling;
}

void rg_display_set_custom_zoom(double factor)
{
    config.custom_zoom = RG_MIN(RG_MAX(0.1, factor), 2.0);
    rg_settings_set_number(NS_APP, SETTING_CUSTOM_ZOOM, config.custom_zoom);
    display.changed = true;
}

double rg_display_get_custom_zoom(void)
{
    return config.custom_zoom;
}

void rg_display_set_filter(display_filter_t filter)
{
    config.filter = RG_MIN(RG_MAX(0, filter), RG_DISPLAY_FILTER_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_FILTER, config.filter);
    display.changed = true;
}

display_filter_t rg_display_get_filter(void)
{
    return config.filter;
}

void rg_display_set_rotation(display_rotation_t rotation)
{
    config.rotation = RG_MIN(RG_MAX(0, rotation), RG_DISPLAY_ROTATION_COUNT - 1);
    rg_settings_set_number(NS_APP, SETTING_SCALING, config.rotation);
    display.changed = true;
}

display_rotation_t rg_display_get_rotation(void)
{
    return config.rotation;
}

void rg_display_set_backlight(display_backlight_t percent)
{
    config.backlight = RG_MIN(RG_MAX(percent, RG_DISPLAY_BACKLIGHT_MIN), RG_DISPLAY_BACKLIGHT_MAX);
    rg_settings_set_number(NS_GLOBAL, SETTING_BACKLIGHT, config.backlight);
    lcd_set_backlight(config.backlight);
}

display_backlight_t rg_display_get_backlight(void)
{
    return config.backlight;
}

void rg_display_set_border(const char *filename)
{
    free(config.border_file);
    config.border_file = NULL;

    if (load_border_file(filename))
    {
        rg_settings_set_string(NS_APP, SETTING_BORDER, filename);
        config.border_file = strdup(filename);
    }
    else
    {
        rg_settings_set_string(NS_APP, SETTING_BORDER, NULL);
        config.border_file = NULL;
    }
    display.changed = true;
}

char *rg_display_get_border(void)
{
    return rg_settings_get_string(NS_APP, SETTING_BORDER, NULL);
}

void rg_display_submit(const rg_surface_t *update, uint32_t flags)
{
    const int64_t time_start = rg_system_timer();

    // Those things should probably be asserted, but this is a new system let's be forgiving...
    if (!update || !update->data)
        return;

    if (display.source.width != update->width || display.source.height != update->height)
    {
        rg_display_sync(true);
        display.source.width = update->width;
        display.source.height = update->height;
        display.changed = true;
    }

    rg_task_send(display_task_queue, &(rg_task_msg_t){.dataPtr = update});

    counters.blockTime += rg_system_timer() - time_start;
    counters.totalFrames++;
}

bool rg_display_sync(bool block)
{
    while (block && rg_task_messages_waiting(display_task_queue))
        continue; // We should probably yield?
    return !rg_task_messages_waiting(display_task_queue);
}

void rg_display_write_rect(int left, int top, int width, int height, int stride, const uint16_t *buffer, uint32_t flags)
{
    RG_ASSERT_ARG(buffer);

    // calc stride before clipping width
    stride = RG_MAX(stride, width * 2);

    // Clipping
    width = RG_MIN(width, display.screen.width - left);
    height = RG_MIN(height, display.screen.height - top);

    // This can happen when left or top is out of bound
    if (width < 0 || height < 0)
        return;

    // This will work for now because we rarely draw from different threads (so all we need is ensure
    // that we're not interrupting a display update). But what we SHOULD be doing is acquire a lock
    // before every call to lcd_set_window and release it only after the last call to lcd_send_buffer.
    if (!(flags & RG_DISPLAY_WRITE_NOSYNC))
        rg_display_sync(true);

    // This isn't really necessary but it makes sense to invalidate
    // the lines we're about to overwrite...
    for (size_t y = 0; y < height; ++y)
        screen_line_checksum[top + y] = 0;

    lcd_set_window(left + display.screen.margins.left, top + display.screen.margins.top, width, height);

    for (size_t y = 0; y < height;)
    {
        uint16_t *lcd_buffer = lcd_get_buffer(LCD_BUFFER_LENGTH);
        size_t num_lines = RG_MIN(LCD_BUFFER_LENGTH / width, height - y);

        // Copy line by line because stride may not match width
        for (size_t line = 0; line < num_lines; ++line)
        {
            uint16_t *src = (void *)buffer + ((y + line) * stride);
            uint16_t *dst = lcd_buffer + (line * width);
            if (flags & RG_DISPLAY_WRITE_NOSWAP)
            {
                memcpy(dst, src, width * 2);
            }
            else
            {
                for (size_t i = 0; i < width; ++i)
                    dst[i] = (src[i] >> 8) | (src[i] << 8);
            }
        }

        lcd_send_buffer(lcd_buffer, width * num_lines);
        y += num_lines;
    }

    lcd_sync();
}

void rg_display_clear_rect(int left, int top, int width, int height, uint16_t color_le)
{
    const uint16_t color_be = (color_le << 8) | (color_le >> 8);
    int pixels_remaining = width * height;
    if (pixels_remaining > 0)
    {
        lcd_set_window(left + display.screen.margins.left, top + display.screen.margins.top, width, height);
        while (pixels_remaining > 0)
        {
            uint16_t *buffer = lcd_get_buffer(LCD_BUFFER_LENGTH);
            int pixels = RG_MIN(pixels_remaining, LCD_BUFFER_LENGTH);
            for (size_t j = 0; j < pixels; ++j)
                buffer[j] = color_be;
            lcd_send_buffer(buffer, pixels);
            pixels_remaining -= pixels;
        }
    }
}

void rg_display_clear_except(int left, int top, int width, int height, uint16_t color_le)
{
    // Clear everything except the specified area
    // FIXME: Do not ignore left/top...
    int left_offset = -display.screen.margins.left;
    int top_offset = -display.screen.margins.top;
    int horiz = (display.screen.real_width - width + 1) / 2;
    int vert = (display.screen.real_height - height + 1) / 2;
    rg_display_clear_rect(left_offset, top_offset, horiz, display.screen.real_height, color_le); // Left
    rg_display_clear_rect(left_offset + horiz + width, top_offset, horiz, display.screen.real_height, color_le); // Right
    rg_display_clear_rect(left_offset + horiz, top_offset, display.screen.real_width - horiz * 2, vert, color_le); // Top
    rg_display_clear_rect(left_offset + horiz, top_offset + vert + height, display.screen.real_width - horiz * 2, vert, color_le); // Bottom
}

void rg_display_clear(uint16_t color_le)
{
    // We ignore margins here, we want to fill the entire screen
    rg_display_clear_rect(-display.screen.margins.left, -display.screen.margins.top, display.screen.real_width,
                          display.screen.real_height, color_le);
}

void rg_display_deinit(void)
{
    rg_task_send(display_task_queue, &(rg_task_msg_t){.type = RG_TASK_MSG_STOP});
    // lcd_set_backlight(0);
    lcd_deinit();
    RG_LOGI("Display terminated.\n");
}

void rg_display_init(void)
{
    RG_LOGI("Initialization...\n");
    // TO DO: We probably should call the setters to ensure valid values...
    config = (rg_display_config_t){
        .backlight = rg_settings_get_number(NS_GLOBAL, SETTING_BACKLIGHT, 80),
        .scaling = rg_settings_get_number(NS_APP, SETTING_SCALING, RG_DISPLAY_SCALING_FIT),
        .filter = rg_settings_get_number(NS_APP, SETTING_FILTER, RG_DISPLAY_FILTER_BOTH),
        .rotation = rg_settings_get_number(NS_APP, SETTING_ROTATION, RG_DISPLAY_ROTATION_AUTO),
        .border_file = rg_settings_get_string(NS_APP, SETTING_BORDER, NULL),
        .custom_zoom = rg_settings_get_number(NS_APP, SETTING_CUSTOM_ZOOM, 1.0),
        .touch_overlay = rg_settings_get_number(NS_GLOBAL, SETTING_TOUCH_OVERLAY, 1),
    };
    display = (rg_display_t){
        .screen.real_width = RG_SCREEN_WIDTH,
        .screen.real_height = RG_SCREEN_HEIGHT,
        .screen.width = RG_SCREEN_WIDTH,
        .screen.height = RG_SCREEN_HEIGHT,
        .screen.margins = RG_SCREEN_VISIBLE_AREA,
        .changed = true,
    };
    display.screen.width -= display.screen.margins.left + display.screen.margins.right;
    display.screen.height -= display.screen.margins.top + display.screen.margins.bottom;
    #ifdef RG_GAMEPAD_TOUCH_MAP
    touch_overlay_build_mask();
    #endif
    lcd_init();
    rg_display_clear(C_BLACK);
    rg_task_delay(80); // Wait for the screen be cleared before turning on the backlight (40ms doesn't seem to be enough...)
    lcd_set_backlight(config.backlight);
    display_task_queue = rg_task_create("rg_display", &display_task, NULL, 4 * 1024, RG_TASK_PRIORITY_6, 1);
    if (config.border_file)
        load_border_file(config.border_file);
    RG_LOGI("Display ready.\n");
}
