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
 *
 */

/* NES 2.0 mapper 335 is used for a 10-in-1 multicart.
 * Its UNIF board name is BMC-CTC-09.
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_335 */

#include "mapinc.h"

#define PRG 0
#define CHR 1

static uint8 regs[2];

static SFORMAT StateRegs[] =
{
	{ regs, 2, "REGS" },
	{ 0 }
};

static void Sync(void) {
	if (regs[PRG] & 0x10) {
		setprg16(0x8000, ((regs[PRG] & 0x07) << 1) | ((regs[PRG] >> 3) & 1));
		setprg16(0xC000, ((regs[PRG] & 0x07) << 1) | ((regs[PRG] >> 3) & 1));
	} else
		setprg32(0x8000, regs[PRG] & 0x07);

	setchr8(regs[CHR] & 0x0F);
	setmirror(((regs[PRG] >> 5) & 1) ^ 1);
}

static DECLFW(WritePRG) {
	regs[PRG] = V;
	Sync();
}

static DECLFW(WriteCHR) {
	regs[CHR] = V;
	Sync();
}

static void BMCCTC09Power(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xBFFF, WriteCHR);
	SetWriteHandler(0xC000, 0xFFFF, WritePRG);
}

static void StateRestore(int version) {
	Sync();
}

void BMCCTC09_Init(CartInfo *info) {
	info->Power = BMCCTC09Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
