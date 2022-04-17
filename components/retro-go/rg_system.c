#include <freertos/FreeRTOS.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_task_wdt.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_printf.h"

#ifndef RG_BUILD_USER
#define RG_BUILD_USER "ducalex"
#endif

#define SETTING_BOOT_NAME   "BootName"
#define SETTING_BOOT_PART   "BootPart"
#define SETTING_BOOT_ARGS   "BootArgs"
#define SETTING_BOOT_FLAGS  "BootFlags"
#define SETTING_RTC_TIME    "RTCTime"

#define RG_STRUCT_MAGIC 0x12345678

typedef struct
{
    uint32_t magicWord;
    char message[256];
    char context[128];
    rg_stats_t statistics;
    rg_logbuf_t log;
} panic_trace_t;

typedef struct
{
    uint32_t totalFrames, fullFrames, ticks, busyTime;
    uint64_t updateTime;
} counters_t;

// The trace will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
static rg_stats_t statistics;
static rg_app_t app;
static rg_logbuf_t logbuf;
static int ledValue = 0;
static int wdtCounter = 0;
static bool exitCalled = false;
static bool initialized = false;

#define WDT_TIMEOUT 10000000
#define WDT_RELOAD(val) wdtCounter = (val)


static const char *htime(time_t ts)
{
    static char buffer[32];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T", gmtime(&ts));
    return buffer;
}

static void rtc_time_init(void)
{
    const char *source = "settings";
    time_t timestamp;
#if 0
    if (rg_i2c_read(0x68, 0x00, data, sizeof(data)))
    {
        source = "DS3231";
    }
    else
#endif
    if (!(timestamp = rg_settings_get_number(NS_GLOBAL, SETTING_RTC_TIME, 0)))
    {
        timestamp = 946702800; // 2000-01-01 00:00:00
        source = "hardcoded";
    }

    settimeofday(&(struct timeval){timestamp, 0}, NULL);

    RG_LOGI("Time is now: %s\n", htime(time(NULL)));
    RG_LOGI("Time loaded from %s\n", source);
}

static void rtc_time_save(void)
{
    time_t now = time(NULL);
#if 0
    if (rg_i2c_write(0x68, 0x00, data, sizeof(data)))
    {
        RG_LOGI("System time saved to DS3231.\n");
    }
    else
#endif
    {
        rg_settings_set_number(NS_GLOBAL, SETTING_RTC_TIME, now);
        RG_LOGI("System time saved to settings.\n");
    }
}

static void exit_handler(void)
{
    RG_LOGI("Exit handler called.\n");
    if (!exitCalled)
    {
        exitCalled = true;
        rg_system_set_boot_app(RG_APP_LAUNCHER);
        rg_system_restart();
    }
}

static inline void logbuf_print(rg_logbuf_t *buf, const char *str)
{
    while (*str)
    {
        buf->buffer[buf->cursor++] = *str++;
        buf->cursor %= RG_LOGBUF_SIZE;
    }
    buf->buffer[buf->cursor] = 0;
}

static inline void begin_panic_trace(const char *context, const char *message)
{
    panicTrace.magicWord = RG_STRUCT_MAGIC;
    panicTrace.statistics = statistics;
    panicTrace.log = logbuf;
    strncpy(panicTrace.message, message ?: "(none)", sizeof(panicTrace.message) - 1);
    strncpy(panicTrace.context, context ?: "(none)", sizeof(panicTrace.context) - 1);
    panicTrace.message[sizeof(panicTrace.message) - 1] = 0;
    panicTrace.context[sizeof(panicTrace.context) - 1] = 0;
    logbuf_print(&panicTrace.log, "\n\n*** PANIC TRACE: ***\n\n");
}

IRAM_ATTR void esp_panic_putchar_hook(char c)
{
    if (panicTrace.magicWord != RG_STRUCT_MAGIC)
        begin_panic_trace("esp_panic", NULL);
    logbuf_print(&panicTrace.log, (char[2]){c, 0});
}

static void update_statistics(void)
{
    static counters_t counters = {0};
    const counters_t previous = counters;
    const rg_display_t *disp = rg_display_get_status();

    counters.totalFrames = disp->counters.totalFrames;
    counters.fullFrames = disp->counters.fullFrames;
    counters.busyTime = statistics.busyTime;
    counters.ticks = statistics.ticks;
    counters.updateTime = rg_system_timer();

    float elapsedTime = (counters.updateTime - previous.updateTime) / 1000000.f;
    statistics.busyPercent = RG_MIN((counters.busyTime - previous.busyTime) / (elapsedTime * 1000000.f) * 100.f, 100.f);
    statistics.totalFPS = (counters.ticks - previous.ticks) / elapsedTime;
    statistics.skippedFPS = statistics.totalFPS - ((counters.totalFrames - previous.totalFrames) / elapsedTime);
    statistics.fullFPS = (counters.fullFrames - previous.fullFrames) / elapsedTime;

    multi_heap_info_t heap_info = {0};
    heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
    statistics.freeMemoryInt = heap_info.total_free_bytes;
    statistics.freeBlockInt = heap_info.largest_free_block;
    statistics.totalMemoryInt = heap_info.total_free_bytes + heap_info.total_allocated_bytes;
    heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    statistics.freeMemoryExt = heap_info.total_free_bytes;
    statistics.freeBlockExt = heap_info.largest_free_block;
    statistics.totalMemoryExt = heap_info.total_free_bytes + heap_info.total_allocated_bytes;

    statistics.freeStackMain = uxTaskGetStackHighWaterMark(app.mainTaskHandle);

    if (!rg_input_read_battery(&statistics.batteryPercent, &statistics.batteryVoltage))
    {
        statistics.batteryPercent = -1.f;
        statistics.batteryVoltage = -1.f;
    }
}

static void system_monitor_task(void *arg)
{
    int64_t lastLoop = rg_system_timer();
    bool ledState = false;

    // Give the app a few seconds to start before monitoring
    vTaskDelay(pdMS_TO_TICKS(2500));
    WDT_RELOAD(WDT_TIMEOUT);

    while (1)
    {
        int loopTime_us = lastLoop - rg_system_timer();
        lastLoop = rg_system_timer();

        update_statistics();

        if (statistics.batteryPercent < 2)
        {
            ledState = !ledState;
            rg_system_set_led(ledState);
        }
        else if (ledState)
        {
            ledState = false;
            rg_system_set_led(ledState);
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
            statistics.batteryPercent);

        if ((wdtCounter -= loopTime_us) <= 0)
        {
            if (rg_input_gamepad_last_read() > WDT_TIMEOUT)
            {
                RG_LOGW("Application unresponsive!\n");
            #ifndef ENABLE_PROFILING
                RG_PANIC("Application unresponsive!");
            #endif
            }
            WDT_RELOAD(WDT_TIMEOUT);
        }

        #ifdef ENABLE_PROFILING
            static int loops = 0;
            if (((loops++) % 10) == 0)
            {
                rg_profiler_stop();
                rg_profiler_print();
                rg_profiler_start();
            }
        #endif

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

IRAM_ATTR void rg_system_tick(int busyTime)
{
    statistics.busyTime += busyTime;
    statistics.ticks++;
    // WDT_RELOAD(WDT_TIMEOUT);
}

IRAM_ATTR int64_t rg_system_timer(void)
{
    return esp_timer_get_time();
}

rg_stats_t rg_system_get_stats(void)
{
    return statistics;
}

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options)
{
    const esp_app_desc_t *esp_app = esp_ota_get_app_description();
    // const esp_partition_t *esp_part = esp_ota_get_boot_partition();

    printf("\n========================================================\n");
    printf("%s %s (%s %s)\n", esp_app->project_name, esp_app->version, esp_app->date, esp_app->time);
    printf(" built for: %s. aud=%d disp=%d pad=%d sd=%d cfg=%d\n", RG_TARGET_NAME, 0, 0, 0, 0, 0);
    printf("========================================================\n\n");

    #ifdef RG_GPIO_LED
    gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
    #endif
    rg_system_set_led(0);

    // Storage must be initialized first (SPI bus, settings, assets, etc)
    rg_storage_init();

    app = (rg_app_t){
        .name = esp_app->project_name,
        .version = esp_app->version,
        .buildDate = esp_app->date,
        .buildTime = esp_app->time,
        .buildUser = RG_BUILD_USER,
        .configNs = esp_app->project_name,
        .bootFlags = 0,
        .speed = 1.f,
        .refreshRate = 60,
        .sampleRate = sampleRate,
        .logLevel = RG_LOG_INFO,
        .isLauncher = strcmp(esp_app->project_name, RG_APP_LAUNCHER) == 0,
        .romPath = NULL,
        .mainTaskHandle = xTaskGetCurrentTaskHandle(),
        .options = options, // TO DO: We should make a copy of it
    };
    if (handlers)
        app.handlers = *handlers;

    if (!app.isLauncher)
    {
        app.configNs = rg_settings_get_string(NS_GLOBAL, SETTING_BOOT_NAME, app.name);
        app.romPath = rg_settings_get_string(NS_GLOBAL, SETTING_BOOT_ARGS, "");
        app.bootFlags = rg_settings_get_number(NS_GLOBAL, SETTING_BOOT_FLAGS, 0);
        if (app.bootFlags & RG_BOOT_ONCE)
            rg_system_set_boot_app(RG_APP_LAUNCHER);
    }

    // Now we init everything else!
    rg_input_init(); // aw9523 goes first!
    rg_display_init();
    rg_gui_init();
    rg_gui_draw_hourglass();
    rg_audio_init(sampleRate);

    // At this point the timer will provide enough entropy
    srand((unsigned)rg_system_timer());

    // Init last but before panic check, it could be useful
    update_statistics();

    // Show alert if we've just rebooted from a panic
    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        char message[400] = "Application crashed";

        if (panicTrace.magicWord == RG_STRUCT_MAGIC)
        {
            RG_LOGI("Panic log found, saving to sdcard...\n");
            if (panicTrace.message[0] && strcmp(panicTrace.message, "(none)") != 0)
                strcpy(message, panicTrace.message);

            if (rg_system_save_trace(RG_ROOT_PATH "/crash.log", 1))
                strcat(message, "\nLog saved to SD Card.");
        }

        rg_display_clear(C_BLUE);
        rg_gui_alert("System Panic!", message);
        rg_system_set_boot_app(RG_APP_LAUNCHER);
        rg_system_restart();
    }

    // Force return to launcher on key held and clear settings if we're already in launcher
    if (rg_input_key_is_pressed(RG_KEY_UP|RG_KEY_DOWN|RG_KEY_LEFT|RG_KEY_RIGHT))
    {
        if (app.isLauncher)
            rg_settings_reset();
        rg_system_set_boot_app(RG_APP_LAUNCHER);
        rg_system_restart();
    }

    // Show alert if storage isn't available
    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert("SD Card Error", "Mount failed."); // esp_err_to_name(ret)
        rg_system_set_boot_app(RG_APP_LAUNCHER);
        rg_system_restart();
    }

    panicTrace.magicWord = 0;

    #ifdef ENABLE_PROFILING
    RG_LOGI("Profiling has been enabled at compile time!\n");
    rg_profiler_init();
    #endif

    #ifdef ENABLE_NETPLAY
    rg_netplay_init(app.netplay_handler);
    #endif

    // Do this last to not interfere with panic handling above
    atexit(&exit_handler);
    rtc_time_init();

    xTaskCreate(&system_monitor_task, "sysmon", 2560, NULL, 7, NULL);

    initialized = true;

    RG_LOGI("Retro-Go ready.\n\n");

    return &app;
}

rg_app_t *rg_system_get_app(void)
{
    return &app;
}

void rg_system_event(rg_event_t event, void *arg)
{
    RG_LOGD("Event %d(%p)\n", event, arg);
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
        strcpy(buffer, RG_ROOT_PATH);

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

bool rg_emu_load_state(int slot)
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

    WDT_RELOAD(WDT_TIMEOUT);
    free(filename);

    return success;
}

bool rg_emu_save_state(int slot)
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

    if (!rg_mkdir(rg_dirname(filename)))
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
        rg_emu_screenshot(strcat(filename, ".png"), rg_display_get_status()->screen.width / 2, 0);

        // And set bootflags to resume from this state on next boot
        if ((app.bootFlags & (RG_BOOT_ONCE|RG_BOOT_RESUME)) == 0)
        {
            app.bootFlags |= RG_BOOT_RESUME;
            rg_settings_set_number(NS_GLOBAL, SETTING_BOOT_FLAGS, app.bootFlags);
        }
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

    rg_system_set_led(1);

    if (!rg_mkdir(rg_dirname(filename)))
    {
        RG_LOGE("Unable to create dir, save might fail...\n");
    }

    // FIXME: We should allocate a framebuffer to pass to the handler and ask it
    // to fill it, then we'd resize and save to png from here...
    bool success = (*app.handlers.screenshot)(filename, width, height);

    rg_storage_commit();
    rg_system_set_led(0);

    return success;
}

bool rg_emu_reset(int hard)
{
    if (app.handlers.reset)
        return app.handlers.reset(hard);
    return false;
}

static void shutdown_cleanup(void)
{
    rg_display_clear(C_BLACK);                  // Let the user know that something is happening
    rg_gui_draw_hourglass();                    // ...
    rg_system_event(RG_EVENT_SHUTDOWN, NULL);   // Allow apps to save their state if they want
    rg_audio_deinit();                          // Disable sound ASAP to avoid audio garbage
    rtc_time_save();                            // RTC might save to storage, do it before
    rg_storage_deinit();                        // Unmount storage
    rg_input_wait_for_key(RG_KEY_ALL, false);   // Wait for all keys to be released
    rg_input_deinit();                          // Now we can shutdown input
    rg_i2c_deinit();                            // Must be after input, sound, and rtc
    rg_display_deinit();                        // Do this very last to reduce flicker time
}

void rg_system_shutdown(void)
{
    RG_LOGI("Halting system.\n");
    exitCalled = true;
    shutdown_cleanup();
    vTaskSuspendAll();
    while (1);
}

void rg_system_sleep(void)
{
    RG_LOGI("Going to sleep!\n");
    shutdown_cleanup();
    vTaskDelay(100);
    esp_deep_sleep_start();
}

void rg_system_restart(void)
{
    exitCalled = true;
    shutdown_cleanup();
    esp_restart();
}

void rg_system_start_app(const char *app, const char *name, const char *args, int flags)
{
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_PART, app);
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_NAME, name);
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_ARGS, args);
    rg_settings_set_number(NS_GLOBAL, SETTING_BOOT_FLAGS, flags);
    rg_system_set_boot_app(app);
    rg_system_restart();
}

bool rg_system_find_app(const char *app)
{
    return esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, app) != NULL;
}

void rg_system_set_boot_app(const char *app)
{
    const esp_partition_t* partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, app);

    if (partition == NULL)
        RG_PANIC("Unable to set boot app: App not found!");

    esp_err_t err = esp_ota_set_boot_partition(partition);
    if (err != ESP_OK)
    {
        RG_LOGE("esp_ota_set_boot_partition returned 0x%02X!\n", err);
        RG_PANIC("Unable to set boot app!");
    }

    RG_LOGI("Boot partition set to %d '%s'\n", partition->subtype, partition->label);
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

    logbuf_print(&logbuf, buffer);
    fputs(buffer, stdout);
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
    rg_stats_t *stats = panic_trace ? &panicTrace.statistics : &statistics;
    rg_logbuf_t *log = panic_trace ? &panicTrace.log : &logbuf;
    RG_ASSERT(filename, "bad param");

    FILE *fp = fopen(filename, "w");
    if (fp)
    {
        fprintf(fp, "Application: %s\n", app.name);
        fprintf(fp, "Version: %s\n", app.version);
        fprintf(fp, "Build date: %s %s\n", app.buildDate, app.buildTime);
        fprintf(fp, "ESP-IDF: %s\n", esp_get_idf_version());
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
    #ifdef RG_GPIO_LED
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
    char caps_list[100] = {0};
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
    if ((ptr = heap_caps_calloc(1, size, esp_caps & ~(MALLOC_CAP_SPIRAM|MALLOC_CAP_INTERNAL))))
    {
        RG_LOGW("SIZE=%u, CAPS=%s, PTR=%p << CAPS not fully met! (available: %d)\n",
                    size, caps_list, ptr, available);
        return ptr;
    }

    RG_LOGE("SIZE=%u, CAPS=%s << FAILED! (available: %d)\n", size, caps_list, available);
    RG_PANIC("Memory allocation failed!");
}
