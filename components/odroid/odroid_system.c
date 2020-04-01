#include "freertos/FreeRTOS.h"
#include "odroid_image_sdcard.h"
#include "odroid_system.h"
#include "esp_heap_caps.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/rtc_io.h"
#include "string.h"
#include "stdio.h"

int8_t speedupEnabled = 0;

static int8_t applicationId = -1;
static int8_t startAction = 0;
static char *romPath = NULL;
static state_handler_t loadState;
static state_handler_t saveState;

static SemaphoreHandle_t spiMutex;
static int8_t spiMutexOwner;

void odroid_system_init(int appId, int sampleRate)
{
    printf("odroid_system_init: %d KB free\n", esp_get_free_heap_size() / 1024);

    odroid_settings_init(appId);
    odroid_overlay_init();
    odroid_system_gpio_init();
    odroid_input_gamepad_init();
    odroid_input_battery_level_init();
    odroid_audio_init(sampleRate);

    spiMutex = xSemaphoreCreateMutex();
    spiMutexOwner = -1;
    applicationId = appId;

    odroid_gamepad_state bootState = odroid_input_read_raw();
    if (bootState.values[ODROID_INPUT_MENU])
    {
        // Force return to menu to recover from ROM loading crashes
        odroid_system_set_boot_app(0);
        esp_restart();
    }

    //sdcard init must be before LCD init
    esp_err_t sd_init = odroid_sdcard_open();

    odroid_display_init();

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        odroid_system_panic("The application crashed");
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
            image_sdcard_red_48dp.pixel_data);
        odroid_system_halt();
    }

    printf("odroid_system_init: System ready!\n");
}

void odroid_system_emu_init(char **_romPath, int8_t *_startAction,
                            state_handler_t load, state_handler_t save)
{
    romPath = odroid_settings_RomFilePath_get();
    if (!romPath || strlen(romPath) < 4)
    {
        odroid_system_panic("ROM File not found");
    }

    startAction = odroid_settings_StartAction_get();
    if (startAction == ODROID_START_ACTION_RESTART)
    {
        odroid_settings_StartAction_set(ODROID_START_ACTION_RESUME);
    }

    *_startAction = startAction;
    *_romPath = romPath;

    loadState = load;
    saveState = save;

    printf("odroid_system_emu_init: ROM: %s, action: %d\n", romPath, startAction);
}

void odroid_system_set_app_id(int appId)
{
    applicationId = appId;
    odroid_settings_set_app_id(appId);
}

void odroid_system_gpio_init()
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

char* odroid_system_get_path(char *_romPath, emu_path_type_t type)
{
    char* fileName = strstr(_romPath ?: romPath, ODROID_BASE_PATH_ROMS);
    char *buffer = malloc(128); // Lazy arbitrary length...

    if (!fileName)
    {
        printf("%s: Invalid rom path.\n", __func__);
        abort();
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

        case ODROID_PATH_SAVE_SRAM:
            strcpy(buffer, ODROID_BASE_PATH_SAVES);
            strcat(buffer, fileName);
            strcat(buffer, ".sram");
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

    return buffer;
}

bool odroid_system_load_state(int slot)
{
    if (!romPath || !loadState)
        odroid_system_panic("Emulator not initialized");

    odroid_display_show_hourglass();
    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *pathName = odroid_system_get_path(NULL, ODROID_PATH_SAVE_STATE);
    bool success = (*loadState)(pathName);

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

    if (!success)
    {
        printf("odroid_system_load_state: Load failed!\n");
    }

    free(pathName);

    return success;
}

bool odroid_system_save_state(int slot)
{
    if (!romPath || !saveState)
        odroid_system_panic("Emulator not initialized");

    odroid_input_battery_monitor_enabled_set(0);
    odroid_system_set_led(1);
    odroid_display_show_hourglass();
    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    char *pathName = odroid_system_get_path(NULL, ODROID_PATH_SAVE_STATE);
    bool success = (*saveState)(pathName);

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);
    odroid_system_set_led(0);
    odroid_input_battery_monitor_enabled_set(1);

    if (!success)
    {
        printf("odroid_system_save_state: Save failed!\n");
        odroid_overlay_alert("Save failed");
    }

    free(pathName);

    return success;
}

void odroid_system_quit_app(bool save)
{
    printf("odroid_system_quit_app: Stopping tasks.\n");

    odroid_audio_terminate();

    // odroid_display_queue_update(NULL);
    odroid_display_clear(0);
    odroid_display_show_hourglass();

    if (save)
    {
        printf("odroid_system_quit_app: Saving state.\n");
        odroid_system_save_state(0);
    }

    // Set menu application
    odroid_system_set_boot_app(0);

    // Reset
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

void odroid_system_sleep()
{
    printf("odroid_system_sleep: Entered.\n");

    // Wait for button release
    odroid_input_wait_for_key(ODROID_INPUT_MENU, false);

    vTaskDelay(100);
    esp_deep_sleep_start();
}

void odroid_system_panic(const char *reason)
{
    printf("PANIC: %s\n", reason);

    // Here we should stop unecessary tasks

    odroid_system_set_boot_app(0);

    odroid_audio_terminate();

    // In case we panicked from inside a dialog
    odroid_system_spi_lock_release(SPI_LOCK_ANY);

    odroid_display_clear(C_BLUE);

    odroid_overlay_alert(reason);

    esp_restart();
}

void odroid_system_halt()
{
    printf("odroid_system_halt: Halting system!\n");
    vTaskSuspendAll();
    while (1);
}

void odroid_system_set_led(int value)
{
    gpio_set_level(GPIO_NUM_2, value);
}

void odroid_system_print_stats(uint us, uint frames, uint skippedFrames, uint fullFrames)
{
    float seconds = (float)us / 1000000.f;
    float fps = frames / seconds;

    odroid_battery_state battery;
    odroid_input_battery_level_read(&battery);

    printf("HEAP:%d+%d, FPS:%f, SKIP:%d, FULL:%d, BATTERY:%d [%d]\n",
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024,
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
        fps, skippedFrames, fullFrames,
        battery.millivolts, battery.percentage);
}

void IRAM_ATTR odroid_system_spi_lock_acquire(spi_lock_res_t owner)
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

void IRAM_ATTR odroid_system_spi_lock_release(spi_lock_res_t owner)
{
    if (owner == spiMutexOwner || owner == SPI_LOCK_ANY)
    {
        xSemaphoreGive(spiMutex);
        spiMutexOwner = SPI_LOCK_ANY;
    }
}
