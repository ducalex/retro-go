#include "rg_system.h"
#include "rg_input.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef RG_TARGET_SDL2
#include <SDL2/SDL.h>
#else
#include <driver/gpio.h>
#endif

#if RG_GAMEPAD_DRIVER == 1 || defined(RG_BATTERY_ADC_CHANNEL)
#include <esp_adc_cal.h>
#include <driver/adc.h>
#endif

static bool input_task_running = false;
static uint32_t gamepad_state = -1; // _Atomic
static int battery_level = -1;
#if defined(RG_BATTERY_ADC_CHANNEL)
static esp_adc_cal_characteristics_t adc_chars;
#endif


static inline int battery_read(void)
{
#if defined(RG_BATTERY_ADC_CHANNEL)

    uint32_t adc_sample = 0;
    for (int i = 0; i < 4; ++i)
        adc_sample += esp_adc_cal_raw_to_voltage(adc1_get_raw(RG_BATTERY_ADC_CHANNEL), &adc_chars);
    return adc_sample / 4;

#elif RG_GAMEPAD_DRIVER == 3 /* I2C */

    uint8_t data[5];
    if (rg_i2c_read(0x20, -1, &data, 5))
        return data[4];
    return -1;

#else
    // No battery or unknown
    return -1;
#endif
}

static inline uint32_t gamepad_read(void)
{
    uint32_t state = 0;

#if RG_GAMEPAD_DRIVER == 1    // GPIO

    int joyX = adc1_get_raw(RG_GPIO_GAMEPAD_X);
    int joyY = adc1_get_raw(RG_GPIO_GAMEPAD_Y);

    if (!gpio_get_level(RG_GPIO_GAMEPAD_MENU))   state |= RG_KEY_MENU;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_OPTION)) state |= RG_KEY_OPTION;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_SELECT)) state |= RG_KEY_SELECT;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_START))  state |= RG_KEY_START;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_A))      state |= RG_KEY_A;
    if (!gpio_get_level(RG_GPIO_GAMEPAD_B))      state |= RG_KEY_B;

    #ifndef RG_TARGET_RETRO_ESP32
        if (joyY > 2048 + 1024) state |= RG_KEY_UP;
        else if (joyY > 1024)   state |= RG_KEY_DOWN;
        if (joyX > 2048 + 1024) state |= RG_KEY_LEFT;
        else if (joyX > 1024)   state |= RG_KEY_RIGHT;
    #else
        if (joyY > 2048) state |= RG_KEY_UP;
        else if (joyY > 1024) state |= RG_KEY_DOWN;
        if (joyX > 2048) state |= RG_KEY_LEFT;
        else if (joyX > 1024) state |= RG_KEY_RIGHT;
        if (state == (RG_KEY_SELECT|RG_KEY_A))
            state = RG_KEY_OPTION;
        if (state == (RG_KEY_START|RG_KEY_SELECT))
            state = RG_KEY_MENU;
    #endif

#elif RG_GAMEPAD_DRIVER == 2  // Serial

    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 0);
    usleep(5);
    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 1);
    usleep(1);

    uint32_t buttons = 0;
    for (int i = 0; i < 16; i++)
    {
        buttons |= gpio_get_level(RG_GPIO_GAMEPAD_DATA) << (15 - i);
        gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 0);
        usleep(1);
        gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 1);
        usleep(1);
    }
    if (buttons & RG_GAMEPAD_MAP_MENU) state |= RG_KEY_MENU;
    if (buttons & RG_GAMEPAD_MAP_OPTION) state |= RG_KEY_OPTION;
    if (buttons & RG_GAMEPAD_MAP_START) state |= RG_KEY_START;
    if (buttons & RG_GAMEPAD_MAP_SELECT) state |= RG_KEY_SELECT;
    if (buttons & RG_GAMEPAD_MAP_UP) state |= RG_KEY_UP;
    if (buttons & RG_GAMEPAD_MAP_RIGHT) state |= RG_KEY_RIGHT;
    if (buttons & RG_GAMEPAD_MAP_DOWN) state |= RG_KEY_DOWN;
    if (buttons & RG_GAMEPAD_MAP_LEFT) state |= RG_KEY_LEFT;
    if (buttons & RG_GAMEPAD_MAP_A) state |= RG_KEY_A;
    if (buttons & RG_GAMEPAD_MAP_B) state |= RG_KEY_B;
    if (buttons & RG_GAMEPAD_MAP_X) state |= RG_KEY_X;
    if (buttons & RG_GAMEPAD_MAP_Y) state |= RG_KEY_Y;
    if (buttons & RG_GAMEPAD_MAP_L) state |= RG_KEY_L;
    if (buttons & RG_GAMEPAD_MAP_R) state |= RG_KEY_R;

#elif RG_GAMEPAD_DRIVER == 3  // I2C

    uint8_t data[5];
    if (rg_i2c_read(0x20, -1, &data, 5))
    {
        uint32_t buttons = ~((data[2] << 8) | data[1]);

        if (buttons & RG_GAMEPAD_MAP_MENU) state |= RG_KEY_MENU;
        if (buttons & RG_GAMEPAD_MAP_OPTION) state |= RG_KEY_OPTION;
        if (buttons & RG_GAMEPAD_MAP_START) state |= RG_KEY_START;
        if (buttons & RG_GAMEPAD_MAP_SELECT) state |= RG_KEY_SELECT;
        if (buttons & RG_GAMEPAD_MAP_UP) state |= RG_KEY_UP;
        if (buttons & RG_GAMEPAD_MAP_RIGHT) state |= RG_KEY_RIGHT;
        if (buttons & RG_GAMEPAD_MAP_DOWN) state |= RG_KEY_DOWN;
        if (buttons & RG_GAMEPAD_MAP_LEFT) state |= RG_KEY_LEFT;
        if (buttons & RG_GAMEPAD_MAP_A) state |= RG_KEY_A;
        if (buttons & RG_GAMEPAD_MAP_B) state |= RG_KEY_B;
        if (buttons & RG_GAMEPAD_MAP_X) state |= RG_KEY_X;
        if (buttons & RG_GAMEPAD_MAP_Y) state |= RG_KEY_Y;
        if (buttons & RG_GAMEPAD_MAP_L) state |= RG_KEY_L;
        if (buttons & RG_GAMEPAD_MAP_R) state |= RG_KEY_R;
    }

#elif RG_GAMEPAD_DRIVER == 4  // I2C via AW9523

    uint32_t buttons = ~(rg_i2c_gpio_read_port(0) | rg_i2c_gpio_read_port(1) << 8);

    if (buttons & RG_GAMEPAD_MAP_MENU) state |= RG_KEY_MENU;
    if (buttons & RG_GAMEPAD_MAP_OPTION) state |= RG_KEY_OPTION;
    if (buttons & RG_GAMEPAD_MAP_START) state |= RG_KEY_START;
    if (buttons & RG_GAMEPAD_MAP_SELECT) state |= RG_KEY_SELECT;
    if (buttons & RG_GAMEPAD_MAP_UP) state |= RG_KEY_UP;
    if (buttons & RG_GAMEPAD_MAP_RIGHT) state |= RG_KEY_RIGHT;
    if (buttons & RG_GAMEPAD_MAP_DOWN) state |= RG_KEY_DOWN;
    if (buttons & RG_GAMEPAD_MAP_LEFT) state |= RG_KEY_LEFT;
    if (buttons & RG_GAMEPAD_MAP_A) state |= RG_KEY_A;
    if (buttons & RG_GAMEPAD_MAP_B) state |= RG_KEY_B;
    if (buttons & RG_GAMEPAD_MAP_X) state |= RG_KEY_X;
    if (buttons & RG_GAMEPAD_MAP_Y) state |= RG_KEY_Y;
    if (buttons & RG_GAMEPAD_MAP_L) state |= RG_KEY_L;
    if (buttons & RG_GAMEPAD_MAP_R) state |= RG_KEY_R;

#elif RG_GAMEPAD_DRIVER == 6

    const uint8_t *keys = SDL_GetKeyboardState(NULL);

    if (keys[RG_GAMEPAD_MAP_MENU]) state |= RG_KEY_MENU;
    if (keys[RG_GAMEPAD_MAP_OPTION]) state |= RG_KEY_OPTION;
    if (keys[RG_GAMEPAD_MAP_START]) state |= RG_KEY_START;
    if (keys[RG_GAMEPAD_MAP_SELECT]) state |= RG_KEY_SELECT;
    if (keys[RG_GAMEPAD_MAP_UP]) state |= RG_KEY_UP;
    if (keys[RG_GAMEPAD_MAP_LEFT]) state |= RG_KEY_LEFT;
    if (keys[RG_GAMEPAD_MAP_DOWN]) state |= RG_KEY_DOWN;
    if (keys[RG_GAMEPAD_MAP_RIGHT]) state |= RG_KEY_RIGHT;
    if (keys[RG_GAMEPAD_MAP_A]) state |= RG_KEY_A;
    if (keys[RG_GAMEPAD_MAP_B]) state |= RG_KEY_B;
    if (keys[RG_GAMEPAD_MAP_X]) state |= RG_KEY_X;
    if (keys[RG_GAMEPAD_MAP_Y]) state |= RG_KEY_Y;
    if (keys[RG_GAMEPAD_MAP_L]) state |= RG_KEY_L;
    if (keys[RG_GAMEPAD_MAP_R]) state |= RG_KEY_R;

#endif

    // Virtual buttons (combos) to replace essential missing buttons.
#if !RG_GAMEPAD_HAS_MENU_BTN
    state &= ~RG_KEY_MENU;
    if (state == (RG_KEY_START|RG_KEY_SELECT))
        state = RG_KEY_MENU;
#endif

    return state;
}

static void input_task(void *arg)
{
    const uint8_t debounce_level = 0x03;
    uint8_t debounce[RG_KEY_COUNT];
    uint32_t local_gamepad_state = 0;
    uint32_t loop_count = 0;

    memset(debounce, debounce_level, sizeof(debounce));
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
                local_gamepad_state |= (1 << i);
            }
            else if (debounce[i] == 0x00) // Released
            {
                local_gamepad_state &= ~(1 << i);
            }
        }

        gamepad_state = local_gamepad_state;

        if ((loop_count % 100) == 0)
        {
            int level = battery_read();
            if (level > 0 && battery_level > 0)
                battery_level = (battery_level + level) / 2;
            else
                battery_level = level;
        }

        rg_task_delay(10);
        loop_count++;
    }

    input_task_running = false;
    gamepad_state = -1;
    rg_task_delete(NULL);
}

void rg_input_init(void)
{
    if (input_task_running)
        return;

#if RG_GAMEPAD_DRIVER == 1 // GPIO

    const char *driver = "GPIO";

    adc1_config_width(ADC_WIDTH_MAX - 1);
    adc1_config_channel_atten(RG_GPIO_GAMEPAD_X, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(RG_GPIO_GAMEPAD_Y, ADC_ATTEN_DB_11);

    gpio_set_direction(RG_GPIO_GAMEPAD_MENU, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_MENU, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_OPTION, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(RG_GPIO_GAMEPAD_OPTION, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_SELECT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_SELECT, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_START, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(RG_GPIO_GAMEPAD_START, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_A, GPIO_PULLUP_ONLY);
    gpio_set_direction(RG_GPIO_GAMEPAD_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RG_GPIO_GAMEPAD_B, GPIO_PULLUP_ONLY);

#elif RG_GAMEPAD_DRIVER == 2 // Serial

    const char *driver = "SERIAL";

    gpio_set_direction(RG_GPIO_GAMEPAD_CLOCK, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_LATCH, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_DATA, GPIO_MODE_INPUT);

    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 0);
    gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 1);

#elif RG_GAMEPAD_DRIVER == 3 // I2C

    const char *driver = "I2C";

    rg_i2c_init();
    gamepad_read(); // First read contains garbage

#elif RG_GAMEPAD_DRIVER == 4 // I2C w/AW9523

    const char *driver = "AW9523-I2C";

    rg_i2c_gpio_init();

    // All that below should be moved elsewhere, and possibly specific to the qtpy...
    rg_i2c_gpio_set_direction(AW_TFT_RESET, 0);
    rg_i2c_gpio_set_direction(AW_TFT_BACKLIGHT, 0);
    rg_i2c_gpio_set_direction(AW_HEADPHONE_EN, 0);

    rg_i2c_gpio_set_level(AW_TFT_BACKLIGHT, 1);
    rg_i2c_gpio_set_level(AW_HEADPHONE_EN, 1);

    // tft reset
    rg_i2c_gpio_set_level(AW_TFT_RESET, 0);
    usleep(10 * 1000);
    rg_i2c_gpio_set_level(AW_TFT_RESET, 1);
    usleep(10 * 1000);

#elif RG_GAMEPAD_DRIVER == 6 // SDL2

    const char *driver = "SDL2";

#else

    #error "No gamepad driver selected"

#endif

#if defined(RG_BATTERY_ADC_CHANNEL)
    adc1_config_width(ADC_WIDTH_MAX - 1);
    adc1_config_channel_atten(RG_BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_MAX - 1, 1100, &adc_chars);
#endif

    // Start background polling
    rg_task_create("rg_input", &input_task, NULL, 2 * 1024, RG_TASK_PRIORITY - 1, 1);
    while (gamepad_state == -1)
        rg_task_delay(1);
    RG_LOGI("Input ready. driver='%s', state=" PRINTF_BINARY_16 "\n", driver, PRINTF_BINVAL_16(gamepad_state));
}

void rg_input_deinit(void)
{
    input_task_running = false;
    // while (gamepad_state != -1)
    //     rg_task_delay(1);
    RG_LOGI("Input terminated.\n");
}

uint32_t rg_input_read_gamepad(void)
{
#ifdef RG_TARGET_SDL2
    SDL_PumpEvents();
#endif
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
        rg_task_delay(10);
        rg_system_tick(0);
    }
}

bool rg_input_read_battery(float *percent, float *volts)
{
    if (battery_level < 0) // No battery or error?
        return false;

    if (percent)
    {
        *percent = RG_BATTERY_CALC_PERCENT(battery_level);
        *percent = RG_MAX(0.f, RG_MIN(100.f, *percent));
    }

    if (volts)
        *volts = RG_BATTERY_CALC_VOLTAGE(battery_level);

    return true;
}

const char *rg_input_get_key_name(rg_key_t key)
{
    switch (key)
    {
    case RG_KEY_UP: return "Up";
    case RG_KEY_RIGHT: return "Right";
    case RG_KEY_DOWN: return "Down";
    case RG_KEY_LEFT: return "Left";
    case RG_KEY_SELECT: return "Select";
    case RG_KEY_START: return "Start";
    case RG_KEY_MENU: return "Menu";
    case RG_KEY_OPTION: return "Option";
    case RG_KEY_A: return "A";
    case RG_KEY_B: return "B";
    case RG_KEY_X: return "X";
    case RG_KEY_Y: return "Y";
    case RG_KEY_L: return "Left Shoulder";
    case RG_KEY_R: return "Right Shoulder";
    case RG_KEY_NONE: return "None";
    default: return "Unknown";
    }
}