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

/* NES 2.0 Mapper 533 is used for the Sachen 3014 board, used for the game
 * 動動腦 II: 國中英文(一) (Dòngdòngnǎo II: Guózhōng Yīngwén (I),
 * also known as Middle School English II, SA-003).
 * It's a CNROM-like board with the added ability to read back
 * the latch content for protection purposes.
 */

#include "mapinc.h"

static uint8 latche;

static SFORMAT StateRegs[] = {
	{ &latche, 1, "LATC" },
	{ 0 }
};

static void Sync(void) {
	setprg32(0x8000, 0);
	setchr8((latche >> 4) & 1);
}

static DECLFR(M533Read) {
	return ((PRGptr[0][A] & 0xF0) | (latche >> 4));
}

static DECLFW(M533Write) {
	latche = (V & CartBR(A));
	Sync();
}

static void M533Power(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBROB);
	SetReadHandler(0xE000, 0xEFFF, M533Read);
	SetWriteHandler(0x8000, 0xFFFF, M533Write);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper533_Init(CartInfo* info) {
	info->Power = M533Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
