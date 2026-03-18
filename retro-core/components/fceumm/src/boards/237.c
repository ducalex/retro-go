#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2019 Libretro Team
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

/* Mapper 237 - "Teletubbies / Y2K" 420-in-1 pirate multicart.
 * Dipswitch settings:
 * 0: 42-in-1
 * 1: 5,000-in-1
 * 2: 420-in-1
 * 3: 10,000,000-in-1 (lol)
 */

#include "mapinc.h"

static uint8 reg[2];
static uint8 dipswitch;

static SFORMAT StateRegs[] =
{
	{ &reg[0],    1, "REG0" },
	{ &reg[1],    1, "REG1" },
	{ &dipswitch, 1, "DPSW" },
	{ 0 }
};

static void Sync(void) {
	uint8 bank = (reg[1] & 0x07);
	uint8 base = (reg[1] & 0x18) | ((reg[0] & 0x04) << 3);
	uint8 mode = (reg[1] & 0xC0) >> 6;

	setchr8(0);
	setprg16(0x8000, base | (bank & ~(mode & 1)));
	setprg16(0xC000, base | ((mode & 0x02) ? (bank | (mode & 0x01)) : 0x07));
	setmirror(((reg[1] & 0x20) >> 5) ^ 1);
}

static DECLFW(M237Write) {
	if (!(reg[0] & 0x02)) {
		reg[0] = A & 0x0F;
		reg[1] = (reg[1] & 0x07) | (V & 0xF8);
	}
	reg[1] = (reg[1] & 0xF8) | (V & 0x07);
	Sync();
}

static DECLFR(M237Read) {
	if (!(reg[0] & 0x02) && (reg[0] & 0x01))
		return dipswitch;
	return CartBR(A);
}

static void M237Power(void) {
	Sync();
	SetReadHandler (0x8000, 0xFFFF, M237Read);
	SetWriteHandler(0x8000, 0xFFFF, M237Write);
}

static void M237Reset(void) {
	reg[0] = reg[1] = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper237_Init(CartInfo *info) {
	/* The menu system used by this cart seems to be configurable as 4 different types:
	 * 0: 42-in-1
	 * 1: 5,000-in-1
	 * 2: 420-in-1
	 * 3: 10,000,000-in-1 (lol)
	 */
	dipswitch = 0;
	if ((info->CRC32) == 0x272709b9) /* Teletubbies Y2K (420-in-1) */
		dipswitch = 2;
	info->Power = M237Power;
	info->Reset = M237Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
