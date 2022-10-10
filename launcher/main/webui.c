#include "rg_system.h"

#ifdef RG_ENABLE_NETWORKING
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <cJSON.h>

// static const char webui_html[];
#include "webui.html.h"

static httpd_handle_t server;


static esp_err_t http_api_handler(httpd_req_t *req)
{
    char http_buffer[0x1000] = {0};
    esp_err_t ret = ESP_OK;

    if (httpd_req_recv(req, http_buffer, sizeof(http_buffer)) <= 0) {
        return ESP_FAIL;
    }

    cJSON *content = cJSON_Parse(http_buffer);
    cJSON *response = cJSON_CreateObject();

    if (!content || !response) {
        goto fail;
    }

    const char *cmd  = cJSON_GetStringValue(cJSON_GetObjectItem(content, "cmd")) ?: "-";
    const char *arg1 = cJSON_GetStringValue(cJSON_GetObjectItem(content, "arg1")) ?: "";
    const char *arg2 = cJSON_GetStringValue(cJSON_GetObjectItem(content, "arg2")) ?: "";

    // Echo the command we're about to run
    cJSON_AddStringToObject(response, "cmd", cmd);

    if (strcmp(cmd, "list") == 0)
    {
        cJSON_AddStringToObject(response, "path", arg1);
        cJSON *list = cJSON_AddArrayToObject(response, "files");
        rg_scandir_t *files = rg_storage_scandir(arg1, NULL);
        for (rg_scandir_t *entry = files; entry && entry->is_valid; ++entry)
        {
            cJSON *obj = cJSON_CreateObject();
            cJSON_AddStringToObject(obj, "name", entry->name);
            cJSON_AddBoolToObject(obj, "is_dir", entry->is_dir);
            cJSON_AddItemToArray(list, obj);
        }
        free(files);
    }
    else if (strcmp(cmd, "rename") == 0)
    {
        cJSON_AddStringToObject(response, "path1", arg1);
        cJSON_AddStringToObject(response, "path2", arg2);
        cJSON_AddBoolToObject(response, "success", rename(arg1, arg2) == 0);
    }
    else if (strcmp(cmd, "delete") == 0)
    {
        cJSON_AddStringToObject(response, "path", arg1);
        cJSON_AddBoolToObject(response, "success", rg_storage_delete(arg1));
    }
    else if (strcmp(cmd, "mkdir") == 0)
    {
        cJSON_AddStringToObject(response, "path", arg1);
        cJSON_AddBoolToObject(response, "success", rg_storage_mkdir(arg1));
    }
    else if (strcmp(cmd, "touch") == 0)
    {
        FILE *fp = fopen(arg1, "wb");
        cJSON_AddStringToObject(response, "path", arg1);
        cJSON_AddBoolToObject(response, "success", fp && fclose(fp) == 0);
    }
    else if (strcmp(cmd, "download") == 0)
    {
        RG_LOGI("Sending file %s\n", arg1);

        FILE *fp = fopen(arg1, "rb");
        if (!fp)
            goto fail;

        httpd_resp_set_type(req, "application/binary");
        for (size_t size; (size = fread(http_buffer, 1, sizeof(http_buffer), fp));)
        {
            httpd_resp_send_chunk(req, http_buffer, size);
        }
        fclose(fp);
        httpd_resp_send_chunk(req, NULL, 0);
        RG_LOGI("File sending complete");
        goto done;
    }
    else
    {
        cJSON_AddBoolToObject(response, "success", false);
    }

    // Send JSON response
    char *response_text = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response_text);
    free(response_text);
    goto done;

fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server error");
    RG_LOGE("Request failed!");
    ret = ESP_FAIL;

done:
    cJSON_free(response);
    cJSON_free(content);
    return ret;
}

static esp_err_t http_upload_handler(httpd_req_t *req)
{
    char http_buffer[0x1000];

    // FIXME: We must decode req->uri...
    FILE *fp = fopen(req->uri, "wb");
    if (!fp)
        return ESP_FAIL;

    for (int pos = 0; pos < req->content_len;)
    {
        int length = httpd_req_recv(req, http_buffer, sizeof(http_buffer));
        if (length <= 0)
            break;
        if (!fwrite(http_buffer, length, 1, fp))
        {
            RG_LOGI("Write failure at %d bytes", pos);
            break;
        }
        rg_task_delay(10);
        pos += length;
    }

    fclose(fp);

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t http_get_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, webui_html);
    return ESP_OK;
}

void webui_stop(void)
{
    if (!server) // Already stopped
        return;

    httpd_stop(server);
    server = NULL;
}

void webui_start(void)
{
    if (server) // Already started
        return;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.uri_match_fn = httpd_uri_match_wildcard;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_register_uri_handler(server, &(httpd_uri_t){
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = http_get_handler,
    });

    httpd_register_uri_handler(server, &(httpd_uri_t){
        .uri       = "/api",
        .method    = HTTP_POST,
        .handler   = http_api_handler,
    });

    httpd_register_uri_handler(server, &(httpd_uri_t){
        .uri       = "/*",
        .method    = HTTP_PUT,
        .handler   = http_upload_handler,
    });

    RG_LOGI("File server started");
}

#endif
