#if 0
#include <freertos/FreeRTOS.h>
#include <tcpip_adapter.h>
#include <esp_http_server.h>
#include <esp_system.h>
#include <esp_event.h>
// #include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <rg_system.h>

tcpip_adapter_ip_info_t ip_info;
static char local_ipv4[16];
static bool wifi_ready = false;

#define WIFI_SSID "RETRO-GO"
#define WIFI_CHANNEL 12


static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_AP_STAIPASSIGNED) // A client connected to us
    {
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
        sprintf(local_ipv4, IPSTR, IP2STR(&ip_info.ip));
        wifi_ready = true;
    }
    else if (event_id == IP_EVENT_STA_GOT_IP) // We connected to an AP
    {
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
        sprintf(local_ipv4, IPSTR, IP2STR(&ip_info.ip));
        wifi_ready = true;
    }
}


static esp_err_t http_handler(httpd_req_t *req)
{
    if (req->method == HTTP_POST)
    {
        char content[100];
        size_t bytes = 0;

        while (bytes < req->content_len)
        {
            int ret = httpd_req_recv(req, content, sizeof(content));
            if (ret < 0)
            {
                return ESP_FAIL;
            }
            bytes += ret;
        }
    }

    httpd_resp_send(req, "Hello World!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static void ftp_server_task(void *arg)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_mode_t mode = arg ? WIFI_MODE_STA : WIFI_MODE_AP;
    httpd_handle_t http_server = NULL;
    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();

    wifi_ready = false;

    tcpip_adapter_init();
    esp_event_loop_create_default();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_config(mode, &(wifi_config_t){
        .sta.ssid = WIFI_SSID,
        .sta.channel = WIFI_CHANNEL,
    }));
    ESP_ERROR_CHECK(esp_wifi_set_config(mode, &(wifi_config_t){
        .ap.ssid = WIFI_SSID,
        .ap.authmode = WIFI_AUTH_OPEN,
        .ap.channel = WIFI_CHANNEL,
        .ap.max_connection = 2,
    }));
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
    ESP_ERROR_CHECK(esp_wifi_start());
    if (mode == WIFI_MODE_STA)
        ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for Wifi to connect/start
    for (int i = 0; i < 1000 && !wifi_ready; i++)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (!wifi_ready)
    {
        RG_LOGE("Wifi init failure.\n");
        return;
    }

    RG_LOGI("Wifi connected. Local IP = %s\n", local_ipv4);

    if (httpd_start(&http_server, &http_config) == ESP_OK)
    {
        httpd_register_uri_handler(http_server, &(httpd_uri_t){"/", HTTP_GET, http_handler, 0});
        httpd_register_uri_handler(http_server, &(httpd_uri_t){"/", HTTP_POST, http_handler, 0});
        RG_LOGI("Web Server started!\n");
    }

    while (1)
    {
        vTaskDelay(10);
    }

    httpd_stop(http_server);
}

void ftp_server_start(void)
{
    ftp_server_task(1);
}

void ftp_server_stop(void)
{

}
#else
void ftp_server_start(void) {}
void ftp_server_stop(void) {}
#endif
