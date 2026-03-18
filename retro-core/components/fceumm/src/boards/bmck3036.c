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

#include "mapinc.h"

static uint8 regs[2];

static SFORMAT StateRegs[] =
{
	{ regs,  2, "REGS" },
	{ 0 }
};

static void Sync(void) {
	if (regs[0] &0x20) { /* NROM-128 */
		setprg16(0x8000, regs[0] >>2 &0x20 | regs[0] &0x1F);
		setprg16(0xC000, regs[0] >>2 &0x20 | regs[0] &0x1F);
	} else {	    /* UNROM */
		setprg16(0x8000, regs[0] >>2 &0x20 | regs[0] | regs[1] &7);
		setprg16(0xC000, regs[0] >>2 &0x20 | regs[0] |          7);
	}
	setchr8(0);
	setmirror((regs[0] &0x40 || regs[0] &0x20 && regs[0] &0x04)? MI_H: MI_V);
}

static DECLFW(M340Write) {
	regs[0] = A & 0xFF;
	regs[1] = V;
	Sync();
}

static void BMCK3036Power(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M340Write);
}

static void BMCK3036Reset(void) {
	regs[0] = regs[1] = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void BMCK3036_Init(CartInfo *info) {
	info->Power = BMCK3036Power;
	info->Reset = BMCK3036Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
