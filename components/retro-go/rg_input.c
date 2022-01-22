#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc_cal.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <string.h>

#include "rg_system.h"
#include "rg_input.h"
#include "rg_i2c.h"

static bool input_task_running = false;
static int64_t last_gamepad_read = 0;
// Keep the following <= 32bit to ensure atomic access without mutexes
static uint32_t gamepad_state = -1;
static float battery_level = -1;
static float battery_volts = 0;

bool aw_digitalWrite(uint8_t pin, bool value) {
  uint16_t pins;
  uint8_t c;
  rg_i2c_read(AW9523_DEFAULT_ADDR, AW9523_REG_OUTPUT0+1, &c, 1);
  pins = c;
  pins <<= 8;
  rg_i2c_read(AW9523_DEFAULT_ADDR, AW9523_REG_OUTPUT0, &c, 1);
  pins |= c;

  if (value) {
    pins |= 1UL << pin;
  } else {
    pins &= ~(1UL << pin);
  }
  c = pins & 0xFF;
  rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_OUTPUT0, &c, 1);
  c = pins >> 8;
  return rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_OUTPUT0+1, &c, 1);
}

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

#elif RG_GAMEPAD_DRIVER == 4  // I2C via AW9523
    state = 0;
    battery_level = 69;

    uint16_t aw_buttons = 0;
    uint8_t aw_data;
    rg_i2c_read(AW9523_DEFAULT_ADDR, AW9523_REG_INPUT0+1, &aw_data, 1);
    aw_buttons = aw_data;
    aw_buttons <<= 8;
    rg_i2c_read(AW9523_DEFAULT_ADDR, AW9523_REG_INPUT0, &aw_data, 1);
    aw_buttons |= aw_data;
    aw_buttons = ~aw_buttons;

    if (aw_buttons & (1<<AW_GAMEPAD_IO_UP)) state |= RG_KEY_UP;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_DOWN)) state |= RG_KEY_DOWN;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_LEFT)) state |= RG_KEY_LEFT;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_RIGHT)) state |= RG_KEY_RIGHT;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_A)) state |= RG_KEY_A;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_B)) state |= RG_KEY_B;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_SELECT)) state |= RG_KEY_SELECT;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_START)) state |= RG_KEY_START;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_MENU)) state |= RG_KEY_MENU;
    if (aw_buttons & (1<<AW_GAMEPAD_IO_OPTION)) state |= RG_KEY_OPTION;

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
    input_task_running = true;

    while (input_task_running)
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

    input_task_running = false;
    gamepad_state = -1;
    vTaskDelete(NULL);
}

void rg_input_init(void)
{
    if (input_task_running)
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

#elif RG_GAMEPAD_DRIVER == 4  // I2C w/AW9523

    const char *driver = "QTPY-AW9523";
    rg_i2c_init();

    // soft reset
    RG_LOGI("AW9523 Reset\n");
    uint8_t val;

    val = 0;
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_SOFTRESET, &val, 1);
    // check id?
    uint8_t id=0;
    rg_i2c_read(AW9523_DEFAULT_ADDR, AW9523_REG_CHIPID, &id, 1);
    RG_LOGI("AW9523 ID code 0x%x found\n", id);
    assert(id == 0x23);

    // set gpio to input!
    uint16_t buttonmask = (1<<AW_GAMEPAD_IO_UP) | (1<<AW_GAMEPAD_IO_DOWN) |
      (1<<AW_GAMEPAD_IO_LEFT) | (1<<AW_GAMEPAD_IO_RIGHT) | (1<<AW_GAMEPAD_IO_SELECT) |
      (1<<AW_GAMEPAD_IO_START) | (1<<AW_GAMEPAD_IO_A) | (1<<AW_GAMEPAD_IO_B) |
      (1<<AW_GAMEPAD_IO_MENU) | (1<<AW_GAMEPAD_IO_OPTION) | (1<<AW_CARDDET);
    val = buttonmask & 0xFF;
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_CONFIG0, &val, 1);
    val = buttonmask >> 8;
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_CONFIG0+1, &val, 1);

    val = 0xFF;
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_LEDMODE, &val, 1);
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_LEDMODE+1, &val, 1);
    
    // pushpull mode
    val = 1<<4;
    rg_i2c_write(AW9523_DEFAULT_ADDR, AW9523_REG_GCR, &val, 1);

    // turn on backlight!
    aw_digitalWrite(AW_TFT_BACKLIGHT, 1);

    // tft reset
    aw_digitalWrite(AW_TFT_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    aw_digitalWrite(AW_TFT_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // This will initialize i2c and discard the first read (garbage)
    gamepad_read();

#else
    #error "No gamepad driver selected"

#endif

    // Start background polling
    xTaskCreatePinnedToCore(&input_task, "input_task", 1024, NULL, 5, NULL, 1);
    // while (gamepad_state == -1) vPortYield();
    RG_LOGI("Input ready. driver='%s'\n", driver);
}

void rg_input_deinit(void)
{
    input_task_running = false;
    // while (gamepad_state == -1) vPortYield();
    RG_LOGI("Input terminated.\n");
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
