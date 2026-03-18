#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2015 CaH4e3
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

 /* Updated 6-27-19 */

#include "mapinc.h"

static uint8 datalatch, addrlatch, lock, hw_mode;

static SFORMAT StateRegs[] =
{
	{ &addrlatch, 1, "ADRL" },
	{ &datalatch, 1, "DATL" },
	{ &hw_mode,   1, "HWMO" },
	{ &lock,      1, "LOCK" },
	{ 0 }
};

static void Sync(void) {
	uint8 prg = (addrlatch & 0x3F);
	setchr8(datalatch);
	if (addrlatch & 0x80) {
		setprg16(0x8000,prg);
		setprg16(0xC000,prg);
	} else {
		setprg32(0x8000,prg >> 1);
	}
	setmirror(((datalatch >> 7) & 1) ^ 1);
}

static DECLFW(EH8813AWrite) {
	if (lock == 0) {
		addrlatch = A & 0xFF;
		datalatch = V & 0xFC;
		lock = (A & 0x100) >> 8;
	}
	datalatch = (datalatch & ~0x03) | (V & 0x03);
	Sync();
}

static DECLFR(EH8813ARead) {
	if (addrlatch & 0x40)
		A= (A & 0xFFF0) + hw_mode;
	return CartBR(A);
}

static void EH8813APower(void) {
	addrlatch = datalatch = hw_mode = lock = 0;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, EH8813ARead);
	SetWriteHandler(0x8000, 0xFFFF, EH8813AWrite);
}

static void EH8813AReset(void) {
	addrlatch = datalatch = lock = 0;
	hw_mode = (hw_mode + 1) & 0xF;
	FCEU_printf("Hardware Switch is %01X\n", hw_mode);
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void UNLEH8813A_Init(CartInfo *info) {
	info->Reset = EH8813AReset;
	info->Power = EH8813APower;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
