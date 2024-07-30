/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef INPUT_H
#define INPUT_H

#include "libretro.h"

typedef enum
{
  BUTTON_L      = 0x200,
  BUTTON_R      = 0x100,
  BUTTON_DOWN   = 0x80,
  BUTTON_UP     = 0x40,
  BUTTON_LEFT   = 0x20,
  BUTTON_RIGHT  = 0x10,
  BUTTON_START  = 0x08,
  BUTTON_SELECT = 0x04,
  BUTTON_B      = 0x02,
  BUTTON_A      = 0x01,
  BUTTON_NONE   = 0x00
} input_buttons_type;

typedef struct
{
   unsigned retropad ;
   input_buttons_type gba;
} map;

static const map btn_map[] = {
   { RETRO_DEVICE_ID_JOYPAD_L,      BUTTON_L },
   { RETRO_DEVICE_ID_JOYPAD_R,      BUTTON_R },
   { RETRO_DEVICE_ID_JOYPAD_DOWN,   BUTTON_DOWN },
   { RETRO_DEVICE_ID_JOYPAD_UP,     BUTTON_UP },
   { RETRO_DEVICE_ID_JOYPAD_LEFT,   BUTTON_LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT,  BUTTON_RIGHT },
   { RETRO_DEVICE_ID_JOYPAD_START,  BUTTON_START },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, BUTTON_SELECT },
   { RETRO_DEVICE_ID_JOYPAD_B,      BUTTON_B },
   { RETRO_DEVICE_ID_JOYPAD_A,      BUTTON_A }
};

extern bool libretro_supports_bitmasks;
extern bool libretro_supports_ff_override;
extern bool libretro_ff_enabled;
extern bool libretro_ff_enabled_prev;

/* Minimum (and default) turbo pulse train
 * is 2 frames ON, 2 frames OFF */
#define TURBO_PERIOD_MIN      4
#define TURBO_PERIOD_MAX      120
#define TURBO_PULSE_WIDTH_MIN 2
#define TURBO_PULSE_WIDTH_MAX 15

extern unsigned turbo_period;
extern unsigned turbo_pulse_width;
extern unsigned turbo_a_counter;
extern unsigned turbo_b_counter;

void init_input(void);
u32 update_input(void);

bool input_check_savestate(const u8 *src);
unsigned input_write_savestate(u8* dst);
bool input_read_savestate(const u8 *src);

#endif
