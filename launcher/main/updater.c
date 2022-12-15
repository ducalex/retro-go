#include "rg_system.h"
#include "gui.h"

#include <cJSON.h>

typedef struct
{
    char date[12];
    char name[32];
    char url[96];
    size_t size;
} release_t;

typedef struct
{
    size_t length;
    release_t releases[];
} releases_t;


static releases_t *get_releases_from_github(void)
{
    size_t max_length = 256 * 1024;
    char *buffer = malloc(max_length);
    rg_http_req_t *req = NULL;
    releases_t *res = NULL;
    cJSON *releases = NULL;

    if (!(req = rg_network_http_open("https://api.github.com/repos/ducalex/retro-go/releases", NULL)))
        goto cleanup;

    if (rg_network_http_read(req, buffer, max_length) < 16)
        goto cleanup;

    if (!(releases = cJSON_Parse(buffer)))
        goto cleanup;

    // Limit to 10 because our dialog system isn't super reliable with large lists :(
    size_t num_releases = RG_MIN(cJSON_GetArraySize(releases), 10);
    RG_LOGI("num_releases = %d", num_releases);

    res = malloc(sizeof(releases_t) + (num_releases * sizeof(release_t)));
    res->length = num_releases;

    for (size_t i = 0; i < num_releases; i++)
    {
        cJSON *el = cJSON_GetArrayItem(releases, i);
        snprintf(res->releases[i].date, 10, "%s", cJSON_GetStringValue(cJSON_GetObjectItem(el, "published_at")));
        snprintf(res->releases[i].name, 32, "%s", cJSON_GetStringValue(cJSON_GetObjectItem(el, "name")));
        snprintf(res->releases[i].url, 96, "%s", cJSON_GetStringValue(cJSON_GetObjectItem(el, "url")));
    }

cleanup:
    rg_network_http_close(req);
    cJSON_Delete(releases);
    free(buffer);
    return res;
}

void updater_show_dialog(void)
{
    rg_gui_draw_hourglass();

    releases_t *res = get_releases_from_github();

    if (res && res->length > 0)
    {
        rg_gui_option_t options[res->length + 1];
        rg_gui_option_t *opt = options;

        for (size_t i = 0; i < res->length; i++)
        {
            release_t *release = &res->releases[i];
            *opt++ = (rg_gui_option_t){i, release->name, release->date, 1, NULL};
        }
        *opt++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

        int sel = rg_gui_dialog("Download release:", options, 0);
        if (sel != RG_DIALOG_CANCELLED)
        {
            RG_LOGI("Downloading '%s' to '/odroid/firmware'...", res->releases[sel].url);
        }
    }
    else
    {
        rg_gui_alert("Error", "Received empty list");
    }

    free(res);
}
