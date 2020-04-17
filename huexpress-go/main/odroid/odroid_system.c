#include "odroid_system.h"

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/rtc_io.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "odroid_input.h"

static bool system_initialized = false;

void odroid_system_application_set(int slot)
{
    const esp_partition_t* partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_OTA_MIN + slot,
        NULL);
    if (partition != NULL)
    {
        esp_err_t err = esp_ota_set_boot_partition(partition);
        if (err != ESP_OK)
        {
            printf("odroid_system_application_set: esp_ota_set_boot_partition failed.\n");
            abort();
        }
    }
}

void odroid_system_sleep()
{
    printf("odroid_system_sleep: Entered.\n");

    // Wait for button release
    odroid_gamepad_state joystick;
    odroid_input_gamepad_read(&joystick);

    while (joystick.values[ODROID_INPUT_MENU])
    {
        vTaskDelay(1);
        odroid_input_gamepad_read(&joystick);
    }

    //odroid_input_gamepad_terminate();


    // Configure button to wake
    printf("odroid_system_sleep: Configuring deep sleep.\n");
#if 1
    esp_err_t err = esp_sleep_enable_ext0_wakeup(ODROID_GAMEPAD_IO_MENU, 0);
#else
    const int ext_wakeup_pin_1 = ODROID_GAMEPAD_IO_MENU;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

    esp_err_t err = esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ALL_LOW);
#endif
    if (err != ESP_OK)
    {
        printf("odroid_system_sleep: esp_sleep_enable_ext0_wakeup failed.\n");
        abort();
    }

    err = rtc_gpio_pullup_en(ODROID_GAMEPAD_IO_MENU);
    if (err != ESP_OK)
    {
        printf("odroid_system_sleep: rtc_gpio_pullup_en failed.\n");
        abort();
    }


    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#if 1
    rtc_gpio_isolate(GPIO_NUM_34);
    rtc_gpio_isolate(GPIO_NUM_35);
    rtc_gpio_isolate(GPIO_NUM_0);
    rtc_gpio_isolate(GPIO_NUM_39);
    //rtc_gpio_isolate(GPIO_NUM_14);
#endif

    // Sleep
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    vTaskDelay(100);
    esp_deep_sleep_start();
}

void odroid_system_init()
{
    rtc_gpio_deinit(ODROID_GAMEPAD_IO_MENU);
    //rtc_gpio_deinit(GPIO_NUM_14);

    // blue led
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 0);

    system_initialized = true;
}

void odroid_system_led_set(int value)
{
    if (!system_initialized)
    {
        printf("odroid_system_init not called before use.\n");
        abort();
    }

    gpio_set_level(GPIO_NUM_2, value);
}
