#include "rg_system.h"
#include "rg_gui.h"

#include <cJSON.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "bitmaps/image_hourglass.h"
#include "fonts/fonts.h"

static struct
{
    uint16_t *screen_buffer, *draw_buffer;
    int screen_width, screen_height;
    struct
    {
        const rg_font_t *font;
        int font_type;
        int font_points;
        rg_color_t box_background;
        rg_color_t box_header;
        rg_color_t box_border;
        rg_color_t item_standard;
        rg_color_t item_disabled;
        rg_color_t scrollbar;
    } style;
    char theme[32];
    bool initialized;
} gui;

static const char *SETTING_FONTTYPE  = "FontType";
static const char *SETTING_THEME     = "Theme";


void rg_gui_init(void)
{
    gui.screen_width = rg_display_get_info()->screen.width;
    gui.screen_height = rg_display_get_info()->screen.height;
    gui.draw_buffer = rg_alloc(RG_MAX(gui.screen_width, gui.screen_height) * 20 * 2, MEM_SLOW);
    rg_gui_set_font_type(rg_settings_get_number(NS_GLOBAL, SETTING_FONTTYPE, RG_FONT_VERA_12));
    rg_gui_set_theme(rg_settings_get_string(NS_GLOBAL, SETTING_THEME, NULL));
    gui.initialized = true;
}

static int get_theme_value(cJSON *theme, const char *key, int default_value)
{
    cJSON *obj = cJSON_GetObjectItem(theme, key);
    if (obj && cJSON_IsNumber(obj))
        return obj->valueint;
    // TODO: We must parse stringy hex values!
    return default_value;
}

bool rg_gui_set_theme(const char *theme_name)
{
    char theme_path[RG_PATH_MAX];
    cJSON *theme = NULL;

    if (theme_name && theme_name[0])
    {
        snprintf(theme_path, RG_PATH_MAX, "%s/%s/theme.json", RG_BASE_PATH_THEMES, theme_name);
        FILE *fp = fopen(theme_path, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            long length = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buffer = calloc(1, length + 1);
            if (fread(buffer, 1, length, fp))
                theme = cJSON_Parse(buffer);
            free(buffer);
            fclose(fp);
        }
        if (!theme)
            RG_LOGE("Failed to load theme '%s'!\n", theme_name);
    }

    gui.style.box_background = get_theme_value(theme, "box_background", C_NAVY);
    gui.style.box_header = get_theme_value(theme, "box_header", C_WHITE);
    gui.style.box_border = get_theme_value(theme, "box_border", C_DIM_GRAY);
    gui.style.item_standard = get_theme_value(theme, "item_standard", C_WHITE);
    gui.style.item_disabled = get_theme_value(theme, "item_disabled", C_GRAY);
    gui.style.scrollbar = get_theme_value(theme, "scrollbar", C_WHITE);

    RG_LOGI("Theme set to '%s'!\n", theme_name ?: "(none)");

    rg_settings_set_string(NS_GLOBAL, SETTING_THEME, theme_name);
    strcpy(gui.theme, theme_name ?: "");

    cJSON_Delete(theme);

    if (gui.initialized)
        rg_system_event(RG_EVENT_REDRAW, NULL);

    return true;
}

const char *rg_gui_get_theme(void)
{
    return strlen(gui.theme) ? gui.theme : NULL;
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

    const rg_font_t *font = fonts[type];

    gui.style.font = font;
    gui.style.font_type = type;
    gui.style.font_points = (type < 3) ? (8 + type * 4) : font->height;

    rg_settings_set_number(NS_GLOBAL, SETTING_FONTTYPE, type);

    RG_LOGI("Font set to: points=%d, scaling=%.2f\n",
        gui.style.font_points, (float)gui.style.font_points / font->height);

    if (gui.initialized)
        rg_system_event(RG_EVENT_REDRAW, NULL);

    return true;
}

rg_rect_t rg_gui_draw_text(int x_pos, int y_pos, int width, const char *text,
                           rg_color_t color_fg, rg_color_t color_bg, uint32_t flags)
{
    if (x_pos < 0) x_pos += gui.screen_width;
    if (y_pos < 0) y_pos += gui.screen_height;
    if (!text || *text == 0) text = " ";

    int padding = (flags & RG_TEXT_NO_PADDING) ? 0 : 1;
    int font_height = gui.style.font_points;
    int line_height = font_height + padding * 2;
    const rg_font_t *font = gui.style.font;

    if (width == 0)
    {
        // Find the longest line to determine our box width
        int line_width = padding * 2;
        for (const char *ptr = text; *ptr; )
        {
            int chr = *ptr++;
            line_width += get_glyph(0, font, font_height, chr);

            if (chr == '\n' || *ptr == 0)
            {
                width = RG_MAX(line_width, width);
                line_width = padding * 2;
            }
        }
    }

    // TO DO: this should honor wrapping, multiline, etc...
    if (flags & RG_TEXT_ALIGN_MIDDLE)
        y_pos = (gui.screen_height - line_height) / 2;
    else if (flags & RG_TEXT_ALIGN_BOTTOM)
        y_pos = (gui.screen_height - line_height);
    else if (flags & RG_TEXT_ALIGN_TOP)
        y_pos = 0;

    int draw_width = RG_MIN(width, gui.screen_width - x_pos);
    int y_offset = 0;

    for (const char *ptr = text; *ptr;)
    {
        int x_offset = padding;

        for (size_t p = draw_width * line_height; p--; )
            gui.draw_buffer[p] = color_bg;

        if (flags & (RG_TEXT_ALIGN_LEFT|RG_TEXT_ALIGN_CENTER))
        {
            // Find the current line's text width
            const char *line = ptr;
            while (x_offset < draw_width && *line && *line != '\n')
            {
                int width = get_glyph(0, font, font_height, *line++);
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
            int width = get_glyph(bitmap, font, font_height, *ptr++);

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
                    uint16_t *output = &gui.draw_buffer[(draw_width * (y + padding)) + x_offset];
                    for (int x = 0; x < width; x++)
                        output[x] = (bitmap[y] & (1 << x)) ? color_fg : color_bg;
                }
            }

            x_offset += width;

            if (*ptr == 0 || *ptr == '\n')
                break;
        }

        if (!(flags & RG_TEXT_DUMMY_DRAW))
            rg_gui_copy_buffer(x_pos, y_pos + y_offset, draw_width, line_height, 0, gui.draw_buffer);

        y_offset += line_height;

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

    if (height > 0 && fill_color != -1) // C_TRANSPARENT
    {
        size_t p = width * RG_MIN(height, 16);
        while (p--)
            gui.draw_buffer[p] = fill_color;

        for (int y = 0; y < height; y += 16)
            rg_gui_copy_buffer(x_pos, y_pos + y, width, RG_MIN(height - y, 16), 0, gui.draw_buffer);
    }
}

void rg_gui_draw_image(int x_pos, int y_pos, int width, int height, bool resample, const rg_image_t *img)
{
    if (img && resample && (width && height) && (width != img->width || height != img->height))
    {
        RG_LOGI("Resampling image (%dx%d => %dx%d)\n", img->width, img->height, width, height);
        rg_image_t *new_img = rg_image_copy_resampled(img, width, height, 0);
        rg_gui_copy_buffer(x_pos, y_pos, width, height, new_img->width * 2, new_img->data);
        rg_image_free(new_img);
    }
    else if (img)
    {
        int draw_width = width ? RG_MIN(width, img->width) : img->width;
        int draw_height = height ? RG_MIN(height, img->height) : img->height;
        rg_gui_copy_buffer(x_pos, y_pos, draw_width, draw_height, img->width * 2, img->data);
    }
    else // We fill a rect to show something is missing instead of abort...
    {
        rg_gui_draw_rect(x_pos, y_pos, width, height, 2, C_RED, C_BLACK);
    }
}

void rg_gui_draw_battery(int x_pos, int y_pos)
{
    if (x_pos < 0)
        x_pos += gui.screen_width;
    if (y_pos < 0)
        y_pos += gui.screen_height;

    int width = 16, height = 10;
    int width_fill = width;
    rg_color_t color_fill = C_DARK_GRAY;
    rg_color_t color_border = C_SILVER;
    rg_color_t color_empty = C_BLACK;
    float percentage;

    if (rg_input_read_battery(&percentage, NULL))
    {
        width_fill = width / 100.f * percentage;
        if (percentage < 20.f)
            color_fill = C_RED;
        else if (percentage < 40.f)
            color_fill = C_ORANGE;
        else
            color_fill = C_FOREST_GREEN;
    }

    rg_gui_draw_rect(x_pos, y_pos, width + 2, height, 1, color_border, -1);
    rg_gui_draw_rect(x_pos + width + 2, y_pos + 2, 2, height - 4, 1, color_border, -1);
    rg_gui_draw_rect(x_pos + 1, y_pos + 1, width_fill, height - 2, 0, 0, color_fill);
    rg_gui_draw_rect(x_pos + 1 + width_fill, y_pos + 1, width - width_fill, 8, 0, 0, color_empty);
}

void rg_gui_draw_radio(int x_pos, int y_pos)
{
    if (x_pos < 0)
        x_pos += gui.screen_width;
    if (y_pos < 0)
        y_pos += gui.screen_height;

    rg_network_t net = rg_network_get_info();
    rg_color_t color_fill = (net.state == RG_NETWORK_CONNECTED) ? C_GREEN : -1;
    rg_color_t color_border = (net.state == RG_NETWORK_CONNECTED) ? C_SILVER : C_DIM_GRAY;

    int seg_width = 4;
    y_pos += 6;
    rg_gui_draw_rect(x_pos, y_pos, seg_width, 4, 1, color_border, color_fill);
    x_pos += seg_width + 2;
    y_pos -= 3;
    rg_gui_draw_rect(x_pos, y_pos, seg_width, 7, 1, color_border, color_fill);
    x_pos += seg_width + 2;
    y_pos -= 3;
    rg_gui_draw_rect(x_pos, y_pos, seg_width, 10, 1, color_border, color_fill);
}

void rg_gui_draw_clock(int x_pos, int y_pos)
{
    if (x_pos < 0)
        x_pos += gui.screen_width;
    if (y_pos < 0)
        y_pos += gui.screen_height;

    char buffer[10];
    time_t time_sec = time(NULL);
    struct tm *time = localtime(&time_sec);

    // FIXME: Use a fixed small font here, that's why we're doing it in rg_gui...
    sprintf(buffer, "%02d:%02d", time->tm_hour, time->tm_min);
    rg_gui_draw_text(x_pos, y_pos, 0, buffer, C_WHITE, C_TRANSPARENT, 0);
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

void rg_gui_draw_status_bars(void)
{
    int max_len = RG_MIN(gui.screen_width / RG_MAX(gui.style.font->width, 7), 99);
    char header[100] = {0};
    char footer[100] = {0};

    const rg_app_t *app = rg_system_get_app();
    rg_stats_t stats = rg_system_get_counters();

    if (!app->initialized || app->isLauncher)
        return;

    snprintf(header, 100, "SPEED: %.0f%% (%.0f/%.0f) / BUSY: %.0f%%",
        round(stats.totalFPS / app->refreshRate * 100.f),
        round(stats.totalFPS - stats.skippedFPS),
        round(stats.totalFPS),
        round(stats.busyPercent));

    if (app->romPath && strlen(app->romPath) > max_len)
        snprintf(footer, 100, "...%s", app->romPath + (strlen(app->romPath) - (max_len - 3)));
    else if (app->romPath)
        snprintf(footer, 100, "%s", app->romPath);
    else
        snprintf(footer, 100, "Retro-Go %s", app->version);

    rg_gui_draw_text(0, 0, gui.screen_width, header, C_WHITE, C_BLACK, RG_TEXT_ALIGN_TOP);
    rg_gui_draw_text(0, 0, gui.screen_width, footer, C_WHITE, C_BLACK, RG_TEXT_ALIGN_BOTTOM);

    rg_gui_draw_battery(-22, 3);
    rg_gui_draw_radio(-54, 3);
}

static size_t get_dialog_items_count(const rg_gui_option_t *options)
{
    const rg_gui_option_t last = RG_DIALOG_CHOICE_LAST;
    size_t count = 0;

    if (!options)
        return 0;

    while (memcmp(options++, &last, sizeof(last)) != 0)
        count++;

    return count;
}

void rg_gui_draw_dialog(const char *title, const rg_gui_option_t *options, int sel)
{
    const size_t options_count = get_dialog_items_count(options);
    const int sep_width = TEXT_RECT(": ", 0).width;
    const int font_height = gui.style.font_points;
    const int max_box_width = 0.82f * gui.screen_width;
    const int max_box_height = 0.82f * gui.screen_height;
    const int box_padding = 6;
    const int row_padding_y = 0; // now handled by draw_text
    const int row_padding_x = 8;

    int box_width = box_padding * 2;
    int box_height = box_padding * 2 + (title ? font_height + 6 : 0);
    int inner_width = TEXT_RECT(title, 0).width;
    int max_inner_width = max_box_width - sep_width - (row_padding_x + box_padding) * 2;
    int col1_width = -1;
    int col2_width = -1;
    int row_height[options_count];

    for (size_t i = 0; i < options_count; i++)
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

    if (title)
    {
        int width = inner_width + row_padding_x * 2;
        rg_gui_draw_text(x, y, width, title, gui.style.box_header, gui.style.box_background, RG_TEXT_ALIGN_CENTER);
        rg_gui_draw_rect(x, y + font_height, width, 6, 0, 0, gui.style.box_background);
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
        uint16_t color = (options[i].flags == RG_DIALOG_FLAG_NORMAL) ? gui.style.item_standard : gui.style.item_disabled;
        uint16_t fg = (i == sel) ? gui.style.box_background : color;
        uint16_t bg = (i == sel) ? color : gui.style.box_background;
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
        rg_gui_draw_rect(box_x, y, box_width, (box_y + box_height) - y, 0, 0, gui.style.box_background);
    }

    rg_gui_draw_rect(box_x, box_y, box_width, box_height, box_padding, gui.style.box_background, -1);
    rg_gui_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, gui.style.box_border, -1);

    // Basic scroll indicators are overlayed at the end...
    if (top_i > 0)
    {
        int x = box_x + box_width - 10;
        int y = box_y + box_padding + 2;
        rg_gui_draw_rect(x + 0, y - 0, 6, 2, 0, 0, gui.style.scrollbar);
        rg_gui_draw_rect(x + 1, y - 2, 4, 2, 0, 0, gui.style.scrollbar);
        rg_gui_draw_rect(x + 2, y - 4, 2, 2, 0, 0, gui.style.scrollbar);
    }

    if (i < options_count)
    {
        int x = box_x + box_width - 10;
        int y = box_y + box_height - 6;
        rg_gui_draw_rect(x + 0, y - 4, 6, 2, 0, 0, gui.style.scrollbar);
        rg_gui_draw_rect(x + 1, y - 2, 4, 2, 0, 0, gui.style.scrollbar);
        rg_gui_draw_rect(x + 2, y - 0, 2, 2, 0, 0, gui.style.scrollbar);
    }

    rg_gui_flush();
}

int rg_gui_dialog(const char *title, const rg_gui_option_t *options_const, int selected_index)
{
    size_t options_count = get_dialog_items_count(options_const);
    int sel = selected_index < 0 ? (options_count + selected_index) : selected_index;
    int sel_old = -1;

    // Constrain initial cursor and skip FLAG_SKIP items
    sel = RG_MIN(RG_MAX(0, sel), options_count - 1);

    // We create a copy of options because the callbacks might modify it (ie option->value)
    rg_gui_option_t options[options_count + 1];
    char *text_buffer = calloc(2, 1024);
    char *text_buffer_ptr = text_buffer;

    memcpy(options, options_const, sizeof(options));

    for (size_t i = 0; i < options_count; i++)
    {
        rg_gui_option_t *option = &options[i];
        if (option->value && text_buffer) {
            option->value = strcpy(text_buffer_ptr, option->value);
            text_buffer_ptr += strlen(text_buffer_ptr) + 24;
        }
        if (option->label && text_buffer) {
            option->label = strcpy(text_buffer_ptr, option->label);
            text_buffer_ptr += strlen(text_buffer_ptr) + 24;
        }
        if (option->update_cb)
            option->update_cb(option, RG_DIALOG_INIT);
    }
    RG_LOGI("text_buffer usage = %d\n", (intptr_t)(text_buffer_ptr - text_buffer));

    rg_gui_draw_status_bars();
    rg_gui_draw_dialog(title, options, sel);
    rg_input_wait_for_key(RG_KEY_ALL, false);
    rg_task_delay(100);

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

        if (event == RG_DIALOG_CLOSE)
            break;

        if (sel_old == -1)
            rg_gui_draw_status_bars();

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
            if (options[sel].update_cb)
                event = options[sel].update_cb(&options[sel], RG_DIALOG_FOCUS);
            rg_gui_draw_dialog(title, options, sel);
            sel_old = sel;
        }

        rg_task_delay(20);
    }

    rg_input_wait_for_key(joystick, false);

    rg_display_force_redraw();

    free(text_buffer);

    if (sel == -1)
        return -1;

    return options[sel].arg;
}

bool rg_gui_confirm(const char *title, const char *message, bool default_yes)
{
    const rg_gui_option_t options[] = {
        {0, (char *)message, NULL, -1, NULL},
        {0, "", NULL, -1, NULL},
        {1, "Yes", NULL, 1, NULL},
        {0, "No ", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    return rg_gui_dialog(title, message ? options : options + 1, default_yes ? -2 : -1) == 1;
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

char *rg_gui_file_picker(const char *title, const char *path, bool (*validator)(const char *path))
{
    rg_scandir_t *files = rg_storage_scandir(path, validator, false);

    if (!files || !files[0].is_valid)
    {
        free(files);
        rg_gui_alert(title, "Folder is empty.");
        return NULL;
    }

    size_t count = 0;
    while (files[count].is_valid)
        count++;

    RG_LOGI("count=%d\n", count);

    // Unfortunately, at this time, any more than that will blow the stack.
    count = RG_MIN(count, 20);

    rg_gui_option_t *options = calloc(count + 1, sizeof(rg_gui_option_t));
    for (size_t i = 0; i < count; ++i)
    {
        // To do: check extension...
        options[i].arg = i;
        options[i].flags = 1;
        options[i].label = files[i].name;
    }
    options[count] = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

    int sel = rg_gui_dialog(title, options, 0);
    char *filename = NULL;

    if (sel >= 0 && sel < count)
        filename = strdup(files[sel].name);

    free(options);
    free(files);

    return filename;
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
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        if (event == RG_DIALOG_PREV && !rg_gui_set_font_type(gui.style.font_type - 1)) {
            rg_gui_set_font_type(0);
        }
        if (event == RG_DIALOG_NEXT && !rg_gui_set_font_type(gui.style.font_type + 1)) {
            rg_gui_set_font_type(0);
        }
    }
    sprintf(option->value, "%s %d", gui.style.font->name, gui.style.font_points);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t theme_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        char *path = rg_gui_file_picker("Theme", RG_BASE_PATH_THEMES, NULL);
        const char *theme = path ? rg_basename(path) : NULL;
        rg_gui_set_theme(theme);
        free(path);
    }

    sprintf(option->value, "%s", rg_gui_get_theme() ?: "Default");
    return RG_DIALOG_VOID;
}

int rg_gui_options_menu(void)
{
    rg_gui_option_t options[24];
    rg_gui_option_t *opt = &options[0];
    rg_app_t *app = rg_system_get_app();

    *opt++ = (rg_gui_option_t){0, "Brightness", "50%", 1, &brightness_update_cb};
    *opt++ = (rg_gui_option_t){0, "Volume    ", "50%", 1, &volume_update_cb};
    *opt++ = (rg_gui_option_t){0, "Audio out ", "Speaker", 1, &audio_update_cb};

    // Global settings that aren't essential to show when inside a game
    if (app->isLauncher)
    {
        *opt++ = (rg_gui_option_t){0, "Disk LED  ", "...", 1, &disk_activity_cb};
        *opt++ = (rg_gui_option_t){0, "Font type ", "...", 1, &font_type_cb};
        *opt++ = (rg_gui_option_t){0, "Theme     ", "...", 1, &theme_cb};
    }
    // App settings that are shown only inside a game
    else
    {
        *opt++ = (rg_gui_option_t){0, "Scaling", "Full", 1, &scaling_update_cb};
        *opt++ = (rg_gui_option_t){0, "Filter", "None", 1, &filter_update_cb};
        *opt++ = (rg_gui_option_t){0, "Update", "Partial", 1, &update_mode_update_cb};
        *opt++ = (rg_gui_option_t){0, "Speed", "1x", 1, &speedup_update_cb};
    }

    size_t extra_options = get_dialog_items_count(app->options);
    for (size_t i = 0; i < extra_options; i++)
        *opt++ = app->options[i];

    *opt++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

    rg_audio_set_mute(true);

    int sel = rg_gui_dialog("Options", options, 0);

    rg_settings_commit();
    rg_system_save_time();
    rg_audio_set_mute(false);

    return sel;
}

int rg_gui_about_menu(const rg_gui_option_t *extra_options)
{
    char build_ver[32], build_date[32], build_user[32], network_str[64];

    const rg_gui_option_t options[] = {
        {0, "Version", build_ver, 1, NULL},
        {0, "Date", build_date, 1, NULL},
        {0, "By", build_user, 1, NULL},
        {0, "Network", network_str, 1, NULL},
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

    rg_network_t net = rg_network_get_info();
    if (net.state == RG_NETWORK_CONNECTED) sprintf(network_str, "%s\n%s", net.name, net.ip_addr);
    else if (net.state == RG_NETWORK_CONNECTING) sprintf(network_str, "%s\n%s", net.name, "connecting...");
    else if (net.name[0]) sprintf(network_str, "%s\n%s", net.name, "disconnected");
    else strcpy(network_str, "disconnected");

    int sel = rg_gui_dialog("Retro-Go", options, -1);

    rg_settings_commit();
    rg_system_save_time();

    switch (sel)
    {
        case 1000:
            rg_system_switch_app(RG_APP_FACTORY, RG_APP_FACTORY, 0, 0);
            break;
        case 2000:
            if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                rg_settings_reset();
                rg_system_restart();
            }
            break;
        case 3000:
            rg_storage_delete(RG_BASE_PATH_CACHE);
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
    char local_time[32], timezone[32], uptime[20];

    const rg_gui_option_t options[] = {
        {0, "Screen Res", screen_res, 1, NULL},
        {0, "Source Res", source_res, 1, NULL},
        {0, "Scaled Res", scaled_res, 1, NULL},
        {0, "Stack HWM ", stack_hwm, 1, NULL},
        {0, "Heap free ", heap_free, 1, NULL},
        {0, "Block free", block_free, 1, NULL},
        {0, "Local time", local_time, 1, NULL},
        {0, "Timezone  ", timezone, 1, NULL},
        {0, "Uptime    ", uptime, 1, NULL},
        RG_DIALOG_SEPARATOR,
        {1000, "Save screenshot", NULL, 1, NULL},
        {2000, "Save trace", NULL, 1, NULL},
        {3000, "Cheats", NULL, 1, NULL},
        {4000, "Crash", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    const rg_display_t *display = rg_display_get_info();
    rg_stats_t stats = rg_system_get_counters();
    time_t now = time(NULL);

    strftime(local_time, 32, "%F %T", localtime(&now));
    snprintf(timezone, 32, "%s", getenv("TZ") ?: "N/A");
    snprintf(screen_res, 20, "%dx%d", display->screen.width, display->screen.height);
    snprintf(source_res, 20, "%dx%d", display->source.width, display->source.height);
    snprintf(scaled_res, 20, "%dx%d", display->viewport.width, display->viewport.height);
    snprintf(stack_hwm, 20, "%d", stats.freeStackMain);
    snprintf(heap_free, 20, "%d+%d", stats.freeMemoryInt, stats.freeMemoryExt);
    snprintf(block_free, 20, "%d+%d", stats.freeBlockInt, stats.freeBlockExt);
    snprintf(uptime, 20, "%ds", (int)(rg_system_timer() / 1000000));

    int sel = rg_gui_dialog("Debugging", options, 0);

    if (sel == 1000)
    {
        rg_emu_screenshot(RG_STORAGE_ROOT "/screenshot.png", 0, 0);
    }
    else if (sel == 2000)
    {
        rg_system_save_trace(RG_STORAGE_ROOT "/trace.txt", 0);
    }
    else if (sel == 4000)
    {
        RG_PANIC("Crash test!");
    }

    return sel;
}

static rg_emu_state_t *savestate;

static rg_gui_event_t slot_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    #define draw_status(x...) snprintf(buffer, sizeof(buffer), x); \
        rg_gui_draw_text(2, margin + 2, gui.screen_width - 4, buffer, C_WHITE, C_BLACK, RG_TEXT_ALIGN_CENTER);
    if (event == RG_DIALOG_FOCUS)
    {
        rg_emu_slot_t *slot = &savestate->slots[option->arg % 4];
        size_t margin = TEXT_RECT("ABC", 0).height;
        char buffer[100];
        if (slot->exists)
        {
            draw_status("Loading preview...");
            rg_image_t *img = rg_image_load_from_file(slot->preview, 0);
            rg_gui_draw_image(0, margin, gui.screen_width, gui.screen_height - margin * 2, true, img);
            rg_gui_draw_rect(0, margin, gui.screen_width, gui.screen_height - margin * 2, 2, C_BLUE, -1);
            draw_status(slot == savestate->lastused ? "Slot %d (last used)" : "Slot %d", slot->id);
            rg_image_free(img);
        }
        else
        {
            rg_gui_draw_rect(0, margin, gui.screen_width, gui.screen_height - margin * 2, 2, C_RED, C_BLACK);
            draw_status("Slot %d is empty", slot->id);
        }
    }
    else if (event == RG_DIALOG_ENTER)
    {
        return RG_DIALOG_CLOSE;
    }
    return RG_DIALOG_VOID;
    #undef draw_status
}

int rg_gui_savestate_menu(const char *title, const char *rom_path, bool quick_return)
{
    rg_gui_option_t choices[] = {
        {0, "Slot 0", NULL,  1, &slot_select_cb},
        {1, "Slot 1", NULL,  1, &slot_select_cb},
        {2, "Slot 2", NULL,  1, &slot_select_cb},
        {3, "Slot 3", NULL,  1, &slot_select_cb},
        RG_DIALOG_CHOICE_LAST
    };
    int sel;

    savestate = rg_emu_get_states(rom_path ?: rg_system_get_app()->romPath, 4);

    if (!rom_path)
        sel = rg_system_get_app()->saveSlot;
    else if (savestate->lastused)
        sel = savestate->lastused->id;
    else
        sel = 0;

    sel = rg_gui_dialog(title, choices, sel);

    free(savestate);

    return sel;
}

int rg_gui_game_menu(void)
{
    const rg_gui_option_t choices[] = {
        {1000, "Save & Continue", NULL,  1, NULL},
        {2000, "Save & Quit", NULL, 1, NULL},
        // {1000, "Save game", NULL, 1, NULL},
        {3001, "Load game", NULL, 1, NULL},
        {3000, "Reset", NULL, 1, NULL},
        #ifdef RG_ENABLE_NETPLAY
        {5000, "Netplay", NULL, 1, NULL},
        #endif
        #if !RG_GAMEPAD_HAS_OPTION_BTN
        {5500, "Options", NULL, 1, NULL},
        #endif
        {6000, "About", NULL, 1, NULL},
        {7000, "Quit", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    int slot, sel;

    rg_audio_set_mute(true);

    sel = rg_gui_dialog("Retro-Go", choices, 0);

    rg_settings_commit();
    rg_system_save_time();

    if (sel == 3000)
    {
        const rg_gui_option_t choices[] = {
            {3002, "Soft reset", NULL, 1, NULL},
            {3003, "Hard reset", NULL, 1, NULL},
            RG_DIALOG_CHOICE_LAST
        };
        sel = rg_gui_dialog("Reset Emulation?", choices, 0);
    }

    switch (sel)
    {
        case 1000: if ((slot = rg_gui_savestate_menu("Save", 0, 0)) >= 0) rg_emu_save_state(slot); break;
        case 2000: if ((slot = rg_gui_savestate_menu("Save", 0, 0)) >= 0) {rg_emu_save_state(slot); rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0);} break;
        case 3001: if ((slot = rg_gui_savestate_menu("Load", 0, 0)) >= 0) rg_emu_load_state(slot); break;
        case 3002: rg_emu_reset(false); break;
        case 3003: rg_emu_reset(true); break;
    #ifdef RG_ENABLE_NETPLAY
        case 5000: rg_netplay_quick_start(); break;
    #endif
        case 5500: rg_gui_options_menu(); break;
        case 6000: rg_gui_about_menu(NULL); break;
        case 7000: rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0); break;
    }

    rg_audio_set_mute(false);

    return sel;
}
