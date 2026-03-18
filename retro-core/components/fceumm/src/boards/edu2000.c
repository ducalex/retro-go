#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM = NULL;
static uint8 reg;

static SFORMAT StateRegs[] =
{
	{ &reg, 1, "REG" },
	{ 0 }
};

static void Sync(void) {
	setchr8(0);
	setprg8r(0x10, 0x6000, (reg & 0xC0) >> 6);
	setprg32(0x8000, reg & 0x1F);
/*	setmirror(((reg&0x20)>>5)); */
}

static DECLFW(UNLEDU2000HiWrite) {
/*	FCEU_printf("%04x:%02x\n",A,V); */
	reg = V;
	Sync();
}

static void UNLEDU2000Power(void) {
	setmirror(MI_0);
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0xFFFF, CartBW);
	SetWriteHandler(0x8000, 0xFFFF, UNLEDU2000HiWrite);
	FCEU_CheatAddRAM(32, 0x6000, WRAM);
	reg = 0;
	Sync();
}

static void UNLEDU2000Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void UNLEDU2000Restore(int version) {
	Sync();
}

void UNLEDU2000_Init(CartInfo *info) {
	info->Power = UNLEDU2000Power;
	info->Close = UNLEDU2000Close;
	GameStateRestore = UNLEDU2000Restore;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(32768);
#else
	WRAM = (uint8*)ahb_calloc(1, 32768);
#endif
	SetupCartPRGMapping(0x10, WRAM, 32768, 1);
	if (info->battery) {
		info->SaveGame[0] = WRAM;
		info->SaveGameLen[0] = 32768;
	}
	AddExState(WRAM, 32768, 0, "WRAM");
	AddExState(StateRegs, ~0, 0, 0);
}
#endif
