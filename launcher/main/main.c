#include <rg_system.h>
#include <rg_printf.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ftp_server.h"
#include "emulators.h"
#include "bookmarks.h"
#include "gui.h"


static rg_gui_event_t toggle_tab_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    tab_t *tab = gui.tabs[option->id];
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
        gui_load_preview(gui_get_current_tab());
    }
    if (event == RG_DIALOG_NEXT) {
        if (++gui.show_preview >= PREVIEW_MODE_COUNT) gui.show_preview = 0;
        gui_load_preview(gui_get_current_tab());
    }
    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover only", "Save only "};
    strcpy(option->value, values[gui.show_preview % PREVIEW_MODE_COUNT]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t color_theme_cb(rg_gui_option_t *option, rg_gui_event_t event)
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

static void retro_loop(void)
{
    tab_t *tab = gui_get_current_tab();
    int next_repeat = 0;
    int repeats = 0;
    int joystick, prev_joystick;
    int selected_tab_last = -1;
    int browse_last = -1;
    int next_idle_event = 0;
    bool redraw_pending = true;

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

            selected_tab_last = gui.selected;
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
                next_repeat = get_elapsed_time() + 400000;
            }
            else if (get_elapsed_time_since(next_repeat) >= 0)
            {
                joystick = gui.joystick;
                repeats++;
                next_repeat = get_elapsed_time() + 400000 / (repeats + 1);
            }
        }

        #if !RG_GAMEPAD_OPTION_BTN
        if (joystick == RG_KEY_MENU)
        #else
        if (joystick == RG_KEY_OPTION)
        #endif
        {
            if (rg_gui_options_menu() == 123)
                rg_gui_about_menu(NULL);
            gui_save_config(true);
            redraw_pending = true;
        }
        else if (joystick == RG_KEY_MENU) {
            rg_gui_about_menu(NULL);
            redraw_pending = true;
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

        if (redraw_pending)
        {
            redraw_pending = false;
            gui_redraw();
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
            next_idle_event = get_elapsed_time() + 100000;
            redraw_pending = true;
            gui.joystick = 0;
        }
        else if (gui.idle_counter)
        {
            usleep(10000);
        }
    }
}

static void try_migrate(void)
{
    if (rg_settings_get_number(NS_GLOBAL, "Migration", 0) < 1290)
    {
#if 0 // Changed my mind for now, it might be a hassle to some users
        rmdir(RG_BASE_PATH_COVERS); // Remove if present but empty
        rmdir(RG_BASE_PATH_SAVES);  // Remove if present but empty
        bool mv_data = !access("/sd/odroid/data", F_OK) && access(RG_BASE_PATH_SAVES, F_OK);
        bool mv_art = !access("/sd/romart", F_OK) && access(RG_BASE_PATH_COVERS, F_OK);

        if (mv_data && rg_gui_confirm("New save path in 1.29", "Can I move /odroid/data to /retro-go/saves?", true))
            rename("/sd/odroid/data", RG_BASE_PATH_SAVES);

        if (mv_art && rg_gui_confirm("New cover path in 1.29", "Can I move /romart to /retro-go/covers?", true))
            rename("/sd/romart", RG_BASE_PATH_COVERS);
#endif

        // These don't conflict, no need to ask
        rg_mkdir(RG_BASE_PATH_CONFIG);
        rename(RG_ROOT_PATH "/odroid/favorite.txt", RG_BASE_PATH_CONFIG "/favorite.txt");
        rename(RG_ROOT_PATH "/odroid/recent.txt", RG_BASE_PATH_CONFIG "/recent.txt");

        rg_settings_set_number(NS_GLOBAL, "Migration", 1290);

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
        RG_DIALOG_SEPARATOR,
        {0, "Startup app ", "...", 1, &startup_app_cb},
        #if !RG_GAMEPAD_OPTION_BTN
        RG_DIALOG_SEPARATOR,
        {123, "About...  ", NULL,  1, NULL},
        #endif
        RG_DIALOG_CHOICE_LAST
    };

    rg_system_init(32000, &handlers, options);
    rg_gui_set_buffered(true);

    rg_mkdir(RG_BASE_PATH_CACHE);
    rg_mkdir(RG_BASE_PATH_CONFIG);
    rg_mkdir(RG_BASE_PATH_SYSTEM);

    try_migrate();
    gui_init();

    emulators_init();
    bookmarks_init();

    ftp_server_start();

    retro_loop();
}
