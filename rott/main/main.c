#define NDEBUG
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rg_system.h>
#include <rg_display.h>
#include <rg_gui.h>
#include <rg_storage.h>

#define ROTT_AUDIO_OUTPUT_RATE 22050
#define ROTT_TICK_RATE 35

/* ROTT expectations */
#define PLATFORM_UNIX 1

/* External engine functions */
extern void allocate_rott_memory(void);
extern int rott_main(int argc, char *argv[]);
extern char ApogeePath[256];
extern bool rg_sdl_save_screenshot(const char *filename, int width, int height);
extern void rg_sdl_redraw(void);

static rg_app_t *app;

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_sdl_save_screenshot(filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    (void)filename;
    rg_gui_alert("Not implemented", "Please use ROTT's in-game Save Game menu.");
    return false;
}

static bool load_state_handler(const char *filename)
{
    (void)filename;
    rg_gui_alert("Not implemented", "Please use ROTT's in-game Load Game menu.");
    return false;
}

static bool reset_handler(bool hard)
{
    (void)hard;
    rg_gui_alert("Not implemented", "Please use ROTT's in-game menu.");
    return false;
}

static void event_handler(int event, void *arg) {
    if (event == RG_EVENT_SHUTDOWN)
        rg_audio_set_mute(true);
    else if (event == RG_EVENT_REDRAW)
        rg_sdl_redraw();
}

static bool copy_path(char *dest, size_t dest_size, const char *source)
{
    int length = snprintf(dest, dest_size, "%s", source);
    if (length < 0 || (size_t)length >= dest_size) {
        rg_gui_alert("Invalid data path", "The selected ROTT data path is too long.");
        return false;
    }
    return true;
}

static bool resolve_data_directory(char *dest, size_t dest_size)
{
    const char *selected = app->romPath;

    if (selected && selected[0]) {
        struct stat info;

        if (rg_extension_match(selected, "zip")) {
            rg_gui_alert("Unsupported data file",
                         "ROTT requires DARKWAR.WAD and its companion files "
                         "in the same directory. ZIP archives are not supported.");
            return false;
        }

        if (stat(selected, &info) == 0 && S_ISDIR(info.st_mode)) {
            return copy_path(dest, dest_size, selected);
        }

        if (!copy_path(dest, dest_size, selected))
            return false;

        char *separator = strrchr(dest, '/');
        char *backslash = strrchr(dest, '\\');
        if (backslash && (!separator || backslash > separator))
            separator = backslash;

        if (separator) {
            if (separator == dest)
                separator[1] = '\0';
            else
                *separator = '\0';
            return true;
        }
    }

    int length = snprintf(dest, dest_size, "%s/rott", RG_BASE_PATH_ROMS);
    return length >= 0 && (size_t)length < dest_size;
}

typedef struct {
    int argc;
    char **argv;
} rott_task_args_t;

static void rott_task(void *pvParameters) {
    rott_task_args_t *args = (rott_task_args_t *)pvParameters;
    printf("ROTT Task started. Entering rott_main...\n");
    rott_main(args->argc, args->argv);
    printf("ROTT Task finished. Exiting...\n");
    rg_system_exit();
    vTaskDelete(NULL);
}

void app_main()
{
    const rg_config_t config = {
        .sampleRate = ROTT_AUDIO_OUTPUT_RATE,
        .frameRate = ROTT_TICK_RATE,
        .storageRequired = true,
        .romRequired = true,
        .handlers = {
            .loadState = &load_state_handler,
            .saveState = &save_state_handler,
            .reset = &reset_handler,
            .screenshot = &screenshot_handler,
            .event = &event_handler,
        },
    };

    app = rg_system_init(&config);
    // Retro-Go's generic auto-frameskip deliberately settles at a minimum of
    // one after any overload. ROTT has its own per-frame renderer budget and
    // video-only skip controller, so leaving the generic controller enabled
    // would permanently limit recovered scenes to alternating drawn frames.
    app->frameskip = -1;
    // Use Full for a fresh ROTT installation, but preserve subsequent changes
    // made through Retro-Go's Options menu.
    if (!rg_settings_exists(NS_APP, "DispScaling"))
        rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);
    
    // Suppress serial flood during massive allocation
    rg_system_set_log_level(RG_LOG_WARN);

    // Give storage a moment to stabilize
    vTaskDelay(pdMS_TO_TICKS(200));

    allocate_rott_memory();

    // Allocation is intentionally quiet, but restore Retro-Go's normal
    // runtime diagnostics once startup allocation has completed.
    rg_system_set_log_level(app->isRelease ? RG_LOG_INFO : RG_LOG_DEBUG);
    
    static char rott_argv_dir[256];
    if (!resolve_data_directory(rott_argv_dir, sizeof(rott_argv_dir))) {
        rg_system_exit();
        return;
    }
    printf("ROTT data directory: %s (selected: %s)\n",
           rott_argv_dir, app->romPath ? app->romPath : "<none>");

    // Set ApogeePath to the Retro-Go save directory.
    snprintf(ApogeePath, 256, "%s/rott", RG_BASE_PATH_SAVES);
    rg_storage_mkdir(ApogeePath);

    static char *rott_argv[10];
    int rott_argc = 0;

    rott_argv[rott_argc++] = "rott";
    rott_argv[rott_argc++] = "-dir";
    rott_argv[rott_argc++] = rott_argv_dir;
    rott_argv[rott_argc] = NULL;

    printf("Starting ROTT task with argc=%d\n", rott_argc);
    
    static rott_task_args_t task_args;
    task_args.argc = rott_argc;
    task_args.argv = rott_argv;

    // ROTT uses about 6.6KB in measured gameplay; keep substantial headroom
    // while returning internal RAM for later latency-sensitive allocations.
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreatePinnedToCore(rott_task, "rott_task", 20 * 1024, &task_args, 5, NULL, 1);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
