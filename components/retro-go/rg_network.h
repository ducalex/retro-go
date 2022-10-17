#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_WIFI_INVALID = 0,
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
    char local_addr[16];
    char remote_addr[16];
    int rssi, state;
} rg_network_t;

bool rg_network_init(void);
void rg_network_deinit(void);
bool rg_network_wifi_start_ap(const char *ssid, const char *password, int channel);
bool rg_network_wifi_start(const char *ssid, const char *password, int channel);
void rg_network_wifi_stop(void);
bool rg_network_sync_time(const char *host, int *out_delta);
rg_network_t rg_network_get_info(void);
