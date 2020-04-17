#pragma once

#include <stdint.h>


#define ODROID_GAMEPAD_IO_X ADC1_CHANNEL_6
#define ODROID_GAMEPAD_IO_Y ADC1_CHANNEL_7
#define ODROID_GAMEPAD_IO_SELECT GPIO_NUM_27
#define ODROID_GAMEPAD_IO_START GPIO_NUM_39
#define ODROID_GAMEPAD_IO_A GPIO_NUM_32
#define ODROID_GAMEPAD_IO_B GPIO_NUM_33
#define ODROID_GAMEPAD_IO_MENU GPIO_NUM_13
#define ODROID_GAMEPAD_IO_VOLUME GPIO_NUM_0


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

	ODROID_INPUT_MAX
};

typedef struct
{
    uint8_t values[ODROID_INPUT_MAX];
} odroid_gamepad_state;

typedef struct
{
	int millivolts;
	int percentage;
} odroid_battery_state;

void odroid_input_gamepad_init();
void odroid_input_gamepad_terminate();
void odroid_input_gamepad_read(odroid_gamepad_state* out_state);
odroid_gamepad_state odroid_input_read_raw();

void odroid_input_battery_level_init();
void odroid_input_battery_level_read(odroid_battery_state* out_state);
void odroid_input_battery_level_force_voltage(float volts);
void odroid_input_battery_monitor_enabled_set(int value);

int32_t odroid_Backlight_get();
void odroid_Backlight_set(int32_t value);
