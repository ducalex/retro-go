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

static rg_network_t netstate = {0};
static bool initialized = false;

static const char *SETTING_WIFI_SSID = "ssid";
static const char *SETTING_WIFI_PASSWORD = "password";


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
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        RG_LOGE("Got disconnected from AP. Reconnecting...\n");
        netstate.state = RG_WIFI_CONNECTING;
        rg_system_event(RG_EVENT_NETWORK_DISCONNECTED, NULL);
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        wifi_ap_record_t wifidata;
        snprintf(netstate.local_addr, 16, IPSTR, IP2STR(&event->ip_info.ip));
        if (esp_wifi_sta_get_ap_info(&wifidata) == ESP_OK)
            netstate.rssi = wifidata.rssi;
        netstate.state = RG_WIFI_CONNECTED;
        RG_LOGI("Connected! IP: %s, RSSI: %d", netstate.local_addr, netstate.rssi);
        if (rg_network_sync_time("pool.ntp.org", 0))
            rg_system_save_time();
        rg_system_event(RG_EVENT_NETWORK_CONNECTED, NULL);
    }
    else
    {
        RG_LOGI("Event: %d %d\n", (int)event_base, (int)event_id);
    }
}
#endif

void rg_network_wifi_stop(void)
{
#ifdef RG_ENABLE_NETWORKING
    esp_wifi_stop();
    netstate.state = RG_WIFI_DISCONNECTED;
#endif
}

bool rg_network_wifi_start(const char *ssid, const char *password, int channel)
{
    RG_ASSERT(initialized, "Please call rg_network_init() first");
    if (ssid)
    {
        RG_LOGI("Replacing SSID '%s' with '%s'.\n", netstate.ssid, ssid);
        snprintf(netstate.ssid, 32, "%s", ssid);
        snprintf(netstate.password, 64, "%s", password ?: "");
    }
    if (!netstate.ssid[0])
    {
        RG_LOGW("Can't start wifi: No SSID has been configured.\n");
        return false;
    }
#ifdef RG_ENABLE_NETWORKING
    wifi_config_t wifi_config = {0};
    memcpy(wifi_config.sta.ssid, netstate.ssid, 32);
    memcpy(wifi_config.sta.password, netstate.password, 64);
    esp_err_t err;
    RG_LOGI("Connecting to '%s' with password '%s'...\n", (char *)wifi_config.sta.ssid,
            (char *)wifi_config.sta.password);
    TRY(esp_wifi_set_mode(WIFI_MODE_STA));
    TRY(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    TRY(esp_wifi_start());
    netstate.state = RG_WIFI_CONNECTING;
    return true;
fail:
#endif
    return false;
}

rg_network_t rg_network_get_info(void)
{
#ifdef RG_ENABLE_NETWORKING
#endif
    return netstate;
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

    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
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

    // Preload values from configuration
    char *temp;
    if ((temp = rg_settings_get_string(NS_WIFI, SETTING_WIFI_SSID, NULL)))
        snprintf(netstate.ssid, 32, "%s", temp), free(temp);
    if ((temp = rg_settings_get_string(NS_WIFI, SETTING_WIFI_PASSWORD, NULL)))
        snprintf(netstate.password, 64, "%s", temp), free(temp);

#ifdef RG_ENABLE_NETWORKING
    if (nvs_flash_init() != ESP_OK && nvs_flash_erase() == ESP_OK)
        nvs_flash_init();

    esp_netif_init();

    esp_err_t err;
    TRY(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    TRY(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
    TRY(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &network_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    TRY(esp_wifi_init(&cfg));

    initialized = true;
    return true;
fail:
#else
    RG_LOGE("Network was disabled at build time!\n");
#endif
    return false;
}
