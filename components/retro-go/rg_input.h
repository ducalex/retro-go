#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_KEY_UP      = (1 << 0),
    RG_KEY_RIGHT   = (1 << 1),
    RG_KEY_DOWN    = (1 << 2),
    RG_KEY_LEFT    = (1 << 3),
    RG_KEY_SELECT  = (1 << 4),
    RG_KEY_START   = (1 << 5),
    RG_KEY_MENU    = (1 << 6),
    RG_KEY_OPTION  = (1 << 7),
    RG_KEY_A       = (1 << 8),
    RG_KEY_B       = (1 << 9),
    RG_KEY_X       = (1 << 10),
    RG_KEY_Y       = (1 << 11),
    RG_KEY_L       = (1 << 12),
    RG_KEY_R       = (1 << 13),
    RG_KEY_COUNT   = 14,
    RG_KEY_ANY     = 0xFFFF,
    RG_KEY_ALL     = 0xFFFF,
    RG_KEY_NONE    = 0,
} rg_key_t;

// #define RG_GAMEPAD_ADC_MAP {{}, ...} to use ADC driver
typedef struct
{
    rg_key_t key;
    int unit;   // adc_unit_t
    int channel;// adc_channel_t
    int atten;  // adc_atten_t
    int min, max;
} rg_keymap_adc_t;

// #define RG_GAMEPAD_GPIO_MAP {{}, ...} to use GPIO driver
typedef struct
{
    rg_key_t key;
    int num;    // gpio_num_t
    int pull;   // gpio_pull_mode_t
    int level;  // 0-1
} rg_keymap_gpio_t;

// #define RG_GAMEPAD_I2C_MAP {{}, ...} to use I2C driver
typedef struct
{
    rg_key_t key;
    uint32_t src;
} rg_keymap_i2c_t;

// #define RG_GAMEPAD_KBD_MAP {{}, ...} for Keyboard driver
typedef struct
{
    rg_key_t key;
    uint32_t src;
} rg_keymap_kbd_t;

// #define RG_GAMEPAD_SERIAL_MAP {{}, ...} to use Serial (74164, SNES, etc) driver
typedef struct
{
    rg_key_t key;
    uint32_t src;
} rg_keymap_serial_t;

// #define RG_GAMEPAD_VIRT_MAP {{}, ...} to add virtual buttons (eg start+select = menu)
typedef struct
{
    rg_key_t key;
    uint32_t src;
} rg_keymap_virt_t;

// FIXME: Create a single unified keymap...
// ...

typedef struct
{
    float level;
    float volts;
    bool present;
    bool charging;
} rg_battery_t;

typedef struct
{
    size_t columns, rows;
    char data[];
} rg_keyboard_map_t;

void rg_input_init(void);
void rg_input_deinit(void);
bool rg_input_key_is_pressed(rg_key_t mask);
bool rg_input_wait_for_key(rg_key_t mask, bool pressed, int timeout_ms);
const char *rg_input_get_key_name(rg_key_t key);
const char *rg_input_get_key_mapping(rg_key_t key);
uint32_t rg_input_read_gamepad(void);
int rg_input_read_keyboard(const rg_keyboard_map_t *map);
rg_battery_t rg_input_read_battery(void);
bool rg_input_read_gamepad_raw(uint32_t *out);
bool rg_input_read_keyboard_raw(int *out);
bool rg_input_read_battery_raw(rg_battery_t *out);
