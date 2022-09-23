#include "rg_system.h"
#include "rg_network.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRY(x)                           \
    if ((err = (x)) != ESP_OK) {         \
        RG_LOGE("%s = 0x%x\n", #x, err); \
        goto fail;                       \
    }

static rg_network_t netstate;

static const char *SETTING_WIFI_SSID     = "Wifi.SSID";
static const char *SETTING_WIFI_PASSWORD = "Wifi.Password";


#ifdef RG_ENABLE_WIFI
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        RG_LOGI("Got IP:" IPSTR "\n", IP2STR(&event->ip_info.ip));
        snprintf(netstate.local_addr, 16, IPSTR, IP2STR(&event->ip_info.ip));
        netstate.connected = true;
        netstate.connecting = false;
    }
    RG_LOGI("%d %d\n", (int)event_base, (int)event_id);
}
#endif

rg_network_t rg_network_get_info(void)
{
#ifdef RG_ENABLE_WIFI
    wifi_ap_record_t wifidata;
    if (esp_wifi_sta_get_ap_info(&wifidata) == ESP_OK)
        netstate.rssi = wifidata.rssi;
#endif
    return netstate;
}

void rg_network_wifi_stop(void)
{
    // fail:
    //     return false;
}

bool rg_network_wifi_start(int mode, const char *ssid, const char *password, int channel)
{
    RG_ASSERT(netstate.initialized, "Please call rg_network_init() first");
    if (ssid)
    {
        RG_LOGI("Replacing SSID '%s' with '%s'.\n", netstate.ssid, ssid);
        snprintf(netstate.ssid, 32, "%s", ssid);
        snprintf(netstate.password, 64, "%s", password ?: "");
    }
#ifdef RG_ENABLE_WIFI
    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, netstate.ssid, 32);
    memcpy(wifi_config.sta.password, netstate.password, 64);
    esp_err_t err;
    RG_LOGI("Connecting to '%s'...\n", (char*)wifi_config.sta.ssid);
    TRY(esp_wifi_set_mode(WIFI_MODE_STA));
    TRY(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    TRY(esp_wifi_start());
    TRY(esp_wifi_connect());
    netstate.connecting = true;
    return true;
fail:
#endif
    return false;
}

void rg_network_deinit(void)
{
#ifdef RG_ENABLE_WIFI
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
#endif
}

bool rg_network_init(void)
{
    if (netstate.initialized)
        return true;

    // Preload values from configuration
    char *temp;
    if ((temp = rg_settings_get_string(NS_GLOBAL, SETTING_WIFI_SSID, NULL)))
        snprintf(netstate.ssid, 32, "%s", temp), free(temp);
    if ((temp = rg_settings_get_string(NS_GLOBAL, SETTING_WIFI_PASSWORD, NULL)))
        snprintf(netstate.password, 64, "%s", temp), free(temp);

#ifdef RG_ENABLE_WIFI
    if (nvs_flash_init() != ESP_OK && nvs_flash_erase() == ESP_OK)
        nvs_flash_init();

    esp_err_t err;
    TRY(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    TRY(esp_wifi_init(&cfg));

    TRY(esp_event_loop_create_default());
    TRY(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    TRY(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    netstate.initialized = true;
    return true;
fail:
#endif
    return false;
}
