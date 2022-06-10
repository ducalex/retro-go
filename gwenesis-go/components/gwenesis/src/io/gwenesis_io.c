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
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "gwenesis_io.h"
#include "gwenesis_savestate.h"

unsigned short button_state[3];
static unsigned short gwenesis_io_pad_state[3];
static unsigned char io_reg[16] = {0x80, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x40, 0xff, 0, 0, 0xff, 0, 0, 0xff, 0, 0}; /* initial state */

void gwenesis_io_pad_press_button(int pad, int button)
{
    button_state[pad] |= (1 << button);
}

void gwenesis_io_pad_release_button(int pad, int button)
{
    button_state[pad] &= ~(1 << button);
}

static inline void gwenesis_io_pad_write(int pad, int value)
{
    unsigned char mask = io_reg[pad + 4];

    gwenesis_io_pad_state[pad] &= ~mask;
    gwenesis_io_pad_state[pad] |= value & mask;
}

static inline  unsigned char gwenesis_io_pad_read(int pad)
{
    unsigned char value;

    /* get host button */
    gwenesis_io_get_buttons();

    value = gwenesis_io_pad_state[pad] & 0x40;
    value |= 0x3f;

    if (value & 0x40)
    {
        value &= ~(button_state[pad] & 0x3f);
    }
    else
    {
        value &= ~(0xc | (button_state[pad] & 3) | ((button_state[pad] >> 2) & 0x30));
    }
    return value;
}

void gwenesis_io_write_ctrl(unsigned int address, unsigned int value)
{
    address >>= 1;
    // JOYPAD DATA
    if (address >= 0x1 && address < 0x3)
    {
        io_reg[address] = value;
        gwenesis_io_pad_write(address - 1, value);
        return;
    }
    // JOYPAD CTRL
    else if (address >= 0x4 && address < 0x6)
    {
        if (io_reg[address] != value)
        {
            io_reg[address] = value;
            gwenesis_io_pad_write(address - 4, io_reg[address - 3]);
        }
        return;
    }
    printf("io_write_memory(%x, %x)\n", address, value);
    return;
}

unsigned int gwenesis_io_read_ctrl(unsigned int address)
{
    address >>= 1;
    if (address >= 0x1 && address < 0x4)
    {
        unsigned char mask = 0x80 | io_reg[address + 3];
        unsigned char value;
        value = io_reg[address] & mask;
        value |= gwenesis_io_pad_read(address - 1) & ~mask;
        return value;
    }
    else
    {
        return io_reg[address];
    }
}

void gwenesis_io_set_reg(unsigned int reg, unsigned int value) {
    io_reg[reg] = value;
    return;
}

void gwenesis_io_save_state() {
    SaveState* state;
    state = saveGwenesisStateOpenForWrite("io");
    saveGwenesisStateSetBuffer(state, "button_state", button_state, sizeof(button_state));
    saveGwenesisStateSetBuffer(state, "gwenesis_io_pad_state", gwenesis_io_pad_state, sizeof(gwenesis_io_pad_state));
    saveGwenesisStateSetBuffer(state, "io_reg", io_reg, sizeof(io_reg));
}

void gwenesis_io_load_state() {
    SaveState* state = saveGwenesisStateOpenForRead("io");
    saveGwenesisStateGetBuffer(state, "button_state", button_state, sizeof(button_state));
    saveGwenesisStateGetBuffer(state, "gwenesis_io_pad_state", gwenesis_io_pad_state, sizeof(gwenesis_io_pad_state));
    saveGwenesisStateGetBuffer(state, "io_reg", io_reg, sizeof(io_reg));
}
