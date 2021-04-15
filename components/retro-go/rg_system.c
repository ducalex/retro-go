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
#include <assert.h>
#include <string.h>

#include "rg_system.h"

// On the Odroid-GO the SPI bus is shared between the SD Card and the LCD
// That isn't the case on other devices, so for performance we disable the mutex
#if (RG_GPIO_LCD_MISO == RG_GPIO_SD_MISO || \
     RG_GPIO_LCD_MOSI == RG_GPIO_SD_MOSI || \
     RG_GPIO_LCD_CLK == RG_GPIO_SD_CLK)
#define USE_SPI_MUTEX 1
#else
#define USE_SPI_MUTEX 0
#endif

#define PANIC_TRACE_MAGIC 0x12345678

#ifdef ENABLE_PROFILING
#define INPUT_TIMEOUT -1
#else
#define INPUT_TIMEOUT 5000000
#endif

#define SETTING_ROM_FILE_PATH "RomFilePath"
#define SETTING_START_ACTION  "StartAction"
#define SETTING_STARTUP_APP   "StartupApp"

typedef struct
{
    uint32_t magicWord;
    char output[2048];
    size_t cursor;
} panic_console_t;

typedef struct
{
    uint32_t magicWord;
    char message[256];
    char context[128];
    char appname[128];
} panic_trace_t;

// These will survive a software reset
static RTC_NOINIT_ATTR panic_trace_t panicTrace;
static RTC_NOINIT_ATTR panic_console_t panicConsole;

static rg_app_desc_t app;
static runtime_stats_t statistics;
static runtime_counters_t counters;
static long inputTimeout = -1;
static bool initialized = false;

#if USE_SPI_MUTEX
static SemaphoreHandle_t spiMutex;
static spi_lock_res_t spiMutexOwner;
#endif


IRAM_ATTR void esp_panic_putchar_hook(char c)
{
    if (panicConsole.magicWord != PANIC_TRACE_MAGIC)
    {
        panicConsole.magicWord = PANIC_TRACE_MAGIC;
        panicConsole.cursor = 0;
    }

    if (panicConsole.cursor < sizeof(panicConsole.output) - 1)
    {
        panicConsole.output[panicConsole.cursor++] = c;
        panicConsole.output[panicConsole.cursor] = 0;
    }
}

static void system_monitor_task(void *arg)
{
    runtime_counters_t current = {0};
    multi_heap_info_t heap_info = {0};
    time_t lastTime = time(NULL);
    bool ledState = false;

    // Give the app a few seconds to start before monitoring
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1)
    {
        float tickTime = get_elapsed_time() - counters.resetTime;
        // long  ticks = counters.ticks - current.ticks;

        // Make a copy and reset counters immediately because processing could take 1-2ms
        current = counters;
        counters.totalFrames = counters.fullFrames = 0;
        counters.skippedFrames = counters.busyTime = 0;
        counters.resetTime = get_elapsed_time();

        statistics.battery = rg_input_read_battery();
        statistics.busyPercent = RG_MIN(current.busyTime / tickTime * 100.f, 100.f);
        statistics.skippedFPS = current.skippedFrames / (tickTime / 1000000.f);
        statistics.totalFPS = current.totalFrames / (tickTime / 1000000.f);
        statistics.freeStackMain = uxTaskGetStackHighWaterMark(app.mainTaskHandle);

        heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
        statistics.freeMemoryInt = heap_info.total_free_bytes;
        statistics.freeBlockInt = heap_info.largest_free_block;

        heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        statistics.freeMemoryExt = heap_info.total_free_bytes;
        statistics.freeBlockExt = heap_info.largest_free_block;

        if (statistics.battery.percentage < 2)
        {
            ledState = !ledState;
            rg_system_set_led(ledState);
        }
        else if (ledState)
        {
            ledState = false;
            rg_system_set_led(ledState);
        }

        RG_LOGX("STACK:%d, HEAP:%d+%d (%d+%d), BUSY:%.4f, FPS:%.4f (SKIP:%d, PART:%d, FULL:%d), BATTERY:%d\n",
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
            statistics.battery.millivolts);

        // if (statistics.freeStackMain < 1024)
        // {
        //     RG_PANIC("Running out of stack space!");
        // }

        // if (RG_MAX(statistics.freeBlockInt, statistics.freeBlockExt) < 8192)
        // {
        //     RG_PANIC("Running out of heap space!");
        // }

        if (rg_input_gamepad_last_read() > (unsigned long)inputTimeout)
        {
            RG_PANIC("Application unresponsive");
        }

        if (abs(time(NULL) - lastTime) > 60)
        {
            RG_LOGI("System time suddenly changed! Saving...\n");
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

IRAM_ATTR void rg_system_tick(bool skippedFrame, bool fullFrame, int busyTime)
{
    if (skippedFrame)
        counters.skippedFrames++;
    else if (fullFrame)
        counters.fullFrames++;

    counters.busyTime += busyTime;

    counters.totalFrames++;
    counters.ticks++;

    // Reduce the inputTimeout once the emulation is running
    if (counters.ticks == 1)
    {
        inputTimeout = INPUT_TIMEOUT;
    }
}

runtime_stats_t rg_system_get_stats()
{
    return statistics;
}

void rg_system_time_init()
{
    // Query an external RTC or NTP or load saved timestamp from disk
}

void rg_system_time_save()
{
    // Update external RTC or save timestamp to disk
}

rg_app_desc_t *rg_system_init(int sampleRate, const rg_emu_proc_t *handlers)
{
    const esp_app_desc_t *esp_app = esp_ota_get_app_description();

    RG_LOGX("\n========================================================\n");
    RG_LOGX("%s %s (%s %s)\n", esp_app->project_name, esp_app->version, esp_app->date, esp_app->time);
    RG_LOGX("========================================================\n\n");

    #if USE_SPI_MUTEX
    spiMutex = xSemaphoreCreateMutex();
    spiMutexOwner = -1;
    #endif

    // Seed C's pseudo random number generator
    srand(esp_random());

    memset(&app, 0, sizeof(app));
    app.name = esp_app->project_name;
    app.version = esp_app->version;
    app.buildDate = esp_app->date;
    app.buildTime = esp_app->time;
    app.refreshRate = 1;
    app.sampleRate = sampleRate;
    app.mainTaskHandle = xTaskGetCurrentTaskHandle();
    if (handlers)
        app.handlers = *handlers;

    // Blue LED
    gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LED, 0);

    // This must be before rg_display_init() and rg_settings_init()
    rg_vfs_init();
    bool sd_init = rg_vfs_mount(RG_SDCARD);
    rg_settings_init(app.name);
    rg_display_init();
    rg_gui_init();
    rg_gui_draw_hourglass();
    rg_audio_init(sampleRate);
    rg_input_init();
    rg_system_time_init();

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        char message[400] = "Application crashed";

        if (panicTrace.magicWord == PANIC_TRACE_MAGIC)
        {
            RG_LOGX(" *** PANIC TRACE: %s (%s) *** \n", panicTrace.message, panicTrace.context);
            strcpy(message, panicTrace.message);
        }

        if (panicConsole.magicWord == PANIC_TRACE_MAGIC)
        {
            RG_LOGI("Panic log found, saving to sdcard...\n");
            FILE *fp = fopen(RG_BASE_PATH "/crash.log", "w");
            if (fp)
            {
                fprintf(fp, "Application: %s %s\n", app.name, app.version);
                fprintf(fp, "Build date: %s %s\n", app.buildDate, app.buildTime);
                if (panicTrace.magicWord == PANIC_TRACE_MAGIC)
                {
                    fprintf(fp, "Message: %.256s\n", panicTrace.message);
                    fprintf(fp, "Context: %.256s\n", panicTrace.context);
                }
                fputs("\nConsole:\n", fp);
                fputs(panicConsole.output, fp);
                fputs("\n\nEnd of log\n", fp);
                fclose(fp);
                strcat(message, "\nLog saved to SD Card.");
            }
        }

        rg_display_clear(C_BLUE);
        // rg_gui_set_font_size(12);
        rg_gui_alert("System Panic!", message);
        rg_vfs_deinit();
        rg_audio_deinit();
        rg_system_switch_app(RG_APP_LAUNCHER);
    }
    else
    {
        panicConsole.magicWord = 0;
        panicTrace.magicWord = 0;
    }

    if (!sd_init)
    {
        rg_display_clear(C_SKY_BLUE);
        // rg_gui_set_font_size(12);
        rg_gui_alert("SD Card Error", "Mount failed."); // esp_err_to_name(ret)
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    if (strcmp(app.name, RG_APP_LAUNCHER) != 0)
    {
        app.startAction = rg_settings_get_int32(SETTING_START_ACTION, 0);
        app.romPath = rg_settings_get_string(SETTING_ROM_FILE_PATH, NULL);
        app.refreshRate = 60;

        // If any key is pressed we abort and go back to the launcher
        if (rg_input_key_is_pressed(GAMEPAD_KEY_ANY))
        {
            rg_system_switch_app(RG_APP_LAUNCHER);
        }

        // Only boot this app once, next time will return to launcher
        if (rg_system_get_startup_app() == 0)
        {
            // This might interfer with our panic capture above and, at the very least, make
            // it report wrong app/version...
            rg_system_set_boot_app(RG_APP_LAUNCHER);
        }

        if (!app.romPath || strlen(app.romPath) < 4)
        {
            rg_gui_alert("SD Card Error", "Invalid ROM Path.");
            rg_system_switch_app(RG_APP_LAUNCHER);
        }
    }

    #ifdef ENABLE_PROFILING
        RG_LOGI("Profiling has been enabled at compile time!\n");
        rg_profiler_init();
    #endif

    #ifdef ENABLE_NETPLAY
    rg_netplay_init(app.netplay_handler);
    #endif

    xTaskCreate(&system_monitor_task, "sysmon", 2048, NULL, 7, NULL);

    // This is to allow time for app starting
    inputTimeout = INPUT_TIMEOUT * 5;
    initialized = true;

    RG_LOGI("Retro-Go init done.\n\n");

    return &app;
}

rg_app_desc_t *rg_system_get_app()
{
    return &app;
}

char *rg_emu_get_path(rg_path_type_t type, const char *_romPath)
{
    const char *fileName = _romPath ?: app.romPath;
    char buffer[256];

    if (strstr(fileName, RG_BASE_PATH_ROMS))
    {
        fileName = strstr(fileName, RG_BASE_PATH_ROMS);
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

        case RG_PATH_TEMP_FILE:
            sprintf(buffer, "%s/%X%X.tmp", RG_BASE_PATH_TEMP, (uint32_t)get_elapsed_time(), rand());
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

    rg_gui_draw_hourglass();

    // Increased input timeout, this might take a while
    inputTimeout = INPUT_TIMEOUT * 5;

    char *pathName = rg_emu_get_path(RG_PATH_SAVE_STATE, app.romPath);
    bool success = (*app.handlers.loadState)(pathName);
    // bool success = rg_emu_notify(RG_MSG_LOAD_STATE, pathName);

    inputTimeout = INPUT_TIMEOUT;

    if (!success)
    {
        RG_LOGE("Load failed!\n");
    }

    free(pathName);

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

    rg_system_set_led(1);
    rg_gui_draw_hourglass();

    char *saveName = rg_emu_get_path(RG_PATH_SAVE_STATE, app.romPath);
    char path_buffer[PATH_MAX + 1];
    bool success = false;

    // Increased input timeout, this might take a while
    inputTimeout = INPUT_TIMEOUT * 5;

    sprintf(path_buffer, "%s.new", saveName);
    if ((*app.handlers.saveState)(path_buffer))
    {
        sprintf(path_buffer, "%s.bak", saveName);
        rename(saveName, path_buffer);

        sprintf(path_buffer, "%s.new", saveName);
        if (rename(path_buffer, saveName) == 0)
        {
            sprintf(path_buffer, "%s.bak", saveName);
            unlink(path_buffer);

            success = true;

            rg_settings_set_int32(SETTING_START_ACTION, RG_START_ACTION_RESUME);
            rg_settings_save();
        }
    }

    if (!success)
    {
        RG_LOGE("Save failed!\n");

        sprintf(path_buffer, "%s.bak", saveName);
        rename(saveName, path_buffer);
        sprintf(path_buffer, "%s.new", saveName);
        unlink(path_buffer);

        rg_gui_alert("Save failed", NULL);
    }
    else
    {
        // Save succeeded, let's take a pretty screenshot for the launcher!
        char *fileName = rg_emu_get_path(RG_PATH_SCREENSHOT, app.romPath);
        rg_emu_screenshot(fileName, 160, 0);
        free(fileName);
    }

    inputTimeout = INPUT_TIMEOUT;

    rg_system_set_led(0);

    free(saveName);

    return success;
}

bool rg_emu_screenshot(const char *file, int width, int height)
{
    if (!app.handlers.screenshot)
    {
        RG_LOGE("No handler defined...\n");
        return false;
    }

    RG_LOGI("Saving screenshot %dx%d to '%s'.\n", width, height, file);

    rg_system_set_led(1);

    // FIXME: We should allocate a framebuffer to pass to the handler and ask it
    // to fill it, then we'd resize and save to png from here...
    bool success = (*app.handlers.screenshot)(file, width, height);

    rg_system_set_led(0);

    return success;
}

bool rg_emu_reset(int hard)
{
    if (app.handlers.reset)
        return app.handlers.reset(hard);
    return false;
    // return rg_emu_notify(RG_MSG_RESET, (void*)hard);
}

bool rg_emu_notify(int msg, void *arg)
{
    if (app.handlers.message)
        return app.handlers.message(msg, arg);
    return false;
}

void rg_emu_start_game(const char *emulator, const char *romPath, rg_start_action_t action)
{
    rg_settings_set_string(SETTING_ROM_FILE_PATH, romPath);
    rg_settings_set_int32(SETTING_START_ACTION, action);
    rg_settings_save();

    if (emulator)
        rg_system_switch_app(emulator);
    else
        esp_restart();
}

void rg_system_restart()
{
    // FIX ME: Ensure the boot loader points to us
    esp_restart();
}

void rg_system_switch_app(const char *app)
{
    RG_LOGI("Switching to app '%s'.\n", app ? app : "NULL");

    rg_display_clear(C_BLACK);
    rg_gui_draw_hourglass();

    rg_system_set_boot_app(app);

    rg_audio_deinit();
    rg_vfs_deinit();
    // rg_display_deinit();

    esp_restart();
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
    {
        // RG_PANIC("Application '%s' not found!", app);
        RG_PANIC("Application not found!");
    }

    if (esp_ota_set_boot_partition(partition) != ESP_OK)
    {
        RG_PANIC("Unable to set boot app!");
    }

    RG_LOGI("Boot partition set to %d '%s'\n", partition->subtype, partition->label);
}

void rg_system_panic(const char *message, const char *context)
{
    strcpy(panicTrace.message, message ? message : "");
    strcpy(panicTrace.context, context ? context : "");
    panicTrace.magicWord = PANIC_TRACE_MAGIC;

    RG_LOGX("*** PANIC  : %s\n", panicTrace.message);
    RG_LOGX("*** CONTEXT: %s\n", panicTrace.context);

    abort();
}

void rg_system_halt()
{
    RG_LOGI("Halting system!\n");
    vTaskSuspendAll();
    while (1);
}

void rg_system_sleep()
{
    RG_LOGI("Going to sleep!\n");

    // Wait for button release
    rg_input_wait_for_key(GAMEPAD_KEY_MENU, false);
    rg_audio_deinit();
    vTaskDelay(100);
    esp_deep_sleep_start();
}

void rg_system_set_led(int value)
{
    gpio_set_level(RG_GPIO_LED, value);
}

int rg_system_get_led(void)
{
    return gpio_get_level(RG_GPIO_LED);
}

int32_t rg_system_get_startup_app(void)
{
    return rg_settings_get_int32(SETTING_STARTUP_APP, 1);
}

void rg_system_set_startup_app(int32_t value)
{
    rg_settings_set_int32(SETTING_STARTUP_APP, value);
}

IRAM_ATTR void rg_spi_lock_acquire(spi_lock_res_t owner)
{
#if USE_SPI_MUTEX
    if (owner == spiMutexOwner)
    {
        return;
    }
    else if (xSemaphoreTake(spiMutex, pdMS_TO_TICKS(10000)) == pdPASS)
    {
        spiMutexOwner = owner;
    }
    else
    {
        RG_PANIC("SPI Mutex Lock Acquisition failed!");
    }
#endif
}

IRAM_ATTR void rg_spi_lock_release(spi_lock_res_t owner)
{
#if USE_SPI_MUTEX
    if (owner == spiMutexOwner || owner == SPI_LOCK_ANY)
    {
        xSemaphoreGive(spiMutex);
        spiMutexOwner = SPI_LOCK_ANY;
    }
#endif
}

/* Memory */

void *rg_alloc(size_t size, uint32_t mem_type)
{
    uint32_t caps = 0;

    if (mem_type & MEM_SLOW)  caps |= MALLOC_CAP_SPIRAM;
    if (mem_type & MEM_FAST)  caps |= MALLOC_CAP_INTERNAL;
    if (mem_type & MEM_DMA)   caps |= MALLOC_CAP_DMA;
    if (mem_type & MEM_32BIT) caps |= MALLOC_CAP_32BIT;
    else caps |= MALLOC_CAP_8BIT;

    void *ptr = heap_caps_calloc(1, size, caps);

    RG_LOGX("[RG_ALLOC] SIZE: %u  [SPIRAM: %u; 32BIT: %u; DMA: %u]  PTR: %p\n",
            size, (caps & MALLOC_CAP_SPIRAM) != 0, (caps & MALLOC_CAP_32BIT) != 0,
            (caps & MALLOC_CAP_DMA) != 0, ptr);

    if (!ptr)
    {
        size_t availaible = heap_caps_get_largest_free_block(caps);

        // Loosen the caps and try again
        ptr = heap_caps_calloc(1, size, caps & ~(MALLOC_CAP_SPIRAM|MALLOC_CAP_INTERNAL));
        if (!ptr)
        {
            RG_LOGX("[RG_ALLOC] ^-- Allocation failed! (available: %d)\n", availaible);
            RG_PANIC("Memory allocation failed!");
        }

        RG_LOGX("[RG_ALLOC] ^-- CAPS not fully met! (available: %d)\n", availaible);
    }

    return ptr;
}

void rg_free(void *ptr)
{
    return heap_caps_free(ptr);
}
