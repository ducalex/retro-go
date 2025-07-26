#include <rg_system.h>
#include <malloc.h>
#include <string.h>
#include <cJSON.h>

#include "gui.h"

#if defined(RG_ENABLE_NETWORKING) && RG_UPDATER_ENABLE
typedef struct
{
    char name[64];
    char url[256];
} asset_t;

typedef struct
{
    char name[64];
    char date[32];
    char url[256];
    asset_t *assets;
    size_t assets_count;
} release_t;

static bool download_file(const char *url, const char *filename)
{
    RG_ASSERT_ARG(url && filename);

    rg_http_req_t *req = NULL;
    FILE *fp = NULL;
    void *buffer = NULL;
    int received = 0;
    int written = 0;
    int len;

    RG_LOGI("Downloading: '%s' to '%s'", url, filename);
    rg_gui_draw_message("Connecting...");

    if (!(req = rg_network_http_open(url, NULL)))
    {
        rg_gui_alert("Download failed!", "Connection failed!");
        return false;
    }

    if (!(buffer = malloc(16 * 1024)))
    {
        rg_network_http_close(req);
        rg_gui_alert("Download failed!", "Out of memory!");
        return false;
    }

    if (!(fp = fopen(filename, "wb")))
    {
        rg_network_http_close(req);
        free(buffer);
        rg_gui_alert("Download failed!", "File open failed!");
        return false;
    }

    rg_gui_draw_message("Receiving file...");
    int content_length = req->content_length;

    while ((len = rg_network_http_read(req, buffer, 16 * 1024)) > 0)
    {
        received += len;
        written += fwrite(buffer, 1, len, fp);
        rg_gui_draw_message("Received %d / %d", received, content_length);
        if (received != written)
            break; // No point in continuing
    }

    rg_network_http_close(req);
    free(buffer);
    fclose(fp);

    if (received != written || (received != content_length && content_length != -1))
    {
        rg_storage_delete(filename);
        rg_gui_alert("Download failed!", "Read/write error!");
        return false;
    }

    gui_redraw();
    return true;
}

static cJSON *fetch_json(const char *url)
{
    RG_ASSERT_ARG(url);

    RG_LOGI("Fetching: '%s'", url);
    rg_gui_draw_hourglass();

    rg_http_req_t *req = NULL;
    char *buffer = NULL;
    cJSON *json = NULL;

    if (!(req = rg_network_http_open(url, NULL)))
    {
        RG_LOGW("Could not open releases URL '%s', aborting update check.", url);
        goto cleanup;
    }

    size_t buffer_length = RG_MAX(256 * 1024, req->content_length);

    if (!(buffer = calloc(1, buffer_length + 1)))
    {
        RG_LOGE("Out of memory, aborting update check!");
        goto cleanup;
    }

    if (rg_network_http_read(req, buffer, buffer_length) < 16)
    {
        RG_LOGW("Read from releases URL '%s' returned (almost) no bytes, aborting update check.", url);
        goto cleanup;
    }

    if (!(json = cJSON_Parse(buffer)))
        RG_LOGW("Could not parse JSON received from releases URL '%s'.", url);

cleanup:
    rg_network_http_close(req);
    free(buffer);
    gui_redraw();
    return json;
}

static rg_gui_event_t view_release_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    const release_t *release = (release_t *)option->arg;

    if (event == RG_DIALOG_ENTER)
    {
    #if defined(RG_UPDATER_APPLICATION) && defined(RG_UPDATER_DOWNLOAD_LOCATION)
        rg_gui_option_t options[release->assets_count + 4];
        rg_gui_option_t *opt = options;

        *opt++ = (rg_gui_option_t){0, _("Date"), (char *)release->date, RG_DIALOG_FLAG_MESSAGE, NULL};
        *opt++ = (rg_gui_option_t){0, _("Files:"), NULL, RG_DIALOG_FLAG_MESSAGE, NULL};

        for (int i = 0; i < release->assets_count; i++)
            *opt++ = (rg_gui_option_t){i, release->assets[i].name, NULL, RG_DIALOG_FLAG_NORMAL, NULL};
        *opt++ = (rg_gui_option_t)RG_DIALOG_END;

        int sel = rg_gui_dialog(release->name, options, 0);
        if (sel != RG_DIALOG_CANCELLED)
        {
            char dest_path[RG_PATH_MAX];
            snprintf(dest_path, RG_PATH_MAX, "%s/%s", RG_UPDATER_DOWNLOAD_LOCATION, release->assets[sel].name);
            if (download_file(release->assets[sel].url, dest_path))
            {
                if (rg_gui_confirm(_("Download complete!"), _("Reboot to flash?"), true))
                    rg_system_switch_app(RG_UPDATER_APPLICATION, NULL, dest_path, 0);
            }
        }
    #else
        rg_gui_alert(release->name, release->url);
    #endif
        return RG_DIALOG_REDRAW;
    }

    return RG_DIALOG_VOID;
}

void updater_show_dialog(void)
{
    cJSON *releases_json = fetch_json(RG_UPDATER_GITHUB_RELEASES);
    size_t releases_count = RG_MIN(cJSON_GetArraySize(releases_json), 10);
    const char *dialog_title = _("Available Releases");
    rg_gui_option_t dialog_options[releases_count + 1];

    if (!releases_json)
    {
        rg_gui_alert(dialog_title, _("Connection failed!"));
        return;
    }
    if (!releases_count)
    {
        rg_gui_alert(dialog_title, _("Received empty list!"));
        cJSON_Delete(releases_json);
        return;
    }

    release_t *releases = calloc(releases_count, sizeof(release_t));
    rg_gui_option_t *opt = dialog_options;

    for (int i = 0; i < releases_count; ++i)
    {
        cJSON *release_json = cJSON_GetArrayItem(releases_json, i);
        cJSON *assets_json = cJSON_GetObjectItem(release_json, "assets");
        size_t assets_count = cJSON_GetArraySize(assets_json);
        char *name = cJSON_GetStringValue(cJSON_GetObjectItem(release_json, "name"));
        char *date = cJSON_GetStringValue(cJSON_GetObjectItem(release_json, "published_at"));
        char *url = cJSON_GetStringValue(cJSON_GetObjectItem(release_json, "html_url"));

        release_t *release = releases + i;
        snprintf(release->name, sizeof(release->name), "%s", name ?: "N/A");
        snprintf(release->date, sizeof(release->date), "%s", date ?: "N/A");
        snprintf(release->url, sizeof(release->url), "%s", url ?: "N/A");
        release->assets = calloc(assets_count, sizeof(asset_t));
        release->assets_count = 0;

        for (int j = 0; j < assets_count; ++j)
        {
            cJSON *asset_json = cJSON_GetArrayItem(assets_json, j);
            char *name = cJSON_GetStringValue(cJSON_GetObjectItem(asset_json, "name"));
            char *url = cJSON_GetStringValue(cJSON_GetObjectItem(asset_json, "browser_download_url"));
            if (name && url && rg_extension_match(name, "fw img"))
            {
                asset_t *asset = &release->assets[release->assets_count++];
                snprintf(asset->name, sizeof(asset->name), "%s", name);
                snprintf(asset->url, sizeof(asset->url), "%s", url);
            }
        }
        *opt++ = (rg_gui_option_t){(intptr_t)release, release->name, NULL, RG_DIALOG_FLAG_NORMAL, &view_release_cb};
    }
    *opt++ = (rg_gui_option_t)RG_DIALOG_END;

    cJSON_Delete(releases_json);

    rg_gui_dialog(dialog_title, dialog_options, 0);

    for (int i = 0; i < releases_count; ++i)
        free(releases[i].assets);
    free(releases);
}
#endif
