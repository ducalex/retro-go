#include "odroid_overlay.h"
#include "odroid_display.h"
#include "odroid_settings.h"
#include "odroid_audio.h"
#include "odroid_font8x8.h"
#include "odroid_input.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

static uint16_t *overlay_buffer = NULL;
extern bool scaling_enabled;

int ODROID_FONT_WIDTH = 8;
int ODROID_FONT_HEIGHT = 8;

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
    ODROID_FONT_HEIGHT = odroid_settings_int32_get("FontSize", 0) ? 16 : 8;
}

void odroid_overlay_draw_chars(uint16_t x_pos, uint16_t y_pos, uint16_t width, char *text, uint16_t color, uint16_t color_bg)
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

void odroid_overlay_draw_battery(int x_pos, int y_pos, int percentage)
{
    if (percentage == -1) {
        odroid_battery_state battery_state;
        odroid_input_battery_level_read(&battery_state);
        percentage = battery_state.percentage;
    }

    uint16_t color_border = C_SILVER;
    int width = 20 + 3;
    int height = 8;

    int value = ((percentage-1)/5) + 1;

    if (value<=0) value = 0;
    else if (value>20) value = 20;

    for (int x = 0; x< width; x++)
    {
        overlay_buffer[x] = color_border;
        overlay_buffer[x + (height-1)*width] = color_border;
    }
    for (int y = 0; y< height; y++)
    {
        overlay_buffer[y*width] = color_border;
        overlay_buffer[width-2 + y*width] = color_border;
        if (y>2 && y <height -2)
        {
            overlay_buffer[width-1 + y*width] = color_border;
        }
        else
        {
            overlay_buffer[width-1 + y*width] = C_BLACK;
        }
    }

    uint16_t col;
    if (percentage>40) col = C_FOREST_GREEN;
    else if (percentage>20) col = C_ORANGE;
    else col = C_RED;

    odroid_overlay_fill_rect(1,1, value, height-2, col, width);
    odroid_overlay_fill_rect(1 + value,1, 20-value, height-2, C_BLACK, width);
    ili9341_write_frame_rectangleLE(x_pos, y_pos, width, height, overlay_buffer);
}

void odroid_overlay_dialog_draw(char *header, odroid_dialog_choice_t *options, int options_count, int sel)
{
    int width = header ? strlen(header) : 8;
    int padding = 0;
    int len = 0;

    char row_format[16] = " %s ";
    char row_format_kv[16] = " %s: %s ";
    char rows[16][32];

    for (int i = 0; i < options_count; i++) {
        if (options[i].value[0]) {
            len = strlen(options[i].label);
            padding = (len > padding) ? len : padding;
        }
    }

    sprintf(row_format_kv, " %%-%ds: %%s ", padding);

    for (int i = 0; i < options_count; i++) {
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

    int box_width = width * ODROID_FONT_WIDTH;
    int box_height = ODROID_FONT_HEIGHT * (options_count + 2);
    int x = (320 - box_width) / 2;
    int y = (240 - box_height) / 2;

    for (int i = 0; i < options_count + 2; i++) {
        odroid_overlay_draw_chars(x - 4, y + i * ODROID_FONT_HEIGHT, box_width + 8, (char*)" ", C_WHITE, C_NAVY);
    }

    if (header) {
        int pad = (0.5f * (width - strlen(header)) * ODROID_FONT_WIDTH);
        odroid_overlay_draw_chars(x - 4, y - ODROID_FONT_HEIGHT, box_width + 8, (char*)" ", C_WHITE, C_NAVY);
        odroid_overlay_draw_chars(x + pad, y - 2, 0, header, C_WHITE, C_NAVY);
        y += 2;
    }

    for (int i = 0; i < options_count; i++) {
        int color = options[i].enabled ? C_WHITE : C_GRAY;
        int xo = x, yo = y + ODROID_FONT_HEIGHT + i * ODROID_FONT_HEIGHT;
        odroid_overlay_draw_chars(xo, yo, box_width, rows[i], i == sel ? C_NAVY : color, i == sel ? color : C_NAVY);
    }
}

int odroid_overlay_dialog(char *header, odroid_dialog_choice_t *options, int options_count, int selected)
{
    int sel = selected;
    int sel_old = selected;
    int last_key = -1;
    bool select = false;
    odroid_gamepad_state joystick;

    odroid_overlay_dialog_draw(header, options, options_count, sel);
    // Debounce
    // while (odroid_input_gamepad_read_masked() != 0) {
    //     usleep(1000);
    // }
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
            odroid_overlay_dialog_draw(header, options, options_count, sel);
            sel_old = sel;
        }

        usleep(20 * 1000UL);
    }

    odroid_input_wait_for_key(last_key, false);

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

void odroid_overlay_fill_rect(int x_, int y_, int width, int height, uint16_t color, int buffer_width)
{
    if (width == 0 || height == 0) return;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0;y < height; y++)
        {
            overlay_buffer[x_+x+(y_+y)*buffer_width] = color;
        }
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

    strcpy(option->value, (sink == ODROID_AUDIO_SINK_DAC) ? "DAC" : "Spkr");
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
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        scaling_enabled = !scaling_enabled;
        odroid_settings_ScaleDisabled_set(1, !scaling_enabled);
    }

    strcpy(option->value, scaling_enabled ? "Yes" : "No");
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
        {3, "Scaling   ", "Yes",  1, &scaling_update_cb},
    };
    int options_count = 4;

    if (extra_options_count > 0) {
        memcpy(options + options_count, extra_options, extra_options_count * sizeof(odroid_dialog_choice_t));
        options_count += extra_options_count;
    }

    int r = odroid_overlay_dialog("Options", options, options_count, 0);
    odroid_display_unlock();

    return r;
}

// We should use pointers instead...
extern void QuitEmulator(bool save);
extern void SaveState();

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
        esp_restart();
    } else if (r == 3) {
        SaveState();
    } else if (r == 4) {
        QuitEmulator(true);
    } else if (r == 5) {
        QuitEmulator(false);
    }

    return r;
}
