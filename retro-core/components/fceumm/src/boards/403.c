#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* NES 2.0 Mapper 403 denotes the 89433 circuit board with up to 1 MiB PRG-ROM and 32 KiB of CHR-RAM, bankable with 8 KiB granularity.
 *
 * Tetris Family - 玩家 19-in-1 智瑟實典 (NO-1683)
 * Sachen Superpack (versions A-C)
 */

#include "mapinc.h"

static uint8 reg[3];

static void Sync(void) {
	uint8 prg  = reg[0];
	uint8 chr  = reg[1];
	uint8 mode = reg[2];

	/* NROM-128 */
	if (mode & 1) {
		setprg16(0x8000, prg >> 1);
		setprg16(0xC000, prg >> 1);
	/* NROM-256 */
	} else
		setprg32(0x8000, prg >> 2);
	setchr8(chr);
	setmirror(((mode >> 4) & 1) ^ 1);
}

static DECLFW(M403Write4) {
	reg[A & 3] = V;
	Sync();
}

static DECLFW(M403Write8) {
	if (reg[2] & 4) {
		reg[1] = V;
		Sync();
	}
}

static void M403Reset(void) {
	reg[0] = reg[1] = reg[2] = 0;
	Sync();
}

static void M403Power(void) {
	reg[0] = reg[1] = reg[2] = 0;
	Sync();
	SetReadHandler(0x6000, 0x7FFF, CartBR); /* For TetrisA (Tetris Family 19-in-1 NO 1683) */
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4100, 0x4103, M403Write4);
	SetWriteHandler(0x8000, 0xFFFF, M403Write8);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper403_Init(CartInfo *info) {
	info->Reset = M403Reset;
	info->Power = M403Power;
	GameStateRestore = StateRestore;
	AddExState(&reg, 3, 0, "REGS");
}
#endif
