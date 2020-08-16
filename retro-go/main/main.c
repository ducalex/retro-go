#include <odroid_system.h>
#include <esp_ota_ops.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emulators.h"
#include "favorites.h"
#include "gui.h"

#define KEY_SELECTED_TAB  "SelectedTab"
#define KEY_GUI_THEME     "ColorTheme"
#define KEY_SHOW_EMPTY    "ShowEmptyTabs"
#define KEY_SHOW_COVER    "ShowGameCover"

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int font_size = odroid_settings_FontSize_get();
    if (event == ODROID_DIALOG_PREV) {
        if (--font_size < 1) font_size = 2;
        odroid_overlay_set_font_size(font_size);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++font_size > 2) font_size = 1;
        odroid_overlay_set_font_size(font_size);
        gui_redraw();
    }
    strcpy(option->value, font_size > 1 ? "Large" : "Small");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_empty_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        gui.show_empty = !gui.show_empty;
        odroid_settings_int32_set(KEY_SHOW_EMPTY, gui.show_empty);
    }
    strcpy(option->value, gui.show_empty ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool startup_app_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int startup_app = odroid_settings_StartupApp_get();
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        odroid_settings_StartupApp_set(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher" : "Previous");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cover_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV) {
        if (--gui.show_cover < 0) gui.show_cover = 2;
        odroid_settings_int32_set(KEY_SHOW_COVER, gui.show_cover);
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++gui.show_cover > 2) gui.show_cover = 0;
        odroid_settings_int32_set(KEY_SHOW_COVER, gui.show_cover);
    }
    if (gui.show_cover == 0) strcpy(option->value, "No");
    if (gui.show_cover == 1) strcpy(option->value, "Slow");
    if (gui.show_cover == 2) strcpy(option->value, "Fast");
    return event == ODROID_DIALOG_ENTER;
}

static bool color_shift_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == ODROID_DIALOG_PREV) {
        if (--gui.theme < 0) gui.theme = max;
        odroid_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        odroid_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

void retro_loop()
{
    tab_t *tab = gui_get_current_tab();
    int debounce = 0;
    int last_key = -1;
    int selected_tab_last = -1;

    gui.selected   = odroid_settings_int32_get(KEY_SELECTED_TAB, 0);
    gui.theme      = odroid_settings_int32_get(KEY_GUI_THEME, 0);
    gui.show_empty = odroid_settings_int32_get(KEY_SHOW_EMPTY, 1);
    gui.show_cover = odroid_settings_int32_get(KEY_SHOW_COVER, 1);

    while (true)
    {
        if (gui.selected != selected_tab_last)
        {
            int dir = gui.selected - selected_tab_last;

            if (gui.selected >= gui.tabcount) gui.selected = 0;
            if (gui.selected < 0) gui.selected = gui.tabcount - 1;

            tab = gui_get_current_tab();

            gui_redraw();

            if (!tab->initialized)
            {
                gui_init_tab(tab);
                gui_draw_status(tab);
                gui_draw_list(tab);
            }

            // To do remove flicker by not always redrawing above...
            if (!gui.show_empty && tab->listbox.length == 0)
            {
                gui.selected += dir;
                continue;
            }

            selected_tab_last = gui.selected;
        }

        odroid_input_read_gamepad(&gui.joystick);

        if (gui.idle_counter > 0 && gui.joystick.bitmask == 0)
        {
            gui_event(TAB_IDLE, tab);

            if (gui.idle_counter % 100 == 0)
                gui_draw_status(tab);
        }

        if (last_key >= 0) {
            if (!gui.joystick.values[last_key]) {
                last_key = -1;
                debounce = 0;
            } else if (debounce++ > 12) {
                debounce = 12;
                last_key = -1;
            }
        } else {
            for (int i = 0; i < ODROID_INPUT_MAX; i++)
                if (gui.joystick.values[i]) last_key = i;

            if (last_key == ODROID_INPUT_MENU) {
                odroid_dialog_choice_t choices[] = {
                    {0, "Build", "", 1, NULL},
                    {0, "Date ", "", 1, NULL},
                    {0, "Type ", "n/a", 1, NULL},
                    {0, "By   ", "ducalex", 1, NULL},
                    {0, "", "", -1, NULL},
                    {1, "Reboot to firmware", "", 1, NULL},
                    {2, "Reset settings", "", 1, NULL},
                    {0, "Close", "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST
                };

                const esp_app_desc_t *app = esp_ota_get_app_description();
                sprintf(choices[0].value, "%.30s", app->version);
                sprintf(choices[1].value, "%s %.5s", app->date, app->time);
                #ifdef RETRO_GO_RELEASE
                    sprintf(choices[2].value, "%s", "Release");
                #else
                    sprintf(choices[2].value, "%s", "Test");
                #endif

                int sel = odroid_overlay_dialog("Retro-Go", choices, -1);
                if (sel == 1) {
                    odroid_system_switch_app(-16);
                }
                else if (sel == 2) {
                    if (odroid_overlay_confirm("Reset all settings?", false) == 1) {
                        odroid_settings_reset();
                        esp_restart();
                    }
                }
                gui_redraw();
            }
            else if (last_key == ODROID_INPUT_VOLUME) {
                odroid_dialog_choice_t choices[] = {
                    {0, "---", "", -1, NULL},
                    {0, "Color theme", "1/10", 1, &color_shift_cb},
                    {0, "Font size", "Small", 1, &font_size_cb},
                    {0, "Show cover", "Yes", 1, &show_cover_cb},
                    {0, "Show empty", "Yes", 1, &show_empty_cb},
                    {0, "---", "", -1, NULL},
                    {0, "Startup app", "Last", 1, &startup_app_cb},
                    ODROID_DIALOG_CHOICE_LAST
                };
                odroid_overlay_settings_menu(choices);
                gui_redraw();
            }
            else if (last_key == ODROID_INPUT_SELECT) {
                debounce = -10;
                gui.selected--;
            }
            else if (last_key == ODROID_INPUT_START) {
                debounce = -10;
                gui.selected++;
            }
            else if (last_key == ODROID_INPUT_UP) {
                gui_scroll_list(tab, LINE_UP);
            }
            else if (last_key == ODROID_INPUT_DOWN) {
                gui_scroll_list(tab, LINE_DOWN);
            }
            else if (last_key == ODROID_INPUT_LEFT) {
                gui_scroll_list(tab, PAGE_UP);
            }
            else if (last_key == ODROID_INPUT_RIGHT) {
                gui_scroll_list(tab, PAGE_DOWN);
            }
            else if (last_key == ODROID_INPUT_A) {
                gui_event(KEY_PRESS_A, tab);
            }
            else if (last_key == ODROID_INPUT_B) {
                gui_event(KEY_PRESS_B, tab);
            }
        }

        if (gui.joystick.bitmask) {
            gui.idle_counter = 0;
        } else {
            gui.idle_counter++;
        }

        usleep(15 * 1000UL);
    }
}

void app_main(void)
{
    odroid_system_init(0, 32000);
    odroid_display_clear(0);

    emulators_init();
    favorites_init();

    retro_loop();
}
