#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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

#include "mapinc.h"

static uint8 inner_bank;
static uint8 outer_bank;

static SFORMAT StateRegs[] =
{
	{ &outer_bank, 1, "OUTB" },
	{ &inner_bank, 1, "INNB" },
	{ 0 }
};

static void Sync(void) {
	setprg16(0x8000, ((outer_bank >> 2) & ~7) | (inner_bank & 7));
	setprg16(0xC000, ((outer_bank >> 2) & ~7) | 7);
	setchr8(0);
	setmirror((outer_bank & 1) ^ 1);
}

static DECLFW(M431Write) {
	if (A < 0xC000) outer_bank = V;
    else inner_bank = V;
	Sync();
}

static void M431Power(void) {
    inner_bank = 0;
    outer_bank = 0;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M431Write);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper431_Init(CartInfo *info) {
	info->Power = M431Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
