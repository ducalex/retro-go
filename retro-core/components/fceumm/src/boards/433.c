#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
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
 *
 */

/* NES 2.0 Mapper 433 denotes the NC-20MB PCB, used for the 20-in-1 (CA-006) multicart. It is almost identical to INES Mapper 433, except that mirroring is selected just by single bit 6 (1=Horizontal).
 */

#include "mapinc.h"

static uint8 latche;

static SFORMAT StateRegs[] =
{
	{ &latche, 1, "LATC" },
	{ 0 }
};

static void Sync(void) {
	if (!(latche & 0x20))
		setprg32(0x8000, (latche & 0x1f) >> 1);
	else {
		setprg16(0x8000, (latche & 0x1f));
		setprg16(0xC000, (latche & 0x1f));
	}
    setmirror(((latche >> 6) & 1) ^ 1);
	setchr8(0);
}

static DECLFW(M433Write) {
	latche = V;
	Sync();
}

static void M433Power(void) {
	latche = 0;
	Sync();
	SetWriteHandler(0x8000, 0xFFFF, M433Write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void StateRestore(int version) {
	Sync();
}

static void M433Reset(void) {
	latche = 0;
	Sync();
}

void Mapper433_Init(CartInfo *info) {
	info->Power = M433Power;
	info->Reset = M433Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
}
#endif
