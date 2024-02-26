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
    size_t draw_buffer_size;
    int screen_width, screen_height;
    struct
    {
        const rg_font_t *font;
        int font_height;
        int font_width;
        rg_color_t box_background;
        rg_color_t box_header;
        rg_color_t box_border;
        rg_color_t item_standard;
        rg_color_t item_disabled;
        rg_color_t item_message;
        rg_color_t scrollbar;
    } style;
    char theme_name[32];
    cJSON *theme_obj;
    int font_index;
    bool show_clock;
    bool initialized;
} gui;

static const char *SETTING_FONTTYPE = "FontType";
static const char *SETTING_CLOCK = "Clock";
static const char *SETTING_THEME = "Theme";


static uint16_t *get_draw_buffer(int width, int height, rg_color_t fill_color)
{
    size_t pixels = width * height;
    if (pixels > gui.draw_buffer_size)
    {
        if (gui.draw_buffer != NULL)
        {
            RG_LOGW("Growing drawing buffer to %dx%d...", width, height);
            free(gui.draw_buffer);
        }
        gui.draw_buffer = rg_alloc(pixels * 2, MEM_SLOW);
        gui.draw_buffer_size = pixels;
    }

    if (!gui.draw_buffer)
        RG_PANIC("Failed to allocate draw buffer!");

    for (size_t i = 0; i < pixels; ++i)
        gui.draw_buffer[i] = fill_color;

    return gui.draw_buffer;
}

static int get_horizontal_position(int x_pos, int width)
{
    int type = (x_pos & 0xFF0000) | 0x8000;
    int offset = (x_pos & 0xFFFF) - 0x8000;
    if (type == RG_GUI_CENTER)
        return ((gui.screen_width - width) / 2) + offset;
    else if (type == RG_GUI_LEFT)
        return 0 + offset;
    else if (type == RG_GUI_RIGHT)
        return (gui.screen_width - width) - offset;
    else if (x_pos < 0)
        return x_pos + gui.screen_width;
    return x_pos;
}

static int get_vertical_position(int y_pos, int height)
{
    int type = (y_pos & 0xFF0000) | 0x8000;
    int offset = (y_pos & 0xFFFF) - 0x8000;
    if (type == RG_GUI_CENTER)
        return ((gui.screen_height - height) / 2) + offset;
    else if (type == RG_GUI_TOP)
        return 0 + offset;
    else if (type == RG_GUI_BOTTOM)
        return (gui.screen_height - height) - offset;
    else if (y_pos < 0)
        return y_pos + gui.screen_height;
    return y_pos;
}

void rg_gui_init(void)
{
    gui.screen_width = rg_display_get_info()->screen.width;
    gui.screen_height = rg_display_get_info()->screen.height;
    gui.draw_buffer = get_draw_buffer(gui.screen_width, 18, C_BLACK);
    rg_gui_set_font(rg_settings_get_number(NS_GLOBAL, SETTING_FONTTYPE, RG_FONT_VERA_12));
    rg_gui_set_theme(rg_settings_get_string(NS_GLOBAL, SETTING_THEME, NULL));
    gui.show_clock = rg_settings_get_number(NS_GLOBAL, SETTING_CLOCK, 0);
    gui.initialized = true;
}

bool rg_gui_set_theme(const char *theme_name)
{
    char pathbuf[RG_PATH_MAX];
    cJSON *new_theme = NULL;

    // Cleanup the current theme
    cJSON_Delete(gui.theme_obj);
    gui.theme_obj = NULL;

    if (theme_name && theme_name[0])
    {
        snprintf(pathbuf, RG_PATH_MAX, "%s/%s/theme.json", RG_BASE_PATH_THEMES, theme_name);
        FILE *fp = fopen(pathbuf, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            long length = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buffer = calloc(1, length + 1);
            if (fread(buffer, 1, length, fp))
                new_theme = cJSON_Parse(buffer);
            free(buffer);
            fclose(fp);
            if (!new_theme)
                RG_LOGE("Failed to load theme JSON from '%s'!\n", pathbuf);
        }
    }

    if (new_theme)
    {
        rg_settings_set_string(NS_GLOBAL, SETTING_THEME, theme_name);
        strcpy(gui.theme_name, theme_name);
        // FIXME: Keeping the theme around uses quite a lot of internal memory (about 3KB)...
        //        We should probably convert it to a regular array or hashmap.
        gui.theme_obj = new_theme;
        RG_LOGI("Theme set to '%s'!\n", theme_name);
    }
    else
    {
        rg_settings_set_string(NS_GLOBAL, SETTING_THEME, NULL);
        strcpy(gui.theme_name, "");
        gui.theme_obj = NULL;
        RG_LOGI("Using built-in theme!\n");
    }

    gui.style.box_background = rg_gui_get_theme_color("dialog", "background", C_NAVY);
    gui.style.box_header = rg_gui_get_theme_color("dialog", "header", C_WHITE);
    gui.style.box_border = rg_gui_get_theme_color("dialog", "border", C_DIM_GRAY);
    gui.style.item_standard = rg_gui_get_theme_color("dialog", "item_standard", C_WHITE);
    gui.style.item_disabled = rg_gui_get_theme_color("dialog", "item_disabled", C_GRAY);
    gui.style.item_message = rg_gui_get_theme_color("dialog", "item_message", C_SILVER);
    gui.style.scrollbar = rg_gui_get_theme_color("dialog", "scrollbar", C_WHITE);

    return true;
}

rg_color_t rg_gui_get_theme_color(const char *section, const char *key, rg_color_t default_value)
{
    cJSON *root = section ? cJSON_GetObjectItem(gui.theme_obj, section) : gui.theme_obj;
    cJSON *obj = cJSON_GetObjectItem(root, key);
    if (cJSON_IsNumber(obj))
        return obj->valueint;
    char *strval = cJSON_GetStringValue(obj);
    if (!strval || strlen(strval) < 4)
        return default_value;
    if (strcmp(strval, "transparent") == 0)
        return C_TRANSPARENT;
    int intval = (int)strtol(strval, NULL, 0);
    // It is better to specify colors as RGB565 to avoid data loss, but we also accept RGB888 for convenience
    if (strlen(strval) == 8 && strval[0] == '0' && strval[1] == 'x')
        return (((intval >> 19) & 0x1F) << 11) | (((intval >> 10) & 0x3F) << 5) | (((intval >> 3) & 0x1F));
    return intval;
}

rg_image_t *rg_gui_get_theme_image(const char *name)
{
    char pathbuf[RG_PATH_MAX];
    if (!name || !gui.theme_name[0])
        return NULL;
    snprintf(pathbuf, RG_PATH_MAX, "%s/%s/%s", RG_BASE_PATH_THEMES, gui.theme_name, name);
    return rg_image_load_from_file(pathbuf, 0);
}

const char *rg_gui_get_theme_name(void)
{
    return gui.theme_name[0] ? gui.theme_name : NULL;
}

bool rg_gui_set_font(int index)
{
    if (index < 0 || index > RG_FONT_MAX - 1)
        return false;

    const rg_font_t *font = fonts[index];

    gui.font_index = index;
    gui.style.font = font;
    gui.style.font_height = (index < 3) ? (8 + index * 4) : font->height;
    gui.style.font_width = font->width ?: 8;

    rg_settings_set_number(NS_GLOBAL, SETTING_FONTTYPE, index);

    RG_LOGI("Font set to: points=%d, scaling=%.2f\n",
        gui.style.font_height, (float)gui.style.font_height / font->height);

    return true;
}

void rg_gui_set_buffered(uint16_t *framebuffer)
{
    gui.screen_buffer = framebuffer;
}

void rg_gui_copy_buffer(int left, int top, int width, int height, int stride, const void *buffer)
{
    left = get_horizontal_position(left, width);
    top = get_vertical_position(top, height);

    if (gui.screen_buffer)
    {
        if (stride < width)
            stride = width * 2;

        width = RG_MIN(width, gui.screen_width - left);
        height = RG_MIN(height, gui.screen_height - top);

        for (int y = 0; y < height; ++y)
        {
            uint16_t *dst = gui.screen_buffer + (top + y) * gui.screen_width + left;
            const uint16_t *src = (void *)buffer + y * stride;
            for (int x = 0; x < width; ++x)
                if (src[x] != C_TRANSPARENT)
                    dst[x] = src[x];
        }
    }
    else
    {
        rg_display_write(left, top, width, height, stride, buffer, 0);
    }
}

static size_t get_glyph(uint16_t *output, const rg_font_t *font, int points, int c)
{
    size_t glyph_width = 8;

    // Some glyphs are always zero width
    if (c == '\r' || c == '\n' || c < 8 || c > 254)
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
                for (int y = 0; y < height; y++)
                {
                    output[adjYOffset + y] = 0;
                    for (int x = 0; x < width; x++)
                    {
                        if (((x + (y * width)) % 8) == 0)
                        {
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

rg_rect_t rg_gui_draw_text(int x_pos, int y_pos, int width, const char *text, // const rg_font_t *font,
                           rg_color_t color_fg, rg_color_t color_bg, uint32_t flags)
{
    int padding = (flags & RG_TEXT_NO_PADDING) ? 0 : 1;
    int font_height = (flags & RG_TEXT_BIGGER) ? gui.style.font_height * 2 : gui.style.font_height;
    int monospace = ((flags & RG_TEXT_MONOSPACE) || gui.style.font->type == 0) ? gui.style.font_width : 0;
    int line_height = font_height + padding * 2;
    int line_count = 0;
    const rg_font_t *font = gui.style.font;

    if (!text || *text == 0)
        text = " ";

    if (width == 0)
    {
        // Find the longest line to determine our box width
        int line_width = padding * 2;
        for (const char *ptr = text; *ptr;)
        {
            int chr = *ptr++;
            line_width += monospace ?: get_glyph(NULL, font, font_height, chr);

            if (chr == '\n' || *ptr == 0)
            {
                width = RG_MAX(line_width, width);
                line_width = padding * 2;
                line_count++;
            }
        }
    }

    x_pos = get_horizontal_position(x_pos, width);
    y_pos = get_vertical_position(y_pos, line_height);

    if (x_pos + width > gui.screen_width || y_pos + line_height > gui.screen_height)
    {
        RG_LOGD("Texbox (pos: %dx%d, size: %dx%d) will be truncated!", width, line_height, x_pos, y_pos);
        // return;
    }

    int draw_width = RG_MIN(width, gui.screen_width - x_pos);
    int y_offset = 0;

    for (const char *ptr = text; *ptr;)
    {
        int x_offset = padding;

        if (flags & (RG_TEXT_ALIGN_LEFT|RG_TEXT_ALIGN_CENTER))
        {
            // Find the current line's text width
            const char *line = ptr;
            while (x_offset < draw_width && *line && *line != '\n')
            {
                int chr = *line++;
                int width = monospace ?: get_glyph(NULL, font, font_height, chr);
                if (draw_width - x_offset < width) // Do not truncate glyphs
                    break;
                x_offset += width;
            }
            if (flags & RG_TEXT_ALIGN_CENTER)
                x_offset = (draw_width - x_offset) / 2;
            else if (flags & RG_TEXT_ALIGN_LEFT)
                x_offset = draw_width - x_offset;
        }

        uint16_t *draw_buffer = NULL;

        if (!(flags & RG_TEXT_DUMMY_DRAW))
            draw_buffer = get_draw_buffer(draw_width, line_height, color_bg);

        while (x_offset < draw_width)
        {
            uint16_t bitmap[32] = {0};
            int glyph_width = get_glyph(bitmap, font, font_height, *ptr++);
            int width = monospace ?: glyph_width;

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
                    uint16_t *output = &draw_buffer[(draw_width * (y + padding)) + x_offset];
                    for (int x = 0; x < width; x++)
                        output[x] = (bitmap[y] & (1 << x)) ? color_fg : color_bg;
                }
            }

            x_offset += width;

            if (*ptr == 0 || *ptr == '\n')
                break;
        }

        if (!(flags & RG_TEXT_DUMMY_DRAW))
            rg_gui_copy_buffer(x_pos, y_pos + y_offset, draw_width, line_height, 0, draw_buffer);

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

    x_pos = get_horizontal_position(x_pos, width);
    y_pos = get_vertical_position(y_pos, height);

    if (border_size > 0)
    {
        uint16_t *draw_buffer = get_draw_buffer(border_size, RG_MAX(width, height), border_color);

        rg_gui_copy_buffer(x_pos, y_pos, width, border_size, 0, draw_buffer);                        // Top
        rg_gui_copy_buffer(x_pos, y_pos + height - border_size, width, border_size, 0, draw_buffer); // Bottom
        rg_gui_copy_buffer(x_pos, y_pos, border_size, height, 0, draw_buffer);                       // Left
        rg_gui_copy_buffer(x_pos + width - border_size, y_pos, border_size, height, 0, draw_buffer); // Right

        x_pos += border_size;
        y_pos += border_size;
        width -= border_size * 2;
        height -= border_size * 2;
    }

    if (width > 0 && height > 0 && fill_color != C_NONE)
    {
        uint16_t *draw_buffer = get_draw_buffer(width, RG_MIN(height, 16), fill_color);
        for (int y = 0; y < height; y += 16)
            rg_gui_copy_buffer(x_pos, y_pos + y, width, RG_MIN(height - y, 16), 0, draw_buffer);
    }
}

void rg_gui_draw_image(int x_pos, int y_pos, int width, int height, bool resample, const rg_image_t *img)
{
    if (img && resample && (width && height) && (width != img->width || height != img->height))
    {
        RG_LOGD("Resampling image (%dx%d => %dx%d)\n", img->width, img->height, width, height);
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

void rg_gui_draw_icons(void)
{
    const rg_app_t *app = rg_system_get_app();
    rg_battery_t battery = rg_input_read_battery();
    rg_network_t network = rg_network_get_info();
    rg_rect_t txt = TEXT_RECT("00:00", 0);
    int right = 0;
    int top = app->isLauncher ? 0 : -1;

    if (battery.present)
    {
        right += 22;

        int width = 16;
        int height = 10;
        int width_fill = width / 100.f * battery.level;
        rg_color_t color_fill = (battery.level > 20 ? (battery.level > 40 ? C_FOREST_GREEN : C_ORANGE) : C_RED);
        rg_color_t color_border = C_SILVER;
        rg_color_t color_empty = C_BLACK;

        int x_pos = -right;
        int y_pos = RG_MAX(0, (txt.height - 10) / 2 + top);

        rg_gui_draw_rect(x_pos, y_pos, width + 2, height, 1, color_border, C_NONE);
        rg_gui_draw_rect(x_pos + width + 2, y_pos + 2, 2, height - 4, 1, color_border, C_NONE);
        rg_gui_draw_rect(x_pos + 1, y_pos + 1, width_fill, height - 2, 0, 0, color_fill);
        rg_gui_draw_rect(x_pos + 1 + width_fill, y_pos + 1, width - width_fill, 8, 0, 0, color_empty);
    }

    if (network.state > RG_NETWORK_DISCONNECTED)
    {
        right += 22;

        int x_pos = -right;
        int y_pos = RG_MAX(0, (txt.height - 10) / 2 + top);

        int width = 16;
        int height = 10;
        int seg_width = (width - 2 - 2) / 3;
        rg_color_t color_fill = (network.state == RG_NETWORK_CONNECTED) ? C_GREEN : C_NONE;
        rg_color_t color_border = (network.state == RG_NETWORK_CONNECTED) ? C_SILVER : C_DIM_GRAY;

        y_pos += height * 0.6;
        rg_gui_draw_rect(x_pos, y_pos, seg_width, height * 0.4, 1, color_border, color_fill);
        x_pos += seg_width + 2;
        y_pos -= height * 0.3;
        rg_gui_draw_rect(x_pos, y_pos, seg_width, height * 0.7, 1, color_border, color_fill);
        x_pos += seg_width + 2;
        y_pos -= height * 0.3;
        rg_gui_draw_rect(x_pos, y_pos, seg_width, height * 1.0, 1, color_border, color_fill);
    }

    if (gui.show_clock)
    {
        right += txt.width + 4;

        int x_pos = -right;
        int y_pos = RG_MAX(0, 1 + top);
        char buffer[12];
        time_t time_sec = time(NULL);
        struct tm *time = localtime(&time_sec);

        sprintf(buffer, "%02d:%02d", time->tm_hour, time->tm_min);
        rg_gui_draw_text(x_pos, y_pos, 0, buffer, C_SILVER, app->isLauncher ? C_TRANSPARENT : C_BLACK, 0);
    }
}

void rg_gui_draw_hourglass(void)
{
    rg_display_write((gui.screen_width / 2) - (image_hourglass.width / 2),
        (gui.screen_height / 2) - (image_hourglass.height / 2),
        image_hourglass.width,
        image_hourglass.height,
        image_hourglass.width * 2,
        (uint16_t*)image_hourglass.pixel_data, 0);
}

void rg_gui_draw_status_bars(void)
{
    size_t max_len = RG_MIN(gui.screen_width / RG_MAX(gui.style.font_width, 7), 99) + 1;
    char header[max_len];
    char footer[max_len];

    const rg_app_t *app = rg_system_get_app();
    rg_stats_t stats = rg_system_get_counters();

    if (!app->initialized || app->isLauncher)
        return;

    snprintf(header, max_len, "SPEED: %d%% (%d %d) / BUSY: %d%%",
        (int)round(stats.totalFPS / app->tickRate * 100.f),
        (int)round(stats.totalFPS),
        (int)app->frameskip,
        (int)round(stats.busyPercent));

    if (app->romPath && strlen(app->romPath) > max_len - 1)
        snprintf(footer, max_len, "...%s", app->romPath + (strlen(app->romPath) - (max_len - 4)));
    else if (app->romPath)
        snprintf(footer, max_len, "%s", app->romPath);
    else
        snprintf(footer, max_len, "Retro-Go %s", app->version);

    rg_gui_draw_text(0, RG_GUI_TOP, gui.screen_width, header, C_WHITE, C_BLACK, 0);
    rg_gui_draw_text(0, RG_GUI_BOTTOM, gui.screen_width, footer, C_WHITE, C_BLACK, 0);

    rg_gui_draw_icons();
}

static size_t get_dialog_items_count(const rg_gui_option_t *options)
{
    if (!options)
        return 0;

    const rg_gui_option_t *opt = options;
    while (opt->arg || opt->label || opt->value || opt->flags || opt->update_cb)
        opt++;
    return opt - options;
}

void rg_gui_draw_dialog(const char *title, const rg_gui_option_t *options, int sel)
{
    const size_t options_count = get_dialog_items_count(options);
    const int sep_width = TEXT_RECT(": ", 0).width;
    const int font_height = gui.style.font_height;
    const int max_box_width = 0.82f * gui.screen_width;
    const int max_box_height = 0.82f * gui.screen_height;
    const int box_padding = 6;
    const int row_padding_y = 0; // now handled by draw_text
    const int row_padding_x = 8;
    const int max_inner_width = max_box_width - sep_width - (row_padding_x + box_padding) * 2;
    const int min_row_height = TEXT_RECT(" ", max_inner_width).height + row_padding_y * 2;

    int box_width = box_padding * 2;
    int box_height = box_padding * 2 + (title ? font_height + 6 : 0);
    int inner_width = TEXT_RECT(title, 0).width;
    int col1_width = -1;
    int col2_width = -1;
    int row_height[options_count];

    for (size_t i = 0; i < options_count; i++)
    {
        rg_rect_t label = {0, min_row_height};
        rg_rect_t value = {0};

        if (options[i].flags == RG_DIALOG_FLAG_HIDDEN)
        {
            row_height[i] = 0;
            continue;
        }

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
        uint16_t color, fg, bg;
        int xx = x + row_padding_x;
        int yy = y + row_padding_y;
        int height = 8;

        if (options[i].flags == RG_DIALOG_FLAG_NORMAL)
            color = gui.style.item_standard;
        else if (options[i].flags == RG_DIALOG_FLAG_MESSAGE)
            color = gui.style.item_message;
        else
            color = gui.style.item_disabled;

        fg = (i == sel) ? gui.style.box_background : color;
        bg = (i == sel) ? color : gui.style.box_background;

        if (y + row_height[i] >= box_y + box_height)
            break;

        if (options[i].flags == RG_DIALOG_FLAG_HIDDEN)
            continue;

        if (false && options[i].flags == RG_DIALOG_FLAG_SEPARATOR)
        {
            // FIXME: Draw a nice dim line...
        }
        else if (options[i].value)
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

    rg_gui_draw_rect(box_x, box_y, box_width, box_height, box_padding, gui.style.box_background, C_NONE);
    rg_gui_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, gui.style.box_border, C_NONE);

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
}

intptr_t rg_gui_dialog(const char *title, const rg_gui_option_t *options_const, int selected_index)
{
    size_t options_count = get_dialog_items_count(options_const);
    int sel = selected_index < 0 ? (options_count + selected_index) : selected_index;
    int sel_old = -1;
    bool redraw = false;

    // Constrain initial cursor and skip FLAG_SKIP items
    sel = RG_MIN(RG_MAX(0, sel), options_count - 1);

    // We create a copy of options because the callbacks might modify it (ie option->value)
    rg_gui_option_t options[options_count + 1];
    char *text_buffer = calloc(options_count, 32);
    char *text_buffer_ptr = text_buffer;

    memcpy(options, options_const, sizeof(options));

    for (size_t i = 0; i < options_count; i++)
    {
        rg_gui_option_t *option = &options[i];
        if (!option->label)
            option->label = "";
        if (option->value && text_buffer)
            option->value = strcpy(text_buffer_ptr, option->value);
        if (option->update_cb)
            option->update_cb(option, RG_DIALOG_INIT);
        if (option->value && text_buffer)
            text_buffer_ptr += RG_MAX(strlen(option->value), 31) + 1;
    }

    rg_gui_draw_status_bars();
    rg_gui_draw_dialog(title, options, sel);
    rg_input_wait_for_key(RG_KEY_ALL, false, 1000);
    rg_task_delay(80);

    rg_gui_event_t event = RG_DIALOG_VOID;
    uint32_t joystick = 0, joystick_old;
    uint64_t joystick_last = 0;

    while (event != RG_DIALOG_CLOSE && event != RG_DIALOG_CANCEL)
    {
        // TO DO: Add acceleration!
        joystick_old = ((rg_system_timer() - joystick_last) > 300000) ? 0 : joystick;
        joystick = rg_input_read_gamepad();
        event = RG_DIALOG_VOID;

        if (joystick ^ joystick_old)
        {
            bool active_selection = options[sel].flags == RG_DIALOG_FLAG_NORMAL;
            rg_gui_callback_t callback = active_selection ? options[sel].update_cb : NULL;

            if (joystick & RG_KEY_UP) {
                if (--sel < 0) sel = options_count - 1;
            }
            else if (joystick & RG_KEY_DOWN) {
                if (++sel > options_count - 1) sel = 0;
            }
            else if (joystick & (RG_KEY_B|RG_KEY_OPTION|RG_KEY_MENU)) {
                event = RG_DIALOG_CANCEL;
            }
            else if (joystick & RG_KEY_LEFT && callback) {
                event = callback(&options[sel], RG_DIALOG_PREV);
                redraw = true;
            }
            else if (joystick & RG_KEY_RIGHT && callback) {
                event = callback(&options[sel], RG_DIALOG_NEXT);
                redraw = true;
            }
            else if (joystick & RG_KEY_START && callback) {
                event = callback(&options[sel], RG_DIALOG_ALT);
                redraw = true;
            }
            else if (joystick & RG_KEY_A && callback) {
                event = callback(&options[sel], RG_DIALOG_ENTER);
                redraw = true;
            }
            else if (joystick & RG_KEY_A && active_selection) {
                event = RG_DIALOG_CLOSE;
            }

            joystick_last = rg_system_timer();
        }

        if (sel_old != sel)
        {
            for (size_t i = 0; i < options_count; ++i)
            {
                // If the item is selectable, we stop here
                if (options[sel].flags == RG_DIALOG_FLAG_NORMAL)
                    break;
                if (options[sel].flags == RG_DIALOG_FLAG_DISABLED)
                    break;

                // Otherwise move to the next
                sel += (joystick == RG_KEY_UP) ? -1 : 1;

                if (sel < 0)
                    sel = options_count - 1;

                if (sel >= options_count)
                    sel = 0;
            }
            if (sel_old != -1 && options[sel_old].update_cb)
                options[sel_old].update_cb(&options[sel_old], RG_DIALOG_LEAVE);
            if (options[sel].update_cb)
                options[sel].update_cb(&options[sel], RG_DIALOG_FOCUS);
            redraw = true;
            sel_old = sel;
        }

        if (event == RG_DIALOG_REDRAW)
        {
            rg_display_force_redraw();
            rg_gui_draw_status_bars();
            redraw = true;
        }

        if (redraw)
        {
            for (size_t i = 0; i < options_count; i++)
            {
                if (options[i].update_cb)
                    options[i].update_cb(&options[i], RG_DIALOG_REDRAW);
            }
            rg_gui_draw_dialog(title, options, sel);
            redraw = false;
        }

        rg_task_delay(20);
        rg_system_tick(0);
    }

    rg_input_wait_for_key(joystick, false, 1000);
    rg_display_force_redraw();
    free(text_buffer);

    if (event == RG_DIALOG_CANCEL || sel < 0)
        return RG_DIALOG_CANCELLED;

    return options[sel].arg;
}

bool rg_gui_confirm(const char *title, const char *message, bool default_yes)
{
    const rg_gui_option_t options[] = {
        {0, message, NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, "",      NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {1, "Yes",   NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
        {0, "No ",   NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
        RG_DIALOG_END,
    };
    return rg_gui_dialog(title, message ? options : options + 1, default_yes ? -2 : -1) == 1;
}

void rg_gui_alert(const char *title, const char *message)
{
    const rg_gui_option_t options[] = {
        {0, message, NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, "",      NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {1, "OK",    NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
        RG_DIALOG_END,
    };
    rg_gui_dialog(title, message ? options : options + 1, -1);
}

typedef struct
{
    rg_gui_option_t options[22];
    size_t count;
    bool (*validator)(const char *path);
} file_picker_opts_t;

static int file_picker_cb(const rg_scandir_t *entry, void *arg)
{
    file_picker_opts_t *f = arg;
    if (f->validator && !(f->validator)(entry->path))
        return RG_SCANDIR_STOP;
    char *path = strdup(entry->path);
    f->options[f->count].arg = (intptr_t)path;
    f->options[f->count].flags = RG_DIALOG_FLAG_NORMAL;
    f->options[f->count].label = rg_basename(path);
    f->count++;
    if (f->count > 18)
        return RG_SCANDIR_STOP;
    return RG_SCANDIR_CONTINUE;
}

char *rg_gui_file_picker(const char *title, const char *path, bool (*validator)(const char *path), bool none_option)
{
    file_picker_opts_t options = {
        .options = {},
        .count = 0,
        .validator = validator,
    };

    if (!title)
        title = "Select file";

    if (none_option)
    {
        options.options[options.count++] = (rg_gui_option_t){0, "<None>", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
        // options.options[options.count++] = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
    }

    if (!rg_storage_scandir(path, file_picker_cb, &options, 0) || options.count < 1)
    {
        rg_gui_alert(title, "Folder is empty.");
        return NULL;
    }

    options.options[options.count] = (rg_gui_option_t)RG_DIALOG_END;

    char *filepath = (char *)rg_gui_dialog(title, options.options, 0);

    if (filepath != (void *)RG_DIALOG_CANCELLED)
        filepath = strdup(filepath ? filepath : "");
    else
        filepath = NULL;

    for (size_t i = 0; i < options.count; ++i)
        free((void *)(options.options[i].arg));

    return filepath;
}

void rg_gui_draw_keyboard(const char *title, const rg_gui_keyboard_t *map, size_t cursor)
{
    RG_ASSERT(map, "Bad param");

    int width = map->columns * 16 + 16;
    int height = map->rows * 16 + 32;

    int x_pos = (gui.screen_width - width) / 2;
    int y_pos = (gui.screen_height - height);

    char buf[2] = {0};

    rg_gui_draw_rect(x_pos, y_pos, width, height, 2, gui.style.box_border, gui.style.box_background);
    rg_gui_draw_text(x_pos + 4, y_pos + 4, width - 8, title, gui.style.item_message, gui.style.box_background, 0);
    y_pos += 16;

    for (size_t i = 0; i < map->columns * map->rows; ++i)
    {
        int x = x_pos + 8 + (i % map->columns) * 16;
        int y = y_pos + 8 + (i / map->columns) * 16;
        if (!map->data[i])
            continue;
        buf[0] = map->data[i];
        rg_gui_draw_text(x + 1, y + 1, 14, buf, C_BLACK, i == cursor ? C_CYAN : C_IVORY, RG_TEXT_ALIGN_CENTER);
    }
}

static rg_gui_event_t volume_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int level = rg_audio_get_volume();
    int prev_level = level;

    if (event == RG_DIALOG_PREV)
        level -= 5;
    if (event == RG_DIALOG_NEXT)
        level += 5;

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

    if (event == RG_DIALOG_PREV)
        level -= 10;
    if (event == RG_DIALOG_NEXT)
        level += 10;

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

    if (event == RG_DIALOG_PREV && --sink < 0)
        sink = max;
    if (event == RG_DIALOG_NEXT && ++sink > max)
        sink = 0;

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

    if (event == RG_DIALOG_PREV && --mode < 0)
        mode = max;
    if (event == RG_DIALOG_NEXT && ++mode > max)
        mode = 0;

    if (mode != prev_mode)
    {
        rg_display_set_filter(mode);
        return RG_DIALOG_REDRAW;
    }

    if (mode == RG_DISPLAY_FILTER_OFF)
        strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_FILTER_HORIZ)
        strcpy(option->value, "Horiz");
    if (mode == RG_DISPLAY_FILTER_VERT)
        strcpy(option->value, "Vert ");
    if (mode == RG_DISPLAY_FILTER_BOTH)
        strcpy(option->value, "Both ");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t scaling_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = RG_DISPLAY_SCALING_COUNT - 1;
    int mode = rg_display_get_scaling();
    int prev_mode = mode;

    if (event == RG_DIALOG_PREV && --mode < 0)
        mode = max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max)
        mode = 0; // max;

    if (mode != prev_mode)
    {
        rg_display_set_scaling(mode);
        return RG_DIALOG_REDRAW;
    }

    if (mode == RG_DISPLAY_SCALING_OFF)
        strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_SCALING_FIT)
        strcpy(option->value, "Fit ");
    if (mode == RG_DISPLAY_SCALING_FULL)
        strcpy(option->value, "Full ");
    if (mode == RG_DISPLAY_SCALING_CUSTOM)
        strcpy(option->value, "Custom");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t custom_width_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (rg_display_get_scaling() != RG_DISPLAY_SCALING_CUSTOM)
    {
        option->flags = RG_DIALOG_FLAG_HIDDEN;
        return RG_DIALOG_VOID;
    }

    if (event == RG_DIALOG_PREV)
        rg_display_set_custom_width(rg_display_get_custom_width() - 4);
    if (event == RG_DIALOG_NEXT)
        rg_display_set_custom_width(rg_display_get_custom_width() + 4);

    sprintf(option->value, "%d", rg_display_get_custom_width());
    option->flags = RG_DIALOG_FLAG_NORMAL;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        return RG_DIALOG_REDRAW;
    return RG_DIALOG_VOID;
}

static rg_gui_event_t custom_height_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (rg_display_get_scaling() != RG_DISPLAY_SCALING_CUSTOM)
    {
        option->flags = RG_DIALOG_FLAG_HIDDEN;
        return RG_DIALOG_VOID;
    }

    if (event == RG_DIALOG_PREV)
        rg_display_set_custom_height(rg_display_get_custom_height() - 4);
    if (event == RG_DIALOG_NEXT)
        rg_display_set_custom_height(rg_display_get_custom_height() + 4);

    sprintf(option->value, "%d", rg_display_get_custom_height());
    option->flags = RG_DIALOG_FLAG_NORMAL;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        return RG_DIALOG_REDRAW;
    return RG_DIALOG_VOID;
}

static rg_gui_event_t speedup_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    rg_app_t *app = rg_system_get_app();

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        // Probably should be broken into a function in rg_system or rg_emu...
        float change = (event == RG_DIALOG_NEXT) ? 0.5f : -0.5f;
        app->speed = RG_MIN(2.5f, RG_MAX(0.5f, app->speed + change));
        app->frameskip = (app->speed > 1.0f) ? 2 : 0; // Reset auto frameskip
        rg_audio_set_sample_rate(app->sampleRate * app->speed);
        rg_system_event(RG_EVENT_SPEEDUP, NULL);
    }

    sprintf(option->value, "%.1fx", app->speed);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t disk_activity_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        rg_storage_set_activity_led(!rg_storage_get_activity_led());
    }
    strcpy(option->value, rg_storage_get_activity_led() ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t show_clock_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        gui.show_clock = !gui.show_clock;
        rg_settings_set_number(NS_GLOBAL, SETTING_CLOCK, gui.show_clock);
        return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, gui.show_clock ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t font_type_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV && rg_gui_set_font(gui.font_index - 1))
        return RG_DIALOG_REDRAW;
    if (event == RG_DIALOG_NEXT && rg_gui_set_font(gui.font_index + 1))
        return RG_DIALOG_REDRAW;
    sprintf(option->value, "%s %d", gui.style.font->name, gui.style.font_height);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t theme_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        char *path = rg_gui_file_picker("Theme", RG_BASE_PATH_THEMES, NULL, true);
        if (path != NULL)
        {
            const char *theme = strlen(path) > 0 ? rg_basename(path) : NULL;
            rg_gui_set_theme(theme);
            free(path);
            return RG_DIALOG_REDRAW;
        }
    }

    strcpy(option->value, rg_gui_get_theme_name() ?: "Default");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t border_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        char *path = rg_gui_file_picker("Border", RG_BASE_PATH_BORDERS, NULL, true);
        if (path != NULL)
        {
            rg_display_set_border(strlen(path) ? path : NULL);
            free(path);
            return RG_DIALOG_REDRAW;
        }
    }
    char *border = rg_display_get_border();
    sprintf(option->value, "%.9s", border ? rg_basename(border) : "None");
    free(border);
    return RG_DIALOG_VOID;
}

void rg_gui_options_menu(void)
{
    rg_gui_option_t options[24];
    rg_gui_option_t *opt = &options[0];
    rg_app_t *app = rg_system_get_app();

    *opt++ = (rg_gui_option_t){0, "Brightness", "-", RG_DIALOG_FLAG_NORMAL, &brightness_update_cb};
    *opt++ = (rg_gui_option_t){0, "Volume    ", "-", RG_DIALOG_FLAG_NORMAL, &volume_update_cb};
    *opt++ = (rg_gui_option_t){0, "Audio out ", "-", RG_DIALOG_FLAG_NORMAL, &audio_update_cb};

    // Global settings that aren't essential to show when inside a game
    if (app->isLauncher)
    {
        *opt++ = (rg_gui_option_t){0, "Disk LED  ", "-", RG_DIALOG_FLAG_NORMAL, &disk_activity_cb};
        *opt++ = (rg_gui_option_t){0, "Font type ", "-", RG_DIALOG_FLAG_NORMAL, &font_type_cb};
        *opt++ = (rg_gui_option_t){0, "Theme     ", "-", RG_DIALOG_FLAG_NORMAL, &theme_cb};
        *opt++ = (rg_gui_option_t){0, "Show clock", "-", RG_DIALOG_FLAG_NORMAL, &show_clock_cb};
    }
    // App settings that are shown only inside a game
    else
    {
        *opt++ = (rg_gui_option_t){0, "Scaling",   "-", RG_DIALOG_FLAG_NORMAL, &scaling_update_cb};
        *opt++ = (rg_gui_option_t){0, "  Width",   "-", RG_DIALOG_FLAG_HIDDEN, &custom_width_cb};
        *opt++ = (rg_gui_option_t){0, "  Height",  "-", RG_DIALOG_FLAG_HIDDEN, &custom_height_cb};
        *opt++ = (rg_gui_option_t){0, "Filter",    "-", RG_DIALOG_FLAG_NORMAL, &filter_update_cb};
        *opt++ = (rg_gui_option_t){0, "Border",    "-", RG_DIALOG_FLAG_NORMAL, &border_update_cb};
        *opt++ = (rg_gui_option_t){0, "Speed",     "-", RG_DIALOG_FLAG_NORMAL, &speedup_update_cb};
    }

    size_t extra_options = get_dialog_items_count(app->options);
    for (size_t i = 0; i < extra_options; i++)
        *opt++ = app->options[i];

    *opt++ = (rg_gui_option_t)RG_DIALOG_END;

    rg_audio_set_mute(true);

    rg_gui_dialog("Options", options, 0);

    rg_settings_commit();
    rg_system_save_time();
    rg_audio_set_mute(false);
}

void rg_gui_sysinfo_menu(void)
{
    char screen_str[32], network_str[64], memory_str[32];
    char storage_str[32], uptime[32];

    const rg_gui_option_t options[] = {
        {0, "Console", RG_TARGET_NAME, RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Screen ", screen_str,     RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Memory ", memory_str,     RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Network", network_str,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Storage", storage_str,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Uptime ", uptime,         RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_SEPARATOR,
        {0, "Close  ", NULL,           RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END
    };

    const rg_display_t *display = rg_display_get_info();
    rg_stats_t stats = rg_system_get_counters();

    snprintf(screen_str, 32, "%dx%d (%d)", display->screen.width, display->screen.height, display->screen.format);
    snprintf(memory_str, 32, "%dKB + %dKB", stats.totalMemoryInt / 1024, stats.totalMemoryExt / 1024);
    snprintf(uptime, 32, "%ds", stats.uptime);
    snprintf(storage_str, 32, "%s", "N/A");

#ifdef RG_ENABLE_NETWORKING
    rg_network_t net = rg_network_get_info();
    if (net.state == RG_NETWORK_CONNECTED)
        snprintf(network_str, 64, "%s\n%s", net.name, net.ip_addr);
    else if (net.state == RG_NETWORK_CONNECTING)
        snprintf(network_str, 64, "%s\n%s", net.name, "connecting...");
    else if (net.name[0])
        snprintf(network_str, 64, "%s\n%s", net.name, "disconnected");
    else
        snprintf(network_str, 64, "%s", "disconnected");
#else
    strcpy(network_str, "No adapter");
#endif

    rg_gui_dialog("System Information", options, -1);
}

void rg_gui_about_menu(const rg_gui_option_t *extra_options)
{
    char build_ver[40], build_date[40], build_user[40], title[40];

    size_t extra_options_count = get_dialog_items_count(extra_options);

    rg_gui_option_t options[16 + extra_options_count];
    rg_gui_option_t *opt = &options[0];

    *opt++ = (rg_gui_option_t){0, "Version", build_ver,  RG_DIALOG_FLAG_MESSAGE, NULL};
    *opt++ = (rg_gui_option_t){0, "Date   ", build_date, RG_DIALOG_FLAG_MESSAGE, NULL};
    *opt++ = (rg_gui_option_t){0, "By     ", build_user, RG_DIALOG_FLAG_MESSAGE, NULL};
    *opt++ = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
    *opt++ = (rg_gui_option_t){1000, "System information", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    for (size_t i = 0; i < extra_options_count; i++)
        *opt++ = extra_options[i];
    *opt++ = (rg_gui_option_t){2000, "Reset settings", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    *opt++ = (rg_gui_option_t){3000, "Debug", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    *opt++ = (rg_gui_option_t)RG_DIALOG_END;

    const rg_app_t *app = rg_system_get_app();

    snprintf(build_ver, sizeof(build_ver), "%s", app->version);
    snprintf(build_date, sizeof(build_date), "%s", app->buildDate);
    snprintf(build_user, sizeof(build_user), "%s", app->buildUser);
    snprintf(title, sizeof(title), "About Retro-Go"); // , app->name

    while (true)
    {
        switch (rg_gui_dialog(title, options, 4))
        {
            case 1000:
                rg_gui_sysinfo_menu();
                break;
            case 2000:
                if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                    rg_storage_delete(RG_BASE_PATH_CACHE);
                    rg_settings_reset();
                    rg_system_restart();
                    return;
                }
                break;
            case 3000:
                rg_gui_debug_menu(NULL);
                break;
            default:
                return;
        }
    }
}

void rg_gui_debug_menu(const rg_gui_option_t *extra_options)
{
    char screen_res[20], source_res[20], scaled_res[20];
    char stack_hwm[20], heap_free[20], block_free[20];
    char local_time[32], timezone[32], uptime[20];
    char battery_info[25], frame_time[32];

    const rg_gui_option_t options[] = {
        {0, "Screen res", screen_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Source res", source_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Scaled res", scaled_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Stack HWM ", stack_hwm,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Heap free ", heap_free,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Block free", block_free,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Local time", local_time,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Timezone  ", timezone,     RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Uptime    ", uptime,       RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Battery   ", battery_info, RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Blit time ", frame_time,   RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_SEPARATOR,
        {1, "Reboot to firmware", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, "Clear cache    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3, "Save screenshot", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {4, "Save trace", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {5, "Cheats    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {6, "Crash     ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END
    };

    const rg_display_t *display = rg_display_get_info();
    rg_display_counters_t display_stats = rg_display_get_counters();
    rg_stats_t stats = rg_system_get_counters();
    time_t now = time(NULL);

    strftime(local_time, 32, "%F %T", localtime(&now));
    snprintf(timezone, 32, "%s", getenv("TZ") ?: "N/A");
    snprintf(screen_res, 20, "%dx%d", display->screen.width, display->screen.height);
    snprintf(source_res, 20, "%dx%d", display->source.width, display->source.height);
    snprintf(scaled_res, 20, "%dx%d", display->viewport.width, display->viewport.height);
    if (display_stats.totalFrames > 0)
    {
        int ms = (float)display_stats.busyTime / display_stats.totalFrames / 1000.f;
        int full = (float)display_stats.fullFrames / display_stats.totalFrames * 100.f;
        snprintf(frame_time, 20, "%dms (FULL: %d%%)", ms, full);
    }
    else
        snprintf(frame_time, 20, "N/A");
    snprintf(stack_hwm, 20, "%d", stats.freeStackMain);
    snprintf(heap_free, 20, "%d+%d", stats.freeMemoryInt, stats.freeMemoryExt);
    snprintf(block_free, 20, "%d+%d", stats.freeBlockInt, stats.freeBlockExt);
    snprintf(uptime, 20, "%ds", (int)(rg_system_timer() / 1000000));
    rg_battery_t battery;
    if (rg_input_read_battery_raw(&battery))
    {
        snprintf(battery_info, sizeof(battery_info), "%.2f%% | %.2fV", battery.level, battery.volts);
    }
    else
    {
        snprintf(battery_info, sizeof(battery_info), "N/A");
    }

    switch (rg_gui_dialog("Debugging", options, 0))
    {
    case 1:
        rg_system_switch_app(RG_APP_FACTORY, 0, 0, 0);
        break;
    case 2:
        rg_storage_delete(RG_BASE_PATH_CACHE);
        rg_system_restart();
        break;
    case 3:
        rg_emu_screenshot(RG_STORAGE_ROOT "/screenshot.png", 0, 0);
        break;
    case 4:
        rg_system_save_trace(RG_STORAGE_ROOT "/trace.txt", 0);
        break;
    case 5:
        break;
    case 6:
        RG_PANIC("Crash test!");
        break;
    }
}

static rg_gui_event_t slot_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_FOCUS)
    {
        rg_emu_slot_t *slot = (rg_emu_slot_t *)option->arg;
        rg_image_t *preview = NULL;
        rg_color_t color = C_BLUE;
        size_t margin = 0; // TEXT_RECT("ABC", 0).height;
        size_t border = 3;
        char buffer[100];
        if (slot->is_used)
        {
            preview = rg_image_load_from_file(slot->preview, 0);
            if (slot->is_lastused)
                snprintf(buffer, sizeof(buffer), "Slot %d (last used)", slot->id);
            else
                snprintf(buffer, sizeof(buffer), "Slot %d", slot->id);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "Slot %d is empty", slot->id);
            color = C_RED;
        }
        rg_gui_draw_image(0, margin, gui.screen_width, gui.screen_height - margin * 2, true, preview);
        rg_gui_draw_rect(0, margin, gui.screen_width, gui.screen_height - margin * 2, border, color, C_NONE);
        rg_gui_draw_rect(border, margin + border, gui.screen_width - border * 2, gui.style.font_height * 2 + 6, 0, C_BLACK, C_BLACK);
        rg_gui_draw_text(border + 60, margin + border + 5, gui.screen_width - border * 2 - 120, buffer, C_WHITE, C_BLACK, RG_TEXT_ALIGN_CENTER|RG_TEXT_BIGGER|RG_TEXT_NO_PADDING);
        rg_image_free(preview);
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
    rg_emu_states_t *savestates = rg_emu_get_states(rom_path ?: rg_system_get_app()->romPath, 4);
    const rg_gui_option_t choices[] = {
        {(intptr_t)&savestates->slots[0], "Slot 0", NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[1], "Slot 1", NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[2], "Slot 2", NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[3], "Slot 3", NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        RG_DIALOG_END
    };
    int sel = 0;

    if (!rom_path)
        sel = rg_system_get_app()->saveSlot;
    else if (savestates->lastused)
        sel = savestates->lastused->id;

    intptr_t ret = rg_gui_dialog(title, choices, sel);
    if (ret && ret != RG_DIALOG_CANCELLED)
        sel = ((rg_emu_slot_t *)ret)->id;
    else
        sel = -1;

    free(savestates);

    return sel;
}

void rg_gui_game_menu(void)
{
    const rg_gui_option_t choices[] = {
        {1000, "Save & Continue", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2000, "Save & Quit    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3001, "Load game      ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3000, "Reset          ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        #ifdef RG_ENABLE_NETPLAY
        {5000, "Netplay ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        #endif
        #if !RG_GAMEPAD_HAS_OPTION_BTN
        {5500, "Options ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        #endif
        {6000, "About   ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {7000, "Quit    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END
    };
    int slot, sel;

    rg_audio_set_mute(true);

    sel = rg_gui_dialog("Retro-Go", choices, 0);

    rg_settings_commit();
    rg_system_save_time();

    if (sel == 3000)
    {
        const rg_gui_option_t choices[] = {
            {3002, "Soft reset", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            {3003, "Hard reset", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            RG_DIALOG_END
        };
        sel = rg_gui_dialog("Reset Emulation?", choices, 0);
    }

    switch (sel)
    {
        case 1000: if ((slot = rg_gui_savestate_menu("Save", 0, 0)) >= 0) rg_emu_save_state(slot); break;
        case 2000: if ((slot = rg_gui_savestate_menu("Save", 0, 0)) >= 0) {rg_emu_save_state(slot); rg_system_exit();} break;
        case 3001: if ((slot = rg_gui_savestate_menu("Load", 0, 0)) >= 0) rg_emu_load_state(slot); break;
        case 3002: rg_emu_reset(false); break;
        case 3003: rg_emu_reset(true); break;
    #ifdef RG_ENABLE_NETPLAY
        case 5000: rg_netplay_quick_start(); break;
    #endif
        case 5500: rg_gui_options_menu(); break;
        case 6000: rg_gui_about_menu(NULL); break;
        case 7000: rg_system_exit(); break;
    }

    rg_audio_set_mute(false);
}
