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
static odroid_gamepad_state gamepad_state;
static SemaphoreHandle_t xSemaphore;
static esp_adc_cal_characteristics_t adc_chars;

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

static void input_task(void *arg)
{
    input_task_is_running = true;

    uint8_t debounce[ODROID_INPUT_MAX];

    // Initialize debounce state
    memset(debounce, 0xFF, ODROID_INPUT_MAX);

    while (input_task_is_running)
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
		}

        xSemaphoreGive(xSemaphore);

        // delay
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vSemaphoreDelete(xSemaphore);
    vTaskDelete(NULL);
}

void odroid_input_gamepad_init()
{
    if (input_task_is_running) abort();

    xSemaphore = xSemaphoreCreateMutex();

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

    // Start background polling
    xTaskCreatePinnedToCore(&input_task, "input_task", 2048, NULL, 5, NULL, 1);

  	printf("odroid_input_gamepad_init done.\n");
}

void odroid_input_gamepad_terminate()
{
    input_task_is_running = false;
}

void odroid_input_gamepad_read(odroid_gamepad_state* out_state)
{
    if (!input_task_is_running) abort();

    xSemaphoreTake(xSemaphore, portMAX_DELAY);

    *out_state = gamepad_state;

    xSemaphoreGive(xSemaphore);
}

bool odroid_input_key_is_pressed(int key)
{
    odroid_gamepad_state joystick;
    odroid_input_gamepad_read(&joystick);

    if (key == ODROID_INPUT_ANY) {
        for (int i = 0; i < ODROID_INPUT_MAX; i++) {
            if (joystick.values[i] == true) {
                return true;
            }
        }
        return false;
    }

    return joystick.values[key];
}

void odroid_input_wait_for_key(int key, bool pressed)
{
	while (true)
    {
        if (odroid_input_key_is_pressed(key) == pressed)
        {
            break;
        }
        vTaskDelay(1);
    }
}

odroid_battery_state odroid_input_battery_read()
{
    static float adcValue = 0.0f;

    short sampleCount = 4;
    float adcSample = 0.0f;

    // ADC not initialized
    if (adc_chars.vref == 0)
    {
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    }

    for (int i = 0; i < sampleCount; ++i)
    {
        adcSample += esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc_chars) * 0.001f;
    }
    adcSample /= sampleCount;


    if (adcValue == 0.0f)
    {
        adcValue = adcSample;
    }
    else
    {
        adcValue += adcSample;
        adcValue /= 2.0f;
    }


    // Vo = (Vs * R2) / (R1 + R2)
    // Vs = Vo / R2 * (R1 + R2)
    const float R1 = 10000;
    const float R2 = 10000;
    const float Vo = adcValue;
    const float Vs = (Vo / R2 * (R1 + R2));

    const float FullVoltage = 4.2f;
    const float EmptyVoltage = 3.5f;

    odroid_battery_state out_state = {
        .millivolts = (int)(Vs * 1000),
        .percentage = (int)((Vs - EmptyVoltage) / (FullVoltage - EmptyVoltage) * 100.0f),
    };

    return out_state;
}
