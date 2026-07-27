#include <rg_system.h>
#include <inttypes.h>
#include "music.h"
#include "game.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Duke3D stack usage peaked at ~36KB in testing. Use 48KB in Internal DRAM.
// Must use MEM_FAST (internal DRAM) so that SPI flash cache disable operations
// work correctly when using LittleFS on internal storage (no SD card).
#define DUKE_STACK_SIZE (48 * 1024)
#define DUKE_ENGINE_CORE 0

static TaskHandle_t duke_task_handle = NULL;
// Flag to signal app_main that the engine task has finished cleanup.
volatile bool reboot_ready_flag = false;

static void ensure_dir(const char *path)
{
    char tmp[512];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    
    // Skip the root / mount point
    char *start = tmp + 1;
    if (strncmp(tmp, RG_STORAGE_ROOT "/", strlen(RG_STORAGE_ROOT "/")) == 0) 
        start = tmp + strlen(RG_STORAGE_ROOT "/");

    for (p = start; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            if (!rg_storage_exists(tmp)) {
                rg_storage_mkdir(tmp);
            }
            *p = '/';
        }
    }
    if (!rg_storage_exists(tmp)) {
        rg_storage_mkdir(tmp);
    }
}

#include "rg_system.h"
#include "game.h"
#include "controls.h"

const key_mapping_t keymap[] = {
    // Navigation (Any mode)
    {RG_KEY_UP,     SDL_SCANCODE_UP,    SDLK_UP,    RG_MODE_ANY},
    {RG_KEY_DOWN,   SDL_SCANCODE_DOWN,  SDLK_DOWN,  RG_MODE_ANY},
    {RG_KEY_LEFT,   SDL_SCANCODE_LEFT,  SDLK_LEFT,  RG_MODE_ANY},
    {RG_KEY_RIGHT,  SDL_SCANCODE_RIGHT, SDLK_RIGHT, RG_MODE_ANY},
    
    // A button
    {RG_KEY_A,      SDL_SCANCODE_LCTRL,  SDLK_LCTRL,    RG_MODE_GAME}, // Fire in game
    {RG_KEY_A,      SDL_SCANCODE_KP_ENTER, SDLK_KP_ENTER, RG_MODE_MENU}, // Confirm in menu
    {RG_KEY_A,      SDL_SCANCODE_RETURN,   SDLK_RETURN,   RG_MODE_MENU}, // Alternative confirm
    
    // B button
    {RG_KEY_B,      SDL_SCANCODE_A,      SDLK_a,        RG_MODE_GAME}, // Jump in game
    {RG_KEY_B,      SDL_SCANCODE_ESCAPE, SDLK_ESCAPE,   RG_MODE_MENU}, // Back in menu
    
    // Start button
    {RG_KEY_START,  SDL_SCANCODE_SPACE,  SDLK_SPACE,    RG_MODE_GAME}, // Use/Open in game
    {RG_KEY_START,  SDL_SCANCODE_KP_ENTER, SDLK_KP_ENTER, RG_MODE_MENU}, // Confirm in menu
    {RG_KEY_START,  SDL_SCANCODE_RETURN,   SDLK_RETURN,   RG_MODE_MENU}, // Alternative confirm
    
    // Select: Weapon Cycle
    {RG_KEY_SELECT, SDL_SCANCODE_APOSTROPHE, SDLK_QUOTE, RG_MODE_GAME},
    
    // X: Crouch
    {RG_KEY_X,      SDL_SCANCODE_Z,      SDLK_z,        RG_MODE_GAME},
    
    // Y: Jetpack
    {RG_KEY_Y,      SDL_SCANCODE_J,      SDLK_j,        RG_MODE_GAME},
    
    // Shoulder buttons: Strafe
    {RG_KEY_L,      SDL_SCANCODE_COMMA,  SDLK_COMMA,    RG_MODE_GAME},
    {RG_KEY_R,      SDL_SCANCODE_PERIOD, SDLK_PERIOD,   RG_MODE_GAME},

    // Dedicated Menu button always sends ESC to toggle menu
    {RG_KEY_MENU,   SDL_SCANCODE_ESCAPE, SDLK_ESCAPE,   RG_MODE_ANY},
    
    // Option: Crouch
    {RG_KEY_OPTION, SDL_SCANCODE_Z,      SDLK_z,        RG_MODE_GAME},
};

const size_t keymap_count = sizeof(keymap) / sizeof(keymap[0]);

// Hotkey mappings
const key_mapping_t shifted_keymap[] = {
    {RG_KEY_UP,    SDL_SCANCODE_RETURN,      SDLK_RETURN,       RG_MODE_GAME}, // Use Inventory Item (Up + Hotkey)
    {RG_KEY_DOWN,  SDL_SCANCODE_J,           SDLK_j,            RG_MODE_GAME}, // Jetpack (Down + Hotkey)
    {RG_KEY_LEFT,  SDL_SCANCODE_LEFTBRACKET, SDLK_LEFTBRACKET,  RG_MODE_GAME}, // Previous Item (Left + Hotkey)
    {RG_KEY_RIGHT, SDL_SCANCODE_RIGHTBRACKET,SDLK_RIGHTBRACKET, RG_MODE_GAME}, // Next Item (Right + Hotkey)
    {RG_KEY_B,     SDL_SCANCODE_END,         SDLK_END,          RG_MODE_GAME}, // Look Down (B + Hotkey)
    {RG_KEY_X,     SDL_SCANCODE_HOME,        SDLK_HOME,         RG_MODE_GAME}, // Look Up (X + Hotkey)
    {RG_KEY_OPTION,SDL_SCANCODE_HOME,        SDLK_HOME,         RG_MODE_GAME}, // Look Up (OPTION + Hotkey)
};

const size_t shifted_keymap_count = sizeof(shifted_keymap) / sizeof(shifted_keymap[0]);

void dukeTask(void *pvParameters)
{
    void audio_shutdown(void);

    RG_LOGI("dukeTask: Starting main loop on Core %d with %dKB stack\n", 
           xPortGetCoreID(), DUKE_STACK_SIZE / 1024);
    char *argv[]={"duke3d", NULL};
    int argc = 1;
    while (!rg_system_should_exit())
    {
        main(argc, argv);
        break; // main returned, we are done
    }
    RG_LOGW("dukeTask: Main loop exited.");
    
    // Give some time for background tasks or final OS cleanup on this core
    vTaskDelay(pdMS_TO_TICKS(50));

    // Signal app_main to perform the reboot.
    reboot_ready_flag = true;
    
    // We are done. The engine core will now run its idle task.
    vTaskDelete(NULL);
}

static void event_handler(int event, void *arg)
{
    // Default handlers
}

void app_main(void)
{
    int sample_rate = 11025;

#if defined(RG_AUDIO_USE_INT_DAC) && RG_AUDIO_USE_INT_DAC > 0
    sample_rate = 22050;
#endif

    const rg_config_t config = {
        .sampleRate = sample_rate,
        .frameRate = 30,
        .storageRequired = true,
        .romRequired = false,
#if CONFIG_IDF_TARGET_ESP32
        .mallocAlwaysInternal = 512,
#else
        .mallocAlwaysInternal = 1024,
#endif
        .handlers.event = &event_handler,
    };

    rg_system_init(&config);
    // Keep Retro-Go's periodic STACK/HEAP/BUSY/FPS status line enabled.
    rg_system_set_log_level(RG_LOG_DEBUG);
    if (!rg_settings_exists(NS_APP, "DispScaling")) {
        rg_display_set_scaling(RG_DISPLAY_SCALING_FULL);
    }

    ensure_dir(RG_BASE_PATH_SAVES "/duke3d");
    ensure_dir(RG_BASE_PATH_CONFIG);

    RG_LOGI("app_main: Spawning Duke3D task...");
    
    static StaticTask_t duke_task_buffer;
    void *stack_ptr = rg_alloc(DUKE_STACK_SIZE, MEM_FAST);

    if (!stack_ptr) {
        RG_LOGE("Failed to allocate %dKB stack!", DUKE_STACK_SIZE / 1024);
        return;
    }
    
    duke_task_handle = xTaskCreateStaticPinnedToCore(
        dukeTask,           /* Function that implements the task. */
        "dukeTask",         /* Text name for the task. */
        DUKE_STACK_SIZE,    /* Stack size in bytes. */
        NULL,               /* Parameter passed into the task. */
        5,                  /* Priority at which the task is created. */
        stack_ptr,          /* Stack buffer */
        &duke_task_buffer,  /* Task buffer */
        DUKE_ENGINE_CORE    /* Keep the renderer off Retro-Go's Core 1 display task */
    );

    if (duke_task_handle == NULL) {
        RG_LOGE("Failed to create dukeTask!");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (reboot_ready_flag) {
            vTaskDelay(pdMS_TO_TICKS(250)); // Final safety buffer
            RG_LOGI("app_main: Rebooting system...");
            rg_system_exit();
        }
    }
}
