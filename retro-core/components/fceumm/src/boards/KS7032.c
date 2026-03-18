#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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
 *
 * FDS Conversion
 *
 */

/* added 2019-5-23
 * - Mapper 56 - UNL KS202
 *   FDS Conversion: Super Mario Bros. 3 (Pirate, Alt)
 *   similar to M142 but use WRAM instead? $D000 additional IRQ trigger
 * - fix IRQ counter, noticeable in status bars of both SMB2J(KS7032) and SMB3J(KS202)
 */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 reg[8], creg[8], mirr, cmd, IRQa = 0;
static int32 IRQCount, IRQLatch;
static uint8 KS7032;
static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegsKS7032[] =
{
	{ &cmd, 1, "CMD" },
	{ reg, 8, "REGS" },
	{ &IRQa, 1, "IRQA" },
	{ &IRQCount, 4 | FCEUSTATE_RLSB, "IRQC" },
	{ 0 }
};

static SFORMAT StateRegsKS202[] =
{
	{ creg, 8, "CREG" },
	{ &mirr, 1, "MIRR" },
	{ 0 }
};

static void Sync(void) {
	setprg8(0x8000, reg[0]);
	setprg8(0xA000, reg[1]);
	setprg8(0xC000, reg[2]);
	setprg8(0xE000, ~0);
	setchr8(0);
	if (KS7032)
		setprg8(0x6000, reg[3]);
	else {
		setprg8r(0x10, 0x6000, 0);
		setchr1(0x0000, creg[0]);
		setchr1(0x0400, creg[1]);
		setchr1(0x0800, creg[2]);
		setchr1(0x0C00, creg[3]);
		setchr1(0x1000, creg[4]);
		setchr1(0x1400, creg[5]);
		setchr1(0x1800, creg[6]);
		setchr1(0x1C00, creg[7]);
		setmirror(mirr);
	}
}

static DECLFW(UNLKS7032Write) {
/*	FCEU_printf("bs %04x %02x\n",A,V); */
	switch (A & 0xF000) {
/*		case 0x8FFF: reg[4]=V; Sync(); break; */
	case 0x8000: IRQLatch = (IRQLatch & 0xFFF0) | (V & 0x0F); break;
	case 0x9000: IRQLatch = (IRQLatch & 0xFF0F) | ((V & 0x0F) << 4); break;
	case 0xA000: IRQLatch = (IRQLatch & 0xF0FF) | ((V & 0x0F) << 8); break;
	case 0xB000: IRQLatch = (IRQLatch & 0x0FFF) | (V << 12); break;
	case 0xC000:
		IRQa = (V & 0xF);
		if (IRQa)
			IRQCount = IRQLatch;
		X6502_IRQEnd(FCEU_IQEXT); break;
	case 0xD000: X6502_IRQEnd(FCEU_IQEXT); break;
	case 0xE000: cmd = V & 7; break;
	case 0xF000: {
		uint8 bank = (cmd - 1);
		if (bank < 3)
			reg[bank] = (reg[bank] & 0x10) | (V & 0x0F);
		else if (bank < 4)
			reg[bank] = V;
		Sync();
		switch (A & 0xFC00) {
		case 0xF000:
			A &= 3;
			if (A < 3)
				reg[bank] = (reg[bank] & 0x0F) | (V & 0x10);
			Sync();
			break;
		case 0xF800:
			mirr = (V & 1);
			Sync();
			break;
		case 0xFC00:
			creg[A & 7] = V;
			Sync();
			break;
		}
		}
		break;
	}
}

static void FP_FASTAPASS(1) UNLSMB2JIRQHook(int a) {
	if (IRQa) {
		IRQCount += a;
		if (IRQCount >= 0xFFFF) {
			IRQCount = IRQLatch;
			X6502_IRQBegin(FCEU_IQEXT);
		}
	}
}

static void UNLKS7032Power(void) {
	Sync();
	SetReadHandler(0x6000, 0x7FFF, CartBR);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4020, 0xFFFF, UNLKS7032Write);
	if (!KS7032) {
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
	}
}

static void UNLKS7032Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void StateRestore(int version) {
	Sync();
}

void UNLKS7032_Init(CartInfo *info) {
	KS7032 = 1;
	info->Power = UNLKS7032Power;
	info->Close = UNLKS7032Close;
	MapIRQHook = UNLSMB2JIRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegsKS7032, ~0, 0, 0);
}

void UNLKS202_Init(CartInfo *info) {
	KS7032 = 0;
	info->Power = UNLKS7032Power;
	info->Close = UNLKS7032Close;
	MapIRQHook = UNLSMB2JIRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegsKS7032, ~0, 0, 0);
	AddExState(&StateRegsKS202, ~0, 0, 0);

	WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
	WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	if (info->battery) {
		info->SaveGame[0] = WRAM;
		info->SaveGameLen[0] = WRAMSIZE;
	}
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");
}
#endif
