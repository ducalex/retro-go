#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020
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

/* Mapper 554 - Kaiser KS-7010
 * used for one of several cartridge conversions of the FDS version of
 * 悪魔城 Dracula, the Japanese version of Castlevania
 */

#include "mapinc.h"

static uint8 reg;

static SFORMAT StateRegs[] =
{
	{ &reg, 1, "REG" },
	{ 0 }
};

static void Sync(void) {
	setprg8(0x6000, reg);
    setprg8(0x8000, 10);
    setprg8(0xA000, 11);
    setprg8(0xC000, 6);
    setprg8(0xE000, 7);
    setchr8(reg);
}

static DECLFR(M554Read) {
    int A1 = A &~1;
    if ((A >= 0xCAB6) && (A <= 0xCAD7))
    {
        reg = (A >> 2) & 0x0F;
        Sync();
    }
    else if ((A1 == 0xEBE2) || (A1 == 0xEE32))
    {
        reg = (A >> 2) & 0x0F;
        Sync();
    }
    else if (A1 == 0xFFFC)
    {
        reg = (A >> 2) & 0x0F;
        Sync();
    }
    return CartBR(A);
}

static void M554Power(void) {
	Sync();
    SetReadHandler(0x6000, 0xFFFF, M554Read);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper554_Init(CartInfo *info) {
	info->Power = M554Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
