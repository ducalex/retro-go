#include <rg_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "applications.h"
#include "fileserver.h"
#include "bookmarks.h"
#include "themes.h"
#include "gui.h"


static rg_gui_event_t toggle_tab_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    tab_t *tab = gui.tabs[option->arg];
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        tab->enabled = !tab->enabled;
    }
    strcpy(option->value, tab->enabled ? "Show" : "Hide");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t toggle_tabs_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER) {
        rg_gui_option_t options[gui.tabcount + 1];
        rg_gui_option_t *option = &options[0];

        for (int i = 0; i < gui.tabcount; ++i)
        {
            *option++ = (rg_gui_option_t) {i, gui.tabs[i]->name, "...", 1, &toggle_tab_cb};
        }

        *option++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

        rg_gui_dialog("Tabs Visibility", options, 0);
        gui_redraw();
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t start_screen_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const char *modes[] = {"Auto", "Carousel", "Browser"};
    int max = 2;

    if (event == RG_DIALOG_PREV && --gui.start_screen < 0) gui.start_screen = max;
    if (event == RG_DIALOG_NEXT && ++gui.start_screen > max) gui.start_screen = 0;

    strcpy(option->value, modes[gui.start_screen % (max+1)]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t show_preview_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV) {
        if (--gui.show_preview < 0) gui.show_preview = PREVIEW_MODE_COUNT - 1;
        gui_set_preview(gui_get_current_tab(), NULL);
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.show_preview >= PREVIEW_MODE_COUNT) gui.show_preview = 0;
        gui_set_preview(gui_get_current_tab(), NULL);
    }
    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover only", "Save only "};
    strcpy(option->value, values[gui.show_preview % PREVIEW_MODE_COUNT]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t color_theme_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == RG_DIALOG_PREV) {
        if (--gui.color_theme < 0) gui.color_theme = max;
        gui_redraw();
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.color_theme > max) gui.color_theme = 0;
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.color_theme + 1, max + 1);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t startup_app_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const char *modes[] = {"Last game", "Launcher"};
    int max = 1;

    if (event == RG_DIALOG_PREV && --gui.startup < 0) gui.startup = max;
    if (event == RG_DIALOG_NEXT && ++gui.startup > max) gui.startup = 0;

    strcpy(option->value, modes[gui.startup % (max+1)]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t about_app_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
        rg_gui_about_menu(NULL);
    return RG_DIALOG_VOID;
}

static void retro_loop(void)
{
    tab_t *tab = gui_get_current_tab();
    int64_t next_repeat = 0;
    int64_t next_idle_event = 0;
    int repeats = 0;
    int joystick, prev_joystick;
    int change_tab = 0;
    int browse_last = -1;
    bool redraw_pending = true;

    while (true)
    {
        if (!tab->enabled && !change_tab)
        {
            change_tab = 1;
        }

        if (change_tab || gui.browse != browse_last)
        {
            if (change_tab)
            {
                gui_event(TAB_LEAVE, tab);
                tab = gui_set_current_tab(gui.selected + change_tab);
                for (int tabs = gui.tabcount; !tab->enabled && --tabs > 0;)
                    tab = gui_set_current_tab(gui.selected + change_tab);
                change_tab = 0;
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

            browse_last = gui.browse;
            redraw_pending = true;
        }

        prev_joystick = gui.joystick;
        joystick = 0;

        if ((gui.joystick = rg_input_read_gamepad()))
        {
            if (prev_joystick != gui.joystick)
            {
                joystick = gui.joystick;
                repeats = 0;
                next_repeat = rg_system_timer() + 400000;
            }
            else if ((rg_system_timer() - next_repeat) >= 0)
            {
                joystick = gui.joystick;
                repeats++;
                next_repeat = rg_system_timer() + 400000 / (repeats + 1);
            }
        }

        if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
        #ifdef RG_TARGET_ODROID_GO
            if (joystick == RG_KEY_MENU)
                rg_gui_about_menu(NULL);
            else
        #endif
            rg_gui_options_menu();

            gui_save_config();
            rg_storage_commit();
            redraw_pending = true;
        }

        if (gui.browse)
        {
            if (joystick == RG_KEY_SELECT) {
                change_tab = -1;
            }
            else if (joystick == RG_KEY_START) {
                change_tab = 1;
            }
            else if (joystick == RG_KEY_UP) {
                gui_scroll_list(tab, SCROLL_LINE, -1);
            }
            else if (joystick == RG_KEY_DOWN) {
                gui_scroll_list(tab, SCROLL_LINE, 1);
            }
            else if (joystick == RG_KEY_LEFT) {
                gui_scroll_list(tab, SCROLL_PAGE, -1);
            }
            else if (joystick == RG_KEY_RIGHT) {
                gui_scroll_list(tab, SCROLL_PAGE, 1);
            }
            else if (joystick == RG_KEY_A) {
                gui_event(TAB_ACTION, tab);
                redraw_pending = true;
            }
            else if (joystick == RG_KEY_B) {
                if (tab->navpath)
                    gui_event(TAB_BACK, tab);
                else
                    gui.browse = false;
                redraw_pending = true;
            }
        }
        else
        {
            if (joystick & (RG_KEY_UP|RG_KEY_LEFT|RG_KEY_SELECT)) {
                change_tab = -1;
            }
            else if (joystick & (RG_KEY_DOWN|RG_KEY_RIGHT|RG_KEY_START)) {
                change_tab = 1;
            }
            else if (joystick == RG_KEY_A) {
                gui.browse = true;
            }
        }

        if (redraw_pending)
        {
            redraw_pending = false;
            gui_redraw();
        }

        if ((gui.joystick|joystick) & RG_KEY_ANY)
        {
            gui.idle_counter = 0;
            next_idle_event = rg_system_timer() + 100000;
        }
        else if (rg_system_timer() >= next_idle_event)
        {
            gui.idle_counter++;
            gui.joystick = 0;
            prev_joystick = 0;
            gui_event(TAB_IDLE, tab);
            next_idle_event = rg_system_timer() + 100000;
            redraw_pending = true;
        }
        else if (gui.idle_counter)
        {
            usleep(10000);
        }
    }
}

static void try_migrate(void)
{
    // A handful of retro-go versions used the weird /odroid/*.txt to store books. Let's move them!
    if (rg_settings_get_number(NS_GLOBAL, "Migration", 0) < 1290)
    {
    #ifdef RG_TARGET_ODROID_GO
        rg_mkdir(RG_BASE_PATH_CONFIG);
        rename(RG_ROOT_PATH "/odroid/favorite.txt", RG_BASE_PATH_CONFIG "/favorite.txt");
        rename(RG_ROOT_PATH "/odroid/recent.txt", RG_BASE_PATH_CONFIG "/recent.txt");
    #endif
        rg_settings_set_number(NS_GLOBAL, "Migration", 1290);
        rg_storage_commit();
    }

    // Some of our save formats have diverged and cause issue when they're shared with Go-Play
    if (rg_settings_get_number(NS_GLOBAL, "Migration", 0) < 1390)
    {
    #ifdef RG_TARGET_ODROID_GO
        if (access(RG_ROOT_PATH"/odroid/data", F_OK) == 0)
            rg_gui_alert("Save path changed in 1.32",
                "Save format is no longer fully compatible with Go-Play and can cause corruption.\n\n"
                "Please copy the contents of:\n /odroid/data\nto\n /retro-go/saves.");
    #endif
        rg_settings_set_number(NS_GLOBAL, "Migration", 1390);
        rg_storage_commit();
    }
}

void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
        gui_redraw();
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Color theme ", "...", 1, &color_theme_cb},
        {0, "Preview     ", "...", 1, &show_preview_cb},
        {0, "Start screen", "...", 1, &start_screen_cb},
        {0, "Hide tabs   ", "...", 1, &toggle_tabs_cb},
        {0, "Startup app ", "...", 1, &startup_app_cb},
    #ifndef RG_TARGET_ODROID_GO
        RG_DIALOG_SEPARATOR,
        {0, "About Retro-Go", NULL,  1, &about_app_cb},
    #endif
        RG_DIALOG_CHOICE_LAST
    };

    rg_system_init(32000, &handlers, options);

    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert("SD Card Error", "Storage mount failed.\nMake sure the card is FAT32.");
    }
    else
    {
        rg_mkdir(RG_BASE_PATH_CACHE);
        rg_mkdir(RG_BASE_PATH_CONFIG);
        rg_mkdir(RG_BASE_PATH_SYSTEM);
        try_migrate();
    }

    rg_gui_set_buffered(true);

    gui_init();
    applications_init();
    bookmarks_init();
    themes_init();

    ftp_server_start();

    retro_loop();
}
