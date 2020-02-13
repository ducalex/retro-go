#pragma once

typedef enum {
    ODROID_NETPLAY_MODE_SERVER,
    ODROID_NETPLAY_MODE_CLIENT,
    ODROID_NETPLAY_MODE_NULL,
} ODROID_NETPLAY_MODE;

void odroid_netplay_init();
void odroid_netplay_start_client();
void odroid_netplay_start_server();
void odroid_netplay_sync_init(void *data);
void odroid_netplay_sync_frame(void *data);

extern ODROID_NETPLAY_MODE netplay_mode;
