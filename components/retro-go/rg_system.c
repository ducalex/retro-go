#include "rg_system.h"
#include "rg_printf.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef RG_TARGET_SDL2
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include <esp_chip_info.h>
#endif
#else
#include <SDL2/SDL.h>
#endif

#define RG_LOGBUF_SIZE 2048
typedef struct
{
    char buffer[RG_LOGBUF_SIZE];
    size_t cursor;
} logbuf_t;

#define RG_STRUCT_MAGIC 0x12345678
typedef struct
{
    uint32_t magicWord;
    char message[256];
    char context[128];
    rg_stats_t statistics;
    logbuf_t logbuf;
} panic_trace_t;

typedef struct
{
    int32_t totalFrames, fullFrames, ticks;
    int64_t busyTime, updateTime;
} counters_t;

typedef struct
{
    TaskHandle_t handle;
    char name[20];
} rg_task_t;

#ifdef RG_ENABLE_PROFILING
typedef struct
{
    void *func_ptr, *caller_ptr;
    int32_t num_calls, run_time;
    int64_t enter_time;
} profile_frame_t;

static struct
{
    int64_t time_started;
    int32_t total_frames;
    SemaphoreHandle_t lock;
    profile_frame_t frames[512];
} *profile;
#endif

// The trace will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
static RTC_NOINIT_ATTR time_t rtcValue;
static rg_stats_t statistics;
static rg_app_t app;
static logbuf_t logbuf;
static rg_task_t tasks[8];
static int ledValue = -1;
static int wdtCounter = 0;
static bool exitCalled = false;

static const char *SETTING_BOOT_NAME = "BootName";
static const char *SETTING_BOOT_ARGS = "BootArgs";
static const char *SETTING_BOOT_FLAGS = "BootFlags";
static const char *SETTING_TIMEZONE = "Timezone";

#define WDT_TIMEOUT 10000000
#define WDT_RELOAD(val) wdtCounter = (val)

#define logbuf_putc(buf, c) (buf)->buffer[(buf)->cursor++] = c, (buf)->cursor %= RG_LOGBUF_SIZE;
#define logbuf_puts(buf, str) for (const char *ptr = str; *ptr; ptr++) logbuf_putc(buf, *ptr);


void rg_system_load_time(void)
{
    time_t time_sec = RG_MAX(rtcValue, RG_BUILD_TIME);
    FILE *fp;
#if 0
    if (rg_i2c_read(0x68, 0x00, data, sizeof(data)))
    {
        RG_LOGI("Time loaded from DS3231\n");
    }
    else
#endif
    if ((fp = fopen(RG_BASE_PATH_CACHE "/clock.bin", "rb")))
    {
        fread(&time_sec, sizeof(time_sec), 1, fp);
        fclose(fp);
        RG_LOGI("Time loaded from storage\n");
    }
#ifndef RG_TARGET_SDL2
    settimeofday(&(struct timeval){time_sec, 0}, NULL);
#endif
    time_sec = time(NULL); // Read it back to be sure it worked
    RG_LOGI("Time is now: %s\n", asctime(localtime(&time_sec)));
}

void rg_system_save_time(void)
{
    time_t time_sec = time(NULL);
    FILE *fp;
    // We always save to storage in case the RTC disappears.
    if ((fp = fopen(RG_BASE_PATH_CACHE "/clock.bin", "wb")))
    {
        fwrite(&time_sec, sizeof(time_sec), 1, fp);
        fclose(fp);
        RG_LOGI("System time saved to storage.\n");
    }
#if 0
    if (rg_i2c_write(0x68, 0x00, data, sizeof(data)))
    {
        RG_LOGI("System time saved to DS3231.\n");
    }
#endif
}

void rg_system_set_timezone(const char *TZ)
{
    rg_settings_set_string(NS_GLOBAL, SETTING_TIMEZONE, TZ);
    setenv("TZ", TZ, 1);
    tzset();
}

static inline void begin_panic_trace(const char *context, const char *message)
{
    panicTrace.magicWord = RG_STRUCT_MAGIC;
    panicTrace.statistics = statistics;
    panicTrace.logbuf = logbuf;
    strncpy(panicTrace.message, message ?: "(none)", sizeof(panicTrace.message) - 1);
    strncpy(panicTrace.context, context ?: "(none)", sizeof(panicTrace.context) - 1);
    panicTrace.message[sizeof(panicTrace.message) - 1] = 0;
    panicTrace.context[sizeof(panicTrace.context) - 1] = 0;
    logbuf_puts(&panicTrace.logbuf, "\n\n*** PANIC TRACE: ***\n\n");
}

IRAM_ATTR void esp_panic_putchar_hook(char c)
{
    if (panicTrace.magicWord != RG_STRUCT_MAGIC)
        begin_panic_trace("esp_panic", NULL);
    logbuf_putc(&panicTrace.logbuf, c);
}

static void update_memory_statistics(void)
{
#ifndef RG_TARGET_SDL2
    multi_heap_info_t heap_info;

    heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    statistics.freeMemoryInt = heap_info.total_free_bytes;
    statistics.freeBlockInt = heap_info.largest_free_block;
    statistics.totalMemoryInt = heap_info.total_free_bytes + heap_info.total_allocated_bytes;
    heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    statistics.freeMemoryExt = heap_info.total_free_bytes;
    statistics.freeBlockExt = heap_info.largest_free_block;
    statistics.totalMemoryExt = heap_info.total_free_bytes + heap_info.total_allocated_bytes;

    statistics.freeStackMain = uxTaskGetStackHighWaterMark(tasks[0].handle);
#endif
}

static void update_statistics(void)
{
    static counters_t counters = {0};
    const counters_t previous = counters;

    rg_display_counters_t display = rg_display_get_counters();
    // rg_audio_counters_t audio = rg_audio_get_counters();

    counters.totalFrames = display.totalFrames;
    counters.fullFrames = display.fullFrames;
    counters.busyTime = statistics.busyTime;
    counters.ticks = statistics.ticks;
    counters.updateTime = rg_system_timer();

    float elapsedTime = (counters.updateTime - previous.updateTime) / 1000000.f;
    statistics.busyPercent = RG_MIN((counters.busyTime - previous.busyTime) / (elapsedTime * 1000000.f) * 100.f, 100.f);
    statistics.totalFPS = (counters.ticks - previous.ticks) / elapsedTime;
    statistics.skippedFPS = statistics.totalFPS - ((counters.totalFrames - previous.totalFrames) / elapsedTime);
    statistics.fullFPS = (counters.fullFrames - previous.fullFrames) / elapsedTime;

    update_memory_statistics();
}

static void system_monitor_task(void *arg)
{
    int64_t lastLoop = rg_system_timer();
    int32_t numLoop = 0;
    float batteryPercent = 0.f;
    bool ledState = false;

    // Give the app a few seconds to start before monitoring
    rg_task_delay(2500);
    WDT_RELOAD(WDT_TIMEOUT);

    while (!exitCalled)
    {
        int loopTime_us = lastLoop - rg_system_timer();
        lastLoop = rg_system_timer();
        rtcValue = time(NULL);

        // Maybe we should *try* to wait for vsync before updating?
        update_statistics();

        if (rg_input_read_battery(&batteryPercent, NULL))
        {
            if (batteryPercent < 2)
                rg_system_set_led((ledState ^= 1));
            else if (ledState)
                rg_system_set_led((ledState = 0));
        }

        RG_LOGX("STACK:%d, HEAP:%d+%d (%d+%d), BUSY:%.2f, FPS:%.2f (SKIP:%d, PART:%d, FULL:%d), BATT:%.2f\n",
            statistics.freeStackMain,
            statistics.freeMemoryInt / 1024,
            statistics.freeMemoryExt / 1024,
            statistics.freeBlockInt / 1024,
            statistics.freeBlockExt / 1024,
            statistics.busyPercent,
            statistics.totalFPS,
            (int)(statistics.skippedFPS + 0.9f),
            (int)(statistics.totalFPS - statistics.skippedFPS - statistics.fullFPS + 0.9f),
            (int)(statistics.fullFPS + 0.9f),
            batteryPercent);

        if ((wdtCounter -= loopTime_us) <= 0)
        {
            if (rg_input_gamepad_last_read() > WDT_TIMEOUT)
            {
            #ifdef RG_ENABLE_PROFILING
                RG_LOGW("Application unresponsive!\n");
            #else
                RG_PANIC("Application unresponsive!");
            #endif
            }
            WDT_RELOAD(WDT_TIMEOUT);
        }

        #ifdef RG_ENABLE_PROFILING
            if ((numLoop % 10) == 0)
                rg_system_dump_profile();
        #endif

        // if ((numLoop % 300) == 299)
        // {
        //     RG_LOGI("Saving system time");
        //     rg_system_save_time();
        // }

        rg_task_delay(1000);
        numLoop++;
    }

    rg_task_delete(NULL);
}

static void enter_recovery_mode(void)
{
    RG_LOGW("Entering recovery mode...\n");

    const rg_gui_option_t options[] = {
        {0, "Reset all settings", NULL, 1, NULL},
        {1, "Reboot to factory ", NULL, 1, NULL},
        {2, "Reboot to launcher", NULL, 1, NULL},
        RG_DIALOG_CHOICE_LAST,
    };
    while (true)
    {
        switch (rg_gui_dialog("Recovery mode", options, -1))
        {
        case 0:
            rg_storage_delete(RG_BASE_PATH_CONFIG);
            rg_storage_delete(RG_BASE_PATH_CACHE);
            rg_settings_reset();
            break;
        case 1:
            rg_system_switch_app(RG_APP_FACTORY, RG_APP_FACTORY, 0, 0);
        case 2:
        default:
            rg_system_switch_app(RG_APP_FACTORY, RG_APP_LAUNCHER, 0, 0);
        }
    }
}

static void setup_gpios(void)
{
// At boot time those pins are muxed to JTAG and can interfere with other things.
#if CONFIG_IDF_TARGET_ESP32
    gpio_reset_pin(GPIO_NUM_12);
    gpio_reset_pin(GPIO_NUM_13);
    gpio_reset_pin(GPIO_NUM_14);
    gpio_reset_pin(GPIO_NUM_15);
#endif
#ifndef RG_TARGET_SDL2
    if (RG_GPIO_LED != GPIO_NUM_NC)
        gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
#endif
}

rg_app_t *rg_system_reinit(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options)
{
    if (!app.initialized)
        return rg_system_init(sampleRate, handlers, options);

    app.sampleRate = sampleRate;
    if (handlers)
        app.handlers = *handlers;
    app.options = options;
    tasks[0].handle = xTaskGetCurrentTaskHandle();
    rg_audio_set_sample_rate(app.sampleRate);

    return &app;
}

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options)
{
    RG_ASSERT(app.initialized == false, "rg_system_init() was already called.");
    const esp_app_desc_t *esp_app = esp_ota_get_app_description();

    app = (rg_app_t){
        .name = esp_app->project_name,
        .version = esp_app->version,
        .buildDate = esp_app->date,
        .buildTime = esp_app->time,
        .buildUser = RG_BUILD_USER,
        .toolchain = esp_get_idf_version(),
        .bootArgs = NULL,
        .bootFlags = 0,
        .bootType = RG_RST_POWERON,
        .speed = 1.f,
        .refreshRate = 60,
        .sampleRate = sampleRate,
        .logLevel = RG_LOG_INFO,
        .options = options, // TO DO: We should make a copy of it?
    };

    tasks[0] = (rg_task_t){xTaskGetCurrentTaskHandle(), "main"};

    // Do this very early, may be needed to enable serial console
    setup_gpios();

    printf("\n========================================================\n");
    printf("%s %s (%s %s)\n", app.name, app.version, app.buildDate, app.buildTime);
    printf(" built for: %s. aud=%d disp=%d pad=%d sd=%d cfg=%d\n", RG_TARGET_NAME, 0, 0, 0, 0, 0);
    printf("========================================================\n\n");

#ifndef RG_TARGET_SDL2
    esp_reset_reason_t r_reason = esp_reset_reason();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    RG_LOGI("Chip info: model %d rev%d (%d cores), reset reason: %d\n",
        chip_info.model, chip_info.revision, chip_info.cores, r_reason);
    if (r_reason == ESP_RST_PANIC || r_reason == ESP_RST_TASK_WDT || r_reason == ESP_RST_INT_WDT)
        app.bootType = RG_RST_PANIC;
    else if (r_reason == ESP_RST_SW)
        app.bootType = RG_RST_RESTART;
#endif

    update_memory_statistics();
    RG_LOGI("Internal memory: free=%d, total=%d\n", statistics.freeMemoryInt, statistics.totalMemoryInt);
    RG_LOGI("External memory: free=%d, total=%d\n", statistics.freeMemoryExt, statistics.totalMemoryExt);

    rg_system_set_led(0);

    // Storage must be initialized first (SPI bus, settings, assets, etc)
    rg_storage_init();

    if ((app.isLauncher = strcmp(app.name, RG_APP_LAUNCHER) == 0))
    {
        app.configNs = app.name;
    }
    else
    {
        app.configNs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_NAME, app.name);
        app.bootArgs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_ARGS, "");
        app.bootFlags = rg_settings_get_number(NS_BOOT, SETTING_BOOT_FLAGS, 0);
        app.saveSlot = (app.bootFlags & RG_BOOT_SLOT_MASK) >> 4;
        app.romPath = app.bootArgs;
    }

    rg_input_init(); // Must be first for the qtpy (input -> aw9523 -> lcd)
    rg_display_init();
    rg_gui_init();

    // Test for recovery request as early as possible
    for (int timeout = 5, btn; (btn = rg_input_read_gamepad() & RG_RECOVERY_BTN) && timeout >= 0; --timeout)
    {
        RG_LOGW("Button " PRINTF_BINARY_16 " being held down...\n", PRINTF_BINVAL_16(btn));
        rg_task_delay(100);
        if (timeout == 0)
            enter_recovery_mode();
    }

    rg_gui_draw_hourglass();
    rg_audio_init(sampleRate);

    // Show alert if we've just rebooted from a panic
    if (app.bootType == RG_RST_PANIC)
    {
        char message[400] = "Application crashed";

        if (panicTrace.magicWord == RG_STRUCT_MAGIC)
        {
            RG_LOGI("Panic log found, saving to sdcard...\n");
            if (panicTrace.message[0] && strcmp(panicTrace.message, "(none)") != 0)
                strcpy(message, panicTrace.message);

            if (rg_system_save_trace(RG_STORAGE_ROOT "/crash.log", 1))
                strcat(message, "\nLog saved to SD Card.");
        }

        RG_LOGW("Aborting: panic!\n");
        rg_display_clear(C_BLUE);
        rg_gui_alert("System Panic!", message);
        rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0);
    }
    panicTrace.magicWord = 0;

#ifndef RG_TARGET_SDL2
    if (app.bootFlags & RG_BOOT_ONCE)
        esp_ota_set_boot_partition(esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, RG_APP_LAUNCHER));
#endif

    rg_system_set_timezone(rg_settings_get_string(NS_GLOBAL, SETTING_TIMEZONE, "EST+5"));
    rg_system_load_time();

    // Do these last to not interfere with panic handling above
    if (handlers)
        app.handlers = *handlers;

    #ifdef RG_ENABLE_PROFILING
    RG_LOGI("Profiling has been enabled at compile time!\n");
    profile = rg_alloc(sizeof(*profile), MEM_SLOW);
    profile->lock = xSemaphoreCreateMutex();
    #endif

    rg_task_create("rg_system", &system_monitor_task, NULL, 3 * 1024, RG_TASK_PRIORITY, -1);

    app.initialized = true;

    RG_LOGI("Retro-Go ready.\n\n");

    return &app;
}

bool rg_task_create(const char *name, void (*taskFunc)(void *data), void *data, size_t stackSize, int priority, int affinity)
{
    RG_ASSERT(name && taskFunc, "bad param");
    TaskHandle_t handle = NULL;

#ifndef RG_TARGET_SDL2
    if (affinity < 0)
        affinity = tskNO_AFFINITY;
    if (xTaskCreatePinnedToCore(taskFunc, name, stackSize, data, priority, &handle, affinity) != pdPASS)
        handle = NULL; // should already be NULL...
#else
    if ((handle = SDL_CreateThread(taskFunc, name, data)))
        SDL_DetachThread(handle);
#endif

    if (!handle)
    {
        RG_LOGE("Task creation failed: name='%s', fn='%p', stack=%d\n", name, taskFunc, stackSize);
        return false;
    }

    for (size_t i = 0; i < RG_COUNT(tasks); ++i)
    {
        if (tasks[i].handle == NULL)
        {
            tasks[i].handle = handle;
            strncpy(tasks[i].name, name, 20);
            return true;
        }
    }

    RG_LOGW("Task queue full! Task '%s' is running but not tracked...\n", name);
    return true;
}

bool rg_task_delete(const char *name)
{
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    for (size_t i = 0; i < RG_COUNT(tasks); ++i)
    {
        if ((!name && tasks[i].handle == current) || (name && strncmp(tasks[i].name, name, 20) != 0))
        {
        #ifndef RG_TARGET_SDL2
            vTaskDelete(tasks[i].handle);
        #endif
            tasks[i].handle = NULL;
            return true;
        }
    }
    return false;
}

void rg_task_delay(int ms)
{
    // Note: rg_task_delay MUST yield at least once, even if ms = 0
    // Keep in mind that delay may not be very accurate, use usleep().
#ifndef RG_TARGET_SDL2
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    SDL_PumpEvents();
    SDL_Delay(ms);
#endif
}

rg_app_t *rg_system_get_app(void)
{
    return &app;
}

rg_stats_t rg_system_get_counters(void)
{
    return statistics;
}

IRAM_ATTR void rg_system_tick(int busyTime)
{
    statistics.busyTime += busyTime;
    statistics.ticks++;
    // WDT_RELOAD(WDT_TIMEOUT);
}

IRAM_ATTR int64_t rg_system_timer(void)
{
#ifndef RG_TARGET_SDL2
    return esp_timer_get_time();
#else
    return SDL_GetTicks() * 1000;
#endif
}

void rg_system_event(int event, void *arg)
{
    RG_LOGD("Dispatching event:%d arg:%p\n", event, arg);
    if (app.handlers.event)
        app.handlers.event(event, arg);
}

char *rg_emu_get_path(rg_path_type_t pathType, const char *filename)
{
    char *buffer = malloc(RG_PATH_MAX + 1);
    int type = pathType & ~0xFF;
    int slot = pathType & 0xFF;

    if (!buffer)
        RG_PANIC("Out of memory!");

    if (type == RG_PATH_SAVE_STATE || type == RG_PATH_SAVE_SRAM)
        strcpy(buffer, RG_BASE_PATH_SAVES);
    else if (type == RG_PATH_SCREENSHOT)
        strcpy(buffer, RG_BASE_PATH_SAVES);
    else if (type == RG_PATH_ROM_FILE)
        strcpy(buffer, RG_BASE_PATH_ROMS);
    else if (type == RG_PATH_CACHE_FILE)
        strcpy(buffer, RG_BASE_PATH_CACHE);
    else
        strcpy(buffer, RG_STORAGE_ROOT);

    if (filename != NULL)
    {
        // Often filename will be an absolute ROM, let's remove that part!
        if (strstr(filename, RG_BASE_PATH_ROMS) == filename)
            filename += strlen(RG_BASE_PATH_ROMS) + 1;

        // TO DO: We probably should append app->name when needed...

        strcat(buffer, "/");
        strcat(buffer, filename);

        if (slot > 0)
            sprintf(buffer + strlen(buffer), "-%d", slot);

        if (type == RG_PATH_SAVE_STATE)
            strcat(buffer, ".sav");
        else if (type == RG_PATH_SAVE_SRAM)
            strcat(buffer, ".sram");
        else if (type == RG_PATH_SCREENSHOT)
            strcat(buffer, ".png");
    }

    // Don't shrink the buffer, we could use the extra space (append extension, etc).
    return buffer;
}

static void emu_update_save_slot(uint8_t slot)
{
    static uint8_t last_written = 0xFF;
    if (slot != last_written)
    {
        char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + 0xFF, app.romPath);
        FILE *fp = fopen(filename, "wb");
        if (fp)
        {
            fwrite(&slot, 1, 1, fp);
            fclose(fp);
            last_written = slot;
        }
        free(filename);
    }
    app.saveSlot = slot;

    // Set bootflags to resume from this state on next boot
    if ((app.bootFlags & RG_BOOT_ONCE) == 0)
    {
        app.bootFlags &= ~RG_BOOT_SLOT_MASK;
        app.bootFlags |= app.saveSlot << 4;
        app.bootFlags |= RG_BOOT_RESUME;
        rg_settings_set_number(NS_BOOT, SETTING_BOOT_FLAGS, app.bootFlags);
    }

    rg_storage_commit();
}

bool rg_emu_load_state(uint8_t slot)
{
    bool success = false;

    if (!app.romPath || !app.handlers.loadState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app.romPath);
    RG_LOGI("Loading state from '%s'.\n", filename);
    WDT_RELOAD(30 * 1000000);

    rg_gui_draw_hourglass();

    if (!(success = (*app.handlers.loadState)(filename)))
    {
        RG_LOGE("Load failed!\n");
    }
    else
    {
        emu_update_save_slot(slot);
    }

    WDT_RELOAD(WDT_TIMEOUT);
    free(filename);

    return success;
}

bool rg_emu_save_state(uint8_t slot)
{
    if (!app.romPath || !app.handlers.saveState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app.romPath);
    char tempname[RG_PATH_MAX + 8];
    bool success = false;

    RG_LOGI("Saving state to '%s'.\n", filename);
    WDT_RELOAD(30 * 1000000);

    rg_system_set_led(1);
    rg_gui_draw_hourglass();

    if (!rg_storage_mkdir(rg_dirname(filename)))
    {
        RG_LOGE("Unable to create dir, save might fail...\n");
    }

    #define tempname(ext) strcat(strcpy(tempname, filename), ext)

    if ((*app.handlers.saveState)(tempname(".new")))
    {
        rename(filename, tempname(".bak"));

        if (rename(tempname(".new"), filename) == 0)
        {
            unlink(tempname(".bak"));
            success = true;
        }
    }

    if (!success)
    {
        RG_LOGE("Save failed!\n");
        rename(filename, tempname(".bak"));
        unlink(tempname(".new"));
        rg_gui_alert("Save failed", NULL);
    }
    else
    {
        // Save succeeded, let's take a pretty screenshot for the launcher!
        char *filename = rg_emu_get_path(RG_PATH_SCREENSHOT + slot, app.romPath);
        rg_emu_screenshot(filename, rg_display_get_info()->screen.width / 2, 0);
        free(filename);

        emu_update_save_slot(slot);
    }

    #undef tempname
    free(filename);

    rg_storage_commit();
    rg_system_set_led(0);

    WDT_RELOAD(WDT_TIMEOUT);

    return success;
}

bool rg_emu_screenshot(const char *filename, int width, int height)
{
    if (!app.handlers.screenshot)
    {
        RG_LOGE("No handler defined...\n");
        return false;
    }

    RG_LOGI("Saving screenshot %dx%d to '%s'.\n", width, height, filename);

    if (!rg_storage_mkdir(rg_dirname(filename)))
    {
        RG_LOGE("Unable to create dir, save might fail...\n");
    }

    // FIXME: We should allocate a framebuffer to pass to the handler and ask it
    // to fill it, then we'd resize and save to png from here...
    bool success = (*app.handlers.screenshot)(filename, width, height);

    rg_storage_commit();

    return success;
}

rg_emu_state_t *rg_emu_get_states(const char *romPath, size_t slots)
{
    rg_emu_state_t *result = calloc(1, sizeof(rg_emu_state_t) + sizeof(rg_emu_slot_t) * slots);
    uint8_t last_used_slot = 0xFF;

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + 0xFF, romPath);
    FILE *fp = fopen(filename, "rb");
    if (fp)
    {
        fread(&last_used_slot, 1, 1, fp);
        fclose(fp);
    }
    free(filename);

    for (size_t i = 0; i < slots; i++)
    {
        rg_emu_slot_t *slot = &result->slots[i];
        char *preview = rg_emu_get_path(RG_PATH_SCREENSHOT + i, romPath);
        char *file = rg_emu_get_path(RG_PATH_SAVE_STATE + i, romPath);
        struct stat st;
        strcpy(slot->preview, preview);
        strcpy(slot->file, file);
        slot->id = i;
        slot->exists = stat(slot->file, &st) == 0;
        slot->mtime = st.st_mtime;
        if (slot->exists)
        {
            if (!result->latest || slot->mtime > result->latest->mtime)
                result->latest = slot;
            if (slot->id == last_used_slot)
                result->lastused = slot;
            result->used++;
        }
        free(preview);
        free(file);
    }
    if (!result->lastused && result->latest)
        result->lastused = result->latest;
    result->total = slots;

    return result;
}

bool rg_emu_reset(bool hard)
{
    if (app.handlers.reset)
        return app.handlers.reset(hard);
    return false;
}

static void shutdown_cleanup(void)
{
    rg_display_clear(C_BLACK);                // Let the user know that something is happening
    rg_gui_draw_hourglass();                  // ...
    rg_system_event(RG_EVENT_SHUTDOWN, NULL); // Allow apps to save their state if they want
    rg_audio_deinit();                        // Disable sound ASAP to avoid audio garbage
    rg_system_save_time();                    // RTC might save to storage, do it before
    rg_storage_deinit();                      // Unmount storage
    rg_input_wait_for_key(RG_KEY_ALL, false); // Wait for all keys to be released (boot is sensitive to GPIO0,32,33)
    rg_input_deinit();                        // Now we can shutdown input
    rg_i2c_deinit();                          // Must be after input, sound, and rtc
    rg_display_deinit();                      // Do this very last to reduce flicker time
}

void rg_system_shutdown(void)
{
    RG_LOGI("Halting system.\n");
    exitCalled = true;
    shutdown_cleanup();
    vTaskSuspendAll();
    while (1)
        ;
}

void rg_system_sleep(void)
{
    RG_LOGI("Going to sleep!\n");
    exitCalled = true;
    shutdown_cleanup();
    rg_task_delay(1000);
    esp_deep_sleep_start();
}

void rg_system_restart(void)
{
    RG_LOGI("Restarting system.\n");
    exitCalled = true;
    shutdown_cleanup();
    esp_restart();
}

void rg_system_switch_app(const char *partition, const char *name, const char *args, uint32_t flags)
{
    RG_LOGI("Switching to app %s (%s)!\n", partition, name ?: "-");
    exitCalled = true;

    if (app.initialized)
    {
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_NAME, name);
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_ARGS, args);
        rg_settings_set_number(NS_BOOT, SETTING_BOOT_FLAGS, flags);
        rg_settings_commit();
    }

    esp_err_t err = esp_ota_set_boot_partition(esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, partition));
    if (err != ESP_OK)
    {
        RG_LOGE("esp_ota_set_boot_partition returned 0x%02X!\n", err);
        RG_PANIC("Unable to set boot app!");
    }

    rg_system_restart();
}

bool rg_system_have_app(const char *app)
{
    return esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, app) != NULL;
}

void rg_system_panic(const char *context, const char *message)
{
    // Call begin_panic_trace first, it will normalize context and message for us
    begin_panic_trace(context, message);
    // Avoid using printf functions in case we're crashing because of a busted stack
    fputs("\n*** RG_PANIC() CALLED IN '", stdout);
    fputs(panicTrace.context, stdout);
    fputs("' ***\n*** ", stdout);
    fputs(panicTrace.message, stdout);
    fputs(" ***\n\n", stdout);
    abort();
}

void rg_system_vlog(int level, const char *context, const char *format, va_list va)
{
    const char *levels[RG_LOG_MAX] = {NULL, "=", "error", "warn", "info", "debug"};
    char buffer[300];
    size_t len = 0;

    if (app.logLevel && level > app.logLevel)
        return;

    if (level > RG_LOG_PRINT && level < RG_LOG_MAX)
    {
        if (levels[level])
            len += sprintf(buffer + len, "[%s] ", levels[level]);
        if (context)
            len += sprintf(buffer + len, "%s: ", context);
    }

    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, va);

    // Append a newline if needed only when possible
    if (len > 0 && buffer[len - 1] != '\n')
    {
        buffer[len++] = '\n';
        buffer[len] = 0;
    }

    logbuf_puts(&logbuf, buffer);
    fputs(buffer, stdout);

    #ifdef RG_TARGET_SDL2
    fflush(stdout);
    #endif
}

void rg_system_log(int level, const char *context, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    rg_system_vlog(level, context, format, va);
    va_end(va);
}

bool rg_system_save_trace(const char *filename, bool panic_trace)
{
    RG_ASSERT(filename, "bad param");

    rg_stats_t *stats = panic_trace ? &panicTrace.statistics : &statistics;
    logbuf_t *log = panic_trace ? &panicTrace.logbuf : &logbuf;

    RG_LOGI("Saving debug trace to '%s'...\n", filename);
    FILE *fp = fopen(filename, "w");
    if (fp)
    {
        fprintf(fp, "Application: %s (%s)\n", app.name, app.configNs);
        fprintf(fp, "Version: %s\n", app.version);
        fprintf(fp, "Build date: %s %s\n", app.buildDate, app.buildTime);
        fprintf(fp, "Toolchain: %s\n", app.toolchain);
        fprintf(fp, "Total memory: %d + %d\n", stats->totalMemoryInt, stats->totalMemoryExt);
        fprintf(fp, "Free memory: %d + %d\n", stats->freeMemoryInt, stats->freeMemoryExt);
        fprintf(fp, "Free block: %d + %d\n", stats->freeBlockInt, stats->freeBlockExt);
        fprintf(fp, "Stack HWM: %d\n", stats->freeStackMain);
        fprintf(fp, "Uptime: %ds (%d ticks)\n", (int)(rg_system_timer() / 1000000), stats->ticks);
        if (panic_trace && panicTrace.message[0])
            fprintf(fp, "Panic message: %.256s\n", panicTrace.message);
        if (panic_trace && panicTrace.context[0])
            fprintf(fp, "Panic context: %.256s\n", panicTrace.context);
        fputs("\nLog output:\n", fp);
        for (size_t i = 0; i < RG_LOGBUF_SIZE; i++)
        {
            size_t index = (log->cursor + i) % RG_LOGBUF_SIZE;
            if (log->buffer[index])
                fputc(log->buffer[index], fp);
        }
        fputs("\n\nEnd of trace\n\n", fp);
        fclose(fp);
    }

    return (fp != NULL);
}

void rg_system_set_led(int value)
{
#ifndef RG_TARGET_SDL2
    if (RG_GPIO_LED > -1 && ledValue != value)
        gpio_set_level(RG_GPIO_LED, value);
#endif
    ledValue = value;
}

int rg_system_get_led(void)
{
    return ledValue;
}

// Note: You should use calloc/malloc everywhere possible. This function is used to ensure
// that some memory is put in specific regions for performance or hardware reasons.
// Memory from this function should be freed with free()
void *rg_alloc(size_t size, uint32_t caps)
{
#ifndef RG_TARGET_SDL2
    char caps_list[36] = {0};
    uint32_t esp_caps = 0;
    void *ptr;

    esp_caps |= (caps & MEM_SLOW ? MALLOC_CAP_SPIRAM : (caps & MEM_FAST ? MALLOC_CAP_INTERNAL : 0));
    esp_caps |= (caps & MEM_DMA ? MALLOC_CAP_DMA : 0);
    esp_caps |= (caps & MEM_EXEC ? MALLOC_CAP_EXEC : 0);
    esp_caps |= (caps & MEM_32BIT ? MALLOC_CAP_32BIT : MALLOC_CAP_8BIT);

    if (esp_caps & MALLOC_CAP_SPIRAM)   strcat(caps_list, "SPIRAM|");
    if (esp_caps & MALLOC_CAP_INTERNAL) strcat(caps_list, "INTERNAL|");
    if (esp_caps & MALLOC_CAP_DMA)      strcat(caps_list, "DMA|");
    if (esp_caps & MALLOC_CAP_EXEC)     strcat(caps_list, "IRAM|");
    strcat(caps_list, (esp_caps & MALLOC_CAP_32BIT) ? "32BIT" : "8BIT");

    if ((ptr = heap_caps_calloc(1, size, esp_caps)))
    {
        RG_LOGI("SIZE=%u, CAPS=%s, PTR=%p\n", size, caps_list, ptr);
        return ptr;
    }

    size_t available = heap_caps_get_largest_free_block(esp_caps);
    // Loosen the caps and try again
    if ((ptr = heap_caps_calloc(1, size, esp_caps & ~(MALLOC_CAP_SPIRAM | MALLOC_CAP_INTERNAL))))
    {
        RG_LOGW("SIZE=%u, CAPS=%s, PTR=%p << CAPS not fully met! (available: %d)\n",
                    size, caps_list, ptr, available);
        return ptr;
    }

    RG_LOGE("SIZE=%u, CAPS=%s << FAILED! (available: %d)\n", size, caps_list, available);
    RG_PANIC("Memory allocation failed!");
#else
    return calloc(1, size);
#endif
}

#ifdef RG_ENABLE_PROFILING
// Note this profiler might be inaccurate because of:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=28205

// We also need to add multi-core support. Currently it's not an issue
// because all our profiled code runs on the same core.

#define LOCK_PROFILE()   xSemaphoreTake(lock, pdMS_TO_TICKS(10000));
#define UNLOCK_PROFILE() xSemaphoreGive(lock);

NO_PROFILE static inline profile_frame_t *find_frame(void *this_fn, void *call_site)
{
    for (int i = 0; i < 512; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];
        if (frame->func_ptr == 0)
        {
            profile->total_frames = RG_MAX(profile->total_frames, i + 1);
            frame->func_ptr = this_fn;
        }
        if (frame->func_ptr == this_fn) // && frame->caller_ptr == call_site)
            return frame;
    }

    RG_PANIC("Profile memory exhausted!");
}

NO_PROFILE void rg_system_dump_profile(void)
{
    LOCK_PROFILE();

    printf("RGD:PROF:BEGIN %d %d\n", profile->total_frames, (int)(rg_system_timer() - profile->time_started));

    for (int i = 0; i < profile->total_frames; ++i)
    {
        profile_frame_t *frame = &profile->frames[i];

        printf(
            "RGD:PROF:DATA %p\t%p\t%u\t%u\n",
            frame->caller_ptr,
            frame->func_ptr,
            frame->num_calls,
            frame->run_time
        );
    }

    printf("RGD:PROF:END\n");

    memset(profile->frames, 0, sizeof(profile->frames));
    profile->time_started = rg_system_timer();
    profile->total_frames = 0;

    UNLOCK_PROFILE();
}

NO_PROFILE void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (!profile)
        return;

    int64_t now = rg_system_timer();
    LOCK_PROFILE();
    profile_frame_t *fn = find_frame(this_fn, call_site);
    // Recursion will need a real stack, this is absurd
    if (fn->enter_time != 0)
        fn->run_time += now - fn->enter_time;
    fn->enter_time = rg_system_timer(); // not now, because we delayed execution
    fn->num_calls++;
    UNLOCK_PROFILE();
}

NO_PROFILE void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (!profile)
        return;

    int64_t now = rg_system_timer();
    LOCK_PROFILE();
    profile_frame_t *fn = find_frame(this_fn, call_site);
    // Ignore if profiler was disabled when function entered
    if (fn->enter_time != 0)
        fn->run_time += now - fn->enter_time;
    fn->enter_time = 0;
    UNLOCK_PROFILE();
}
#endif
