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
#include <freertos/semphr.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
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
    uint32_t magicWord;
} boot_config_t;

typedef struct
{
    int32_t totalFrames, fullFrames, partFrames, ticks;
    int64_t busyTime, updateTime;
} counters_t;

struct rg_task_s
{
    void (*func)(void *arg);
    void *arg;
    // bool blocked;
#ifdef ESP_PLATFORM
    QueueHandle_t queue;
    TaskHandle_t handle;
#else
    rg_task_msg_t msg;
    int msgWaiting;
    SDL_threadID handle;
#endif
    char name[16];
};

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
    rg_mutex_t *lock;
    profile_frame_t frames[512];
} *profile;
#endif

// The trace will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
// static RTC_NOINIT_ATTR boot_config_t bootConfig;
static RTC_NOINIT_ATTR time_t rtcValue;
static bool panicTraceCleared = false;
static bool exitCalled = false;
static uint32_t indicators;
static rg_color_t ledColor = -1;
static rg_stats_t statistics;
static rg_app_t app;
static rg_task_t tasks[8];

static const char *SETTING_BOOT_NAME = "BootName";
static const char *SETTING_BOOT_ARGS = "BootArgs";
static const char *SETTING_BOOT_SLOT = "BootSlot";
static const char *SETTING_BOOT_FLAGS = "BootFlags";
static const char *SETTING_TIMEZONE = "Timezone";
static const char *SETTING_INDICATOR_MASK = "Indicators";

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

static bool update_boot_config(const char *partition, const char *name, const char *args, int save_slot, uint32_t flags)
{
    if (app.initialized)
    {
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_NAME, name);
        rg_settings_set_string(NS_BOOT, SETTING_BOOT_ARGS, args);
        rg_settings_set_number(NS_BOOT, SETTING_BOOT_SLOT, save_slot);
        rg_settings_set_number(NS_BOOT, SETTING_BOOT_FLAGS, flags);
        rg_settings_commit();
    }
    else
    {
        rg_storage_delete(RG_BASE_PATH_CONFIG "/boot.json");
    }
#if defined(ESP_PLATFORM)
    // Check if the OTA settings are already correct, and if so do not call esp_ota_set_boot_partition
    // This is simply to avoid an unecessary flash write...
    const esp_partition_t *current = esp_ota_get_boot_partition();
    if (partition && (!current || strncmp(current->label, partition, 16) != 0))
    {
        esp_err_t err = esp_ota_set_boot_partition(esp_partition_find_first(
                ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, partition));
        if (err != ESP_OK)
        {
            RG_LOGE("esp_ota_set_boot_partition returned 0x%02X!", err);
            return false;
        }
    }
#endif
    return true;
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
#else
    statistics.freeMemoryInt = statistics.freeBlockInt = statistics.totalMemoryInt = (1 << 28);
    statistics.freeMemoryExt = statistics.freeBlockExt = statistics.totalMemoryExt = (1 << 28);
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

static void update_indicators(bool reset_animation)
{
    uint32_t visibleIndicators = indicators & app.indicatorsMask;
    static int animation_step = 0;
    rg_color_t newColor = 0; // C_GREEN

    if (reset_animation)
        animation_step = 0;
    else
        animation_step++;

    if (indicators & (3 << RG_INDICATOR_CRITICAL))
        newColor = C_RED; // Make it flash rapidly!
    else if (visibleIndicators & (1 << RG_INDICATOR_POWER_LOW))
        newColor = (animation_step & 1) ? C_NONE : C_RED;
    else if (visibleIndicators)
        newColor = C_BLUE;

    if (newColor != ledColor)
        rg_system_set_led_color(newColor);
}

static void system_monitor_task(void *arg)
{
    int64_t nextLoopTime = 0;
    time_t prevTime = time(NULL);

    rg_task_delay(2000);

    while (!exitCalled)
    {
        nextLoopTime = rg_system_timer() + 1000000;
        rtcValue = time(NULL);

        update_statistics();

        rg_battery_t battery = rg_input_read_battery();
        rg_system_set_indicator(RG_INDICATOR_POWER_LOW, (battery.present && battery.level <= 2.f));
        update_indicators(false);

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

        int seconds = (int)difftime(prevTime, rtcValue);
        if (abs(seconds) > 60)
        {
            RG_LOGI("System time suddenly changed %d seconds.", seconds);
            rg_system_save_time(); // Not sure if this is thread safe...
        }
        prevTime = rtcValue;

        if (nextLoopTime > rg_system_timer())
        {
            rg_task_delay((nextLoopTime - rg_system_timer()) / 1000 + 1);
        }
    }
}

static void enter_recovery_mode(void)
{
    RG_LOGW("Entering recovery mode...\n");

    // FIXME: At this point we don't have valid settings, we should find way to get the user's language...
    const rg_gui_option_t options[] = {
        {0, _("Reset all settings"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {1, _("Reboot to factory "), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        {2, _("Reboot to launcher"), NULL, RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END,
    };
    while (true)
    {
        switch (rg_gui_dialog(_("Recovery mode"), options, -1))
        {
        case 0:
            rg_storage_delete(RG_BASE_PATH_CONFIG);
            rg_storage_delete(RG_BASE_PATH_CACHE);
            break;
        case 1:
            rg_system_switch_app(RG_APP_FACTORY, NULL, NULL, 0, 0);
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
        gpio_set_level(RG_GPIO_LED, 0);
    #endif
#elif defined(RG_TARGET_SDL2)
    freopen("stdout.txt", "w", stdout);
    freopen("stderr.txt", "w", stderr);
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
        RG_PANIC("SDL Init failed!");
#endif

#if defined(RG_CUSTOM_PLATFORM_INIT)
    RG_LOGI("Running platform-specific init...\n");
    RG_CUSTOM_PLATFORM_INIT();
#endif
}

rg_app_t *rg_system_reinit(int sampleRate, const rg_handlers_t *handlers, void *_unused)
{
    if (!app.initialized)
        return rg_system_init(sampleRate, handlers, NULL);

    app.sampleRate = sampleRate;
    if (handlers)
        app.handlers = *handlers;
    rg_audio_set_sample_rate(app.sampleRate);

    return &app;
}

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, void *_unused)
{
    RG_ASSERT(app.initialized == false, "rg_system_init() was already called.");
    bool enterRecoveryMode = false;
    bool showCrashDialog = false;

    app = (rg_app_t){
        .name = RG_PROJECT_APP,
        .version = RG_PROJECT_VER,
        .buildDate = RG_BUILD_DATE,
        .buildInfo = RG_BUILD_INFO,
        .configNs = RG_PROJECT_APP,
        .bootArgs = NULL,
        .bootFlags = 0,
        .indicatorsMask = (1 << RG_INDICATOR_POWER_LOW),
        .speed = 1.f,
        .sampleRate = sampleRate,
        .tickRate = 60,
        .tickTimeout = 3000000,
        .frameTime = 1000000 / 60,
        .frameskip = 1, // This can be overriden on a per-app basis if needed, do not set 0 here!
        .overclock_level = 0,
        .overclock_mhz = 240,
        .lowMemoryMode = false,
        .enWatchdog = true,
        .isColdBoot = true,
        .isLauncher = false,
    #if RG_BUILD_RELEASE
        .isRelease = true,
        .logLevel = RG_LOG_INFO,
    #else
        .isRelease = false,
        .logLevel = RG_LOG_DEBUG,
    #endif
    };

    // Do this very early, may be needed to enable serial console
    platform_init();

#if defined(ESP_PLATFORM)
    esp_reset_reason_t r_reason = esp_reset_reason();
    showCrashDialog = (r_reason == ESP_RST_PANIC); // || r_reason == ESP_RST_TASK_WDT ||
                       // r_reason == ESP_RST_INT_WDT || r_reason == ESP_RST_WDT);
    app.isColdBoot = r_reason != ESP_RST_SW;
    tasks[0] = (rg_task_t){.handle = xTaskGetCurrentTaskHandle(), .name = "main"};
#elif defined(RG_TARGET_SDL2)
    tasks[0] = (rg_task_t){.handle = SDL_ThreadID(), .name = "main"};
#endif

    printf("\n========================================================\n");
    printf("%s %s (%s)\n", app.name, app.version, app.buildDate);
    printf(" built for: %s. type: %s\n", RG_TARGET_NAME, app.isRelease ? "release" : "dev");
    printf("========================================================\n\n");

#ifdef RG_I2C_GPIO_DRIVER
    rg_i2c_init();
    rg_i2c_gpio_init();
#endif

    rg_storage_init();
    rg_input_init();

    // Test for recovery request as early as possible
    for (int timeout = 5, btn; (btn = rg_input_read_gamepad() & RG_RECOVERY_BTN) && timeout >= 0; --timeout)
    {
        RG_LOGW("Button " PRINTF_BINARY_16 " being held down...\n", PRINTF_BINVAL_16(btn));
        enterRecoveryMode = (timeout == 0);
        rg_task_delay(100);
    }

    rg_settings_init(enterRecoveryMode || showCrashDialog);
    app.configNs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_NAME, app.configNs);
    app.bootArgs = rg_settings_get_string(NS_BOOT, SETTING_BOOT_ARGS, app.bootArgs);
    app.bootFlags = rg_settings_get_number(NS_BOOT, SETTING_BOOT_FLAGS, app.bootFlags);
    app.saveSlot = rg_settings_get_number(NS_BOOT, SETTING_BOOT_SLOT, app.saveSlot);
    rg_display_init();
    rg_gui_init();

    if (enterRecoveryMode)
    {
        enter_recovery_mode();
    }
    else if (showCrashDialog)
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
        rg_display_clear(C_BLUE);
        rg_gui_alert("System Panic!", message);
        rg_system_exit();
    }
    memset(&panicTrace, 0, sizeof(panicTrace));
    panicTraceCleared = true;

    update_memory_statistics();
    app.lowMemoryMode = statistics.totalMemoryExt == 0;

    app.indicatorsMask = rg_settings_get_number(NS_GLOBAL, SETTING_INDICATOR_MASK, app.indicatorsMask);
    app.romPath = app.bootArgs ?: ""; // For whatever reason some of our code isn't NULL-aware, sigh..

    rg_gui_draw_hourglass();
    rg_audio_init(sampleRate);

    rg_system_set_timezone(rg_settings_get_string(NS_GLOBAL, SETTING_TIMEZONE, "EST+5"));
    rg_system_load_time();

    // Do these last to not interfere with panic handling above
    if (handlers)
        app.handlers = *handlers;

#ifdef RG_ENABLE_PROFILING
    RG_LOGI("Profiling has been enabled at compile time!\n");
    profile = rg_alloc(sizeof(*profile), MEM_SLOW);
    profile->lock = rg_mutex_create();
#endif

    if (app.lowMemoryMode)
        rg_gui_alert("External memory not detected", "Boot will continue but it will surely crash...");

    if (app.bootFlags & RG_BOOT_ONCE)
        update_boot_config(RG_APP_LAUNCHER, NULL, NULL, 0, 0);

    rg_task_create("rg_sysmon", &system_monitor_task, NULL, 3 * 1024, RG_TASK_PRIORITY_5, -1);
    app.initialized = true;

    update_memory_statistics();
    RG_LOGI("Available memory: %d/%d + %d/%d", statistics.freeMemoryInt / 1024, statistics.totalMemoryInt / 1024,
            statistics.freeMemoryExt / 1024, statistics.totalMemoryExt / 1024);
    RG_LOGI("Retro-Go ready.\n\n");

    return &app;
}

// FIXME: None of this is threadsafe. It works for now, but eventually it needs fixing...

#ifdef ESP_PLATFORM
static void task_wrapper(void *arg)
{
    rg_task_t *task = arg;
    task->handle = xTaskGetCurrentTaskHandle();
    task->queue = xQueueCreate(1, sizeof(rg_task_msg_t));
    (task->func)(task->arg);
    vQueueDelete(task->queue);
    memset(task, 0, sizeof(rg_task_t));
    vTaskDelete(NULL);
}
#else
static int task_wrapper(void *arg)
{
    rg_task_t *task = arg;
    task->handle = SDL_ThreadID();
    (task->func)(task->arg);
    memset(task, 0, sizeof(rg_task_t));
    return 0;
}
#endif

rg_task_t *rg_task_create(const char *name, void (*taskFunc)(void *arg), void *arg, size_t stackSize, int priority, int affinity)
{
    RG_ASSERT_ARG(name && taskFunc);
    rg_task_t *task = NULL;

    for (size_t i = 1; i < RG_COUNT(tasks); ++i)
    {
        if (tasks[i].func)
            continue;
        task = memset(&tasks[i], 0, sizeof(rg_task_t));
        break;
    }
    RG_ASSERT(task, "Out of task slots");

    task->func = taskFunc;
    task->arg = arg;
    task->handle = 0;
    strncpy(task->name, name, 15);

#if defined(ESP_PLATFORM)
    TaskHandle_t handle = NULL;
    if (affinity < 0)
        affinity = tskNO_AFFINITY;
    if (xTaskCreatePinnedToCore(task_wrapper, name, stackSize, task, priority, &handle, affinity) == pdPASS)
        return task;
#elif defined(RG_TARGET_SDL2)
    SDL_Thread *thread = SDL_CreateThread(task_wrapper, name, task);
    SDL_DetachThread(thread);
    if (thread)
        return task;
#endif

    RG_LOGE("Task creation failed: name='%s', fn='%p', stack=%d\n", name, taskFunc, (int)stackSize);
    memset(task, 0, sizeof(rg_task_t));

    return NULL;
}

rg_task_t *rg_task_find(const char *name)
{
    RG_ASSERT_ARG(name);
    for (size_t i = 0; i < RG_COUNT(tasks); ++i)
    {
        if (strncmp(tasks[i].name, name, 16) == 0)
            return &tasks[i];
    }
    return NULL;
}

rg_task_t *rg_task_current(void)
{
#if defined(ESP_PLATFORM)
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
#elif defined(RG_TARGET_SDL2)
    SDL_threadID handle = SDL_ThreadID();
#endif
    for (size_t i = 0; i < RG_COUNT(tasks); ++i)
    {
        if (tasks[i].handle == handle)
            return &tasks[i];
    }
    return NULL;
}

bool rg_task_send(rg_task_t *task, const rg_task_msg_t *msg)
{
    RG_ASSERT_ARG(task && msg);
#if defined(ESP_PLATFORM)
    return xQueueSend(task->queue, msg, portMAX_DELAY) == pdTRUE;
#elif defined(RG_TARGET_SDL2)
    while (task->msgWaiting > 0)
        continue;
    task->msg = *msg;
    task->msgWaiting = 1;
    return true;
#endif
}

bool rg_task_peek(rg_task_msg_t *out)
{
    rg_task_t *task = rg_task_current();
    bool success = false;
    if (!task || !out)
        return false;
    // task->blocked = true;
#if defined(ESP_PLATFORM)
    success = xQueuePeek(task->queue, out, portMAX_DELAY) == pdTRUE;
#elif defined(RG_TARGET_SDL2)
    while (task->msgWaiting < 1)
        continue;
    *out = task->msg;
#endif
    // task->blocked = false;
    return success;
}

bool rg_task_receive(rg_task_msg_t *out)
{
    rg_task_t *task = rg_task_current();
    bool success = false;
    if (!task || !out)
        return false;
    // task->blocked = true;
#if defined(ESP_PLATFORM)
    success = xQueueReceive(task->queue, out, portMAX_DELAY) == pdTRUE;
#elif defined(RG_TARGET_SDL2)
    while (task->msgWaiting < 1)
        continue;
    *out = task->msg;
    task->msgWaiting = 0;
#endif
    // task->blocked = false;
    return success;
}

size_t rg_task_messages_waiting(rg_task_t *task)
{
    if (!task) task = rg_task_current();
#if defined(ESP_PLATFORM)
    return uxQueueMessagesWaiting(task->queue);
#elif defined(RG_TARGET_SDL2)
    return task->msgWaiting;
#endif
}

// bool rg_task_is_blocked(rg_task_t *task)
// {
//     return task->blocked;
// }

void rg_task_delay(uint32_t ms)
{
#if defined(ESP_PLATFORM)
    vTaskDelay(pdMS_TO_TICKS(ms));
#elif defined(RG_TARGET_SDL2)
    SDL_PumpEvents();
    SDL_Delay(ms);
#endif
}

void rg_task_yield(void)
{
#if defined(ESP_PLATFORM)
    vPortYield();
#elif defined(RG_TARGET_SDL2)
    SDL_PumpEvents();
#endif
}

rg_mutex_t *rg_mutex_create(void)
{
#if defined(ESP_PLATFORM)
    return (rg_mutex_t *)xSemaphoreCreateMutex();
#elif defined(RG_TARGET_SDL2)
    return (rg_mutex_t *)SDL_CreateMutex();
#endif
}

void rg_mutex_free(rg_mutex_t *mutex)
{
    if (!mutex) return;
#if defined(ESP_PLATFORM)
    vSemaphoreDelete((QueueHandle_t)mutex);
#elif defined(RG_TARGET_SDL2)
    SDL_DestroyMutex((SDL_mutex *)mutex);
#endif
}

bool rg_mutex_give(rg_mutex_t *mutex)
{
    RG_ASSERT_ARG(mutex);
#if defined(ESP_PLATFORM)
    return xSemaphoreGive((QueueHandle_t)mutex) == pdPASS;
#elif defined(RG_TARGET_SDL2)
    return SDL_UnlockMutex((SDL_mutex *)mutex) == 0;
#endif
}

bool rg_mutex_take(rg_mutex_t *mutex, int timeoutMS)
{
    RG_ASSERT_ARG(mutex);
#if defined(ESP_PLATFORM)
    int timeout = timeoutMS >= 0 ? pdMS_TO_TICKS(timeoutMS) : portMAX_DELAY;
    return xSemaphoreTake((QueueHandle_t)mutex, timeout) == pdPASS;
#elif defined(RG_TARGET_SDL2)
    return SDL_LockMutex((SDL_mutex *)mutex) == 0;
#endif
}

void rg_system_load_time(void)
{
    time_t time_sec = RG_MAX(rtcValue, RG_BUILD_TIME);
#if 0
    if (rg_i2c_read(0x68, 0x00, data, sizeof(data)))
    {
        RG_LOGI("Time loaded from DS3231\n");
    }
    else
#endif
    void *data_ptr = (void *)&time_sec;
    size_t data_len = sizeof(time_sec);
    if (rg_storage_read_file(RG_BASE_PATH_CACHE "/clock.bin", &data_ptr, &data_len, RG_FILE_USER_BUFFER))
    {
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
    // We always save to storage in case the RTC disappears.
    if (rg_storage_write_file(RG_BASE_PATH_CACHE "/clock.bin", (void *)&time_sec, sizeof(time_sec), 0))
    {
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

void rg_system_set_tick_rate(int tickRate)
{
    app.tickRate = tickRate;
    app.frameTime = 1000000 / (app.tickRate * app.speed);
}

int rg_system_get_tick_rate(void)
{
    return app.tickRate;
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
#if defined(ESP_PLATFORM)
    return esp_timer_get_time();
#elif defined(RG_TARGET_SDL2)
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
    exitCalled = true;
    rg_display_clear(C_BLACK);                // Let the user know that something is happening
    rg_gui_draw_hourglass();                  // ...
    rg_system_event(RG_EVENT_SHUTDOWN, NULL); // Allow apps to save their state if they want
    rg_audio_deinit();                        // Disable sound ASAP to avoid audio garbage
    // rg_system_save_time();                    // RTC might save to storage, do it before
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
    rg_system_switch_app(RG_APP_LAUNCHER, NULL, NULL, 0, 0);
}

void rg_system_switch_app(const char *partition, const char *name, const char *args, int save_slot, uint32_t flags)
{
    RG_LOGI("Switching to app %s (%s)", partition ?: "-", name ?: "-");

    if (update_boot_config(partition, name, args, save_slot, flags))
        rg_system_restart();

    RG_PANIC("Failed to switch app!");
}

bool rg_system_have_app(const char *app)
{
#if defined(ESP_PLATFORM)
    return esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, app) != NULL;
#elif defined(RG_TARGET_SDL2)
    char exe[strlen(app) + 5];
    sprintf(exe, "%s.exe", app);
    return rg_storage_stat(app).is_file || rg_storage_stat(exe).is_file;
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
    fprintf(fp, "Build info: %s\n", app.buildInfo);
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

void rg_system_set_indicator(rg_indicator_t indicator, bool on)
{
    uint32_t old_indicators = indicators;
    indicators &= ~(1 << indicator);
    indicators |= (on << indicator);
    if (old_indicators != indicators)
        update_indicators(true);
}

bool rg_system_get_indicator(rg_indicator_t indicator)
{
    return indicators & (1 << indicator);
}

void rg_system_set_indicator_mask(rg_indicator_t indicator, bool on)
{
    app.indicatorsMask &= ~(1 << indicator);
    app.indicatorsMask |= (on << indicator);
    rg_settings_set_number(NS_GLOBAL, SETTING_INDICATOR_MASK, app.indicatorsMask);
}

bool rg_system_get_indicator_mask(rg_indicator_t indicator)
{
    return app.indicatorsMask & (1 << indicator);
}

bool rg_system_set_led_color(rg_color_t color)
{
    ledColor = color;
#if defined(RG_GPIO_LED)
    int value = color > 0; // GPIO LED doesn't support colors, so any color = on
    #if defined(RG_GPIO_LED_INVERT)
    value = !value;
    #endif
    if (RG_GPIO_LED != GPIO_NUM_NC)
        return gpio_set_level(RG_GPIO_LED, value) == ESP_OK;
#endif
    return true;
}

rg_color_t rg_system_get_led_color(void)
{
    return ledColor;
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

int rg_system_get_cpu_speed(void)
{
    return app.overclock_mhz;
}

void rg_system_set_overclock(int level)
{
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
    // None of this is documented by espressif but there are comments to be found in the file `rtc_clk.c` and `clk_tree_ll.h`
    extern void rom_i2c_writeReg(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t data);
    extern uint8_t rom_i2c_readReg(uint8_t block, uint8_t host_id, uint8_t reg_add);
    extern int uart_set_baudrate(int uart_num, uint32_t baud_rate);
    extern uint64_t esp_rtc_get_time_us(void);
    extern unsigned xthal_get_ccount(void);
    // #include "driver/uart.h"
#if CONFIG_IDF_TARGET_ESP32
    #define I2C_BBPLL                   0x66
    #define I2C_BBPLL_HOSTID               4
    #define I2C_BBPLL_OC_DIV_7_0           3    // This is the PLL divider to get the CPU clock (our main concern)
    #define OC_MAX_LEVEL                   6
    #define OC_MIN_LEVEL                  -5
    #define OC_DIV7_MULTIPLIER             5
#else // CONFIG_IDF_TARGET_ESP32S3
    #define I2C_BBPLL                   0x66
    #define I2C_BBPLL_HOSTID               1
    #define I2C_BBPLL_OC_DIV_7_0           3
    #define OC
    #define OC_MAX_LEVEL                   8
    #define OC_MIN_LEVEL                  -8
    #define OC_DIV7_MULTIPLIER             1
#endif
    if (level < OC_MIN_LEVEL || level > OC_MAX_LEVEL)
    {
        RG_LOGW("Invalid level %d, min:%d max:%d", level, OC_MIN_LEVEL, OC_MAX_LEVEL);
        return;
    }
    static int original_div7_0 = -1;
    if (original_div7_0 == -1)
        original_div7_0 = rom_i2c_readReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DIV_7_0);
    uint8_t div7_0 = original_div7_0 + (level * OC_DIV7_MULTIPLIER);
    rom_i2c_writeReg(I2C_BBPLL, I2C_BBPLL_HOSTID, I2C_BBPLL_OC_DIV_7_0, div7_0);
    rg_task_delay(20);

    // RTC clock isn't affected by the CPU or APB clocks, so it remains our only reliable time measurement
    uint64_t t = esp_rtc_get_time_us(); // The - 10000 is to account for time wasted on mutexes
    uint32_t cc = xthal_get_ccount(); // Obtain it *after* calling esp_rtc_get_time_us because it is slow
    rg_usleep(100000);
    int real_mhz = (double)(xthal_get_ccount() - cc) / (esp_rtc_get_time_us() - t);
    // float factor = 240.f / real_mhz;

#if CONFIG_IDF_TARGET_ESP32
    // Most audio devices rely on either the APB or the CPU clocks, which we've just skewed. So we have to
    // compensate. The external DAC uses the APLL which is an independant clock source, no need to correct.
    if (strcmp(rg_audio_get_sink()->name, "Ext DAC") != 0)
        rg_audio_set_sample_rate(app.sampleRate * (240.0 / real_mhz));
    uart_set_baudrate(0, 115200.0 * (240.0 / real_mhz));
    // esp_timer_impl_update_apb_freq(80.0 / 240.0 * real_mhz);
    // ets_update_cpu_frequency(real_mhz);
#endif

    // This is a lazy hack to report a more accurate emulation speed. Obviously this isn't a real solution.
    static int original_tickRate = 0;
    if (!original_tickRate)
        original_tickRate = app.tickRate;
    app.tickRate = original_tickRate * (240.f / real_mhz);
    app.overclock_level = level;
    app.overclock_mhz = real_mhz;
    app.frameskip = 1;

    RG_LOGW("Overclock level %d applied: %dMhz", level, real_mhz);
#else
    RG_LOGE("Overclock not supported on this platform!");
#endif
}

int rg_system_get_overclock(void)
{
    return app.overclock_level;
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
        if (rg_storage_write_file(filename, (void *)&slot, sizeof(slot), 0))
            last_written = slot;
        free(filename);
    }
    app.saveSlot = slot;

    // Set bootflags to resume from this state on next boot
    if ((app.bootFlags & RG_BOOT_ONCE) == 0)
        update_boot_config(NULL, app.configNs, app.bootArgs, app.saveSlot, app.bootFlags | RG_BOOT_RESUME);

    rg_storage_commit();
}

bool rg_emu_load_state(uint8_t slot)
{
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
    if (!app.romPath || !app.handlers.saveState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + slot, app.romPath);
    char tempname[RG_PATH_MAX + 8];
    bool success = false;

    RG_LOGI("Saving state to '%s'.\n", filename);

    rg_system_set_indicator(RG_INDICATOR_ACTIVITY_SYSTEM, 1);
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
        rg_emu_screenshot(filename, rg_display_get_width() / 2, 0);
        free(filename);
        emu_update_save_slot(slot);
    }

    #undef tempname
    free(filename);

    rg_storage_commit();
    rg_system_set_indicator(RG_INDICATOR_ACTIVITY_SYSTEM, 0);

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

uint8_t rg_emu_get_last_used_slot(const char *romPath)
{
    uint8_t last_used_slot = 0xFF;
    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE + 0xFF, romPath);
    void *data_ptr = (void *)&last_used_slot;
    size_t data_len = sizeof(last_used_slot);
    rg_storage_read_file(filename, &data_ptr, &data_len, RG_FILE_USER_BUFFER);
    free(filename);
    return last_used_slot;
}

rg_emu_states_t *rg_emu_get_states(const char *romPath, size_t slots)
{
    rg_emu_states_t *result = calloc(1, sizeof(rg_emu_states_t) + sizeof(rg_emu_slot_t) * slots);

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
            result->used++;
        }
        free(preview);
        free(file);
    }
    if (result->used)
    {
        uint8_t last_used_slot = rg_emu_get_last_used_slot(romPath);
        if (last_used_slot < slots)
            result->lastused = &result->slots[last_used_slot];
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
    if (app.speed != 1.f)
        rg_emu_set_speed(1.f);
    if (app.handlers.reset)
        return app.handlers.reset(hard);
    return false;
}

void rg_emu_set_speed(float speed)
{
    app.speed = RG_MIN(2.5f, RG_MAX(0.5f, speed));
    // FIXME: We need to store the actual default frameskip so we can return to it...
    app.frameskip = (app.speed - 0.5f) * 3;
    app.frameTime = 1000000.f / (app.tickRate * app.speed);
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

#define LOCK_PROFILE()   rg_mutex_take(profile->lock, 10000)
#define UNLOCK_PROFILE() rg_mutex_give(lock)

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
