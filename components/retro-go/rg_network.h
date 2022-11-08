#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RG_EVENT_NETWORK_DISCONNECTED (RG_EVENT_TYPE_NETWORK | 1)
#define RG_EVENT_NETWORK_CONNECTED    (RG_EVENT_TYPE_NETWORK | 2)

typedef struct
{
    char ssid[32];
    char password[64];
    int channel;
    bool ap_mode;
} rg_wifi_config_t;

typedef enum
{
    RG_NETWORK_DISCONNECTED,
    RG_NETWORK_CONNECTING,
    RG_NETWORK_CONNECTED,
} rg_network_state_t;

typedef struct
{
    char name[32];
    char ip_addr[16];
    int channel, rssi;
    int state;
} rg_network_t;

bool rg_network_init(void);
void rg_network_deinit(void);
bool rg_network_wifi_set_config(const char *ssid, const char *password, int channel, int mode);
bool rg_network_wifi_load_config(int slot);
bool rg_network_wifi_start(void);
void rg_network_wifi_stop(void);
bool rg_network_sync_time(const char *host, int *out_delta);
rg_network_t rg_network_get_info(void);
