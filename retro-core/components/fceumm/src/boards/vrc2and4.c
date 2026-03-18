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
 *
 * VRC-2/VRC-4 Konami
 * VRC-4 Pirate
 *
 */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 isPirate, is22;
static uint16 IRQCount;
static uint8 IRQLatch, IRQa;
static uint8 prgreg[4], chrreg[8];
static uint16 chrhi[8];
static uint8 regcmd, irqcmd, mirr, big_bank;
static uint16 acount = 0;

static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static uint8 prgMask = 0x1F;

static SFORMAT StateRegs[] =
{
	{ prgreg, 4, "PREG" },
	{ chrreg, 8, "CREG" },
	{ &chrhi[0], 2 | FCEUSTATE_RLSB, "CRH0" },
	{ &chrhi[1], 2 | FCEUSTATE_RLSB, "CRH1" },
	{ &chrhi[2], 2 | FCEUSTATE_RLSB, "CRH2" },
	{ &chrhi[3], 2 | FCEUSTATE_RLSB, "CRH3" },
	{ &chrhi[4], 2 | FCEUSTATE_RLSB, "CRH4" },
	{ &chrhi[5], 2 | FCEUSTATE_RLSB, "CRH5" },
	{ &chrhi[6], 2 | FCEUSTATE_RLSB, "CRH6" },
	{ &chrhi[7], 2 | FCEUSTATE_RLSB, "CRH7" },
	{ &regcmd, 1, "CMDR" },
	{ &irqcmd, 1, "CMDI" },
	{ &mirr, 1, "MIRR" },
	{ &prgMask, 1, "MAK" },
	{ &big_bank, 1, "BIGB" },
	{ &IRQCount, 2 | FCEUSTATE_RLSB, "IRQC" },
	{ &IRQLatch, 1, "IRQL" },
	{ &IRQa, 1, "IRQA" },
	{ 0 }
};

static void Sync(void) {
	if (regcmd & 2) {
		setprg8(0xC000, prgreg[0] | big_bank);
		setprg8(0x8000, (prgreg[2] & prgMask) | big_bank);
	} else {
		setprg8(0x8000, prgreg[0] | big_bank);
		setprg8(0xC000, (prgreg[2] & prgMask) | big_bank);
	}
	setprg8(0xA000, prgreg[1] | big_bank);
	setprg8(0xE000, (prgreg[3] & prgMask) | big_bank);
#ifndef TARGET_GNW
	if (UNIFchrrama)
		setchr8(0);
	else
#endif
	{
		uint8 i;
		for (i = 0; i < 8; i++)
			setchr1(i << 10, (chrhi[i] | chrreg[i]) >> is22);
	}
	switch (mirr & 0x3) {
	case 0: setmirror(MI_V); break;
	case 1: setmirror(MI_H); break;
	case 2: setmirror(MI_0); break;
	case 3: setmirror(MI_1); break;
	}
}

static DECLFW(VRC24Write) {
	A &= 0xF003;
	if ((A >= 0xB000) && (A <= 0xE003)) {
#ifndef TARGET_GNW
		if (UNIFchrrama)
			big_bank = (V & 8) << 2;							/* my personally many-in-one feature ;) just for support pirate cart 2-in-1 */
		else
#endif
		{
			uint16 i = ((A >> 1) & 1) | ((A - 0xB000) >> 11);
			uint16 nibble = ((A & 1) << 2);
			chrreg[i] = (chrreg[i] & (0xF0 >> nibble)) | ((V & 0xF) << nibble);
			if (nibble)
				chrhi[i] = (V & 0x10) << 4;						/* another one many in one feature from pirate carts */
		}
		Sync();
	} else {
		switch (A & 0xF003) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			if (!isPirate) {
				prgreg[0] = V & prgMask;
				Sync();
			}
			break;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			if (!isPirate)
			{
				prgreg[1] = V & prgMask;
			}
			else {
				prgreg[0] = (V & prgMask) << 1;
				prgreg[1] = ((V & prgMask) << 1) | 1;
			}
			Sync();
			break;
		case 0x9000:
		case 0x9001: if (V != 0xFF) mirr = V; Sync(); break;
		case 0x9002:
		case 0x9003: regcmd = V; Sync(); break;
        case 0xF000: X6502_IRQEnd(FCEU_IQEXT); IRQLatch &= 0xF0; IRQLatch |= V & 0xF; break;
		case 0xF001: X6502_IRQEnd(FCEU_IQEXT); IRQLatch &= 0x0F; IRQLatch |= V << 4; break;
		case 0xF002: X6502_IRQEnd(FCEU_IQEXT); acount = 0; IRQCount = IRQLatch; IRQa = V & 2; irqcmd = V & 1; break;
		case 0xF003: X6502_IRQEnd(FCEU_IQEXT); IRQa = irqcmd; break;
        }
	}
}

static DECLFW(M21Write) {
	A = (A & 0xF000) | ((A >> 1) & 0x3) | ((A >> 6) & 0x3);		/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [!] is Mapper 21*/
	VRC24Write(A, V);
}

static DECLFW(M22Write) {
#if 0
	/* Removed this hack, which was a bug in actual game cart.
	 * http://forums.nesdev.com/viewtopic.php?f=3&t=6584
	 */
	if ((A >= 0xC004) && (A <= 0xC007)) {						/* Ganbare Goemon Gaiden does strange things!!! at the end credits
		weirdo = 1;												 * quick dirty hack, seems there is no other games with such PCB, so
																 * we never know if it will not work for something else lol
																 */
	}
#endif
	A |= ((A >> 2) & 0x3);										/* It's just swapped lines from 21 mapper
																 */
	VRC24Write((A & 0xF000) | ((A >> 1) & 1) | ((A << 1) & 2), V);
}

static DECLFW(M23Write) {
	A |= ((A >> 2) & 0x3) | ((A >> 4) & 0x3);	/* actually there is many-in-one mapper source, some pirate or
												 * licensed games use various address bits for registers
												 */
	VRC24Write(A, V);
}

static void VRC24PowerCommon(void (*WRITEFUNC)(uint32 A, uint8 V)) {
	Sync();
	if (WRAMSIZE) {
		setprg8r(0x10, 0x6000, 0);	/* Only two Goemon games are have battery backed RAM, three more shooters
									 * (Parodius Da!, Gradius 2 and Crisis Force uses 2k or SRAM at 6000-67FF only
									 */
		SetReadHandler(0x6000, 0x7FFF, CartBR);
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
	}
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, WRITEFUNC);
}

static void M21Power(void) {
	VRC24PowerCommon(M21Write);
}

static void M22Power(void) {
	VRC24PowerCommon(M22Write);
}

static void M23Power(void) {
	big_bank = 0x20;
	if ((prgreg[2] == 0x30) && (prgreg[3] == 0x31))
		big_bank = 0x00;
	VRC24PowerCommon(M23Write);
}

static void M25Power(void) {
	big_bank = 0x20;
	VRC24PowerCommon(M22Write);
}

void FP_FASTAPASS(1) VRC24IRQHook(int a) {
	#define LCYCS 341
	if (IRQa) {
		acount += a * 3;
		if (acount >= LCYCS) {
			while (acount >= LCYCS) {
				acount -= LCYCS;
				IRQCount++;
				if (IRQCount & 0x100) {
					X6502_IRQBegin(FCEU_IQEXT);
					IRQCount = IRQLatch;
				}
			}
		}
	}
}

static void StateRestore(int version) {
	Sync();
}

static void VRC24Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void VRC24_Init(CartInfo *info, uint32 hasWRAM) {
	info->Close = VRC24Close;
	MapIRQHook = VRC24IRQHook;
	GameStateRestore = StateRestore;

	prgMask = 0x1F;
	prgreg[2] = ~1;
	prgreg[3] = ~0;

	WRAMSIZE = 0;

	/* 400K PRG + 128K CHR Contra rom hacks */
	if (info->PRGRomSize == 400 * 1024 && info->CHRRomSize == 128 * 1024) {
		prgreg[2] = 0x30;
		prgreg[3] = 0x31;
		prgMask = 0x3F;
	}

	if (hasWRAM) {
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
	}

	AddExState(&StateRegs, ~0, 0, 0);
}

void Mapper21_Init(CartInfo *info) {
	isPirate = 0;
	is22 = 0;
	info->Power = M21Power;
	VRC24_Init(info, 1);
}

void Mapper22_Init(CartInfo *info) {
	isPirate = 0;
	is22 = 1;
	info->Power = M22Power;
	VRC24_Init(info, 0);
}

void Mapper23_Init(CartInfo *info) {
	isPirate = 0;
	is22 = 0;
	info->Power = M23Power;
	VRC24_Init(info, 1);
}

void Mapper25_Init(CartInfo *info) {
	isPirate = 0;
	is22 = 0;
	info->Power = M25Power;
	VRC24_Init(info, 1);
}

void UNLT230_Init(CartInfo *info) {
	isPirate = 1;
	is22 = 0;
	info->Power = M23Power;
	VRC24_Init(info, 1);
}

/* -------------------- UNL-TH2131-1 -------------------- */
/* https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_308
 * NES 2.0 Mapper 308 is used for a bootleg version of the Sunsoft game Batman
 * similar to Mapper 23 Submapper 3) with custom IRQ functionality.
 * UNIF board name is UNL-TH2131-1.
 */

static DECLFW(TH2131Write) {
	switch (A & 0xF003) {
	case 0xF000: X6502_IRQEnd(FCEU_IQEXT); IRQa = 0; IRQCount = 0; break;
	case 0xF001: IRQa = 1; break;
	case 0xF003: IRQLatch = (V & 0xF0) >> 4; break;
	}
}

void FP_FASTAPASS(1) TH2131IRQHook(int a) {
	int count;

	if (!IRQa)
		return;

	for (count = 0; count < a; count++) {
		IRQCount++;
		if ((IRQCount & 0x0FFF) == 2048)
			IRQLatch--;
		if (!IRQLatch && (IRQCount & 0x0FFF) < 2048)
			X6502_IRQBegin(FCEU_IQEXT);
	}
}

static void TH2131Power(void) {
	VRC24PowerCommon(VRC24Write);
	SetWriteHandler(0xF000, 0xFFFF, TH2131Write);
}

void UNLTH21311_Init(CartInfo *info) {
	info->Power = TH2131Power;
	VRC24_Init(info, 1);
	MapIRQHook = TH2131IRQHook;
}

/* -------------------- UNL-KS7021A -------------------- */
/* http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_525
 * NES 2.0 Mapper 525 is used for a bootleg version of versions of Contra and 月風魔伝 (Getsu Fūma Den).
 * Its similar to Mapper 23 Submapper 3) with non-nibblized CHR-ROM bank registers.
 */

static DECLFW(KS7021AWrite) {
	switch (A & 0xB000) {
	case 0xB000: chrreg[A & 0x07] = V; Sync(); break;
	}
}

static void KS7021APower(void) {
	VRC24PowerCommon(VRC24Write);
	SetWriteHandler(0xB000, 0xBFFF, KS7021AWrite);
}

void UNLKS7021A_Init(CartInfo *info) {
	info->Power = KS7021APower;
	VRC24_Init(info, 1);
}

/* -------------------- BTL-900218 -------------------- */
/* http://wiki.nesdev.com/w/index.php/UNIF/900218
 * NES 2.0 Mapper 524 describes the PCB used for the pirate port Lord of King or Axe of Fight.
 * UNIF board name is BTL-900218.
 */

static DECLFW(BTL900218Write) {
	switch (A & 0xF00C) {
	case 0xF008: IRQa = 1; break;
	case 0xF00C: X6502_IRQEnd(FCEU_IQEXT); IRQa = 0; IRQCount = 0; break;
	}
}

void FP_FASTAPASS(1) BTL900218IRQHook(int a) {
	if (!IRQa)
		return;

	IRQCount += a;
	if (IRQCount & 1024)
		X6502_IRQBegin(FCEU_IQEXT);
}

static void BTL900218Power(void) {
	VRC24PowerCommon(VRC24Write);
	SetWriteHandler(0xF000, 0xFFFF, BTL900218Write);
}

void BTL900218_Init(CartInfo *info) {
	info->Power = BTL900218Power;
	VRC24_Init(info, 1);
	MapIRQHook = BTL900218IRQHook;
}
#endif
