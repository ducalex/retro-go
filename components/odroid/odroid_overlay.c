#include "odroid_overlay.h"
#include "odroid_font8x8.h"
#include "odroid_system.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

static uint16_t *overlay_buffer = NULL;
static const char *NVS_KEY_FONTSIZE = "FontSize";

short ODROID_FONT_WIDTH = 8;
short ODROID_FONT_HEIGHT = 8;

static void wait_all_keys_released()
{
    odroid_gamepad_state joystick;
    bool pressed = false;
    do {
        pressed = false;
        odroid_input_gamepad_read(&joystick);
        for (int i = 0; i < ODROID_INPUT_MAX; i++) {
            pressed = pressed || joystick.values[i];
        }
    } while (pressed);
}

void odroid_overlay_init()
{
	if (!overlay_buffer) {
        overlay_buffer = (uint16_t *)heap_caps_calloc_prefer(2, 320 * 16, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT);
    }
    odroid_overlay_set_font_size(odroid_settings_int32_get(NVS_KEY_FONTSIZE, 1));
}

void odroid_overlay_set_font_size(int factor)
{
    factor = factor < 1 ? 1 : factor;
    factor = factor > 3 ? 3 : factor;
    ODROID_FONT_HEIGHT = 8 * factor;
    if (factor != odroid_settings_int32_get(NVS_KEY_FONTSIZE, 0)) {
        odroid_settings_int32_set(NVS_KEY_FONTSIZE, factor);
    }
}

void odroid_overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, char *text, uint16_t color, uint16_t color_bg)
{
    int x_offset = 0;
    int multi = ODROID_FONT_HEIGHT / 8;
    int text_len = strlen(text);

    if (width < 1) {
        width = text_len * ODROID_FONT_WIDTH;
    }

    for (int i = 0; i < (width / ODROID_FONT_WIDTH); i++) {
        char *bitmap = font8x8_basic[(i < text_len) ? text[i] : ' '];
        for (int y = 0; y < 8; y++) {
            int offset = x_offset + (width * (y * multi));
            for (int x = 0; x < 8; x++) {
                uint16_t pixel = (bitmap[y] & 1 << x) ? color : color_bg;
                overlay_buffer[offset + x] = pixel;
                if (multi > 1) {
                    overlay_buffer[offset + x + width] = pixel;
                }
            }
        }
        x_offset += ODROID_FONT_WIDTH;
    }

    ili9341_write_frame_rectangleLE(x_pos, y_pos, width, ODROID_FONT_HEIGHT, overlay_buffer);
}

void odroid_overlay_draw_battery(int x_pos, int y_pos)
{
    odroid_battery_state battery_state;
    odroid_input_battery_level_read(&battery_state);

    uint16_t percentage = battery_state.percentage;
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

void odroid_overlay_draw_dialog(char *header, odroid_dialog_choice_t *options, int options_count, int sel)
{
    int width = header ? strlen(header) : 8;
    int padding = 0;
    int len = 0;

    int box_color = C_NAVY;
    int box_border_color = C_DIM_GRAY;
    int box_shadow_color = C_BLACK;
    int box_text_color = C_WHITE;

    char row_format[16] = " %s ";
    char row_format_kv[16] = " %s: %s ";
    char rows[16][32];

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0]) {
            len = strlen(options[i].label);
            padding = (len > padding) ? len : padding;
        }
    }

    sprintf(row_format_kv, " %%-%ds: %%s ", padding);

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].update_cb != NULL) {
            options[i].update_cb(&options[i], ODROID_DIALOG_INIT);
        }
        if (options[i].value[0]) {
            len = sprintf(rows[i], row_format_kv, options[i].label, options[i].value);
        } else {
            len = sprintf(rows[i], row_format, options[i].label);
        }
        width = len > width ? len : width;
    }

    int box_width = (ODROID_FONT_WIDTH * width) + 12;
    int box_height = (ODROID_FONT_HEIGHT * options_count) + 12;

    if (header)
    {
        box_height += ODROID_FONT_HEIGHT + 4;
    }

    int x = (320 - box_width) / 2;
    int y = (240 - box_height) / 2;

    // odroid_overlay_draw_fill_rect(x + 5, y + box_height + 1, box_width, 4, box_shadow_color); // Bottom
    // odroid_overlay_draw_fill_rect(x + box_width + 1, y + 5, 4, box_height, box_shadow_color); // Rright

    odroid_overlay_draw_rect(x, y, box_width, box_height, 6, box_color);
    odroid_overlay_draw_rect(x - 1, y - 1, box_width + 2, box_height + 2, 1, box_border_color);

    x += 6;
    y += 6;

    if (header)
    {
        int pad = (0.5f * (width - strlen(header)) * ODROID_FONT_WIDTH);
        odroid_overlay_draw_rect(x, y, box_width - 8, ODROID_FONT_HEIGHT + 4, (ODROID_FONT_HEIGHT + 4 / 2), box_color);
        odroid_overlay_draw_text(x + pad, y, 0, header, box_text_color, box_color);
        y += ODROID_FONT_HEIGHT + 4;
    }

    uint16_t fg, bg, color;
    for (int i = 0; i < options_count; i++)
    {
        color = options[i].enabled ? box_text_color : C_GRAY;
        fg = (i == sel) ? box_color : color;
        bg = (i == sel) ? color : box_color;
        odroid_overlay_draw_text(x, y + i * ODROID_FONT_HEIGHT, width * ODROID_FONT_WIDTH, rows[i], fg, bg);
    }
}

int odroid_overlay_dialog(char *header, odroid_dialog_choice_t *options, int options_count, int selected)
{
    int sel = selected;
    int sel_old = selected;
    int last_key = -1;
    bool select = false;
    odroid_gamepad_state joystick;

    odroid_overlay_draw_dialog(header, options, options_count, sel);

    wait_all_keys_released();

    while (1)
    {
        odroid_input_gamepad_read(&joystick);
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
            odroid_overlay_draw_dialog(header, options, options_count, sel);
            sel_old = sel;
        }

        usleep(20 * 1000UL);
    }

    odroid_input_wait_for_key(last_key, false);

    forceVideoRefresh = true;

    return sel < 0 ? sel : options[sel].id;
}

int odroid_overlay_confirm(char *text, bool yes_selected)
{
    odroid_dialog_choice_t choices[] = {
        {1, "Yes", "", 1, NULL},
        {0, "No ", "", 1, NULL},
    };
    return odroid_overlay_dialog(text, choices, 2, yes_selected ? 1 : 0);
}

void odroid_overlay_alert(char *text)
{
    odroid_dialog_choice_t choices[] = {
        {1, "OK", "", 1, NULL},
    };
    odroid_overlay_dialog(text, choices, 1, 0);
}

void odroid_overlay_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
    {
        overlay_buffer[i] = color;
    }
    ili9341_write_frame_rectangleLE(x, y, width, border, overlay_buffer); // T
    ili9341_write_frame_rectangleLE(x, y + height - border, width, border, overlay_buffer); // B
    ili9341_write_frame_rectangleLE(x, y, border, height, overlay_buffer); // L
    ili9341_write_frame_rectangleLE(x + width - border, y, border, height, overlay_buffer); // R
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
        ili9341_write_frame_rectangleLE(x, y_pos, width, thickness, overlay_buffer);
        y_pos += 16;
    }
}

static bool audio_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int sink = odroid_settings_AudioSink_get();

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        sink = (sink == ODROID_AUDIO_SINK_SPEAKER) ? ODROID_AUDIO_SINK_DAC : ODROID_AUDIO_SINK_SPEAKER;
        odroid_settings_AudioSink_set(sink);
        odroid_audio_set_sink(sink);
    }

    strcpy(option->value, (sink == ODROID_AUDIO_SINK_DAC) ? "Ext DAC" : "Speaker");
    return event == ODROID_DIALOG_ENTER;
}

static bool display_mode_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV && displayUpdateMode > 0) {
        odroid_settings_DisplayUpdateMode_set(--displayUpdateMode);
        forceVideoRefresh = true;
    }

    if (event == ODROID_DIALOG_NEXT && displayUpdateMode < ODROID_DISPLAY_UPDATE_COUNT - 1) {
        odroid_settings_DisplayUpdateMode_set(++displayUpdateMode);
        forceVideoRefresh = true;
    }

    if (displayUpdateMode == ODROID_DISPLAY_UPDATE_AUTO)    strcpy(option->value, "Auto");
    if (displayUpdateMode == ODROID_DISPLAY_UPDATE_FULL)    strcpy(option->value, "Full");
    if (displayUpdateMode == ODROID_DISPLAY_UPDATE_PARTIAL) strcpy(option->value, "Partial");

    return event == ODROID_DIALOG_ENTER;
}

static bool volume_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int level = odroid_settings_Volume_get();
    int max = ODROID_VOLUME_LEVEL_COUNT - 1;

    if (event == ODROID_DIALOG_PREV && level > 0) {
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
    int level = odroid_settings_Backlight_get();
    int max = ODROID_BACKLIGHT_LEVEL_COUNT - 1;

    if (event == ODROID_DIALOG_PREV && level > 0) {
        odroid_display_backlight_set(--level);
    }

    if (event == ODROID_DIALOG_NEXT && level < max) {
        odroid_display_backlight_set(++level);
    }

    sprintf(option->value, "%d/%d", level + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

static bool scaling_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    uint8_t max = 2;

    if (event == ODROID_DIALOG_PREV && scalingMode > 0) {
        odroid_settings_Scaling_set(--scalingMode);
        forceVideoRefresh = true;
    }

    if (event == ODROID_DIALOG_NEXT && scalingMode < max) {
        odroid_settings_Scaling_set(++scalingMode);
        forceVideoRefresh = true;
    }

    if (scalingMode == 0) strcpy(option->value, "Off  ");
    if (scalingMode == 1) strcpy(option->value, "Scale");
    if (scalingMode == 2) strcpy(option->value, "Full ");

    return event == ODROID_DIALOG_ENTER;
}

bool speedup_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV && speedupEnabled > 0) --speedupEnabled;
    if (event == ODROID_DIALOG_NEXT && speedupEnabled < 2) ++speedupEnabled;

    sprintf(option->value, "%dx", speedupEnabled + 1);
    return event == ODROID_DIALOG_ENTER;
}

int odroid_overlay_settings_menu(odroid_dialog_choice_t *extra_options, int extra_options_count)
{
    odroid_audio_clear_buffer();
    odroid_display_lock();
    wait_all_keys_released();

    odroid_dialog_choice_t options[12] = {
        {0, "Brightness", "50%",  1, &brightness_update_cb},
        {1, "Volume    ", "50%",  1, &volume_update_cb},
        {2, "Audio out ", "Spkr", 1, &audio_update_cb},
    };
    int options_count = 3;

    if (extra_options_count > 0) {
        memcpy(options + options_count, extra_options, extra_options_count * sizeof(odroid_dialog_choice_t));
        options_count += extra_options_count;
    }

    int r = odroid_overlay_dialog("Options", options, options_count, 0);
    odroid_display_unlock();

    return r;
}

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options, int extra_options_count)
{
    odroid_dialog_choice_t options[12] = {
        {10, "Scaling", "Full", 1, &scaling_update_cb},
        {12, "Filter", "Nearest", 1, NULL}, // Interpolation
        {11, "Update Mode", "Auto", 1, &display_mode_update_cb},
        {13, "Speed", "1x", 1, &speedup_update_cb},
    };
    int options_count = 4;

    if (extra_options_count > 0) {
        memcpy(options + options_count, extra_options, extra_options_count * sizeof(odroid_dialog_choice_t));
        options_count += extra_options_count;
    }

    return odroid_overlay_settings_menu(options, options_count);
}

// We should use pointers instead...
extern void QuitEmulator(bool save);
extern void SaveState();
extern void LoadState();

int odroid_overlay_game_menu()
{
    odroid_audio_clear_buffer();
    odroid_display_lock();
    wait_all_keys_released();

    odroid_dialog_choice_t choices[] = {
        {0, "Continue", "",  1, NULL},
        {3, "Save & Continue", "",  1, NULL},
        {4, "Save & Quit", "", 1, NULL},
        {2, "Reload", "", 1, NULL},
        {5, "Quit", "", 1, NULL},
    };

    int r = odroid_overlay_dialog("Retro-Go", choices, sizeof(choices) / sizeof(choices[0]), 0);
    odroid_display_unlock();

    if (r == 2) {
        LoadState();
        // esp_restart();
    } else if (r == 3) {
        SaveState();
    } else if (r == 4) {
        QuitEmulator(true);
    } else if (r == 5) {
        QuitEmulator(false);
    }

    return r;
}
