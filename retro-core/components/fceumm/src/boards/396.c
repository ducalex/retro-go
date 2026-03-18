#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * NES 2.0 Mapper 396 - BMC-830752C
 * 1995 Super 8-in-1 (JY-050 rev0)
 * Super 8-in-1 Gold Card Series (JY-085)
 * Super 8-in-1 Gold Card Series (JY-086)
 * 2-in-1 (GN-51)
 */

#include "mapinc.h"

static uint8 reg[2];

static void Sync (void) {
	setprg16(0x8000, reg[1] << 3 | reg[0] & 7);
	setprg16(0xC000, reg[1] << 3 | 7);
	setchr8(0);
	setmirror(reg[1] & 0x60 ? 0 : 1);
}

static DECLFW(M396WriteInnerBank) {
	reg[0] = V;
	Sync();
}

static DECLFW(M396WriteOuterBank) {
	reg[1] = V;
	Sync();
}

static void M396Reset(void) {
	reg[0] = 0x00;
	reg[1] = 0x00;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

static void M396Power(void) {
	reg[0] = 0x00;
	reg[1] = 0x00;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0x9FFF, M396WriteInnerBank);
	SetWriteHandler(0xA000, 0xBFFF, M396WriteOuterBank);
	SetWriteHandler(0xC000, 0xFFFF, M396WriteInnerBank);
}

void Mapper396_Init(CartInfo *info) {
	info->Power = M396Power;
	info->Reset = M396Reset;
	GameStateRestore = StateRestore;
	AddExState(reg, 2, 0, "REGS");
}
#endif
