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

/* FDS Conversion - Kid Icarus (パルテナの鏡) (Parthena) */

#include "mapinc.h"
#include "../fds_apu.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 preg;
static uint8 mirr;
static uint8 *WRAM;

static SFORMAT StateRegs[] =
{
	{ &preg, 1, "PREG" },
	{ &mirr, 1, "MIRR" },
	{ 0 }
};

static uint32 GetWRAMAddress(uint32 A) {
	return  ((A & 0x1FFF) |
			((A < 0xC000) ? 0x1000 : 0x0000) |
			((A < 0x8000) ? 0x0800 : 0x000));
}

static void Sync(void) {
	setprg8(0x6000, 13);
	setprg8(0x8000, 12);
	setprg8(0xA000, preg);
	setprg8(0xC000, 14);
	setprg8(0xE000, 15);
	setchr8(0);
	setmirror(((mirr & 8) >> 3) ^ 1);
}

static DECLFR(M539Read) {
	switch (A >> 8) {
	case 0x60: case 0x62: case 0x64: case 0x65: case 0x82: case 0xC0: case 0xC1: case 0xC2:
	case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA:
	case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1: case 0xDF:
		return WRAM[GetWRAMAddress(A)];
	default:
		return CartBR(A);
	}
}

static DECLFW(M539Write) {
	switch (A >> 8) {
	case 0x60: case 0x62: case 0x64: case 0x65: case 0x82: case 0xC0: case 0xC1: case 0xC2:
	case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA:
	case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1: case 0xDF:
		WRAM[GetWRAMAddress(A)] = V;
		break;
	default:
		switch (A & 0xF000) {
		case 0xA000:
			preg = V;
			Sync();
			break;
		case 0xF000:
			if ((A & 0x25) == 0x25) {
				mirr = V;
				Sync();
			}
			break;
		}
		break;
	}
}

static void M539Power(void) {
	FDSSoundPower();
	preg = 0;
	mirr = 0;
	Sync();
	SetReadHandler(0x6000, 0xFFFF, M539Read);
	SetWriteHandler(0x6000, 0xFFFF, M539Write);
}

#ifndef FCEU_NO_MALLOC
static void M539Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}
#endif

static void StateRestore(int version) {
	Sync();
}

void Mapper539_Init(CartInfo *info) {
	info->Power = M539Power;
#ifndef FCEU_NO_MALLOC
	info->Close = M539Close;
#endif
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(8192);
#else
	WRAM = (uint8*)itc_calloc(1,8192);
#endif
	GameStateRestore = StateRestore;
	AddExState(WRAM, 8192, 0, "WRAM");
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
