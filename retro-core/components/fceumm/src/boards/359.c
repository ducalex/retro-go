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

/* NES 2.0 Mapper 359 - BMC-SB-5013
 * NES 2.0 Mapper 540 - UNL-82112C
 */

#include "mapinc.h"
#include "../fds_apu.h"

static uint32 mapperNum;
static uint8 preg[4];
static uint8 creg[8];
static uint8 exRegs[4];
static uint8 IRQReload;
static uint8 IRQa;
static uint8 irqPA12;
static uint8 IRQAutoEnable;
static uint8 IRQLatch;
static uint8 IRQCount;
static int16 IRQCount16;

static SFORMAT StateRegs[] = {
	{ preg, 4, "PREG" },
	{ creg, 8, "CREG" },
	{ exRegs, 4, "EXPR" },
	{ &IRQReload, 1, "IRQL" },
	{ &IRQa, 1, "IRQa" },
	{ &irqPA12, 1, "IRQp" },
	{ &IRQAutoEnable, 1, "IRQe" },
	{ &IRQLatch, 1, "IRQl" },
	{ &IRQCount, 1, "IRQ8" },
	{ &IRQCount16, 2 | FCEUSTATE_RLSB, "IRQC" },
	{ 0 }
};

static void Sync(void) {
	uint8 prgMask = 0x3F;
	uint8 prgOuterBank = (exRegs[0] & 0x38) << 1;

	switch (exRegs[1] & 3) {
	case 0: prgMask = 0x3F; break;
	case 1: prgMask = 0x1F; break;
	case 2: prgMask = 0x2F; break;
	case 3: prgMask = 0x0F; break;
	}

	setprg8(0x6000, (preg[3] & prgMask) | prgOuterBank);
	setprg8(0x8000, (preg[0] & prgMask) | prgOuterBank);
	setprg8(0xA000, (preg[1] & prgMask) | prgOuterBank);
	setprg8(0xC000, (preg[2] & prgMask) | prgOuterBank);
	setprg8(0xE000, (     ~0 & prgMask) | prgOuterBank);

	if (!UNIFchrrama) {
		switch (mapperNum) {
		case 359: {
			uint8 i;
			uint8 chrMask = (exRegs[1] & 0x40) ? 0xFF : 0x7F;
			uint16 chrOuterBank = (exRegs[3] << 7);
			
			for (i = 0; i < 8; i++)
				setchr1(i << 10, (creg[i] & chrMask) | chrOuterBank);
		} break;
		case 540:
			setchr2(0x0000, creg[0]);
			setchr2(0x0800, creg[1]);
			setchr2(0x1000, creg[6]);
			setchr2(0x1800, creg[7]);
			break;
		}
	} else
		setchr8(0);
	
	if (exRegs[2] & 2)
		setmirror(MI_0 + (exRegs[2] & 1));
	else
		setmirror((exRegs[2] & 1) ^ 1);
}

static DECLFW(M359WriteIRQ) {
	switch (A & 0xF003) {
	case 0xC000:
		if (IRQAutoEnable) IRQa = 0;
		IRQCount16 &= 0xFF00;
		IRQCount16 |= V;
		IRQReload = 1;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xC001:
		if (IRQAutoEnable) IRQa = 1;
		IRQCount16 &= 0x00FF;
		IRQCount16 |= (V << 8);
		IRQLatch = V;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xC002:
		IRQa = (V & 1);
		irqPA12 = (V & 2) >> 1;
		IRQAutoEnable = (V & 4) >> 2; 
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0xC003:
		IRQa = (V & 1);
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	}
}

static DECLFW(M359WritePRG) {
	uint8 i = A & 3;
	preg[i] = V;
	Sync();
}

static DECLFW(M359WriteCHR) {
	uint8 i = ((A >> 10) & 4) | (A & 3);
	creg[i] = V;
	Sync();
}

static DECLFW(M359WriteEx) {
	uint8 i = A & 3;
	exRegs[i] = V;
	Sync();
}

static void M359Power(void) {
	FDSSoundPower();
	Sync();
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0x8FFF, M359WritePRG);
	SetWriteHandler(0x9000, 0x9FFF, M359WriteEx);
	SetWriteHandler(0xA000, 0xBFFF, M359WriteCHR);
	SetWriteHandler(0xC000, 0xCFFF, M359WriteIRQ);
}

static void M359Reset(void) {
	FDSSoundReset();
	Sync();
}

static void FP_FASTAPASS(1) M359CPUHook(int a) {
	if (!irqPA12) {
		if (IRQa && IRQCount16) {
			IRQCount16 -= a;
			if (IRQCount16 <= 0)
				X6502_IRQBegin(FCEU_IQEXT);
		}
	}
}

static void M359IRQHook(void) {
	if (irqPA12) {
		if (!IRQCount || IRQReload) {
			IRQCount = IRQLatch;
			IRQReload = 0;
		} else
			IRQCount--;
		if (!IRQCount && IRQa)
			X6502_IRQBegin(FCEU_IQEXT);
	}
}

static void StateRestore(int version) {
	Sync();
}

void Mapper359_Init(CartInfo* info) {
	mapperNum = 359;
	info->Power = M359Power;
	info->Reset = M359Reset;
	MapIRQHook = M359CPUHook;
	GameHBIRQHook = M359IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}

void Mapper540_Init(CartInfo* info) {
	mapperNum = 540;
	info->Power = M359Power;
	MapIRQHook = M359CPUHook;
	GameHBIRQHook = M359IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
