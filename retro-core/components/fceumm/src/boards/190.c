#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright (C) 2017 FCEUX Team
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
 * Magic Kid GooGoo
 */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 preg, creg[4];
static uint8 *WRAM = NULL;

static SFORMAT StateRegs[] =
{
	{ &preg, 1, "PREG" },
	{ creg, 4, "CREG" },
	{ 0 }
};

static void Sync(void) {
	setprg8r(0x10, 0x6000, 0);
	setprg16(0x8000, preg);
	setprg16(0xC000, 0);
	setchr2(0x0000, creg[0]);
	setchr2(0x0800, creg[1]);
	setchr2(0x1000, creg[2]);
	setchr2(0x1800, creg[3]);
}

static DECLFW(M190Write89) {
	preg = V & 7;
	Sync();
}

static DECLFW(M190WriteCD) {
	preg = 8 | (V & 7);
	Sync();
}

static DECLFW(M190WriteAB) {
	creg[A & 3] = V;
	Sync();
}

static void M190Power(void) {
	creg[0] = creg[1] = creg[2] = creg[3] = 0;
	preg = 0;
	FCEU_CheatAddRAM(0x2000 >> 10, 0x6000, WRAM);
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);
	SetWriteHandler(0x8000, 0x9FFF, M190Write89);
	SetWriteHandler(0xA000, 0xBFFF, M190WriteAB);
	SetWriteHandler(0xC000, 0xDFFF, M190WriteCD);
	setmirror(MI_V);
	Sync();
}

static void M190Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void StateRestore(int version) {
	Sync();
}

void Mapper190_Init(CartInfo *info) {
	info->Power = M190Power;
	info->Close = M190Close;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(0x2000);
#else
	WRAM = (uint8*)ahb_calloc(1, 0x2000);
#endif
	SetupCartPRGMapping(0x10, WRAM, 0x2000, 1);
	AddExState(WRAM, 0x2000, 0, "WRAM");
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
