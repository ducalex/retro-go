#include "rg_system.h"
#include "gui.h"

#include <cJSON.h>

typedef struct
{
    char name[32];
    char url[96];
    size_t size;
} release_t;

typedef struct
{
    size_t count;
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

    int num_releases = cJSON_GetArraySize(releases);
    res = malloc(sizeof(releases_t) + (num_releases * sizeof(release_t)));
    res->count = num_releases;

    for (int i = 0; i < num_releases; i++)
    {
        cJSON *el = cJSON_GetArrayItem(releases, i);
        cJSON *name = cJSON_GetObjectItem(el, "name");
        cJSON *url = cJSON_GetObjectItem(el, "url");
        snprintf(res->releases[i].name, 32, "%s", cJSON_GetStringValue(name));
        snprintf(res->releases[i].url, 96, "%s", cJSON_GetStringValue(url));
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

    releases_t *releases = get_releases_from_github();
    rg_gui_option_t *options = NULL;

    if (!releases || releases->count == 0)
    {
        rg_gui_alert("Error", "Received empty list");
        goto cleanup;
    }

    options = malloc(sizeof(rg_gui_option_t) * (releases->count + 1));
    rg_gui_option_t *option = options;

    for (size_t i = 0; i < releases->count; i++)
    {
        *option++ = (rg_gui_option_t){i, releases->releases[i].name, NULL, 1, NULL};
    }
    *option++ = (rg_gui_option_t)RG_DIALOG_CHOICE_LAST;

    int sel = rg_gui_dialog("Download release:", options, 0);

    if (sel != RG_DIALOG_CANCELLED)
    {
        char *url = releases->releases[sel].url;
        RG_LOGI("Downloading '%s' to '/odroid/firmware'...", url);
    }

cleanup:
    free(releases);
    free(options);
}
