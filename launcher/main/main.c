#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef CONFIG_IDF_TARGET
#include <esp_heap_caps.h>
#endif

#include "applications.h"
#include "bookmarks.h"
#include "music.h"
#include "gui.h"
#include "webui.h"
#include "timezones.h"
#include "updater.h"

#define MAX_AP_LIST 5

static const char *SETTING_WIFI_SLOT = "slot";

static rg_app_t *app;

static rg_gui_event_t toggle_tab_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    tab_t *tab = gui.tabs[option->arg];
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        tab->enabled = !tab->enabled;
    }
    strcpy(option->value, tab->enabled ? "Show" : "Hide");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t toggle_tabs_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        rg_gui_option_t options[gui.tabs_count + 1];
        rg_gui_option_t *opt = options;

        for (size_t i = 0; i < gui.tabs_count; ++i)
            *opt++ = (rg_gui_option_t){i, gui.tabs[i]->name, "...", 1, &toggle_tab_cb};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        rg_gui_dialog("Tabs Visibility", options, 0);
        gui_redraw();
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t timezone_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        rg_gui_option_t options[timezones_count + 1];
        rg_gui_option_t *opt = options;

        for (size_t i = 0; i < timezones_count; i++)
            *opt++ = (rg_gui_option_t){i, timezones[i].name, NULL, 1, NULL};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog("Timezone", options, 0);
        if (sel != RG_DIALOG_CANCELLED)
            rg_system_set_timezone(timezones[sel].TZ);
        gui_redraw();
    }

    strcpy(option->value, getenv("TZ"));

    for (size_t i = 0; i < timezones_count; i++)
    {
        if (strcmp(timezones[i].TZ, option->value) == 0)
        {
            strcpy(option->value, timezones[i].name);
            break;
        }
    }

    return RG_DIALOG_VOID;
}

static rg_gui_event_t start_screen_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const char *modes[] = {"Auto", "Carousel", "Browser"};
    int max = 2;

    if (event == RG_DIALOG_PREV && --gui.start_screen < 0)
        gui.start_screen = max;
    if (event == RG_DIALOG_NEXT && ++gui.start_screen > max)
        gui.start_screen = 0;

    strcpy(option->value, modes[gui.start_screen % (max + 1)]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t show_preview_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV && --gui.show_preview < 0)
        gui.show_preview = PREVIEW_MODE_COUNT - 1;
    if (event == RG_DIALOG_NEXT && ++gui.show_preview >= PREVIEW_MODE_COUNT)
        gui.show_preview = 0;
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        gui_set_preview(gui_get_current_tab(), NULL);

    const char *values[] = {"None      ", "Cover,Save", "Save,Cover", "Cover only", "Save only "};
    strcpy(option->value, values[gui.show_preview % PREVIEW_MODE_COUNT]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_switch_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        wifi_set_switch(!wifi_get_switch());
    }
    strcpy(option->value, wifi_get_switch() ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        rg_gui_option_t options[MAX_AP_LIST + 2];
        rg_gui_option_t *opt = options;

        for (size_t i = 0; i < MAX_AP_LIST; i++)
        {
            char slot[6];
            sprintf(slot, "ssid%d", i);
            char *ap_name = rg_settings_get_string(NS_WIFI, slot, NULL);
            *opt++ = (rg_gui_option_t){i, ap_name ?: "(empty)", NULL, ap_name ? 1 : 0, NULL};
        }
        char *ap_name = rg_settings_get_string(NS_WIFI, "ssid", NULL);
        *opt++ = (rg_gui_option_t){-1, ap_name ?: "(empty)", NULL, ap_name ? 1 : 0, NULL};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog("Select saved AP", options, rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, 0));
        if (sel != RG_DIALOG_CANCELLED)
        {
            rg_settings_set_number(NS_WIFI, SETTING_WIFI_SLOT, sel);
            if (rg_network_wifi_load_config(sel))
            {
                rg_network_wifi_stop();
                rg_network_wifi_start();
            }
        }
        gui_redraw();
    }
    return RG_DIALOG_VOID;
 }

static rg_gui_event_t webui_switch_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        webui_set_switch(!webui_get_switch());
    }
    strcpy(option->value, webui_get_switch() ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_access_point_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        if (rg_gui_confirm("Wi-Fi AP", "Start access point?\n\nSSID: retro-go\nPassword: retro-go", true))
        {
            rg_network_wifi_stop();
            rg_network_wifi_set_config("retro-go", "retro-go", 6, 1);
            rg_network_wifi_start();
        }
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_options_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[] = {
            {0, "Wi-Fi"       , "...", 1, &wifi_switch_cb},
            {0, "Wi-Fi select", "...", 1, &wifi_select_cb},
            {0, "Wi-Fi Access Point", NULL, 1, &wifi_access_point_cb},
            RG_DIALOG_SEPARATOR,
            {0, "File server" , "...", 1, &webui_switch_cb},
            {0, "Time sync" , "On", 0, NULL},
            RG_DIALOG_END,
        };
        rg_gui_dialog("Wifi Options", options, 0);
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t color_theme_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int max = gui_themes_count - 1;

    if (event == RG_DIALOG_PREV && --gui.color_theme < 0)
        gui.color_theme = max;
    if (event == RG_DIALOG_NEXT && ++gui.color_theme > max)
        gui.color_theme = 0;
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
        gui_redraw();

    sprintf(option->value, "%d/%d", gui.color_theme + 1, max + 1);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t startup_app_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const char *modes[] = {"Last game", "Launcher"};
    int max = 1;

    if (event == RG_DIALOG_PREV && --gui.startup_mode < 0)
        gui.startup_mode = max;
    if (event == RG_DIALOG_NEXT && ++gui.startup_mode > max)
        gui.startup_mode = 0;

    strcpy(option->value, modes[gui.startup_mode % (max + 1)]);
    return RG_DIALOG_VOID;
}

static rg_gui_event_t updater_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        updater_show_dialog();
        gui_redraw();
    }
    return RG_DIALOG_VOID;
}

static void show_about_menu(void)
{
    const rg_gui_option_t options[] = {
        {0, "Check for updates", NULL, 1, &updater_cb},
        RG_DIALOG_END,
    };
    rg_gui_about_menu(options);
}

static rg_gui_event_t about_app_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
        show_about_menu();
    return RG_DIALOG_VOID;
}

static void retro_loop(void)
{
    tab_t *tab = NULL;
    int64_t next_repeat = 0;
    int64_t next_idle_event = 0;
    int repeats = 0;
    int joystick, prev_joystick;
    int change_tab = 0;
    int browse_last = -1;
    bool redraw_pending = true;

    gui_init();
    applications_init();
    bookmarks_init();
    music_init();

#ifdef RG_ENABLE_NETWORKING
    rg_network_init();
    wifi_set_switch(wifi_get_switch());
    webui_set_switch(webui_get_switch());
#endif

    tab = gui_get_current_tab();
    if (!tab)
    {
        gui.selected_tab = 0;
        tab = gui_get_current_tab();
    }

    while (true)
    {
        // At the moment the HTTP server has absolute priority because it may change UI elements.
        // It's also risky to let the user do file accesses at the same time (thread safety, SPI, etc)...
        if (gui.http_lock)
        {
            rg_gui_draw_dialog("HTTP Server Busy...", NULL, 0);
            redraw_pending = true;
            while (gui.http_lock)
            {
                rg_task_delay(100);
                rg_system_tick(0);
            }
        }

        if (!tab->enabled && !change_tab)
        {
            change_tab = 1;
        }

        if (change_tab || gui.browse != browse_last)
        {
            if (change_tab)
            {
                gui_event(TAB_LEAVE, tab);
                tab = gui_set_current_tab(gui.selected_tab + change_tab);
                for (int tabs = gui.tabs_count; !tab->enabled && --tabs > 0;)
                    tab = gui_set_current_tab(gui.selected_tab + change_tab);
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
        #if RG_GAMEPAD_HAS_OPTION_BTN
            if (joystick == RG_KEY_MENU)
                show_about_menu();
            else
        #endif
            rg_gui_options_menu();

            gui_set_theme(rg_gui_get_theme());
            gui_save_config();
            rg_settings_commit();
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

        rg_system_tick(0);
    }
}

static void try_migrate(void)
{
    // A handful of retro-go versions used the weird /odroid/*.txt to store books. Let's move them!
    if (rg_settings_get_number(NS_GLOBAL, "Migration", 0) < 1290)
    {
    #ifdef RG_TARGET_ODROID_GO
        rg_storage_mkdir(RG_BASE_PATH_CONFIG);
        rename(RG_STORAGE_ROOT "/odroid/favorite.txt", RG_BASE_PATH_CONFIG "/favorite.txt");
        rename(RG_STORAGE_ROOT "/odroid/recent.txt", RG_BASE_PATH_CONFIG "/recent.txt");
    #endif
        rg_settings_set_number(NS_GLOBAL, "Migration", 1290);
        rg_settings_commit();
    }

    // Some of our save formats have diverged and cause issue when they're shared with Go-Play
    if (rg_settings_get_number(NS_GLOBAL, "Migration", 0) < 1390)
    {
    #ifdef RG_TARGET_ODROID_GO
        if (access(RG_STORAGE_ROOT "/odroid/data", F_OK) == 0)
            rg_gui_alert("Save path changed in 1.32",
                "Save format is no longer fully compatible with Go-Play and can cause corruption.\n\n"
                "Please copy the contents of:\n /odroid/data\nto\n /retro-go/saves.");
    #endif
        rg_settings_set_number(NS_GLOBAL, "Migration", 1390);
        rg_settings_commit();
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
        {0, " - Color    ", "...", 1, &color_theme_cb},
        {0, "Preview     ", "...", 1, &show_preview_cb},
        {0, "Start screen", "...", 1, &start_screen_cb},
        {0, "Hide tabs   ", "...", 1, &toggle_tabs_cb},
        {0, "Startup app ", "...", 1, &startup_app_cb},
        {0, "Timezone    ", "...", 1, &timezone_cb},
        {0, "Wi-Fi options...", NULL,  1, &wifi_options_cb},
    #if !RG_GAMEPAD_HAS_OPTION_BTN
        RG_DIALOG_SEPARATOR,
        {0, "About Retro-Go", NULL,  1, &about_app_cb},
    #endif
        RG_DIALOG_END,
    };

    app = rg_system_init(32000, &handlers, options);
    app->configNs = "launcher";
    app->isLauncher = true;

    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert("SD Card Error", "Storage mount failed.\nMake sure the card is FAT32.");
    }
    else
    {
        rg_storage_mkdir(RG_BASE_PATH_CACHE);
        rg_storage_mkdir(RG_BASE_PATH_CONFIG);
        try_migrate();
    }

#ifdef CONFIG_IDF_TARGET
    // The launcher makes a lot of small allocations and it sometimes fills internal RAM, causing the SD Card driver to
    // stop working. Lowering CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL and manually using rg_alloc to do internal allocs when
    // needed is a better solution, but that would have to be done for every app. This is a good workaround for now.
    heap_caps_malloc_extmem_enable(1024);
#endif

    retro_loop();
}
