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
#ifndef _gwenesis_savestate_H_
#define _gwenesis_savestate_H_

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct SaveState SaveState;

bool initLoadGwenesisState(unsigned char *srcBuffer);
int saveGwenesisState(unsigned char *destBuffer, int save_size);
int loadGwenesisState(unsigned char *srcBuffer);

SaveState* saveGwenesisStateOpenForRead(const char* fileName);
SaveState* saveGwenesisStateOpenForWrite(const char* fileName);

int saveGwenesisStateGet(SaveState* state, const char* tagName);
void saveGwenesisStateSet(SaveState* state, const char* tagName, int value);

void saveGwenesisStateGetBuffer(SaveState* state, const char* tagName, void* buffer, int length);
void saveGwenesisStateSetBuffer(SaveState* state, const char* tagName, void* buffer, int length);

void gwenesis_save_state();
void gwenesis_load_state();
#endif