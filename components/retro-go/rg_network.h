#pragma once

#include <stdbool.h>
#include <stdint.h>

#define RG_EVENT_NETWORK_DISCONNECTED (RG_EVENT_TYPE_NETWORK | 1)
#define RG_EVENT_NETWORK_CONNECTED    (RG_EVENT_TYPE_NETWORK | 2)

typedef struct
{
    char ssid[32 + 1];
    char password[64 + 1];
    int channel;
    bool ap_mode;
} rg_wifi_config_t;

typedef enum
{
    RG_NETWORK_DISABLED,
    RG_NETWORK_DISCONNECTED,
    RG_NETWORK_CONNECTING,
    RG_NETWORK_CONNECTED,
} rg_network_state_t;

typedef struct
{
    char name[32 + 1];
    char ip_addr[16];
    int channel, rssi;
    int state;
} rg_network_t;

bool rg_network_init(void);
void rg_network_deinit(void);
bool rg_network_wifi_set_config(const rg_wifi_config_t *config);
bool rg_network_wifi_read_config(int slot, rg_wifi_config_t *out);
bool rg_network_wifi_start(void);
void rg_network_wifi_stop(void);
rg_network_t rg_network_get_info(void);

typedef struct
{
    int max_redirections;
    int timeout_ms;
    // Perform POST request
    const void *post_data;
    int post_len;
} rg_http_cfg_t;

#define RG_HTTP_DEFAULT_CONFIG() \
    {                            \
        .max_redirections = 5,   \
        .timeout_ms = 30000,     \
        .post_data = NULL,       \
        .post_len = 0,           \
    }

typedef struct
{
    rg_http_cfg_t config;
    int status_code;
    int content_length;
    int received_bytes;
    int redirections;
    void *client;
} rg_http_req_t;

rg_http_req_t *rg_network_http_open(const char *url, const rg_http_cfg_t *cfg);
int rg_network_http_read(rg_http_req_t *req, void *buffer, size_t buffer_len);
void rg_network_http_close(rg_http_req_t *req);
