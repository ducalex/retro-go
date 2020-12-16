#include <freertos/FreeRTOS.h>
#include <esp_freertos_hooks.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <string.h>
#include <stdio.h>

#include "bitmaps/image_sdcard.h"
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

typedef struct
{
    uint magicWord;
    char message[128];
    char function[128];
    char file[128];
} panic_trace_t;

// This is a direct pointer to rtc slow ram which isn't cleared on
// panic. We don't use this region so we can point anywhere in it.
static panic_trace_t *panicTrace = (void *)0x50001000;

static rg_app_desc_t currentApp;
static runtime_stats_t statistics;
static runtime_counters_t counters;

#if USE_SPI_MUTEX
static SemaphoreHandle_t spiMutex;
static spi_lock_res_t spiMutexOwner;
#endif


static void system_monitor_task(void *arg)
{
    runtime_counters_t current;
    bool letState = false;
    float tickTime = 0;
    uint loops = 0;

    while (1)
    {
        // Make a copy and reset counters immediately because processing could take 1-2ms
        current = counters;
        counters.totalFrames = counters.fullFrames = 0;
        counters.skippedFrames = counters.busyTime = 0;
        counters.resetTime = get_elapsed_time();

        tickTime = (counters.resetTime - current.resetTime);

        if (current.busyTime > tickTime)
            current.busyTime = tickTime;

        statistics.battery = rg_input_read_battery();
        statistics.busyPercent = current.busyTime / tickTime * 100.f;
        statistics.skippedFPS = current.skippedFrames / (tickTime / 1000000.f);
        statistics.totalFPS = current.totalFrames / (tickTime / 1000000.f);
        // To do get the actual game refresh rate somehow
        statistics.emulatedSpeed = statistics.totalFPS / 60 * 100.f;

        statistics.freeMemoryInt = heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
        statistics.freeMemoryExt = heap_caps_get_free_size(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        statistics.freeBlockInt = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
        statistics.freeBlockExt = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);

    #if (configGENERATE_RUN_TIME_STATS == 1)
        TaskStatus_t pxTaskStatusArray[16];
        uint ulTotalTime = 0;
        uint uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, 16, &ulTotalTime);
        ulTotalTime /= 100UL;

        for (x = 0; x < uxArraySize; x++)
        {
            if (pxTaskStatusArray[x].xHandle == xTaskGetIdleTaskHandleForCPU(0))
                statistics.idleTimeCPU0 = 100 - (pxTaskStatusArray[x].ulRunTimeCounter / ulTotalTime);
            if (pxTaskStatusArray[x].xHandle == xTaskGetIdleTaskHandleForCPU(1))
                statistics.idleTimeCPU1 = 100 - (pxTaskStatusArray[x].ulRunTimeCounter / ulTotalTime);
        }
    #endif

        // Applications should never stop polling input. If they do, they're probably unresponsive...
        if (statistics.lastTickTime > 0 && rg_input_gamepad_last_read() > 5000000)
        {
            RG_PANIC("Application is unresponsive!");
        }

        if (statistics.battery.percentage < 2)
        {
            letState = !letState;
            rg_system_set_led(letState);
        }
        else if (letState)
        {
            letState = false;
            rg_system_set_led(letState);
        }

        printf("HEAP:%d+%d (%d+%d), BUSY:%.4f, FPS:%.4f (SKIP:%d, PART:%d, FULL:%d), BATTERY:%d\n",
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

        #ifdef ENABLE_PROFILING
            if ((loops % 10) == 0)
            {
                rg_profiler_stop();
                rg_profiler_print();
                rg_profiler_start();
            }
        #endif

        vTaskDelay(pdMS_TO_TICKS(1000));
        loops++;
    }

    vTaskDelete(NULL);
}

static void rg_gpio_init()
{
    rtc_gpio_deinit(RG_GPIO_GAMEPAD_MENU);
    //rtc_gpio_deinit(GPIO_NUM_14);

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

    printf("\n========================================================\n");
    printf("%s %s (%s %s)\n", app->project_name, app->version, app->date, app->time);
    printf("========================================================\n\n");

    printf("%s: %d KB free\n", __func__, esp_get_free_heap_size() / 1024);

    #if USE_SPI_MUTEX
    spiMutex = xSemaphoreCreateMutex();
    spiMutexOwner = -1;
    #endif

    // Seed C's pseudo random number generator
    srand(esp_random());

    memset(&currentApp, 0, sizeof(currentApp));
    currentApp.id = appId;

    // sdcard init must be before rg_display_init()
    // and rg_settings_init() if JSON is used
    bool sd_init = (rg_sdcard_mount() == 0);

    rg_settings_init();
    rg_gpio_init();
    rg_gui_init();
    rg_input_init();
    rg_audio_init(sampleRate);
    rg_display_init();

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        if (panicTrace->magicWord == PANIC_TRACE_MAGIC)
            rg_system_panic_dialog(panicTrace->message);
        else
            rg_system_panic_dialog("Reason unknown");
    }

    if (esp_reset_reason() != ESP_RST_SW)
    {
        rg_display_clear(0);
        rg_display_show_hourglass();
    }

    if (!sd_init)
    {
        rg_display_clear(C_WHITE);
        rg_display_write((RG_SCREEN_WIDTH - image_sdcard.width) / 2,
            (RG_SCREEN_HEIGHT - image_sdcard.height) / 2,
            image_sdcard.width,
            image_sdcard.height,
            image_sdcard.width * 2,
            (uint16_t*)image_sdcard.pixel_data);
        rg_system_halt();
    }

    #ifdef ENABLE_PROFILING
        printf("%s: Profiling has been enabled at compile time!\n", __func__);
        rg_profiler_init();
    #endif

    xTaskCreate(&system_monitor_task, "sysmon", 2048, NULL, 7, NULL);

    panicTrace->magicWord = 0;

    printf("%s: System ready!\n\n", __func__);
}

void rg_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb)
{
    // If any key is pressed we go back to the menu (recover from ROM crash)
    if (rg_input_key_is_pressed(GAMEPAD_KEY_ANY))
    {
        rg_system_switch_app(RG_APP_LAUNCHER);
    }

    if (rg_settings_StartupApp_get() == 0)
    {
        // Only boot this emu once, next time will return to launcher
        rg_system_set_boot_app(RG_APP_LAUNCHER);
    }

    currentApp.startAction = rg_settings_StartAction_get();
    if (currentApp.startAction == EMU_START_ACTION_NEWGAME)
    {
        rg_settings_StartAction_set(EMU_START_ACTION_RESUME);
        rg_settings_commit();
    }

    currentApp.romPath = rg_settings_RomFilePath_get();
    if (!currentApp.romPath || strlen(currentApp.romPath) < 4)
    {
        RG_PANIC("Invalid ROM Path!");
    }

    if (netplay_cb != NULL)
    {
        rg_netplay_pre_init(netplay_cb);
    }

    currentApp.loadState = load;
    currentApp.saveState = save;

    printf("%s: Init done. romPath='%s'\n", __func__, currentApp.romPath);
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

        case EMU_PATH_TEMP_FILE:
            sprintf(buffer, "%s/%X%X.tmp", RG_BASE_PATH_TEMP, get_elapsed_time(), rand());
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
            RG_PANIC("Unknown Type");
    }

    return strdup(buffer);
}

bool rg_emu_load_state(int slot)
{
    if (!currentApp.romPath || !currentApp.loadState)
    {
        printf("%s: No game/emulator loaded...\n", __func__);
        return false;
    }

    printf("%s: Loading state %d.\n", __func__, slot);

    rg_display_show_hourglass();
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *pathName = rg_emu_get_path(EMU_PATH_SAVE_STATE, currentApp.romPath);
    bool success = (*currentApp.loadState)(pathName);

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    if (!success)
    {
        printf("%s: Load failed!\n", __func__);
    }

    free(pathName);

    return success;
}

bool rg_emu_save_state(int slot)
{
    if (!currentApp.romPath || !currentApp.saveState)
    {
        printf("%s: No game/emulator loaded...\n", __func__);
        return false;
    }

    printf("%s: Saving state %d.\n", __func__, slot);

    rg_system_set_led(1);
    rg_display_show_hourglass();
    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *saveName = rg_emu_get_path(EMU_PATH_SAVE_STATE, currentApp.romPath);
    char *backName = rg_emu_get_path(EMU_PATH_SAVE_BACK, currentApp.romPath);
    char *tempName = rg_emu_get_path(EMU_PATH_TEMP_FILE, currentApp.romPath);

    bool success = false;

    if ((*currentApp.saveState)(tempName))
    {
        rename(saveName, backName);

        if (rename(tempName, saveName) == 0)
        {
            unlink(backName);
            success = true;
        }
    }

    unlink(tempName);

    rg_spi_lock_release(SPI_LOCK_SDCARD);
    rg_system_set_led(0);

    if (!success)
    {
        printf("%s: Save failed!\n", __func__);
        rg_gui_alert("Save failed");
    }

    free(saveName);
    free(backName);
    free(tempName);

    return success;
}

void rg_system_restart()
{
    // FIX ME: Ensure the boot loader points to us
    esp_restart();
}

void rg_system_switch_app(const char *app)
{
    printf("%s: Switching to app '%s'.\n", __func__, app ? app : "NULL");

    rg_display_clear(0);
    rg_display_show_hourglass();

    rg_audio_terminate();
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

    printf("%s: Boot partition set to %d '%s'\n", __func__, partition->subtype, partition->label);
}

void rg_system_panic(const char *reason, const char *function, const char *file)
{
    printf("*** PANIC: %s\n  *** FUNCTION: %s\n  *** FILE: %s\n", reason, function, file);

    strcpy(panicTrace->message, reason);
    strcpy(panicTrace->file, file);
    strcpy(panicTrace->function, function);

    panicTrace->magicWord = PANIC_TRACE_MAGIC;

    abort();
}

void rg_system_panic_dialog(const char *reason)
{
    printf(" *** PREVIOUS PANIC: %s *** \n", reason);

    // Clear the trace to avoid a boot loop
    panicTrace->magicWord = 0;

    rg_audio_terminate();

    // In case we panicked from inside a dialog
    rg_spi_lock_release(SPI_LOCK_ANY);

    // Blue screen of death!
    rg_display_clear(C_BLUE);

    dialog_choice_t choices[] = {
        {0, reason, "", -1, NULL},
        {1, "OK", "", 1, NULL},
        RG_DIALOG_CHOICE_LAST
    };
    rg_gui_dialog("The application crashed!", choices, 1);

    rg_system_switch_app(RG_APP_LAUNCHER);
}

void rg_system_halt()
{
    printf("%s: Halting system!\n", __func__);
    vTaskSuspendAll();
    while (1);
}

void rg_system_sleep()
{
    printf("%s: Going to sleep!\n", __func__);

    // Wait for button release
    rg_input_wait_for_key(GAMEPAD_KEY_MENU, false);
    rg_audio_terminate();
    vTaskDelay(100);
    esp_deep_sleep_start();
}

void rg_system_set_led(int value)
{
    gpio_set_level(RG_GPIO_LED, value);
}

IRAM_ATTR void rg_system_tick(uint skippedFrame, uint fullFrame, uint busyTime)
{
    if (skippedFrame) counters.skippedFrames++;
    else if (fullFrame) counters.fullFrames++;
    counters.totalFrames++;
    counters.busyTime += busyTime;

    statistics.lastTickTime = get_elapsed_time();
}

runtime_stats_t rg_system_get_stats()
{
    return statistics;
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

/* Utilities */

void *rg_alloc(size_t size, uint32_t caps)
{
    void *ptr;

    if (!(caps & MALLOC_CAP_32BIT))
    {
    caps |= MALLOC_CAP_8BIT;
    }

    ptr = heap_caps_calloc(1, size, caps);

    printf("RG_ALLOC: SIZE: %u  [SPIRAM: %u; 32BIT: %u; DMA: %u]  PTR: %p\n",
        size, (caps & MALLOC_CAP_SPIRAM) != 0, (caps & MALLOC_CAP_32BIT) != 0,
        (caps & MALLOC_CAP_DMA) != 0, ptr);

    if (!ptr)
    {
    size_t availaible = heap_caps_get_largest_free_block(caps);

    // Loosen the caps and try again
    ptr = heap_caps_calloc(1, size, caps & ~(MALLOC_CAP_SPIRAM|MALLOC_CAP_INTERNAL));
    if (!ptr)
    {
        printf("RG_ALLOC: ^-- Allocation failed! (available: %d)\n", availaible);
        RG_PANIC("Memory allocation failed!");
    }

    printf("RG_ALLOC: ^-- CAPS not fully met! (available: %d)\n", availaible);
    }

    return ptr;
}

void rg_free(void *ptr)
{
    return heap_caps_free(ptr);
}
