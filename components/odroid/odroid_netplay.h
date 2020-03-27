#pragma once

typedef enum {
    NETPLAY_MODE_NONE,
    NETPLAY_MODE_HOST,
    NETPLAY_MODE_GUEST,
} netplay_mode_t;

typedef enum {
    NETPLAY_EVENT_PACKET_RECEIVED,
    NETPLAY_EVENT_STATUS_CHANGED,
} netplay_event_t;

typedef enum {
    NETPLAY_STATUS_NONE,
    NETPLAY_STATUS_LISTENING,
    NETPLAY_STATUS_CONNECTING,
    NETPLAY_STATUS_CONNECTED,
    NETPLAY_STATUS_DISCONNECTED,
    NETPLAY_STATUS_FAILURE,
} netplay_status_t;

typedef enum {
    NETPLAY_PACKET_RAW_DATA,
    NETPLAY_PACKET_SYNC_INPUT,
    NETPLAY_PACKET_SYNC,
    NETPLAY_PACKET_RESET,
} netplay_packet_type_t;

typedef struct {
    void* data;
    short size;
} netplay_packet_t;

typedef void (*netplay_callback_t)(netplay_event_t event, void *arg);

void odroid_netplay_init();
void odroid_netplay_deinit();
void odroid_netplay_set_handler(netplay_callback_t callback);
netplay_callback_t odroid_netplay_get_handler();
bool odroid_netplay_quick_start();
bool odroid_netplay_start(netplay_mode_t mode);
bool odroid_netplay_stop();

netplay_status_t odroid_netplay_status();
netplay_mode_t odroid_netplay_mode();

bool odroid_netplay_send(uint8_t *data, short len);
void odroid_netplay_sync_input(void *data, short len);
void odroid_netplay_sync_frame(void *data);

