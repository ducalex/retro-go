#include "odroid_input.h"
#include "odroid_settings.h"
#include "odroid_system.h"

#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include <string.h>

static volatile bool input_task_is_running = false;
static volatile odroid_gamepad_state gamepad_state;
static volatile bool input_gamepad_initialized = false;
static SemaphoreHandle_t xSemaphore;

static esp_adc_cal_characteristics_t characteristics;
static bool input_battery_initialized = false;
static float adc_value = 0.0f;
static bool battery_monitor_enabled = true;
static odroid_battery_state battery_state;

odroid_gamepad_state odroid_input_read_raw()
{
    odroid_gamepad_state state = {0};

    int joyX = adc1_get_raw(ODROID_GAMEPAD_IO_X);
    int joyY = adc1_get_raw(ODROID_GAMEPAD_IO_Y);

    if (joyX > 2048 + 1024)
    {
        state.values[ODROID_INPUT_LEFT] = 1;
        state.values[ODROID_INPUT_RIGHT] = 0;
    }
    else if (joyX > 1024)
    {
        state.values[ODROID_INPUT_LEFT] = 0;
        state.values[ODROID_INPUT_RIGHT] = 1;
    }
    else
    {
        state.values[ODROID_INPUT_LEFT] = 0;
        state.values[ODROID_INPUT_RIGHT] = 0;
    }

    if (joyY > 2048 + 1024)
    {
        state.values[ODROID_INPUT_UP] = 1;
        state.values[ODROID_INPUT_DOWN] = 0;
    }
    else if (joyY > 1024)
    {
        state.values[ODROID_INPUT_UP] = 0;
        state.values[ODROID_INPUT_DOWN] = 1;
    }
    else
    {
        state.values[ODROID_INPUT_UP] = 0;
        state.values[ODROID_INPUT_DOWN] = 0;
    }

    state.values[ODROID_INPUT_SELECT] = !(gpio_get_level(ODROID_GAMEPAD_IO_SELECT));
    state.values[ODROID_INPUT_START] = !(gpio_get_level(ODROID_GAMEPAD_IO_START));

    state.values[ODROID_INPUT_A] = !(gpio_get_level(ODROID_GAMEPAD_IO_A));
    state.values[ODROID_INPUT_B] = !(gpio_get_level(ODROID_GAMEPAD_IO_B));

    state.values[ODROID_INPUT_MENU] = !(gpio_get_level(ODROID_GAMEPAD_IO_MENU));
    state.values[ODROID_INPUT_VOLUME] = !(gpio_get_level(ODROID_GAMEPAD_IO_VOLUME));

    return state;
}

static void odroid_input_task(void *arg)
{
    input_task_is_running = true;

    odroid_gamepad_state previous_gamepad_state;
    uint8_t debounce[ODROID_INPUT_MAX];

    // Initialize state
    for(int i = 0; i < ODROID_INPUT_MAX; ++i)
    {
        debounce[i] = 0xff;
    }

    while(input_task_is_running)
    {
        // Read hardware
        odroid_gamepad_state state = odroid_input_read_raw();

        for(int i = 0; i < ODROID_INPUT_MAX; ++i)
		{
            // Shift current values
			debounce[i] <<= 1;
		}

        // Debounce
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        gamepad_state.bitmask = 0;

        for(int i = 0; i < ODROID_INPUT_MAX; ++i)
		{
            debounce[i] |= state.values[i] ? 1 : 0;
            uint8_t val = debounce[i] & 0x03; //0x0f;
            switch (val) {
                case 0x00:
                    gamepad_state.values[i] = 0;
                    break;

                case 0x03: //0x0f:
                    gamepad_state.values[i] = 1;
                    gamepad_state.bitmask |= 1 << i;
                    break;

                default:
                    // ignore
                    break;
            }

            //gamepad_state.values[i] = (debounce[i] == 0xff);
            //printf("odroid_input_task: %d=%d (raw=%d)\n", i, gamepad_state.values[i], state.values[i]);
		}

        previous_gamepad_state = gamepad_state;

        xSemaphoreGive(xSemaphore);

        // delay
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    input_gamepad_initialized = false;

    vSemaphoreDelete(xSemaphore);

    // Remove the task from scheduler
    vTaskDelete(NULL);

    // Never return
    while (1) { vTaskDelay(1);}
}

void odroid_input_gamepad_init()
{
    //printf("portTICK_PERIOD_MS = %d\n", portTICK_PERIOD_MS);

    xSemaphore = xSemaphoreCreateMutex();

    if(xSemaphore == NULL)
    {
        printf("xSemaphoreCreateMutex failed.\n");
        abort();
    }

	gpio_set_direction(ODROID_GAMEPAD_IO_SELECT, GPIO_MODE_INPUT);
	gpio_set_pull_mode(ODROID_GAMEPAD_IO_SELECT, GPIO_PULLUP_ONLY);

	gpio_set_direction(ODROID_GAMEPAD_IO_START, GPIO_MODE_INPUT);

	gpio_set_direction(ODROID_GAMEPAD_IO_A, GPIO_MODE_INPUT);
	gpio_set_pull_mode(ODROID_GAMEPAD_IO_A, GPIO_PULLUP_ONLY);

    gpio_set_direction(ODROID_GAMEPAD_IO_B, GPIO_MODE_INPUT);
	gpio_set_pull_mode(ODROID_GAMEPAD_IO_B, GPIO_PULLUP_ONLY);

	adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ODROID_GAMEPAD_IO_X, ADC_ATTEN_11db);
	adc1_config_channel_atten(ODROID_GAMEPAD_IO_Y, ADC_ATTEN_11db);

	gpio_set_direction(ODROID_GAMEPAD_IO_MENU, GPIO_MODE_INPUT);
	gpio_set_pull_mode(ODROID_GAMEPAD_IO_MENU, GPIO_PULLUP_ONLY);

	gpio_set_direction(ODROID_GAMEPAD_IO_VOLUME, GPIO_MODE_INPUT);

    input_gamepad_initialized = true;

    // Start background polling
    xTaskCreatePinnedToCore(&odroid_input_task, "odroid_input_task", 1024 * 2, NULL, 5, NULL, 1);

  	printf("odroid_input_gamepad_init done.\n");

}

void odroid_input_gamepad_terminate()
{
    if (!input_gamepad_initialized) abort();

    input_task_is_running = false;
}

void odroid_input_gamepad_read(odroid_gamepad_state* out_state)
{
    if (!input_gamepad_initialized) abort();

    xSemaphoreTake(xSemaphore, portMAX_DELAY);

    *out_state = gamepad_state;

    xSemaphoreGive(xSemaphore);
}

void odroid_input_wait_for_key(int key, bool pressed)
{
	while (true)
    {
        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (joystick.values[key] == pressed) {
        	break;
        }
    }
}

static void odroid_battery_monitor_task()
{
    bool led_state = false;

    while(true)
    {
        if (battery_monitor_enabled)
        {
            odroid_input_battery_level_read_now(&battery_state);

            if (battery_state.percentage < 2)
            {
                led_state = !led_state;
                odroid_system_led_set(led_state);
            }
            else if (led_state)
            {
                led_state = 0;
                odroid_system_led_set(led_state);
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


#define DEFAULT_VREF 1100
void odroid_input_battery_level_init()
{
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);

    //int vref_value = odroid_settings_VRef_get();
    //esp_adc_cal_get_characteristics(vref_value, ADC_ATTEN_11db, ADC_WIDTH_12Bit, &characteristics);

    //Characterize ADC
    //adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, DEFAULT_VREF, &characteristics);

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("ADC: Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("ADC: Characterized using eFuse Vref\n");
    } else {
        printf("ADC: Characterized using Default Vref\n");
    }

    input_battery_initialized = true;
    xTaskCreatePinnedToCore(&odroid_battery_monitor_task, "battery_monitor", 1024, NULL, 5, NULL, 1);
}

void odroid_input_battery_level_read_now(odroid_battery_state* out_state)
{
    if (!input_battery_initialized)
    {
        printf("odroid_input_battery_level_read: not initilized.\n");
        abort();
    }


    const int sampleCount = 4;

    float adcSample = 0.0f;
    for (int i = 0; i < sampleCount; ++i)
    {
        //adcSample += adc1_to_voltage(ADC1_CHANNEL_0, &characteristics) * 0.001f;
        adcSample += esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &characteristics) * 0.001f;
    }
    adcSample /= sampleCount;


    if (adc_value == 0.0f)
    {
        adc_value = adcSample;
    }
    else
    {
        adc_value += adcSample;
        adc_value /= 2.0f;
    }


    // Vo = (Vs * R2) / (R1 + R2)
    // Vs = Vo / R2 * (R1 + R2)
    const float R1 = 10000;
    const float R2 = 10000;
    const float Vo = adc_value;
    const float Vs = (Vo / R2 * (R1 + R2));

    const float FullVoltage = 4.2f;
    const float EmptyVoltage = 3.5f;

    out_state->millivolts = (int)(Vs * 1000);
    out_state->percentage = (int)((Vs - EmptyVoltage) / (FullVoltage - EmptyVoltage) * 100.0f);
}

void odroid_input_battery_level_read(odroid_battery_state* out_state)
{
    if (battery_monitor_enabled) {
        out_state->millivolts = battery_state.millivolts;
        out_state->percentage = battery_state.percentage;
    } else {
        odroid_input_battery_level_read_now(out_state);
    }
}

void odroid_input_battery_monitor_enabled_set(int value)
{
    battery_monitor_enabled = value;
}
