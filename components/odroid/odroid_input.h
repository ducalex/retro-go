#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	ODROID_INPUT_UP = 0,
    ODROID_INPUT_RIGHT,
    ODROID_INPUT_DOWN,
    ODROID_INPUT_LEFT,
    ODROID_INPUT_SELECT,
    ODROID_INPUT_START,
    ODROID_INPUT_A,
    ODROID_INPUT_B,
    ODROID_INPUT_MENU,
    ODROID_INPUT_VOLUME,

	ODROID_INPUT_MAX,

    ODROID_INPUT_ANY,
} odroid_gamepad_key_t;

typedef struct
{
    uint8_t values[ODROID_INPUT_MAX];
    uint16_t bitmask;
} odroid_gamepad_state_t;

typedef struct
{
	int millivolts;
	int percentage;
} odroid_battery_state_t;

void odroid_input_init();
void odroid_input_terminate();
long odroid_input_gamepad_last_read();
bool odroid_input_key_is_pressed(odroid_gamepad_key_t key);
void odroid_input_wait_for_key(odroid_gamepad_key_t key, bool pressed);
void odroid_input_read_gamepad(odroid_gamepad_state_t* out_state);
odroid_gamepad_state_t odroid_input_read_gamepad_();
odroid_battery_state_t odroid_input_read_battery();
