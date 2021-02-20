#include <freertos/FreeRTOS.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_task_wdt.h>
#include <esp_vfs_fat.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

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

typedef struct
{
    uint32_t magicWord;
    char message[256];
    char function[64];
    char file[256];
} panic_trace_t;

// This is a direct pointer to rtc slow ram which isn't cleared on
// panic. We don't use this region so we can point anywhere in it.
static panic_trace_t *panicTrace = (void *)0x50001000;

static rg_app_desc_t currentApp;
static runtime_stats_t statistics;
static runtime_counters_t counters;
static long inputTimeout = -1;

#if USE_SPI_MUTEX
static SemaphoreHandle_t spiMutex;
static spi_lock_res_t spiMutexOwner;
#endif


static void system_monitor_task(void *arg)
{
    runtime_counters_t current = {0};
    multi_heap_info_t heap_info = {0};
    time_t lastTime = time(NULL);
    bool ledState = false;

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
        statistics.freeStackMain = uxTaskGetStackHighWaterMark(currentApp.mainTaskHandle);

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

bool rg_sdcard_mount()
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT; // SDMMC_FREQ_26M;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = RG_GPIO_SD_MISO;
    slot_config.gpio_mosi = RG_GPIO_SD_MOSI;
    slot_config.gpio_sck  = RG_GPIO_SD_CLK;
    slot_config.gpio_cs = RG_GPIO_SD_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .allocation_unit_size = 0,
        .format_if_mount_failed = 0,
        .max_files = 5,
    };

    esp_vfs_fat_sdmmc_unmount();

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(RG_BASE_PATH, &host, &slot_config, &mount_config, NULL);

    if (ret == ESP_OK)
    {
        RG_LOGI("SD Card mounted, freq=%d\n", 0);
        return true;
    }

    RG_LOGE("SD Card mounting failed (%d)\n", ret);
    return false;
}

bool rg_sdcard_unmount()
{
    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret == ESP_OK)
    {
        RG_LOGI("SD Card unmounted\n");
        return true;
    }
    RG_LOGE("SD Card unmounting failed (%d)\n", ret);
    return false;
}

void rg_system_time_init()
{
    // Query an external RTC or NTP or load saved timestamp from disk
}

void rg_system_time_save()
{
    // Update external RTC or save timestamp to disk
}

void rg_system_gpio_init()
{
    // Blue LED
    gpio_set_direction(RG_GPIO_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LED, 0);

    // Disable LCD CD to prevent garbage
    gpio_set_direction(RG_GPIO_LCD_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(RG_GPIO_LCD_CS, 1);

    // Disable speaker to prevent hiss/pops
    gpio_set_direction(RG_GPIO_DAC1, GPIO_MODE_INPUT);
    gpio_set_direction(RG_GPIO_DAC2, GPIO_MODE_INPUT);
    gpio_set_level(RG_GPIO_DAC1, 0);
    gpio_set_level(RG_GPIO_DAC2, 0);
}

void rg_system_init(int appId, int sampleRate)
{
    const esp_app_desc_t *app = esp_ota_get_app_description();

    RG_LOGX("\n========================================================\n");
    RG_LOGX("%s %s (%s %s)\n", app->project_name, app->version, app->date, app->time);
    RG_LOGX("========================================================\n\n");

    #if USE_SPI_MUTEX
    spiMutex = xSemaphoreCreateMutex();
    spiMutexOwner = -1;
    #endif

    // Seed C's pseudo random number generator
    srand(esp_random());

    memset(&currentApp, 0, sizeof(currentApp));
    currentApp.id = appId;
    currentApp.refreshRate = 1;
    currentApp.mainTaskHandle = xTaskGetCurrentTaskHandle();

    // sdcard init must be before rg_display_init()
    // and rg_settings_init() if JSON is used
    bool sd_init = rg_sdcard_mount();

    rg_system_gpio_init();
    rg_settings_init();
    rg_gui_init();
    rg_input_init();
    rg_audio_init(sampleRate);
    rg_display_init();
    rg_system_time_init();

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        if (panicTrace->magicWord == PANIC_TRACE_MAGIC)
            RG_LOGX(" *** PREVIOUS PANIC: %s *** \n", panicTrace->message);
        else  // Presumably abort()
            strcpy(panicTrace->message, "Application crashed");
        panicTrace->magicWord = 0;
        rg_audio_deinit();
        rg_display_clear(C_BLUE);
        rg_gui_set_font_size(12);
        rg_gui_alert("System Panic!", panicTrace->message);
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    if (esp_reset_reason() != ESP_RST_SW)
    {
        rg_display_clear(0);
        rg_gui_draw_hourglass();
    }

    if (!sd_init)
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_set_font_size(12);
        rg_gui_alert("SD Card Error", "Mount failed."); // esp_err_to_name(ret)
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    if (strcmp(app->project_name, RG_APP_LAUNCHER) != 0)
    {
        // If any key is pressed we abort and go back to the launcher
        if (rg_input_key_is_pressed(GAMEPAD_KEY_ANY))
        {
            rg_system_switch_app(RG_APP_LAUNCHER);
        }

        // Only boot this app once, next time will return to launcher
        if (rg_settings_StartupApp_get() == 0)
        {
            rg_system_set_boot_app(RG_APP_LAUNCHER);
        }
    }

    #ifdef ENABLE_PROFILING
        RG_LOGI("Profiling has been enabled at compile time!\n");
        rg_profiler_init();
    #endif

    xTaskCreate(&system_monitor_task, "sysmon", 2048, NULL, 7, NULL);

    panicTrace->magicWord = 0;

    RG_LOGI("Retro-Go init done.\n\n");
}

void rg_emu_init(const rg_emu_proc_t *handlers)
{
    currentApp.startAction = rg_settings_StartAction_get();
    currentApp.refreshRate = 60;

    currentApp.romPath = rg_settings_RomFilePath_get();
    if (!currentApp.romPath || strlen(currentApp.romPath) < 4)
    {
        RG_PANIC("Invalid ROM path!");
    }

    if (handlers)
    {
        memcpy(&currentApp.handlers, handlers, sizeof(rg_emu_proc_t));
    }

    #ifdef ENABLE_NETPLAY
    if (currentApp.handlers.netplay)
    {
        rg_netplay_init(currentApp.handlers.netplay);
    }
    #endif

    // This is to allow time for rom loading
    inputTimeout = INPUT_TIMEOUT * 5;

    RG_LOGI("Emu init done. romPath='%s'\n\n", currentApp.romPath);
}

rg_app_desc_t *rg_system_get_app()
{
    return &currentApp;
}

char* rg_emu_get_path(emu_path_type_t type, const char *_romPath)
{
    const char *fileName = _romPath ?: currentApp.romPath;
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
        case EMU_PATH_SAVE_STATE:
        case EMU_PATH_SAVE_STATE_1:
        case EMU_PATH_SAVE_STATE_2:
        case EMU_PATH_SAVE_STATE_3:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sav");
            break;

        case EMU_PATH_SAVE_BACK:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sav.bak");
            break;

        case EMU_PATH_SAVE_SRAM:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sram");
            break;

        case EMU_PATH_SCREENSHOT:
            strcpy(buffer, RG_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".png");
            break;

        case EMU_PATH_TEMP_FILE:
            sprintf(buffer, "%s/%X%X.tmp", RG_BASE_PATH_TEMP, (uint32_t)get_elapsed_time(), rand());
            break;

        case EMU_PATH_ROM_FILE:
            strcpy(buffer, RG_BASE_PATH_ROMS);
            strcat(buffer, fileName);
            break;

        case EMU_PATH_CRC_CACHE:
            strcpy(buffer, RG_BASE_PATH_CRC_CACHE);
            strcat(buffer, fileName);
            strcat(buffer, ".crc");
            break;

        default:
            RG_PANIC("Unknown path type");
    }

    return strdup(buffer);
}

bool rg_emu_load_state(int slot)
{
    if (!currentApp.romPath || !currentApp.handlers.loadState)
    {
        RG_LOGE("No game/emulator loaded...\n");
        return false;
    }

    RG_LOGI("Loading state %d.\n", slot);

    rg_gui_draw_hourglass();
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    // Increased input timeout, this might take a while
    inputTimeout = INPUT_TIMEOUT * 5;

    char *pathName = rg_emu_get_path(EMU_PATH_SAVE_STATE, currentApp.romPath);
    bool success = (*currentApp.handlers.loadState)(pathName);

    inputTimeout = INPUT_TIMEOUT;

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    if (!success)
    {
        RG_LOGE("Load failed!\n");
    }

    free(pathName);

    return success;
}

bool rg_emu_save_state(int slot)
{
    if (!currentApp.romPath || !currentApp.handlers.saveState)
    {
        RG_LOGE("No game/emulator loaded...\n");
        return false;
    }

    RG_LOGI("Saving state %d.\n", slot);

    rg_system_set_led(1);
    rg_gui_draw_hourglass();
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *saveName = rg_emu_get_path(EMU_PATH_SAVE_STATE, currentApp.romPath);
    char *backName = rg_emu_get_path(EMU_PATH_SAVE_BACK, currentApp.romPath);
    char *tempName = rg_emu_get_path(EMU_PATH_TEMP_FILE, currentApp.romPath);

    bool success = false;

    // Increased input timeout, this might take a while
    inputTimeout = INPUT_TIMEOUT * 5;

    if ((*currentApp.handlers.saveState)(tempName))
    {
        rename(saveName, backName);

        if (rename(tempName, saveName) == 0)
        {
            unlink(backName);
            success = true;

            rg_settings_StartAction_set(EMU_START_ACTION_RESUME);
            rg_settings_commit();
        }
    }

    unlink(tempName);

    inputTimeout = INPUT_TIMEOUT;

    rg_spi_lock_release(SPI_LOCK_SDCARD);
    rg_system_set_led(0);

    if (!success)
    {
        RG_LOGE("Save failed!\n");
        rg_gui_alert("Save failed", NULL);
    }

    free(saveName);
    free(backName);
    free(tempName);

    return success;
}

bool rg_emu_reset(int hard)
{
    if (!currentApp.handlers.reset)
    {
        RG_LOGE("Emulator has no reset handler...\n");
        return false;
    }

    return (*currentApp.handlers.reset)(hard);
}

bool rg_emu_notify(int msg, void *arg)
{
    return false;
}

void rg_system_restart()
{
    // FIX ME: Ensure the boot loader points to us
    esp_restart();
}

void rg_system_switch_app(const char *app)
{
    RG_LOGI("Switching to app '%s'.\n", app ? app : "NULL");

    rg_display_clear(0);
    rg_gui_draw_hourglass();

    rg_audio_deinit();
    rg_sdcard_unmount();

    rg_system_set_boot_app(app);
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

void rg_system_panic(const char *reason, const char *function, const char *file)
{
    RG_LOGX("*** PANIC: %s\n  *** FUNCTION: %s\n  *** FILE: %s\n", reason, function, file);

    strcpy(panicTrace->message, reason);
    strcpy(panicTrace->file, file);
    strcpy(panicTrace->function, function);

    panicTrace->magicWord = PANIC_TRACE_MAGIC;

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

/* File System */

FILE *rg_fopen(const char *filename, const char *mode)
{
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        // We release the lock because rg_fclose might not
        // be called if rg_fopen fails.
        rg_spi_lock_release(SPI_LOCK_SDCARD);
    }
    return fp;
}

int rg_fclose(FILE *fp)
{
    int ret = fclose(fp);
    rg_spi_lock_release(SPI_LOCK_SDCARD);
    return ret;
}

bool rg_mkdir(const char *dir)
{
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
        {
            return true;
        }

        char temp[255];
        strncpy(temp, dir, sizeof(temp) - 1);

        for (char *p = temp + strlen(RG_BASE_PATH) + 1; *p; p++) {
            if (*p == '/') {
                *p = 0;
                if (strlen(temp) > 0) {
                    RG_LOGI("Creating %s\n", temp);
                    mkdir(temp, 0777);
                }
                *p = '/';
                while (*(p+1) == '/') p++;
            }
        }

        ret = mkdir(temp, 0777);
    }

    if (ret == 0)
    {
        RG_LOGI("Folder created %s\n", dir);
    }

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    return (ret == 0);
}

bool rg_readdir(const char* path, char **out_files, size_t *out_count)
{
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    DIR* dir = opendir(path);
    if (!dir)
    {
        rg_spi_lock_release(SPI_LOCK_SDCARD);
        return false;
    }

    // TO DO: We should use a struct instead of a packed list of strings
    char *buffer = NULL;
    size_t bufsize = 0;
    size_t bufpos = 0;
    size_t count = 0;
    struct dirent* file;

    while ((file = readdir(dir)))
    {
        size_t len = strlen(file->d_name) + 1;

        if (len < 2) continue;

        if ((bufsize - bufpos) < len)
        {
            bufsize += 1024;
            buffer = realloc(buffer, bufsize);
        }

        memcpy(buffer + bufpos, &file->d_name, len);
        bufpos += len;
        count++;
    }

    buffer = realloc(buffer, bufpos + 1);
    buffer[bufpos] = 0;

    closedir(dir);

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    if (out_files) *out_files = buffer;
    if (out_count) *out_count = count;

    return true;
}

bool rg_unlink(const char *path)
{
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    bool ret = unlink(path) == 0 || rmdir(path) == 0;

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    return ret;
}

long rg_filesize(const char *path)
{
    long ret = -1;
    FILE* f;

    if ((f = rg_fopen(path, "rb")))
    {
        fseek(f, 0, SEEK_END);
        ret = ftell(f);
        rg_fclose(f);
    }

    return ret;
}

const char* rg_get_filename(const char *path)
{
    const char *name = strrchr(path, '/');
    return name ? name + 1 : NULL;
}

const char* rg_get_extension(const char *path)
{
    const char *ext = strrchr(path, '.');
    return ext ? ext + 1 : NULL;
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
