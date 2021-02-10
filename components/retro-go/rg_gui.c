#include <esp_heap_caps.h>
#include <esp_system.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <lupng.h>

#include "bitmaps/image_hourglass.h"
#include "bitmaps/font_basic.h"
#include "rg_system.h"
#include "rg_gui.h"

static uint16_t *overlay_buffer = NULL;

static const dialog_theme_t default_theme = {
    .box_background = C_NAVY,
    .box_header = C_WHITE,
    .box_border = C_DIM_GRAY,
    .item_standard = C_WHITE,
    .item_disabled = C_DIM_GRAY,
};
static dialog_theme_t theme;
static font_info_t font_info;

#define wait_all_keys_released() while (rg_input_key_is_pressed(GAMEPAD_KEY_ANY))

void rg_gui_init(void)
{
    overlay_buffer = (uint16_t *)rg_alloc(RG_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    rg_gui_set_font_size(rg_settings_FontSize_get());
    rg_gui_set_theme(&default_theme);
}

void rg_gui_set_theme(const dialog_theme_t *new_theme)
{
    if (new_theme) {
        memcpy(&theme, new_theme, sizeof(dialog_theme_t));
    }
}

void rg_gui_set_font_size(int points)
{
    font_info.points = RG_MAX(8, RG_MIN(32, points));
    font_info.height = font_info.points;
    font_info.width = 8;
    rg_settings_FontSize_set(points);
}

font_info_t rg_gui_get_font_info(void)
{
    return font_info;
}

int rg_gui_draw_text(int x_pos, int y_pos, int width, const char *text, uint16_t color, uint16_t color_bg)
{
    int glyph_width = font_info.width;
    int glyph_height = font_info.height;
    int text_len = 1;
    int height = 0;
    float scale = (float)glyph_height / 8;

    if (text == NULL || text[0] == 0) {
        text = " ";
    }

    text_len = strlen(text);

    if (width < 1) {
        width = text_len * glyph_width;
    }

    if (width > (RG_SCREEN_WIDTH - x_pos)) {
        width = (RG_SCREEN_WIDTH - x_pos);
    }

    int line_len = width / glyph_width;
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = 0;

        int chunk_len = strlen(buffer);
        int x_offset = 0;

        for (int i = 0; i < line_len; i++)
        {
            const char *glyph = font8x8_basic[(i < chunk_len) ? buffer[i] : ' '];
            for (int y = 0; y < glyph_height; y++)
            {
                int offset = x_offset + (width * y);
                for (int x = 0; x < 8; x++)
                {
                    overlay_buffer[offset + x] = (glyph[(int)(y/scale)] & 1 << x) ? color : color_bg;
                }
            }
            x_offset += glyph_width;
        }

        rg_display_write(x_pos, y_pos + height, width, glyph_height, 0, overlay_buffer);

        height += glyph_height;
        pos += chunk_len;

        if (*(text + pos) == 0 || *(text + pos) == '\n') pos++;
    }

    return height;
}

void rg_gui_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0 || border == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
    {
        overlay_buffer[i] = color;
    }
    rg_display_write(x, y, width, border, 0, overlay_buffer); // T
    rg_display_write(x, y + height - border, width, border, 0, overlay_buffer); // B
    rg_display_write(x, y, border, height, 0, overlay_buffer); // L
    rg_display_write(x + width - border, y, border, height, 0, overlay_buffer); // R
}

void rg_gui_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    if (width == 0 || height == 0)
        return;

    for (int i = 0; i < width * 16; i++)
    {
        overlay_buffer[i] = color;
    }

    int y_pos = y;
    int y_end = y + height;

    while (y_pos < y_end)
    {
        int thickness = (y_end - y_pos >= 16) ? 16 : (y_end - y_pos);
        rg_display_write(x, y_pos, width, thickness, 0, overlay_buffer);
        y_pos += 16;
    }
}

static rg_image_t *new_image(int width, int height)
{
    width = RG_MIN(width, RG_SCREEN_WIDTH);
    height = RG_MIN(height, RG_SCREEN_HEIGHT);
    rg_image_t *img = rg_alloc(sizeof(rg_image_t) + width * height * 2, MEM_SLOW);
    img->width = width;
    img->height = height;
    return img;
}

rg_image_t *rg_gui_load_image_file(const char *file)
{
    if (!file)
        return NULL;

    FILE *fp = rg_fopen(file, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        size_t data_len = RG_MAX(0x80000, ftell(fp));
        void *data = rg_alloc(data_len, MEM_SLOW);
        fseek(fp, 0, SEEK_SET);
        fread(data, data_len, 1, fp);
        rg_fclose(fp);

        rg_image_t *img = rg_gui_load_image(data, data_len);
        free(data);

        return img;
    }

    RG_LOGE("Unable to load image file '%s'!\n", file);
    return NULL;
}

rg_image_t *rg_gui_load_image(const uint8_t *data, size_t data_len)
{
    if (!data || data_len < 16)
        return NULL;

    LuImage *png = luPngReadMem(data, data_len);
    if (png)
    {
        rg_image_t *img = new_image(png->width, png->height);
        uint16_t *ptr = img->data;

        for (int y = 0; y < img->height; ++y) {
            for (int x = 0; x < img->width; ++x) {
                int offset = (y * png->width * 3) + (x * 3);
                int r = (png->data[offset+0] >> 3) & 0x1F;
                int g = (png->data[offset+1] >> 2) & 0x3F;
                int b = (png->data[offset+2] >> 3) & 0x1F;
                *(ptr++) = (r << 11) | (g << 5) | b;
            }
        }

        luImageRelease(png, NULL);
        return img;
    }

    // Not valid PNG, try loading as raw 565
    uint16_t *img_data = (uint16_t *)data;
    size_t img_width = *img_data++;
    size_t img_height = *img_data++;

    if (data_len < (img_width * img_height * 2))
    {
        RG_LOGE("Invalid RAW data!\n");
        return false;
    }

    rg_image_t *img = new_image(img_width, img_height);

    for (int y = 0; y < img->height; ++y)
    {
        memcpy(img->data + (y * img->width), img_data + (y * img_width), img->width * 2);
    }

    return img;
}

void rg_gui_free_image(rg_image_t *img)
{
    free(img);
}

void rg_gui_draw_image(int x, int y, int width, int height, const rg_image_t *img)
{
    if (img && x < RG_SCREEN_WIDTH && y < RG_SCREEN_HEIGHT)
    {
        width = RG_MIN(width > 0 ? width : img->width, RG_SCREEN_WIDTH);
        height = RG_MIN(height > 0 ? height : img->height, RG_SCREEN_HEIGHT);
        rg_display_write(x, y, width, height, img->width * 2, img->data);
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

static int get_dialog_items_count(dialog_choice_t *options)
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

void rg_gui_draw_dialog(const char *header, dialog_choice_t *options, int sel)
{
    int width = header ? strlen(header) : 8;
    int padding = 0;
    int len = 0;
    int row_margin = 1;
    int row_height = font_info.height + row_margin * 2;
    int box_width = 64;
    int box_height = 64;
    int box_padding = 6;

    int options_count = get_dialog_items_count(options);

    char *rows = malloc(options_count * 256);

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].label == NULL) {
            options[i].label = "";
        }
        if (options[i].value[0]) {
            len = strlen(options[i].label);
            padding = (len > padding) ? len : padding;
        }
    }

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].update_cb != NULL) {
            options[i].update_cb(&options[i], RG_DIALOG_INIT);
        }
        if (options[i].value[0]) {
            len = sprintf(rows + i * 256, " %*s: %s ", -padding, options[i].label, options[i].value);
        } else {
            len = sprintf(rows + i * 256, " %s ", options[i].label);
        }
        width = len > width ? len : width;
    }

    if (width > 32) width = 32;

    box_width = (font_info.width * width) + box_padding * 2;
    box_height = (row_height * options_count) + (header ? row_height + 4 : 0) + box_padding * 2;

    int box_x = (RG_SCREEN_WIDTH - box_width) / 2;
    int box_y = (RG_SCREEN_HEIGHT - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;
    int inner_width = box_width - (box_padding * 2);

    if (header)
    {
        int pad = (0.5f * (width - strlen(header)) * font_info.width);
        rg_gui_draw_rect(x, y, box_width - 8, row_height + 4, (row_height + 4 / 2), theme.box_background);
        rg_gui_draw_text(x + pad, y, 0, header, theme.box_header, theme.box_background);
        y += row_height + 4;
    }

    for (int i = 0; i < options_count; i++)
    {
        uint16_t color = (options[i].flags == RG_DIALOG_FLAG_NORMAL) ? theme.item_standard : theme.item_disabled;
        uint16_t fg = (i == sel) ? theme.box_background : color;
        uint16_t bg = (i == sel) ? color : theme.box_background;
        row_height = rg_gui_draw_text(x, y + row_margin, inner_width, rows + i * 256, fg, bg);
        row_height += row_margin * 2;
        rg_gui_draw_rect(x, y, inner_width, row_height, row_margin, bg);
        y += row_height;
    }

    box_height = y - box_y + box_padding;

    rg_gui_draw_rect(box_x, box_y, box_width, box_height, box_padding, theme.box_background);
    rg_gui_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, theme.box_border);

    free(rows);
}

int rg_gui_dialog(const char *header, dialog_choice_t *options, int selected)
{
    int options_count = get_dialog_items_count(options);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;
    int last_key = -1;
    bool select = false;

    rg_gui_draw_dialog(header, options, sel);

    wait_all_keys_released();

    while (1)
    {
        gamepad_state_t joystick = rg_input_read_gamepad();

        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
            }
        }
        else {
            if (joystick.values[GAMEPAD_KEY_UP]) {
                last_key = GAMEPAD_KEY_UP;
                if (--sel < 0) sel = options_count - 1;
            }
            else if (joystick.values[GAMEPAD_KEY_DOWN]) {
                last_key = GAMEPAD_KEY_DOWN;
                if (++sel > options_count - 1) sel = 0;
            }
            else if (joystick.values[GAMEPAD_KEY_B]) {
                last_key = GAMEPAD_KEY_B;
                sel = -1;
                break;
            }
            else if (joystick.values[GAMEPAD_KEY_VOLUME]) {
                last_key = GAMEPAD_KEY_VOLUME;
                sel = -1;
                break;
            }
            else if (joystick.values[GAMEPAD_KEY_MENU]) {
                last_key = GAMEPAD_KEY_MENU;
                sel = -1;
                break;
            }
            if (options[sel].flags != RG_DIALOG_FLAG_DISABLED) {
                select = false;
                if (joystick.values[GAMEPAD_KEY_LEFT]) {
                    last_key = GAMEPAD_KEY_LEFT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_PREV);
                        sel_old = -1;
                    }
                }
                else if (joystick.values[GAMEPAD_KEY_RIGHT]) {
                    last_key = GAMEPAD_KEY_RIGHT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_NEXT);
                        sel_old = -1;
                    }
                }
                else if (joystick.values[GAMEPAD_KEY_A]) {
                    last_key = GAMEPAD_KEY_A;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], RG_DIALOG_ENTER);
                        sel_old = -1;
                    } else {
                        select = true;
                    }
                }

                if (select) {
                    break;
                }
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

    rg_display_force_refresh();

    return sel < 0 ? sel : options[sel].id;
}

bool rg_gui_confirm(const char *title, const char *message, bool yes_selected)
{
    dialog_choice_t choices[] = {
        {0, message, "", -1, NULL},
        {0, "", "", -1, NULL},
        {1, "Yes", "", 1, NULL},
        {0, "No ", "", 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    return rg_gui_dialog(title, message ? choices : choices + 1, yes_selected ? -2 : -1) == 1;
}

void rg_gui_alert(const char *title, const char *message)
{
    dialog_choice_t choices[] = {
        {0, message, "", -1, NULL},
        {0, "", "", -1, NULL},
        {1, "OK", "", 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    rg_gui_dialog(title, message ? choices : choices + 1, -1);
}

static bool volume_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    int8_t level = rg_audio_volume_get();
    int8_t min = RG_AUDIO_VOL_MIN;
    int8_t max = RG_AUDIO_VOL_MAX;

    if (event == RG_DIALOG_PREV && level > min) {
        rg_audio_volume_set(--level);
    }

    if (event == RG_DIALOG_NEXT && level < max) {
        rg_audio_volume_set(++level);
    }

    sprintf(option->value, "%d/%d", level, max);
    return event == RG_DIALOG_ENTER;
}

static bool brightness_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    int8_t level = rg_display_get_backlight();
    int8_t max = RG_BACKLIGHT_LEVEL_COUNT - 1;

    if (event == RG_DIALOG_PREV && level > 0) {
        rg_display_set_backlight(--level);
    }

    if (event == RG_DIALOG_NEXT && level < max) {
        rg_display_set_backlight(++level);
    }

    sprintf(option->value, "%d/%d", level + 1, max + 1);
    return event == RG_DIALOG_ENTER;
}

static bool audio_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    int8_t sink = rg_audio_get_sink();

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        sink = (sink == RG_AUDIO_SINK_SPEAKER) ? RG_AUDIO_SINK_DAC : RG_AUDIO_SINK_SPEAKER;
        rg_audio_set_sink(sink);
    }

    strcpy(option->value, (sink == RG_AUDIO_SINK_DAC) ? "Ext DAC" : "Speaker");
    return event == RG_DIALOG_ENTER;
}

static bool filter_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    int8_t max = RG_DISPLAY_FILTER_COUNT - 1;
    int8_t mode = rg_display_get_filter_mode();
    int8_t prev = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode = max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0; // max;

    if (mode != prev)
    {
        rg_display_set_filter_mode(mode);
    }

    if (mode == RG_DISPLAY_FILTER_OFF)      strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_FILTER_LINEAR_X) strcpy(option->value, "Horiz");
    if (mode == RG_DISPLAY_FILTER_LINEAR_Y) strcpy(option->value, "Vert ");
    if (mode == RG_DISPLAY_FILTER_BILINEAR) strcpy(option->value, "Both ");

    return event == RG_DIALOG_ENTER;
}

static bool scaling_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    int8_t max = RG_DISPLAY_SCALING_COUNT - 1;
    int8_t mode = rg_display_get_scaling_mode();
    int8_t prev = mode;

    if (event == RG_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == RG_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev) {
        rg_display_set_scaling_mode(mode);
    }

    if (mode == RG_DISPLAY_SCALING_OFF)  strcpy(option->value, "Off  ");
    if (mode == RG_DISPLAY_SCALING_FIT)  strcpy(option->value, "Fit ");
    if (mode == RG_DISPLAY_SCALING_FILL) strcpy(option->value, "Full ");

    return event == RG_DIALOG_ENTER;
}

static bool speedup_update_cb(dialog_choice_t *option, dialog_event_t event)
{
    rg_app_desc_t *app = rg_system_get_app();
    if (event == RG_DIALOG_PREV && --app->speedupEnabled < 0) app->speedupEnabled = 2;
    if (event == RG_DIALOG_NEXT && ++app->speedupEnabled > 2) app->speedupEnabled = 0;

    sprintf(option->value, "%dx", app->speedupEnabled + 1);
    return event == RG_DIALOG_ENTER;
}

int rg_gui_settings_menu(dialog_choice_t *extra_options)
{
    dialog_choice_t options[12] = {
        {0, "Brightness", "50%",  1, &brightness_update_cb},
        {1, "Volume    ", "50%",  1, &volume_update_cb},
        {2, "Audio out ", "Speaker", 1, &audio_update_cb},
        RG_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(options + options_count, extra_options, (extra_options_count + 1) * sizeof(dialog_choice_t));
    }

    int ret = rg_gui_dialog("Options", options, 0);

    rg_settings_commit();

    return ret;
}

static void draw_game_status_bar(runtime_stats_t stats)
{
    int width = RG_SCREEN_WIDTH, height = 16;
    int pad_text = (height - font_info.height) / 2;
    char header[41] = {0};
    char footer[41] = {0};

    const rg_app_desc_t *app = rg_system_get_app();

    snprintf(header, 40, "SPEED: %.0f%% (%.0f/%.0f) / BUSY: %.0f%%",
        round(stats.totalFPS / app->refreshRate * 100.f),
        round(stats.totalFPS - stats.skippedFPS),
        round(stats.totalFPS),
        round(stats.busyPercent));

    if (app->romPath)
        snprintf(footer, 40, "%s", app->romPath + strlen(RG_BASE_PATH_ROMS));

    rg_gui_draw_fill_rect(0, 0, width, height, C_BLACK);
    rg_gui_draw_fill_rect(0, RG_SCREEN_HEIGHT - height, width, height, C_BLACK);
    rg_gui_draw_text(0, pad_text, width, header, C_LIGHT_GRAY, C_BLACK);
    rg_gui_draw_text(0, RG_SCREEN_HEIGHT - height + pad_text, width, footer, C_LIGHT_GRAY, C_BLACK);
    rg_gui_draw_battery(width - 26, 3);
}

int rg_gui_game_settings_menu(dialog_choice_t *extra_options)
{
    dialog_choice_t options[12] = {
        {10, "Scaling", "Full", 1, &scaling_update_cb},
        {12, "Filtering", "None", 1, &filter_update_cb}, // Interpolation
        {13, "Speed", "1x", 1, &speedup_update_cb},
        RG_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(options + options_count, extra_options, (extra_options_count + 1) * sizeof(dialog_choice_t));
    }

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = rg_system_get_stats();

    rg_audio_mute(true);
    wait_all_keys_released();
    draw_game_status_bar(stats);

    int r = rg_gui_settings_menu(options);

    rg_audio_mute(false);

    return r;
}

int rg_gui_game_debug_menu(void)
{
    dialog_choice_t options[12] = {
        {10, "Screen Res", "A", 1, NULL},
        {10, "Game Res", "B", 1, NULL},
        {10, "Scaled Res", "C", 1, NULL},
        {10, "Cheats", "C", 1, NULL},
        {10, "Rewind", "C", 1, NULL},
        {10, "Registers", "C", 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    // runtime_stats_t stats = rg_system_get_stats();

    wait_all_keys_released();

    return rg_gui_dialog("Debugging", options, 0);
}

int rg_gui_game_menu(void)
{
    dialog_choice_t choices[] = {
        // {0, "Continue", "",  1, NULL},
        {10, "Save & Continue", "",  1, NULL},
        {20, "Save & Quit", "", 1, NULL},
        {30, "Reload", "", 1, NULL},
        #ifdef ENABLE_NETPLAY
        {40, "Netplay", "", 1, NULL},
        #endif
        // {50, "Tools", "", 1, NULL},
        {100, "Quit", "", 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = rg_system_get_stats();

    rg_audio_mute(true);
    wait_all_keys_released();
    draw_game_status_bar(stats);

    int r = rg_gui_dialog("Retro-Go", choices, 0);

    switch (r)
    {
        case 10: rg_emu_save_state(0); break;
        case 20: rg_emu_save_state(0); rg_system_switch_app(RG_APP_LAUNCHER); break;
        case 30: rg_emu_load_state(0); break; // esp_restart();
    #ifdef ENABLE_NETPLAY
        case 40: rg_netplay_quick_start(); break;
    #endif
        case 50: rg_gui_game_debug_menu(); break;
        case 100: rg_system_switch_app(RG_APP_LAUNCHER); break;
    }

    rg_audio_mute(false);

    return r;
}
