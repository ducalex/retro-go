#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    GAMEPAD_KEY_UP      = (1 << 0),
    GAMEPAD_KEY_RIGHT   = (1 << 1),
    GAMEPAD_KEY_DOWN    = (1 << 2),
    GAMEPAD_KEY_LEFT    = (1 << 3),
    GAMEPAD_KEY_SELECT  = (1 << 4),
    GAMEPAD_KEY_START   = (1 << 5),
    GAMEPAD_KEY_A       = (1 << 6),
    GAMEPAD_KEY_B       = (1 << 7),
    GAMEPAD_KEY_MENU    = (1 << 8),
    GAMEPAD_KEY_VOLUME  = (1 << 9),
    GAMEPAD_KEY_ANY     = 0xFFFF,
    GAMEPAD_KEY_ALL     = 0xFFFF,
} gamepad_key_t;

#define GAMEPAD_KEY_COUNT 16

typedef uint32_t gamepad_state_t;

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
