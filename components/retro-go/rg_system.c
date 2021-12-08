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

#include "rg_system.h"

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

// These will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
static rg_stats_t statistics;
static rg_counters_t counters;
static rg_app_t app;
static int ledValue = 0;
static int wdtCounter = 0;
static bool initialized = false;

#define WDT_TIMEOUT 15000000
#define WDT_RELOAD(val) wdtCounter = (val)


static const char *htime(time_t ts)
{
    static char buffer[32];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T", gmtime(&ts));
    return buffer;
}

static inline void logbuf_print(rg_logbuf_t *buf, const char *str)
{
    while (*str)
    {
        buf->buffer[buf->cursor++] = *str++;
        buf->cursor %= LOG_BUFFER_SIZE;
    }
    buf->buffer[buf->cursor] = 0;
}

static inline void begin_panic_trace()
{
    panicTrace.magicWord = RG_STRUCT_MAGIC;
    panicTrace.message[0] = 0;
    panicTrace.context[0] = 0;
    panicTrace.statistics = statistics;
    panicTrace.log = app.log;
    logbuf_print(&panicTrace.log, "\n\n*** PANIC TRACE: ***\n\n");
}

IRAM_ATTR void esp_panic_putchar_hook(char c)
{
    if (panicTrace.magicWord != RG_STRUCT_MAGIC)
        begin_panic_trace();
    logbuf_print(&panicTrace.log, (char[2]){c, 0});
}

static void system_monitor_task(void *arg)
{
    multi_heap_info_t heap_info = {0};
    time_t lastLoop = get_elapsed_time();
    time_t lastTime = time(NULL);
    bool ledState = false;

    memset(&statistics, 0, sizeof(statistics));
    memset(&counters, 0, sizeof(counters));

    // Give the app a few seconds to start before monitoring
    vTaskDelay(pdMS_TO_TICKS(2000));
    WDT_RELOAD(WDT_TIMEOUT);

    while (1)
    {
        float loopTime = get_elapsed_time_since(lastLoop);
        rg_counters_t current = counters;

        counters = (rg_counters_t){0};
        lastLoop = get_elapsed_time();

        statistics.busyPercent = RG_MIN(current.busyTime / loopTime * 100.f, 100.f);
        statistics.skippedFPS = current.skippedFrames / (loopTime / 1000000.f);
        statistics.totalFPS = current.totalFrames / (loopTime / 1000000.f);
        statistics.freeStackMain = uxTaskGetStackHighWaterMark(app.mainTaskHandle);

        heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
        statistics.freeMemoryInt = heap_info.total_free_bytes;
        statistics.freeBlockInt = heap_info.largest_free_block;
        heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        statistics.freeMemoryExt = heap_info.total_free_bytes;
        statistics.freeBlockExt = heap_info.largest_free_block;

        rg_input_read_battery(&statistics.batteryPercent, &statistics.batteryVoltage);

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
            current.skippedFrames,
            current.totalFrames - current.fullFrames - current.skippedFrames,
            current.fullFrames,
            statistics.batteryPercent);

        // if (statistics.freeStackMain < 1024)
        // {
        //     RG_LOGW("Running out of stack space!");
        // }

        // if (RG_MAX(statistics.freeBlockInt, statistics.freeBlockExt) < 8192)
        // {
        //     RG_LOGW("Running out of heap space!");
        // }

        if ((wdtCounter -= loopTime) <= 0)
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

        if (abs(time(NULL) - lastTime) > 60)
        {
            RG_LOGI("System time suddenly changed!\n");
            RG_LOGI("    old time: %s\n", htime(lastTime));
            RG_LOGI("    new time: %s\n", htime(time(NULL)));
            rg_system_time_save();
        }
        lastTime = time(NULL);

        #ifdef ENABLE_PROFILING
            static long loops = 0;
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
    static uint32_t totalFrames = 0;
    static uint32_t fullFrames = 0;

    const rg_display_t *disp = rg_display_get_status();

    if (disp->counters.totalFrames == totalFrames)
        counters.skippedFrames++;
    else if (disp->counters.fullFrames > fullFrames)
        counters.fullFrames++;

    totalFrames = disp->counters.totalFrames;
    fullFrames = disp->counters.fullFrames;

    counters.totalFrames++;
    counters.busyTime += busyTime;

    // WDT_RELOAD(WDT_TIMEOUT);
}

rg_stats_t rg_system_get_stats()
{
    return statistics;
}

void rg_system_time_init()
{
    const char *source = "settings";
    time_t timestamp;
#if 0
    if (rg_i2c_read(0x68, 0x00, data, sizeof(data)))
    {
        // ...
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

void rg_system_time_save()
{
    time_t now = time(NULL);
#if 0
    // ...
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

rg_app_t *rg_system_init(int sampleRate, const rg_handlers_t *handlers, const rg_gui_option_t *options)
{
    const esp_app_desc_t *esp_app = esp_ota_get_app_description();

    RG_LOGX("\n========================================================\n");
    RG_LOGX("%s %s (%s %s)\n", esp_app->project_name, esp_app->version, esp_app->date, esp_app->time);
    RG_LOGX(" built for: %s. aud=%d disp=%d pad=%d sd=%d cfg=%d\n", RG_TARGET_NAME, 0, 0, 0, 0, 0);
    RG_LOGX("========================================================\n\n");

    // Seed C's pseudo random number generator
    srand(esp_random());

    memset(&app, 0, sizeof(app));
    app.realname = esp_app->project_name;
    app.name = app.realname;
    app.version = esp_app->version;
    app.buildDate = esp_app->date;
    app.buildTime = esp_app->time;
    app.buildUser = RG_BUILD_USER;
    app.refreshRate = 60;
    app.sampleRate = sampleRate;
    app.logLevel = RG_LOG_INFO;
    app.isLauncher = (strcmp(app.realname, RG_APP_LAUNCHER) == 0);
    app.mainTaskHandle = xTaskGetCurrentTaskHandle();
    app.options = options; // Todo: We should make a copy of it
    if (handlers)
        app.handlers = *handlers;

    // Status LED
    #ifdef RG_GPIO_LED
        gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
    #endif
    rg_system_set_led(0);

    // sdcard must be first because it fails if the SPI bus is already initialized
    rg_storage_init();
    rg_input_init();

    if (!app.isLauncher)
    {
        app.name = rg_settings_get_string(NS_GLOBAL, SETTING_BOOT_NAME, app.realname);
        app.romPath = rg_settings_get_string(NS_GLOBAL, SETTING_BOOT_ARGS, "");
        app.bootFlags = rg_settings_get_number(NS_GLOBAL, SETTING_BOOT_FLAGS, 0);
        if (app.bootFlags & RG_BOOT_ONCE)
            rg_system_set_boot_app(RG_APP_LAUNCHER);
    }

    rg_display_init();
    rg_gui_init();
    rg_gui_draw_hourglass();
    rg_audio_init(sampleRate);
    rg_system_time_init();

    // Force return to launcher (recovery)
    if (rg_input_key_is_pressed(RG_KEY_ANY))
    {
        if (app.isLauncher) rg_settings_reset();
        rg_input_wait_for_key(RG_KEY_ALL, 0);
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    // Show alert if we've just rebooted from a panic
    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        char message[400] = "Application crashed";

        if (panicTrace.magicWord == RG_STRUCT_MAGIC)
        {
            RG_LOGI("Panic log found, saving to sdcard...\n");
            if (panicTrace.message[0])
                strcpy(message, panicTrace.message);

            if (rg_system_save_trace(RG_ROOT_PATH "/crash.log", 1))
                strcat(message, "\nLog saved to SD Card.");
        }

        rg_display_clear(C_BLUE);
        rg_gui_alert("System Panic!", message);
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    // Show alert if storage isn't available
    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert("SD Card Error", "Mount failed."); // esp_err_to_name(ret)
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    panicTrace.magicWord = 0;

    #ifdef ENABLE_PROFILING
    RG_LOGI("Profiling has been enabled at compile time!\n");
    rg_profiler_init();
    #endif

    #ifdef ENABLE_NETPLAY
    rg_netplay_init(app.netplay_handler);
    #endif

    xTaskCreate(&system_monitor_task, "sysmon", 2560, NULL, 7, NULL);

    initialized = true;

    RG_LOGI("Retro-Go ready.\n\n");

    return &app;
}

rg_app_t *rg_system_get_app()
{
    return &app;
}

void rg_system_event(rg_event_t event, void *arg)
{
    RG_LOGD("Event %d(%p)\n", event, arg);
    if (app.handlers.event)
        app.handlers.event(event, arg);
}

char *rg_emu_get_path(rg_path_type_t type, const char *_romPath)
{
    const char *fileName = _romPath ?: app.romPath;
    char buffer[PATH_MAX + 1];

    if (strstr(fileName, RG_BASE_PATH_ROMS) == fileName)
    {
        fileName += strlen(RG_BASE_PATH_ROMS);
    }

    if (!fileName || strlen(fileName) < 4)
    {
        RG_PANIC("Invalid ROM path!");
    }

    switch (type)
    {
        case RG_PATH_SAVE_STATE:
        case RG_PATH_SAVE_STATE_1:
        case RG_PATH_SAVE_STATE_2:
        case RG_PATH_SAVE_STATE_3:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sav");
            break;

        case RG_PATH_SAVE_SRAM:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sram");
            break;

        case RG_PATH_SCREENSHOT:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".png");
            break;

        case RG_PATH_ROM_FILE:
            strcpy(buffer, RG_BASE_PATH_ROMS);
            strcat(buffer, fileName);
            break;

        default:
            RG_PANIC("Unknown path type");
    }

    return strdup(buffer);
}

bool rg_emu_load_state(int slot)
{
    if (!app.romPath || !app.handlers.loadState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    RG_LOGI("Loading state %d.\n", slot);

    WDT_RELOAD(30 * 1000000);

    rg_gui_draw_hourglass();

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE, app.romPath);
    bool success = (*app.handlers.loadState)(filename);

    if (!success)
    {
        RG_LOGE("Load failed!\n");
    }

    free(filename);

    WDT_RELOAD(WDT_TIMEOUT);

    return success;
}

bool rg_emu_save_state(int slot)
{
    if (!app.romPath || !app.handlers.saveState)
    {
        RG_LOGE("No rom or handler defined...\n");
        return false;
    }

    RG_LOGI("Saving state %d.\n", slot);

    WDT_RELOAD(30 * 1000000);

    rg_system_set_led(1);
    rg_gui_draw_hourglass();

    char *filename = rg_emu_get_path(RG_PATH_SAVE_STATE, app.romPath);
    char path_buffer[PATH_MAX + 1];
    bool success = false;

    if (!rg_mkdir(rg_dirname(filename)))
    {
        RG_LOGE("Unable to create dir, save might fail...\n");
    }

    sprintf(path_buffer, "%s.new", filename);
    if ((*app.handlers.saveState)(path_buffer))
    {
        sprintf(path_buffer, "%s.bak", filename);
        rename(filename, path_buffer);

        sprintf(path_buffer, "%s.new", filename);
        if (rename(path_buffer, filename) == 0)
        {
            sprintf(path_buffer, "%s.bak", filename);
            unlink(path_buffer);
            success = true;
        }
    }

    if (!success)
    {
        RG_LOGE("Save failed!\n");

        sprintf(path_buffer, "%s.bak", filename);
        rename(filename, path_buffer);
        sprintf(path_buffer, "%s.new", filename);
        unlink(path_buffer);

        rg_gui_alert("Save failed", NULL);
    }
    else
    {
        // Save succeeded, let's take a pretty screenshot for the launcher!
        char *fileName = rg_emu_get_path(RG_PATH_SCREENSHOT, app.romPath);
        rg_emu_screenshot(fileName, rg_display_get_status()->screen.width / 2, 0);
        free(fileName);

        // And set bootflags to resume from this state on next boot
        if ((app.bootFlags & (RG_BOOT_ONCE|RG_BOOT_RESUME)) == 0)
        {
            app.bootFlags |= RG_BOOT_RESUME;
            rg_settings_set_number(NS_GLOBAL, SETTING_BOOT_FLAGS, app.bootFlags);
        }
    }

    rg_storage_commit();
    rg_system_set_led(0);

    free(filename);

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

static void shutdown_cleanup()
{
    // Prepare the system for a power change (deep sleep, restart, shutdown)
    // Wait for all keys to be released, they could interfer with the restart process
    rg_input_wait_for_key(RG_KEY_ALL, false);
    rg_system_event(RG_EVENT_SHUTDOWN, NULL);
    rg_system_time_save();
    rg_audio_deinit();
    rg_input_deinit();
    rg_i2c_deinit();
    rg_storage_deinit();
    rg_display_deinit();
}

void rg_system_shutdown()
{
    RG_LOGI("System halted.\n");
    shutdown_cleanup();
    vTaskSuspendAll();
    while (1);
}

void rg_system_sleep()
{
    RG_LOGI("Going to sleep!\n");
    shutdown_cleanup();
    vTaskDelay(100);
    esp_deep_sleep_start();
}

void rg_system_restart()
{
    shutdown_cleanup();
    esp_restart();
}

void rg_system_switch_app(const char *app)
{
    RG_LOGI("Switching to app '%s'.\n", app ? app : "NULL");
    rg_display_clear(C_BLACK);
    rg_gui_draw_hourglass();
    rg_system_set_boot_app(app);
    rg_system_restart();
}

void rg_system_start_app(const char *app, const char *name, const char *args, int flags)
{
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_PART, app);
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_NAME, name);
    rg_settings_set_string(NS_GLOBAL, SETTING_BOOT_ARGS, args);
    rg_settings_set_number(NS_GLOBAL, SETTING_BOOT_FLAGS, flags);
    rg_system_switch_app(app);
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

void rg_system_panic(const char *message, const char *context)
{
    if (panicTrace.magicWord != RG_STRUCT_MAGIC)
        begin_panic_trace();

    strcpy(panicTrace.message, message ? message : "");
    strcpy(panicTrace.context, context ? context : "");

    RG_LOGX("*** PANIC  : %s\n", panicTrace.message);
    RG_LOGX("*** CONTEXT: %s\n", panicTrace.context);

    abort();
}

void rg_system_log(int level, const char *context, const char *format, ...)
{
    static const char *levels[RG_LOG_MAX] = {NULL, "error", "warn", "info", "debug"};
    char buffer[256];
    size_t len = 0;
    va_list args;

    if (app.logLevel && level > app.logLevel)
        return;

    if (level < 0 || level >= RG_LOG_MAX)
        len += sprintf(buffer, "[log:%d] %s: ", level, context);
    else if (levels[level])
        len += sprintf(buffer, "[%s] %s: ", levels[level], context);

    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    logbuf_print(&app.log, buffer);
    fwrite(buffer, len, 1, stdout);
}

bool rg_system_save_trace(const char *filename, bool panic_trace)
{
    rg_stats_t *stats = panic_trace ? &panicTrace.statistics : &statistics;
    rg_logbuf_t *log = panic_trace ? &panicTrace.log : &app.log;
    RG_ASSERT(filename, "bad param");

    FILE *fp = fopen(filename, "w");
    if (fp)
    {
        fprintf(fp, "Application: %s\n", app.realname);
        fprintf(fp, "Version: %s\n", app.version);
        fprintf(fp, "Build date: %s %s\n", app.buildDate, app.buildTime);
        fprintf(fp, "ESP-IDF: %s\n", esp_get_idf_version());
        fprintf(fp, "Free memory: %d + %d\n", stats->freeMemoryInt, stats->freeMemoryExt);
        fprintf(fp, "Free block: %d + %d\n", stats->freeBlockInt, stats->freeBlockExt);
        fprintf(fp, "Stack HWM: %d\n", stats->freeStackMain);
        fprintf(fp, "Uptime: %ds\n", (int)(get_elapsed_time() / 1000 / 1000));
        if (panic_trace && panicTrace.message[0])
            fprintf(fp, "Panic message: %.256s\n", panicTrace.message);
        if (panic_trace && panicTrace.context[0])
            fprintf(fp, "Panic context: %.256s\n", panicTrace.context);
        fputs("\nLog output:\n", fp);
        for (size_t i = 0; i < LOG_BUFFER_SIZE; i++)
        {
            size_t index = (log->cursor + i) % LOG_BUFFER_SIZE;
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

    RG_LOGE("SIZE=%u, CAPS=%s << FAILED! (available: %d)\n", size, caps_list, ptr, available);
    RG_PANIC("Memory allocation failed!");
}
