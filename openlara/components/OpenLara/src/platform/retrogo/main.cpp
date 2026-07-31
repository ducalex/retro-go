#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <retro-go.h>

#include <esp_memory_utils.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "game.h"

#if RG_SCREEN_PIXEL_FORMAT == 0
    #define FB_PIXEL_FORMAT RG_PIXEL_565_BE
    #define FB_PIXEL_BYTE_ORDER "BE"
#else
    #define FB_PIXEL_FORMAT RG_PIXEL_565_LE
    #define FB_PIXEL_BYTE_ORDER "LE"
#endif

Inventory *inventory = NULL;

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;
static rg_surface_t *volatile lastUpdate;

enum MenuRequest {
    MENU_REQUEST_NONE,
    MENU_REQUEST_GAME,
    MENU_REQUEST_OPTIONS,
};

static volatile int menu_request = MENU_REQUEST_NONE;
static volatile bool timing_rebase_requested = false;
static volatile bool shutdown_requested = false;
static volatile bool game_started = false;
static volatile bool game_finished = false;

namespace Game {
    volatile bool sound_active = false;
    volatile bool sound_running = false;
    volatile bool sound_finished = false;
}

// --- Timing ---
int osGetTimeMS() {
    return (int)(rg_system_timer() / 1000);
}

// --- Input ---
bool osJoyReady(int index) { return index == 0; }
void osJoyVibrate(int index, float L, float R) {}

static const struct { uint32_t rg; JoyKey ol; } keymap[] = {
    {RG_KEY_UP, jkUp}, {RG_KEY_DOWN, jkDown}, {RG_KEY_LEFT, jkLeft}, {RG_KEY_RIGHT, jkRight},
    {RG_KEY_A, jkA}, {RG_KEY_B, jkB},
    {RG_KEY_X | RG_KEY_SELECT, jkX}, // X or SELECT for Duck (jkX)
    {RG_KEY_Y | RG_KEY_START, jkY},  // Y or START for Weapon (jkY)
    {RG_KEY_MENU, jkSelect}, {RG_KEY_OPTION, jkLT}, // MENU for Inventory (jkSelect), OPTION for Roll (jkLT)
    {RG_KEY_L, jkLB}, {RG_KEY_R, jkRB}
};

static void setJoyState(uint32_t joystick) {
    for (int i = 0; i < RG_COUNT(keymap); i++) {
        Input::setJoyDown(0, keymap[i].ol, (joystick & keymap[i].rg) != 0);
    }
}

static bool joyUpdate() {
    static const int MENU_HOLD_TICKS = 15;
    static uint32_t heldMenuKey = 0;
    static int heldTicks = 0;
    static bool menuConsumed = false;

    uint32_t joystick = rg_input_read_gamepad();
    uint32_t menuKey = (joystick & RG_KEY_MENU) ? RG_KEY_MENU :
                       (joystick & RG_KEY_OPTION) ? RG_KEY_OPTION : 0;

    if (menuKey != heldMenuKey) {
        heldMenuKey = menuKey;
        heldTicks = 0;
        menuConsumed = false;
    }

    if (menuKey && !menuConsumed && ++heldTicks >= MENU_HOLD_TICKS) {
        menuConsumed = true;
        setJoyState(0);
        Game::sound_active = false;
        menu_request = (menuKey == RG_KEY_MENU) ? MENU_REQUEST_GAME : MENU_REQUEST_OPTIONS;

        while (menu_request != MENU_REQUEST_NONE && !shutdown_requested) {
            rg_task_delay(10);
        }

        if (!shutdown_requested) {
            Game::sound_active = true;
        }
        return true;
    }

    // A long press belongs to RetroGo until the key is released. Short presses
    // continue to reach OpenLara as Inventory and Roll.
    if (menuConsumed) {
        joystick &= ~heldMenuKey;
    }

    setJoyState(joystick);
    return false;
}

// --- Sound ---
#define AUDIO_SAMPLES 1024
static int16_t sound_buffer[AUDIO_SAMPLES * 2];

static void sound_task(void *arg) {
    Game::sound_finished = false;

    while (!Core::isQuit && !shutdown_requested) {
        // Publish ownership before sampling sound_active. This closes the
        // hand-off race with Game::startLevel(), which disables audio and
        // waits for sound_running before replacing engine sound state.
        Game::sound_running = true;
        bool active = Game::sound_active;
        if (active) {
            Sound::fill((Sound::Frame*)sound_buffer, AUDIO_SAMPLES);
            rg_audio_submit((rg_audio_frame_t*)sound_buffer, AUDIO_SAMPLES);
        }
        Game::sound_running = false;

        if (!active) {
            rg_task_yield();
        }
    }

    Game::sound_running = false;
    rg_system_log(RG_LOG_INFO, "OpenLara", "Audio task minimum free stack: %u bytes\n",
                  (unsigned)uxTaskGetStackHighWaterMark(NULL));
    Game::sound_finished = true;
    vTaskDelete(NULL);
}

// --- RetroGo handlers ---
static bool screenshot_handler(const char *filename, int width, int height) {
    const rg_surface_t *surface = lastUpdate;
    return surface && rg_surface_save_image_file(surface, filename, width, height);
}

static bool save_state_handler(const char *filename) {
    rg_gui_alert(_("Not implemented"), _("Please use OpenLara's inventory save system."));
    return false;
}

static bool load_state_handler(const char *filename) {
    rg_gui_alert(_("Not implemented"), _("Please use OpenLara's inventory load system."));
    return false;
}

static bool reset_handler(bool hard) {
    rg_gui_alert(_("Not implemented"), _("Please use OpenLara's title menu."));
    return false;
}

static void event_handler(int event, void *arg) {
    if (event == RG_EVENT_REDRAW) {
        const rg_surface_t *surface = lastUpdate;
        if (surface) {
            rg_display_submit(surface, 0);
        }
    } else if (event == RG_EVENT_SPEEDUP) {
        // The local renderer-only frameskip policy owns this field.
        rg_system_get_app()->frameskip = -1;
        timing_rebase_requested = true;
    } else if (event == RG_EVENT_SHUTDOWN) {
        // Shutdown may be requested by the RetroGo menu. Let the engine and
        // audio task release their state before shared services are stopped.
        shutdown_requested = true;
        Game::sound_active = false;
        while (game_started && !game_finished) {
            rg_task_delay(10);
        }
    }
}

static bool formatPath(char *dest, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(dest, size, format, args);
    va_end(args);
    return length >= 0 && (size_t)length < size;
}

// --- Entry Point ---
static void openlara_task(void *arg) {
    rg_app_t *app = (rg_app_t*)arg;
    game_started = true;

    // Set OpenLara paths
    char levelName[256] = "";
    if (app->romPath && app->romPath[0]) {
        if (!formatPath(contentDir, sizeof(contentDir), "%s", app->romPath)) {
            RG_PANIC("OpenLara content path is too long.");
        }

        char *lastSlash = strrchr(contentDir, '/');
        if (lastSlash) {
            if (!formatPath(levelName, sizeof(levelName), "%s", lastSlash + 1)) {
                RG_PANIC("OpenLara level name is too long.");
            }
            *(lastSlash + 1) = 0;
        }

        // Try to find the assets directory by looking for TITLE files
        char base[256];
        if (!formatPath(base, sizeof(base), "%s", contentDir)) {
            RG_PANIC("OpenLara base path is too long.");
        }

        const char *dataFolders[] = {"data/", "PSXDATA/", "DELDATA/", "TR1_PSX/PSXDATA/", "TR1_PC/DATA/", ""};
        const char *titles[] = {"TITLE.PHD", "title.phd", "TITLE.PSX", "title.psx", "GAME.PSX", "game.psx"};

        bool found = false;
        for (int i = 0; i < RG_COUNT(dataFolders); i++) {
            for (int j = 0; j < RG_COUNT(titles); j++) {
                char path[256];
                if (!formatPath(path, sizeof(path), "%s%s%s", base, dataFolders[i], titles[j])) {
                    continue;
                }
                if (rg_storage_exists(path)) {
                    if (!formatPath(contentDir, sizeof(contentDir), "%s%s", base, dataFolders[i]) ||
                        !formatPath(levelName, sizeof(levelName), "%s", titles[j])) {
                        RG_PANIC("Resolved OpenLara path is too long.");
                    }
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        // The documented PC layout keeps TITLE.PHD below data/DATA while
        // contentDir must remain at data/ so PIX and AUDIO stay reachable.
        if (!found) {
            const char *nestedDataFolders[] = {"data/DATA/", "data/data/"};
            for (int i = 0; i < 2 && !found; i++) {
                for (int j = 0; j < 2; j++) {
                    char path[256];
                    if (!formatPath(path, sizeof(path), "%s%s%s", base, nestedDataFolders[i], titles[j])) {
                        continue;
                    }
                    if (rg_storage_exists(path)) {
                        if (!formatPath(contentDir, sizeof(contentDir), "%sdata/", base) ||
                            !formatPath(levelName, sizeof(levelName), "%s", titles[j])) {
                            RG_PANIC("Resolved OpenLara path is too long.");
                        }
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) {
            if (!formatPath(contentDir, sizeof(contentDir), "%sdata/", base) ||
                !formatPath(levelName, sizeof(levelName), "%s", "TITLE.PHD")) {
                RG_PANIC("Default OpenLara path is too long.");
            }
            rg_system_log(RG_LOG_WARN, "OpenLara", "No game data found, using defaults.\n");
        } else {
            rg_system_log(RG_LOG_INFO, "OpenLara", "Found game data at: %s\n", contentDir);
        }
    } else {
        RG_ASSERT(formatPath(contentDir, sizeof(contentDir), "%s", RG_BASE_PATH_ROMS "/openlara/data/"),
                  "Default OpenLara content path is too long.");
        RG_ASSERT(formatPath(levelName, sizeof(levelName), "%s", "TITLE.PHD"),
                  "Default OpenLara level name is too long.");
    }
    RG_ASSERT(formatPath(saveDir, sizeof(saveDir), "%s", RG_BASE_PATH_SAVES "/openlara"),
              "OpenLara save path is too long.");
    RG_ASSERT(formatPath(cacheDir, sizeof(cacheDir), "%s", RG_BASE_PATH_CACHE "/openlara"),
              "OpenLara cache path is too long.");

    RG_ASSERT(rg_storage_mkdir(saveDir), "Unable to create OpenLara save directory.");
    RG_ASSERT(rg_storage_mkdir(cacheDir), "Unable to create OpenLara cache directory.");

    rg_system_log(RG_LOG_INFO, "OpenLara", "Paths: contentDir='%s' levelName='%s'\n", contentDir, levelName);

    // Initialize display surfaces - use double buffering to prevent screen tearing!
    updates[0] = rg_surface_create(320, 240, FB_PIXEL_FORMAT, MEM_SLOW);
    updates[1] = rg_surface_create(320, 240, FB_PIXEL_FORMAT, MEM_SLOW);
    RG_ASSERT(updates[0] && updates[1], "Unable to allocate OpenLara display surfaces.");
    RG_ASSERT(esp_ptr_external_ram(updates[0]) && esp_ptr_external_ram(updates[1]),
              "OpenLara display surfaces must reside in PSRAM.");
    rg_system_log(RG_LOG_INFO, "OpenLara", "Framebuffer: RGB565-%s native byte order.\n",
                  FB_PIXEL_BYTE_ORDER);
    currentUpdate = updates[0];
    lastUpdate = NULL;

    // Force display to stretch and fill the screen (ignoring aspect ratio).
    rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);

    Core::width = 320;
    Core::height = 240;
    GAPI::resize();
    GAPI::swColor = (uint16_t*)currentUpdate->data;

    // Initialize Sound Task
    rg_task_t *soundTask = rg_task_create("ol_sound", sound_task, NULL, 16384, 1, RG_TASK_PRIORITY_6, 1);
    RG_ASSERT(soundTask, "Unable to start the OpenLara audio task.");

    // Initialize Game
    TR::useEasyStart = true;
    TR::isGameEnded = false;
    Game::sound_active = false;
    Game::init(levelName[0] ? levelName : NULL);
    Game::sound_active = true;

    uint32_t last_time = rg_system_timer();
    int skipFrames = 0;

    // OpenLara uses its measured per-frame cost below to choose render skips.
    // Disable the slower system controller so both policies cannot oscillate.
    app->frameskip = -1;

    while (!Core::isQuit && !shutdown_requested) {
        if (timing_rebase_requested) {
            timing_rebase_requested = false;
            last_time = rg_system_timer();
            skipFrames = 0;
            continue;
        }

        uint32_t current_time = rg_system_timer();
        uint32_t frame_time_us = app->frameTime;
        int32_t frames = (current_time - last_time) / frame_time_us;

        if (frames > 0) {
            // Preserve the 30 Hz deadline phase through ordinary slow frames.
            // Very large stalls are not useful catch-up work and are discarded.
            if (frames > 4) {
                last_time = current_time;
                skipFrames = RG_MAX(skipFrames, 1);
            } else {
                last_time += frame_time_us;
                // If more than one deadline is already due, spend the backlog
                // on simulation-only ticks before attempting another render.
                if (frames > 1)
                    skipFrames = RG_MAX(skipFrames, RG_MIN(frames - 1, 5));
            }

            const int64_t start_time = rg_system_timer();
            const bool draw_frame = (skipFrames == 0);
            bool rendered = false;

            if (joyUpdate()) {
                last_time = rg_system_timer();
                skipFrames = 0;
                continue;
            }

            Game::update();
            const bool timing_reset = Core::resetState;

            if (draw_frame) {
                GAPI::swColor = (uint16_t*)currentUpdate->data;
                rendered = Game::render();
            }

            // CPU busy time deliberately excludes display queue waiting and pacing.
            const int elapsed_before_submit = (int)(rg_system_timer() - start_time);
            const int busy_time = timing_reset ? 0 : elapsed_before_submit;

            if (rendered) {
                // Wait for queue space without charging display latency as CPU work.
                while (rg_display_is_busy()) {
                    rg_task_yield();
                }

                lastUpdate = currentUpdate;
                rg_display_submit(currentUpdate, 0);
                currentUpdate = updates[currentUpdate == updates[0]];
            }

            rg_system_tick(busy_time);

            if (timing_reset) {
                // Level changes deliberately reset OpenLara's clock. Do the same
                // for our deadline and do not let loading poison auto-frameskip.
                last_time = rg_system_timer();
                skipFrames = 0;
                continue;
            }

            // Skip rendering only. Input, simulation, audio, and system ticks continue.
            if (skipFrames == 0) {
                int elapsed = (int)(rg_system_timer() - start_time);
                int required_skip = app->frameskip > 0 ? app->frameskip : 0;
                if (elapsed > app->frameTime + 1500)
                    required_skip = RG_MAX(required_skip, elapsed / app->frameTime);
                skipFrames = RG_MIN(required_skip, 5);
            } else {
                skipFrames--;
            }
        } else {
            rg_task_yield();
        }
    }

    Core::isQuit = true;
    Game::sound_active = false;

    // Sound::fill() and rg_audio_submit() must both be finished before the
    // engine releases their state during Game::deinit().
    while (!Game::sound_finished) {
        rg_task_delay(10);
    }

    Game::deinit();

    // Keep submitted surface pixels alive until the asynchronous display releases them.
    while (rg_display_is_busy()) {
        rg_task_delay(10);
    }

    GAPI::swColor = NULL;
    lastUpdate = NULL;
    currentUpdate = NULL;
    rg_surface_free(updates[0]);
    rg_surface_free(updates[1]);
    updates[0] = updates[1] = NULL;

    rg_system_log(RG_LOG_INFO, "OpenLara", "Main task minimum free stack: %u bytes\n",
                  (unsigned)uxTaskGetStackHighWaterMark(NULL));
    game_finished = true;
    vTaskDelete(NULL);
}

extern "C" void app_main() {
    const rg_config_t config = {
        .sampleRate = 22050,
        .frameRate = 30,
        .storageRequired = true,
        .romRequired = false,
        .isLauncher = false,
        .handlers = {
            .loadState = &load_state_handler,
            .saveState = &save_state_handler,
            .reset = &reset_handler,
            .screenshot = &screenshot_handler,
            .event = &event_handler,
        },
        .mallocAlwaysInternal = 0
    };
    rg_app_t *app = rg_system_init(&config);

    // Launch main engine loop in a dedicated task with a huge stack
    // Allocate the stack in PSRAM (MEM_SLOW) to prevent starving Retro-Go's DMA queues!
    // The TCB MUST be in Internal RAM (MEM_FAST) per ESP-IDF FreeRTOS requirements!
    static StaticTask_t *ol_task_tcb = (StaticTask_t*)rg_alloc(sizeof(StaticTask_t), MEM_FAST);
    static StackType_t *ol_task_stack = (StackType_t*)rg_alloc(48 * 1024, MEM_SLOW);
    RG_ASSERT(esp_ptr_internal(ol_task_tcb), "OpenLara task control block must reside in internal RAM.");
    RG_ASSERT(esp_ptr_external_ram(ol_task_stack), "OpenLara main stack must reside in PSRAM.");

    TaskHandle_t openlaraTask = xTaskCreateStaticPinnedToCore(
        openlara_task,
        "ol_main",
        48 * 1024,
        app,
        RG_TASK_PRIORITY_2,
        ol_task_stack,
        ol_task_tcb,
        0
    );
    RG_ASSERT(openlaraTask, "Unable to start the OpenLara main task.");

    // Run RetroGo menus from this internal-stack context. This also keeps every
    // shutdown path safe on targets whose flash APIs cannot use a PSRAM stack.
    while (!game_finished) {
        int request = menu_request;
        if (request == MENU_REQUEST_GAME) {
            rg_gui_game_menu();
            menu_request = MENU_REQUEST_NONE;
        } else if (request == MENU_REQUEST_OPTIONS) {
            rg_gui_options_menu();
            menu_request = MENU_REQUEST_NONE;
        } else {
            rg_task_delay(10);
        }
    }

    // Ensure the engine has finished teardown before returning to the launcher.
    rg_task_delay(50); // Extra safety for FreeRTOS idle cleanup
    rg_system_exit();
}
