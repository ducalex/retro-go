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

void rg_input_init(void);
void rg_input_deinit(void);
bool rg_input_key_is_pressed(rg_key_t key);
void rg_input_wait_for_key(rg_key_t key, bool pressed);
const char *rg_input_get_key_name(rg_key_t key);
uint32_t rg_input_read_gamepad(void);
bool rg_input_read_battery(float *percent, float *volts);
