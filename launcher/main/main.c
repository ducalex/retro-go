#include <rg_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ftp_server.h"
#include "emulators.h"
#include "bookmarks.h"
#include "gui.h"


static dialog_return_t font_type_cb(dialog_option_t *option, dialog_event_t event)
{
    rg_gui_font_t info = rg_gui_get_font_info();

    if (event == RG_DIALOG_PREV) {
        rg_gui_set_font_type((int)info.type - 1);
        info = rg_gui_get_font_info();
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        if (!rg_gui_set_font_type((int)info.type + 1))
            rg_gui_set_font_type(0);
        info = rg_gui_get_font_info();
        gui_redraw();
    }

    sprintf(option->value, "%s %d", info.font->name, info.height);

    return RG_DIALOG_IGNORE;
}

static dialog_return_t toggle_tab_cb(dialog_option_t *option, dialog_event_t event)
{
    tab_t *tab = gui.tabs[option->id];

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        tab->enabled = !tab->enabled;
    }

    strcpy(option->value, tab->enabled ? "Show" : "Hide");
    return RG_DIALOG_IGNORE;
}

static dialog_return_t toggle_tabs_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_ENTER) {
        dialog_option_t options[gui.tabcount + 1];
        dialog_option_t *option = &options[0];

        for (int i = 0; i < gui.tabcount; ++i)
        {
            *option++ = (dialog_option_t) {i, gui.tabs[i]->name, "...", 1, &toggle_tab_cb};
        }

        *option++ = (dialog_option_t)RG_DIALOG_CHOICE_LAST;

        rg_gui_dialog("Toggle Tabs", options, 0);
    }
    return RG_DIALOG_IGNORE;
}

static dialog_return_t startup_app_cb(dialog_option_t *option, dialog_event_t event)
{
    int startup_app = rg_system_get_startup_app();
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        rg_system_set_startup_app(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher " : "Last used");
    return RG_DIALOG_IGNORE;
}

static dialog_return_t disk_activity_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        rg_sdcard_set_enable_activity_led(!rg_sdcard_get_enable_activity_led());
    }
    strcpy(option->value, rg_sdcard_get_enable_activity_led() ? "On " : "Off");
    return RG_DIALOG_IGNORE;
}

static dialog_return_t show_preview_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV) {
        if (--gui.show_preview < 0) gui.show_preview = PREVIEW_MODE_COUNT - 1;
        gui_load_preview(gui_get_current_tab());
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.show_preview >= PREVIEW_MODE_COUNT) gui.show_preview = 0;
        gui_load_preview(gui_get_current_tab());
    }
    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover only", "Save only "};
    strcpy(option->value, values[gui.show_preview % PREVIEW_MODE_COUNT]);
    return RG_DIALOG_IGNORE;
}

// static dialog_return_t show_preview_speed_cb(dialog_option_t *option, dialog_event_t event)
// {
//     if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
//         gui.show_preview_fast = gui.show_preview_fast ? 0 : 1;
//     }
//     strcpy(option->value, gui.show_preview_fast ? "Short" : "Long");
//     return RG_DIALOG_IGNORE;
// }

static dialog_return_t color_shift_cb(dialog_option_t *option, dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == RG_DIALOG_PREV) {
        if (--gui.theme < 0) gui.theme = max;
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return RG_DIALOG_IGNORE;
}

static void show_about_dialog(void)
{
    const dialog_option_t options[] = {
        {1, "Clear cache", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    if (rg_gui_about_menu(options) == 1) {
        unlink(CRC_CACHE_PATH);
        rg_system_restart();
    }
}

static void show_options_dialog(void)
{
    const dialog_option_t options[] = {
        RG_DIALOG_SEPARATOR,
        {0, "Color theme", "...", 1, &color_shift_cb},
        {0, "Font type  ", "...", 1, &font_type_cb},
        {0, "Preview    ", "...", 1, &show_preview_cb},
        // {0, "    - Delay", "...", 1, &show_preview_speed_cb},
        {0, "Toggle tabs", "...", 1, &toggle_tabs_cb},
        {0, "Startup app", "...", 1, &startup_app_cb},
        {0, "Disk LED   ", "...", 1, &disk_activity_cb},
        #if !RG_GAMEPAD_OPTION_BTN
        RG_DIALOG_SEPARATOR,
        {100, "About... ", NULL,  1, NULL},
        #endif
        RG_DIALOG_CHOICE_LAST
    };
    int sel = rg_gui_settings_menu(options);
    gui_save_config(true);
    if (sel == 100)
        show_about_dialog();
}

static void retro_loop(void)
{
    tab_t *tab = gui_get_current_tab();
    int next_repeat = 0;
    int repeats = 0;
    int joystick, prev_joystick;
    int selected_tab_last = -1;
    int browse_last = -1;
    int next_idle_event = 0;

    while (true)
    {
        if (gui.selected != selected_tab_last || gui.browse != browse_last)
        {
            int direction = (gui.selected - selected_tab_last) < 0 ? -1 : 1;
            int skipped = 0;

            gui_event(TAB_LEAVE, tab);

            tab = gui_set_current_tab(gui.selected);

            while (!tab->enabled && skipped < gui.tabcount)
            {
                gui.selected += direction;
                tab = gui_set_current_tab(gui.selected);
                skipped++;
            }

            if (gui.browse)
            {
                if (!tab->initialized)
                {
                    gui_redraw();
                    gui_init_tab(tab);
                }
                gui_event(TAB_ENTER, tab);
            }

            gui_redraw();

            selected_tab_last = gui.selected;
            browse_last = gui.browse;
        }

        prev_joystick = gui.joystick;
        joystick = 0;

        if ((gui.joystick = rg_input_read_gamepad()))
        {
            if (prev_joystick != gui.joystick)
            {
                joystick = gui.joystick;
                repeats = 0;
                next_repeat = get_elapsed_time() + 400000;
            }
            else if (get_elapsed_time_since(next_repeat) >= 0)
            {
                joystick = gui.joystick;
                repeats++;
                next_repeat = get_elapsed_time() + 400000 / (repeats + 1);
            }
        }

        if (joystick == RG_KEY_MENU) {
        #if !RG_GAMEPAD_OPTION_BTN
            show_options_dialog();
        #else
            show_about_dialog();
        #endif
            gui_redraw();
        }
        else if (joystick == RG_KEY_OPTION) {
            show_options_dialog();
            gui_redraw();
        }

        if (gui.browse)
        {
            if (joystick == RG_KEY_SELECT) {
                gui.selected--;
            }
            else if (joystick == RG_KEY_START) {
                gui.selected++;
            }
            else if (joystick == RG_KEY_UP) {
                gui_scroll_list(tab, SCROLL_LINE_UP, 0);
            }
            else if (joystick == RG_KEY_DOWN) {
                gui_scroll_list(tab, SCROLL_LINE_DOWN, 0);
            }
            else if (joystick == RG_KEY_LEFT) {
                gui_scroll_list(tab, SCROLL_PAGE_UP, 0);
            }
            else if (joystick == RG_KEY_RIGHT) {
                gui_scroll_list(tab, SCROLL_PAGE_DOWN, 0);
            }
            else if (joystick == RG_KEY_A) {
                gui_event(KEY_PRESS_A, tab);
            }
            else if (joystick == RG_KEY_B) {
                gui.browse = false;
            }
        }
        else
        {
            if (joystick & (RG_KEY_UP|RG_KEY_LEFT|RG_KEY_SELECT)) {
                gui.selected--;
            }
            else if (joystick & (RG_KEY_DOWN|RG_KEY_RIGHT|RG_KEY_START)) {
                gui.selected++;
            }
            else if (joystick == RG_KEY_A) {
                gui.browse = true;
            }
        }

        if ((gui.joystick|joystick) & RG_KEY_ANY)
        {
            gui.idle_counter = 0;
            next_idle_event = get_elapsed_time() + 100000;
        }
        else if (get_elapsed_time() >= next_idle_event)
        {
            gui.idle_counter++;
            gui_event(TAB_IDLE, tab);
            gui_redraw();
            next_idle_event = get_elapsed_time() + 100000;
        }
        else if (gui.idle_counter)
        {
            usleep(10000);
        }
    }
}

void app_main(void)
{
    rg_system_init(32000, NULL);
    rg_gui_set_buffered(true);

    gui_init();

    emulators_init();
    bookmarks_init();

    ftp_server_start();

    retro_loop();
}
