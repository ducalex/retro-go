#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"

#include "rg_system.h"
#include "quake_main.h"
#include "esp32quake/music_esp32.h"

static const char TAG[] = "main";

#define QUAKE_AUDIO_RATE 22050

#if defined(CONFIG_IDF_TARGET_ESP32)
#define ESP32_QUAKE_TASK_STACK_SIZE 65536
static DRAM_ATTR uint8_t quake_task_stack[ESP32_QUAKE_TASK_STACK_SIZE];
#else
// 160 KiB task retains about 39 KiB / 33 KiB of observed headroom.
#define ESP32_QUAKE_TASK_STACK_SIZE (160 * 1024)
static EXT_RAM_BSS_ATTR uint8_t quake_task_stack[ESP32_QUAKE_TASK_STACK_SIZE];
#endif
static DRAM_ATTR StaticTask_t quake_task_internal;
static volatile TaskHandle_t quake_task;

// Keep the renderer off Retro-Go's display, LCD and S3 audio task core.
#define QUAKE_TASK_CORE 0

static rg_app_t *app;

#define SETTING_MUSIC "Music"

static void load_music_setting(void)
{
    Music_SetEnabled(rg_settings_get_boolean(NS_APP, SETTING_MUSIC, true));
}

static rg_gui_event_t music_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    bool enabled = Music_IsEnabled();
    bool changed = event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT || event == RG_DIALOG_ENTER;
    if (changed) {
        enabled = !enabled;
        Music_SetEnabled(enabled);
        rg_settings_set_boolean(NS_APP, SETTING_MUSIC, enabled);
    }
    sprintf(option->value, "%s", enabled ? _("On") : _("Off"));
    return changed ? RG_DIALOG_REDRAW : RG_DIALOG_VOID;
}

static void options_handler(rg_gui_option_t *dest)
{
    *dest++ = (rg_gui_option_t){0, _("Music"), "-", RG_DIALOG_FLAG_NORMAL, music_update_cb};
    *dest++ = (rg_gui_option_t)RG_DIALOG_END;
}

void __real_rg_system_exit(void);

// The shared menu calls rg_system_exit before RG_EVENT_SHUTDOWN. Launcher
// selection maps flash, which cannot run on Quake's S3/P4 PSRAM stack.
// Finish on the engine owner, then let app_main perform the platform exit
// on its internal stack. Never return into the engine after freeing it.
void __wrap_rg_system_exit(void)
{
    if (quake_task && xTaskGetCurrentTaskHandle() == quake_task) {
        quake_shutdown();
        quake_task = NULL;
        vTaskDelete(NULL);
        return;
    }
    __real_rg_system_exit();
}

static const char *selected_pak_path(void)
{
    return (app->romPath && *app->romPath)
        ? app->romPath
        : RG_BASE_PATH_ROMS "/quake/id1/pak0.pak";
}

typedef struct {
    char pak0[QUAKE_MAX_PATH];
    char pak1[QUAKE_MAX_PATH];
} pak_siblings_t;

static int find_pak_siblings(const rg_scandir_t *file, void *arg)
{
    pak_siblings_t *paks = arg;
    char *destination = NULL;

    if (!file->is_file)
        return RG_SCANDIR_CONTINUE;
    if (!strcasecmp(file->basename, "pak0.pak"))
        destination = paks->pak0;
    else if (!strcasecmp(file->basename, "pak1.pak"))
        destination = paks->pak1;

    if (destination && strlen(file->path) < QUAKE_MAX_PATH)
        strcpy(destination, file->path);
    return RG_SCANDIR_CONTINUE;
}

static bool native_save_handler(const char *filename)
{
    (void)filename;
    rg_gui_alert("Quake saves", "Please use Quake's in-game Save menu.");
    return false;
}

static bool native_load_handler(const char *filename)
{
    (void)filename;
    rg_gui_alert("Quake saves", "Please use Quake's in-game Load menu.");
    return false;
}

static bool native_reset_handler(bool hard)
{
    (void)hard;
    rg_gui_alert("Restart game", "Please use Quake's in-game New Game menu.");
    return false;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return quake_screenshot(filename, width, height);
}

static void event_handler(int event, void *data)
{
    (void)data;

    if (event == RG_EVENT_REDRAW) {
        quake_redraw();
    } else if (event == RG_EVENT_SPEEDUP) {
        // Quake consumes app->speed directly; fixed-ratio auto frameskip is
        // still inappropriate for its variable wall-clock timestep.
        app->frameskip = -1;
    } else if (event == RG_EVENT_SHUTDOWN) {
        ESP_LOGI(TAG, "Shutdown event received");
        quake_request_exit();

        // The shared game menu runs in the Quake task. It restarts the system
        // directly after this callback, so finish engine and proxy teardown
        // here rather than expecting the native loop to regain control.
        if (quake_task && xTaskGetCurrentTaskHandle() == quake_task) {
            quake_shutdown();
        } else {
            // For a shutdown initiated by another task, let Quake unwind on
            // its owner task before Retro-Go unmounts storage.
            int64_t deadline = rg_system_timer() + 3000000;
            while (quake_task && rg_system_timer() < deadline) {
                rg_task_yield();
            }
            if (quake_task) {
                ESP_LOGW(TAG, "Quake did not finish shutdown before timeout");
            }
        }
    }
}

static void user_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Quake task started");

    char basedir[RG_PATH_MAX + 1];
    const char *selectedPak = selected_pak_path();
    const char *pak0Path = selectedPak;
    const char *pak1Path = NULL;
    pak_siblings_t paks = {0};

    if (!rg_extension_match(selectedPak, "pak")) {
        rg_gui_alert("Invalid game data", "Please select a Quake .pak file.");
        quake_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    // Start from the selected PAK's directory. Conventional pak0/pak1 names
    // are resolved below; a non-standard filename remains authoritative.
    const char *slash = strrchr(selectedPak, '/');
    size_t dir_len = slash ? (size_t)(slash - selectedPak) : 0;
    if (dir_len == 0 || dir_len >= QUAKE_MAX_PATH ||
        strlen(selectedPak) >= QUAKE_MAX_PATH) {
        rg_gui_alert("Invalid game data", "The selected PAK path is invalid.");
        quake_task = NULL;
        vTaskDelete(NULL);
        return;
    }
    memcpy(basedir, selectedPak, dir_len);
    basedir[dir_len] = '\0';

    // pak0 is always the base data set. Selecting either conventional pack
    // resolves both siblings using the filesystem's real filename case.
    // pak1 is optional and upgrades the same launch to registered Quake.
    const char *selected_name = slash + 1;
    bool conventional_pak = !strcasecmp(selected_name, "pak0.pak") ||
                            !strcasecmp(selected_name, "pak1.pak");
    if (conventional_pak) {
        rg_storage_scandir(basedir, find_pak_siblings, &paks,
                           RG_SCANDIR_FILES | RG_SCANDIR_STAT);
        if (!paks.pak0[0]) {
            rg_gui_alert("Missing game data",
                         "pak0.pak is required. Keep pak0.pak and optional pak1.pak in the same folder.");
            quake_task = NULL;
            vTaskDelete(NULL);
            return;
        }
        pak0Path = paks.pak0;
        pak1Path = paks.pak1[0] ? paks.pak1 : NULL;
    } else if (!rg_storage_stat(selectedPak).is_file) {
        rg_gui_alert("Missing game data", "The selected Quake PAK could not be opened.");
        quake_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    // A base id1 selection stays exact so another copy in its parent cannot
    // consume descriptors. Mission packs and mods retain parent/id1 layering.
    const char *game = NULL;
    const char *leaf = strrchr(basedir, '/');
    leaf = leaf ? leaf + 1 : basedir;
    bool mission_game_dir = !strcmp(leaf, "rogue") || !strcmp(leaf, "hipnotic");
    bool quake_mod_dir = !strncmp(basedir, RG_BASE_PATH_ROMS "/quake/",
                                  strlen(RG_BASE_PATH_ROMS "/quake/"));
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    if (mission_game_dir) {
        rg_gui_alert("Unsupported mission pack",
                     "Official Quake mission packs require more memory than 8 MiB S3 devices provide. They are supported on ESP32-P4.");
        quake_task = NULL;
        vTaskDelete(NULL);
        return;
    }
#endif
    if (mission_game_dir || (quake_mod_dir && strcmp(leaf, "id1"))) {
        if (!strcmp(leaf, "rogue") || !strcmp(leaf, "hipnotic"))
            game = leaf;
        else if (quake_mod_dir && strcmp(leaf, "id1"))
            game = leaf;
        char *parent_slash = strrchr(basedir, '/');
        if (parent_slash)
            *parent_slash = '\0';
    }

    ESP_LOGI(TAG, "Using base PAK: %s", pak0Path);
    ESP_LOGI(TAG, "Registered PAK: %s", pak1Path ? pak1Path : "not found (shareware)");
    quake_main(basedir, game, pak0Path, pak1Path, 0, NULL);

    quake_task = NULL;
    vTaskDelete(NULL);
}

void app_main(void)
{
    const rg_config_t config = {
        .sampleRate = QUAKE_AUDIO_RATE,
        // Quake accepts at most 72 host updates per emulated second. Its
        // variable timestep keeps game time correct below that ceiling.
        .frameRate = 72,
        .storageRequired = true,
        .romRequired = true,
        .handlers = {
            .loadState = native_load_handler,
            .saveState = native_save_handler,
            .reset = native_reset_handler,
            .screenshot = screenshot_handler,
            .event = event_handler,
            .options = options_handler,
        },
    };

    app = rg_system_init(&config);
    load_music_setting();
    app->frameskip = -1;

    const char *pak_path = selected_pak_path();
    if (!rg_extension_match(pak_path, "pak") || !strrchr(pak_path, '/') ||
        strlen(pak_path) >= QUAKE_MAX_PATH) {
        rg_gui_alert("Missing game data", "Please select a valid Quake .pak file.");
        rg_system_exit();
    }

    rg_storage_mkdir(RG_BASE_PATH_CONFIG "/quake");
    rg_storage_mkdir(RG_BASE_PATH_SAVES "/quake");
    if ((quake_task = xTaskCreateStaticPinnedToCore(user_task, "quake_task", ESP32_QUAKE_TASK_STACK_SIZE, NULL, RG_TASK_PRIORITY_5, quake_task_stack, &quake_task_internal, QUAKE_TASK_CORE)) == NULL)
    {
        ESP_LOGE(TAG, "failed to start quake task");
        rg_system_exit();
    }

    while (quake_task != NULL)
    {
        rg_task_delay(100);
    }

    ESP_LOGI(TAG, "Quake task finished, exiting...");
    rg_system_exit();
}
