#pragma once

typedef enum {
    NETPLAY_MODE_NONE,
    NETPLAY_MODE_HOST,
    NETPLAY_MODE_GUEST,
} netplay_mode_t;

typedef enum {
    NETPLAY_EVENT_PACKET_RECEIVED,
    NETPLAY_EVENT_STATUS_CHANGED,
    NETPLAY_EVENT_GAME_RESET,
} netplay_event_t;

typedef enum {
    NETPLAY_STATUS_NOT_INIT,
    NETPLAY_STATUS_STOPPED,
    NETPLAY_STATUS_DISCONNECTED,
    NETPLAY_STATUS_LISTENING,
    NETPLAY_STATUS_CONNECTING,
    NETPLAY_STATUS_HANDSHAKE,
    NETPLAY_STATUS_CONNECTED,
} netplay_status_t;

typedef enum {
    NETPLAY_PACKET_INFO,       // Send protocol version and player struct
    NETPLAY_PACKET_PING,       //
    NETPLAY_PACKET_PONG,       //
    NETPLAY_PACKET_READY,      // Sent by the host once all players are ready
    // Synchronization packets
    NETPLAY_PACKET_SYNC_REQ,   // Sent by the host to all players after reading input
    NETPLAY_PACKET_SYNC_ACK,   // Sent by the guests after reading input and receiving SYNC_REQ
    NETPLAY_PACKET_SYNC_DONE,  // Sent by the host when all guests have ACK, starts frame emulation
    NETPLAY_PACKET_RESYNC,     // Sent when sync is lost, expects an exact frame number to resume at
    // User interaction packets
    NETPLAY_PACKET_GAME_START,  //
    NETPLAY_PACKET_GAME_PAUSE,  //
    NETPLAY_PACKET_GAME_RESET,  //
    NETPLAY_PACKET_INPUT,       // Send gamepad data
    NETPLAY_PACKET_SERIAL,      // Send serial data
    NETPLAY_PACKET_RAW_DATA,    // Send raw data for the emulator to handle (serial, memory copy, etc)
} netplay_packet_type_t;

typedef struct __attribute__ ((packed)) {
    uint8_t player_id;
    uint8_t cmd;
    uint8_t arg; // seq
    uint8_t data_len;
    uint8_t data[128];
} netplay_packet_t;

typedef struct __attribute__ ((packed)) {
    uint8_t  version;
    uint8_t  id;
    uint32_t game_id;
    uint16_t status;
    uint32_t ip_addr;
    uint32_t last_contact;
    uint8_t  sync_data[16];
} netplay_player_t;

typedef void (*netplay_callback_t)(netplay_event_t event, void *arg);

void odroid_netplay_pre_init(netplay_callback_t callback);
bool odroid_netplay_quick_start();
bool odroid_netplay_start(netplay_mode_t mode);
bool odroid_netplay_stop();
void odroid_netplay_sync(void *data_in, void *data_out, uint8_t data_len);

netplay_mode_t odroid_netplay_mode();
netplay_status_t odroid_netplay_status();
