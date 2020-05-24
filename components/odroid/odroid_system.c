#include <freertos/FreeRTOS.h>
#include <esp_freertos_hooks.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <esp_event.h>
#include <driver/rtc_io.h>
#include <rom/crc.h>
#include <string.h>
#include <stdio.h>

#include "odroid_image_sdcard.h"
#include "odroid_system.h"

int8_t speedupEnabled = 0;

static uint applicationId = 0;
static uint gameId = 0;
static uint startAction = 0;
static char *romPath = NULL;
static state_handler_t loadState;
static state_handler_t saveState;

static SemaphoreHandle_t spiMutex;
static spi_lock_res_t spiMutexOwner;
static runtime_stats_t statistics;
static runtime_counters_t counters;

static void odroid_system_monitor_task(void *arg);


static void odroid_system_gpio_init()
{
    rtc_gpio_deinit(ODROID_GAMEPAD_IO_MENU);
    //rtc_gpio_deinit(GPIO_NUM_14);

    // Blue LED
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 0);

    // Disable LCD CD to prevent garbage
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_5, 1);

    // Disable speaker to prevent hiss/pops
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);
    gpio_set_level(GPIO_NUM_25, 0);
    gpio_set_level(GPIO_NUM_26, 0);
}

void odroid_system_init(int appId, int sampleRate)
{
    printf("odroid_system_init: %d KB free\n", esp_get_free_heap_size() / 1024);

    spiMutex = xSemaphoreCreateMutex();
    spiMutexOwner = -1;
    applicationId = appId;

    odroid_settings_init();
    odroid_overlay_init();
    odroid_system_gpio_init();
    odroid_input_gamepad_init();
    odroid_audio_init(sampleRate);

    //sdcard init must be before LCD init
    esp_err_t sd_init = odroid_sdcard_open();

    odroid_display_init();

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        odroid_system_panic("The application crashed!");
    }

    if (esp_reset_reason() != ESP_RST_SW)
    {
        odroid_display_clear(0);
        odroid_display_show_hourglass();
    }

    if (sd_init != ESP_OK)
    {
        odroid_display_clear(C_WHITE);
        odroid_display_write((ODROID_SCREEN_WIDTH - image_sdcard_red_48dp.width) / 2,
            (ODROID_SCREEN_HEIGHT - image_sdcard_red_48dp.height) / 2,
            image_sdcard_red_48dp.width,
            image_sdcard_red_48dp.height,
            (uint16_t*)image_sdcard_red_48dp.pixel_data);
        odroid_system_halt();
    }

    startAction = odroid_settings_StartAction_get();
    if (startAction == ODROID_START_ACTION_RESTART)
    {
        odroid_settings_StartAction_set(ODROID_START_ACTION_RESUME);
    }

    xTaskCreate(&odroid_system_monitor_task, "sysmon", 2048, NULL, 7, NULL);

    // esp_task_wdt_init(5, true);
    // esp_task_wdt_add(xTaskGetCurrentTaskHandle());

    printf("odroid_system_init: System ready!\n");
}

void odroid_system_emu_init(state_handler_t load, state_handler_t save, netplay_callback_t netplay_cb)
{
    uint8_t buffer[0x150];

    // If any key is pressed we go back to the menu (recover from ROM crash)
    if (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
    {
        odroid_system_switch_app(0);
    }

    if (netplay_cb != NULL)
    {
        odroid_netplay_pre_init(netplay_cb);
    }

    romPath = odroid_settings_RomFilePath_get();
    if (!romPath || strlen(romPath) < 4)
    {
        odroid_system_panic("Invalid ROM Path!");
    }

    printf("odroid_system_emu_init: romPath='%s'\n", romPath);

    // Read some of the ROM to derive a unique id
    if (!odroid_sdcard_copy_file_to_memory(romPath, buffer, sizeof(buffer)))
    {
        odroid_system_panic("ROM File not found!");
    }

    gameId = crc32_le(0, buffer, sizeof(buffer));
    loadState = load;
    saveState = save;

    printf("odroid_system_emu_init: Init done. GameId=%08X\n", gameId);
}

void odroid_system_set_app_id(int _appId)
{
    applicationId = _appId;
}

uint odroid_system_get_app_id()
{
    return applicationId;
}

uint odroid_system_get_game_id()
{
    return gameId;
}

uint odroid_system_get_start_action()
{
    return startAction;
}

const char* odroid_system_get_rom_path()
{
    return romPath;
}

char* odroid_system_get_path(emu_path_type_t type, char *_romPath)
{
    char *fileName = strstr(_romPath ?: romPath, ODROID_BASE_PATH_ROMS);
    char buffer[256];

    if (!fileName || strlen(fileName) < 4)
    {
        odroid_system_panic("Invalid ROM path!");
    }

    fileName += strlen(ODROID_BASE_PATH_ROMS);

    switch (type)
    {
        case ODROID_PATH_SAVE_STATE:
        case ODROID_PATH_SAVE_STATE_1:
        case ODROID_PATH_SAVE_STATE_2:
        case ODROID_PATH_SAVE_STATE_3:
            strcpy(buffer, ODROID_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sav");
            break;

        case ODROID_PATH_SAVE_BACK:
            strcpy(buffer, ODROID_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sav.bak");
            break;

        case ODROID_PATH_SAVE_SRAM:
            strcpy(buffer, ODROID_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sram");
            break;

        case ODROID_PATH_TEMP_FILE:
            sprintf(buffer, "%s/%X%X.tmp", ODROID_BASE_PATH_TEMP, get_elapsed_time(), rand());
            break;

        case ODROID_PATH_ROM_FILE:
            strcpy(buffer, ODROID_BASE_PATH_ROMS);
            strcat(buffer, fileName);
            break;

        case ODROID_PATH_CRC_CACHE:
            strcpy(buffer, ODROID_BASE_PATH_CRC_CACHE);
            strcat(buffer, fileName);
            strcat(buffer, ".crc");
            break;

        default:
            abort();
    }

    return strdup(buffer);
}

bool odroid_system_emu_load_state(int slot)
{
    if (!romPath || !loadState)
    {
        printf("%s: No game/emulator loaded...\n", __func__);
        return false;
    }

    printf("odroid_system_emu_load_state: Loading state %d.\n", slot);

    odroid_display_show_hourglass();
    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *pathName = odroid_system_get_path(ODROID_PATH_SAVE_STATE, romPath);
    bool success = (*loadState)(pathName);

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

    if (!success)
    {
        printf("%s: Load failed!\n", __func__);
    }

    free(pathName);

    return success;
}

bool odroid_system_emu_save_state(int slot)
{
    if (!romPath || !saveState)
    {
        printf("%s: No game/emulator loaded...\n", __func__);
        return false;
    }

    printf("odroid_system_emu_save_state: Saving state %d.\n", slot);

    odroid_system_set_led(1);
    odroid_display_show_hourglass();
    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *saveName = odroid_system_get_path(ODROID_PATH_SAVE_STATE, romPath);
    char *backName = odroid_system_get_path(ODROID_PATH_SAVE_BACK, romPath);
    char *tempName = odroid_system_get_path(ODROID_PATH_TEMP_FILE, romPath);

    bool success = false;

    if ((*saveState)(tempName))
    {
        rename(saveName, backName);

        if (rename(tempName, saveName) == 0)
        {
            unlink(backName);
            success = true;
        }
    }

    unlink(tempName);

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);
    odroid_system_set_led(0);

    if (!success)
    {
        printf("%s: Save failed!\n", __func__);
        odroid_overlay_alert("Save failed");
    }

    free(saveName);
    free(backName);
    free(tempName);

    return success;
}

void odroid_system_reload_app()
{
    //
}

void odroid_system_switch_app(int app)
{
    printf("odroid_system_switch_app: Switching to app %d.\n", app);

    odroid_display_clear(0);
    odroid_display_show_hourglass();

    odroid_audio_terminate();
    odroid_sdcard_close();

    odroid_system_set_boot_app(app);
    esp_restart();
}

void odroid_system_set_boot_app(int slot)
{
    const esp_partition_t* partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_OTA_MIN + slot,
        NULL);
    // Do not overwrite the boot sector for nothing
    const esp_partition_t* boot_partition = esp_ota_get_boot_partition();

    if (partition != NULL && partition != boot_partition)
    {
        esp_err_t err = esp_ota_set_boot_partition(partition);
        if (err != ESP_OK)
        {
            printf("odroid_system_set_boot_app: esp_ota_set_boot_partition failed.\n");
            abort();
        }
    }
}

void odroid_system_panic(const char *reason)
{
    printf(" *** PANIC: %s *** \n", reason);

    // Here we should stop unecessary tasks

    odroid_audio_terminate();

    // In case we panicked from inside a dialog
    odroid_system_spi_lock_release(SPI_LOCK_ANY);

    odroid_display_clear(C_BLUE);

    odroid_overlay_alert(reason);

    odroid_system_switch_app(0);
}

void odroid_system_halt()
{
    printf("odroid_system_halt: Halting system!\n");
    vTaskSuspendAll();
    while (1);
}

void odroid_system_sleep()
{
    printf("odroid_system_sleep: Entered.\n");

    // Wait for button release
    odroid_input_wait_for_key(ODROID_INPUT_MENU, false);
    odroid_audio_terminate();
    vTaskDelay(100);
    esp_deep_sleep_start();
}

void odroid_system_set_led(int value)
{
    gpio_set_level(GPIO_NUM_2, value);
}

static void odroid_system_monitor_task(void *arg)
{
    runtime_counters_t current;
    bool led_state = false;
    float tickTime = 0;

    while (1)
    {
        // Make a copy and reset counters immediately because processing could take 1-2ms
        current = counters;
        counters.totalFrames = counters.fullFrames = 0;
        counters.skippedFrames = counters.busyTime = 0;
        counters.resetTime = get_elapsed_time();

        tickTime = (counters.resetTime - current.resetTime);
        statistics.battery = odroid_input_battery_read();
        statistics.busyPercent = current.busyTime / tickTime * 100.f;
        statistics.skippedFPS = current.skippedFrames / (tickTime / 1000000.f);
        statistics.totalFPS = current.totalFrames / (tickTime / 1000000.f);
        // To do get the actual game refresh rate somehow
        statistics.emulatedSpeed = statistics.totalFPS / 60 * 100.f;

        statistics.freeMemoryInt = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        statistics.freeMemoryExt = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

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

        if (statistics.lastTickTime > 0 && tickTime > 1000000.f)
        {
            // printf("WATCHDOG: Last emulation tick was %ds ago\n", seconds);
            // odroid_system_panic("The application froze!");
        }

        if (statistics.battery.percentage < 2)
        {
            led_state = !led_state;
            odroid_system_set_led(led_state);
        }
        else if (led_state)
        {
            led_state = false;
            odroid_system_set_led(led_state);
        }

        printf("HEAP:%d+%d, BUSY:%.4f, FPS:%.4f (SKIP:%d, PART:%d, FULL:%d), BATTERY:%d\n",
            statistics.freeMemoryInt / 1024,
            statistics.freeMemoryExt / 1024,
            statistics.busyPercent,
            statistics.totalFPS,
            current.skippedFrames,
            current.totalFrames - current.fullFrames - current.skippedFrames,
            current.fullFrames,
            statistics.battery.millivolts);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

IRAM_ATTR void odroid_system_tick(uint skippedFrame, uint fullFrame, uint busyTime)
{
    if (skippedFrame) counters.skippedFrames++;
    else if (fullFrame) counters.fullFrames++;
    counters.totalFrames++;
    counters.busyTime += busyTime;

    statistics.lastTickTime = get_elapsed_time();
}

runtime_stats_t odroid_system_get_stats()
{
    return statistics;
}

IRAM_ATTR void odroid_system_spi_lock_acquire(spi_lock_res_t owner)
{
    if (owner == spiMutexOwner)
    {
        return;
    }
    else if (xSemaphoreTake(spiMutex, 10000 / portTICK_RATE_MS) == pdPASS)
    {
        spiMutexOwner = owner;
    }
    else
    {
        printf("SPI Mutex Lock Acquisition failed!\n");
        abort();
    }
}

IRAM_ATTR void odroid_system_spi_lock_release(spi_lock_res_t owner)
{
    if (owner == spiMutexOwner || owner == SPI_LOCK_ANY)
    {
        xSemaphoreGive(spiMutex);
        spiMutexOwner = SPI_LOCK_ANY;
    }
}

/* helpers */

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
               printf("RG_ALLOC: *** Memory allocation failed ***\n");
               odroid_system_panic("Memory allocation failed!");
          }

          printf("RG_ALLOC: *** CAPS not fully met (req: %d, available: %d) ***\n", size, availaible);
     }

     return ptr;
}

void rg_free(void *ptr)
{
     return heap_caps_free(ptr);
}
