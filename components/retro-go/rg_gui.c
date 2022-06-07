#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "bitmaps/image_hourglass.h"
#include "fonts/fonts.h"
#include "rg_system.h"
#include "rg_printf.h"
#include "rg_gui.h"
#include "lodepng.h"

static const rg_gui_theme_t default_theme = {
    .box_background = C_NAVY,
    .box_header = C_WHITE,
    .box_border = C_DIM_GRAY,
    .item_standard = C_WHITE,
    .item_disabled = C_GRAY, // C_DIM_GRAY
    .scrollbar = C_RED,
};
static rg_gui_t gui;

// static const char *SETTING_FONTSIZE     = "FontSize";
static const char *SETTING_FONTTYPE     = "FontType";


void rg_gui_init(void)
{
    gui.screen_width = rg_display_get_status()->screen.width;
    gui.screen_height = rg_display_get_status()->screen.height;
    gui.draw_buffer = rg_alloc(RG_MAX(gui.screen_width, gui.screen_height) * 20 * 2, MEM_SLOW);
    rg_gui_set_font_type(rg_settings_get_number(NS_GLOBAL, SETTING_FONTTYPE, RG_FONT_VERA_12));
    rg_gui_set_theme(&default_theme);
    gui.initialized = true;
}

bool rg_gui_set_theme(const rg_gui_theme_t *theme)
{
    if (!theme)
        return false;

    gui.theme = *theme;
    if (gui.initialized)
        rg_system_event(RG_EVENT_REDRAW, NULL);

    return true;
}

void rg_gui_set_buffered(bool buffered)
{
    if (!buffered)
        free(gui.screen_buffer), gui.screen_buffer = NULL;
    else if (!gui.screen_buffer)
        gui.screen_buffer = rg_alloc(gui.screen_width * gui.screen_height * 2, MEM_SLOW);
}

void rg_gui_flush(void)
{
    if (gui.screen_buffer)
        rg_display_write(0, 0, gui.screen_width, gui.screen_height, 0, gui.screen_buffer);
}

void rg_gui_copy_buffer(int left, int top, int width, int height, int stride, const void *buffer)
{
    if (gui.screen_buffer)
    {
        if (left < 0) left += gui.screen_width;
        if (top < 0) top += gui.screen_height;
        if (stride < width) stride = width * 2;

        width = RG_MIN(width, gui.screen_width - left);
        height = RG_MIN(height, gui.screen_height - top);

        for (int y = 0; y < height; ++y)
        {
            uint16_t *dst = gui.screen_buffer + (top + y) * gui.screen_width + left;
            const uint16_t *src = (void*)buffer + y * stride;
            for (int x = 0; x < width; ++x)
                if (src[x] != C_TRANSPARENT)
                    dst[x] = src[x];
        }
    }
    else
    {
        rg_display_write(left, top, width, height, stride, buffer);
    }
}

static size_t get_glyph(uint16_t *output, const rg_font_t *font, int points, uint8_t c)
{
    size_t glyph_width = 8;

    // Some glyphs are always zero width
    if (c == '\r' || c == '\n')
        return 0;

    if (font->type == 0) // Bitmap
    {
        if (output && c < font->chars)
            for (int y = 0; y < font->height; y++)
                output[y] = font->data[(c * font->height) + y];
        glyph_width = font->width;
    }
    else // Proportional
    {
        // Based on code by Boris Lovosevic (https://github.com/loboris)
        int charCode, adjYOffset, width, height, xOffset, xDelta;
        const uint8_t *data = font->data;
        do {
            charCode = *data++;
            adjYOffset = *data++;
            width = *data++;
            height = *data++;
            xOffset = *data++;
            xOffset = xOffset < 0x80 ? xOffset : -(0xFF - xOffset);
            xDelta = *data++;

            if (c != charCode && charCode != 0xFF && width != 0) {
                data += (((width * height) - 1) / 8) + 1;
            }
        } while ((c != charCode) && (charCode != 0xFF));

        if (c == charCode)
        {
            glyph_width = ((width > xDelta) ? width : xDelta);
            if (output)
            {
                int ch = 0, mask = 0x80;
                for (int y = 0; y < height; y++) {
                    output[adjYOffset + y] = 0;
                    for (int x = 0; x < width; x++) {
                        if (((x + (y * width)) % 8) == 0) {
                            mask = 0x80;
                            ch = *data++;
                        }
                        if ((ch & mask) != 0)
                            output[adjYOffset + y] |= (1 << (xOffset + x));
                        mask >>= 1;
                    }
                }
            }
        }
    }

    // Vertical stretching
    if (output && points && points != font->height)
    {
        float scale = (float)points / font->height;
        for (int y = points - 1; y >= 0; y--)
            output[y] = output[(int)(y / scale)];
    }

    return glyph_width;
}

bool rg_gui_set_font_type(int type)
{
    if (type < 0)
        type += RG_FONT_MAX;

    if (type < 0 || type > RG_FONT_MAX - 1)
        return false;

    gui.font = fonts[type];
    gui.font_type = type;
    gui.font_points = (type < 3) ? (8 + type * 4) : gui.font->height;

    rg_settings_set_number(NS_GLOBAL, SETTING_FONTTYPE, type);

    RG_LOGI("Font set to: points=%d, scaling=%.2f\n",
        gui.font_points, (float)gui.font_points / gui.font->height);

    if (gui.initialized)
        rg_system_event(RG_EVENT_REDRAW, NULL);

    return true;
}

const rg_gui_t *rg_gui_get_info(void)
{
    return &gui;
}

rg_rect_t rg_gui_draw_text(int x_pos, int y_pos, int width, const char *text,
                           rg_color_t color_fg, rg_color_t color_bg, uint32_t flags)
{
    if (x_pos < 0) x_pos += gui.screen_width;
    if (y_pos < 0) y_pos += gui.screen_height;
    if (!text || *text == 0) text = " ";

    if (width == 0)
    {
        // Find the longest line to determine our box width
        int line_width = 0;
        const char *ptr = text;
        while (*ptr)
        {
            int chr = *ptr++;
            line_width += get_glyph(0, gui.font, gui.font_points, chr);

            if (chr == '\n' || *ptr == 0)
            {
                width = RG_MAX(line_width, width);
                line_width = 0;
            }
        }
    }

    int draw_width = RG_MIN(width, gui.screen_width - x_pos);
    int font_height = gui.font_points;
    int y_offset = 0;
    const char *ptr = text;

    while (*ptr)
    {
        int x_offset = 0;

        size_t p = draw_width * font_height;
        while (p--)
            gui.draw_buffer[p] = color_bg;

        if (flags & (RG_TEXT_ALIGN_LEFT|RG_TEXT_ALIGN_CENTER))
        {
            // Find the current line's text width
            const char *line = ptr;
            while (x_offset < draw_width && *line && *line != '\n')
            {
                int width = get_glyph(0, gui.font, gui.font_points, *line++);
                if (draw_width - x_offset < width) // Do not truncate glyphs
                    break;
                x_offset += width;
            }
            if (flags & RG_TEXT_ALIGN_CENTER)
                x_offset = (draw_width - x_offset) / 2;
            else if (flags & RG_TEXT_ALIGN_LEFT)
                x_offset = draw_width - x_offset;
        }

        while (x_offset < draw_width)
        {
            uint16_t bitmap[24] = {0};
            int width = get_glyph(bitmap, gui.font, gui.font_points, *ptr++);

            if (draw_width - x_offset < width) // Do not truncate glyphs
            {
                if (flags & RG_TEXT_MULTILINE)
                    ptr--;
                break;
            }

            if (!(flags & RG_TEXT_DUMMY_DRAW))
            {
                for (int y = 0; y < font_height; y++)
                {
                    uint16_t *output = &gui.draw_buffer[x_offset + (draw_width * y)];
                    for (int x = 0; x < width; x++)
                        output[x] = (bitmap[y] & (1 << x)) ? color_fg : color_bg;
                }
            }

            x_offset += width;

            if (*ptr == 0 || *ptr == '\n')
                break;
        }

        if (!(flags & RG_TEXT_DUMMY_DRAW))
            rg_gui_copy_buffer(x_pos, y_pos + y_offset, draw_width, font_height, 0, gui.draw_buffer);

        y_offset += font_height;

        if (!(flags & RG_TEXT_MULTILINE))
            break;
    }

    return (rg_rect_t){draw_width, y_offset};
}

void rg_gui_draw_rect(int x_pos, int y_pos, int width, int height, int border_size,
                      rg_color_t border_color, rg_color_t fill_color)
{
    if (width <= 0 || height <= 0)
        return;

    if (x_pos < 0)
        x_pos += gui.screen_width;
    if (y_pos < 0)
        y_pos += gui.screen_height;

    if (border_size > 0)
    {
        size_t p = border_size * RG_MAX(width, height);
        while (p--)
            gui.draw_buffer[p] = border_color;

        rg_gui_copy_buffer(x_pos, y_pos, width, border_size, 0, gui.draw_buffer); // Top
        rg_gui_copy_buffer(x_pos, y_pos + height - border_size, width, border_size, 0, gui.draw_buffer); // Bottom
        rg_gui_copy_buffer(x_pos, y_pos, border_size, height, 0, gui.draw_buffer); // Left
        rg_gui_copy_buffer(x_pos + width - border_size, y_pos, border_size, height, 0, gui.draw_buffer); // Right

        x_pos += border_size;
        y_pos += border_size;
        width -= border_size * 2;
        height -= border_size * 2;
    }

    if (height > 0 && fill_color != -1)
    {
        size_t p = width * RG_MIN(height, 16);
        while (p--)
            gui.draw_buffer[p] = fill_color;

        for (int y = 0; y < height; y += 16)
            rg_gui_copy_buffer(x_pos, y_pos + y, width, RG_MIN(height - y, 16), 0, gui.draw_buffer);
    }
}

void rg_gui_draw_image(int x_pos, int y_pos, int max_width, int max_height, const rg_image_t *img)
{
    if (img)
    {
        int width = max_width ? RG_MIN(max_width, img->width) : img->width;
        int height = max_height ? RG_MIN(max_height, img->height) : img->height;
        rg_gui_copy_buffer(x_pos, y_pos, width, height, img->width * 2, img->data);
    }
    else // We fill a rect to show something is missing instead of abort...
        rg_gui_draw_rect(x_pos, y_pos, max_width, max_height, 2, C_RED, C_BLACK);
}

void rg_gui_draw_battery(int x_pos, int y_pos)
{
    int width = 20, height = 10;
    int width_fill = width;
    rg_color_t color_fill = C_DARK_GRAY;
    rg_color_t color_border = C_SILVER;
    rg_color_t color_empty = C_BLACK;

    float percentage = rg_system_get_stats().batteryPercent;
    if (percentage >= 0.f && percentage <= 100.f)
    {
        width_fill = width / 100.f * percentage;
        if (percentage < 20.f)
            color_fill = C_RED;
        else if (percentage < 40.f)
            color_fill = C_ORANGE;
        else
            color_fill = C_FOREST_GREEN;
    }

    if (x_pos < 0) x_pos += gui.screen_width;
    if (y_pos < 0) y_pos += gui.screen_height;

    rg_gui_draw_rect(x_pos, y_pos, width + 2, height, 1, color_border, -1);
    rg_gui_draw_rect(x_pos + width + 2, y_pos + 2, 2, height - 4, 1, color_border, -1);
    rg_gui_draw_rect(x_pos + 1, y_pos + 1, width_fill, height - 2, 0, 0, color_fill);
    rg_gui_draw_rect(x_pos + 1 + width_fill, y_pos + 1, width - width_fill, 8, 0, 0, color_empty);
}

void rg_gui_draw_hourglass(void)
{
    rg_display_write((gui.screen_width / 2) - (image_hourglass.width / 2),
        (gui.screen_height / 2) - (image_hourglass.height / 2),
        image_hourglass.width,
        image_hourglass.height,
        image_hourglass.width * 2,
        (uint16_t*)image_hourglass.pixel_data);
}

void rg_gui_clear(rg_color_t color)
{
    if (gui.screen_buffer)
    {
        size_t pixels = gui.screen_width * gui.screen_height;
        while (pixels > 0)
            gui.screen_buffer[--pixels] = color;
    }
    else
        rg_display_clear(color);
}

static int get_dialog_items_count(const rg_gui_option_t *options)
{
    if (options == NULL)
        return 0;

    for (int i = 0; i < 16; i++)
    {
        if (options[i].flags == RG_DIALOG_FLAG_LAST) {
            return i;
        }
    }
    return 0;
}

void rg_gui_draw_dialog(const char *header, const rg_gui_option_t *options, int sel)
{
    const int options_count = get_dialog_items_count(options);
    const int sep_width = TEXT_RECT(": ", 0).width;
    const int font_height = gui.font_points;
    const int max_box_width = 0.82f * gui.screen_width;
    const int max_box_height = 0.82f * gui.screen_height;
    const int box_padding = 6;
    const int row_padding_y = 1;
    const int row_padding_x = 8;

    int box_width = box_padding * 2;
    int box_height = box_padding * 2 + (header ? font_height + 6 : 0);
    int inner_width = TEXT_RECT(header, 0).width;
    int max_inner_width = max_box_width - sep_width - (row_padding_x + box_padding) * 2;
    int col1_width = -1;
    int col2_width = -1;
    int row_height[options_count];

    for (int i = 0; i < options_count; i++)
    {
        rg_rect_t label = {0, font_height};
        rg_rect_t value = {0};

        if (options[i].label)
        {
            label = TEXT_RECT(options[i].label, max_inner_width);
            inner_width = RG_MAX(inner_width, label.width);
        }

        if (options[i].value)
        {
            value = TEXT_RECT(options[i].value, max_inner_width - label.width);
            col1_width = RG_MAX(col1_width, label.width);
            col2_width = RG_MAX(col2_width, value.width);
        }

        row_height[i] = RG_MAX(label.height, value.height) + row_padding_y * 2;
        box_height += row_height[i];
    }

    col1_width = RG_MIN(col1_width, max_box_width);
    col2_width = RG_MIN(col2_width, max_box_width);

    if (col2_width >= 0)
        inner_width = RG_MAX(inner_width, col1_width + col2_width + sep_width);

    inner_width = RG_MIN(inner_width, max_box_width);
    col2_width = inner_width - col1_width - sep_width;
    box_width += inner_width + row_padding_x * 2;
    box_height = RG_MIN(box_height, max_box_height);

    const int box_x = (gui.screen_width - box_width) / 2;
    const int box_y = (gui.screen_height - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;

    if (header)
    {
        int width = inner_width + row_padding_x * 2;
        rg_gui_draw_text(x, y, width, header, gui.theme.box_header, gui.theme.box_background, RG_TEXT_ALIGN_CENTER);
        rg_gui_draw_rect(x, y + font_height, width, 6, 0, 0, gui.theme.box_background);
        y += font_height + 6;
    }

    int top_i = 0;

    if (sel >= 0 && sel < options_count)
    {
        int yy = y;

        for (int i = 0; i < options_count; i++)
        {
            yy += row_height[i];

            if (yy >= box_y + box_height)
            {
                if (sel < i)
                    break;
                yy = y;
                top_i = i;
            }
        }
    }

    int i = top_i;
    for (; i < options_count; i++)
    {
        uint16_t color = (options[i].flags == RG_DIALOG_FLAG_NORMAL) ? gui.theme.item_standard : gui.theme.item_disabled;
        uint16_t fg = (i == sel) ? gui.theme.box_background : color;
        uint16_t bg = (i == sel) ? color : gui.theme.box_background;
        int xx = x + row_padding_x;
        int yy = y + row_padding_y;
        int height = 8;

        if (y + row_height[i] >= box_y + box_height)
        {
            break;
        }

        if (options[i].value)
        {
            rg_gui_draw_text(xx, yy, col1_width, options[i].label, fg, bg, 0);
            rg_gui_draw_text(xx + col1_width, yy, sep_width, ": ", fg, bg, 0);
            height = rg_gui_draw_text(xx + col1_width + sep_width, yy, col2_width, options[i].value, fg, bg, RG_TEXT_MULTILINE).height;
            rg_gui_draw_rect(xx, yy + font_height, inner_width - col2_width, height - font_height, 0, 0, bg);
        }
        else
        {
            height = rg_gui_draw_text(xx, yy, inner_width, options[i].label, fg, bg, RG_TEXT_MULTILINE).height;
        }

        rg_gui_draw_rect(x, yy, row_padding_x, height, 0, 0, bg);
        rg_gui_draw_rect(xx + inner_width, yy, row_padding_x, height, 0, 0, bg);
        rg_gui_draw_rect(x, y, inner_width + row_padding_x * 2, row_padding_y, 0, 0, bg);
        rg_gui_draw_rect(x, yy + height, inner_width + row_padding_x * 2, row_padding_y, 0, 0, bg);

        y += height + row_padding_y * 2;
    }

    if (y < (box_y + box_height))
    {
        rg_gui_draw_rect(box_x, y, box_width, (box_y + box_height) - y, 0, 0, gui.theme.box_background);
    }

    rg_gui_draw_rect(box_x, box_y, box_width, box_height, box_padding, gui.theme.box_background, -1);
    rg_gui_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, gui.theme.box_border, -1);

    // Basic scroll indicators are overlayed at the end...
    if (top_i > 0)
    {
        int x = box_x + inner_width + box_padding;
        int y = box_y + box_padding - 1;
        rg_gui_draw_rect(x, y, 3, 3, 0, 0, gui.theme.scrollbar);
        rg_gui_draw_rect(x + 6, y, 3, 3, 0, 0, gui.theme.scrollbar);
        rg_gui_draw_rect(x + 12, y, 3, 3, 0, 0, gui.theme.scrollbar);
    }

    if (i < options_count)
    {
        int x = box_x + inner_width + box_padding;
        int y = box_y + box_height - box_padding - 1;
        rg_gui_draw_rect(x, y, 3, 3, 0, 0, gui.theme.scrollbar);
        rg_gui_draw_rect(x + 6, y, 3, 3, 0, 0, gui.theme.scrollbar);
        rg_gui_draw_rect(x + 12, y, 3, 3, 0, 0, gui.theme.scrollbar);
    }

    rg_gui_flush();
}

int rg_gui_dialog(const char *header, const rg_gui_option_t *options_const, int selected)
{
    int options_count = get_dialog_items_count(options_const);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;

    // We create a copy of options because the callbacks might modify it (ie option->value)
    rg_gui_option_t options[options_count + 1];

    for (int i = 0; i <= options_count; i++)
    {
        char value_buffer[128] = {0xFF, 0};

        options[i] = options_const[i];

        if (options[i].update_cb)
        {
            options[i].value = value_buffer;
            options[i].update_cb(&options[i], RG_DIALOG_INIT);
            if (value_buffer[0] == 0xFF) // Not updated, reset ptr
                options[i].value = options_const[i].value;
        }

        if (options[i].value)
        {
            char *new_value = malloc(strlen(options[i].value) + 16);
            strcpy(new_value, options[i].value);
            options[i].value = new_value;
        }
    }

    // Constrain initial cursor and skip FLAG_SKIP items
    sel = RG_MIN(RG_MAX(0, sel), options_count - 1);

    rg_input_wait_for_key(RG_KEY_ALL, false);
    rg_gui_draw_dialog(header, options, sel);

    rg_gui_event_t event = RG_DIALOG_INIT;
    uint32_t joystick = 0, joystick_old;
    uint64_t joystick_last = 0;

    while (event != RG_DIALOG_CLOSE)
    {
        // TO DO: Add acceleration!
        joystick_old = ((rg_system_timer() - joystick_last) > 300000) ? 0 : joystick;
        joystick = rg_input_read_gamepad();
        event = RG_DIALOG_VOID;

        if (joystick ^ joystick_old)
        {
            bool selected = options[sel].flags != RG_DIALOG_FLAG_DISABLED && options[sel].flags != RG_DIALOG_FLAG_SKIP;
            rg_gui_callback_t callback = selected ? options[sel].update_cb : NULL;

            if (joystick & RG_KEY_UP) {
                if (--sel < 0) sel = options_count - 1;
            }
            else if (joystick & RG_KEY_DOWN) {
                if (++sel > options_count - 1) sel = 0;
            }
            else if (joystick & (RG_KEY_B|RG_KEY_OPTION|RG_KEY_MENU)) {
                event = RG_DIALOG_DISMISS;
            }
            else if (joystick & RG_KEY_LEFT && callback) {
                event = callback(&options[sel], RG_DIALOG_PREV);
                sel_old = -1;
            }
            else if (joystick & RG_KEY_RIGHT && callback) {
                event = callback(&options[sel], RG_DIALOG_NEXT);
                sel_old = -1;
            }
            else if (joystick & RG_KEY_START && callback) {
                event = callback(&options[sel], RG_DIALOG_ALT);
                sel_old = -1;
            }
            else if (joystick & RG_KEY_A && callback) {
                event = callback(&options[sel], RG_DIALOG_ENTER);
                sel_old = -1;
            }
            else if (joystick & RG_KEY_A && selected) {
                event = RG_DIALOG_CLOSE;
            }

            if (event == RG_DIALOG_DISMISS) {
                event = RG_DIALOG_CLOSE;
                sel = -1;
            }

            joystick_last = rg_system_timer();
        }

        if (sel_old != sel)
        {
            while (options[sel].flags == RG_DIALOG_FLAG_SKIP && sel_old != sel)
            {
                sel += (joystick == RG_KEY_DOWN) ? 1 : -1;

                if (sel < 0)
                    sel = options_count - 1;

                if (sel >= options_count)
                    sel = 0;
            }
            rg_gui_draw_dialog(header, options, sel);
            sel_old = sel;
        }

        usleep(20 * 1000UL);
    }

    rg_input_wait_for_key(joystick, false);

    rg_display_force_redraw();

    for (int i = 0; i < options_count; i++)
    {
        free(options[i].value);
    }

    return sel < 0 ? sel : options[sel].id;
}

bool rg_gui_confirm(const char *title, const char *message, bool yes_selected)
{
    const rg_gui_option_t options[] = {
        {0, (char *)message, NULL, -1, NULL},
        {0, "", NULL, -1, NULL},
        {1, "Yes", NULL, 1, NULL},
        {0, "No ", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    return rg_gui_dialog(title, message ? options : options + 1, yes_selected ? -2 : -1) == 1;
}

void rg_gui_alert(const char *title, const char *message)
{
    const rg_gui_option_t options[] = {
        {0, (char *)message, NULL, -1, NULL},
        {0, "", NULL, -1, NULL},
        {1, "OK", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    rg_gui_dialog(title, message ? options : options + 1, -1);
}

static rg_gui_event_t volume_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int level = rg_audio_get_volume();
    int prev_level = level;

    if (event == RG_DIALOG_PREV) level -= 5;
    if (event == RG_DIALOG_NEXT) level += 5;

    level -= (level % 5);

    if (level != prev_level)
        rg_audio_set_volume(level);

    sprintf(option->value, "%d%%", rg_audio_get_volume());

    return RG_DIALOG_VOID;
}

static rg_gui_event_t brightness_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int level = rg_display_get_backlight();
    int prev_level = level;

    if (event == RG_DIALOG_PREV) level -= 10;
    if (event == RG_DIALOG_NEXT) level += 10;

    level -= (level % 10);

    if (level != prev_level)
        rg_display_set_backlight(RG_MAX(level, 1));

    sprintf(option->value, "%d%%", rg_display_get_backlight());

    return RG_DIALOG_VOID;
}

static rg_gui_event_t audio_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    size_t count = 0;
    const rg_audio_sink_t *sinks = rg_audio_get_sinks(&count);
    const rg_audio_sink_t *ssink = rg_audio_get_sink();
    int max = count - 1;
    int sink = 0;

    for (int i = 0; i < count; ++i)
        if (sinks[i].type == ssink->type)
            sink = i;

    int prev_sink = sink;

    if (event == RG_DIALOG_PREV && --sink < 0) sink = max;
    if (event == RG_DIALOG_NEXT && ++sink > max) sink = 0;

    if (sink != prev_sink)
        rg_audio_set_sink(sinks[sink].type);

    strcpy(option->value, sinks[sink].name);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t filter_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = RG_DISPLAY_FILTER_COUNT - 1;
    int mode = rg_display_get_filter();
    int prev_mode = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode = max;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0;

    if (mode != prev_mode)
        rg_display_set_filter(mode);

    if (mode == RG_DISPLAY_FILTER_OFF)   strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_FILTER_HORIZ) strcpy(option->value, "Horiz");
    if (mode == RG_DISPLAY_FILTER_VERT)  strcpy(option->value, "Vert ");
    if (mode == RG_DISPLAY_FILTER_BOTH)  strcpy(option->value, "Both ");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t scaling_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = RG_DISPLAY_SCALING_COUNT - 1;
    int mode = rg_display_get_scaling();
    int prev_mode = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev_mode)
        rg_display_set_scaling(mode);

    if (mode == RG_DISPLAY_SCALING_OFF)  strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_SCALING_FIT)  strcpy(option->value, "Fit ");
    if (mode == RG_DISPLAY_SCALING_FILL) strcpy(option->value, "Full ");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t update_mode_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = RG_DISPLAY_UPDATE_COUNT - 1;
    int mode = rg_display_get_update_mode();
    int prev_mode = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev_mode)
        rg_display_set_update_mode(mode);

    if (mode == RG_DISPLAY_UPDATE_PARTIAL)   strcpy(option->value, "Partial");
    if (mode == RG_DISPLAY_UPDATE_FULL)      strcpy(option->value, "Full   ");
    // if (mode == RG_DISPLAY_UPDATE_INTERLACE) strcpy(option->value, "Interlace");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t speedup_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    rg_app_t *app = rg_system_get_app();

    if (event == RG_DIALOG_PREV && (app->speed -= 0.5f) < 0.5f) app->speed = 2.5f;
    if (event == RG_DIALOG_NEXT && (app->speed += 0.5f) > 2.5f) app->speed = 0.5f;

    sprintf(option->value, "%.1fx", app->speed);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t disk_activity_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        rg_storage_set_activity_led(!rg_storage_get_activity_led());
    }
    strcpy(option->value, rg_storage_get_activity_led() ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t font_type_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV && !rg_gui_set_font_type((int)gui.font_type - 1)) {
        rg_gui_set_font_type(0);
    }
    if (event == RG_DIALOG_NEXT && !rg_gui_set_font_type((int)gui.font_type + 1)) {
        rg_gui_set_font_type(0);
    }
    sprintf(option->value, "%s %d", gui.font->name, gui.font_points);
    return RG_DIALOG_VOID;
}

static void draw_game_status_bars(void)
{
    int height = RG_MAX(gui.font_points + 4, 16);
    int padding = (height - gui.font_points) / 2;
    int max_len = RG_MIN(gui.screen_width / RG_MAX(gui.font->width, 7), 99);
    char header[100] = {0};
    char footer[100] = {0};

    const rg_stats_t stats = rg_system_get_stats();
    const rg_app_t *app = rg_system_get_app();

    snprintf(header, 100, "SPEED: %.0f%% (%.0f/%.0f) / BUSY: %.0f%%",
        round(stats.totalFPS / app->refreshRate * 100.f),
        round(stats.totalFPS - stats.skippedFPS),
        round(stats.totalFPS),
        round(stats.busyPercent));

    if (app->romPath && strlen(app->romPath) > max_len)
        snprintf(footer, 100, "...%s", app->romPath + (strlen(app->romPath) - (max_len - 3)));
    else if (app->romPath)
        snprintf(footer, 100, "%s", app->romPath);

    rg_input_wait_for_key(RG_KEY_ALL, false);

    rg_gui_draw_rect(0, 0, gui.screen_width, height, 0, 0, C_BLACK);
    rg_gui_draw_rect(0, -height, gui.screen_width, height, 0, 0, C_BLACK);
    rg_gui_draw_text(0, padding, gui.screen_width, header, C_LIGHT_GRAY, C_BLACK, 0);
    rg_gui_draw_text(0, -height + padding, gui.screen_width, footer, C_LIGHT_GRAY, C_BLACK, 0);
    rg_gui_draw_battery(-26, 3);
}

int rg_gui_options_menu(void)
{
    rg_gui_option_t options[24];
    rg_gui_option_t *opt = &options[0];
    rg_app_t *app = rg_system_get_app();

    *opt++ = (rg_gui_option_t){0, "Brightness", "50%",  1, &brightness_update_cb};
    *opt++ = (rg_gui_option_t){0, "Volume    ", "50%",  1, &volume_update_cb};
    *opt++ = (rg_gui_option_t){0, "Audio out ", "Speaker", 1, &audio_update_cb};

    // Global settings that aren't essential to show when inside a game
    if (app->isLauncher)
    {
        *opt++ = (rg_gui_option_t){0, "Disk LED   ", "...", 1, &disk_activity_cb};
        *opt++ = (rg_gui_option_t){0, "Font type  ", "...", 1, &font_type_cb};
    }
    // App settings that are shown only inside a game
    else
    {
        *opt++ = (rg_gui_option_t){0, "Scaling", "Full", 1, &scaling_update_cb};
        *opt++ = (rg_gui_option_t){0, "Filter", "None", 1, &filter_update_cb};
        *opt++ = (rg_gui_option_t){0, "Update", "Partial", 1, &update_mode_update_cb};
        *opt++ = (rg_gui_option_t){0, "Speed", "1x", 1, &speedup_update_cb};
    }

    int extra_options = get_dialog_items_count(app->options);
    if (extra_options)
    {
        // *opt++ = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
        for (int i = 0; i < extra_options; i++)
            *opt++ = app->options[i];
    }

    *opt++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

    rg_audio_set_mute(true);
    draw_game_status_bars();

    int sel = rg_gui_dialog("Options", options, 0);

    rg_storage_commit();
    rg_audio_set_mute(false);

    return sel;
}

int rg_gui_about_menu(const rg_gui_option_t *extra_options)
{
    char build_ver[32], build_date[32], build_user[32];

    const rg_gui_option_t options[] = {
        {0, "Ver.", build_ver, 1, NULL},
        {0, "Date", build_date, 1, NULL},
        {0, "By", build_user, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1000, "Reboot to firmware", NULL, 1, NULL},
        {2000, "Reset settings", NULL, 1, NULL},
        {3000, "Clear cache", NULL, 1, NULL},
        {4000, "Debug", NULL, 1, NULL},
        {0000, "Close", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    const rg_app_t *app = rg_system_get_app();

    sprintf(build_ver, "%.30s", app->version);
    sprintf(build_date, "%s %.5s", app->buildDate, app->buildTime);
    sprintf(build_user, "%.30s", app->buildUser);

    char *rel_hash = strstr(build_ver, "-0-g");
    if (rel_hash)
    {
        rel_hash[0] = ' ';
        rel_hash[1] = ' ';
        rel_hash[2] = ' ';
        rel_hash[3] = '(';
        strcat(build_ver, ")");
    }

    int sel = rg_gui_dialog("Retro-Go", options, -1);

    switch (sel)
    {
        case 1000:
            rg_system_set_boot_app(RG_APP_FACTORY);
            rg_system_restart();
            break;
        case 2000:
            if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                unlink(RG_BASE_PATH_CONFIG "/favorite.txt");
                unlink(RG_BASE_PATH_CONFIG "/recent.txt");
                rg_settings_reset();
                rg_system_restart();
            }
            break;
        case 3000:
            unlink(RG_BASE_PATH_CACHE "/crc32.bin");
            rg_system_restart();
            break;
        case 4000:
            rg_gui_debug_menu(NULL);
            break;
    }

    return sel;
}

int rg_gui_debug_menu(const rg_gui_option_t *extra_options)
{
    char screen_res[20], source_res[20], scaled_res[20];
    char stack_hwm[20], heap_free[20], block_free[20];
    char system_rtc[20], uptime[20];

    const rg_gui_option_t options[] = {
        {0, "Screen Res", screen_res, 1, NULL},
        {0, "Source Res", source_res, 1, NULL},
        {0, "Scaled Res", scaled_res, 1, NULL},
        {0, "Stack HWM ", stack_hwm, 1, NULL},
        {0, "Heap free ", heap_free, 1, NULL},
        {0, "Block free", block_free, 1, NULL},
        {0, "System RTC", system_rtc, 1, NULL},
        {0, "Uptime    ", uptime, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1000, "Save screenshot", NULL, 1, NULL},
        {2000, "Save trace", NULL, 1, NULL},
        {3000, "Cheats", NULL, 1, NULL},
        {4000, "Crash", NULL, 1, NULL},
        {5000, "Random time", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    const rg_stats_t stats = rg_system_get_stats();
    const rg_display_t *display = rg_display_get_status();
    time_t now = time(NULL);

    strftime(system_rtc, 20, "%F %T", gmtime(&now));
    sprintf(screen_res, "%dx%d", display->screen.width, display->screen.height);
    sprintf(source_res, "%dx%d", display->source.width, display->source.height);
    sprintf(scaled_res, "%dx%d", display->viewport.width, display->viewport.height);
    sprintf(stack_hwm, "%d", stats.freeStackMain);
    sprintf(heap_free, "%d+%d", stats.freeMemoryInt, stats.freeMemoryExt);
    sprintf(block_free, "%d+%d", stats.freeBlockInt, stats.freeBlockExt);
    sprintf(uptime, "%ds", (int)(rg_system_timer() / 1000000));

    int sel = rg_gui_dialog("Debugging", options, 0);

    if (sel == 1000)
    {
        rg_emu_screenshot(RG_ROOT_PATH "/screenshot.png", 0, 0);
    }
    else if (sel == 2000)
    {
        rg_system_save_trace(RG_ROOT_PATH "/trace.txt", 0);
    }
    else if (sel == 4000)
    {
        RG_PANIC("Crash test!");
    }
    else if (sel == 5000)
    {
        struct timeval tv = {rand() % 1893474000, 0};
        settimeofday(&tv, NULL);
    }

    return sel;
}

int rg_gui_game_menu(void)
{
    const rg_gui_option_t choices[] = {
        {1000, "Save & Continue", NULL,  1, NULL},
        {2000, "Save & Quit", NULL, 1, NULL},
        {3000, "Reset game", NULL, 1, NULL},
        #ifdef ENABLE_NETPLAY
        {5000, "Netplay", NULL, 1, NULL},
        #endif
        {5500, "Options", NULL, 1, NULL},
        {6000, "About", NULL, 1, NULL},
        {7000, "Quit", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    rg_audio_set_mute(true);
    draw_game_status_bars();

    int sel = rg_gui_dialog("Retro-Go", choices, 0);

    if (sel == 3000)
    {
        const rg_gui_option_t choices[] = {
            {3001, "Reload save", NULL,  1, NULL},
            {3002, "Soft reset", NULL, 1, NULL},
            {3003, "Hard reset", NULL, 1, NULL},
            RG_DIALOG_CHOICE_LAST
        };
        sel = rg_gui_dialog("Reset Emulation", choices, 0);
    }

    switch (sel)
    {
        case 1000: rg_emu_save_state(0); break;
        case 2000: if (rg_emu_save_state(0)) exit(0); break;
        case 3001: rg_emu_load_state(0); break; // rg_system_restart();
        case 3002: rg_emu_reset(false); break;
        case 3003: rg_emu_reset(true); break;
    #ifdef ENABLE_NETPLAY
        case 5000: rg_netplay_quick_start(); break;
    #endif
        case 5500: rg_gui_options_menu(); break;
        case 6000: rg_gui_about_menu(NULL); break;
        case 7000: exit(0); break;
    }

    rg_audio_set_mute(false);

    return sel;
}

rg_image_t *rg_image_load_from_file(const char *filename, uint32_t flags)
{
    RG_ASSERT(filename, "bad param");

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        RG_LOGE("Unable to open image file '%s'!\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    size_t data_len = ftell(fp);
    uint8_t *data = malloc(data_len);
    if (!data)
    {
        RG_LOGE("Memory allocation failed (%d bytes)!\n", data_len);
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    fread(data, data_len, 1, fp);
    fclose(fp);

    rg_image_t *img = rg_image_load_from_memory(data, data_len, flags);
    free(data);

    return img;
}

rg_image_t *rg_image_load_from_memory(const uint8_t *data, size_t data_len, uint32_t flags)
{
    RG_ASSERT(data && data_len >= 16, "bad param");

    if (memcmp(data, "\x89PNG", 4) == 0)
    {
        unsigned error, width, height;
        uint8_t *image = NULL;

        error = lodepng_decode24(&image, &width, &height, data, data_len);
        if (error)
        {
            RG_LOGE("PNG decoding failed: %d\n", error);
            return NULL;
        }

        rg_image_t *img = rg_image_alloc(width, height);
        if (img)
        {
            size_t pixel_count = width * height;
            const uint8_t *src = image;
            uint16_t *dest = img->data;

            // RGB888 or RGBA8888 to RGB565
            for (size_t i = 0; i < pixel_count; ++i)
            {
                // TO DO: Properly scale values instead of discarding extra bits
                *dest++ = (((src[0] >> 3) & 0x1F) << 11)
                        | (((src[1] >> 2) & 0x3F) << 5)
                        | (((src[2] >> 3) & 0x1F));
                src += 3;
            }
        }
        free(image);
        return img;
    }
    else // RAW565 (uint16 width, uint16 height, uint16 data[])
    {
        int width = ((uint16_t *)data)[0];
        int height = ((uint16_t *)data)[1];
        int size_diff = (width * height * 2 + 4) - data_len;

        if (size_diff >= 0 && size_diff <= 100)
        {
            rg_image_t *img = rg_image_alloc(width, height);
            if (img)
            {
                // Image is already RGB565 little endian, just copy it
                memcpy(img->data, data + 4, data_len - 4);
            }
            // else Maybe we could just return (rg_image_t *)buffer; ?
            return img;
        }
    }

    RG_LOGE("Image format not recognized!\n");
    return NULL;
}

bool rg_image_save_to_file(const char *filename, const rg_image_t *img, uint32_t flags)
{
    RG_ASSERT(filename && img, "bad param");

    size_t pixel_count = img->width * img->height;
    uint8_t *image = malloc(pixel_count * 3);
    uint8_t *dest = image;
    unsigned error;

    if (!image)
    {
        RG_LOGE("Memory alloc failed!\n");
        return false;
    }

    // RGB565 to RGB888
    for (int i = 0; i < pixel_count; ++i)
    {
        dest[0] = ((img->data[i] >> 11) & 0x1F) << 3;
        dest[1] = ((img->data[i] >> 5) & 0x3F) << 2;
        dest[2] = ((img->data[i] & 0x1F) << 3);
        dest += 3;
    }

    error = lodepng_encode24_file(filename, image, img->width, img->height);
    free(image);

    if (error)
    {
        RG_LOGE("PNG encoding failed: %d\n", error);
        return false;
    }

    return true;
}

rg_image_t *rg_image_copy_resampled(const rg_image_t *img, int new_width, int new_height, int new_format)
{
    RG_ASSERT(img, "bad param");

    if (new_width <= 0 && new_height <= 0)
    {
        new_width = img->width;
        new_height = img->height;
    }
    else if (new_width <= 0)
    {
        new_width = img->width * ((float)new_height / img->height);
    }
    else if (new_height <= 0)
    {
        new_height = img->height * ((float)new_width / img->width);
    }

    rg_image_t *new_img = rg_image_alloc(new_width, new_height);
    if (!new_img)
    {
        RG_LOGW("Out of memory!\n");
    }
    else if (new_width == img->width && new_height == img->height)
    {
        memcpy(new_img, img, (2 + new_width * new_height) * 2);
    }
    else
    {
        float step_x = (float)img->width / new_width;
        float step_y = (float)img->height / new_height;
        uint16_t *dst = new_img->data;
        for (int y = 0; y < new_height; y++)
        {
            for (int x = 0; x < new_width; x++)
            {
                *(dst++) = img->data[((int)(y * step_y) * img->width) + (int)(x * step_x)];
            }
        }
    }
    return new_img;
}

rg_image_t *rg_image_alloc(size_t width, size_t height)
{
    rg_image_t *img = malloc(sizeof(rg_image_t) + width * height * 2);
    if (!img)
    {
        RG_LOGE("Image alloc failed (%dx%d)\n", width, height);
        return NULL;
    }
    img->width = width;
    img->height = height;
    return img;
}

void rg_image_free(rg_image_t *img)
{
    free(img);
}
