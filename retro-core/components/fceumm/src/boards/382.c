#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020
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

/* NES 2.0 Mapper 382 - denotes the 830928C circuit board,
 * used on a 512 KiB 5-in-1 and a 1 MiB 9-in-1 multicart containing
 * the BNROM game Journey to the West and Capcom/Konami UNROM games.
 */

#include "mapinc.h"

static uint8 preg[2];
static uint8 mode;
static uint8 mirr;
static uint8 lock;

static SFORMAT StateRegs[] = {
	{ &preg[0], 1, "PRG0" },
	{ &preg[1], 1, "PRG1" },
	{ &mode, 1, "MODE" },
	{ &mirr, 1, "MIRR" },
	{ &lock, 1, "LOCK" },
	{ 0 }
};

static void Sync(void) {
	switch (mode) {
	case 1:
		/* bnrom */
		setprg32(0x8000, (preg[1] << 2) | (preg[0] & 3));
		break;
	default:
		/* unrom */
		setprg16(0x8000, (preg[1] << 3) | (preg[0] & 7));
		setprg16(0xC000, (preg[1] << 3) | 7);
		break;
	}
	setchr8(0);
	setmirror(mirr ^ 1);
	/* FCEU_printf("inB[0]:%02x outB[1]:%02x mode:%02x mirr:%02x lock:%02x\n", preg[0], preg[1], mode, mirr, lock); */
}

static DECLFW(M382Write) {
	if (!lock) {
		preg[1] = (A & 0x07);
		mode = (A & 0x08) >> 3;
		mirr = (A & 0x10) >> 4;
		lock = (A & 0x20) >> 5;
	}
	/* inner bank subject to bus conflicts */
	preg[0] = V & CartBR(A);
	Sync();
}

static void M382Power(void) {
	preg[0] = preg[1] = 0;
	mode = 0;
	mirr = 0;
	lock = 0;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M382Write);
}

static void M382Reset(void) {
	preg[1] = 0;
	mode = 0;
	mirr = 0;
	lock = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper382_Init(CartInfo* info) {
	info->Power = M382Power;
	info->Reset = M382Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
