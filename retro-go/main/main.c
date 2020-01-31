#include "odroid_settings.h"
#include "odroid_audio.h"
#include "odroid_input.h"
#include "odroid_system.h"
#include "odroid_settings.h"
#include "odroid_display.h"
#include "odroid_overlay.h"
#include "odroid_sdcard.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emulators.h"
#include "gui.h"

const char* SD_BASE_PATH = "/sd";

bool scaling_enabled = true;
extern int gui_themes_count;

static bool show_empty = true;
static bool show_cover = true;
static int  selected_emu = 0;
static int  font_size = 0;
static int  theme = 0;
static bool redraw = true;

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV) {
        if (--font_size < 0) font_size = 0;
        odroid_settings_int32_set("FontSize", font_size);
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++font_size > 1) font_size = 1;
        odroid_settings_int32_set("FontSize", font_size);
    }
    ODROID_FONT_HEIGHT = font_size ? 16 : 8;
    strcpy(option->value, font_size ? "Large" : "Small");
    return event == ODROID_DIALOG_ENTER;
}

static bool hide_empty_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        show_empty = !show_empty;
        odroid_settings_int32_set("ShowEmpty", show_empty);
    }
    strcpy(option->value, show_empty ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cover_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        show_cover = !show_cover;
        odroid_settings_int32_set("ShowCover", show_cover);
    }
    strcpy(option->value, show_cover ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool color_shift_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == ODROID_DIALOG_PREV) {
        if (--theme < 0) theme = 0;
        odroid_settings_int32_set("Theme", theme);
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++theme > max) theme = max;
        odroid_settings_int32_set("Theme", theme);
    }
    sprintf(option->value, "%d/%d", theme + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

static void load_config()
{
    show_empty = odroid_settings_int32_get("ShowEmpty", 1);
    show_cover = odroid_settings_int32_get("ShowCover", 1);
    font_size  = odroid_settings_int32_get("FontSize", 0);
    theme      = odroid_settings_int32_get("Theme", 0);
    selected_emu = odroid_settings_int32_get("SelectedEmu", 0);
    char keyBuffer[16];
    for (int i = 0; i < emulators->count; i++) {
        sprintf(keyBuffer, "Sel.%s", emulators->entries[i].dirname);
        emulators->entries[i].roms.selected = odroid_settings_int32_get(keyBuffer, 0);
    }
}

static void save_config()
{
    // odroid_settings_int32_set("ShowEmpty", show_empty);
    // odroid_settings_int32_get("ShowCover", show_cover);
    // odroid_settings_int32_get("Background", color_shift);
    // odroid_settings_int32_get("FontSize", font_size);
    odroid_settings_int32_set("SelectedEmu", selected_emu);
    char keyBuffer[16];
    for (int i = 0; i < emulators->count; i++) {
        sprintf(keyBuffer, "Sel.%s", emulators->entries[i].dirname);
        odroid_settings_int32_set(keyBuffer, emulators->entries[i].roms.selected);
    }
}

void retro_loop()
{
    retro_emulator_t *emu = NULL;
    int debounce = 0;
    int last_key = -1;
    int selected_emu_last = -1;
    int idle_counter = 0;

    emulators_init();
    load_config();

    odroid_display_lock();

    while (true)
    {
        if (selected_emu != selected_emu_last)
        {
            int dir = selected_emu - selected_emu_last;

            if (selected_emu >= emulators->count) selected_emu = 0;
            if (selected_emu < 0) selected_emu = emulators->count - 1;

            emu = &emulators->entries[selected_emu];

            if (!emu->initialized)
            {
                odroid_overlay_draw_chars(48, 40, 272, "Loading directory...", C_WHITE, C_BLACK);
                emulators_init_emu(emu);
            }

            if (!show_empty && emu->roms.count == 0)
            {
                selected_emu += dir;
                continue;
            }

            gui_header_draw(emu);

            selected_emu_last = selected_emu;
            redraw = true;
        }

        if (redraw ||idle_counter % 100 == 0)
        {
            odroid_overlay_draw_battery(320 - 23, 1, -1);
        }

        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (redraw)
        {
            gui_list_draw(emu, theme);
            idle_counter = 1;
            redraw = false;
        }

        if (show_cover && idle_counter == 5)
        {
            gui_cover_draw(emu, &joystick);
        }

        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
                debounce = 0;
            } else if (debounce++ > 15) {
                debounce = 15;
                last_key = -1;
            }
        } else {
            if (joystick.values[ODROID_INPUT_A]) {
                last_key = ODROID_INPUT_A;
                if (emu->roms.selected < emu->roms.count)
                {
                    char *save_path = odroid_sdcard_get_savefile_path(gui_list_selected_file(emu)->path);
                    bool has_save = access(save_path, F_OK) != -1;

                    odroid_dialog_choice_t choices[] = {
                        {0, "Resume game", "", has_save, NULL},
                        {1, "New game", "", 1, NULL},
                        {2, "Delete save", "", has_save, NULL},
                    };
                    int sel = odroid_overlay_dialog(NULL, &choices, 3, has_save ? 0 : 1);

                    if (sel == 0) {
                        break;
                    }
                    else if (sel == 1) {
                        odroid_settings_StartAction_set(ODROID_START_ACTION_RESTART);
                        break;
                    }
                    else if (sel == 2) {
                        if (odroid_overlay_confirm("Delete savestate?", true) == 1) {
                            unlink(save_path);
                        }
                    }
                    free(save_path);
                    redraw = true;
                    continue;
                }
            }
            else if (joystick.values[ODROID_INPUT_SELECT]) {
                last_key = ODROID_INPUT_SELECT;
                selected_emu--;
                debounce = -10;
            }
            else if (joystick.values[ODROID_INPUT_START]) {
                last_key = ODROID_INPUT_START;
                selected_emu++;
                debounce = -10;
            }
            else if (joystick.values[ODROID_INPUT_MENU]) {
                last_key = ODROID_INPUT_MENU;
                odroid_dialog_choice_t choices[] = {
                    {0, "Date", COMPILEDATE, 1, NULL},
                    {0, "Commit", GITREV, 1, NULL},
                };
                odroid_overlay_dialog("Retro-Go", choices, 2, 3);
                redraw = true;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME]) {
                last_key = ODROID_INPUT_VOLUME;
                odroid_dialog_choice_t choices[] = {
                    {0, "---", "", 0, NULL},
                    {0, "Color theme", "1/10", 1, &color_shift_cb},
                    {0, "Font size", "Small", 1, &font_size_cb},
                    {0, "Show cover", "Yes", 1, &show_cover_cb},
                    {0, "Show empty", "Yes", 1, &hide_empty_cb},
                };
                odroid_display_unlock();
                odroid_overlay_settings_menu(choices, sizeof(choices) / sizeof(choices[0]));
                odroid_display_lock();
                selected_emu_last = -1;
                redraw = true;
            }
            else if (gui_list_handle_input(emu, &joystick, &last_key)) {
                redraw = true;
            }
        }

        if (last_key < 0) {
            idle_counter++;
        }

        usleep(20 * 1000UL);
    }

    odroid_display_unlock();
    save_config();

    emulators_start_emu(emu);
}


void app_main(void)
{
    printf("Retro-Go (%s-%s).\n", COMPILEDATE, GITREV);

	odroid_settings_init();
    odroid_system_init();
    odroid_overlay_init();
    odroid_input_gamepad_init();
    odroid_input_battery_level_init();

    esp_err_t r = odroid_sdcard_open(SD_BASE_PATH);
    ili9341_init();

    if (r != ESP_OK)
    {
        odroid_display_show_sderr(ODROID_SD_ERR_NOCARD);
        usleep(100 * 1000UL);
        abort();
    }

    ili9341_blank_screen();
    retro_loop();
}
