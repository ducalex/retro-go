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
#ifndef _gwenesis_io_H_
#define _gwenesis_io_H_

#pragma once

void gwenesis_io_pad_press_button(int pad, int button);
void gwenesis_io_pad_release_button(int pad, int button);

void gwenesis_io_write_ctrl(unsigned int address, unsigned int value);
unsigned int gwenesis_io_read_ctrl(unsigned int address);

void gwenesis_io_set_reg(unsigned int reg, unsigned int value);
void gwenesis_io_get_buttons();

void gwenesis_io_save_state();
void gwenesis_io_load_state();

#endif