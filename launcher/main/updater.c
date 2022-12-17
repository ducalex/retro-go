#include "rg_system.h"
#include "gui.h"

#include <string.h>
#include <cJSON.h>

#define GITHUB_RELEASES_URL "https://api.github.com/repos/ducalex/retro-go/releases"

#ifdef RG_TARGET_ODROID_GO
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/odroid/firmware"
#else
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/espgbc/firmware"
#endif

static int download_file(const char *url, const char *filename)
{
    RG_ASSERT(url && filename, "bad param");

    rg_http_req_t *req = NULL;
    FILE *fp = NULL;
    void *buffer = NULL;
    int received = 0;
    int len;
    int ret = -1;

    RG_LOGI("Downloading: '%s' to '%s'", url, filename);
    rg_gui_draw_dialog("Connecting...", NULL, 0);

    if (!(req = rg_network_http_open(url, NULL)))
        goto cleanup;

    if (!(fp = fopen(filename, "wb")))
        goto cleanup;

    if (!(buffer = malloc(16 * 1024)))
        goto cleanup;

    rg_gui_draw_dialog("Receiving...", NULL, 0);

    while ((len = rg_network_http_read(req, buffer, 16 * 1024)) > 0)
    {
        received += len;
        fwrite(buffer, 1, len, fp);
        sprintf(buffer, "Received %d / %d", received, req->content_length);
        rg_gui_draw_dialog(buffer, NULL, 0);
        rg_system_tick(0);
    }

    if (req->content_length == received)
        ret = 0;
    else if (req->content_length == -1)
        ret = 0;

cleanup:
    rg_network_http_close(req);
    free(buffer);
    fclose(fp);

    return ret;
}

static cJSON *fetch_json(const char *url)
{
    RG_ASSERT(url, "bad param");

    RG_LOGI("Fetching: '%s'", url);
    rg_gui_draw_hourglass();

    rg_http_req_t *req = NULL;
    char *buffer = NULL;
    cJSON *json = NULL;

    if (!(req = rg_network_http_open(url, NULL)))
        goto cleanup;

    size_t buffer_length = RG_MAX(256 * 1024, req->content_length);

    if (!(buffer = malloc(buffer_length)))
        goto cleanup;

    if (rg_network_http_read(req, buffer, buffer_length) < 16)
        goto cleanup;

    if (!(json = cJSON_Parse(buffer)))
        goto cleanup;

cleanup:
    rg_network_http_close(req);
    free(buffer);
    return json;
}

void updater_show_dialog(void)
{
    cJSON *releases = fetch_json(GITHUB_RELEASES_URL);
    size_t releases_length = RG_MIN(cJSON_GetArraySize(releases), 16);

    rg_gui_option_t options[releases_length + 1];
    rg_gui_option_t *opt = options;

    if (releases_length == 0)
    {
        rg_gui_alert("Error", "Received empty list");
        goto cleanup;
    }

    for (int i = 0; i < releases_length; i++)
    {
        cJSON *release = cJSON_GetArrayItem(releases, i);
        *opt++ = (rg_gui_option_t){i, cJSON_GetStringValue(cJSON_GetObjectItem(release, "name")), NULL, 1, NULL};
    }
    *opt++ = (rg_gui_option_t)RG_DIALOG_END;

    int sel = rg_gui_dialog("Available releases", options, 0);
    if (sel != RG_DIALOG_CANCELLED)
    {
        cJSON *release = cJSON_GetArrayItem(releases, sel);
        char *release_name = cJSON_GetStringValue(cJSON_GetObjectItem(release, "name"));
        char *release_date = cJSON_GetStringValue(cJSON_GetObjectItem(release, "published_at"));
        cJSON *assets = cJSON_GetObjectItem(release, "assets");
        size_t assets_length = cJSON_GetArraySize(assets);

        rg_gui_option_t options[assets_length + 4];
        rg_gui_option_t *opt = options;

        *opt++ = (rg_gui_option_t){0, "Date", release_date, -1, NULL};
        *opt++ = (rg_gui_option_t){0, "Downloads:", NULL, -1, NULL};

        for (int i = 0; i < assets_length; i++)
        {
            cJSON *asset = cJSON_GetArrayItem(assets, i);
            *opt++ = (rg_gui_option_t){i, cJSON_GetStringValue(cJSON_GetObjectItem(asset, "name")), NULL, 1, NULL};
        }
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog(release_name, options, -1);
        if (sel != RG_DIALOG_CANCELLED)
        {
            cJSON *asset = cJSON_GetArrayItem(assets, sel);
            char *asset_name = strdup(cJSON_GetStringValue(cJSON_GetObjectItem(asset, "name")));
            char *asset_url = strdup(cJSON_GetStringValue(cJSON_GetObjectItem(asset, "browser_download_url")));
            char *dest_path = malloc(RG_PATH_MAX);
            snprintf(dest_path, RG_PATH_MAX, "%s/%s", DOWNLOAD_LOCATION, asset_name);

            // This is a bit ugly but at this point we need to free some internal heap because mbedtls
            // doesn't seem to want to use SPIRAM :(
            cJSON_Delete(releases);

            // Clean the screen, make it easier to see progress
            gui_redraw();

            if (download_file(asset_url, dest_path) == 0)
                rg_gui_alert("Download complete!", dest_path);
            else
                rg_gui_alert("Download failed!", dest_path);

            free(asset_name);
            free(asset_url);
            free(dest_path);
            return;
        }
    }

cleanup:
    cJSON_Delete(releases);
}
