#ifdef RG_ENABLE_NETPLAY

#include <freertos/FreeRTOS.h>
#include <lwip/ip_addr.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>

#include "rg_system.h"
#include "rg_netplay.h"

#define NETPLAY_VERSION 0x01
#define MAX_PLAYERS 8

#define BROADCAST (inet_addr(WIFI_BROADCAST_ADDR))

// The SSID should be randomized to avoid conflicts
#define WIFI_SSID "RETRO-GO"
#define WIFI_CHANNEL 12
#define WIFI_BROADCAST_ADDR "192.168.4.255"
#define WIFI_NETPLAY_PORT 1234

// Test to skip the network task and semaphores
#define NETPLAY_SYNCHRONOUS_TEST

static netplay_status_t netplay_status = NETPLAY_STATUS_NOT_INIT;
static netplay_mode_t netplay_mode = NETPLAY_MODE_NONE;
static netplay_callback_t netplay_callback = NULL;
static SemaphoreHandle_t netplay_sync;
// static bool netplay_available = false;

static netplay_player_t players[MAX_PLAYERS];
static netplay_player_t *local_player;
static netplay_player_t *remote_player; // This only works in 2 player mode

static tcpip_adapter_ip_info_t local_if;
static wifi_config_t wifi_config;

static int rx_sock, tx_sock;


static void dummy_netplay_callback(netplay_event_t event, void *arg)
{
    RG_LOGI("...\n");
}


static void network_cleanup()
{
    if (rx_sock) close(rx_sock);
    if (tx_sock) close(tx_sock);

    rx_sock = tx_sock = 0;
    memset(&local_if, 0, sizeof(local_if));
}


static void network_setup(tcpip_adapter_if_t tcpip_if)
{
    tcpip_adapter_get_ip_info(tcpip_if, &local_if);

    int player_id = ((local_if.ip.addr >> 24) & 0xF) - 1;
    struct sockaddr_in rx_addr;
    int bc_val = 1;

    local_player = &players[player_id];
    local_player->id = player_id;
    local_player->version = NETPLAY_VERSION;
    local_player->game_id = rg_system_get_app()->romCRC32;
    local_player->ip_addr = local_if.ip.addr;

    RG_LOGI("netplay: Local player ID: %d\n", local_player->id);

    rx_addr.sin_family = AF_INET;
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rx_addr.sin_port = htons(WIFI_NETPLAY_PORT);

    rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    tx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(rx_sock > 0 && tx_sock > 0);

    setsockopt(rx_sock, SOL_SOCKET, SO_BROADCAST, &bc_val, sizeof bc_val);
    setsockopt(tx_sock, SOL_SOCKET, SO_BROADCAST, &bc_val, sizeof bc_val);

    if (bind(rx_sock, (struct sockaddr *)&rx_addr, sizeof rx_addr) < 0)
    {
        RG_PANIC("netplay: bind() failed");
    }
}


static void set_status(netplay_status_t status)
{
    bool changed = status != netplay_status;

    netplay_status = status;

    if (changed)
    {
        (*netplay_callback)(RG_EVENT_TYPE_NETPLAY|NETPLAY_EVENT_STATUS_CHANGED, &netplay_status);
    }
}


static inline bool receive_packet(netplay_packet_t *packet, int timeout)
{
    fd_set read_fd_set;

    FD_ZERO(&read_fd_set);
    FD_SET(rx_sock, &read_fd_set);

    int sel = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

    if (sel > 0)
    {
        return recv(rx_sock, &packet, sizeof packet, 0) >= 0;
    }
    else if (sel < 0)
    {
        RG_LOGE("netplay: select() failed\n");
    }

    return false;
}


static inline void send_packet(uint32_t dest, uint8_t cmd, uint8_t arg, void *data, uint8_t data_len)
{
    netplay_packet_t packet = {local_player->id, cmd, arg, data_len, {}};
    size_t len = sizeof(packet) - sizeof(packet.data) + data_len;
    struct sockaddr_in tx_addr;

    if (data_len > 0)
    {
        memcpy(&packet.data, data, data_len);
    }

    if (dest < MAX_PLAYERS)
    {
        tx_addr.sin_family = AF_INET;
        tx_addr.sin_port = htons(WIFI_NETPLAY_PORT);
        tx_addr.sin_addr.s_addr = players[dest].ip_addr;
    }
    else
    {
        tx_addr.sin_family = AF_INET;
        tx_addr.sin_port = htons(WIFI_NETPLAY_PORT);
        tx_addr.sin_addr.s_addr = dest;
    }

    if (sendto(tx_sock, &packet, len, 0, (struct sockaddr*)&tx_addr, sizeof tx_addr) <= 0)
    {
        RG_LOGE("netplay: sendto() failed\n");
        // stop network
    }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_AP_START)
        {
            network_setup(TCPIP_ADAPTER_IF_AP);
            set_status(NETPLAY_STATUS_LISTENING);
        }
        else if (event_id == WIFI_EVENT_AP_STOP || event_id == WIFI_EVENT_STA_STOP)
        {
            set_status(NETPLAY_STATUS_STOPPED);
        }
        else if (event_id == WIFI_EVENT_STA_CONNECTED || event_id == WIFI_EVENT_AP_STACONNECTED)
        {
            set_status(NETPLAY_STATUS_CONNECTING);
        }
        else if (event_id == WIFI_EVENT_AP_STADISCONNECTED || event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            set_status(NETPLAY_STATUS_DISCONNECTED);
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            network_setup(TCPIP_ADAPTER_IF_STA);
            set_status(NETPLAY_STATUS_HANDSHAKE);
        }
        else if (event_id == IP_EVENT_AP_STAIPASSIGNED)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

            send_packet(event->ip_info.ip.addr, NETPLAY_PACKET_INFO,
                                       0, (void*)local_player, sizeof(netplay_player_t));
            set_status(NETPLAY_STATUS_HANDSHAKE);
        }
    }
}


static void netplay_task()
{
    int len, expected_len;
    netplay_packet_t packet;

    RG_LOGI("netplay: Task started!\n");

    while (1)
    {
        memset(&packet, 0, sizeof(netplay_packet_t));

    #ifdef NETPLAY_SYNCHRONOUS_TEST
        if (!rx_sock || netplay_status != NETPLAY_STATUS_HANDSHAKE)
    #else
        if (!rx_sock || netplay_status < NETPLAY_STATUS_HANDSHAKE)
    #endif
        {
            rg_task_delay(100);
            continue;
        }

        // if ((len = recv(client_sock, &packet, sizeof packet, 0)) <= 0)
        if ((len = recvfrom(rx_sock, &packet, sizeof packet, 0, NULL, 0)) <= 0)
        {
            RG_LOGE("netplay: Socket disconnected! (recv() failed)\n");
            rg_task_delay(100);
            continue;
        }

        expected_len = sizeof(packet) - sizeof(packet.data) + packet.data_len;

        if (expected_len != len)
        {
            RG_LOGE("netplay: Packet size mismatch. expected=%d received=%d\n", expected_len, len);
            continue;
        }
        else if (packet.player_id >= MAX_PLAYERS)
        {
            RG_LOGE("netplay: Packet invalid player id: %d\n", packet.player_id);
            continue;
        }
        else if (packet.player_id == local_player->id)
        {
            RG_LOGE("netplay: Received echo!\n");
            continue;
        }

        netplay_player_t *packet_from = &players[packet.player_id];
        packet_from->last_contact = rg_system_timer();

        switch (packet.cmd)
        {
            case NETPLAY_PACKET_INFO: // HOST <-> GUEST
                if (packet.data_len != sizeof(netplay_player_t))
                {
                    RG_LOGE("netplay: Player struct size mismatch. expected=%d received=%d\n",
                            sizeof(netplay_player_t), packet.data_len);
                    break;
                }

                memcpy(packet_from, packet.data, packet.data_len);
                remote_player = packet_from;

                RG_LOGI("netplay: Remote client info player_id=%d game_id=%08X version=%02X\n",
                        packet_from->id, packet_from->game_id, packet_from->version);

                if (packet_from->version != NETPLAY_VERSION)
                {
                    RG_LOGE("netplay: Remote client protocol version mismatch.\n");
                    break;
                }

                if (netplay_mode == NETPLAY_MODE_HOST)
                {
                    // Check if all players are ready (at the moment only 1, no need to check) then send NETPLAY_PACKET_READY
                    send_packet(packet_from->id, NETPLAY_PACKET_READY, 0, 0, 0);
                    set_status(NETPLAY_STATUS_CONNECTED);
                }
                else
                {
                    send_packet(packet_from->id, NETPLAY_PACKET_INFO, 1, (void*)local_player, sizeof(netplay_player_t));
                }
                break;

            case NETPLAY_PACKET_READY: // HOST -> GUEST
                // if (packet.data_len != sizeof(players))
                // {
                //     RG_LOGE("netplay: Players array size mismatch. expected=%d received=%d\n",
                //             sizeof(players), packet.data_len);
                //     break;
                // }

                // memcpy(&players, packet.data, packet.data_len);
                set_status(NETPLAY_STATUS_CONNECTED);
                break;

            case NETPLAY_PACKET_SYNC_REQ: // HOST -> GUEST
                memcpy(&packet_from->sync_data, packet.data, packet.data_len);
                xSemaphoreGive(netplay_sync);
                break;

            case NETPLAY_PACKET_SYNC_ACK: // GUEST -> HOST
                memcpy(&packet_from->sync_data, packet.data, packet.data_len);

                // if received all players ACK then:
                send_packet(remote_player->id, NETPLAY_PACKET_SYNC_DONE, packet.arg, 0, 0);
                xSemaphoreGive(netplay_sync);
                break;

            case NETPLAY_PACKET_SYNC_DONE: // HOST -> GUEST
                xSemaphoreGive(netplay_sync);
                break;

            default:
                RG_LOGE("netplay: Received unknown packet type 0x%02x\n", packet.cmd);
        }
    }
}


static void netplay_init()
{
    RG_LOGI("%s called.\n", __func__);

    if (netplay_status == NETPLAY_STATUS_NOT_INIT)
    {
        netplay_status = NETPLAY_STATUS_STOPPED;
        netplay_callback = netplay_callback ?: dummy_netplay_callback;
        netplay_mode = NETPLAY_MODE_NONE;
        netplay_sync = xSemaphoreCreateMutex();

        tcpip_adapter_init();

        esp_event_loop_create_default();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); // Improves latency a lot
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

        rg_task_create("rg_netplay", &netplay_task, NULL, 4096, RG_TASK_PRIORITY - 2, 1);
    }
}


void rg_netplay_init(netplay_callback_t callback)
{
    RG_LOGI("%s called.\n", __func__);

    netplay_callback = callback;
}


bool rg_netplay_quick_start(void)
{
    const char *status_msg = _("Initializing...");
    const char *screen_msg = NULL;
    // int timeout = 100;

    rg_display_clear(0);

    const rg_gui_option_t options[] = {
        {1, _("Host Game (P1)"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, _("Find Game (P2)"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END
    };

    int ret = rg_gui_dialog(_("Netplay"), options, 0);

    if (ret == 1)
        rg_netplay_start(NETPLAY_MODE_HOST);
    else if (ret == 2)
        rg_netplay_start(NETPLAY_MODE_GUEST);
    else
        return false;

    while (1)
    {
        switch (netplay_status)
        {
            case NETPLAY_STATUS_CONNECTED:
                return remote_player->game_id == local_player->game_id
                    || rg_gui_confirm(_("Netplay"), _("ROMs not identical. Continue?"), 1);
                break;

            case NETPLAY_STATUS_HANDSHAKE:
                status_msg = _("Exchanging info...");
                break;

            case NETPLAY_STATUS_CONNECTING:
                status_msg = _("Connecting...");
                break;

            case NETPLAY_STATUS_DISCONNECTED:
                status_msg = _("Unable to find host!");
                break;

            case NETPLAY_STATUS_STOPPED:
                status_msg = _("Connection failed!");
                break;

            case NETPLAY_STATUS_LISTENING:
                status_msg = _("Waiting for peer...");
                break;

            default:
                status_msg = _("Unknown status...");
        }

        if (screen_msg != status_msg)
        {
            rg_display_clear(0);
            rg_gui_draw_dialog(status_msg, NULL, 0);
            screen_msg = status_msg;
        }

        if (rg_input_key_is_pressed(RG_KEY_B))
            break;

        rg_task_delay(10);
    }

    rg_netplay_stop();

    return false;
}


bool rg_netplay_start(netplay_mode_t mode)
{
    RG_LOGI("%s called.\n", __func__);

    esp_err_t ret = ESP_FAIL;

    if (netplay_status == NETPLAY_STATUS_NOT_INIT)
    {
        netplay_init();
    }
    else if (netplay_mode != NETPLAY_MODE_NONE)
    {
        rg_netplay_stop();
    }

    memset(&players, 0xFF, sizeof(players));
    local_player = NULL;
    remote_player = NULL;

    if (mode == NETPLAY_MODE_GUEST)
    {
        RG_LOGI("netplay: Starting in guest mode.\n");

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
        RG_LOGI("netplay: Starting in host mode.\n");

        strncpy((char*)wifi_config.ap.ssid, WIFI_SSID, 32);
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        wifi_config.ap.channel = WIFI_CHANNEL;
        wifi_config.ap.max_connection = MAX_PLAYERS - 1;
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
        ret = esp_wifi_start();
        netplay_mode = NETPLAY_MODE_HOST;
    }
    else
    {
        RG_PANIC("netplay: Error: Unknown mode!");
    }

    return ret == ESP_OK;
}


bool rg_netplay_stop(void)
{
    RG_LOGI("%s called.\n", __func__);

    esp_err_t ret = ESP_FAIL;

    if (netplay_mode != NETPLAY_MODE_NONE)
    {
        network_cleanup();
        ret = esp_wifi_stop();
        netplay_status = NETPLAY_STATUS_STOPPED;
        netplay_mode = NETPLAY_MODE_NONE;
        xSemaphoreGive(netplay_sync);
    }

    return ret == ESP_OK;
}


void rg_netplay_sync(void *data_in, void *data_out, uint8_t data_len)
{
    static uint32_t sync_count = 0, sync_time = 0, start_time = 0;
    static netplay_packet_t packet;

    if (netplay_status != NETPLAY_STATUS_CONNECTED)
    {
        return;
    }

    start_time = rg_system_timer();

    memcpy(&local_player->sync_data, data_in, data_len);

    if (netplay_mode == NETPLAY_MODE_HOST)
    {
        send_packet(remote_player->id, NETPLAY_PACKET_SYNC_REQ, 0, (void*)data_in, data_len);
    }

#ifdef NETPLAY_SYNCHRONOUS_TEST
    if (netplay_mode == NETPLAY_MODE_HOST)
    {
        do {
            recv(rx_sock, &packet, sizeof packet, 0); // ACK
        } while (packet.cmd != NETPLAY_PACKET_SYNC_ACK);
        // send_packet(remote_player->id, NETPLAY_PACKET_SYNC_DONE, packet.arg, 0, 0);
    }
    else
    {
        do {
            recv(rx_sock, &packet, sizeof packet, 0); // REQ
        } while (packet.cmd != NETPLAY_PACKET_SYNC_REQ);
        send_packet(remote_player->id, NETPLAY_PACKET_SYNC_ACK, 0, (void*)data_in, data_len);
        // recv(rx_sock, &packet, sizeof packet, 0); // DONE
    }

    memcpy(&remote_player->sync_data, packet.data, packet.data_len);
    memcpy(data_out, remote_player->sync_data, data_len);
#else
    // wait to receive/send NETPLAY_PACKET_SYNC_DONE
    if (xSemaphoreTake(netplay_sync, 10000 / portTICK_PERIOD_MS) != pdPASS)
    {
        RG_LOGE("netplay: Lost sync...\n");
        rg_netplay_stop();
        return;
    }

    memcpy(data_out, remote_player->sync_data, data_len);

    if (netplay_mode == NETPLAY_MODE_GUEST)
    {
        send_packet(remote_player->id, NETPLAY_PACKET_SYNC_ACK, 0,
                    local_player->sync_data, sizeof(local_player->sync_data));
        xSemaphoreTake(netplay_sync, 1000 / portTICK_PERIOD_MS);
    }
#endif

    sync_time += rg_system_timer() - start_time;

    if (++sync_count == 60)
    {
        RG_LOGI("netplay: Sync delay=%.4fms\n", (float)sync_time / sync_count / 1000);
        sync_count = sync_time = 0;
    }
}


netplay_mode_t rg_netplay_mode()
{
    return netplay_mode;
}


netplay_status_t rg_netplay_status()
{
    return netplay_status;
}

#endif
