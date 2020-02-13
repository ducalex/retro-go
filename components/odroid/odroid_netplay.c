#include "freertos/FreeRTOS.h"
#include "lwip/ip_addr.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "string.h"

#include "odroid_netplay.h"
#include "odroid_overlay.h"
#include "odroid_settings.h"
#include "odroid_system.h"

ODROID_NETPLAY_MODE netplay_mode = ODROID_NETPLAY_MODE_NULL;

static void odroid_network_task()
{

}

static esp_err_t eventHandler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_GOT_IP:
            //strcpy(_localIP, ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            //*_status = WL_CONNECTED;
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            //*_status = WL_CONNECTED;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            //*_status = WL_CONNECTION_LOST;
            break;
        default:
            break;
    }
    return ESP_OK;
}

void odroid_netplay_init()
{
    printf("odroid_netplay_init: Starting Net Play!\n");

    odroid_dialog_choice_t choices[] = {
        {0, "Host Game", "", 1, NULL},
        {1, "Find Game", "", 1, NULL},
        {2, "Quit", "", 1, NULL},
    };

    switch (odroid_overlay_dialog("Net Play Mode", choices, 3, 0))
    {
        case 0:
            netplay_mode = ODROID_NETPLAY_MODE_SERVER;
            break;
        case 1:
            netplay_mode = ODROID_NETPLAY_MODE_CLIENT;
            break;
        default:
            odroid_system_application_set(0);
            esp_restart();
    }

    tcpip_adapter_init();
}

void odroid_netplay_sync_init(void *data)
{

}

void odroid_netplay_sync_frame(void *data)
{

}
