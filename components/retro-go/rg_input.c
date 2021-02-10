#include <freertos/FreeRTOS.h>
#include <esp_adc_cal.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "rg_system.h"
#include "rg_input.h"

static bool input_task_is_running = false;
static bool use_external_gamepad = 0;
static int64_t last_gamepad_read = 0;
static gamepad_state_t gamepad_state;
static SemaphoreHandle_t xSemaphore;

static inline gamepad_state_t console_gamepad_read(void)
{
    gamepad_state_t state = {0};

    int joyX = adc1_get_raw(RG_GPIO_GAMEPAD_X);
    int joyY = adc1_get_raw(RG_GPIO_GAMEPAD_Y);

    state.values[GAMEPAD_KEY_UP]   = (joyY > 2048 + 1024);
    state.values[GAMEPAD_KEY_DOWN] = (joyY < 2048 + 1024) && (joyY > 1024);

    state.values[GAMEPAD_KEY_LEFT]  = (joyX > 2048 + 1024);
    state.values[GAMEPAD_KEY_RIGHT] = (joyX < 2048 + 1024) && (joyX > 1024);

    state.values[GAMEPAD_KEY_MENU] = !gpio_get_level(RG_GPIO_GAMEPAD_MENU);
    state.values[GAMEPAD_KEY_VOLUME] = !gpio_get_level(RG_GPIO_GAMEPAD_VOLUME);

    state.values[GAMEPAD_KEY_SELECT] = !gpio_get_level(RG_GPIO_GAMEPAD_SELECT);
    state.values[GAMEPAD_KEY_START] = !gpio_get_level(RG_GPIO_GAMEPAD_START);

    state.values[GAMEPAD_KEY_A] = !gpio_get_level(RG_GPIO_GAMEPAD_A);
    state.values[GAMEPAD_KEY_B] = !gpio_get_level(RG_GPIO_GAMEPAD_B);

    return state;
}

static inline gamepad_state_t external_gamepad_read(void)
{
    gamepad_state_t state = {0};

    // Unfortunately the GO doesn't bring out enough GPIO for both ext DAC and controller...
    if (rg_audio_get_sink() != RG_AUDIO_SINK_DAC)
    {
        // NES / SNES shift register
    }

    return state;
}

static void input_task(void *arg)
{
    uint8_t debounce[GAMEPAD_KEY_MAX];
    uint8_t debounce_level = 0x03; // 0x0f

    // Initialize debounce state
    memset(debounce, 0xFF, GAMEPAD_KEY_MAX);

    input_task_is_running = true;

    while (input_task_is_running)
    {
        // Read internal controller
        gamepad_state_t state = console_gamepad_read();

        // Read external controller
        if (use_external_gamepad)
        {
            gamepad_state_t ext_state = external_gamepad_read();
            for (int i = 0; i < GAMEPAD_KEY_MAX; ++i)
            {
                state.values[i] |= ext_state.values[i];
            }
        }

        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        for (int i = 0; i < GAMEPAD_KEY_MAX; ++i)
		{
            // Debounce
            debounce[i] = (debounce[i] << 1) | (state.values[i] & 1);

            uint8_t val = debounce[i] & debounce_level;

            if (val == debounce_level) // Pressed
            {
                gamepad_state.values[i] = 1;
                gamepad_state.bitmask |= 1 << i;
            }
            else if (val == 0x00) // Released
            {
                gamepad_state.values[i] = 0;
                gamepad_state.bitmask &= ~(1 << i);
            }
		}

        xSemaphoreGive(xSemaphore);

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vSemaphoreDelete(xSemaphore);
    vTaskDelete(NULL);
}

void rg_input_init(void)
{
    assert(input_task_is_running == false);

    xSemaphore = xSemaphoreCreateMutex();

	adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(RG_GPIO_GAMEPAD_X, ADC_ATTEN_11db);
	adc1_config_channel_atten(RG_GPIO_GAMEPAD_Y, ADC_ATTEN_11db);

	gpio_set_direction(RG_GPIO_GAMEPAD_MENU, GPIO_MODE_INPUT);
	gpio_set_pull_mode(RG_GPIO_GAMEPAD_MENU, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_VOLUME, GPIO_MODE_INPUT);
	// gpio_set_pull_mode(RG_GPIO_GAMEPAD_VOLUME, GPIO_PULLUP_ONLY);

	gpio_set_direction(RG_GPIO_GAMEPAD_SELECT, GPIO_MODE_INPUT);
	gpio_set_pull_mode(RG_GPIO_GAMEPAD_SELECT, GPIO_PULLUP_ONLY);
	gpio_set_direction(RG_GPIO_GAMEPAD_START, GPIO_MODE_INPUT);
	// gpio_set_direction(RG_GPIO_GAMEPAD_START, GPIO_PULLUP_ONLY);

	gpio_set_direction(RG_GPIO_GAMEPAD_A, GPIO_MODE_INPUT);
	gpio_set_pull_mode(RG_GPIO_GAMEPAD_A, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_B, GPIO_MODE_INPUT);
	gpio_set_pull_mode(RG_GPIO_GAMEPAD_B, GPIO_PULLUP_ONLY);

    // Start background polling
    xTaskCreatePinnedToCore(&input_task, "input_task", 2048, NULL, 5, NULL, 1);

  	RG_LOGI("init done.\n");
}

void rg_input_deinit(void)
{
    input_task_is_running = false;
}

long rg_input_gamepad_last_read(void)
{
    if (!last_gamepad_read)
        return 0;

    return get_elapsed_time_since(last_gamepad_read);
}

gamepad_state_t rg_input_read_gamepad(void)
{
    assert(input_task_is_running == true);

    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    gamepad_state_t state = gamepad_state;
    xSemaphoreGive(xSemaphore);

    last_gamepad_read = get_elapsed_time();

    return state;
}

bool rg_input_key_is_pressed(gamepad_key_t key)
{
    gamepad_state_t joystick = rg_input_read_gamepad();

    if (key == GAMEPAD_KEY_ANY) {
        return joystick.bitmask > 0 ? 1 : 0;
    }

    return joystick.values[key];
}

void rg_input_wait_for_key(gamepad_key_t key, bool pressed)
{
	while (rg_input_key_is_pressed(key) != pressed)
    {
        vTaskDelay(1);
    }
}

battery_state_t rg_input_read_battery()
{
    static esp_adc_cal_characteristics_t adc_chars;
    static float adcValue = 0.0f;

    const int sampleCount = 4;
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

    const float Vs = (adcValue / RG_BATT_DIVIDER_R2 * (RG_BATT_DIVIDER_R1 + RG_BATT_DIVIDER_R2));
    const float Vconst = RG_MAX(RG_BATT_VOLTAGE_EMPTY, RG_MIN(Vs, RG_BATT_VOLTAGE_FULL));

    battery_state_t out_state = {
        .millivolts = (int)(Vs * 1000),
        .percentage = (int)((Vconst - RG_BATT_VOLTAGE_EMPTY) / (RG_BATT_VOLTAGE_FULL - RG_BATT_VOLTAGE_EMPTY) * 100.0f),
    };

    return out_state;
}
