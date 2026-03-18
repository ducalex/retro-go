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
 
/* K-3071 */
 
#include "mapinc.h"

static uint8 latch[2];

static void Mapper438_Sync(void) {
	if (latch[0] &1)
		setprg32(0x8000, latch[0] >>2);
	else {
		setprg16(0x8000, latch[0] >>1);
		setprg16(0xC000, latch[0] >>1);
	}
	setchr8(latch[1] >>1);
	setmirror(latch[1] &1 ^1);
	
}

static DECLFW(Mapper438_WriteLatch) {
	latch[0] =A &0xFF;
	latch[1] =V;
	Mapper438_Sync();
}

static void Mapper438_Reset(void) {
	latch[0] =latch[1] =0;
	Mapper438_Sync();
}

static void Mapper438_Power(void) {
	latch[0] =latch[1] =0;
	Mapper438_Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, Mapper438_WriteLatch);
}

static void StateRestore(int version) {
	Mapper438_Sync();
}

void Mapper438_Init(CartInfo *info) {
	info->Reset = Mapper438_Reset;
	info->Power = Mapper438_Power;
	GameStateRestore = StateRestore;
	AddExState(&latch, 2, 0, "LATC");
}
#endif
