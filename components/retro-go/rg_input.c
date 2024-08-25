#include "rg_system.h"
#include "rg_input.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <driver/adc.h>
#else
#include <SDL2/SDL.h>
#endif

#if RG_BATTERY_DRIVER == 1
#include <esp_adc_cal.h>
static esp_adc_cal_characteristics_t adc_chars;
#endif

#ifdef RG_GAMEPAD_KBD_MAP
static rg_keymap_kbd_t keymap_kbd[] = RG_GAMEPAD_KBD_MAP;
#endif
#ifdef RG_GAMEPAD_VIRT_MAP
static rg_keymap_virt_t keymap_virt[] = RG_GAMEPAD_VIRT_MAP;
#endif
static bool input_task_running = false;
static uint32_t gamepad_state = -1; // _Atomic
static rg_battery_t battery_state = {0};

static const rg_input_driver_t *input_drivers[4];
static size_t input_drivers_count = 0;

extern const rg_input_driver_t RG_DRIVER_INPUT_ADC;
extern const rg_input_driver_t RG_DRIVER_INPUT_GPIO;
extern const rg_input_driver_t RG_DRIVER_INPUT_I2C;

#define GPIO &RG_DRIVER_INPUT_GPIO, 0
#define ADC1 &RG_DRIVER_INPUT_ADC, ADC_UNIT_1
#define ADC2 &RG_DRIVER_INPUT_ADC, ADC_UNIT_2
#define I2C  &RG_DRIVER_INPUT_I2C, 0x20

static const rg_keymap_desc_t keymap_config[] = RG_GAMEPAD_MAP;
static rg_keymap_t keymap[RG_COUNT(keymap_config) + 8];
static size_t keymap_count;

static size_t register_driver(const rg_input_driver_t *driver)
{
    for (size_t i = 0; i < input_drivers_count; ++i)
    {
        if (input_drivers[i] == driver)
            return i;
    }
    RG_LOGI("Initializing input driver: %s...", driver->name);
    driver->init();
    input_drivers[input_drivers_count++] = driver;
    return input_drivers_count - 1;
}

bool rg_input_read_battery_raw(rg_battery_t *out)
{
    uint32_t raw_value = 0;
    bool present = true;
    bool charging = false;

#if RG_BATTERY_DRIVER == 1 /* ADC */
    for (int i = 0; i < 4; ++i)
    {
        int value;
        if (!RG_DRIVER_INPUT_ADC.read(RG_BATTERY_ADC_UNIT, RG_BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_11, 0, 0, &value))
            return false;
        raw_value += esp_adc_cal_raw_to_voltage(value, &adc_chars);
    }
    raw_value /= 4;
#elif RG_BATTERY_DRIVER == 2 /* I2C */
    if (!RG_DRIVER_INPUT_I2C.read(0x20, 0xFF, 24, 0, 0, &raw_value))
        return false;
    charging = raw_value == 255;
#else
    return false;
#endif

    if (!out)
        return true;

    *out = (rg_battery_t){
        .level = RG_MAX(0.f, RG_MIN(100.f, RG_BATTERY_CALC_PERCENT(raw_value))),
        .volts = RG_BATTERY_CALC_VOLTAGE(raw_value),
        .present = present,
        .charging = charging,
    };
    return true;
}

bool rg_input_read_gamepad_raw(uint32_t *out)
{
    int values[input_drivers_count][16];
    uint32_t state = 0;

    for (size_t i = 0; i < input_drivers_count; ++i)
    {
        if (!input_drivers[i]->read_inputs(0, 16, &values[i]))
            memset(values[i], 0xAF, sizeof(values[i]));
    }

    for (size_t i = 0; i < keymap_count; ++i)
    {
        rg_keymap_t *mapping = &keymap[i];
        int value = values[mapping->driver_index][mapping->input_index];
        if (value >= mapping->min && value <= mapping->max)
            state |= mapping->key;
    }


#if defined(RG_GAMEPAD_KBD_MAP)
#ifdef RG_TARGET_SDL2
    int numkeys = 0;
    const uint8_t *keys = SDL_GetKeyboardState(&numkeys);
    for (size_t i = 0; i < RG_COUNT(keymap_kbd); ++i)
    {
        const rg_keymap_kbd_t *mapping = &keymap_kbd[i];
        if (mapping->src < 0 || mapping->src >= numkeys)
            continue;
        if (keys[mapping->src])
            state |= mapping->key;
    }
#else
#warning "not implemented"
#endif
#endif

#if defined(RG_GAMEPAD_VIRT_MAP)
    for (size_t i = 0; i < RG_COUNT(keymap_virt); ++i)
    {
        if (state == keymap_virt[i].src)
            state = keymap_virt[i].key;
    }
#endif

    if (out)
        *out = state;
    return true;
}

static void input_task(void *arg)
{
    const uint8_t debounce_level = 0x03;
    uint8_t debounce[RG_KEY_COUNT];
    uint32_t local_gamepad_state = 0;
    uint32_t state;
    int64_t next_battery_update = 0;

    memset(debounce, debounce_level, sizeof(debounce));
    input_task_running = true;

    while (input_task_running)
    {
        if (rg_input_read_gamepad_raw(&state))
        {
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
        }

        if (rg_system_timer() >= next_battery_update)
        {
            rg_battery_t temp = {0};
            if (rg_input_read_battery_raw(&temp))
            {
                if (fabsf(battery_state.level - temp.level) < RG_BATTERY_UPDATE_THRESHOLD)
                    temp.level = battery_state.level;
                if (fabsf(battery_state.volts - temp.volts) < RG_BATTERY_UPDATE_THRESHOLD_VOLT)
                    temp.volts = battery_state.volts;
            }
            battery_state = temp;
            next_battery_update = rg_system_timer() + 2 * 1000000; // update every 2 seconds
        }

        rg_task_delay(10);
    }

    input_task_running = false;
    gamepad_state = -1;
}

void rg_input_init(void)
{
    RG_ASSERT(!input_task_running, "Input already initialized!");

    for (size_t i = 0; i < RG_COUNT(keymap_config); ++i)
    {
        const rg_keymap_desc_t *map = &keymap_config[i];
        size_t driver_index = register_driver(map->driver);
        size_t input_index;
        if (map->driver->add_input(map->port, map->channel, map->arg1, map->arg2, map->arg3, &input_index))
        {
            keymap[keymap_count++] = (rg_keymap_t){
                .key = map->key,
                .driver_index = driver_index,
                .input_index = input_index,
                .min = map->min,
                .max = map->max,
            };
        }
    }


#if RG_BATTERY_DRIVER == 1 /* ADC */
    register_driver(&RG_DRIVER_INPUT_ADC);
    esp_adc_cal_characterize(RG_BATTERY_ADC_UNIT, ADC_ATTEN_DB_11, ADC_WIDTH_MAX - 1, 1100, &adc_chars);
#endif

    // The first read returns bogus data in some drivers, waste it.
    rg_input_read_gamepad_raw(NULL);

    // Start background polling
    rg_task_create("rg_input", &input_task, NULL, 3 * 1024, RG_TASK_PRIORITY_6, 1);
    while (gamepad_state == -1)
        rg_task_yield();
    RG_LOGI("Input ready. state=" PRINTF_BINARY_16 "\n", PRINTF_BINVAL_16(gamepad_state));
}

void rg_input_deinit(void)
{
    input_task_running = false;
    // while (gamepad_state != -1)
    //     rg_task_yield();
    while (input_drivers_count > 0)
        input_drivers[--input_drivers_count]->deinit();
    RG_LOGI("Input terminated.");
}

uint32_t rg_input_read_gamepad(void)
{
#ifdef RG_TARGET_SDL2
    SDL_PumpEvents();
#endif
    return gamepad_state;
}

bool rg_input_key_is_pressed(rg_key_t mask)
{
    return (bool)(rg_input_read_gamepad() & mask);
}

bool rg_input_wait_for_key(rg_key_t mask, bool pressed, int timeout_ms)
{
    int64_t expiration = timeout_ms < 0 ? INT64_MAX : (rg_system_timer() + timeout_ms * 1000);
    while (rg_input_key_is_pressed(mask) != pressed)
    {
        if (rg_system_timer() > expiration)
            return false;
        rg_task_delay(10);
    }
    return true;
}

rg_battery_t rg_input_read_battery(void)
{
    return battery_state;
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

const char *rg_input_get_key_mapping(rg_key_t key)
{
    for (size_t i = 0; i < keymap_count; ++i)
    {
        if (keymap[i].key == key)
            return input_drivers[keymap[i].driver_index]->name;
    }
    return NULL;
}

const rg_keyboard_map_t virtual_map1 = {
    .columns = 10,
    .rows = 4,
    .data = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ',', '.', ' ',
    }
};

int rg_input_read_keyboard(const rg_keyboard_map_t *map)
{
    int cursor = -1;
    int count = map->columns * map->rows;

    if (!map)
        map = &virtual_map1;

    rg_input_wait_for_key(RG_KEY_ALL, false, 1000);

    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();
        int prev_cursor = cursor;

        if (joystick & RG_KEY_A)
            return map->data[cursor];
        if (joystick & RG_KEY_B)
            break;

        if (joystick & RG_KEY_LEFT)
            cursor--;
        if (joystick & RG_KEY_RIGHT)
            cursor++;
        if (joystick & RG_KEY_UP)
            cursor -= map->columns;
        if (joystick & RG_KEY_DOWN)
            cursor += map->columns;

        if (cursor > count - 1)
            cursor = prev_cursor;
        else if (cursor < 0)
            cursor = prev_cursor;

        cursor = RG_MIN(RG_MAX(cursor, 0), count - 1);

        if (cursor != prev_cursor)
            rg_gui_draw_keyboard(map, cursor);

        rg_input_wait_for_key(RG_KEY_ALL, false, 500);
        rg_input_wait_for_key(RG_KEY_ANY, true, 500);

        rg_system_tick(0);
    }

    return -1;
}
