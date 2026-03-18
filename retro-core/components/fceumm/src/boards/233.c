#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
 *  Copyright (C) 2009 qeed
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
 *
 */

/* Updated 2019-07-12
 * Mapper 233 - UNIF 42in1ResetSwitch - reset-based switching
 */

#include "mapinc.h"

static uint8 latche;
static uint8 reset;

static SFORMAT StateRegs[] =
{
	{ &reset, 1, "RST" },
	{ &latche, 1, "LATC" },
	{ 0 }
};

static void Sync(void) {
	uint8 bank = (latche & 0x1f) | (reset << 5);

	if (!(latche & 0x20))
		setprg32(0x8000, bank >> 1);
	else {
		setprg16(0x8000, bank);
		setprg16(0xC000, bank);
	}

	switch ((latche >> 6) & 3) {
	case 0: setmirror(MI_0); break;
	case 1: setmirror(MI_V); break;
	case 2: setmirror(MI_H); break;
	case 3: setmirror(MI_1); break;
	}

	setchr8(0);
}

static DECLFW(M233Write) {
	latche = V;
	Sync();
}

static void M233Power(void) {
	latche = reset = 0;
	Sync();
	SetWriteHandler(0x8000, 0xFFFF, M233Write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void StateRestore(int version) {
	Sync();
}

static void M233Reset(void) {
	latche = 0;
	reset ^= 1;
	Sync();
}

void Mapper233_Init(CartInfo *info) {
	info->Power = M233Power;
	info->Reset = M233Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
}
#endif
