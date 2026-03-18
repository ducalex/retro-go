#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
 *  Copyright (C) 2002 Xodnizel
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
 
/* NTDEC TH2348 circuit board. UNROM plus outer bank register at $5FFx. */
 
#include "mapinc.h"

static uint8 latch;

static void Mapper437_Sync(void) {	
	setprg16(0x8000, latch);
	setprg16(0xC000, latch |7);
	setchr8(0);
	setmirror(latch >>6 &1 ^1);
}

static DECLFW(Mapper437_WriteOuterBank) {
	latch =latch &7 | A <<3;
	Mapper437_Sync();
}

static DECLFW(Mapper437_WriteInnerBank) {
	latch =latch &~7 | V &CartBR(A) &7;
	Mapper437_Sync();
}

static void Mapper437_Reset(void) {
	latch =0;
	Mapper437_Sync();
}

static void Mapper437_Power(void) {
	latch =0;
	Mapper437_Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x5000, 0x5FFF, Mapper437_WriteOuterBank);
	SetWriteHandler(0x8000, 0xFFFF, Mapper437_WriteInnerBank);
}

static void StateRestore(int version) {
	Mapper437_Sync();
}

void Mapper437_Init(CartInfo *info) {
	info->Reset = Mapper437_Reset;
	info->Power = Mapper437_Power;
	GameStateRestore = StateRestore;
	AddExState(&latch, 1, 0, "LATC");
}
#endif
