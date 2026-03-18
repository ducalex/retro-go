#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 * Copyright (C) 2020
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

/* NES 2.0 Mapper 390 - Realtec 8031 */

#include "mapinc.h"

static uint8 regs[2];
static uint8 dipswitch;

static SFORMAT StateRegs[] =
{
	{ &regs, 2, "REG" },
	{ &dipswitch, 1, "DPSW" },
	{ 0 }
};

static void Sync(void) {
	switch ((regs[1] >> 4) & 3) {
	case 0:
	case 1:
		/* UNROM */
		setprg16(0x8000, regs[1]);
		setprg16(0xC000, regs[1] | 7);
		break;
	case 2:
		/* Maybe unused, NROM-256? */
		setprg32(0x8000, regs[1] >> 1);
		break;
	case 3:
		/* NROM-128 */
		setprg16(0x8000, regs[1]);
		setprg16(0xC000, regs[1]);
		break;
	}
	setchr8(regs[0]);
	setmirror(((regs[0] & 0x20) >> 5) ^ 1);
}

static DECLFR(M390Read) {
	uint8 ret = CartBR(A);
	if ((regs[1] & 0x30) == 0x10)
		ret |= dipswitch;
	return ret;
}

static DECLFW(M390Write) {
	regs[(A >> 14) & 1] = A & 0x3F;
	Sync();
}

static void M390Power(void) {
	regs[0] = 0;
	regs[1] = 0;
	dipswitch = 11; /* hard-coded 150-in-1 menu */
	Sync();
	SetReadHandler(0x8000, 0xffff, M390Read);
	SetWriteHandler(0x8000, 0xffff, M390Write);
}

static void M390Reset(void) {
	dipswitch = 11; /* hard-coded 150-in-1 menu */
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper390_Init(CartInfo *info) {
	info->Reset = M390Reset;
	info->Power = M390Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
