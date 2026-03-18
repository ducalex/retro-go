#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright (C) 2019 Libretro Team
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

/* added 2019-5-23
 * Mapper 31 - custom mapper by infiniteneslives
 * https://wiki.nesdev.com/w/index.php/INES_Mapper_031 */

#include "mapinc.h"

static uint8 preg[8];

static SFORMAT StateRegs[] =
{
	{ preg, 8, "PREG" },
	{ 0 }
};

static void Sync(void) {
	setprg4(0x8000, preg[0]);
	setprg4(0x9000, preg[1]);
	setprg4(0xA000, preg[2]);
	setprg4(0xB000, preg[3]);
	setprg4(0xC000, preg[4]);
	setprg4(0xD000, preg[5]);
	setprg4(0xE000, preg[6]);
	setprg4(0xF000, preg[7]);
	setchr8(0);
}

static DECLFW(M31Write) {
	preg[A & 7] = V;
	Sync();
}

static void M31Power(void) {
	preg[7] = ~0;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x5000, 0x5FFF, M31Write);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper31_Init(CartInfo *info) {
	info->Power = M31Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
