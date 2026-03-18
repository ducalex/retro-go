#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022 NewRisingSun
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

static uint8 latch;

static void Mapper294_sync (void) {
	setprg16(0x8000, latch);
	setprg16(0xC000, latch |7);
	setchr8(0);
	setmirror(latch &0x80? MI_H: MI_V);
}

static DECLFW(Mapper294_writeInnerBank) {
	latch =latch &~7 | V&7;
	Mapper294_sync();
}

static DECLFW(Mapper294_writeOuterBank) {
	if (A &0x100) {
		latch =latch &7 | V <<3;
		Mapper294_sync();
	}
}

static void Mapper294_reset(void) {
	latch =0;
	Mapper294_sync();
}

static void Mapper294_power(void) {
	Mapper294_reset();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, Mapper294_writeInnerBank);
	SetWriteHandler(0x4020, 0x7FFF, Mapper294_writeOuterBank);
}

void Mapper294_Init(CartInfo *info) {
	info->Power = Mapper294_power;
	info->Reset = Mapper294_reset;
	AddExState(&latch, 1, 0, "LATC");
}
#endif
