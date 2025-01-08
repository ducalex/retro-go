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
    int screen_safezone; // rg_rect_t
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
        rg_color_t shadow;
    } style;
    char theme_name[32];
    cJSON *theme_obj;
    int font_index;
    bool show_clock;
    bool initialized;
} gui;

#define SETTING_FONTTYPE    "FontType"
#define SETTING_CLOCK       "Clock"
#define SETTING_THEME       "Theme"
#define SETTING_WIFI_ENABLE "Enable"
#define SETTING_WIFI_SLOT   "Slot"
#define SETTING_LANGUAGE    "Language"

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

    if (fill_color != C_NONE)
    {
        for (size_t i = 0; i < pixels; ++i)
            gui.draw_buffer[i] = fill_color;
    }

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
    gui.screen_width = rg_display_get_width();
    gui.screen_height = rg_display_get_height();
    #ifdef RG_SCREEN_HAS_ROUND_CORNERS
    gui.screen_safezone = 20;
    #else
    gui.screen_safezone = 0;
    #endif
    gui.draw_buffer = get_draw_buffer(gui.screen_width, 18, C_BLACK);
    rg_gui_set_language_id(rg_settings_get_number(NS_GLOBAL, SETTING_LANGUAGE, RG_LANG_EN));
    rg_gui_set_font(rg_settings_get_number(NS_GLOBAL, SETTING_FONTTYPE, RG_FONT_VERA_12));
    rg_gui_set_theme(rg_settings_get_string(NS_GLOBAL, SETTING_THEME, NULL));
    gui.show_clock = rg_settings_get_boolean(NS_GLOBAL, SETTING_CLOCK, false);
    gui.initialized = true;
}

bool rg_gui_set_language_id(int index)
{
    if (rg_localization_set_language_id(index))
    {
        rg_settings_set_number(NS_GLOBAL, SETTING_LANGUAGE, index);
        RG_LOGI("Language set to: %s (%d)", rg_localization_get_language_name(index), index);
        return true;
    }
    rg_localization_set_language_id(RG_LANG_EN);
    RG_LOGE("Invalid language id %d!", index);
    return false;
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
        char *data;
        size_t data_len;
        if (rg_storage_read_file(pathbuf, (void **)&data, &data_len, 0))
        {
            new_theme = cJSON_Parse(data);
            if (!new_theme) // Parse failure, clean the markup and try again
                new_theme = cJSON_Parse(rg_json_fixup(data));
            free(data);
        }
        if (!new_theme)
            RG_LOGE("Failed to load theme JSON from '%s'!\n", pathbuf);
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
    gui.style.shadow = rg_gui_get_theme_color("dialog", "shadow", C_NONE);

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
    if (strcmp(strval, "none") == 0)
        return C_NONE;
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
    return rg_surface_load_image_file(pathbuf, 0);
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

void rg_gui_set_surface(rg_surface_t *surface)
{
    gui.screen_buffer = surface ? surface->data : NULL;
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
        rg_display_write_rect(left, top, width, height, stride, buffer, 0);
    }
}

static size_t get_glyph(uint32_t *output, const rg_font_t *font, int points, int c)
{
    // Some glyphs are always zero width
    if (!font || c == '\r' || c == '\n' || c < 8 || c > 254)
        return 0;

    size_t glyph_width = 0;

    if (font->type == 0) // Monospace Bitmap
    {
        glyph_width = font->width;
        if (output)
        {
            if (c >= font->chars)
            {
                for (int y = 0; y < font->height; y++)
                    output[y] = 0;
            }
            else if (font->width > 8)
            {
                uint16_t *pattern = (uint16_t *)font->data + (c * font->height);
                for (int y = 0; y < font->height; y++)
                    output[y] = pattern[y];
            }
            else
            {
                uint8_t *pattern = (uint8_t *)font->data + (c * font->height);
                for (int y = 0; y < font->height; y++)
                    output[y] = pattern[y];
            }
        }
    }
    else if (font->type == 1) // Proportional
    {
        // Based on code by Boris Lovosevic (https://github.com/loboris)
        int charCode, adjYOffset, width, height, xOffset, xDelta;
        const uint8_t *data = font->data;
        while (1)
        {
            charCode = *data++;
            adjYOffset = *data++;
            width = *data++;
            height = *data++;
            xOffset = *data++;
            xOffset = xOffset < 0x80 ? xOffset : -(0xFF - xOffset);
            xDelta = *data++;

            if (charCode == c || charCode == 0xFF)
                break;

            if (width != 0)
                data += (((width * height) - 1) / 8) + 1;
        }

        // If the glyph is not found, we fallback to the basic font which has most glyphs.
        // It will be ugly, but at least the letter won't be missing...
        if (charCode != c)
            return get_glyph(output, &font_basic8x8, RG_MAX(8, points - 2), c);

        glyph_width = RG_MAX(width, xDelta);
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
            uint32_t bitmap[32] = {0};
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
                    uint32_t row = bitmap[y];
                    if (row != 0) // get_draw_buffer fills the bg color, nothing to do if row empty
                    {
                        uint16_t *output = &draw_buffer[(draw_width * (y + padding)) + x_offset];
                        for (int x = 0; x < width; x++)
                            output[x] = ((row >> x) & 1) ? color_fg : color_bg;
                    }
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

    return (rg_rect_t){x_pos, y_pos, draw_width, y_offset};
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
        rg_image_t *new_img = rg_surface_resize(img, width, height);
        rg_gui_copy_buffer(x_pos, y_pos, width, height, new_img->width * 2, new_img->data);
        rg_surface_free(new_img);
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
        // rg_gui_draw_text(x_pos + 2, y_pos + 2, width - 4, "No image", C_DIM_GRAY, C_BLACK, 0);
    }
}

void rg_gui_draw_icons(void)
{
    rg_battery_t battery = rg_input_read_battery();
    rg_network_t network = rg_network_get_info();
    rg_rect_t txt = TEXT_RECT("00:00", 0);
    int bar_height = txt.height;
    int icon_height = RG_MAX(8, bar_height - 4);
    int icon_top = RG_MAX(0, (bar_height - icon_height - 1) / 2);
    int right = gui.screen_safezone;

    if (battery.present)
    {
        right += 22;

        int width = 16;
        int height = icon_height;
        int width_fill = width / 100.f * battery.level;
        int x_pos = -right;
        int y_pos = icon_top;

        rg_color_t color_fill = (battery.level > 20 ? (battery.level > 40 ? C_FOREST_GREEN : C_ORANGE) : C_RED);
        rg_color_t color_border = C_SILVER;
        rg_color_t color_empty = C_BLACK;

        rg_gui_draw_rect(x_pos, y_pos, width + 2, height, 1, color_border, C_NONE);
        rg_gui_draw_rect(x_pos + width + 2, y_pos + 2, 2, height - 4, 1, color_border, C_NONE);
        rg_gui_draw_rect(x_pos + 1, y_pos + 1, width_fill, height - 2, 0, 0, color_fill);
        rg_gui_draw_rect(x_pos + 1 + width_fill, y_pos + 1, width - width_fill, height - 2, 0, 0, color_empty);
    }

    if (network.state > RG_NETWORK_DISCONNECTED)
    {
        right += 22;

        int width = 16;
        int height = icon_height;
        int seg_width = (width - 2 - 2) / 3;
        int x_pos = -right;
        int y_pos = icon_top;

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
        int y_pos = 0;
        char buffer[12];
        time_t time_sec = time(NULL);
        struct tm *time = localtime(&time_sec);

        sprintf(buffer, "%02d:%02d", time->tm_hour, time->tm_min);
        rg_gui_draw_text(x_pos, y_pos, 0, buffer, C_SILVER, gui.screen_buffer ? C_TRANSPARENT : C_BLACK, 0);
    }
}

void rg_gui_draw_hourglass(void)
{
    rg_display_write_rect(
        get_horizontal_position(RG_GUI_CENTER, image_hourglass.width),
        get_vertical_position(RG_GUI_CENTER, image_hourglass.height),
        image_hourglass.width,
        image_hourglass.height,
        image_hourglass.width * 2,
        (uint16_t*)image_hourglass.pixel_data, 0);
}

void rg_gui_draw_status_bars(void)
{
    size_t max_len = gui.screen_width / 8;
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

    // FIXME: Respect gui.safezone (draw black background full screen_width, but pad the text if needed)
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
        rg_rect_t label = {0, 0, 0, min_row_height};
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

        bool highlight = (options[i].flags & RG_DIALOG_FLAG_MODE_MASK) != RG_DIALOG_FLAG_SKIP && i == sel;
        fg = highlight ? gui.style.box_background : color;
        bg = highlight ? color : gui.style.box_background;

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

void rg_gui_draw_message(const char *format, ...)
{
    RG_ASSERT_ARG(format);

    char buffer[512];
    va_list va;
    va_start(va, format);
    vsnprintf(buffer, sizeof(buffer), format, va);
    va_end(va);
    const rg_gui_option_t options[] = {
        {0, buffer, NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        RG_DIALOG_END,
    };
    // FIXME: Should rg_display_force_redraw() be called? Before? After? Both?
    rg_gui_draw_dialog(NULL, options, 0);
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

    while (event != RG_DIALOG_SELECT && event != RG_DIALOG_CANCEL)
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
            else if (joystick & RG_KEY_A && callback) {
                event = callback(&options[sel], RG_DIALOG_ENTER);
                redraw = true;
            }
            else if (joystick & RG_KEY_A && active_selection) {
                event = RG_DIALOG_SELECT;
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
                options[sel_old].update_cb(&options[sel_old], RG_DIALOG_FOCUS_LOST);
            if (options[sel].update_cb)
                options[sel].update_cb(&options[sel], RG_DIALOG_FOCUS_GAINED);
            redraw = true;
            sel_old = sel;
        }

        if (event == RG_DIALOG_REDRAW || event == RG_DIALOG_UPDATE)
        {
            for (size_t i = 0; i < options_count; i++)
            {
                if (options[i].update_cb)
                    options[i].update_cb(&options[i], RG_DIALOG_UPDATE);
            }
            if (event == RG_DIALOG_REDRAW)
            {
                rg_display_force_redraw();
                rg_gui_draw_status_bars();
            }
            redraw = true;
        }

        if (redraw)
        {
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
        {1, _("Yes"),   NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
        {0, _("No"),   NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
        RG_DIALOG_END,
    };
    return rg_gui_dialog(title, message ? options : options + 1, default_yes ? -2 : -1) == 1;
}

void rg_gui_alert(const char *title, const char *message)
{
    const rg_gui_option_t options[] = {
        {0, message, NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, "",      NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
        {1, _("OK"),    NULL, RG_DIALOG_FLAG_NORMAL,  NULL},
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
        return RG_SCANDIR_SKIP;
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
        title = _("Select file");

    if (none_option)
    {
        options.options[options.count++] = (rg_gui_option_t){0, _("<None>"), NULL, RG_DIALOG_FLAG_NORMAL, NULL};
        // options.options[options.count++] = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
    }

    if (!rg_storage_scandir(path, file_picker_cb, &options, 0) || options.count < 1)
    {
        rg_gui_alert(title, _("Folder is empty."));
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

void rg_gui_draw_keyboard(const rg_keyboard_map_t *map, size_t cursor)
{
    RG_ASSERT_ARG(map);

    int width = map->columns * 16 + 16;
    int height = map->rows * 16 + 16;

    int x_pos = (gui.screen_width - width) / 2;
    int y_pos = (gui.screen_height - height);

    char buf[2] = {0};

    rg_gui_draw_rect(x_pos, y_pos, width, height, 2, gui.style.box_border, gui.style.box_background);

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
    // Hide dummy unless it's the only one or we're a debug build
    int min = rg_system_get_app()->isRelease ? (1 % count) : (0);
    int max = count - 1;
    int sink = 0;

    // If there's no choice to be made we can just hide the entry
    if (min == max)
    {
        option->flags |= RG_DIALOG_FLAG_HIDDEN;
        return RG_DIALOG_VOID;
    }

    for (int i = 0; i < count; ++i)
        if (sinks[i].driver == ssink->driver && sinks[i].device == ssink->device)
            sink = i;

    int prev_sink = sink;

    if (event == RG_DIALOG_PREV && --sink < min)
        sink = max;
    if (event == RG_DIALOG_NEXT && ++sink > max)
        sink = min;

    if (sink != prev_sink)
        rg_audio_set_sink(sinks[sink].driver->name, sinks[sink].device);

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
        strcpy(option->value, _("Off"));
    if (mode == RG_DISPLAY_FILTER_HORIZ)
        strcpy(option->value, _("Horiz"));
    if (mode == RG_DISPLAY_FILTER_VERT)
        strcpy(option->value, _("Vert"));
    if (mode == RG_DISPLAY_FILTER_BOTH)
        strcpy(option->value, _("Both"));

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
        strcpy(option->value, _("Off"));
    else if (mode == RG_DISPLAY_SCALING_FIT)
        strcpy(option->value, _("Fit"));
    else if (mode == RG_DISPLAY_SCALING_FULL)
        strcpy(option->value, _("Full"));
    else if (mode == RG_DISPLAY_SCALING_ZOOM)
        strcpy(option->value, _("Zoom"));

    return RG_DIALOG_VOID;
}

static rg_gui_event_t custom_zoom_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (rg_display_get_scaling() != RG_DISPLAY_SCALING_ZOOM)
    {
        option->flags = RG_DIALOG_FLAG_HIDDEN;
        return RG_DIALOG_VOID;
    }

    if (event == RG_DIALOG_PREV)
        rg_display_set_custom_zoom(rg_display_get_custom_zoom() - 0.05);
    if (event == RG_DIALOG_NEXT)
        rg_display_set_custom_zoom(rg_display_get_custom_zoom() + 0.05);

    sprintf(option->value, "%.2f", rg_display_get_custom_zoom());
    option->flags = RG_DIALOG_FLAG_NORMAL;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        return RG_DIALOG_REDRAW;
    return RG_DIALOG_VOID;
}

static rg_gui_event_t overclock_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV)
        rg_system_set_overclock(rg_system_get_overclock() - 1);
    else if (event == RG_DIALOG_NEXT)
        rg_system_set_overclock(rg_system_get_overclock() + 1);
    sprintf(option->value, "%dMhz", 240 + (rg_system_get_overclock() * 40));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t speedup_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        float change = (event == RG_DIALOG_NEXT) ? 0.5f : -0.5f;
        rg_emu_set_speed(rg_emu_get_speed() + change);
    }
    sprintf(option->value, "%.1fx", rg_emu_get_speed());
    return RG_DIALOG_VOID;
}

static rg_gui_event_t led_indicator_opt_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        rg_system_set_indicator_mask(option->arg, !rg_system_get_indicator_mask(option->arg));
    }
    strcpy(option->value, rg_system_get_indicator_mask(option->arg) ? _("On") : _("Off"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t led_indicator_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[] = {
            {RG_INDICATOR_ACTIVITY_SYSTEM, _("System activity"), "-", RG_DIALOG_FLAG_NORMAL, &led_indicator_opt_cb},
            {RG_INDICATOR_ACTIVITY_DISK, _("Disk activity"), "-", RG_DIALOG_FLAG_NORMAL, &led_indicator_opt_cb},
            {RG_INDICATOR_POWER_LOW, _("Low battery"), "-", RG_DIALOG_FLAG_NORMAL, &led_indicator_opt_cb},
            RG_DIALOG_END,
        };
        rg_gui_dialog(option->label, options, 0);
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t show_clock_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        gui.show_clock = !gui.show_clock;
        rg_settings_set_boolean(NS_GLOBAL, SETTING_CLOCK, gui.show_clock);
        return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, gui.show_clock ? _("On") : _("Off"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t timezone_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const char utc_offsets[][10] = {"UTC-12:00", "UTC-11:00", "UTC-10:00", "UTC-09:00", "UTC-09:30", "UTC-08:00",
                                    "UTC-07:00", "UTC-06:00", "UTC-05:00", "UTC-04:00", "UTC-03:30", "UTC-03:00",
                                    "UTC-02:00", "UTC-01:00", "UTC+00:00", "UTC+01:00", "UTC+02:00", "UTC+03:00",
                                    "UTC+03:30", "UTC+04:00", "UTC+04:30", "UTC+05:00", "UTC+05:30", "UTC+06:00",
                                    "UTC+06:30", "UTC+07:00", "UTC+08:00", "UTC+09:00", "UTC+09:30", "UTC+10:00",
                                    "UTC+10:30", "UTC+11:00", "UTC+12:00", "UTC+13:00", "UTC+14:00"};
    int index = 14, old_index = index;
    char *TZ = rg_system_get_timezone();
    if (TZ && strncmp(TZ, "UTC", 3) == 0)
    {
        // TZ has inverted offset for whatever reason
        TZ[3] = TZ[3] == '-' ? '+' : '-';
        for (size_t i = 0; i < RG_COUNT(utc_offsets); ++i)
        {
            if (strcmp(TZ, utc_offsets[i]) == 0)
            {
                index = old_index = i;
                break;
            }
        }
    }
    free(TZ);
    if (event == RG_DIALOG_NEXT && index < RG_COUNT(utc_offsets) - 1)
        index++;
    else if (event == RG_DIALOG_PREV && index > 0)
        index--;
    if (index != old_index)
    {
        char *TZ = strdup(utc_offsets[index]);
        TZ[3] = TZ[3] == '-' ? '+' : '-';
        rg_system_set_timezone(TZ);
        free(TZ);
        if (gui.show_clock)
            return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, utc_offsets[index]);
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

static rg_gui_event_t language_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int language_id = rg_localization_get_language_id();

    if (event == RG_DIALOG_ENTER)
    {
        rg_gui_option_t options[RG_LANG_MAX + 1];
        for (int i = 0; i < RG_LANG_MAX; i++)
            options[i] = (rg_gui_option_t){i, rg_localization_get_language_name(i), NULL, RG_DIALOG_FLAG_NORMAL, NULL};
        options[RG_LANG_MAX] = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog(option->label, options, language_id);
        if (sel != RG_DIALOG_CANCELLED)
        {
            rg_gui_set_language_id(sel);
            if (rg_gui_confirm(_("Language changed!"), _("For these changes to take effect you must restart your device.\nrestart now?"), true))
            {
                rg_system_exit();
            }
            language_id = sel;
        }
        return RG_DIALOG_REDRAW;
    }

    sprintf(option->value, "%s", rg_localization_get_language_name(language_id) ?: "???");
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
    sprintf(option->value, "%.9s", border ? rg_basename(border) : _("None"));
    free(border);
    return RG_DIALOG_VOID;
}

#ifdef RG_ENABLE_NETWORKING
static void wifi_toggle_interactive(bool enable, int slot)
{
    rg_network_state_t target_state = enable ? RG_NETWORK_CONNECTED : RG_NETWORK_DISCONNECTED;
    int64_t timeout = rg_system_timer() + 20 * 1000000;
    rg_gui_draw_message(enable ? _("Connecting...") : _("Disconnecting..."));
    rg_network_wifi_stop();
    if (enable)
    {
        rg_wifi_config_t config = {0};
        rg_network_wifi_read_config(slot, &config);
        rg_network_wifi_set_config(&config);
        if (slot == 9000)
        {
            const rg_wifi_config_t config = {
                .ssid = "retro-go",
                .password = "retro-go",
                .channel = 6,
                .ap_mode = true,
            };
            rg_network_wifi_set_config(&config);
        }
        if (!rg_network_wifi_start())
            return;
    }
    do // Always loop at least once, in case we're in a transition
    {
        rg_task_delay(100);
        if (rg_system_timer() > timeout)
            break;
        if (rg_input_read_gamepad())
            break;
    } while (rg_network_get_info().state != target_state);
}

static rg_gui_event_t wifi_status_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    rg_network_t info = rg_network_get_info();
    if (info.state != RG_NETWORK_CONNECTED)
        strcpy(option->value, _("Not connected"));
    else if (option->arg == 0x10)
        strcpy(option->value, info.name);
    else if (option->arg == 0x11)
        strcpy(option->value, info.ip_addr);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_profile_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int slot = rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, -1);
    char labels[5][40] = {0};
    for (size_t i = 0; i < 5; i++)
    {
        rg_wifi_config_t config;
        strncpy(labels[i], rg_network_wifi_read_config(i, &config) ? config.ssid : _("(empty)"), 32);
    }
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[] = {
            {0, "0", labels[0], RG_DIALOG_FLAG_NORMAL, NULL},
            {1, "1", labels[1], RG_DIALOG_FLAG_NORMAL, NULL},
            {2, "2", labels[2], RG_DIALOG_FLAG_NORMAL, NULL},
            {3, "3", labels[3], RG_DIALOG_FLAG_NORMAL, NULL},
            {4, "4", labels[4], RG_DIALOG_FLAG_NORMAL, NULL},
            RG_DIALOG_END,
        };
        int sel = rg_gui_dialog(option->label, options, slot);
        if (sel != RG_DIALOG_CANCELLED)
        {
            rg_settings_set_boolean(NS_WIFI, SETTING_WIFI_ENABLE, true);
            rg_settings_set_number(NS_WIFI, SETTING_WIFI_SLOT, sel);
            wifi_toggle_interactive(true, sel);
        }
        return RG_DIALOG_REDRAW;
    }
    if (slot >= 0 && slot < RG_COUNT(labels))
        sprintf(option->value, "%d - %s", slot, labels[slot]);
    else
        strcpy(option->value, _("none"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_access_point_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        if (rg_gui_confirm(_("Wi-Fi AP"), _("Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/"), true))
        {
            wifi_toggle_interactive(true, 9000);
        }
        return RG_DIALOG_REDRAW;
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_enable_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    bool enabled = rg_settings_get_boolean(NS_WIFI, SETTING_WIFI_ENABLE, false);
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT || event == RG_DIALOG_ENTER)
    {
        enabled = !enabled;
        rg_settings_set_boolean(NS_WIFI, SETTING_WIFI_ENABLE, enabled);
        wifi_toggle_interactive(enabled, rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, -1));
        return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, enabled ? _("On") : _("Off"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[] = {
            {0, _("Wi-Fi enable"),       "-",  RG_DIALOG_FLAG_NORMAL,  &wifi_enable_cb      },
            {0, _("Wi-Fi profile"),      "-",  RG_DIALOG_FLAG_NORMAL,  &wifi_profile_cb     },
            RG_DIALOG_SEPARATOR,
            {0, _("Wi-Fi access point"), NULL, RG_DIALOG_FLAG_NORMAL,  &wifi_access_point_cb},
            RG_DIALOG_SEPARATOR,
            {0, _("Network"),            "-",  RG_DIALOG_FLAG_MESSAGE, &wifi_status_cb      },
            {0, _("IP address"),         "-",  RG_DIALOG_FLAG_MESSAGE, &wifi_status_cb      },
            RG_DIALOG_END,
        };
        rg_gui_dialog(option->label, options, 0);
    }
    return RG_DIALOG_VOID;
}
#endif

static rg_gui_event_t app_options_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_app_t *app = rg_system_get_app();
        rg_gui_option_t options[16] = {0};
        if (app->handlers.options)
            app->handlers.options(options);
        rg_display_force_redraw();
        rg_gui_dialog(option->label, options, 0);
        return RG_DIALOG_REDRAW;
    }
    return RG_DIALOG_VOID;
}

void rg_gui_options_menu(void)
{
    rg_gui_option_t options[16] = {
        #if RG_SCREEN_BACKLIGHT
        {0, _("Brightness"),    "-", RG_DIALOG_FLAG_NORMAL, &brightness_update_cb},
        #endif
        {0, _("Volume"),        "-", RG_DIALOG_FLAG_NORMAL, &volume_update_cb},
        {0, _("Audio out"),     "-", RG_DIALOG_FLAG_NORMAL, &audio_update_cb},
        RG_DIALOG_END,
    };
    const rg_gui_option_t misc_options[] = {
        {0, _("Font type"),     "-", RG_DIALOG_FLAG_NORMAL, &font_type_cb},
        {0, _("Theme"),         "-", RG_DIALOG_FLAG_NORMAL, &theme_cb},
        {0, _("Show clock"),    "-", RG_DIALOG_FLAG_NORMAL, &show_clock_cb},
        {0, _("Timezone"),      "-", RG_DIALOG_FLAG_NORMAL, &timezone_cb},
        {0, _("Language"),      "-", RG_DIALOG_FLAG_NORMAL, &language_cb},
        #ifdef RG_GPIO_LED // Only show disk LED option if disk LED GPIO pin is defined
        {0, _("LED options"),   NULL, RG_DIALOG_FLAG_NORMAL, &led_indicator_cb},
        #endif
        #ifdef RG_ENABLE_NETWORKING
        {0, _("Wi-Fi options"), NULL, RG_DIALOG_FLAG_NORMAL, &wifi_cb},
        #endif
        {0, _("Launcher options"), NULL, RG_DIALOG_FLAG_NORMAL, &app_options_cb},
        RG_DIALOG_END,
    };
    const rg_gui_option_t game_options[] = {
        {0, _("Scaling"),       "-", RG_DIALOG_FLAG_NORMAL, &scaling_update_cb},
        {0, _("Factor"),        "-", RG_DIALOG_FLAG_HIDDEN, &custom_zoom_cb},
        {0, _("Filter"),        "-", RG_DIALOG_FLAG_NORMAL, &filter_update_cb},
        {0, _("Border"),        "-", RG_DIALOG_FLAG_NORMAL, &border_update_cb},
        {0, _("Speed"),         "-", RG_DIALOG_FLAG_NORMAL, &speedup_update_cb},
        // {0, _("Misc options"),  NULL, RG_DIALOG_FLAG_NORMAL, &misc_options_cb},
        {0, _("Emulator options"), NULL, RG_DIALOG_FLAG_NORMAL, &app_options_cb},
        RG_DIALOG_END,
    };

    const rg_app_t *app = rg_system_get_app();
    if (app->isLauncher)
        memcpy(options + get_dialog_items_count(options), misc_options, sizeof(misc_options));
    else
        memcpy(options + get_dialog_items_count(options), game_options, sizeof(game_options));

    rg_audio_set_mute(true);

    rg_gui_dialog(_("Options"), options, 0);
    rg_settings_commit();

    rg_audio_set_mute(false);
}

void rg_gui_about_menu(void)
{
    const rg_app_t *app = rg_system_get_app();
    bool have_option_btn = rg_input_get_key_mapping(RG_KEY_OPTION);

    // TODO: Add indicator whether or not the build is a release, and if it's official (built by me)
    rg_gui_option_t options[20] = {
        {0, _("Version"), (char *)app->version, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, _("Date"), (char *)app->buildDate, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, _("Target"), (char *)RG_TARGET_NAME, RG_DIALOG_FLAG_MESSAGE, NULL},
        {0, _("Website"), (char *)RG_PROJECT_WEBSITE, RG_DIALOG_FLAG_MESSAGE, NULL},
        RG_DIALOG_SEPARATOR,
        {4, _("Options"), NULL, have_option_btn ? RG_DIALOG_FLAG_HIDDEN : RG_DIALOG_FLAG_NORMAL , NULL},
        // {1, _("View credits", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, _("Debug menu"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3, _("Reset settings"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END,
    };

    if (app->handlers.about)
        app->handlers.about(options + get_dialog_items_count(options));

    while (true)
    {
        switch (rg_gui_dialog(_("About Retro-Go"), options, 4))
        {
            case 1:
                // FIXME: This should probably be a regular dialog so that it's scrollable!
                rg_gui_alert("Credits", RG_PROJECT_CREDITS);
                break;
            case 2:
                rg_gui_debug_menu();
                break;
            case 3:
                if (rg_gui_confirm(_("Reset all settings?"), NULL, false)) {
                    rg_storage_delete(RG_BASE_PATH_CACHE);
                    rg_settings_reset();
                    rg_system_restart();
                    return;
                }
                break;
            case 4:
                rg_gui_options_menu();
                break;
            default:
                return;
        }
    }
}

void rg_gui_debug_menu(void)
{
    char screen_res[20], source_res[20], scaled_res[20];
    char stack_hwm[20], heap_free[20], block_free[20];
    char local_time[32], timezone[32], uptime[20];
    char battery_info[25], frame_time[32];
    char app_name[32], network_str[64];

    const rg_gui_option_t options[] = {
        {0, "Screen res", screen_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Source res", source_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Scaled res", scaled_res,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Stack HWM ", stack_hwm,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Heap free ", heap_free,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Block free", block_free,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "App name  ", app_name,     RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Network   ", network_str,  RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Local time", local_time,   RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Timezone  ", timezone,     RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Uptime    ", uptime,       RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Battery   ", battery_info, RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Blit time ", frame_time,   RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_SEPARATOR,
        {0, "Overclock", "-", RG_DIALOG_FLAG_NORMAL, &overclock_update_cb},
        {1, "Reboot to firmware", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, "Clear cache    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3, "Save screenshot", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {4, "Save trace", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {5, "Cheats    ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {6, "Crash     ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {7, "Log=debug ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
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
        int total = (float)display_stats.busyTime / display_stats.totalFrames / 1000.f;
        int block = (float)display_stats.blockTime / display_stats.totalFrames / 1000.f;
        snprintf(frame_time, 20, "%dms (block: %dms)", total, block);
    }
    else
        snprintf(frame_time, 20, "N/A");
    snprintf(stack_hwm, 20, "%d", stats.freeStackMain);
    snprintf(heap_free, 20, "%d+%d", stats.freeMemoryInt, stats.freeMemoryExt);
    snprintf(block_free, 20, "%d+%d", stats.freeBlockInt, stats.freeBlockExt);
    snprintf(app_name, 32, "%s", rg_system_get_app()->name);
    snprintf(uptime, 20, "%ds", (int)(rg_system_timer() / 1000000));

    rg_battery_t battery;
    if (rg_input_read_battery_raw(&battery))
        snprintf(battery_info, sizeof(battery_info), "%.2f%% | %.2fV", battery.level, battery.volts);
    else
        snprintf(battery_info, sizeof(battery_info), "N/A");

    rg_network_t net = rg_network_get_info();
    if (net.state == RG_NETWORK_DISABLED)
        snprintf(network_str, 64, "%s", "not available");
    else if (net.state == RG_NETWORK_CONNECTED)
        snprintf(network_str, 64, "%s\n%s", net.name, net.ip_addr);
    else if (net.state == RG_NETWORK_CONNECTING)
        snprintf(network_str, 64, "%s\n%s", net.name, "connecting...");
    else if (net.name[0])
        snprintf(network_str, 64, "%s\n%s", net.name, "disconnected");
    else
        snprintf(network_str, 64, "%s", "disconnected");

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
    case 7:
        rg_system_set_log_level(RG_LOG_DEBUG);
        break;
    }
}

static rg_gui_event_t slot_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    rg_emu_slot_t *slot = (rg_emu_slot_t *)option->arg;
    if (event == RG_DIALOG_FOCUS_GAINED)
    {
        rg_image_t *preview = NULL;
        rg_color_t color = C_BLUE;
        size_t margin = 0; // TEXT_RECT("ABC", 0).height;
        size_t border = 3;
        char buffer[100];
        if (slot->is_used)
        {
            preview = rg_surface_load_image_file(slot->preview, 0);
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
        rg_surface_free(preview);
    }
    else if (event == RG_DIALOG_ENTER)
    {
        return RG_DIALOG_SELECT;
    }
    return RG_DIALOG_VOID;
    #undef draw_status
}

int rg_gui_savestate_menu(const char *title, const char *rom_path)
{
    rg_emu_states_t *savestates = rg_emu_get_states(rom_path, 4);
    const rg_gui_option_t choices[] = {
        {(intptr_t)&savestates->slots[0], _("Slot 0"), NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[1], _("Slot 1"), NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[2], _("Slot 2"), NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        {(intptr_t)&savestates->slots[3], _("Slot 3"), NULL, RG_DIALOG_FLAG_NORMAL, &slot_select_cb},
        RG_DIALOG_END
    };

    intptr_t ret = rg_gui_dialog(title, choices, savestates->lastused ? savestates->lastused->id : 0);
    int slot = (ret == RG_DIALOG_CANCELLED) ? -1 : ((rg_emu_slot_t *)ret)->id;
    free(savestates);
    return slot;
}

void rg_gui_game_menu(void)
{
    const char *rom_path = rg_system_get_app()->romPath;
    bool have_option_btn = rg_input_get_key_mapping(RG_KEY_OPTION);
    const rg_gui_option_t choices[] = {
        {1000, _("Save & Continue"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2000, _("Save & Quit"),     NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3001, _("Load game"),       NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {3000, _("Reset"),           NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        #ifdef RG_ENABLE_NETPLAY
        {5000, _("Netplay"),         NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        #endif
        {5500, _("Options"),         NULL, have_option_btn ? RG_DIALOG_FLAG_HIDDEN : RG_DIALOG_FLAG_NORMAL, NULL},
        {6000, _("About"),           NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {7000, _("Quit"),            NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END
    };
    int slot, sel;

    rg_audio_set_mute(true);

    sel = rg_gui_dialog("Retro-Go", choices, 0);

    if (sel == 3000)
    {
        const rg_gui_option_t choices[] = {
            {3002, _("Soft reset"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            {3003, _("Hard reset"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            RG_DIALOG_END
        };
        sel = rg_gui_dialog(_("Reset Emulation?"), choices, 0);
    }

    switch (sel)
    {
        case 1000: if ((slot = rg_gui_savestate_menu(_("Save"), rom_path)) >= 0) rg_emu_save_state(slot); break;
        case 2000: if ((slot = rg_gui_savestate_menu(_("Save"), rom_path)) >= 0 && rg_emu_save_state(slot)) rg_system_exit(); break;
        case 3001: if ((slot = rg_gui_savestate_menu(_("Load"), rom_path)) >= 0) rg_emu_load_state(slot); break;
        case 3002: rg_emu_reset(false); break;
        case 3003: rg_emu_reset(true); break;
    #ifdef RG_ENABLE_NETPLAY
        case 5000: rg_netplay_quick_start(); break;
    #endif
        case 5500: rg_gui_options_menu(); break;
        case 6000: rg_gui_about_menu(); break;
        case 7000: rg_system_exit(); break;
    }

    rg_audio_set_mute(false);
}
