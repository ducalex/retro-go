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

#define SETTING_WIFI_ENABLE   "Enable"
#define SETTING_WIFI_SLOT     "Slot"
#define SETTING_WIFI_SSID     "ssid"
#define SETTING_WIFI_PASSWORD "password"
#define SETTING_WIFI_CHANNEL  "channel"
#define SETTING_WIFI_MODE     "mode"

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

static rg_network_state_t network_state = RG_NETWORK_DISABLED;
static rg_wifi_config_t wifi_config = {0};
static esp_netif_t *netif_sta, *netif_ap, *netif;

static void network_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_STOP || event_id == WIFI_EVENT_AP_STOP)
        {
            network_state = RG_NETWORK_DISCONNECTED;
            RG_LOGI("Wifi stopped.");
        }
        else if (event_id == WIFI_EVENT_STA_START)
        {
            network_state = RG_NETWORK_CONNECTING;
            RG_LOGI("Connecting to '%s'...", wifi_config.ssid);
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            network_state = RG_NETWORK_CONNECTING;
            RG_LOGW("Got disconnected from AP. Reconnecting...");
            rg_system_event(RG_EVENT_NETWORK_DISCONNECTED, NULL);
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_AP_START)
        {
            network_state = RG_NETWORK_CONNECTED;
            rg_network_t info = rg_network_get_info();
            RG_LOGI("Access point started! IP: %s", info.ip_addr);
            rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            network_state = RG_NETWORK_CONNECTED;
            rg_network_t info = rg_network_get_info();
            RG_LOGI("Connected! IP: %s, Chan: %d, RSSI: %d", info.ip_addr, info.channel, info.rssi);
            esp_sntp_stop();
            esp_sntp_init();
            rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);
        }
        else if (event_id == IP_EVENT_AP_STAIPASSIGNED)
        {
            ip_event_ap_staipassigned_t* event = (ip_event_ap_staipassigned_t*) event_data;
            RG_LOGI("Access point assigned IP to client: "IPSTR, IP2STR(&event->ip));
        }
    }

    RG_LOGV("Event: %d %d\n", (int)event_base, (int)event_id);
}
#endif

bool rg_network_wifi_read_config(int slot, rg_wifi_config_t *out)
{
    if (slot < 0 || slot > 99)
        return false;

    char key_ssid[16], key_password[16], key_channel[16], key_mode[16];
    rg_wifi_config_t config = {0};
    char *ptr;

    snprintf(key_ssid, 16, "%s%d", SETTING_WIFI_SSID, slot);
    snprintf(key_password, 16, "%s%d", SETTING_WIFI_PASSWORD, slot);
    snprintf(key_channel, 16, "%s%d", SETTING_WIFI_CHANNEL, slot);
    snprintf(key_mode, 16, "%s%d", SETTING_WIFI_MODE, slot);

    RG_LOGD("Looking for '%s' (slot %d)\n", key_ssid, slot);

    if ((ptr = rg_settings_get_string(NS_WIFI, key_ssid, NULL)))
        memccpy(config.ssid, ptr, 0, 32), free(ptr);
    if ((ptr = rg_settings_get_string(NS_WIFI, key_password, NULL)))
        memccpy(config.password, ptr, 0, 64), free(ptr);
    config.channel = rg_settings_get_number(NS_WIFI, key_channel, 0);
    config.ap_mode = rg_settings_get_number(NS_WIFI, key_mode, 0);

    if (!config.ssid[0] && slot == 0)
    {
        if ((ptr = rg_settings_get_string(NS_WIFI, SETTING_WIFI_SSID, NULL)))
            memccpy(config.ssid, ptr, 0, 32), free(ptr);
        if ((ptr = rg_settings_get_string(NS_WIFI, SETTING_WIFI_PASSWORD, NULL)))
            memccpy(config.password, ptr, 0, 64), free(ptr);
        config.channel = rg_settings_get_number(NS_WIFI, SETTING_WIFI_CHANNEL, 0);
        config.ap_mode = rg_settings_get_number(NS_WIFI, SETTING_WIFI_MODE, 0);
    }

    if (!config.ssid[0])
        return false;

    *out = config;
    return true;
}

bool rg_network_wifi_set_config(const rg_wifi_config_t *config)
{
#ifdef RG_ENABLE_NETWORKING
    if (config)
        memcpy(&wifi_config, config, sizeof(wifi_config));
    else
        memset(&wifi_config, 0, sizeof(wifi_config));
    return true;
#else
    return false;
#endif
}

bool rg_network_wifi_start(void)
{
#ifdef RG_ENABLE_NETWORKING
    RG_ASSERT(network_state > RG_NETWORK_DISABLED, "Please call rg_network_init() first");
    wifi_config_t config = {0};
    esp_err_t err;

    if (!wifi_config.ssid[0])
    {
        RG_LOGW("Can't start wifi: No SSID has been configured.\n");
        return false;
    }

    if (wifi_config.ap_mode)
    {
        netif = netif_ap;
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
        netif = netif_sta;
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
    RG_ASSERT(network_state > RG_NETWORK_DISABLED, "Please call rg_network_init() first");
    esp_wifi_stop();
    netif = NULL;
#endif
}

rg_network_t rg_network_get_info(void)
{
    rg_network_t info = {0};
#ifdef RG_ENABLE_NETWORKING
    if (netif)
    {
        memcpy(info.name, wifi_config.ssid, 32);
        info.channel = wifi_config.channel;
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
            snprintf(info.ip_addr, 16, IPSTR, IP2STR(&ip_info.ip));
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        {
            memcpy(info.name, ap_info.ssid, 32);
            info.channel = ap_info.primary;
            info.rssi = ap_info.rssi;
        }
    }
    info.state = network_state;
#endif
    return info;
}

void rg_network_deinit(void)
{
#ifdef RG_ENABLE_NETWORKING
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
    netif = netif_ap = netif_sta = NULL;
    network_state = RG_NETWORK_DISABLED;
#endif
}

bool rg_network_init(void)
{
#ifdef RG_ENABLE_NETWORKING
    if (network_state > RG_NETWORK_DISABLED)
        return true;
    network_state = RG_NETWORK_DISCONNECTED;

    // Init event loop first
    esp_err_t err;
    TRY(esp_event_loop_create_default());
    TRY(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
    TRY(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));

    // Then TCP stack
    TRY(esp_netif_init());
    netif_sta = esp_netif_create_default_wifi_sta();
    netif_ap = esp_netif_create_default_wifi_ap();

    esp_netif_set_hostname(netif_sta, RG_TARGET_NAME);
    esp_netif_set_hostname(netif_ap, RG_TARGET_NAME);

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

    // Load the user's chosen config profile, if any
    int slot = rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, 0);
    rg_network_wifi_read_config(slot, &wifi_config);

    // Auto-start?
    if (rg_settings_get_boolean(NS_WIFI, SETTING_WIFI_ENABLE, false))
        rg_network_wifi_start();

    return true;
fail:
    network_state = RG_NETWORK_DISABLED;
#else
    RG_LOGE("Network was disabled at build time!\n");
#endif
    return false;
}

rg_http_req_t *rg_network_http_open(const char *url, const rg_http_cfg_t *cfg)
{
    RG_ASSERT_ARG(url != NULL);
#ifdef RG_ENABLE_NETWORKING
    rg_http_req_t *req = calloc(1, sizeof(rg_http_req_t));
    if (!req)
    {
        RG_LOGE("Out of memory");
        return NULL;
    }

    req->config = cfg ? *cfg : (rg_http_cfg_t)RG_HTTP_DEFAULT_CONFIG();
    req->client = esp_http_client_init(&(esp_http_client_config_t){
        .url = url,
        .buffer_size = 1024,
        .buffer_size_tx = 1024,
        .method = req->config.post_data ? HTTP_METHOD_POST : HTTP_METHOD_GET,
        .timeout_ms = req->config.timeout_ms,
    });

    if (!req->client)
    {
        RG_LOGE("Error creating client");
        goto fail;
    }

try_again:
    if (esp_http_client_open(req->client, req->config.post_len) != ESP_OK)
    {
        RG_LOGE("Error opening connection");
        goto fail;
    }

    if (req->config.post_data)
    {
        esp_http_client_write(req->client, req->config.post_data, req->config.post_len);
        // Check for errors?
    }

    if (esp_http_client_fetch_headers(req->client) < 0)
    {
        RG_LOGE("Error fetching headers");
        goto fail;
    }

    req->status_code = esp_http_client_get_status_code(req->client);
    req->content_length = esp_http_client_get_content_length(req->client);

    // We must handle redirections manually because we're not using esp_http_client_perform
    if (req->status_code == 301 || req->status_code == 302)
    {
        if (req->redirections < req->config.max_redirections)
        {
            esp_http_client_set_redirection(req->client);
            esp_http_client_close(req->client);
            req->redirections++;
            goto try_again;
        }
    }

    return req;

fail:
    esp_http_client_cleanup(req->client);
    free(req);
#endif
    return NULL;
}

int rg_network_http_read(rg_http_req_t *req, void *buffer, size_t buffer_len)
{
    RG_ASSERT_ARG(req && buffer);
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
