#include <rg_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emulators.h"
#include "favorites.h"
#include "gui.h"

static const char *SETTING_SELECTED_TAB  = "SelectedTab";
static const char *SETTING_GUI_THEME     = "ColorTheme";
static const char *SETTING_SHOW_EMPTY    = "ShowEmptyTabs";
static const char *SETTING_SHOW_PREVIEW  = "ShowPreview";
static const char *SETTING_PREVIEW_SPEED = "PreviewSpeed";


static dialog_return_t font_type_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV) {
        rg_gui_set_font_type(rg_gui_get_font_info().type - 1);
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        rg_gui_set_font_type(rg_gui_get_font_info().type + 1);
        gui_redraw();
    }

    font_info_t info = rg_gui_get_font_info();
    sprintf(option->value, "%s %d", info.font->name, info.height);

    return RG_DIALOG_IGNORE;
}

static dialog_return_t show_empty_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        gui.show_empty = !gui.show_empty;
        rg_settings_set_app_int32(SETTING_SHOW_EMPTY, gui.show_empty);
    }
    strcpy(option->value, gui.show_empty ? "Show" : "Hide");
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
    int disk_activity = rg_vfs_get_enable_disk_led();
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        disk_activity = !disk_activity;
        rg_vfs_set_enable_disk_led(disk_activity);
    }
    strcpy(option->value, disk_activity ? "On " : "Off");
    return RG_DIALOG_IGNORE;
}

static dialog_return_t show_preview_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV) {
        if (--gui.show_preview < 0) gui.show_preview = PREVIEW_MODE_COUNT - 1;
        rg_settings_set_app_int32(SETTING_SHOW_PREVIEW, gui.show_preview);
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.show_preview >= PREVIEW_MODE_COUNT) gui.show_preview = 0;
        rg_settings_set_app_int32(SETTING_SHOW_PREVIEW, gui.show_preview);
    }
    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover only", "Save only "};
    strcpy(option->value, values[gui.show_preview % PREVIEW_MODE_COUNT]);
    return RG_DIALOG_IGNORE;
}

static dialog_return_t show_preview_speed_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        gui.show_preview_fast = gui.show_preview_fast ? 0 : 1;
        rg_settings_set_app_int32(SETTING_PREVIEW_SPEED, gui.show_preview_fast);
    }
    strcpy(option->value, gui.show_preview_fast ? "Short" : "Long");
    return RG_DIALOG_IGNORE;
}

static dialog_return_t color_shift_cb(dialog_option_t *option, dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == RG_DIALOG_PREV) {
        if (--gui.theme < 0) gui.theme = max;
        rg_settings_set_app_int32(SETTING_GUI_THEME, gui.theme);
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        rg_settings_set_app_int32(SETTING_GUI_THEME, gui.theme);
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return RG_DIALOG_IGNORE;
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
    int last_key = -1;
    int repeat = 0;
    int selected_tab_last = -1;

    gui.selected     = rg_settings_get_app_int32(SETTING_SELECTED_TAB, 0);
    gui.theme        = rg_settings_get_app_int32(SETTING_GUI_THEME, 0);
    gui.show_empty   = rg_settings_get_app_int32(SETTING_SHOW_EMPTY, 1);
    gui.show_preview = rg_settings_get_app_int32(SETTING_SHOW_PREVIEW, 1);
    gui.show_preview_fast = rg_settings_get_app_int32(SETTING_PREVIEW_SPEED, 0);

    if (!gui.show_empty)
    {
        // If we're hiding empty tabs then we must preload all files
        // to avoid flicker and delays when skipping empty tabs...
        for (int i = 0; i < gui.tabcount; i++)
        {
            gui_init_tab(gui.tabs[i]);
        }
    }

    rg_display_clear(C_BLACK);

    while (true)
    {
        if (gui.selected != selected_tab_last)
        {
            int direction = (gui.selected - selected_tab_last) < 0 ? -1 : 1;

            gui_event(TAB_LEAVE, tab);

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

            gui_event(TAB_ENTER, tab);

            selected_tab_last = gui.selected;
        }

        gui.joystick = rg_input_read_gamepad();

        if (gui.idle_counter > 0 && (gui.joystick & GAMEPAD_KEY_ANY) == 0)
        {
            gui_event(TAB_IDLE, tab);

            if (gui.idle_counter % 100 == 0)
                gui_draw_status(tab);
        }

        if ((gui.joystick & last_key) && repeat > 0)
        {
            last_key |= (1 << 24); // No repeat
            if (--repeat == 0)
            {
                last_key &= GAMEPAD_KEY_ANY;
                repeat = 4;
            }
        }
        else
        {
            last_key = gui.joystick & GAMEPAD_KEY_ANY;
            repeat = 25;
        }

        if (last_key == GAMEPAD_KEY_MENU) {
            char buildstr[32], datestr[32];

            const dialog_option_t options[] = {
                {0, "Ver.", buildstr, 1, NULL},
                {0, "Date", datestr, 1, NULL},
                {0, "By", "ducalex", 1, NULL},
                RG_DIALOG_SEPARATOR,
                {1, "Reboot to firmware", NULL, 1, NULL},
                {2, "Reset settings", NULL, 1, NULL},
                {3, "Clear cache", NULL, 1, NULL},
                {0, "Close", NULL, 1, NULL},
                RG_DIALOG_CHOICE_LAST
            };

            const rg_app_desc_t *app = rg_system_get_app();
            sprintf(buildstr, "%.30s", app->version);
            sprintf(datestr, "%s %.5s", app->buildDate, app->buildTime);

            char *rel_hash = strstr(buildstr, "-0-g");
            if (rel_hash)
            {
                rel_hash[0] = ' ';
                rel_hash[1] = ' ';
                rel_hash[2] = ' ';
                rel_hash[3] = '(';
                strcat(buildstr, ")");
            }

            int sel = rg_gui_dialog("Retro-Go", options, -1);
            if (sel == 1) {
                rg_system_switch_app(RG_APP_FACTORY);
            }
            else if (sel == 2) {
                if (rg_gui_confirm("Reset all settings?", NULL, false)) {
                    rg_settings_reset();
                    rg_system_restart();
                }
            }
            else if (sel == 3) {
                rg_vfs_delete(CRC_CACHE_PATH);
                rg_system_restart();
            }
            gui_redraw();
        }
        else if (last_key == GAMEPAD_KEY_VOLUME) {
            const dialog_option_t options[] = {
                RG_DIALOG_SEPARATOR,
                {0, "Color theme", "...", 1, &color_shift_cb},
                {0, "Font type  ", "...", 1, &font_type_cb},
                {0, "Empty tabs ", "...", 1, &show_empty_cb},
                {0, "Preview    ", "...", 1, &show_preview_cb},
                {0, "    - Delay", "...", 1, &show_preview_speed_cb},
                {0, "Startup app", "...", 1, &startup_app_cb},
                {0, "Disk LED   ", "...", 1, &disk_activity_cb},
                RG_DIALOG_CHOICE_LAST
            };
            rg_gui_settings_menu(options);
            gui_redraw();
        }
        else if (last_key == GAMEPAD_KEY_SELECT) {
            gui.selected--;
        }
        else if (last_key == GAMEPAD_KEY_START) {
            gui.selected++;
        }
        else if (last_key == GAMEPAD_KEY_UP) {
            gui_scroll_list(tab, SCROLL_LINE_UP, 0);
        }
        else if (last_key == GAMEPAD_KEY_DOWN) {
            gui_scroll_list(tab, SCROLL_LINE_DOWN, 0);
        }
        else if (last_key == GAMEPAD_KEY_LEFT) {
            gui_scroll_list(tab, SCROLL_PAGE_UP, 0);
        }
        else if (last_key == GAMEPAD_KEY_RIGHT) {
            gui_scroll_list(tab, SCROLL_PAGE_DOWN, 0);
        }
        else if (last_key == GAMEPAD_KEY_A) {
            gui_event(KEY_PRESS_A, tab);
        }
        else if (last_key == GAMEPAD_KEY_B) {
            gui_event(KEY_PRESS_B, tab);
        }

        if (gui.joystick & GAMEPAD_KEY_ANY) {
            gui.idle_counter = 0;
        } else {
            gui.idle_counter++;
        }

        usleep(15 * 1000UL);
    }
}

void app_main(void)
{
    rg_system_init(32000, NULL);

    emulators_init();
    favorites_init();

    retro_loop();
}
