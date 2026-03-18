#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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
#include "mmc3.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

writefunc writePPU;
static uint8 *CHRRAM;
static uint32 CHRRAMSIZE;
static uint8 mask;
static uint8 compare;
extern uint32 RefreshAddr;

static void Mapper195_CHRWrap(uint32 A, uint8 V) {
	if ((V &mask) ==compare)
		setchr1r(0x10, A, V);
	else
		setchr1r(0, A, V);
}

static const uint8 compares[8] = { 0x28, 0x00, 0x4C, 0x64, 0x46, 0x7C, 0x04, 0xFF };
static DECLFW(Mapper195_InterceptPPUWrite) {
	if (RefreshAddr <0x2000) {
		int addr =RefreshAddr;
		int reg, bank;

		if (MMC3_cmd &0x80) addr ^=0x1000;
		if (addr <0x1000)
			reg =addr >>11;
		else
			reg =(addr >>10) -2;

		bank =DRegBuf[reg];
		if (bank &0x80) {
			if (bank &0x10) {
				mask =0x00;
				compare =0xFF;
			} else {
				int index =(bank &0x02? 1: 0) | (bank &0x08? 2: 0) | (bank &0x40? 4: 0);
				mask =bank &0x40? 0xFE: 0xFC;
				compare =compares[index];
			}
			FixMMC3CHR(MMC3_cmd);
		}
	}
	writePPU(A, V);
}

static void Mapper195_Power(void) {
	mask =0xFC;
	compare =0x00;
	GenMMC3Power();
	setprg4r(0x10, 0x5000, 2);
	SetWriteHandler(0x5000, 0x5FFF, CartBW);
	SetReadHandler(0x5000, 0x5FFF, CartBR);
	
	if (GetWriteHandler(0x2007) !=Mapper195_InterceptPPUWrite) {
		writePPU =GetWriteHandler(0x2007);
		SetWriteHandler(0x2007, 0x2007, Mapper195_InterceptPPUWrite);
	}
}

void Mapper195_Close(void) {
	if (CHRRAM)
		FCEU_gfree(CHRRAM);
	CHRRAM = NULL;
}

void Mapper195_Init(CartInfo *info) {
	GenMMC3_Init(info, 512, 256, 16, info->battery);
	cwrap = Mapper195_CHRWrap;
	info->Power = Mapper195_Power;
	info->Reset = MMC3RegReset;
	info->Close = Mapper195_Close;
	CHRRAMSIZE =4096;
#ifndef FCEU_NO_MALLOC
	CHRRAM =(uint8*)FCEU_gmalloc(CHRRAMSIZE);
#else
	CHRRAM =(uint8*)ahb_calloc(1, CHRRAMSIZE);
#endif
	SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
	AddExState(CHRRAM, CHRRAMSIZE, 0, "CHRR");
	AddExState(&mask, 1, 0, "EXP0");
	AddExState(&compare, 1, 0, "EXP1");
}
#endif
