#include "rg_system.h"
#include "gui.h"
#include "webui.h"
#include "wifi.h"

#ifdef RG_ENABLE_NETWORKING
#include <malloc.h>
#include <string.h>
#include <stdio.h>

static bool wifi_enable = false;
static bool webui_enable = false;

static const char *SETTING_WIFI_ENABLE = "Enable";
static const char *SETTING_WIFI_SLOT = "Slot";
static const char *SETTING_WEBUI = "HTTPFileServer";
static const size_t MAX_AP_LIST = 5;

static void wifi_toggle(bool enable)
{
    if (wifi_enable)
        rg_network_wifi_stop();
    if (enable)
        rg_network_wifi_start();
    if (webui_enable && enable)
        webui_start();
    wifi_enable = enable;
}

static void webui_toggle(bool enable)
{
    if (webui_enable)
        webui_stop();
    if (wifi_enable && enable)
        webui_start();
    webui_enable = enable;
}

static void wifi_toggle_interactive(bool enable)
{
    rg_network_state_t target_state = enable ? RG_NETWORK_CONNECTED : RG_NETWORK_DISCONNECTED;
    int64_t timeout = rg_system_timer() + 20 * 1000000;
    rg_gui_draw_message(enable ? "Connecting..." : "Disconnecting...");
    wifi_toggle(enable);
    do // Always loop at least once, in case we're in a transition
    {
        rg_task_delay(100);
        if (rg_system_timer() > timeout)
            break;
        if (rg_input_read_gamepad())
            break;
    } while (rg_network_get_info().state != target_state);
}

static rg_gui_event_t wifi_switch_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT || event == RG_DIALOG_ENTER)
    {
        wifi_toggle_interactive(!wifi_enable);
        rg_settings_set_number(NS_WIFI, SETTING_WIFI_ENABLE, wifi_enable);
        return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, wifi_enable ? "On " : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t webui_switch_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT || event == RG_DIALOG_ENTER)
    {
        webui_toggle(!webui_enable);
        rg_settings_set_number(NS_APP, SETTING_WEBUI, webui_enable);
    }
    strcpy(option->value, webui_enable ? "On " : "Off");
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
            rg_settings_set_number(NS_WIFI, SETTING_WIFI_ENABLE, true);
            rg_settings_set_number(NS_WIFI, SETTING_WIFI_SLOT, sel);
            rg_network_wifi_stop();
            rg_network_wifi_load_config(sel);
            wifi_toggle_interactive(true);
        }
        return RG_DIALOG_REDRAW;
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_access_point_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        if (rg_gui_confirm("Wi-Fi AP", "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/", true))
        {
            rg_network_wifi_stop();
            rg_network_wifi_set_config(&(const rg_wifi_config_t){
                .ssid = "retro-go",
                .password = "retro-go",
                .channel = 6,
                .ap_mode = true,
            });
            wifi_toggle_interactive(true);
        }
        return RG_DIALOG_REDRAW;
    }
    return RG_DIALOG_VOID;
}

static rg_gui_event_t wifi_status_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    rg_network_t info = rg_network_get_info();
    if (info.state != RG_NETWORK_CONNECTED)
        strcpy(option->value, "Not connected");
    else if (option->arg == 1)
        strcpy(option->value, info.name);
    else if (option->arg == 2)
        strcpy(option->value, info.ip_addr);
    return RG_DIALOG_VOID;
}

void wifi_show_dialog(void)
{
    const rg_gui_option_t options[] = {
        {0, "Wi-Fi       ", "-",  RG_DIALOG_FLAG_NORMAL, &wifi_switch_cb},
        {0, "Wi-Fi select", "-",  RG_DIALOG_FLAG_NORMAL, &wifi_select_cb},
        {0, "Wi-Fi Access Point", NULL, RG_DIALOG_FLAG_NORMAL, &wifi_access_point_cb},
        RG_DIALOG_SEPARATOR,
        {0, "File server" ,  "-", RG_DIALOG_FLAG_NORMAL, &webui_switch_cb},
        {0, "Time sync  " , "On", RG_DIALOG_FLAG_DISABLED, NULL},
        RG_DIALOG_SEPARATOR,
        {1, "Network   " ,  "-", RG_DIALOG_FLAG_MESSAGE, &wifi_status_cb},
        {2, "IP address" ,  "-", RG_DIALOG_FLAG_MESSAGE, &wifi_status_cb},
        RG_DIALOG_END,
    };
    gui_redraw(); // clear main menu
    rg_gui_dialog("Wifi Options", options, 0);
}

void wifi_init(void)
{
    rg_network_init();
    wifi_toggle(rg_settings_get_number(NS_WIFI, SETTING_WIFI_ENABLE, false));
    webui_toggle(rg_settings_get_number(NS_APP, SETTING_WEBUI, true));
}
#endif
