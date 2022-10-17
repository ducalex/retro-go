/*

This program implements the main functions to use the LCD Game emulator.
It implements also sound buzzer and keyboard functions.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#ifndef _GW_SYSTEM_H_
#define _GW_SYSTEM_H_

#include "gw_type_defs.h"

#define GW_SCREEN_WIDTH 320
#define GW_SCREEN_HEIGHT 240

/* refresh rate 128Hz */
#define GW_REFRESH_RATE 128

/* System clock = Audio clock = 32768Hz */
#define GW_SYS_FREQ 32768U
#define GW_AUDIO_FREQ GW_SYS_FREQ

#define GW_AUDIO_BUFFER_LENGTH (GW_AUDIO_FREQ / GW_REFRESH_RATE)

/* emulated system : number of clock cycles per loop */
#define GW_SYSTEM_CYCLES (GW_AUDIO_FREQ / GW_REFRESH_RATE)

/* Main API to emulate the system */

// Configure, start and reset (in this order)
bool gw_system_config();
void gw_system_start();
void gw_system_reset();

// Run some clock cycles and refresh the display
int gw_system_run(int clock_cycles);
void gw_system_blit(unsigned short *active_framebuffer);

// Audio init
void gw_system_sound_init();

// ROM loader
bool gw_system_romload();

// get buttons state
unsigned int gw_get_buttons();

bool gw_state_load(void *src_ptr);
bool gw_state_save(void *dest_ptr);

typedef struct gw_time_s
{
    unsigned char hours; // 24 format
    unsigned char minutes;
    unsigned char seconds;
} gw_time_t;

void gw_system_set_time(gw_time_t time);
gw_time_t gw_system_get_time();

/* shared audio buffer between host and emulator */
extern unsigned char gw_audio_buffer[GW_AUDIO_BUFFER_LENGTH * 2];
extern bool gw_audio_buffer_copied;

/* Output of emulated system  */
void gw_writeR(unsigned char data);

/* Input of emulated system */
unsigned char gw_readK(unsigned char S);
unsigned char gw_readBA();
unsigned char gw_readB();

#define GW_BUTTON_LEFT 1
#define GW_BUTTON_UP (1 << 1)
#define GW_BUTTON_RIGHT (1 << 2)
#define GW_BUTTON_DOWN (1 << 3)
#define GW_BUTTON_A (1 << 4)
#define GW_BUTTON_B (1 << 5)
#define GW_BUTTON_TIME (1 << 6)
#define GW_BUTTON_GAME (1 << 7)

#define GW_MASK_K1 0x000000ff
#define GW_MASK_K2 0x0000ff00
#define GW_MASK_K3 0x00ff0000
#define GW_MASK_K4 0xff000000

/* Game & Watch Magic word for save file */
#define GW_MAGIC_WORD "G&WS\x00\x00\x00\x05"

/* Game & Watch Save state */
typedef struct gw_state_s
{
    /* Magic work to check if it's Game & Watch Save structure */
    char magic_word[8];

    /* Signature of the original ROM name (8 last characters) */
    /* used to check coherency with save */
    char rom_signature[8];

    // internal RAM 128x4 bits
    un8 gw_ram[128];
    un8 gw_ram_state[128];

    u16 m_pc, m_prev_pc;
    u16 m_op, m_prev_op;
    u8 m_param;
    int m_stack_levels;
    u16 m_stack[4]; // max 4
    int m_icount;

    u8 m_acc;
    u8 m_bl;
    u8 m_bm;
    u8 bool_m_sbm;
    u8 bool_m_sbl;
    u8 m_c;
    u8 bool_m_skip;
    u8 m_w, m_s_out;
    u8 m_r, m_r_out;
    int m_r_mask_option;
    u8 bool_m_k_active;
    u8 bool_m_halt;
    int m_clk_div;

    // melody controller
    u8 m_melody_rd;
    u8 m_melody_step_count;
    u8 m_melody_duty_count;
    u8 m_melody_duty_index;
    u8 m_melody_address;

    // freerun time counter
    u16 m_div;
    u8 bool_m_1s;

    // lcd driver
    u8 flag_lcd_deflicker_level;
    u8 m_l, m_x;
    u8 m_y;
    u8 m_bp;
    u8 bool_m_bc;

    // SM500 internals
    int m_o_pins;     // number of 4-bit O pins
    u8 m_ox[9];       // W' latch, max 9
    u8 m_o[9];        // W latch
    u8 m_ox_state[9]; // W' latch, max 9
    u8 m_o_state[9];  // W latch

    u8 m_cn;
    u8 m_mx;
    u8 trs_field;

    u8 m_cb;
    u8 m_s;
    u8 bool_m_rsub;

} gw_state_t;

#endif /* _GW_SYSTEM_H_ */
