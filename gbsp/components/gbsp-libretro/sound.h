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

#ifndef SOUND_H
#define SOUND_H

#define BUFFER_SIZE        (1 << 11)
#define BUFFER_SIZE_MASK   (BUFFER_SIZE - 1)

#define GBA_SOUND_FREQUENCY   (32768)

#ifdef OVERCLOCK_60FPS
  #define GBC_BASE_RATE ((float)(60 * 228 * (272+960)))
#else
  #define GBC_BASE_RATE ((float)(16 * 1024 * 1024))
#endif

#define DIRECT_SOUND_INACTIVE         0
#define DIRECT_SOUND_RIGHT            1
#define DIRECT_SOUND_LEFT             2
#define DIRECT_SOUND_LEFTRIGHT        3

typedef struct
{
   s8 fifo[32];
   u32 fifo_base;
   u32 fifo_top;
   fixed8_24 fifo_fractional;
   // The + 1 is to give some extra room for linear interpolation
   // when wrapping around.
   u32 buffer_index;
   u32 status;
   u32 volume_halve;
} direct_sound_struct;

#define GBC_SOUND_INACTIVE            0
#define GBC_SOUND_RIGHT               1
#define GBC_SOUND_LEFT                2
#define GBC_SOUND_LEFTRIGHT           3


typedef struct
{
   u32 rate;
   fixed16_16 frequency_step;
   fixed16_16 sample_index;
   fixed16_16 tick_counter;
   u32 total_volume;
   u32 envelope_initial_volume;
   u32 envelope_volume;
   u32 envelope_direction;
   u32 envelope_status;
   u32 envelope_ticks;
   u32 envelope_initial_ticks;
   u32 sweep_status;
   u32 sweep_direction;
   u32 sweep_ticks;
   u32 sweep_initial_ticks;
   u32 sweep_shift;
   u32 length_status;
   u32 length_ticks;
   u32 noise_type;
   u32 wave_type;
   u32 wave_bank;
   u32 wave_volume;
   u32 status;
   u32 active_flag;
   u32 master_enable;
   u32 sample_table_idx;
} gbc_sound_struct;

const extern s8 square_pattern_duty[4][8];
extern direct_sound_struct direct_sound_channel[2];
extern gbc_sound_struct gbc_sound_channel[4];
extern u32 gbc_sound_master_volume_left;
extern u32 gbc_sound_master_volume_right;
extern u32 gbc_sound_master_volume;
extern u32 gbc_sound_buffer_index;
extern u32 gbc_sound_last_cpu_ticks;

extern const u32 sound_frequency;
extern u32 sound_on;
extern bool sound_master_enable;

void sound_timer_queue32(u32 channel, u32 value);
unsigned sound_timer(fixed8_24 frequency_step, u32 channel);
void sound_reset_fifo(u32 channel);
void render_gbc_sound();
void init_sound();

bool sound_check_savestate(const u8 *src);
unsigned sound_write_savestate(u8 *dst);
bool sound_read_savestate(const u8 *src);

u32 sound_read_samples(s16 *out, u32 frames);

void reset_sound(void);

#endif
