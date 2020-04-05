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

#define NETPLAY_VERSION 0x01
#define MAX_PLAYERS 8

#define EVERYONE 0xFF

// The SSID should be randomized to avoid conflicts
#define WIFI_SSID "RETRO-GO"
#define WIFI_CHANNEL 1
#define WIFI_BROADCAST_ADDR "192.168.4.255"
#define WIFI_NETPLAY_PORT 1234

static netplay_status_t netplay_status = NETPLAY_STATUS_NOT_INIT;
static netplay_mode_t netplay_mode = NETPLAY_MODE_NONE;
static netplay_callback_t netplay_callback = NULL;
static SemaphoreHandle_t netplay_sync;
static bool netplay_available = false;

static netplay_player_t players[MAX_PLAYERS];
static netplay_player_t *local_player;
static netplay_player_t *remote_player; // This only works in 2 player mode

static tcpip_adapter_ip_info_t local_if;
static wifi_config_t wifi_config;

static uint sync_req_time;

static int tx_sock;
static struct sockaddr_in tx_addr;


static void dummy_netplay_callback(netplay_event_t event, void *arg)
{
    printf("dummy_netplay_callback: ...\n");
}


static void set_local_network_interface(tcpip_adapter_if_t tcpip_if)
{
    tcpip_adapter_get_ip_info(tcpip_if, &local_if);
    local_player = &players[local_if.ip.addr - local_if.gw.addr];
    local_player->id = local_if.ip.addr - local_if.gw.addr;
    local_player->version = NETPLAY_VERSION;
    local_player->game_id = odroid_system_get_game_id();
    local_player->ip_addr = local_if.ip.addr;
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    uint temp;

    switch (event->event_id)
    {
        case SYSTEM_EVENT_AP_START:
            netplay_status = NETPLAY_STATUS_LISTENING;
            set_local_network_interface(TCPIP_ADAPTER_IF_AP);
            break;

        case SYSTEM_EVENT_STA_START:
            netplay_status = NETPLAY_STATUS_CONNECTING;
            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
        case SYSTEM_EVENT_STA_CONNECTED:
            netplay_status = NETPLAY_STATUS_CONNECTING;
            break;

        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            netplay_status = NETPLAY_STATUS_CONNECTED;
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            netplay_status = NETPLAY_STATUS_CONNECTED;
            set_local_network_interface(TCPIP_ADAPTER_IF_STA);
            odroid_netplay_send_packet(EVERYONE, NETPLAY_PACKET_INFO, 0, local_player, sizeof(netplay_player_t));
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
        case SYSTEM_EVENT_STA_DISCONNECTED:
            netplay_status = NETPLAY_STATUS_DISCONNECTED;
            // memset(players[ipaddr], 0xFF, sizeof(netplay_player_t));
            break;

        case SYSTEM_EVENT_AP_STOP:
        case SYSTEM_EVENT_STA_STOP:
            netplay_status = NETPLAY_STATUS_STOPPED;
            break;

        default:
            return ESP_OK;
    }

    (*netplay_callback)(NETPLAY_EVENT_STATUS_CHANGED, &netplay_status);

    return ESP_OK;
}


static void netplay_task()
{
    int len, expected_len, rx_sock, temp;
    netplay_packet_t packet;
    struct sockaddr_in rx_addr;
    char buffer[128];

    rx_addr.sin_family = AF_INET;
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rx_addr.sin_port = htons(WIFI_NETPLAY_PORT);

    rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (rx_sock < 0 || bind(rx_sock, (struct sockaddr *)&rx_addr, sizeof rx_addr) < 0)
    {
        abort();
    }

    printf("netplay: Task started!\n");

    while (1)
    {
        memset(&packet, 0, sizeof(netplay_packet_t));

        if ((len = recvfrom(rx_sock, &packet, sizeof packet, 0, NULL, 0)) < 0)
        {
            printf("netplay: Network error in network task!\n");
            vTaskDelay(10);
        }

        expected_len = sizeof(packet) - sizeof(packet.data) + packet.data_len;

        if (expected_len != len)
        {
            printf("netplay: [Error] Packet size mismatch. expected=%d received=%d\n", expected_len, len);
            continue;
        }
        else if (packet.player_id >= MAX_PLAYERS)
        {
            printf("netplay: [Error] Packet invalid player id: %d\n", packet.player_id);
            continue;
        }
        else if (packet.player_id == local_player->id)
        {
            printf("netplay: [Error] Received echo!\n");
            continue;
        }

        remote_player = &players[packet.player_id];
        remote_player->last_contact = get_elapsed_time();

        switch (packet.cmd)
        {
            case NETPLAY_PACKET_INFO:
                if (packet.data_len != sizeof(netplay_player_t))
                {
                    printf("netplay: [Error] Player struct size mismatch. expected=%d received=%d\n",
                            sizeof(netplay_player_t), packet.data_len);
                    break; // abort();
                }

                if (((netplay_player_t*)packet.data)->version != NETPLAY_VERSION)
                {
                    printf("netplay: [Error] Protocol version mismatch. expected=%d received=%d\n",
                            NETPLAY_VERSION, ((netplay_player_t*)packet.data)->version);
                    break; // abort();
                }

                // If it's a new player (to us) we send our information back
                if (remote_player->id == 0xFF)
                {
                    memcpy(remote_player, packet.data, packet.data_len);
                    odroid_netplay_send_packet(remote_player->id, NETPLAY_PACKET_INFO, 0,
                                               local_player, sizeof(netplay_player_t));
                }
                else
                {
                    memcpy(remote_player, packet.data, packet.data_len);
                }

                // If remote player is Host then his Game ID is decisive
                if (remote_player->id <= 1)
                {
                    if (remote_player->game_id != local_player->game_id)
                    {
                        printf("netplay: Game ID mismatch. received: %08X, expected %08X\n",
                                remote_player->game_id, local_player->game_id);
                        // show warning;
                    }
                }

                printf("netplay: Remote client info player_id=%d game_id=%08X\n",
                        remote_player->id, remote_player->game_id);
                break;

            case NETPLAY_PACKET_SYNC_REQ:
                memcpy(&remote_player->sync_data, packet.data, packet.data_len);
                odroid_netplay_send_packet(0xFF, NETPLAY_PACKET_SYNC_ACK, packet.arg,
                                           local_player->sync_data, sizeof(local_player->sync_data));
                break;

            case NETPLAY_PACKET_SYNC_ACK:
                memcpy(&remote_player->sync_data, packet.data, packet.data_len);
                remote_player->sync_latency = get_elapsed_time_since(sync_req_time);

                if (netplay_mode == NETPLAY_MODE_HOST)
                {
                    // if received all players ACK then:
                    odroid_netplay_send_packet(0xFF, NETPLAY_PACKET_SYNC_DONE, packet.arg, 0, 0);
                    xSemaphoreGive(netplay_sync);
                }
                break;

            case NETPLAY_PACKET_SYNC_DONE:
                xSemaphoreGive(netplay_sync);
                break;

            case NETPLAY_PACKET_RAW_DATA:
                (*netplay_callback)(NETPLAY_EVENT_PACKET_RECEIVED, &packet);

            default:
                printf("netplay: [Error] Received unknown packet type 0x%02x\n", packet.cmd);
        }
    }

    vTaskDelete(NULL);
}


void odroid_netplay_pre_init(netplay_callback_t callback)
{
    printf("netplay: %s called.\n", __func__);

    netplay_callback = callback;
    netplay_available = true;
}


void odroid_netplay_init()
{
    printf("netplay: %s called.\n", __func__);

    if (netplay_status == NETPLAY_STATUS_NOT_INIT)
    {
        netplay_status = NETPLAY_STATUS_STOPPED;
        netplay_callback = netplay_callback ?: dummy_netplay_callback;
        netplay_mode = NETPLAY_MODE_NONE;
        netplay_sync = xSemaphoreCreateMutex();

        tcpip_adapter_init();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_event_loop_init(&event_handler, NULL));

        tx_addr.sin_family = AF_INET;
        tx_addr.sin_addr.s_addr = inet_addr(WIFI_BROADCAST_ADDR);
        tx_addr.sin_port = htons(WIFI_NETPLAY_PORT);

        tx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        assert(tx_sock > 0);

        xTaskCreatePinnedToCore(&netplay_task, "netplay_task", 4096, NULL, 7, NULL, 1);
    }
}


void odroid_netplay_deinit()
{
    printf("netplay: %s called.\n", __func__);
}


bool odroid_netplay_available()
{
    return netplay_available;
}


bool odroid_netplay_quick_start()
{
    const char *status_msg = "Initializing...";
    const char *screen_msg = NULL;
    short timeout = 100;
    odroid_gamepad_state joystick;

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
            status_msg = "Exchanging info...";
            if (remote_player != NULL)
            {
                return true;
            }
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

    if (netplay_status == NETPLAY_STATUS_NOT_INIT)
    {
        odroid_netplay_init();
    }
    else if (netplay_mode != NETPLAY_MODE_NONE)
    {
        odroid_netplay_stop();
    }

    memset(&players, 0xFF, sizeof(players));
    local_player = NULL;
    remote_player = NULL;

    if (mode == NETPLAY_MODE_GUEST)
    {
        printf("netplay: Starting in guest mode.\n");

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
        printf("netplay: Starting in host mode.\n");

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
        abort();
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
        netplay_status = NETPLAY_STATUS_STOPPED;
        netplay_mode = NETPLAY_MODE_NONE;
        xSemaphoreGive(netplay_sync);
    }

    return ret == ESP_OK;
}


void odroid_netplay_sync(void *data_in, void *data_out, uint8_t data_len)
{
    static uint sync_count = 0, sync_time = 0;
    uint start_time = get_elapsed_time();

    if (netplay_status != NETPLAY_STATUS_CONNECTED)
    {
        return;
    }

    memcpy(&local_player->sync_data, data_in, data_len);

    if (netplay_mode == NETPLAY_MODE_HOST)
    {
        sync_req_time = get_elapsed_time();
        odroid_netplay_send_packet(0xFF, NETPLAY_PACKET_SYNC_REQ, 0, local_player->sync_data,
                                   sizeof(local_player->sync_data));
    }

    // wait to receive/send NETPLAY_PACKET_SYNC_DONE
    if (xSemaphoreTake(netplay_sync, 100 / portTICK_PERIOD_MS) == pdPASS)
    {
        memcpy(data_out, remote_player->sync_data, data_len);
    }
    else
    {
        printf("netplay: [Error] Lost sync...\n");
        // abort();
    }

    sync_time += get_elapsed_time_since(start_time);

    if (++sync_count == 60)
    {
        printf("netplay: Sync delay=%.4fms", (float)sync_time / sync_count / 1000);
        sync_count = sync_time = 0;
    }
}


bool odroid_netplay_send_packet(uint8_t dest, uint8_t cmd, uint8_t arg, void *data, uint8_t data_len)
{
    printf("netplay: %s called.\n", __func__);

    if (netplay_status != NETPLAY_STATUS_CONNECTED)
    {
        return false;
    }

    // To do: avoid calling inet_addr every time
    if (dest < MAX_PLAYERS) tx_addr.sin_addr.s_addr = players[dest].ip_addr;
    else tx_addr.sin_addr.s_addr = inet_addr(WIFI_BROADCAST_ADDR);

    netplay_packet_t packet = {local_player->id, arg, cmd, data_len};
    size_t len = sizeof(packet) - sizeof(packet.data) + data_len;

    if (data_len) {
        memcpy(packet.data, data, data_len);
    }

    return sendto(tx_sock, &packet, len, 0, (struct sockaddr*)&tx_addr, sizeof tx_addr) >= 0;
}


netplay_mode_t odroid_netplay_mode()
{
    return netplay_mode;
}


netplay_status_t odroid_netplay_status()
{
    return netplay_status;
}
