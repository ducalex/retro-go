#include "rg_system.h"
#include "rg_network.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRY(x)                           \
    if ((err = (x)) != ESP_OK)           \
    {                                    \
        RG_LOGE("%s = 0x%x\n", #x, err); \
        goto fail;                       \
    }

static rg_network_t network = {0};
static rg_wifi_config_t wifi_config = {0};
static bool initialized = false;

static const char *SETTING_WIFI_SSID = "ssid";
static const char *SETTING_WIFI_PASSWORD = "password";
static const char *SETTING_WIFI_CHANNEL = "channel";
static const char *SETTING_WIFI_MODE = "mode";
static const char *SETTING_WIFI_SLOT = "slot";


#ifdef RG_ENABLE_NETWORKING
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

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
            tcpip_adapter_ip_info_t ip_info;
            if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info) == ESP_OK)
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

            RG_LOGI("Connected! IP: %s, RSSI: %d", network.ip_addr, network.rssi);
            network.state = RG_NETWORK_CONNECTED;
            if (rg_network_sync_time("pool.ntp.org", 0))
                rg_system_save_time();
            rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);
        }
    }

    RG_LOGD("Event: %d %d\n", (int)event_base, (int)event_id);
}
#endif

bool rg_network_wifi_load_config(int slot)
{
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

    char *ssid = rg_settings_get_string(NS_WIFI, key_ssid, NULL);
    char *pass = rg_settings_get_string(NS_WIFI, key_password, NULL);
    int channel = rg_settings_get_number(NS_WIFI, key_channel, 0);
    int ap_mode = rg_settings_get_number(NS_WIFI, key_mode, 0);

    if (!ssid && !pass)
        return false;

    rg_network_wifi_set_config(ssid, pass, channel, ap_mode);
    free(ssid), free(pass);
    return false;
}

bool rg_network_wifi_set_config(const char *ssid, const char *password, int channel, int mode)
{
    snprintf(wifi_config.ssid, 32, "%s", ssid ?: "");
    snprintf(wifi_config.password, 64, "%s", password ?: "");
    wifi_config.channel = channel;
    wifi_config.ap_mode = mode;
    return true;
}

bool rg_network_wifi_start(void)
{
    RG_ASSERT(initialized, "Please call rg_network_init() first");
#ifdef RG_ENABLE_NETWORKING
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
    memset(network.name, 0, 32);
#ifdef RG_ENABLE_NETWORKING
    esp_wifi_stop();
#endif
}

rg_network_t rg_network_get_info(void)
{
    return network;
}

bool rg_network_sync_time(const char *host, int *out_delta)
{
#ifdef RG_ENABLE_NETWORKING
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct hostent *server = gethostbyname(host);
    struct sockaddr_in serv_addr = {};
    struct timeval timeout = {2, 0};
    struct timeval ntp_time = {0, 0};
    struct timeval cur_time;

    if (server == NULL)
    {
        RG_LOGE("Failed to resolve NTP server hostname");
        return false;
    }

    size_t addr_length = RG_MIN(server->h_length, sizeof(serv_addr.sin_addr.s_addr));
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, addr_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(123);

    uint32_t ntp_packet[12] = {0x0000001B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // li, vn, mode.

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    connect(sockfd, (void *)&serv_addr, sizeof(serv_addr));
    send(sockfd, &ntp_packet, sizeof(ntp_packet), 0);

    if (recv(sockfd, &ntp_packet, sizeof(ntp_packet), 0) >= 0)
    {
        ntp_time.tv_sec = ntohl(ntp_packet[10]) - 2208988800UL; // DIFF_SEC_1900_1970;
        ntp_time.tv_usec = (((int64_t)ntohl(ntp_packet[11]) * 1000000) >> 32);

        gettimeofday(&cur_time, NULL);
        settimeofday(&ntp_time, NULL);

        int64_t prev_millis = ((((int64_t)cur_time.tv_sec * 1000000) + cur_time.tv_usec) / 1000);
        int64_t now_millis = ((int64_t)ntp_time.tv_sec * 1000000 + ntp_time.tv_usec) / 1000;
        int ntp_time_delta = (now_millis - prev_millis);

        RG_LOGI("Received Time: %.24s, we were %dms %s\n", ctime(&ntp_time.tv_sec), abs((int)ntp_time_delta),
                ntp_time_delta < 0 ? "ahead" : "behind");

        if (out_delta)
            *out_delta = ntp_time_delta;
        return true;
    }
#endif
    RG_LOGE("Failed to receive NTP time.\n");
    return false;
}

void rg_network_deinit(void)
{
#ifdef RG_ENABLE_NETWORKING
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &network_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
#endif
}

bool rg_network_init(void)
{
    if (initialized)
        return true;

#ifdef RG_ENABLE_NETWORKING
    // Init event loop first
    esp_err_t err;
    TRY(esp_event_loop_create_default());
    TRY(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
    TRY(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &network_event_handler, NULL));

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

    // We try loading the specified slot (if any), and fallback to no slot
    int slot = rg_settings_get_number(NS_WIFI, SETTING_WIFI_SLOT, -1);
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
