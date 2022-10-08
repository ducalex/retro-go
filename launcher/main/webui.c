#include "rg_system.h"

#ifdef RG_ENABLE_NETWORKING
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cJSON.h>

static const char html[] = {
    "<!DOCTYPE html><html><body>"
    "<h1>Retro-Go File Server</h1>"
    "cmd: <input type='text' id='rg_cmd' value='list'><br>"
    "arg1: <input type='text' id='rg_arg1' value='/sd/roms'><br>"
    "arg2: <input type='text' id='rg_arg2'><br>"
    "<button onclick='api_req(window.rg_cmd.value, window.rg_arg1.value, window.rg_arg2.value);'>send</button>"
    "<script>"
    "function api_req(cmd, arg1, arg2) {"
    "   var xmlhttp = new XMLHttpRequest();"
    "   xmlhttp.addEventListener('load', function(){alert(this.responseText)});"
    "   xmlhttp.open('POST', '/api');"
    "   xmlhttp.send(JSON.stringify({cmd, arg1, arg2}));"
    "}"
    "</script>"
    "</body></html>"
};

static httpd_handle_t server;


static esp_err_t http_api_handler(httpd_req_t *req)
{
    char buffer[1024] = {0};

    if (httpd_req_recv(req, buffer, sizeof(buffer)) <= 0) {
        return ESP_FAIL;
    }

    cJSON *content = cJSON_Parse(buffer);
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
            cJSON_AddBoolToObject(obj, "is_idr", entry->is_dir);
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
        cJSON_AddBoolToObject(response, "success", unlink(arg1) == 0 || rmdir(arg1) == 0);
    }
    else if (strcmp(cmd, "download") == 0)
    {
        // send file contents
    }
    else
    {
        cJSON_AddBoolToObject(response, "success", false);
    }

    // Set Content-Type to json

    char *response_text = cJSON_Print(response);
    httpd_resp_sendstr(req, response_text);
    free(response_text);

    cJSON_free(response);
    cJSON_free(content);
    return ESP_OK;

fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server error");
    cJSON_free(response);
    cJSON_free(content);
    return ESP_FAIL;
}

static esp_err_t http_get_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, html);
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

    // httpd_register_uri_handler(server, &(httpd_uri_t){
    //     .uri       = "/upload",
    //     .method    = HTTP_POST,
    //     .handler   = http_upload_handler,
    // });

    RG_LOGI("File server started");
}
#endif
