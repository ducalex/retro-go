/*
 * Based on Espressif's HTTP File Server Example
 */
#include "rg_system.h"

#ifdef RG_ENABLE_NETWORKING
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

static const char header_html[] = {
    "<!DOCTYPE html><html><body>"
    "<div style='float:right'>"
    "   <label>Upload a file <input id='newfile' type='file'></label>"
    "</div>"
    "<h1>Retro-Go File Server</h1>"
    "<form method='post'>"
    "<table class='fixed' border='1'>"
        "<col width='800px' /><col width='300px' /><col width='300px' /><col width='100px' />"
        "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th><th>Delete</th></tr></thead>"
        "<tbody>"
};
static const char footer_html[] = {
    "</tbody></table>"
    "</form>"
    "</body></html>"
};

static httpd_handle_t server = NULL;
static char *scratch_buffer = NULL;


static esp_err_t http_get_handler(httpd_req_t *req)
{
    struct stat file_stat;
    char entrysize[16];
    struct dirent *entry;
    FILE *fd = NULL;

    char filepath[RG_PATH_MAX];
    size_t filepath_len = sprintf(filepath, "%s/%s", RG_STORAGE_ROOT, req->uri + strlen("/"));

    while (filepath[filepath_len-1] == '/') {
        filepath[--filepath_len] = 0;
    }

    RG_LOGI("local path = %s\n", filepath);

    // 404
    if (stat(filepath, &file_stat) == -1) {
        RG_LOGE("Failed to stat path : %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Path does not exist");
        return ESP_FAIL;
    }

    // Directory
    if (S_ISDIR(file_stat.st_mode)) {
        RG_LOGI("Listing dir %s\n", filepath);
        DIR *dir = opendir(filepath);
        if (!dir) {
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
            return ESP_FAIL;
        }

        httpd_resp_sendstr_chunk(req, header_html);

        while ((entry = readdir(dir)) != NULL) {
            strcpy(filepath + filepath_len, "/");
            strcat(filepath, entry->d_name);
            if (stat(filepath, &file_stat) == -1) {
                continue;
            }
            sprintf(entrysize, "%ld", file_stat.st_size);
            sprintf(scratch_buffer,
                "<tr><td><a href='%s%s%s'>%s</a></td><td>%s</td><td>%s</td><td>"
                "<button type='submit' name='delete' value='%s'>Delete</button></form></td></tr>",
                req->uri, entry->d_name, S_ISDIR(file_stat.st_mode) ? "/" : "",
                entry->d_name, S_ISDIR(file_stat.st_mode) ? "dir" : "file", entrysize,
                entry->d_name
            );
            httpd_resp_sendstr_chunk(req, scratch_buffer);
        }
        closedir(dir);

        filepath[filepath_len] = 0;

        httpd_resp_sendstr_chunk(req, footer_html);
        httpd_resp_sendstr_chunk(req, NULL);
        return ESP_OK;
    }
    // Serve file
    else if ((fd = fopen(filepath, "rb"))) {
        RG_LOGI("Sending file %s\n", filepath);

        bool is_text = strstr(filepath, ".json") || strstr(filepath, ".log") || strstr(filepath, ".txt");
        httpd_resp_set_type(req, is_text ? "text/plain" : "application/binary");

        char *chunk = scratch_buffer;
        size_t chunksize;
        do {
            chunksize = fread(chunk, 1, 8192, fd);
            if (chunksize > 0) {
                /* Send the buffer contents as HTTP response chunk */
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(fd);
                    RG_LOGE("File sending failed!");
                    goto fail;
                }
            }
        } while (chunksize != 0);
        fclose(fd);
        httpd_resp_send_chunk(req, NULL, 0);
        RG_LOGI("File sending complete");
        return ESP_OK;
    }

fail:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to serve your request");
    RG_LOGE("Failed to serve request for : %s", filepath);
    return ESP_FAIL;
}

static esp_err_t http_post_handler(httpd_req_t *req)
{
    const char *filename = NULL; // POST[delete]
    char filepath[RG_PATH_MAX];
    size_t filepath_len = sprintf(filepath, "%s/%s/%s", RG_STORAGE_ROOT, req->uri, filename);

    while (filepath[filepath_len-1] == '/') {
        filepath[--filepath_len] = 0;
    }

    RG_LOGI("local path = %s\n", filepath);

    if (unlink(filepath) == -1 && rmdir(filepath) == -1) {
        RG_LOGE("Unlink failed");
    }

    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", req->uri);
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

void ftp_server_stop(void)
{
    if (!server) // Already stopped
        return;

    httpd_stop(server);
    server = NULL;
    free(scratch_buffer);
    scratch_buffer = NULL;
}

void ftp_server_start(void)
{
    if (server) // Already started
        return;

    scratch_buffer = calloc(1, 8192);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_register_uri_handler(server, &(httpd_uri_t){
        .uri       = "/*",
        .method    = HTTP_GET,
        .handler   = http_get_handler,
    });

    httpd_register_uri_handler(server, &(httpd_uri_t){
        .uri       = "/*",
        .method    = HTTP_POST,
        .handler   = http_post_handler,
    });

    RG_LOGI("File server started");
}
#endif
