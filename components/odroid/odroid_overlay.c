#include <esp_heap_caps.h>
#include <esp_system.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "bitmaps/font_basic.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

static uint16_t *overlay_buffer = NULL;
static short dialog_open_depth = 0;
static short font_size = 8;

void odroid_overlay_init()
{
    overlay_buffer = (uint16_t *)rg_alloc(ODROID_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    odroid_overlay_set_font_size(odroid_settings_FontSize_get());
}

void odroid_overlay_set_font_size(int size)
{
    font_size = MAX(8, MIN(32, size));
    odroid_settings_FontSize_set(font_size);
}

int odroid_overlay_get_font_size()
{
    return font_size;
}

int odroid_overlay_get_font_width()
{
    return 8;
}

int odroid_overlay_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = odroid_overlay_get_font_size();
    int font_width = odroid_overlay_get_font_width();
    int x_offset = 0;
    float scale = (float)font_height / 8;
    int text_len = strlen(text);

    for (int i = 0; i < (width / font_width); i++)
    {
        const char *glyph = font8x8_basic[(i < text_len) ? text[i] : ' '];
        for (int y = 0; y < font_height; y++)
        {
            int offset = x_offset + (width * y);
            for (int x = 0; x < 8; x++)
            {
                overlay_buffer[offset + x] = (glyph[(int)(y/scale)] & 1 << x) ? color : color_bg;
            }
        }
        x_offset += font_width;
    }

    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int odroid_overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0) {
        text = " ";
    }

    text_len = strlen(text);

    if (width < 1) {
        width = text_len * odroid_overlay_get_font_width();
    }

    if (width > (ODROID_SCREEN_WIDTH - x_pos)) {
        width = (ODROID_SCREEN_WIDTH - x_pos);
    }

    int line_len = width / odroid_overlay_get_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = 0;
        height += odroid_overlay_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n') pos++;
    }

    return height;
}

void odroid_overlay_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0 || border == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
    {
        overlay_buffer[i] = color;
    }
    odroid_display_write(x, y, width, border, overlay_buffer); // T
    odroid_display_write(x, y + height - border, width, border, overlay_buffer); // B
    odroid_display_write(x, y, border, height, overlay_buffer); // L
    odroid_display_write(x + width - border, y, border, height, overlay_buffer); // R
}

void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
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
        odroid_display_write(x, y_pos, width, thickness, overlay_buffer);
        y_pos += 16;
    }
}

void odroid_overlay_draw_battery(int x_pos, int y_pos)
{
    uint16_t percentage = odroid_input_read_battery().percentage;
    uint16_t color_fill = C_FOREST_GREEN;
    uint16_t color_border = C_SILVER;
    uint16_t color_empty = C_BLACK;
    uint16_t width_fill = 20.f / 100 * percentage;
    uint16_t width_empty = 20 - width_fill;

    if (percentage < 20)
        color_fill = C_RED;
    else if (percentage < 40)
        color_fill = C_ORANGE;

    odroid_overlay_draw_rect(x_pos, y_pos, 22, 10, 1, color_border);
    odroid_overlay_draw_rect(x_pos + 22, y_pos + 2, 2, 6, 1, color_border);
    odroid_overlay_draw_fill_rect(x_pos + 1, y_pos + 1, width_fill, 8, color_fill);
    odroid_overlay_draw_fill_rect(x_pos + 1 + width_fill, y_pos + 1, width_empty, 8, color_empty);
}

static int get_dialog_items_count(odroid_dialog_choice_t *options)
{
    odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;

    if (options == NULL)
        return 0;

    for (int i = 0; i < 16; i++)
    {
        // if (memcmp(&last, options + i, sizeof(last))) {
        if (options[i].id == last.id && options[i].enabled == last.enabled) {
            return i;
        }
    }
    return 0;
}

void odroid_overlay_draw_dialog(const char *header, odroid_dialog_choice_t *options, int sel)
{
    int width = header ? strlen(header) : 8;
    int padding = 0;
    int len = 0;

    int row_margin = 1;
    int row_height = odroid_overlay_get_font_size() + row_margin * 2;

    int box_width = 64;
    int box_height = 64;
    int box_padding = 6;
    int box_color = C_NAVY;
    int box_border_color = C_DIM_GRAY;
    int box_text_color = C_WHITE;

    int options_count = get_dialog_items_count(options);

    char *rows = malloc(options_count * 256);

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0]) {
            len = strlen(options[i].label);
            padding = (len > padding) ? len : padding;
        }
    }

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].update_cb != NULL) {
            options[i].update_cb(&options[i], ODROID_DIALOG_INIT);
        }
        if (options[i].value[0]) {
            len = sprintf(rows + i * 256, " %*s: %s ", -padding, options[i].label, options[i].value);
        } else {
            len = sprintf(rows + i * 256, " %s ", options[i].label);
        }
        width = len > width ? len : width;
    }

    if (width > 32) width = 32;

    box_width = (odroid_overlay_get_font_width() * width) + box_padding * 2;
    box_height = (row_height * options_count) + (header ? row_height + 4 : 0) + box_padding * 2;

    int box_x = (ODROID_SCREEN_WIDTH - box_width) / 2;
    int box_y = (ODROID_SCREEN_HEIGHT - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;

    if (header)
    {
        int pad = (0.5f * (width - strlen(header)) * odroid_overlay_get_font_width());
        odroid_overlay_draw_rect(x, y, box_width - 8, row_height + 4, (row_height + 4 / 2), box_color);
        odroid_overlay_draw_text(x + pad, y, 0, header, box_text_color, box_color);
        y += row_height + 4;
    }

    uint16_t fg, bg, color, inner_width = box_width - (box_padding * 2);
    for (int i = 0; i < options_count; i++)
    {
        color = options[i].enabled == 1 ? box_text_color : C_GRAY;
        fg = (i == sel) ? box_color : color;
        bg = (i == sel) ? color : box_color;
        row_height = odroid_overlay_draw_text(x, y + row_margin, inner_width, rows + i * 256, fg, bg);
        row_height += row_margin * 2;
        odroid_overlay_draw_rect(x, y, inner_width, row_height, row_margin, bg);
        y += row_height;
    }

    box_height = y - box_y + box_padding;

    odroid_overlay_draw_rect(box_x, box_y, box_width, box_height, box_padding, box_color);
    odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, box_border_color);

    free(rows);
}

int odroid_overlay_dialog(const char *header, odroid_dialog_choice_t *options, int selected)
{
    int options_count = get_dialog_items_count(options);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;
    int last_key = -1;
    bool select = false;
    odroid_gamepad_state_t joystick;

    dialog_open_depth++;

    odroid_overlay_draw_dialog(header, options, sel);

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY));

    while (1)
    {
        odroid_input_read_gamepad(&joystick);
        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
            }
        }
        else {
            if (joystick.values[ODROID_INPUT_UP]) {
                last_key = ODROID_INPUT_UP;
                if (--sel < 0) sel = options_count - 1;
            }
            else if (joystick.values[ODROID_INPUT_DOWN]) {
                last_key = ODROID_INPUT_DOWN;
                if (++sel > options_count - 1) sel = 0;
            }
            else if (joystick.values[ODROID_INPUT_B]) {
                last_key = ODROID_INPUT_B;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME]) {
                last_key = ODROID_INPUT_VOLUME;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_MENU]) {
                last_key = ODROID_INPUT_MENU;
                sel = -1;
                break;
            }
            if (options[sel].enabled) {
                select = false;
                if (joystick.values[ODROID_INPUT_LEFT]) {
                    last_key = ODROID_INPUT_LEFT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_PREV);
                        sel_old = -1;
                    }
                }
                else if (joystick.values[ODROID_INPUT_RIGHT]) {
                    last_key = ODROID_INPUT_RIGHT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_NEXT);
                        sel_old = -1;
                    }
                }
                else if (joystick.values[ODROID_INPUT_A]) {
                    last_key = ODROID_INPUT_A;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_ENTER);
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
            int dir = sel - sel_old;
            while (options[sel].enabled == -1 && sel_old != sel)
            {
                sel = (sel + dir) % options_count;
            }
            odroid_overlay_draw_dialog(header, options, sel);
            sel_old = sel;
        }

        usleep(20 * 1000UL);
    }

    odroid_input_wait_for_key(last_key, false);

    odroid_display_force_refresh();

    dialog_open_depth--;

    return sel < 0 ? sel : options[sel].id;
}

int odroid_overlay_confirm(const char *text, bool yes_selected)
{
    odroid_dialog_choice_t choices[] = {
        {1, "Yes", "", 1, NULL},
        {0, "No ", "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };
    return odroid_overlay_dialog(text, choices, yes_selected ? 0 : 1);
}

void odroid_overlay_alert(const char *text)
{
    odroid_dialog_choice_t choices[] = {
        {1, "OK", "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };
    odroid_overlay_dialog(text, choices, 0);
}

bool odroid_overlay_dialog_is_open(void)
{
    return dialog_open_depth > 0;
}

static bool volume_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int8_t level = odroid_audio_volume_get();
    int8_t min = ODROID_AUDIO_VOLUME_MIN;
    int8_t max = ODROID_AUDIO_VOLUME_MAX;

    if (event == ODROID_DIALOG_PREV && level > min) {
        odroid_audio_volume_set(--level);
    }

    if (event == ODROID_DIALOG_NEXT && level < max) {
        odroid_audio_volume_set(++level);
    }

    sprintf(option->value, "%d/%d", level, max);
    return event == ODROID_DIALOG_ENTER;
}

static bool brightness_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int8_t level = odroid_display_get_backlight();
    int8_t max = ODROID_BACKLIGHT_LEVEL_COUNT - 1;

    if (event == ODROID_DIALOG_PREV && level > 0) {
        odroid_display_set_backlight(--level);
    }

    if (event == ODROID_DIALOG_NEXT && level < max) {
        odroid_display_set_backlight(++level);
    }

    sprintf(option->value, "%d/%d", level + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

static bool audio_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int8_t sink = odroid_audio_get_sink();

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        sink = (sink == ODROID_AUDIO_SINK_SPEAKER) ? ODROID_AUDIO_SINK_DAC : ODROID_AUDIO_SINK_SPEAKER;
        odroid_audio_set_sink(sink);
    }

    strcpy(option->value, (sink == ODROID_AUDIO_SINK_DAC) ? "Ext DAC" : "Speaker");
    return event == ODROID_DIALOG_ENTER;
}

static bool filter_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int8_t max = ODROID_DISPLAY_FILTER_COUNT - 1;
    int8_t mode = odroid_display_get_filter_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0) mode = max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0; // max;

    if (mode != prev)
    {
        odroid_display_set_filter_mode(mode);
    }

    if (mode == ODROID_DISPLAY_FILTER_OFF)      strcpy(option->value, "Off  ");
    if (mode == ODROID_DISPLAY_FILTER_LINEAR_X) strcpy(option->value, "Horiz");
    if (mode == ODROID_DISPLAY_FILTER_LINEAR_Y) strcpy(option->value, "Vert ");
    if (mode == ODROID_DISPLAY_FILTER_BILINEAR) strcpy(option->value, "Both ");

    return event == ODROID_DIALOG_ENTER;
}

static bool scaling_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int8_t max = ODROID_DISPLAY_SCALING_COUNT - 1;
    int8_t mode = odroid_display_get_scaling_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev) {
        odroid_display_set_scaling_mode(mode);
    }

    if (mode == ODROID_DISPLAY_SCALING_OFF)  strcpy(option->value, "Off  ");
    if (mode == ODROID_DISPLAY_SCALING_FIT)  strcpy(option->value, "Fit ");
    if (mode == ODROID_DISPLAY_SCALING_FILL) strcpy(option->value, "Full ");

    return event == ODROID_DIALOG_ENTER;
}

bool speedup_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    rg_app_desc_t *app = odroid_system_get_app();
    if (event == ODROID_DIALOG_PREV && --app->speedupEnabled < 0) app->speedupEnabled = 2;
    if (event == ODROID_DIALOG_NEXT && ++app->speedupEnabled > 2) app->speedupEnabled = 0;

    sprintf(option->value, "%dx", app->speedupEnabled + 1);
    return event == ODROID_DIALOG_ENTER;
}

int odroid_overlay_settings_menu(odroid_dialog_choice_t *extra_options)
{
    odroid_dialog_choice_t options[12] = {
        {0, "Brightness", "50%",  1, &brightness_update_cb},
        {1, "Volume    ", "50%",  1, &volume_update_cb},
        {2, "Audio out ", "Spkr", 1, &audio_update_cb},
        ODROID_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(options + options_count, extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    int ret = odroid_overlay_dialog("Options", options, 0);

    odroid_settings_commit();

    return ret;
}

static void draw_game_status_bar(runtime_stats_t stats)
{
    int width = ODROID_SCREEN_WIDTH, height = 16;
    int pad_text = (height - odroid_overlay_get_font_size()) / 2;
    char bottom[40], header[40];

    const char *romPath = odroid_system_get_app()->romPath;

    snprintf(header, 40, "FPS: %.0f (%.0f) / BUSY: %.0f%%",
        round(stats.totalFPS), round(stats.skippedFPS), round(stats.busyPercent));
    snprintf(bottom, 40, "%s", romPath ? (romPath + strlen(ODROID_BASE_PATH_ROMS)) : "N/A");

    odroid_overlay_draw_fill_rect(0, 0, width, height, C_BLACK);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - height, width, height, C_BLACK);
    odroid_overlay_draw_text(0, pad_text, width, header, C_LIGHT_GRAY, C_BLACK);
    odroid_overlay_draw_text(0, ODROID_SCREEN_HEIGHT - height + pad_text, width, bottom, C_LIGHT_GRAY, C_BLACK);
    odroid_overlay_draw_battery(width - 26, 3);
}

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    odroid_dialog_choice_t options[12] = {
        {10, "Scaling", "Full", 1, &scaling_update_cb},
        {12, "Filtering", "None", 1, &filter_update_cb}, // Interpolation
        {13, "Speed", "1x", 1, &speedup_update_cb},
        ODROID_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(options + options_count, extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = odroid_system_get_stats();

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY));
    draw_game_status_bar(stats);

    int r = odroid_overlay_settings_menu(options);

    odroid_audio_mute(false);

    return r;
}

int odroid_overlay_game_debug_menu(void)
{
    odroid_dialog_choice_t options[12] = {
        {10, "Screen Res", "A", 1, NULL},
        {10, "Game Res", "B", 1, NULL},
        {10, "Scaled Res", "C", 1, NULL},
        {10, "Cheats", "C", 1, NULL},
        {10, "Rewind", "C", 1, NULL},
        {10, "Registers", "C", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY));
    return odroid_overlay_dialog("Debugging", options, 0);
}

int odroid_overlay_game_menu()
{
    odroid_dialog_choice_t choices[] = {
        // {0, "Continue", "",  1, NULL},
        {10, "Save & Continue", "",  1, NULL},
        {20, "Save & Quit", "", 1, NULL},
        {30, "Reload", "", 1, NULL},
        #ifdef ENABLE_NETPLAY
        {40, "Netplay", "", 1, NULL},
        #else
        // {40, "Netplay", "", 0, NULL},
        #endif
        {50, "Tools", "", 1, NULL},
        {100, "Quit", "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = odroid_system_get_stats();

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY));
    draw_game_status_bar(stats);

    int r = odroid_overlay_dialog("Retro-Go", choices, 0);

    switch (r)
    {
        case 10: odroid_system_emu_save_state(0); break;
        case 20: odroid_system_emu_save_state(0); odroid_system_switch_app(0); break;
        case 30: odroid_system_emu_load_state(0); break; // esp_restart();
        case 40: odroid_netplay_quick_start(); break;
        case 50: odroid_overlay_game_debug_menu(); break;
        case 100: odroid_system_switch_app(0); break;
    }

    odroid_audio_mute(false);

    return r;
}
