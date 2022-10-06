/*
Gwenesis : Genesis & megadrive Emulator.

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
#ifndef _gwenesis_bus_H_
#define _gwenesis_bus_H_

#pragma once

#include <stdio.h>
#include <string.h>

#define MAX_ROM_SIZE 0x800000
#define MAX_RAM_SIZE 0x10000
#define MAX_Z80_RAM_SIZE 8192

// NTSC PAL timings
#define MCLOCK_PAL 53203424
#define MCLOCK_NTSC 53693175

#define MCYCLES_PER_FRAME_NTSC 896040
#define MCYCLES_PER_FRAME_PAL 1067040
#define LINES_PER_FRAME_NTSC 262
#define LINES_PER_FRAME_PAL 313

#define GWENESIS_REFRESH_RATE_NTSC 60
#define GWENESIS_AUDIO_FREQ_NTSC 53267

#define GWENESIS_REFRESH_RATE_PAL 50
#define GWENESIS_AUDIO_FREQ_PAL 52781

#define GWENESIS_AUDIO_ACCURATE 1

#define Z80_FREQ_DIVISOR 14     // Frequency divisor to Z80 clock
#define VDP_CYCLES_PER_LINE 3420// VDP Cycles per Line
#define SCREEN_WIDTH 320

#define AUDIO_FREQ_DIVISOR 1009
#define GWENESIS_AUDIO_BUFFER_LENGTH_NTSC 888
#define GWENESIS_AUDIO_BUFFER_LENGTH_PAL 1056

/* Audio buffer length */

enum mapped_address
{
    NONE = 0,
    ROM_ADDR,
    ROM_ADDR_MIRROR,
    Z80_RAM_ADDR,
    Z80_RAM_ADDR1K,
    Z80_YM2612_ADDR,
    Z80_BANK_ADDR,
    Z80_VDP_ADDR,
    Z80_SN76489_ADDR,
    IO_CTRL,
    Z80_CTRL,
    TMSS_CTRL,
    VDP_ADDR,
    RAM_ADDR
};

enum gwenesis_bus_pad_button
{
    PAD_UP,
    PAD_DOWN,
    PAD_LEFT,
    PAD_RIGHT,
    PAD_B,
    PAD_C,
    PAD_A,
    PAD_S
};

#if GNW_TARGET_MARIO != 0 | GNW_TARGET_ZELDA != 0
void load_cartridge();
#else
void load_cartridge(unsigned char *buffer, size_t size);
#endif

void power_on();
void reset_emulation();
void set_region();

void gwenesis_bus_save_state();
void gwenesis_bus_load_state();

#endif