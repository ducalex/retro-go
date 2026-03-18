#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
static uint8 hrd_flag;

static SFORMAT StateRegs[] =
{
	{ &hrd_flag, 1, "DPSW" },
	{ regs, 2, "REGS" },
	{ 0 }
};

static void Sync(void) {
	if (regs[1] & 0x10)
		setprg32(0x8000, (regs[1] >> 6) & 3);
	else {
		setprg16(0x8000, (regs[1] >> 5) & 7);
		setprg16(0xC000, (regs[1] >> 5) & 7);
	}
	setmirror((regs[1] & 8) >> 3 ^ 1);
	setchr8((regs[0] & 7) | (regs[1] & 7) | ((regs[0] & 0x40) >> 3));
}

static DECLFR(M57Read) {
	return hrd_flag;
}

static DECLFW(M57Write) {
	switch (A & 0x8800) {
		case 0x8000: regs[0] = V; Sync(); break;
		case 0x8800: regs[1] = V; Sync(); break;
	}
}

static void M57Power(void) {
	regs[1] = regs[0] = 0; /* Always reset to menu */
	hrd_flag = 1; /* YH-xxx "Olympic" multicarts disable the menu after one selection */
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M57Write);
	SetReadHandler(0x6000, 0x6000, M57Read);
	Sync();
}

static void M57Reset(void) {
	regs[1] = regs[0] = 0; /* Always reset to menu */
	hrd_flag++;
	hrd_flag &= 3;
	FCEU_printf("Select Register = %02x\n", hrd_flag);
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper57_Init(CartInfo *info) {
	info->Power = M57Power;
	info->Reset = M57Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
