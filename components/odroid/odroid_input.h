#pragma once

#include <stdbool.h>
#include <stdint.h>

enum
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

    ODROID_INPUT_ANY = 0xF0,
    ODROID_INPUT_ALL = 0xF1,
};

typedef struct
{
    uint8_t values[ODROID_INPUT_MAX];
    uint16_t bitmask;
} odroid_gamepad_state;

typedef struct
{
	int millivolts;
	int percentage;
} odroid_battery_state;

void odroid_input_gamepad_init();
void odroid_input_gamepad_terminate();
long odroid_input_gamepad_last_polled();
void odroid_input_gamepad_read(odroid_gamepad_state* out_state);
bool odroid_input_key_is_pressed(int key);
void odroid_input_wait_for_key(int key, bool pressed);

odroid_gamepad_state odroid_input_gamepad_read_raw();
odroid_battery_state odroid_input_battery_read();
