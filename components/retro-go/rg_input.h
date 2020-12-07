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

typedef union {
    struct {
        uint16_t btn_up: 1;
        uint16_t btn_right: 1;
        uint16_t btn_down: 1;
        uint16_t btn_left: 1;
        uint16_t btn_select: 1;
        uint16_t btn_start: 1;
        uint16_t btn_menu: 1;
        uint16_t btn_volume: 1;
        uint16_t btn_a: 1;
        uint16_t btn_b: 1;
        uint16_t btn_x: 1;
        uint16_t btn_y: 1;
        uint16_t btn_1: 1;
        uint16_t btn_2: 1;
        uint16_t btn_3: 1;
        uint16_t btn_4: 1;
    };
    uint16_t bitmask;
} odroid_gamepad_state2_t;

typedef struct
{
	int millivolts;
	int percentage;
} odroid_battery_state_t;

void odroid_input_init(void);
void odroid_input_terminate(void);
long odroid_input_gamepad_last_read(void);
bool odroid_input_key_is_pressed(odroid_gamepad_key_t key);
void odroid_input_wait_for_key(odroid_gamepad_key_t key, bool pressed);
odroid_gamepad_state_t odroid_input_read_gamepad(void);
odroid_battery_state_t odroid_input_read_battery(void);
