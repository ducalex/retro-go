#include "rg_system.h"

#include <sys/time.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#else
#include <SDL2/SDL.h>
#endif

#define RG_STRUCT_MAGIC 0x12345678
#define RG_LOGBUF_SIZE 2048
typedef struct
{
    uint32_t magicWord;
    char message[256];
    char context[128];
    char configNs[16];
    char console[RG_LOGBUF_SIZE];
    size_t cursor;
    rg_stats_t statistics;
} panic_trace_t;

typedef struct
{
    int32_t totalFrames, fullFrames, partFrames, ticks;
    int64_t busyTime, updateTime;
} counters_t;

typedef struct
{
    void (*func)(void *arg);
    void *arg;
#ifdef ESP_PLATFORM
    TaskHandle_t handle;
#else
    SDL_threadID handle;
#endif
    char name[16];
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
    rg_queue_t *lock;
    profile_frame_t frames[512];
} *profile;
#endif

// The trace will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
static RTC_NOINIT_ATTR time_t rtcValue;
static bool panicTraceCleared = false;
static rg_stats_t statistics;
static rg_app_t app;
static rg_task_t tasks[8];

static const char *SETTING_BOOT_NAME = "BootName";
static const char *SETTING_BOOT_ARGS = "BootArgs";
static const char *SETTING_BOOT_FLAGS = "BootFlags";
static const char *SETTING_TIMEZONE = "Timezone";

#define logbuf_putc(buf, c) (buf)->console[(buf)->cursor++] = c, (buf)->cursor %= RG_LOGBUF_SIZE;
#define logbuf_puts(buf, str) for (const char *ptr = str; *ptr; ptr++) logbuf_putc(buf, *ptr);


static inline void begin_panic_trace(const char *context, const char *message)
{
    panicTrace.magicWord = RG_STRUCT_MAGIC;
    panicTrace.statistics = statistics;
    strncpy(panicTrace.configNs, app.configNs ?: "(none)", sizeof(panicTrace.configNs) - 1);
    strncpy(panicTrace.message, message ?: "(none)", sizeof(panicTrace.message) - 1);
    strncpy(panicTrace.context, context ?: "(none)", sizeof(panicTrace.context) - 1);
    panicTrace.configNs[sizeof(panicTrace.configNs) - 1] = 0;
    panicTrace.message[sizeof(panicTrace.message) - 1] = 0;
    panicTrace.context[sizeof(panicTrace.context) - 1] = 0;
    logbuf_puts(&panicTrace, "\n\n*** PANIC TRACE: ***\n\n");
}

IRAM_ATTR void esp_panic_putchar_hook(char c)
{
    if (panicTrace.magicWord != RG_STRUCT_MAGIC)
        begin_panic_trace("esp_panic", NULL);
    logbuf_putc(&panicTrace, c);
}

static void update_memory_statistics(void)
{
#ifdef ESP_PLATFORM
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
    counters.partFrames = display.partFrames;
    counters.busyTime = statistics.busyTime;
    counters.ticks = statistics.ticks;
    counters.updateTime = statistics.lastTick;

    // We prefer to use the tick time for more accurate FPS
    // but if we're not ticking, we need to use current time
    if (counters.ticks == previous.ticks)
        counters.updateTime = rg_system_timer();

    if (counters.ticks && previous.ticks)
    {
        float totalTime = counters.updateTime - previous.updateTime;
        float totalTimeSecs = totalTime / 1000000.f;
        float busyTime = counters.busyTime - previous.busyTime;
        float ticks = counters.ticks - previous.ticks;
        float fullFrames = counters.fullFrames - previous.fullFrames;
        float partFrames = counters.partFrames - previous.partFrames;
        float frames = counters.totalFrames - previous.totalFrames;

        // Hard to fix this sync issue without a lock, which I don't want to use...
        ticks = RG_MAX(ticks, frames);

        statistics.busyPercent = busyTime / totalTime * 100.f;
        statistics.totalFPS = ticks / totalTimeSecs;
        statistics.skippedFPS = (ticks - frames) / totalTimeSecs;
        statistics.fullFPS = fullFrames / totalTimeSecs;
        statistics.partialFPS = partFrames / totalTimeSecs;
    }
    statistics.uptime = rg_system_timer() / 1000000;

    update_memory_statistics();
}

static void system_monitor_task(void *arg)
{
    bool batteryLedState = false;
    int64_t nextLoopTime = 0;

    rg_task_delay(2000);

    while (!app.exitCalled)
    {
        nextLoopTime = rg_system_timer() + 1000000;
        rtcValue = time(NULL);

        update_statistics();

        rg_battery_t battery = rg_input_read_battery();
        if (battery.present)
        {
            if (battery.level < 2)
                rg_system_set_led((batteryLedState ^= 1));
            else if (batteryLedState)
                rg_system_set_led((batteryLedState = 0));
        }

        // Try to avoid complex conversions that could allocate, prefer rounding/ceiling if necessary.
        rg_system_log(RG_LOG_DEBUG, NULL, "STACK:%d, HEAP:%d+%d (%d+%d), BUSY:%d%%, FPS:%d (%d+%d+%d), BATT:%d\n",
            statistics.freeStackMain,
            statistics.freeMemoryInt / 1024,
            statistics.freeMemoryExt / 1024,
            statistics.freeBlockInt / 1024,
            statistics.freeBlockExt / 1024,
            (int)roundf(statistics.busyPercent),
            (int)roundf(statistics.totalFPS),
            (int)roundf(statistics.skippedFPS),
            (int)roundf(statistics.partialFPS),
            (int)roundf(statistics.fullFPS),
            (int)roundf((battery.volts * 1000) ?: battery.level));

        // Auto frameskip
        if (statistics.ticks > app.tickRate * 2)
        {
            float speed = ((float)statistics.totalFPS / app.tickRate) * 100.f / app.speed;
            // We don't fully go back to 0 frameskip because if we dip below 95% once, we're clearly
            // borderline in power and going back to 0 is just asking for stuttering...
            if (speed > 99.f && statistics.busyPercent < 85.f && app.frameskip > 1)
            {
                app.frameskip--;
                RG_LOGI("Reduced frameskip to %d", app.frameskip);
            }
            else if (speed < 96.f && statistics.busyPercent > 85.f && app.frameskip < 5)
            {
                app.frameskip++;
                RG_LOGI("Raised frameskip to %d", app.frameskip);
            }
        }

        if (statistics.lastTick < rg_system_timer() - app.tickTimeout)
        {
            // App hasn't ticked in a while, listen for MENU presses to give feedback to the user
            if (rg_input_wait_for_key(RG_KEY_MENU, true, 1000))
            {
                const char *message = "App unresponsive... Hold MENU to quit!";
                // Drawing at this point isn't safe. But the alternative is being frozen...
                rg_gui_draw_text(RG_GUI_CENTER, RG_GUI_CENTER, 0, message, C_RED, C_BLACK, RG_TEXT_BIGGER);
                if (!rg_input_wait_for_key(RG_KEY_MENU, false, 2000))
                    RG_PANIC("Application terminated!"); // We're not in a nice state, don't normal exit
            }
        }

        if (nextLoopTime > rg_system_timer())
        {
            rg_task_delay((nextLoopTime - rg_system_timer()) / 1000 + 1);
        }
    }
}

static void enter_recovery_mode(void)
{
    RG_LOGW("Entering recovery mode...\n");

    const rg_gui_option_t options[] = {
        {0, "Reset all settings", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {1, "Reboot to factory ", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, "Reboot to launcher", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END,
    };
    while (true)
    {
        switch (rg_gui_dialog("Recovery mode", options, -1))
        {
        case 0:
            rg_storage_delete(RG_BASE_PATH_CONFIG);
            rg_storage_delete(RG_BASE_PATH_CACHE);
            break;
        case 1:
            rg_system_switch_app(RG_APP_FACTORY, 0, 0, 0);
        case 2:
        default:
            rg_system_exit();
        }
    }
}

static void platform_init(void)
{
#if defined(ESP_PLATFORM)
    // At boot time those pins are muxed to JTAG and can interfere with other things.
    #if CONFIG_IDF_TARGET_ESP32
        gpio_reset_pin(GPIO_NUM_12);
        gpio_reset_pin(GPIO_NUM_13);
        gpio_reset_pin(GPIO_NUM_14);
        gpio_reset_pin(GPIO_NUM_15);
    #endif
    #ifdef RG_GPIO_LED
        gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
    #endif
#elif defined(RG_TARGET_SDL2)
    // freopen("stdout.txt", "w", stdout);
    // freopen("stderr.txt", "w", stderr);
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
        RG_PANIC("SDL Init failed!");
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
    rg_audio_set_sample_rate(app.sampleRate);

    return &app;
}

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options)
{
    RG_ASSERT(app.initialized == false, "rg_system_init() was already called.");

    app = (rg_app_t){
        .name = RG_PROJECT_APP,
        .version = RG_PROJECT_VERSION,
        .buildDate = RG_BUILD_DATE,
        .buildTool = RG_BUILD_TOOL,
        .configNs = RG_PROJECT_APP,
        .bootArgs = NULL,
        .bootFlags = 0,
        .bootType = RG_RST_POWERON,
        .speed = 1.f,
        .sampleRate = sampleRate,
        .tickRate = 60,
        .frameskip = 1,
        .overclock = 0,
        .tickTimeout = 3000000,
        .watchdog = true,
        .lowMemoryMode = false,
    #if RG_BUILD_TYPE == 1
        .isRelease = true,
        .logLevel = RG_LOG_INFO,
    #else
        .isRelease = false,
        .logLevel = RG_LOG_DEBUG,
    #endif
        .options = options, // TO DO: We should make a copy of it?
    };

    // Do this very early, may be needed to enable serial console
    platform_init();
    rg_system_set_led(0);

#ifdef ESP_PLATFORM
    const esp_app_desc_t *esp_app = esp_ota_get_app_description();
    snprintf(app.name, sizeof(app.name), "%s", esp_app->project_name);
    snprintf(app.version, sizeof(app.version), "%s", esp_app->version);
    snprintf(app.buildDate, sizeof(app.buildDate), "%s %s", esp_app->date, esp_app->time);
    snprintf(app.buildTool, sizeof(app.buildTool), "%s", esp_app->idf_ver);
    esp_reset_reason_t r_reason = esp_reset_reason();
    if (r_reason == ESP_RST_PANIC || r_reason == ESP_RST_TASK_WDT || r_reason == ESP_RST_INT_WDT)
        app.bootType = RG_RST_PANIC;
    else if (r_reason == ESP_RST_SW)
        app.bootType = RG_RST_RESTART;
    tasks[0] = (rg_task_t){NULL, NULL, xTaskGetCurrentTaskHandle(), "main"};
#else
    SDL_version version;
    SDL_GetVersion(&version);
    snprintf(app.buildTool, sizeof(app.buildTool), "SDL2 %d.%d.%d / CC %s", version.major,
             version.minor, version.patch, __VERSION__);
    tasks[0] = (rg_task_t){NULL, NULL, SDL_ThreadID(), "main"};
#endif

    printf("\n========================================================\n");
    printf("%s %s (%s)\n", app.name, app.version, app.buildDate);
    printf(" built for: %s. type: %s\n", RG_TARGET_NAME, app.isRelease ? "release" : "dev");
    printf("========================================================\n\n");

    rg_storage_init();
    rg_input_init();

    // Test for recovery request as early as possible
    for (int timeout = 5, btn; (btn = rg_input_read_gamepad() & RG_RECOVERY_BTN) && timeout >= 0; --timeout)
    {
        RG_LOGW("Button " PRINTF_BINARY_16 " being held down...\n", PRINTF_BINVAL_16(btn));
        rg_task_delay(100);
        if (timeout > 0)
            continue;
        rg_display_init();
        rg_gui_init();
        enter_recovery_mode();
    }

    // Show alert if we've just rebooted from a panic
    if (app.bootType == RG_RST_PANIC)
    {
        RG_LOGE("Recoverying from panic!\n");
        char message[400] = "Application crashed";
        if (panicTrace.magicWord == RG_STRUCT_MAGIC)
        {
            RG_LOGI("Panic log found, saving to sdcard...\n");
            if (panicTrace.message[0] && strcmp(panicTrace.message, "(none)") != 0)
                strcpy(message, panicTrace.message);
            if (rg_system_save_trace(RG_STORAGE_ROOT "/crash.log", 1))
                strcat(message, "\nLog saved to SD Card.");
        }
        rg_display_init();
        rg_gui_init();
        rg_display_clear(C_BLUE);
        rg_gui_alert("System Panic!", message);
        rg_system_exit();
    }
    memset(&panicTrace, 0, sizeof(panicTrace));
    panicTraceCleared = true;

    rg_settings_init();
    app.configNs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_NAME, app.name);
    app.bootArgs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_ARGS, "");
    app.bootFlags = rg_settings_get_number(NS_BOOT, SETTING_BOOT_FLAGS, 0);
    app.saveSlot = (app.bootFlags & RG_BOOT_SLOT_MASK) >> 4;
    app.romPath = app.bootArgs;
    app.isLauncher = strcmp(app.name, "launcher") == 0; // Might be overriden after init

    rg_display_init();
    rg_gui_init();
    rg_audio_init(sampleRate);

    rg_storage_set_activity_led(rg_storage_get_activity_led());
    rg_gui_draw_hourglass();

    rg_system_set_timezone(rg_settings_get_string(NS_GLOBAL, SETTING_TIMEZONE, "EST+5"));
    rg_system_load_time();

    // Do these last to not interfere with panic handling above
    if (handlers)
        app.handlers = *handlers;

#ifdef RG_ENABLE_PROFILING
    RG_LOGI("Profiling has been enabled at compile time!\n");
    profile = rg_alloc(sizeof(*profile), MEM_SLOW);
    profile->lock = rg_queue_create(1, 0);
#endif

#ifdef ESP_PLATFORM
    update_memory_statistics();
    RG_LOGI("Available memory: %d/%d + %d/%d", statistics.freeMemoryInt / 1024, statistics.totalMemoryInt / 1024,
            statistics.freeMemoryExt / 1024, statistics.totalMemoryExt / 1024);
    if ((app.lowMemoryMode = (statistics.totalMemoryExt == 0)))
        rg_gui_alert("External memory not detected", "Boot will continue but it will surely crash...");

    if (app.bootFlags & RG_BOOT_ONCE)
        esp_ota_set_boot_partition(esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, RG_APP_LAUNCHER));
#endif

    rg_task_create("rg_sysmon", &system_monitor_task, NULL, 3 * 1024, RG_TASK_PRIORITY_5, -1);
    app.initialized = true;

    RG_LOGI("Retro-Go ready.\n\n");

    return &app;
}

// FIXME: None of this is threadsafe. It works for now, but eventually it needs fixing...

#ifdef ESP_PLATFORM
static void task_wrapper(void *arg)
{
    rg_task_t *task = arg;
    task->handle = xTaskGetCurrentTaskHandle();
    (task->func)(task->arg);
    task->func = NULL;
    vTaskDelete(NULL);
}
#else
static int task_wrapper(void *arg)
{
    rg_task_t *task = arg;
    task->handle = SDL_ThreadID();
    (task->func)(task->arg);
    task->func = NULL;
    return 0;
}
#endif

bool rg_task_create(const char *name, void (*taskFunc)(void *data), void *data, size_t stackSize, int priority, int affinity)
{
    RG_ASSERT(name && taskFunc, "bad param");
    rg_task_t *task = NULL;

    for (size_t i = 1; i < RG_COUNT(tasks); ++i)
    {
        if (tasks[i].func)
            continue;
        task = &tasks[i];
        break;
    }
    RG_ASSERT(task, "Out of task slots");

    task->func = taskFunc;
    task->arg = data;
    task->handle = 0;
    strncpy(task->name, name, 16);

#ifdef ESP_PLATFORM
    TaskHandle_t handle = NULL;
    if (affinity < 0)
        affinity = tskNO_AFFINITY;
    if (xTaskCreatePinnedToCore(task_wrapper, name, stackSize, task, priority, &handle, affinity) == pdPASS)
        return true;
#else
    SDL_Thread *thread = SDL_CreateThread(task_wrapper, name, task);
    SDL_DetachThread(thread);
    if (thread)
        return true;
#endif

    RG_LOGE("Task creation failed: name='%s', fn='%p', stack=%d\n", name, taskFunc, (int)stackSize);
    task->func = NULL;

    return false;
}

void rg_task_delay(uint32_t ms)
{
#ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    SDL_PumpEvents();
    SDL_Delay(ms);
#endif
}

void rg_task_yield(void)
{
#ifdef ESP_PLATFORM
    vPortYield();
#else
    SDL_PumpEvents();
#endif
}

rg_queue_t *rg_queue_create(size_t length, size_t itemSize)
{
    return (rg_queue_t *)xQueueCreate(length, itemSize);
}

void rg_queue_free(rg_queue_t *queue)
{
    if (queue)
        vQueueDelete((QueueHandle_t)queue);
}

bool rg_queue_send(rg_queue_t *queue, const void *item, int timeoutMS)
{
    int ticks = timeoutMS < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMS);
    return xQueueSend((QueueHandle_t)queue, item, ticks) == pdTRUE;
}

bool rg_queue_receive(rg_queue_t *queue, void *out, int timeoutMS)
{
    int ticks = timeoutMS < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMS);
    return xQueueReceive((QueueHandle_t)queue, out, ticks) == pdTRUE;
}

bool rg_queue_receive_from_isr(rg_queue_t *queue, void *out, void *xTaskWokenByReceive)
{
    return xQueueReceiveFromISR((QueueHandle_t)queue, out, xTaskWokenByReceive) == pdTRUE;
}

bool rg_queue_peek(rg_queue_t *queue, void *out, int timeoutMS)
{
    int ticks = timeoutMS < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMS);
    return xQueuePeek((QueueHandle_t)queue, out, ticks) == pdTRUE;
}

bool rg_queue_is_empty(rg_queue_t *queue)
{
    return uxQueueMessagesWaiting((QueueHandle_t)queue) == 0;
}

bool rg_queue_is_full(rg_queue_t *queue)
{
    return uxQueueSpacesAvailable((QueueHandle_t)queue) == 0;
}

size_t rg_queue_messages_waiting(rg_queue_t *queue)
{
    return uxQueueMessagesWaiting((QueueHandle_t)queue);
}

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
#ifdef ESP_PLATFORM
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
#ifdef ESP_PLATFORM
    setenv("TZ", TZ, 1);
    tzset();
#endif
}

char *rg_system_get_timezone(void)
{
    return rg_settings_get_string(NS_GLOBAL, SETTING_TIMEZONE, NULL);
    // return strdup(getenv("TZ"));
}

rg_app_t *rg_system_get_app(void)
{
    return &app;
}

rg_stats_t rg_system_get_counters(void)
{
    return statistics;
}

void rg_system_tick(int busyTime)
{
    statistics.lastTick = rg_system_timer();
    statistics.busyTime += busyTime;
    statistics.ticks++;
    // WDT_RELOAD(WDT_TIMEOUT);
}

IRAM_ATTR int64_t rg_system_timer(void)
{
#ifdef ESP_PLATFORM
    return esp_timer_get_time();
#else
    return (SDL_GetPerformanceCounter() * 1000000.f) / SDL_GetPerformanceFrequency();
#endif
}

void rg_system_event(int event, void *arg)
{
    RG_LOGV("Dispatching event:%d arg:%p\n", event, arg);
    if (app.handlers.event)
        app.handlers.event(event, arg);
}

static void shutdown_cleanup(void)
{
    app.exitCalled = true;
    rg_display_clear(C_BLACK);                // Let the user know that something is happening
    rg_gui_draw_hourglass();                  // ...
    rg_system_event(RG_EVENT_SHUTDOWN, NULL); // Allow apps to save their state if they want
    rg_audio_deinit();                        // Disable sound ASAP to avoid audio garbage
    rg_system_save_time();                    // RTC might save to storage, do it before
    rg_storage_deinit();                      // Unmount storage
    rg_input_wait_for_key(RG_KEY_ALL, 0, -1); // Wait for all keys to be released (boot is sensitive to GPIO0,32,33)
    rg_input_deinit();                        // Now we can shutdown input
    rg_i2c_deinit();                          // Must be after input, sound, and rtc
    rg_display_deinit();                      // Do this very last to reduce flicker time
}

void rg_system_shutdown(void)
{
    RG_LOGW("Halting system!");
    shutdown_cleanup();
#ifdef ESP_PLATFORM
    vTaskSuspendAll();
    while (1)
        ;
#else
    exit(0);
#endif
}

void rg_system_sleep(void)
{
    RG_LOGW("Going to sleep!");
    shutdown_cleanup();
    rg_task_delay(1000);
#ifdef ESP_PLATFORM
    esp_deep_sleep_start();
#else
    exit(0);
#endif
}

void rg_system_restart(void)
{
    RG_LOGW("Restarting system!");
    shutdown_cleanup();
#ifdef ESP_PLATFORM
    esp_restart();
#else
    exit(1);
#endif
}

void rg_system_exit(void)
{
    RG_LOGW("Exiting application!");
    rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0);
}

void rg_system_switch_app(const char *partition, const char *name, const char *args, uint32_t flags)
{
    RG_LOGI("Switching to app %s (%s)", partition ?: "-", name ?: "-");

    if (app.initialized)
    {
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_NAME, name);
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_ARGS, args);
        rg_settings_set_number(NS_BOOT, SETTING_BOOT_FLAGS, flags);
        rg_settings_commit();
    }
#ifdef ESP_PLATFORM
    esp_err_t err = esp_ota_set_boot_partition(esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, partition));
    if (err != ESP_OK)
    {
        RG_LOGE("esp_ota_set_boot_partition returned 0x%02X!\n", err);
        RG_PANIC("Unable to set boot app!");
    }
#endif
    rg_system_restart();
}

bool rg_system_have_app(const char *app)
{
#ifdef ESP_PLATFORM
    return esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, app) != NULL;
#else
    return true;
#endif
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
    const char *levels[RG_LOG_MAX] = {"=", "error", "warn", "info", "debug", "trace"};
    const char *colors[RG_LOG_MAX] = {"", "\e[31m", "\e[33m", "", "\e[34m", "\e[36m"};
    char buffer[300];
    size_t len = 0;

    if (level >= 0 && level < RG_LOG_MAX)
    {
        if (context)
            len = snprintf(buffer, sizeof(buffer), "[%s] %s: ", levels[level], context);
        else
            len = snprintf(buffer, sizeof(buffer), "[%s] ", levels[level]);
    }

    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, va);

    // Append a newline if needed only when possible
    if (len > 0 && buffer[len - 1] != '\n')
    {
        buffer[len++] = '\n';
        buffer[len] = 0;
    }

    if (panicTraceCleared)
    {
        logbuf_puts(&panicTrace, buffer);
    }

    if (level <= app.logLevel)
    {
    #if RG_LOG_COLORS
        if (level >= 0 && level < RG_LOG_MAX)
        {
            fputs(colors[level], stdout);
            fputs(buffer, stdout);
            fputs("\e[0m", stdout);
        }
        else
    #endif
        {
            fputs(buffer, stdout);
        }
    #ifdef RG_TARGET_SDL2
        fflush(stdout);
    #endif
    }
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
    if (!filename)
        filename = RG_STORAGE_ROOT "/trace.txt";

    RG_LOGI("Saving debug trace to '%s'...\n", filename);
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        RG_LOGE("Open file '%s' failed, can't save trace!", filename);
        return false;
    }

    rg_stats_t *stats = panic_trace ? &panicTrace.statistics : &statistics;
    fprintf(fp, "Application: %s (%s)\n", app.name, app.configNs);
    fprintf(fp, "Version: %s\n", app.version);
    fprintf(fp, "Build date: %s\n", app.buildDate);
    fprintf(fp, "Toolchain: %s\n", app.buildTool);
    fprintf(fp, "Total memory: %d + %d\n", stats->totalMemoryInt, stats->totalMemoryExt);
    fprintf(fp, "Free memory: %d + %d\n", stats->freeMemoryInt, stats->freeMemoryExt);
    fprintf(fp, "Free block: %d + %d\n", stats->freeBlockInt, stats->freeBlockExt);
    fprintf(fp, "Stack HWM: %d\n", stats->freeStackMain);
    fprintf(fp, "Uptime: %ds (%d ticks)\n", stats->uptime, stats->ticks);
    if (panic_trace && panicTrace.configNs[0])
        fprintf(fp, "Panic configNs: %.16s\n", panicTrace.configNs);
    if (panic_trace && panicTrace.message[0])
        fprintf(fp, "Panic message: %.256s\n", panicTrace.message);
    if (panic_trace && panicTrace.context[0])
        fprintf(fp, "Panic context: %.256s\n", panicTrace.context);
    fputs("\nLog output:\n", fp);
    for (size_t i = 0; i < RG_LOGBUF_SIZE; i++)
    {
        size_t index = (panicTrace.cursor + i) % RG_LOGBUF_SIZE;
        if (panicTrace.console[index])
            fputc(panicTrace.console[index], fp);
    }
    fputs("\n\nEnd of trace\n\n", fp);
    fclose(fp);

    return true;
}

void rg_system_set_led(int value)
{
#if defined(ESP_PLATFORM) && defined(RG_GPIO_LED)
    if (app.ledValue != value)
        gpio_set_level(RG_GPIO_LED, value);
#endif
    app.ledValue = value;
}

int rg_system_get_led(void)
{
    return app.ledValue;
}

void rg_system_set_log_level(rg_log_level_t level)
{
    if (level >= 0 && level < RG_LOG_MAX)
        app.logLevel = level;
    else
        RG_LOGE("Invalid log level %d", level);
}

int rg_system_get_log_level(void)
{
    return app.logLevel;
}

void rg_system_set_overclock(int level)
{
#ifdef ESP_PLATFORM
    // None of this is documented by espressif but can be found in the file rtc_clk.c
    #define I2C_BBPLL                   0x66
    #define I2C_BBPLL_ENDIV5              11
    #define I2C_BBPLL_BBADC_DSMP           9
    #define I2C_BBPLL_HOSTID               4
    #define I2C_BBPLL_OC_LREF              2
    #define I2C_BBPLL_OC_DIV_7_0           3
    #define I2C_BBPLL_OC_DCUR              5
    #define BBPLL_ENDIV5_VAL_320M       0x43
    #define BBPLL_BBADC_DSMP_VAL_320M   0x84
    #define BBPLL_ENDIV5_VAL_480M       0xc3
    #define BBPLL_BBADC_DSMP_VAL_480M   0x74
    extern void rom_i2c_writeReg(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t data);
    extern uint8_t rom_i2c_readReg(uint8_t block, uint8_t host_id, uint8_t reg_add);

    uint8_t div_ref = 0;
    uint8_t div7_0 = (level + 4) * 8;
    uint8_t div10_8 = 0;
    uint8_t lref = 0;
    uint8_t dcur = 6;
    uint8_t bw = 3;
    uint8_t ENDIV5 = BBPLL_ENDIV5_VAL_480M;
    uint8_t BBADC_DSMP = BBPLL_BBADC_DSMP_VAL_480M;
    uint8_t BBADC_OC_LREF = (lref << 7) | (div10_8 << 4) | (div_ref);
    uint8_t BBADC_OC_DIV_7_0 = div7_0;
    uint8_t BBADC_OC_DCUR = (bw << 6) | dcur;

    static uint8_t BASE_ENDIV5, BASE_BBADC_DSMP, BASE_BBADC_OC_LREF, BASE_BBADC_OC_DIV_7_0, BASE_BBADC_OC_DCUR, BASE_SAVED;
    if (!BASE_SAVED)
    {
        BASE_ENDIV5 = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_ENDIV5);
        BASE_BBADC_DSMP = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_BBADC_DSMP);
        BASE_BBADC_OC_LREF = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_LREF);
        BASE_BBADC_OC_DIV_7_0 = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DIV_7_0);
        BASE_BBADC_OC_DCUR = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DCUR);
        BASE_SAVED = true;
    }

    if (level == 0)
    {
        ENDIV5 = BASE_ENDIV5;
        BBADC_DSMP = BASE_BBADC_DSMP;
        BBADC_OC_LREF = BASE_BBADC_OC_LREF;
        BBADC_OC_DIV_7_0 = BASE_BBADC_OC_DIV_7_0;
        BBADC_OC_DCUR = BASE_BBADC_OC_DCUR;
    }
    else if (level < -4 || level > 3)
    {
        RG_LOGW("Invalid level %d, min:-4 max:3", level);
        return;
    }

    RG_LOGW(" ");
    RG_LOGW("BASE: %d %d %d %d %d", BASE_ENDIV5, BASE_BBADC_DSMP, BASE_BBADC_OC_LREF, BASE_BBADC_OC_DIV_7_0, BASE_BBADC_OC_DCUR);
    RG_LOGW("NEW : %d %d %d %d %d", ENDIV5, BBADC_DSMP, BBADC_OC_LREF, BBADC_OC_DIV_7_0, BBADC_OC_DCUR);
    RG_LOGW(" ");

    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_ENDIV5, ENDIV5);
    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_BBADC_DSMP, BBADC_DSMP);
    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_LREF, BBADC_OC_LREF);
    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DIV_7_0, BBADC_OC_DIV_7_0);
    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DCUR, BBADC_OC_DCUR);

    RG_LOGW("Overclock applied!");
#if 0
    extern uint64_t esp_rtc_get_time_us(void);
    uint64_t start = esp_rtc_get_time_us();
    int64_t end = rg_system_timer() + 1000000;
    while (rg_system_timer() < end)
        continue;
    overclock_ratio = 1000000.f / (esp_rtc_get_time_us() - start);
#endif
    // overclock_ratio = (240 + (app.overclock * 40)) / 240.f;

    // rg_audio_set_sample_rate(app.sampleRate / overclock_ratio);
#endif

    app.overclock = level;
}

int rg_system_get_overclock(void)
{
    return app.overclock;
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
        rg_settings_commit();
    }

    rg_storage_commit();
}

bool rg_emu_load_state(uint8_t slot)
{
    if (slot == 0xFF)
        slot = app.saveSlot;

    if (!app.romPath || !app.handlers.loadState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app.romPath);
    bool success = false;

    RG_LOGI("Loading state from '%s'.\n", filename);

    rg_gui_draw_hourglass();

    if (!(success = (*app.handlers.loadState)(filename)))
    {
        RG_LOGE("Load failed!\n");
    }
    else
    {
        emu_update_save_slot(slot);
    }

    free(filename);

    return success;
}

bool rg_emu_save_state(uint8_t slot)
{
    if (slot == 0xFF)
        slot = app.saveSlot;

    if (!app.romPath || !app.handlers.saveState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app.romPath);
    char tempname[RG_PATH_MAX + 8];
    bool success = false;

    RG_LOGI("Saving state to '%s'.\n", filename);

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
            remove(tempname(".bak"));
            success = true;
        }
    }

    if (!success)
    {
        RG_LOGE("Save failed!\n");
        rename(filename, tempname(".bak"));
        remove(tempname(".new"));
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

rg_emu_states_t *rg_emu_get_states(const char *romPath, size_t slots)
{
    rg_emu_states_t *result = calloc(1, sizeof(rg_emu_states_t) + sizeof(rg_emu_slot_t) * slots);
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
        rg_stat_t info = rg_storage_stat(file);
        strcpy(slot->preview, preview);
        strcpy(slot->file, file);
        slot->id = i;
        slot->is_used = info.exists;
        slot->is_lastused = false;
        slot->mtime = info.mtime;
        if (slot->is_used)
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
    if (result->lastused)
        result->lastused->is_lastused = true;
    result->total = slots;

    return result;
}

bool rg_emu_reset(bool hard)
{
    app.frameskip = 0;
    app.speed = 1.f;
    if (app.handlers.reset)
        return app.handlers.reset(hard);
    return false;
}

void rg_emu_set_speed(float speed)
{
    app.speed = RG_MIN(2.5f, RG_MAX(0.5f, speed));
    app.frameskip = RG_MAX(app.frameskip, (app.speed > 1.0f) ? 2 : 0);
    rg_audio_set_sample_rate(app.sampleRate * app.speed);
    rg_system_event(RG_EVENT_SPEEDUP, NULL);
}

float rg_emu_get_speed(void)
{
    return app.speed;
}

#ifdef RG_ENABLE_PROFILING
// Note this profiler might be inaccurate because of:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=28205

// We also need to add multi-core support. Currently it's not an issue
// because all our profiled code runs on the same core.

#define LOCK_PROFILE()   rg_queue_receive(profile->lock, NULL, 10000)
#define UNLOCK_PROFILE() rg_queue_send(lock, NULL, 0)

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
