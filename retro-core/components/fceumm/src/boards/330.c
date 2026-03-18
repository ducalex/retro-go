#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
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
 *
 * NES 2.0 Mapper 330 is used for a bootleg version of Contra/Gryzor.
 * as implemented from
 * http://forums.nesdev.org/viewtopic.php?f=9&t=17352&p=218722#p218722
 */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM;

static uint8 PRG[3], CHR[8], NTAPage[4];
static uint8 IRQa;
static uint16 IRQCount;

static SFORMAT StateRegs[] =
{
	{ PRG, 3, "PRG" },
	{ CHR, 8, "CHR" },
	{ NTAPage, 4, "NT" },
	{ &IRQa, 1, "IRQA" },
	{ &IRQCount, 2, "IRQC" },
	{ 0 }
};

static void SyncPRG(void) {
	setprg8(0x8000, PRG[0]);
	setprg8(0xA000, PRG[1]);
	setprg8(0xC000, PRG[2]);
	setprg8(0xE000, ~0);
}

static void DoCHR(int x, uint8 V) {
	CHR[x] = V;
	setchr1(x << 10, V);
}

static void FixCHR(void) {
	int x;
	for (x = 0; x < 8; x++)
		DoCHR(x, CHR[x]);
}

static void FASTAPASS(2) DoNTARAM(int w, uint8 V) {
	NTAPage[w] = V;
	setntamem(NTARAM + ((V & 1) << 10), 1, w);
}

static void FixNTAR(void) {
	int x;
	for (x = 0; x < 4; x++)
		DoNTARAM(x, NTAPage[x]);
}

static DECLFW(M330Write) {
	if (!(A & 0x400)) {
		if (A >= 0x8000 && A <= 0xB800)
			DoCHR((A - 0x8000) >> 11, V);
		else if (A >= 0xC000 && A <= 0xD800)
			DoNTARAM((A - 0xC000) >> 11, V);
		else if (A >= 0xE000 && A <= 0xF000) {
			PRG[(A - 0xE000) >> 11] = V;
			SyncPRG();
		}
	} else if ((A < 0xC000) && !(A & 0x4000)) {
		if (A & 0x2000) {
			IRQCount &= 0x00FF;
			IRQCount |= (V & 0x7F) << 8;
			IRQa = V & 0x80;
			X6502_IRQEnd(FCEU_IQEXT);
		} else {
			IRQCount &= 0xFF00;
			IRQCount |= V;
		}
	}
}

static void M330Power(void) {
	int i;
	for (i = 0; i < 3; i++)
		PRG[i] = i;
	for (i = 0; i < 8; i++)
		CHR[i] = i;
	for (i = 0; i < 4; i++)
		NTAPage[i] = ~0;
	IRQa = 0;
	IRQCount = 0;
	SyncPRG();
	FixCHR();
	FixNTAR();
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M330Write);
}

static void FP_FASTAPASS(1) M330IRQHook(int a) {
	if (IRQa) {
		IRQCount += a;
		if (IRQCount > 0x7FFF) {
			X6502_IRQBegin(FCEU_IQEXT);
			IRQa = 0;
			IRQCount = 0;
		}
	}
}

static void StateRestore(int version) {
	SyncPRG();
	FixCHR();
	FixNTAR();
}

void Mapper330_Init(CartInfo *info) {
	info->Power = M330Power;
	MapIRQHook = M330IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8 *)FCEU_gmalloc(8192);
#else
	WRAM = (uint8*)ahb_calloc(1, 8192);
#endif
	SetupCartPRGMapping(0x10, WRAM, 8192, 1);
	AddExState(WRAM, 8192, 0, "WRAM");
}
#endif
