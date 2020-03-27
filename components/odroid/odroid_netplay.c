#include "freertos/FreeRTOS.h"
#include "lwip/ip_addr.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "string.h"
#include "unistd.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "odroid_system.h"

static netplay_status_t netplay_status = NETPLAY_STATUS_NONE;
static netplay_mode_t netplay_mode = NETPLAY_MODE_NONE;
static netplay_callback_t netplay_callback = NULL;

static wifi_config_t wifi_config;

static SemaphoreHandle_t netplay_sync;

static bool netplay_initialized = false;
static int  netplay_client_id = 0;

static int sockSnd, sockRcv;
struct sockaddr_in addrSnd, addrRcv;

// The SSID should be randomized to avoid conflicts
#define WIFI_SSID "RETRO-GO"
#define WIFI_CHANNEL 1

#define WIFI_HOST_ADDR  "192.168.4.1"
#define WIFI_GUEST_ADDR "192.168.4.2"

// We use UDP broadcasts so we don't have to track IPs and TCP status
#define WIFI_BROADCAST_ADDR "192.168.4.255"
#define WIFI_BROADCAST_PORT 1234


static esp_err_t eventHandler(void *ctx, system_event_t *event)
{
    netplay_status_t *_status = (netplay_status_t*)ctx;
    switch (event->event_id)
    {
        case SYSTEM_EVENT_AP_START:
            *_status = NETPLAY_STATUS_LISTENING;
            break;

        case SYSTEM_EVENT_STA_START:
        case SYSTEM_EVENT_AP_STACONNECTED:
        case SYSTEM_EVENT_STA_CONNECTED:
            *_status = NETPLAY_STATUS_CONNECTING;
            break;

        case SYSTEM_EVENT_AP_STAIPASSIGNED:
        case SYSTEM_EVENT_STA_GOT_IP:
            *_status = NETPLAY_STATUS_CONNECTED;
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
        case SYSTEM_EVENT_STA_DISCONNECTED:
            *_status = NETPLAY_STATUS_DISCONNECTED;
            break;

        case SYSTEM_EVENT_AP_STOP:
        case SYSTEM_EVENT_STA_STOP:
            *_status = NETPLAY_STATUS_NONE;
            break;

        default:
            return ESP_OK;
    }

    if (netplay_callback)
    {
        (*netplay_callback)(NETPLAY_EVENT_STATUS_CHANGED, _status);
    }

    return ESP_OK;
}


static void network_task()
{
    uint8_t buffer[256];
    int len;

    sockSnd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(sockSnd > 0);
    sockRcv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(sockRcv > 0);

    addrSnd.sin_family = AF_INET;
    addrSnd.sin_addr.s_addr = inet_addr(WIFI_BROADCAST_ADDR);
    addrSnd.sin_port = htons(WIFI_BROADCAST_PORT);

    addrRcv.sin_family = AF_INET;
    addrRcv.sin_addr.s_addr = htonl(INADDR_ANY);
    addrRcv.sin_port = htons(WIFI_BROADCAST_PORT);

    if (bind(sockRcv, (struct sockaddr *) &addrRcv, sizeof addrRcv) < 0)
        abort();

    printf("netplay: Network task started and listening!\n");

    while (1)
    {
        if ((len = recvfrom(sockRcv, buffer, sizeof buffer, 0, NULL, 0)) < 0)
        {
            printf("netplay: Network error in network task!\n");
            vTaskDelay(10);
        }

        printf("Packet: %s\n", buffer);

        switch (buffer[0])
        {
            case NETPLAY_PACKET_RESET:
                break;

            case NETPLAY_PACKET_SYNC:
                xSemaphoreGive(netplay_sync);
                break;

            default:
                if (netplay_callback)
                {
                    (*netplay_callback)(NETPLAY_EVENT_PACKET_RECEIVED, &buffer);
                }
        }
    }

    vTaskDelete(NULL);
}


void odroid_netplay_init()
{
    printf("netplay: %s called.\n", __func__);

    if (!netplay_initialized)
    {
        netplay_status = NETPLAY_STATUS_NONE;
        netplay_mode = NETPLAY_MODE_NONE;
        netplay_initialized = true;
        netplay_client_id = esp_random();
        netplay_sync = xSemaphoreCreateMutex();

        tcpip_adapter_init();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_event_loop_init(&eventHandler, &netplay_status));

        xTaskCreatePinnedToCore(&network_task, "network_task", 4096, NULL, 7, NULL, 1);
    }
}


void odroid_netplay_deinit()
{
    printf("netplay: %s called.\n", __func__);
}


void odroid_netplay_set_handler(netplay_callback_t callback)
{
    netplay_callback = callback;
}


netplay_callback_t odroid_netplay_get_handler()
{
    return netplay_callback;
}


netplay_status_t odroid_netplay_status()
{
    // printf("netplay: %s called.\n", __func__);

    return netplay_status;
}


netplay_mode_t odroid_netplay_mode()
{
    // printf("netplay: %s called.\n", __func__);

    return netplay_mode;
}


bool odroid_netplay_quick_start()
{
    char *status_msg = "Initializing...";
    char *screen_msg = NULL;
    short timeout = 100;
    odroid_gamepad_state joystick;

    if (!netplay_initialized)
    {
        odroid_netplay_init();
    }

    if (netplay_status == NETPLAY_STATUS_CONNECTED)
    {
        if (odroid_overlay_confirm("Netplay connected. Disconnect?", true))
        {
            odroid_netplay_stop();
        }
        return false;
    }

    odroid_display_clear(0);

    const odroid_dialog_choice_t choices[] = {
        {1, "Host Game (P1)", "", 1, NULL},
        {2, "Find Game (P2)", "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    int ret = odroid_overlay_dialog("Netplay", choices, 0);

    if (ret == 1)
        odroid_netplay_start(NETPLAY_MODE_HOST);
    else if (ret == 2)
        odroid_netplay_start(NETPLAY_MODE_GUEST);
    else
        return false;

    while (1)
    {
        if (netplay_status == NETPLAY_STATUS_CONNECTED)
        {
            return true;
        }
        else if (netplay_status == NETPLAY_STATUS_DISCONNECTED)
        {
            status_msg = "Unable to find host.";
        }
        else if (netplay_status == NETPLAY_STATUS_CONNECTING)
        {
            status_msg = "Connecting...";
        }
        else if (netplay_status == NETPLAY_STATUS_LISTENING)
        {
            status_msg = "Waiting for peer...";
        }

        if (screen_msg != status_msg)
        {
            odroid_display_clear(0);
            odroid_overlay_draw_dialog(status_msg, NULL, 0);
            screen_msg = status_msg;
        }

        odroid_input_gamepad_read(&joystick);

        if (joystick.values[ODROID_INPUT_B]) break;

        vTaskDelay(10 / portTICK_RATE_MS);
    }

    odroid_netplay_stop();

    return false;
}


bool odroid_netplay_start(netplay_mode_t mode)
{
    printf("netplay: %s called.\n", __func__);

    esp_err_t ret = ESP_FAIL;

    if (netplay_mode != NETPLAY_MODE_NONE)
        odroid_netplay_stop();

    if (mode == NETPLAY_MODE_GUEST)
    {
        printf("netplay: Starting in client mode.\n");

        strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, 32);
        wifi_config.sta.channel = WIFI_CHANNEL;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ret = esp_wifi_connect();
        netplay_mode = NETPLAY_MODE_GUEST;
    }
    else if (mode == NETPLAY_MODE_HOST)
    {
        printf("netplay: Starting in server mode.\n");

        strncpy((char*)wifi_config.ap.ssid, WIFI_SSID, 32);
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        wifi_config.ap.channel = WIFI_CHANNEL;
        wifi_config.ap.max_connection = 1;
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
        ret = esp_wifi_start();
        netplay_mode = NETPLAY_MODE_HOST;
    }
    else
    {
        printf("netplay: Error: Unknown mode!\n");
        // netplay_mode == NETPLAY_MODE_NONE;
    }

    return ret == ESP_OK;
}


bool odroid_netplay_stop()
{
    printf("netplay: %s called.\n", __func__);

    esp_err_t ret = ESP_FAIL;

    if (netplay_mode != NETPLAY_MODE_NONE)
    {
        ret = esp_wifi_stop();
        netplay_status = NETPLAY_STATUS_NONE;
        netplay_mode = NETPLAY_MODE_NONE;
        xSemaphoreGive(netplay_sync);
    }

    return ret == ESP_OK;
}


bool odroid_netplay_send(uint8_t *data, short len)
{
    printf("netplay: %s called.\n", __func__);

    if (netplay_status != NETPLAY_STATUS_CONNECTED)
        return false;

    return sendto(sockSnd, data, len, 0, (struct sockaddr*)&addrSnd, sizeof addrSnd) >= 0;
}


void odroid_netplay_sync_input(void *data, short len)
{
    // printf("netplay: %s called.\n", __func__);
    odroid_netplay_send(data, len);

    // Host player controls the flow
    if (netplay_mode == NETPLAY_MODE_HOST)
    {
        uint8_t sync[] = {0xFF};
        odroid_netplay_send(sync, 1);
    }
    else
    {
        // if (xSemaphoreTake(netplay_sync, 100) != pdPASS)
        // {
        //     printf("netplay: odroid_netplay_sync_input failed!\n");
        //     // abort();
        // }
    }
}


void odroid_netplay_sync_frame(void *data)
{
    printf("netplay: %s called.\n", __func__);
}
