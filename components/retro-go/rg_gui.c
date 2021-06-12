#include <esp_heap_caps.h>
#include <esp_system.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "bitmaps/image_hourglass.h"
#include "fonts/fonts.h"
#include "rg_system.h"
#include "rg_gui.h"

static uint16_t *overlay_buffer = NULL;

static const dialog_theme_t default_theme = {
    .box_background = C_NAVY,
    .box_header = C_WHITE,
    .box_border = C_DIM_GRAY,
    .item_standard = C_WHITE,
    .item_disabled = C_GRAY, // C_DIM_GRAY
};

static dialog_theme_t theme;
static font_info_t font_info = {0, 8, 8, 8, &font_basic8x8};

// static const char *SETTING_FONTSIZE     = "FontSize";
static const char *SETTING_FONTTYPE     = "FontType";


void rg_gui_init(void)
{
    overlay_buffer = (uint16_t *)rg_alloc(RG_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    rg_gui_set_font_type(rg_settings_get_int32(SETTING_FONTTYPE, 0));
    rg_gui_set_theme(&default_theme);
}

bool rg_gui_set_theme(const dialog_theme_t *new_theme)
{
    if (!new_theme)
        return false;

    theme = *new_theme;
    return true;
}

static rg_glyph_t get_glyph(const rg_font_t *font, int points, int c)
{
    rg_glyph_t out = {
        .width  = font->width ? font->width : 8,
        .height = font->height,
        .bitmap = {0},
    };

    if (c == '\n') // Some glyphs are always zero width
    {
        out.width = 0;
    }
    else if (font->type == 0) // bitmap
    {
        if (c < font->chars)
        {
            for (int y = 0; y < font->height; y++) {
                out.bitmap[y] = font->data[(c * font->height) + y];
            }
        }
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
            out.width = ((width > xDelta) ? width : xDelta);
            // out.height = height;

            int ch = 0, mask = 0x80;

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (((x + (y * width)) % 8) == 0) {
                        mask = 0x80;
                        ch = *data++;
                    }
                    if ((ch & mask) != 0) {
                        out.bitmap[adjYOffset + y] |= (1 << (xOffset + x));
                    }
                    mask >>= 1;
                }
            }
        }
    }

    if (points && points != font->height)
    {
        float scale = (float)points / font->height;
        rg_glyph_t out2 = out;
        out.height = points;
        for (int y = 0; y < out.height; y++)
            out.bitmap[y] = out2.bitmap[(int)(y / scale)];
    }

    return out;
}

bool rg_gui_set_font_type(int type)
{
    if (type < 0)
        type += fonts_count;

    if (type < 0 || type > fonts_count - 1)
        return false;

    font_info.type = type;
    font_info.font = fonts[font_info.type];
    font_info.points = (font_info.type < 3) ? (8 + font_info.type * 4) : font_info.font->height;
    font_info.width  = RG_MAX(font_info.font->width, 4);
    font_info.height = font_info.points;

    rg_settings_set_int32(SETTING_FONTTYPE, font_info.type);

    RG_LOGI("Font set to: points=%d, size=%dx%d, scaling=%.2f\n",
        font_info.points, font_info.width, font_info.height,
        (float)font_info.points / font_info.font->height);

    return true;
}

font_info_t rg_gui_get_font_info(void)
{
    return font_info;
}

rg_rect_t rg_gui_calc_text_size(const char *text, int wrap_width)
{
    if (wrap_width > 0)
        return rg_gui_draw_text(RG_SCREEN_WIDTH - wrap_width, 0, 0, text, 0, 0, RG_TEXT_MULTILINE|RG_TEXT_DUMMY_DRAW);
    else
        return rg_gui_draw_text(0, 0, 0, text, 0, 0, RG_TEXT_DUMMY_DRAW);
}

rg_rect_t rg_gui_draw_text(int x_pos, int y_pos, int width, const char *text,
                           uint16_t color_fg, uint16_t color_bg, uint32_t flags)
{
    if (x_pos < 0) x_pos += RG_SCREEN_WIDTH;
    if (y_pos < 0) y_pos += RG_SCREEN_HEIGHT;
    if (!text || *text == 0) text = " ";

    // FIX ME: We should cache get_glyph().width to avoid calling it up to 3 times.

    if (width == 0)
    {
        // Find the longest line to determine our box width
        int line_width = 0;
        const char *ptr = text;
        while (*ptr)
        {
            int chr = *ptr++;
            line_width += get_glyph(font_info.font, font_info.points, chr).width;

            if (chr == '\n' || *ptr == 0)
            {
                width = RG_MAX(line_width, width);
                line_width = 0;
            }
        }
    }

    int draw_width = RG_MIN(width, RG_SCREEN_WIDTH - x_pos);
    int font_height = font_info.height;
    int y_offset = 0;
    const char *ptr = text;

    while (*ptr)
    {
        int x_offset = 0;

        for (int y = 0; y < font_height; y++)
            for (int x = 0; x < draw_width; x++)
                overlay_buffer[(draw_width * y) + x] = color_bg;

        if (flags & (RG_TEXT_ALIGN_LEFT|RG_TEXT_ALIGN_CENTER))
        {
            // Find the current line's text width
            const char *line = ptr;
            while (x_offset < draw_width && *line && *line != '\n')
            {
                int width = get_glyph(font_info.font, font_info.points, *line++).width;
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
            rg_glyph_t glyph = get_glyph(font_info.font, font_info.points, *ptr++);

            if (draw_width - x_offset < glyph.width) // Do not truncate glyphs
            {
                if (flags & RG_TEXT_MULTILINE)
                    ptr--;
                break;
            }

            for (int y = 0; y < font_height; y++)
            {
                uint16_t *output = &overlay_buffer[x_offset + (draw_width * y)];

                for (int x = 0; x < glyph.width; x++)
                {
                    output[x] = (glyph.bitmap[y] & (1 << x)) ? color_fg : color_bg;
                }
            }

            x_offset += glyph.width;

            if (*ptr == 0 || *ptr == '\n')
                break;
        }

        if (!(flags & RG_TEXT_DUMMY_DRAW))
            rg_display_write(x_pos, y_pos + y_offset, draw_width, font_height, 0, overlay_buffer);

        y_offset += font_height;

        if (!(flags & RG_TEXT_MULTILINE))
            break;
    }

    return (rg_rect_t){draw_width, y_offset};
}

void rg_gui_draw_rect(int x_pos, int y_pos, int width, int height, int border, uint16_t color)
{
    if (width <= 0 || height <= 0 || border <= 0)
        return;

    if (x_pos < 0) x_pos += RG_SCREEN_WIDTH;
    if (y_pos < 0) y_pos += RG_SCREEN_HEIGHT;

    for (int p = border * RG_MAX(width, height); p >= 0; --p)
        overlay_buffer[p] = color;

    rg_display_write(x_pos, y_pos, width, border, 0, overlay_buffer); // T
    rg_display_write(x_pos, y_pos + height - border, width, border, 0, overlay_buffer); // B
    rg_display_write(x_pos, y_pos, border, height, 0, overlay_buffer); // L
    rg_display_write(x_pos + width - border, y_pos, border, height, 0, overlay_buffer); // R
}

void rg_gui_draw_fill_rect(int x_pos, int y_pos, int width, int height, uint16_t color)
{
    if (width <= 0 || height <= 0)
        return;

    if (x_pos < 0) x_pos += RG_SCREEN_WIDTH;
    if (y_pos < 0) y_pos += RG_SCREEN_HEIGHT;

    for (int p = width * 16; p >= 0; --p)
        overlay_buffer[p] = color;

    for (int y_end = y_pos + height; y_pos < y_end; y_pos += 16)
    {
        int thickness = (y_end - y_pos >= 16) ? 16 : (y_end - y_pos);
        rg_display_write(x_pos, y_pos, width, thickness, 0, overlay_buffer);
    }
}

void rg_gui_draw_image(int x_pos, int y_pos, int width, int height, const rg_image_t *img)
{
    if (x_pos < 0) x_pos += RG_SCREEN_WIDTH;
    if (y_pos < 0) y_pos += RG_SCREEN_HEIGHT;

    if (img && x_pos < RG_SCREEN_WIDTH && y_pos < RG_SCREEN_HEIGHT)
    {
        width = RG_MIN(width > 0 ? width : img->width, RG_SCREEN_WIDTH);
        height = RG_MIN(height > 0 ? height : img->height, RG_SCREEN_HEIGHT);
        rg_display_write(x_pos, y_pos, width, height, img->width * 2, img->data);
    }
}

void rg_gui_draw_battery(int x_pos, int y_pos)
{
    uint16_t percentage = rg_input_read_battery().percentage;
    uint16_t color_fill = C_FOREST_GREEN;
    uint16_t color_border = C_SILVER;
    uint16_t color_empty = C_BLACK;
    uint16_t width_fill = 20.f / 100 * percentage;
    uint16_t width_empty = 20 - width_fill;

    if (percentage < 20)
        color_fill = C_RED;
    else if (percentage < 40)
        color_fill = C_ORANGE;

    if (x_pos < 0) x_pos += RG_SCREEN_WIDTH;
    if (y_pos < 0) y_pos += RG_SCREEN_HEIGHT;

    rg_gui_draw_rect(x_pos, y_pos, 22, 10, 1, color_border);
    rg_gui_draw_rect(x_pos + 22, y_pos + 2, 2, 6, 1, color_border);
    rg_gui_draw_fill_rect(x_pos + 1, y_pos + 1, width_fill, 8, color_fill);
    rg_gui_draw_fill_rect(x_pos + 1 + width_fill, y_pos + 1, width_empty, 8, color_empty);
}

void rg_gui_draw_hourglass(void)
{
    rg_display_write((RG_SCREEN_WIDTH / 2) - (image_hourglass.width / 2),
        (RG_SCREEN_HEIGHT / 2) - (image_hourglass.height / 2),
        image_hourglass.width,
        image_hourglass.height,
        image_hourglass.width * 2,
        (uint16_t*)image_hourglass.pixel_data);
}

static int get_dialog_items_count(const dialog_option_t *options)
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

void rg_gui_draw_dialog(const char *header, const dialog_option_t *options, int sel)
{
    const int options_count = get_dialog_items_count(options);
    const int sep_width = rg_gui_calc_text_size(": ", 0).width;
    const int font_height = font_info.height;
    const int max_box_width = 0.82f * RG_SCREEN_WIDTH;
    const int max_box_height = 0.82f * RG_SCREEN_HEIGHT;
    const int box_padding = 6;
    const int row_padding_y = 1;
    const int row_padding_x = 8;

    int box_width = box_padding * 2;
    int box_height = box_padding * 2 + (header ? font_height + 6 : 0);
    int inner_width = rg_gui_calc_text_size(header, 0).width;
    int col1_width = -1;
    int col2_width = -1;
    int row_height[options_count];

    for (int i = 0; i < options_count; i++)
    {
        rg_rect_t label = {0};
        rg_rect_t value = {0};

        if (options[i].label)
        {
            label = rg_gui_calc_text_size(options[i].label, max_box_width);
            inner_width = RG_MAX(inner_width, label.width);
        }

        if (options[i].value)
        {
            value = rg_gui_calc_text_size(options[i].value, max_box_width - label.width);
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

    const int box_x = (RG_SCREEN_WIDTH - box_width) / 2;
    const int box_y = (RG_SCREEN_HEIGHT - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;

    if (header)
    {
        int width = inner_width + row_padding_x * 2;
        rg_gui_draw_text(x, y, width, header, theme.box_header, theme.box_background, RG_TEXT_ALIGN_CENTER);
        rg_gui_draw_fill_rect(x, y + font_height, width, 6, theme.box_background);
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

    for (int i = top_i; i < options_count; i++)
    {
        uint16_t color = (options[i].flags == RG_DIALOG_FLAG_NORMAL) ? theme.item_standard : theme.item_disabled;
        uint16_t fg = (i == sel) ? theme.box_background : color;
        uint16_t bg = (i == sel) ? color : theme.box_background;
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
            rg_gui_draw_fill_rect(xx, yy + font_height, inner_width - col2_width, height - font_height, bg);
        }
        else
        {
            height = rg_gui_draw_text(xx, yy, inner_width, options[i].label, fg, bg, RG_TEXT_MULTILINE).height;
        }

        rg_gui_draw_fill_rect(x, yy, row_padding_x, height, bg);
        rg_gui_draw_fill_rect(xx + inner_width, yy, row_padding_x, height, bg);
        rg_gui_draw_fill_rect(x, y, inner_width + row_padding_x * 2, row_padding_y, bg);
        rg_gui_draw_fill_rect(x, yy + height, inner_width + row_padding_x * 2, row_padding_y, bg);

        y += height + row_padding_y * 2;
    }

    if (y < (box_y + box_height))
    {
        rg_gui_draw_fill_rect(box_x, y, box_width, (box_y + box_height) - y, theme.box_background);
    }

    rg_gui_draw_rect(box_x, box_y, box_width, box_height, box_padding, theme.box_background);
    rg_gui_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, theme.box_border);
}

int rg_gui_dialog(const char *header, const dialog_option_t *options_const, int selected)
{
    int options_count = get_dialog_items_count(options_const);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;
    int last_key = -1;

    // We create a copy of options because the callbacks might modify it (ie option->value)
    dialog_option_t options[options_count + 1];

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

    rg_audio_clear_buffer();
    rg_input_wait_for_key(GAMEPAD_KEY_ALL, false);
    rg_gui_draw_dialog(header, options, sel);

    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (last_key >= 0) {
            if (!(joystick & last_key)) {
                last_key = -1;
            }
        }
        else {
            dialog_return_t select = RG_DIALOG_IGNORE;

            if (joystick & GAMEPAD_KEY_UP) {
                last_key = GAMEPAD_KEY_UP;
                if (--sel < 0) sel = options_count - 1;
            }
            else if (joystick & GAMEPAD_KEY_DOWN) {
                last_key = GAMEPAD_KEY_DOWN;
                if (++sel > options_count - 1) sel = 0;
            }
            else if (joystick & GAMEPAD_KEY_B) {
                last_key = GAMEPAD_KEY_B;
                select = RG_DIALOG_CANCEL;
            }
            else if (joystick & GAMEPAD_KEY_VOLUME) {
                last_key = GAMEPAD_KEY_VOLUME;
                select = RG_DIALOG_CANCEL;
            }
            else if (joystick & GAMEPAD_KEY_MENU) {
                last_key = GAMEPAD_KEY_MENU;
                select = RG_DIALOG_CANCEL;
            }
            if (options[sel].flags != RG_DIALOG_FLAG_DISABLED) {
                if (joystick & GAMEPAD_KEY_LEFT) {
                    last_key = GAMEPAD_KEY_LEFT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_PREV);
                        sel_old = -1;
                    }
                }
                else if (joystick & GAMEPAD_KEY_RIGHT) {
                    last_key = GAMEPAD_KEY_RIGHT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_NEXT);
                        sel_old = -1;
                    }
                }
                else if (joystick & GAMEPAD_KEY_START) {
                    last_key = GAMEPAD_KEY_START;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_ALT);
                        sel_old = -1;
                    }
                }
                else if (joystick & GAMEPAD_KEY_A) {
                    last_key = GAMEPAD_KEY_A;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_ENTER);
                        sel_old = -1;
                    } else {
                        select = RG_DIALOG_SELECT;
                    }
                }
            }

            if (select == RG_DIALOG_CANCEL) {
                sel = -1;
                break;
            }
            if (select == RG_DIALOG_SELECT) {
                break;
            }
        }
        if (sel_old != sel)
        {
            while (options[sel].flags == RG_DIALOG_FLAG_SKIP && sel_old != sel)
            {
                sel += (last_key == GAMEPAD_KEY_DOWN) ? 1 : -1;

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

    rg_input_wait_for_key(last_key, false);

    if (!rg_emu_notify(RG_MSG_REDRAW, NULL))
        rg_display_reset_config();

    for (int i = 0; i < options_count; i++)
    {
        free(options[i].value);
    }

    return sel < 0 ? sel : options[sel].id;
}

bool rg_gui_confirm(const char *title, const char *message, bool yes_selected)
{
    const dialog_option_t options[] = {
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
    const dialog_option_t options[] = {
        {0, (char *)message, NULL, -1, NULL},
        {0, "", NULL, -1, NULL},
        {1, "OK", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    rg_gui_dialog(title, message ? options : options + 1, -1);
}

static dialog_return_t volume_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int8_t level = rg_audio_get_volume();
    int8_t min = RG_AUDIO_VOL_MIN;
    int8_t max = RG_AUDIO_VOL_MAX;

    if (event == RG_DIALOG_PREV && level > min) {
        rg_audio_set_volume(--level);
    }

    if (event == RG_DIALOG_NEXT && level < max) {
        rg_audio_set_volume(++level);
    }

    sprintf(option->value, "%d/%d", level, max);

    return RG_DIALOG_IGNORE;
}

static dialog_return_t brightness_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int8_t level = rg_display_get_backlight();
    int8_t max = RG_DISPLAY_BACKLIGHT_MAX;

    if (event == RG_DIALOG_PREV && level > 0) {
        rg_display_set_backlight(--level);
    }

    if (event == RG_DIALOG_NEXT && level < max) {
        rg_display_set_backlight(++level);
    }

    sprintf(option->value, "%d/%d", level, max);

    return RG_DIALOG_IGNORE;
}

static dialog_return_t audio_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int8_t sink = rg_audio_get_sink();

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        sink = (sink == RG_AUDIO_SINK_SPEAKER) ? RG_AUDIO_SINK_EXT_DAC : RG_AUDIO_SINK_SPEAKER;
        rg_audio_set_sink(sink);
    }

    strcpy(option->value, (sink == RG_AUDIO_SINK_EXT_DAC) ? "Ext DAC" : "Speaker");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t filter_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int8_t max = RG_DISPLAY_FILTER_COUNT - 1;
    int8_t mode = rg_display_get_filter();
    int8_t prev = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode = max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0; // max;

    if (mode != prev)
    {
        rg_display_set_filter(mode);
    }

    if (mode == RG_DISPLAY_FILTER_OFF)   strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_FILTER_HORIZ) strcpy(option->value, "Horiz");
    if (mode == RG_DISPLAY_FILTER_VERT)  strcpy(option->value, "Vert ");
    if (mode == RG_DISPLAY_FILTER_BOTH)  strcpy(option->value, "Both ");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t scaling_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int8_t max = RG_DISPLAY_SCALING_COUNT - 1;
    int8_t mode = rg_display_get_scaling();
    int8_t prev = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev) {
        rg_display_set_scaling(mode);
    }

    if (mode == RG_DISPLAY_SCALING_OFF)  strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_SCALING_FIT)  strcpy(option->value, "Fit ");
    if (mode == RG_DISPLAY_SCALING_FILL) strcpy(option->value, "Full ");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t speedup_update_cb(dialog_option_t *option, dialog_event_t event)
{
    rg_app_desc_t *app = rg_system_get_app();
    if (event == RG_DIALOG_PREV && --app->speedupEnabled < 0) app->speedupEnabled = 2;
    if (event == RG_DIALOG_NEXT && ++app->speedupEnabled > 2) app->speedupEnabled = 0;

    sprintf(option->value, "%dx", app->speedupEnabled + 1);

    return RG_DIALOG_IGNORE;
}

int rg_gui_settings_menu(const dialog_option_t *extra_options)
{
    dialog_option_t options[16 + get_dialog_items_count(extra_options)];
    dialog_option_t *opt = &options[0];

    *opt++ = (dialog_option_t){0, "Brightness", "50%",  1, &brightness_update_cb};
    *opt++ = (dialog_option_t){0, "Volume    ", "50%",  1, &volume_update_cb};
    *opt++ = (dialog_option_t){0, "Audio out ", "Speaker", 1, &audio_update_cb};

    if (!rg_system_get_app()->isLauncher)
    {
        *opt++ = (dialog_option_t){0, "Scaling", "Full", 1, &scaling_update_cb};
        *opt++ = (dialog_option_t){0, "Filtering", "None", 1, &filter_update_cb};
        *opt++ = (dialog_option_t){0, "Speed", "1x", 1, &speedup_update_cb};
    }

    while (extra_options && (*extra_options).flags != RG_DIALOG_FLAG_LAST) {
        *opt++ = *extra_options++;
    }

    *opt++ = (dialog_option_t)RG_DIALOG_CHOICE_LAST;

    int sel = rg_gui_dialog("Options", options, 0);

    rg_settings_save();

    return sel;
}

int rg_gui_about_menu(const dialog_option_t *extra_options)
{
    char build_ver[32], build_date[32], build_user[32];

    const dialog_option_t options[] = {
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

    const rg_app_desc_t *app = rg_system_get_app();

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
            rg_system_switch_app(RG_APP_FACTORY);
            break;
        case 2000:
            if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                rg_settings_reset();
                rg_system_restart();
            }
            break;
        case 3000:
            break;
        case 4000:
            rg_gui_debug_menu(NULL);
            break;
    }

    return sel;
}

int rg_gui_debug_menu(const dialog_option_t *extra_options)
{
    char screen_res[20], game_res[20], scaled_res[20];
    char stack_hwm[20], heap_free[20], block_free[20];
    char system_rtc[20], uptime[20];

    const dialog_option_t options[] = {
        {0, "Screen Res", screen_res, 1, NULL},
        {0, "Game Res  ", game_res, 1, NULL},
        {0, "Scaled Res", scaled_res, 1, NULL},
        {0, "Stack HWM ", stack_hwm, 1, NULL},
        {0, "Heap free ", heap_free, 1, NULL},
        {0, "Block free", block_free, 1, NULL},
        {0, "System RTC", system_rtc, 1, NULL},
        {0, "Uptime    ", uptime, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1000, "Save screenshot", NULL, 1, NULL},
        {2000, "Save app log", NULL, 1, NULL},
        {3000, "Cheats", NULL, 1, NULL},
        {4000, "Crash", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    runtime_stats_t stats = rg_system_get_stats();
    rg_display_t display = rg_display_get_status();
    rg_app_desc_t *app = rg_system_get_app();
    time_t now = time(NULL);

    strftime(system_rtc, 20, "%F %T", localtime(&now));
    sprintf(screen_res, "%dx%d", RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT);
    sprintf(game_res, "%dx%d", display.source.width, display.source.height);
    sprintf(scaled_res, "%dx%d", display.viewport.width, display.viewport.height);
    sprintf(stack_hwm, "%d", stats.freeStackMain);
    sprintf(heap_free, "%d+%d", stats.freeMemoryInt, stats.freeMemoryExt);
    sprintf(block_free, "%d+%d", stats.freeBlockInt, stats.freeBlockExt);
    sprintf(uptime, "%ds", (int)(get_elapsed_time() / 1000 / 1000));

    int sel = rg_gui_dialog("Debugging", options, 0);

    if (sel == 1000)
    {
        rg_emu_screenshot(RG_BASE_PATH "/screenshot.png", 0, 0);
    }
    else if (sel == 2000)
    {
        FILE *fp = fopen(RG_BASE_PATH "/log.txt", "wb");
        rg_system_write_log(&app->log, fp);
        fclose(fp);
    }
    else if (sel == 4000)
    {
        RG_PANIC("Crash test!");
    }

    return sel;
}

static void draw_game_status_bar(void)
{
    int width = RG_SCREEN_WIDTH, height = 16;
    int pad_text = (height - font_info.height) / 2;
    char header[41] = {0};
    char footer[41] = {0};

    const runtime_stats_t stats = rg_system_get_stats();
    const rg_app_desc_t *app = rg_system_get_app();

    snprintf(header, 40, "SPEED: %.0f%% (%.0f/%.0f) / BUSY: %.0f%%",
        round(stats.totalFPS / app->refreshRate * 100.f),
        round(stats.totalFPS - stats.skippedFPS),
        round(stats.totalFPS),
        round(stats.busyPercent));

    if (app->romPath)
        snprintf(footer, 40, "%s", app->romPath + strlen(RG_BASE_PATH_ROMS));

    rg_input_wait_for_key(GAMEPAD_KEY_ALL, false);

    rg_gui_draw_fill_rect(0, 0, width, height, C_BLACK);
    rg_gui_draw_fill_rect(0, RG_SCREEN_HEIGHT - height, width, height, C_BLACK);
    rg_gui_draw_text(0, pad_text, width, header, C_LIGHT_GRAY, C_BLACK, 0);
    rg_gui_draw_text(0, RG_SCREEN_HEIGHT - height + pad_text, width, footer, C_LIGHT_GRAY, C_BLACK, 0);
    rg_gui_draw_battery(width - 26, 3);
}

int rg_gui_game_settings_menu(const dialog_option_t *extra_options)
{
    draw_game_status_bar();
    return rg_gui_settings_menu(extra_options);
}

int rg_gui_game_menu(void)
{
    const dialog_option_t choices[] = {
        {1000, "Save & Continue", NULL,  1, NULL},
        {2000, "Save & Quit", NULL, 1, NULL},
        {3000, "Restart", NULL, 1, NULL},
        #ifdef ENABLE_NETPLAY
        {5000, "Netplay", NULL, 1, NULL},
        #endif
        {6000, "About", NULL, 1, NULL},
        {7000, "Quit", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    const dialog_option_t choices_restart[] = {
        {3001, "Reload save", NULL,  1, NULL},
        {3002, "Soft reset", NULL, 1, NULL},
        {3003, "Hard reset", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    draw_game_status_bar();

    int sel = rg_gui_dialog("Retro-Go", choices, 0);

    if (sel == 3000)
    {
        sel = rg_gui_dialog("Restart", choices_restart, 0);
    }

    switch (sel)
    {
        case 1000: rg_emu_save_state(0); break;
        case 2000: rg_emu_save_state(0); rg_system_switch_app(RG_APP_LAUNCHER); break;
        case 3001: rg_emu_load_state(0); break; // esp_restart();
        case 3002: rg_emu_reset(false); break;
        case 3003: rg_emu_reset(true); break;
    #ifdef ENABLE_NETPLAY
        case 5000: rg_netplay_quick_start(); break;
    #endif
        case 6000: rg_gui_about_menu(NULL); break;
        case 7000: rg_system_switch_app(RG_APP_LAUNCHER); break;
    }

    return sel;
}
