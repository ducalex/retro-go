#include <rg_system.h>
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
#define KEY_SHOW_PREVIEW  "ShowPreview"
#define KEY_PREVIEW_SPEED "PreviewSpeed"

static bool font_size_cb(dialog_choice_t *option, dialog_event_t event)
{
    int font_size = rg_gui_get_font_info().points;
    if (event == RG_DIALOG_PREV && font_size > 8) {
        rg_gui_set_font_size(font_size -= 4);
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT && font_size < 16) {
        rg_gui_set_font_size(font_size += 4);
        gui_redraw();
    }
    sprintf(option->value, "%d", font_size);
    if (font_size ==  8) strcpy(option->value, "Small ");
    if (font_size == 12) strcpy(option->value, "Medium");
    if (font_size == 16) strcpy(option->value, "Large ");
    return event == RG_DIALOG_ENTER;
}

static bool show_empty_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        gui.show_empty = !gui.show_empty;
        rg_settings_int32_set(KEY_SHOW_EMPTY, gui.show_empty);
    }
    strcpy(option->value, gui.show_empty ? "Show" : "Hide");
    return event == RG_DIALOG_ENTER;
}

static bool startup_app_cb(dialog_choice_t *option, dialog_event_t event)
{
    int startup_app = rg_settings_StartupApp_get();
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        rg_settings_StartupApp_set(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher " : "Last used");
    return event == RG_DIALOG_ENTER;
}

static bool show_preview_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV) {
        if (--gui.show_preview < 0) gui.show_preview = 5;
        rg_settings_int32_set(KEY_SHOW_PREVIEW, gui.show_preview);
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.show_preview > 5) gui.show_preview = 0;
        rg_settings_int32_set(KEY_SHOW_PREVIEW, gui.show_preview);
    }
    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover     ", "Save      "};
    strcpy(option->value, values[gui.show_preview % 5]);
    return event == RG_DIALOG_ENTER;
}

static bool show_preview_speed_cb(dialog_choice_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        gui.show_preview_fast = gui.show_preview_fast ? 0 : 1;
        rg_settings_int32_set(KEY_PREVIEW_SPEED, gui.show_preview_fast);
    }
    strcpy(option->value, gui.show_preview_fast ? "Short" : "Long");
    return event == RG_DIALOG_ENTER;
}

static bool color_shift_cb(dialog_choice_t *option, dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == RG_DIALOG_PREV) {
        if (--gui.theme < 0) gui.theme = max;
        rg_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        rg_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return event == RG_DIALOG_ENTER;
}

static inline bool tab_enabled(tab_t *tab)
{
    int disabled_tabs = 0;

    if (gui.show_empty)
        return true;

    // If all tabs are disabled then we always return true, otherwise it's an endless loop
    for (int i = 0; i < gui.tabcount; ++i)
        if (gui.tabs[i]->initialized && gui.tabs[i]->is_empty)
            disabled_tabs++;

    return (disabled_tabs == gui.tabcount) || (tab->initialized && !tab->is_empty);
}

void retro_loop()
{
    tab_t *tab = gui_get_current_tab();
    int debounce = 0;
    int last_key = -1;
    int selected_tab_last = -1;

    gui.selected     = rg_settings_int32_get(KEY_SELECTED_TAB, 0);
    gui.theme        = rg_settings_int32_get(KEY_GUI_THEME, 0);
    gui.show_empty   = rg_settings_int32_get(KEY_SHOW_EMPTY, 1);
    gui.show_preview = rg_settings_int32_get(KEY_SHOW_PREVIEW, 1);
    gui.show_preview_fast = rg_settings_int32_get(KEY_PREVIEW_SPEED, 0);

    while (true)
    {
        if (gui.selected != selected_tab_last)
        {
            int direction = (gui.selected - selected_tab_last) < 0 ? -1 : 1;

            tab = gui_set_current_tab(gui.selected);

            if (!tab->initialized)
            {
                gui_redraw();
                gui_init_tab(tab);

                if (tab_enabled(tab))
                {
                    gui_draw_status(tab);
                    gui_draw_list(tab);
                }
            }
            else if (tab_enabled(tab))
            {
                gui_redraw();
            }

            if (!tab_enabled(tab))
            {
                gui.selected += direction;
                continue;
            }

            selected_tab_last = gui.selected;
        }

        gui.joystick = rg_input_read_gamepad();

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
            for (int i = 0; i < GAMEPAD_KEY_MAX; i++)
                if (gui.joystick.values[i]) last_key = i;

            if (last_key == GAMEPAD_KEY_MENU) {
                dialog_choice_t choices[] = {
                    {0, "Ver.", "build string", 1, NULL},
                    {0, "Date", "", 1, NULL},
                    {0, "By", "ducalex", 1, NULL},
                    {0, "--", "", -1, NULL},
                    {1, "Reboot to firmware", "", 1, NULL},
                    {2, "Reset settings", "", 1, NULL},
                    {0, "Close", "", 1, NULL},
                    RG_DIALOG_CHOICE_LAST
                };

                const esp_app_desc_t *app = esp_ota_get_app_description();
                sprintf(choices[0].value, "%.30s", app->version);
                sprintf(choices[1].value, "%s %.5s", app->date, app->time);

                if (strstr(app->version, "-0-") == strrchr(app->version, '-') - 2)
                    sprintf(strstr(choices[0].value, "-0-") , " (%s)", strrchr(app->version, '-') + 1);

                int sel = rg_gui_dialog("Retro-Go", choices, -1);
                if (sel == 1) {
                    rg_system_switch_app(RG_APP_FACTORY);
                }
                else if (sel == 2) {
                    if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                        rg_settings_reset();
                        rg_system_restart();
                    }
                }
                gui_redraw();
            }
            else if (last_key == GAMEPAD_KEY_VOLUME) {
                dialog_choice_t choices[] = {
                    {0, "---",         "",    -1, NULL},
                    {0, "Color theme", "...",  1, &color_shift_cb},
                    {0, "Font size  ", "...",  1, &font_size_cb},
                    {0, "Empty tabs ", "...",  1, &show_empty_cb},
                    {0, "Preview    ", "...",  1, &show_preview_cb},
                    {0, "    - Delay", "...",  1, &show_preview_speed_cb},
                    {0, "Startup app", "...",  1, &startup_app_cb},
                    RG_DIALOG_CHOICE_LAST
                };
                rg_gui_settings_menu(choices);
                gui_redraw();
            }
            else if (last_key == GAMEPAD_KEY_SELECT) {
                debounce = -10;
                gui.selected--;
            }
            else if (last_key == GAMEPAD_KEY_START) {
                debounce = -10;
                gui.selected++;
            }
            else if (last_key == GAMEPAD_KEY_UP) {
                gui_scroll_list(tab, LINE_UP);
            }
            else if (last_key == GAMEPAD_KEY_DOWN) {
                gui_scroll_list(tab, LINE_DOWN);
            }
            else if (last_key == GAMEPAD_KEY_LEFT) {
                gui_scroll_list(tab, PAGE_UP);
            }
            else if (last_key == GAMEPAD_KEY_RIGHT) {
                gui_scroll_list(tab, PAGE_DOWN);
            }
            else if (last_key == GAMEPAD_KEY_A) {
                gui_event(KEY_PRESS_A, tab);
            }
            else if (last_key == GAMEPAD_KEY_B) {
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
    rg_system_init(0, 32000);
    rg_display_clear(0);

    emulators_init();
    favorites_init();

    retro_loop();
}
