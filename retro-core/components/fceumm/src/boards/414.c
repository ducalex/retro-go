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
 */

#include "mapinc.h"

static uint8 latch_data;
static uint32 latch_addr;

static SFORMAT StateRegs[] =
{
	{ &latch_addr, 4, "ADDR" },
	{ &latch_data, 1, "DATA" },
	{ 0 }
};

static void Sync(void) {
	if (latch_addr & 0x2000) { /* NROM-256 */
		setprg32(0x8000, latch_addr >> 2);
	} else { /* NROM-128 */
		setprg16(0x8000, latch_addr >> 1);
		setprg16(0xC000, latch_addr >> 1);
	}
	setchr8(latch_data);
	setmirror((latch_addr & 1) ^ 1);
}

static DECLFW(M414Write) {
	latch_addr = A;
	latch_data = V & CartBR(A);
	Sync();
}

static void M414Power(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M414Write);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper414_Init(CartInfo *info) {
	info->Power = M414Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
