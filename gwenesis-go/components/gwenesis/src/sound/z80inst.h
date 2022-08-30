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
#ifndef _Z80_INTERFACE_H_
#define _Z80_INTERFACE_H_

void z80_write_ctrl(unsigned int address, unsigned int value);
unsigned int z80_read_ctrl(unsigned int address);
void z80_start();
void z80_pulse_reset();
void z80_execute(unsigned int target);
void z80_run(int target);
extern int zclk;

void gwenesis_z80inst_save_state();
void gwenesis_z80inst_load_state();

void z80_set_memory(unsigned char *buffer);

void z80_write_memory_8(unsigned int address, unsigned int value);
void z80_write_memory_16(unsigned int address, unsigned int value);
unsigned int z80_read_memory_16(unsigned int address);
unsigned int z80_read_memory_8(unsigned int address);
void z80_irq_line(unsigned int value);

void gwenesis_z80inst_save_state();
void gwenesis_z80inst_load_state();

#endif