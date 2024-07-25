#include "rg_system.h"
#include "rg_network.h"

#include <stdlib.h>
#include <string.h>

#define TRY(x)                           \
    if ((err = (x)) != ESP_OK)           \
    {                                    \
        RG_LOGE("%s = 0x%x\n", #x, err); \
        goto fail;                       \
    }

#ifdef RG_ENABLE_NETWORKING
#include <esp_idf_version.h>
#include <esp_http_client.h>
#include <esp_system.h>
#include <esp_sntp.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)
#define esp_sntp_init sntp_init
#define esp_sntp_stop sntp_stop
#define esp_sntp_setoperatingmode sntp_setoperatingmode
#define esp_sntp_setservername sntp_setservername
#endif

static rg_network_t network = {0};
static rg_wifi_config_t wifi_config = {0};
static bool initialized = false;

static const char *SETTING_WIFI_SSID = "ssid";
static const char *SETTING_WIFI_PASSWORD = "password";
static const char *SETTING_WIFI_CHANNEL = "channel";
static const char *SETTING_WIFI_MODE = "mode";
static const char *SETTING_WIFI_SLOT = "slot";

static void network_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_STOP || event_id == WIFI_EVENT_AP_STOP)
        {
            RG_LOGI("Wifi stopped.\n");
            network.state = RG_NETWORK_DISCONNECTED;
        }
        else if (event_id == WIFI_EVENT_STA_START)
        {
            RG_LOGI("Connecting to '%s'...\n", wifi_config.ssid);
            network.state = RG_NETWORK_CONNECTING;
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            RG_LOGW("Got disconnected from AP. Reconnecting...\n");
            network.state = RG_NETWORK_CONNECTING;
            rg_system_event(RG_EVENT_NETWORK_DISCONNECTED, NULL);
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_AP_START)
        {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info) == ESP_OK)
                snprintf(network.ip_addr, 16, IPSTR, IP2STR(&ip_info.ip));

            RG_LOGI("Access point started! IP: %s\n", network.ip_addr);
            network.state = RG_NETWORK_CONNECTED;
            rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            snprintf(network.ip_addr, 16, IPSTR, IP2STR(&event->ip_info.ip));

            wifi_ap_record_t wifidata;
            if (esp_wifi_sta_get_ap_info(&wifidata) == ESP_OK)
            {
                network.channel = wifidata.primary;
                network.rssi = wifidata.rssi;
            }
            network.state = RG_NETWORK_CONNECTED;

            RG_LOGI("Connected! IP: %s, RSSI: %d", network.ip_addr, network.rssi);
            rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);

            esp_sntp_stop();
            esp_sntp_init();
        }
        else if (event_id == IP_EVENT_AP_STAIPASSIGNED)
        {
            ip_event_ap_staipassigned_t* event = (ip_event_ap_staipassigned_t*) event_data;
            snprintf(network.ip_addr, 16, IPSTR, IP2STR(&event->ip));
            RG_LOGI("Access point assigned IP to client: %s", network.ip_addr);
        }
    }

    RG_LOGV("Event: %d %d\n", (int)event_base, (int)event_id);
}
#endif

bool rg_network_wifi_load_config(int slot)
{
#ifdef RG_ENABLE_NETWORKING
    char key_ssid[16], key_password[16], key_channel[16], key_mode[16];

    if (slot < -1 || slot > 999)
        return false;

    if (slot == -1)
    {
        snprintf(key_ssid, 16, "%s", SETTING_WIFI_SSID);
        snprintf(key_password, 16, "%s", SETTING_WIFI_PASSWORD);
        snprintf(key_channel, 16, "%s", SETTING_WIFI_CHANNEL);
        snprintf(key_mode, 16, "%s", SETTING_WIFI_MODE);
    }
    else
    {
        snprintf(key_ssid, 16, "%s%d", SETTING_WIFI_SSID, slot);
        snprintf(key_password, 16, "%s%d", SETTING_WIFI_PASSWORD, slot);
        snprintf(key_channel, 16, "%s%d", SETTING_WIFI_CHANNEL, slot);
        snprintf(key_mode, 16, "%s%d", SETTING_WIFI_MODE, slot);
    }

    RG_LOGI("Looking for '%s' (slot %d)\n", key_ssid, slot);
    rg_wifi_config_t config = {0};
    char *ptr;

    if ((ptr = rg_settings_get_string(NS_WIFI, key_ssid, NULL)))
        memccpy(config.ssid, ptr, 0, 32), free(ptr);
    if ((ptr = rg_settings_get_string(NS_WIFI, key_password, NULL)))
        memccpy(config.password, ptr, 0, 64), free(ptr);
    config.channel = rg_settings_get_number(NS_WIFI, key_channel, 0);
    config.ap_mode = rg_settings_get_number(NS_WIFI, key_mode, 0);

    if (!config.ssid[0])
        return false;

    return rg_network_wifi_set_config(&config);
#else
    return false;
#endif
}

bool rg_network_wifi_set_config(const rg_wifi_config_t *config)
{
#ifdef RG_ENABLE_NETWORKING
    RG_ASSERT(config, "bad param");
    wifi_config = *config;
    return true;
#else
    return false;
#endif
}

bool rg_network_wifi_start(void)
{
#ifdef RG_ENABLE_NETWORKING
    RG_ASSERT(initialized, "Please call rg_network_init() first");
    wifi_config_t config = {0};
    esp_err_t err;

    if (!wifi_config.ssid[0])
    {
        RG_LOGW("Can't start wifi: No SSID has been configured.\n");
        return false;
    }

    memcpy(network.name, wifi_config.ssid, 32);

    if (wifi_config.ap_mode)
    {
        memcpy(config.ap.ssid, wifi_config.ssid, 32);
        memcpy(config.ap.password, wifi_config.password, 64);
        config.ap.authmode = wifi_config.password[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
        config.ap.channel = wifi_config.channel;
        config.ap.max_connection = 1;
        TRY(esp_wifi_set_mode(WIFI_MODE_AP));
        TRY(esp_wifi_set_config(WIFI_IF_AP, &config));
        TRY(esp_wifi_start());
    }
    else
    {
        memcpy(config.sta.ssid, wifi_config.ssid, 32);
        memcpy(config.sta.password, wifi_config.password, 64);
        config.sta.channel = wifi_config.channel;
        TRY(esp_wifi_set_mode(WIFI_MODE_STA));
        TRY(esp_wifi_set_config(WIFI_IF_STA, &config));
        TRY(esp_wifi_start());
    }
    return true;
fail:
#endif
    return false;
}

void rg_network_wifi_stop(void)
{
#ifdef RG_ENABLE_NETWORKING
    RG_ASSERT(initialized, "Please call rg_network_init() first");
    esp_wifi_stop();
    rg_task_delay(100);
    memset(network.name, 0, sizeof(network.name));
#endif
}

rg_network_t rg_network_get_info(void)
{
#ifdef RG_ENABLE_NETWORKING
    return network;
#else
    return (rg_network_t){0};
#endif
}

void rg_network_deinit(void)
{
#ifdef RG_ENABLE_NETWORKING
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
#endif
}

bool rg_network_init(void)
{
#ifdef RG_ENABLE_NETWORKING
    if (initialized)
        return true;

    // Init event loop first
    esp_err_t err;
    TRY(esp_event_loop_create_default());
    TRY(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
    TRY(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));

    // Then TCP stack
    esp_netif_init();
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    // Wifi may use nvs for calibration data
    if (nvs_flash_init() != ESP_OK && nvs_flash_erase() == ESP_OK)
        nvs_flash_init();

    // Initialize wifi driver (it won't enable the radio yet)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    TRY(esp_wifi_init(&cfg));
    TRY(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Setup SNTP client but don't query it yet
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    // esp_sntp_init();

    // Tell rg_network_get_info() that we're enabled but not yet connected
    network.state = RG_NETWORK_DISCONNECTED;

    // We try loading the specified slot (if any), and fallback to no slot
    int slot = rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, 0);
    if (!rg_network_wifi_load_config(slot) && slot != -1)
        rg_network_wifi_load_config(-1);

    initialized = true;
    return true;
fail:
#else
    RG_LOGE("Network was disabled at build time!\n");
#endif
    return false;
}

rg_http_req_t *rg_network_http_open(const char *url, const rg_http_cfg_t *cfg)
{
    RG_ASSERT(url, "bad param");
#ifdef RG_ENABLE_NETWORKING
    esp_http_client_config_t http_config = {.url = url, .buffer_size = 1024, .buffer_size_tx = 1024};
    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    rg_http_req_t *req = calloc(1, sizeof(rg_http_req_t));

    if (!http_client || !req)
    {
        RG_LOGE("Error creating client");
        goto fail;
    }

try_again:
    if (esp_http_client_open(http_client, 0) != ESP_OK)
    {
        RG_LOGE("Error opening connection");
        goto fail;
    }

    if (esp_http_client_fetch_headers(http_client) < 0)
    {
        RG_LOGE("Error fetching headers");
        goto fail;
    }

    req->status_code = esp_http_client_get_status_code(http_client);
    req->content_length = esp_http_client_get_content_length(http_client);
    req->client = (void *)http_client;

    if (req->status_code == 301 || req->status_code == 302)
    {
        if (req->redirections < 5)
        {
            esp_http_client_set_redirection(http_client);
            esp_http_client_close(http_client);
            req->redirections++;
            goto try_again;
        }
    }

    return req;

fail:
    esp_http_client_cleanup(http_client);
    free(req);
#endif
    return NULL;
}

int rg_network_http_read(rg_http_req_t *req, void *buffer, size_t buffer_len)
{
    RG_ASSERT(req && buffer, "bad param");
#ifdef RG_ENABLE_NETWORKING
    // if (req->content_length >= 0 && req->received_bytes >= req->content_length)
    //     return 0;
    int len = esp_http_client_read_response(req->client, buffer, buffer_len);
    if (len > 0)
        req->received_bytes += len;
    else
        esp_http_client_close(req->client);
    return len;
#else
    return -1;
#endif
}

void rg_network_http_close(rg_http_req_t *req)
{
#ifdef RG_ENABLE_NETWORKING
    if (req == NULL)
        return;
    esp_http_client_cleanup(req->client);
    req->client = NULL;
    free(req);
#endif
}
