#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_WIFI_DISCONNECTED,
    RG_WIFI_CONNECTING,
    RG_WIFI_CONNECTED,
} rg_wifi_state_t;

#define RG_EVENT_NETWORK_DISCONNECTED (RG_EVENT_TYPE_NETWORK | 1)
#define RG_EVENT_NETWORK_CONNECTED    (RG_EVENT_TYPE_NETWORK | 2)

typedef struct
{
    char ssid[32];
    char password[64];
    int channel;
    bool ap_mode;
} rg_wifi_config_t;

typedef struct
{
    char ssid[32];
    char local_addr[16];
    char remote_addr[16];
    int channel, rssi;
    int state;
} rg_network_t;

bool rg_network_init(void);
void rg_network_deinit(void);
bool rg_network_wifi_set_config(const char *ssid, const char *password, int channel, bool ap_mode);
bool rg_network_wifi_start(void);
void rg_network_wifi_stop(void);
bool rg_network_sync_time(const char *host, int *out_delta);
rg_network_t rg_network_get_info(void);
