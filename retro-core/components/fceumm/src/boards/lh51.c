#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2019 Libretro Team
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

/* FDS Conversion
 * NES 2.0 Mapper 309 is used for Whirlwind Manu's ROM cartridge conversion
 * of game 愛戦士ニコル (Ai Senshi Nicol, cartridge code LH51).
 * Its UNIF board name is UNL-LH51.
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_309
 */

#include "mapinc.h"
#include "../fds_apu.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 reg, mirr;
static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[] =
{
	{ &reg, 1, "REG" },
	{ &mirr, 1, "MIRR" },
	{ 0 }
};

static void Sync(void) {
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
	setprg8(0x8000, reg & 0x0F);
	setprg8(0xA000, 13);
	setprg8(0xC000, 14);
	setprg8(0xE000, 15);
	setmirror(((mirr >> 3) & 1) ^ 1);
}

static DECLFW(LH51Write) {
	switch (A & 0xF000) {
	case 0x8000: reg = V; Sync(); break;
	case 0xF000: mirr = V; Sync(); break;
	}
}

static void LH51Power(void) {
	FDSSoundPower();
	Sync();
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);
	SetWriteHandler(0x8000, 0xFFFF, LH51Write);
	FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
}

static void LH51Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void StateRestore(int version) {
	Sync();
}

void LH51_Init(CartInfo *info) {
	info->Power = LH51Power;
	info->Close = LH51Close;
	GameStateRestore = StateRestore;

	WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
	WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");

	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
