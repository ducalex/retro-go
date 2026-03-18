#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 * Copyright (C) 2022
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

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static uint16 addrlatch;
static uint8 datalatch;

static void Sync(void) {
	uint32 S = addrlatch & 1;
	uint32 p = ((addrlatch >> 2) & 0x1F) + ((addrlatch & 0x100) >> 3) + ((addrlatch & 0x400) >> 4);
	uint32 L = (addrlatch >> 9) & 1;
	uint32 p_8000 = p;

	if ((addrlatch >> 11) & 1)
		p_8000 = (p & 0x7E) | (datalatch & 7);

	if ((addrlatch >> 7) & 1) {
		if (S) {
			setprg32(0x8000, p >> 1);
		} else {
			setprg16(0x8000, p_8000);
			setprg16(0xC000, p);
		}
	} else {
		if (S) {
			if (L) {
				setprg16(0x8000, p_8000 & 0x7E);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p_8000 & 0x7E);
				setprg16(0xC000, p & 0x78);
			}
		} else {
			if (L) {
				setprg16(0x8000, p_8000);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p_8000);
				setprg16(0xC000, p & 0x78);
			}
		}
	}

	if ((addrlatch & 0x80) == 0x80)
		/* CHR-RAM write protect hack, needed for some multicarts */
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 0);
	else
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 1);

	setmirror(((addrlatch >> 1) & 1) ^ 1);
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
}

static DECLFW(M375Write) {
	if (addrlatch & 0x800)
		datalatch = V;
	else {
		addrlatch = A;
		datalatch = V;
	}
	Sync();
}

static void M375Reset(void) {
	addrlatch = 0;
	datalatch = 0;
	Sync();
}

static void M375Power(void) {
	addrlatch = 0;
	datalatch = 0;
	Sync();
	setchr8(0);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M375Write);
	if (WRAMSIZE) {
		SetReadHandler(0x6000, 0xFFFF, CartBR);
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
	}
}

static void StateRestore(int version) {
	Sync();
}

void Mapper375_Init(CartInfo *info) {
	info->Power = M375Power;
	info->Reset = M375Reset;
	GameStateRestore = StateRestore;
	AddExState(&addrlatch, 2, 0, "ADDR");
	AddExState(&datalatch, 1, 0, "DATA");

	WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
	WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");
}
#endif
