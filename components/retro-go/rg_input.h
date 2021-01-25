#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	GAMEPAD_KEY_UP = 0,
    GAMEPAD_KEY_RIGHT,
    GAMEPAD_KEY_DOWN,
    GAMEPAD_KEY_LEFT,
    GAMEPAD_KEY_SELECT,
    GAMEPAD_KEY_START,
    GAMEPAD_KEY_A,
    GAMEPAD_KEY_B,
    GAMEPAD_KEY_MENU,
    GAMEPAD_KEY_VOLUME,

	GAMEPAD_KEY_MAX,

    GAMEPAD_KEY_ANY,
} gamepad_key_t;

typedef struct
{
    uint8_t values[GAMEPAD_KEY_MAX];
    uint32_t bitmask;
    // uint32_t changed;
} gamepad_state_t;

typedef struct
{
	int millivolts;
	int percentage;
} battery_state_t;

void rg_input_init(void);
void rg_input_deinit(void);
long rg_input_gamepad_last_read(void);
bool rg_input_key_is_pressed(gamepad_key_t key);
void rg_input_wait_for_key(gamepad_key_t key, bool pressed);
gamepad_state_t rg_input_read_gamepad(void);
battery_state_t rg_input_read_battery(void);
