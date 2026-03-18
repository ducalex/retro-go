#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 reg;

static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[] =
{
	{ &reg, 1, "REG" },
	{ 0 }
};

static void Sync(void) {
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
	setprg32(0x8000, reg & 0x1f);
	setmirror(((reg & 0x20) >> 5) ^ 1);
}

static DECLFW(M177Write) {
	reg = V;
	Sync();
}

static void M177Power(void) {
	reg = 0;
	Sync();
	SetReadHandler(0x6000, 0x7fff, CartBR);
	SetWriteHandler(0x6000, 0x7fff, CartBW);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M177Write);
	FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
}

static void M177Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void M177Reset(void) {
	reg = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper177_Init(CartInfo *info) {
	info->Power = M177Power;
	info->Close = M177Close;
	info->Reset = M177Reset;
	GameStateRestore = StateRestore;

	WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
	WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");
	if (info->battery) {
		info->SaveGame[0] = WRAM;
		info->SaveGameLen[0] = WRAMSIZE;
	}

	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
