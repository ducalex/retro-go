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
 
/* S-009. UNROM plus outer bank register at $6000-$7FFF. */
 
#include "mapinc.h"

static uint16 latch;

static void Mapper434_Sync(void) {	
	setprg16(0x8000, latch);
	setprg16(0xC000, latch |7);
	setchr8(0);
	setmirror(latch >>8 &1);
}

static DECLFW(Mapper434_WriteOuterBank) {
	latch =latch &7 | V <<3;
	Mapper434_Sync();
}

static DECLFW(Mapper434_WriteInnerBank) {
	latch =latch &~7 | V &CartBR(A) &7;
	Mapper434_Sync();
}

static void Mapper434_Reset(void) {
	latch =0;
	Mapper434_Sync();
}

static void Mapper434_Power(void) {
	latch =0;
	Mapper434_Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, Mapper434_WriteOuterBank);
	SetWriteHandler(0x8000, 0xFFFF, Mapper434_WriteInnerBank);
}

static void StateRestore(int version) {
	Mapper434_Sync();
}

void Mapper434_Init(CartInfo *info) {
	info->Reset = Mapper434_Reset;
	info->Power = Mapper434_Power;
	GameStateRestore = StateRestore;
	AddExState(&latch, 2, 0, "LATC");
}
#endif
