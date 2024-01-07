#include "rg_system.h"
#include "gui.h"

#ifdef RG_ENABLE_NETWORKING
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

#define GITHUB_RELEASES_URL "https://api.github.com/repos/ducalex/retro-go/releases"

#if defined(RG_TARGET_MRGC_G32)
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/espgbc/firmware"
#else
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/odroid/firmware"
#endif

typedef struct
{
    char name[32];
    char url[256];
} asset_t;

typedef struct
{
    char name[32];
    char date[32];
    asset_t *assets;
    size_t assets_count;
} release_t;

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

    if (!(buffer = calloc(1, buffer_length + 1)))
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

static rg_gui_event_t view_release_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        release_t *release = (release_t *)option->arg;

        rg_gui_option_t options[release->assets_count + 4];
        rg_gui_option_t *opt = options;

        *opt++ = (rg_gui_option_t){0, "Date", release->date, -1, NULL};
        *opt++ = (rg_gui_option_t){0, "Files:", NULL, -1, NULL};

        for (int i = 0; i < release->assets_count; i++)
            *opt++ = (rg_gui_option_t){i, release->assets[i].name, NULL, 1, NULL};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog(release->name, options, -1);
        if (sel != RG_DIALOG_CANCELLED)
        {
            char dest_path[RG_PATH_MAX];
            snprintf(dest_path, RG_PATH_MAX, "%s/%s", DOWNLOAD_LOCATION, release->assets[sel].name);

            int ret = download_file(release->assets[sel].url, dest_path);
            gui_redraw();

            if (ret != 0)
                rg_gui_alert("Download failed!", "...");
            else if (rg_gui_confirm("Download complete!", "Reboot to flash?", true))
                rg_system_switch_app(RG_APP_FACTORY, NULL, NULL, 0);
        }
        gui_redraw();
    }

    return RG_DIALOG_VOID;
}

void updater_show_dialog(void)
{
    cJSON *releases_json = fetch_json(GITHUB_RELEASES_URL);
    size_t releases_count = RG_MIN(cJSON_GetArraySize(releases_json), 20);

    release_t *releases = calloc(releases_count, sizeof(release_t));
    for (int i = 0; i < releases_count; ++i)
    {
        cJSON *release_json = cJSON_GetArrayItem(releases_json, i);
        char *name = cJSON_GetStringValue(cJSON_GetObjectItem(release_json, "name"));
        char *date = cJSON_GetStringValue(cJSON_GetObjectItem(release_json, "published_at"));

        snprintf(releases[i].name, 32, "%s", name ?: "N/A");
        snprintf(releases[i].date, 32, "%s", date ?: "N/A");

        cJSON *assets_json = cJSON_GetObjectItem(release_json, "assets");
        size_t assets_count = cJSON_GetArraySize(assets_json);
        releases[i].assets = calloc(assets_count, sizeof(asset_t));
        releases[i].assets_count = 0;

        for (int j = 0; j < assets_count; ++j)
        {
            cJSON *asset_json = cJSON_GetArrayItem(assets_json, j);
            char *name = cJSON_GetStringValue(cJSON_GetObjectItem(asset_json, "name"));
            char *url = cJSON_GetStringValue(cJSON_GetObjectItem(asset_json, "browser_download_url"));
            if (name && url)
            {
                asset_t *asset = &releases[i].assets[releases[i].assets_count++];
                snprintf(asset->name, 32, "%s", name);
                snprintf(asset->url, 256, "%s", url);
            }
        }
    }

    cJSON_Delete(releases_json);

    gui_redraw();

    if (releases_count > 0)
    {
        rg_gui_option_t options[releases_count + 1];
        rg_gui_option_t *opt = options;

        for (int i = 0; i < releases_count; ++i)
            *opt++ = (rg_gui_option_t){(intptr_t)&releases[i], releases[i].name, NULL, 1, &view_release_cb};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        rg_gui_dialog("Available Releases", options, 0);
    }
    else
    {
        rg_gui_alert("Available Releases", "Received empty list!");
    }

    for (int i = 0; i < releases_count; ++i)
        free(releases[i].assets);
    free(releases);
}
#endif
