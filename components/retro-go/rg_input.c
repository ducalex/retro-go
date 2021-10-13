#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc_cal.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <string.h>

#include "rg_system.h"
#include "rg_input.h"
#include "rg_i2c.h"

static bool input_initialized = false;
static int64_t last_gamepad_read = 0;
// Keep the following <= 32bit to ensure atomic access without mutexes
static uint32_t gamepad_state = 0;
static float battery_level = -1;
static float battery_volts = 0;


static inline uint32_t gamepad_read(void)
{
    uint32_t state = 0;

#if RG_GAMEPAD_DRIVER == 1    // GPIO

    int joyX = adc1_get_raw(RG_GPIO_GAMEPAD_X);
    int joyY = adc1_get_raw(RG_GPIO_GAMEPAD_Y);

    if (joyY > 2048 + 1024) state |= RG_KEY_UP;
    else if (joyY > 1024)   state |= RG_KEY_DOWN;
    if (joyX > 2048 + 1024) state |= RG_KEY_LEFT;
    else if (joyX > 1024)   state |= RG_KEY_RIGHT;

    if (!gpio_get_level(RG_GPIO_GAMEPAD_MENU))   state |= RG_KEY_MENU;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_OPTION)) state |= RG_KEY_OPTION;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_SELECT)) state |= RG_KEY_SELECT;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_START))  state |= RG_KEY_START;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_A))      state |= RG_KEY_A;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_B))      state |= RG_KEY_B;

#elif RG_GAMEPAD_DRIVER == 2  // Serial

#elif RG_GAMEPAD_DRIVER == 3  // I2C

    uint8_t data[5];
    if (rg_i2c_read(0x20, -1, &data, 5))
    {
        int buttons = ~((data[2] << 8) | data[1]);

        if (buttons & (1 << 2)) state |= RG_KEY_UP;
        if (buttons & (1 << 3)) state |= RG_KEY_DOWN;
        if (buttons & (1 << 4)) state |= RG_KEY_LEFT;
        if (buttons & (1 << 5)) state |= RG_KEY_RIGHT;
        if (buttons & (1 << 8)) state |= RG_KEY_MENU;
        // if (buttons & (1 << 0)) state |= RG_KEY_OPTION;
        if (buttons & (1 << 1)) state |= RG_KEY_SELECT;
        if (buttons & (1 << 0)) state |= RG_KEY_START;
        if (buttons & (1 << 6)) state |= RG_KEY_A;
        if (buttons & (1 << 7)) state |= RG_KEY_B;

        battery_level = RG_MAX(0.f, RG_MIN(100.f, ((int)data[4] - 170) / 30.f * 100.f));
    }

#endif

    return state;
}

static void input_task(void *arg)
{
    const uint8_t debounce_level = 0x03;
    uint8_t debounce[RG_KEY_COUNT];
    uint32_t new_gamepad_state = 0;

    // Initialize debounce state
    memset(debounce, 0xFF, sizeof(debounce));

    while (input_initialized)
    {
        uint32_t state = gamepad_read();

        for (int i = 0; i < RG_KEY_COUNT; ++i)
        {
            debounce[i] = ((debounce[i] << 1) | ((state >> i) & 1));
            debounce[i] &= debounce_level;

            if (debounce[i] == debounce_level) // Pressed
            {
                new_gamepad_state |= (1 << i);
            }
            else if (debounce[i] == 0x00) // Released
            {
                new_gamepad_state &= ~(1 << i);
            }
        }

        gamepad_state = new_gamepad_state;

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void rg_input_init(void)
{
    if (input_initialized)
        return;

#if RG_GAMEPAD_DRIVER == 1    // GPIO

    const char *driver = "GPIO";

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(RG_GPIO_GAMEPAD_X, ADC_ATTEN_11db);
    adc1_config_channel_atten(RG_GPIO_GAMEPAD_Y, ADC_ATTEN_11db);

    gpio_set_direction(RG_GPIO_GAMEPAD_MENU, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_MENU, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_OPTION, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(RG_GPIO_GAMEPAD_OPTION, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_SELECT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_SELECT, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_START, GPIO_MODE_INPUT);
    // gpio_set_direction(RG_GPIO_GAMEPAD_START, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_A, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_B, GPIO_PULLUP_ONLY);

#elif RG_GAMEPAD_DRIVER == 2  // Serial

    const char *driver = "SERIAL";

    gpio_set_direction(RG_GPIO_GAMEPAD_CLOCK, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_LATCH, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_DATA, GPIO_MODE_INPUT);

#elif RG_GAMEPAD_DRIVER == 3  // I2C

    const char *driver = "MRGC-I2C";

    // This will initialize i2c and discard the first read (garbage)
    gamepad_read();

#else

    #error "No gamepad driver selected"

#endif

    // Start background polling
    xTaskCreatePinnedToCore(&input_task, "input_task", 1024, NULL, 5, NULL, 1);

    input_initialized = true;

    RG_LOGI("Input ready. driver='%s'\n", driver);
}

void rg_input_deinit(void)
{
    RG_LOGI("Input terminated.\n");
    input_initialized = false;
}

long rg_input_gamepad_last_read(void)
{
    if (!last_gamepad_read)
        return 0;

    return get_elapsed_time_since(last_gamepad_read);
}

uint32_t rg_input_read_gamepad(void)
{
    last_gamepad_read = get_elapsed_time();
    return gamepad_state;
}

bool rg_input_key_is_pressed(rg_key_t key)
{
    return (rg_input_read_gamepad() & key) ? true : false;
}

void rg_input_wait_for_key(rg_key_t key, bool pressed)
{
    while (rg_input_key_is_pressed(key) != pressed)
    {
        vTaskDelay(1);
    }
}

bool rg_input_read_battery(float *percent, float *volts)
{
#if defined(RG_BATT_ADC_CHANNEL)
    static float adcValue = 0.0f;
    static esp_adc_cal_characteristics_t adc_chars;
    uint32_t adcSample = 0;

    // ADC not initialized
    if (adc_chars.vref == 0)
    {
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    }

    for (int i = 0; i < 4; ++i)
    {
        adcSample += esp_adc_cal_raw_to_voltage(adc1_get_raw(RG_BATT_ADC_CHANNEL), &adc_chars);
    }
    adcSample /= 4;

    if (adcValue == 0.0f)
    {
        adcValue = adcSample * 0.001f;
    }
    else
    {
        adcValue += adcSample * 0.001f;
        adcValue /= 2.0f;
    }

    battery_volts = adcValue * RG_BATT_MULTIPLIER;
    battery_level = ((battery_volts) - RG_BATT_VOLT_MIN) / (RG_BATT_VOLT_MAX - RG_BATT_VOLT_MIN) * 100.f;
    battery_level = RG_MAX(0.f, RG_MIN(100.f, battery_level));
#else
    // We could read i2c here but the i2c API isn't threadsafe, so we'll rely on the input task.
#endif

    if (battery_level < 0.f)
        return false;

    if (percent)
        *percent = battery_level;

    if (volts)
        *volts = battery_volts;

    return true;
}
