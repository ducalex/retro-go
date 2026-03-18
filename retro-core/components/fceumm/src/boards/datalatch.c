#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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
#include "../fds_apu.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 latche, latcheinit, bus_conflict;
static uint16 addrreg0, addrreg1;
static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;
static void (*WSync)(void);

static DECLFW(LatchWrite) {
/*	FCEU_printf("bs %04x %02x\n",A,V); */
	if (bus_conflict)
		latche = V & CartBR(A);
	else
		latche = V;
	WSync();
}

static void LatchPower(void) {
	latche = latcheinit;
	WSync();
	if (WRAM) {
		SetReadHandler(0x6000, 0xFFFF, CartBR);
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
	} else {
		SetReadHandler(0x8000, 0xFFFF, CartBR);
	}
	SetWriteHandler(addrreg0, addrreg1, LatchWrite);
}

static void LatchClose(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void StateRestore(int version) {
	WSync();
}

static void Latch_Init(CartInfo *info, void (*proc)(void), uint8 init, uint16 adr0, uint16 adr1, uint8 wram, uint8 busc) {
	bus_conflict = busc;
	latcheinit = init;
	addrreg0 = adr0;
	addrreg1 = adr1;
	WSync = proc;
	info->Power = LatchPower;
	info->Close = LatchClose;
	GameStateRestore = StateRestore;
	if (wram) {
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
	AddExState(&latche, 1, 0, "LATC");
	AddExState(&bus_conflict, 1, 0, "BUSC");
}

/*------------------ Map 0 ---------------------------*/

#ifdef DEBUG_MAPPER
static DECLFW(NROMWrite) {
	FCEU_printf("bs %04x %02x\n", A, V);
	CartBW(A, V);
}
#endif

static void NROMPower(void) {
	setprg8r(0x10, 0x6000, 0);	/* Famili BASIC (v3.0) need it (uses only 4KB), FP-BASIC uses 8KB */
	setprg16(0x8000, 0);
	setprg16(0xC000, 1);
	setchr8(0);

	SetReadHandler(0x6000, 0x7FFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);
	SetReadHandler(0x8000, 0xFFFF, CartBR);

	FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);

	#ifdef DEBUG_MAPPER
	SetWriteHandler(0x4020, 0xFFFF, NROMWrite);
	#endif
}

void NROM_Init(CartInfo *info) {
	info->Power = NROMPower;
	info->Close = LatchClose;

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

/*------------------ Map 2 ---------------------------*/

static void UNROMSync(void) {
	setprg8r(0x10, 0x6000, 0);
	setprg16(0x8000, latche);
	setprg16(0xc000, ~0);
	setchr8(0);
}

void UNROM_Init(CartInfo *info) {
	/* By default, do not emulate bus conflicts except when explicitly told by a NES 2.0 header to do so. */
	Latch_Init(info, UNROMSync, 0, 0x8000, 0xFFFF, 1, info->iNES2 && info->submapper == 2);
}

/*------------------ Map 3 ---------------------------*/

static void CNROMSync(void) {
	setchr8(latche);
	setprg32(0x8000, 0);
	setprg8r(0x10, 0x6000, 0);	/* Hayauchy IGO uses 2Kb or RAM */
}

void CNROM_Init(CartInfo *info) {
	/* By default, do not emulate bus conflicts except when explicitly told by a NES 2.0 header to do so. */
	Latch_Init(info, CNROMSync, 0, 0x8000, 0xFFFF, 1, info->iNES2 && info->submapper == 2);
}

/*------------------ Map 7 ---------------------------*/

static void ANROMSync(void) {
	setprg32(0x8000, latche & 0xF);
	setmirror(MI_0 + ((latche >> 4) & 1));
	setchr8(0);
}

void ANROM_Init(CartInfo *info) {
	Latch_Init(info, ANROMSync, 0, 0x4020, 0xFFFF, 0, 0);
}

/*------------------ Map 8 ---------------------------*/

static void M8Sync(void) {
	setprg16(0x8000, latche >> 3);
	setprg16(0xc000, 1);
	setchr8(latche & 3);
}

void Mapper8_Init(CartInfo *info) {
	Latch_Init(info, M8Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 11 ---------------------------*/

static void M11Sync(void) {
	setprg32(0x8000, latche & 0xF);
	setchr8(latche >> 4);
}

void Mapper11_Init(CartInfo *info) {
	Latch_Init(info, M11Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

void Mapper144_Init(CartInfo *info) {
	Latch_Init(info, M11Sync, 0, 0x8001, 0xFFFF, 0, 0);
}

/*------------------ Map 13 ---------------------------*/

static void CPROMSync(void) {
	setchr4(0x0000, 0);
	setchr4(0x1000, latche & 3);
	setprg32(0x8000, 0);
}

void CPROM_Init(CartInfo *info) {
	Latch_Init(info, CPROMSync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 29 ---------------------------*/
/* added 2019-5-23
 * Mapper 28, used by homebrew game Glider
 * https://wiki.nesdev.com/w/index.php/INES_Mapper_029 */

static void M29Sync(void) {
	setprg16(0x8000, (latche >> 2) & 7);
	setprg16(0xc000, ~0);
	setchr8r(0, latche & 3);
	setprg8r(0x10, 0x6000, 0);
}

void Mapper29_Init(CartInfo *info) {
	Latch_Init(info, M29Sync, 0x0000, 0x6000, 0xFFFF, 1, 0);
}

/*------------------ Map 38 ---------------------------*/

static void M38Sync(void) {
	setprg32(0x8000, latche & 3);
	setchr8(latche >> 2);
}

void Mapper38_Init(CartInfo *info) {
	Latch_Init(info, M38Sync, 0, 0x7000, 0x7FFF, 0, 0);
}

/*------------------ Map 66 ---------------------------*/

static void MHROMSync(void) {
	setprg32(0x8000, latche >> 4);
	setchr8(latche & 0xF);
}

void MHROM_Init(CartInfo *info) {
	Latch_Init(info, MHROMSync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 70 ---------------------------*/

static void M70Sync(void) {
	setprg16(0x8000, latche >> 4);
	setprg16(0xc000, ~0);
	setchr8(latche & 0xf);
}

void Mapper70_Init(CartInfo *info) {
	Latch_Init(info, M70Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 78 ---------------------------*/
/* Should be two separate emulation functions for this "mapper".  Sigh.  URGE TO KILL RISING. */
static void M78Sync(void) {
	setprg16(0x8000, (latche & 7));
	setprg16(0xc000, ~0);
	setchr8(latche >> 4);
	setmirror(MI_0 + ((latche >> 3) & 1));
}

void Mapper78_Init(CartInfo *info) {
	Latch_Init(info, M78Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 86 ---------------------------*/

static void M86Sync(void) {
	setprg32(0x8000, (latche >> 4) & 3);
	setchr8((latche & 3) | ((latche >> 4) & 4));
}

void Mapper86_Init(CartInfo *info) {
	Latch_Init(info, M86Sync, ~0, 0x6000, 0x6FFF, 0, 0);
}

/*------------------ Map 87 ---------------------------*/

static void M87Sync(void) {
	setprg32(0x8000, 0);
	setchr8(((latche >> 1) & 1) | ((latche << 1) & 2));
}

void Mapper87_Init(CartInfo *info) {
	Latch_Init(info, M87Sync, ~0, 0x6000, 0xFFFF, 0, 0);
}

/*------------------ Map 89 ---------------------------*/

static void M89Sync(void) {
	setprg16(0x8000, (latche >> 4) & 7);
	setprg16(0xc000, ~0);
	setchr8((latche & 7) | ((latche >> 4) & 8));
	setmirror(MI_0 + ((latche >> 3) & 1));
}

void Mapper89_Init(CartInfo *info) {
	Latch_Init(info, M89Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 93 ---------------------------*/

static void SSUNROMSync(void) {
	setprg16(0x8000, latche >> 4);
	setprg16(0xc000, ~0);
	setchr8(0);
}

void SUNSOFT_UNROM_Init(CartInfo *info) {
	Latch_Init(info, SSUNROMSync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 94 ---------------------------*/

static void M94Sync(void) {
	setprg16(0x8000, latche >> 2);
	setprg16(0xc000, ~0);
	setchr8(0);
}

void Mapper94_Init(CartInfo *info) {
	Latch_Init(info, M94Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 97 ---------------------------*/

static void M97Sync(void) {
	setchr8(0);
	setprg16(0x8000, ~0);
	setprg16(0xc000, latche & 15);
	switch (latche >> 6) {
	case 0: break;
	case 1: setmirror(MI_H); break;
	case 2: setmirror(MI_V); break;
	case 3: break;
	}
	setchr8(((latche >> 1) & 1) | ((latche << 1) & 2));
}

void Mapper97_Init(CartInfo *info) {
	Latch_Init(info, M97Sync, ~0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 101 ---------------------------*/

static void M101Sync(void) {
	setprg32(0x8000, 0);
	setchr8(latche);
}

void Mapper101_Init(CartInfo *info) {
	Latch_Init(info, M101Sync, ~0, 0x6000, 0x7FFF, 0, 0);
}

/*------------------ Map 107 ---------------------------*/

static void M107Sync(void) {
	setprg32(0x8000, (latche >> 1) & 3);
	setchr8(latche & 7);
}

void Mapper107_Init(CartInfo *info) {
	Latch_Init(info, M107Sync, ~0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 113 ---------------------------*/

static void M113Sync(void) {
	setprg32(0x8000, (latche >> 3) & 7);
	setchr8(((latche >> 3) & 8) | (latche & 7));
/*	setmirror(latche>>7);*/ /* only for HES 6in1 */
}

void Mapper113_Init(CartInfo *info) {
	Latch_Init(info, M113Sync, 0, 0x4100, 0x7FFF, 0, 0);
}

/*------------------ Map 140 ---------------------------*/

void Mapper140_Init(CartInfo *info) {
	Latch_Init(info, MHROMSync, 0, 0x6000, 0x7FFF, 0, 0);
}

/*------------------ Map 152 ---------------------------*/

static void M152Sync(void) {
	setprg16(0x8000, (latche >> 4) & 7);
	setprg16(0xc000, ~0);
	setchr8(latche & 0xf);
	setmirror(MI_0 + ((latche >> 7) & 1));	/* Saint Seiya...hmm. */
}

void Mapper152_Init(CartInfo *info) {
	Latch_Init(info, M152Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 180 ---------------------------*/

static void M180Sync(void) {
	setprg16(0x8000, 0);
	setprg16(0xc000, latche);
	setchr8(0);
}

void Mapper180_Init(CartInfo *info) {
	Latch_Init(info, M180Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 184 ---------------------------*/

static void M184Sync(void) {
	setchr4(0x0000, latche);
	setchr4(0x1000, latche >> 4);
	setprg32(0x8000, 0);
}

void Mapper184_Init(CartInfo *info) {
	Latch_Init(info, M184Sync, 0, 0x6000, 0x7FFF, 0, 0);
}

/*------------------ Map 203 ---------------------------*/

static void M203Sync(void) {
	setprg16(0x8000, (latche >> 2) & 3);
	setprg16(0xC000, (latche >> 2) & 3);
	setchr8(latche & 3);
}

void Mapper203_Init(CartInfo *info) {
	Latch_Init(info, M203Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 240 ---------------------------*/

static void M240Sync(void) {
	setprg8r(0x10, 0x6000, 0);
	setprg32(0x8000, latche >> 4);
	setchr8(latche & 0xF);
}

void Mapper240_Init(CartInfo *info) {
	Latch_Init(info, M240Sync, 0, 0x4020, 0x5FFF, 1, 0);
}

/*------------------ Map 241 ---------------------------*/
/* Mapper 7 mostly, but with SRAM or maybe prot circuit
 * figure out, which games do need 5xxx area reading
 */

static void M241Sync(void) {
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
	if (latche & 0x80)
		setprg32(0x8000, latche | 8);	/* no 241 actually, but why not afterall? */
	else
		setprg32(0x8000, latche);
}

void Mapper241_Init(CartInfo *info) {
	Latch_Init(info, M241Sync, 0, 0x8000, 0xFFFF, 1, 0);
}

/*------------------ Map 271 ---------------------------*/

static void M271Sync(void) {
	setchr8(latche & 0x0F);
	setprg32(0x8000, latche >> 4);
	setmirror((latche >> 5) & 1);
}

void Mapper271_Init(CartInfo *info) {
	Latch_Init(info, M271Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Map 381 ---------------------------*/
/* 2-in-1 High Standard Game (BC-019), reset-based */
static uint8 reset = 0;
static void M381Sync(void) {
	setprg16(0x8000, ((latche & 0x10) >> 4) | ((latche & 7) << 1) | (reset << 4));
	setprg16(0xC000, 15 | (reset << 4));
	setchr8(0);
}

static void M381Reset(void) {
	reset ^= 1;
	M381Sync();
}

void Mapper381_Init(CartInfo *info) {
	info->Reset = M381Reset;
	Latch_Init(info, M381Sync, 0, 0x8000, 0xFFFF, 1, 0);
	AddExState(&reset, 1, 0, "RST0");
}

/*------------------ Map 538 ---------------------------*/
/* NES 2.0 Mapper 538 denotes the 60-1064-16L PCB, used for a
 * bootleg cartridge conversion named Super Soccer Champion
 * of the Konami FDS game Exciting Soccer.
 */
static uint8 M538Banks[16] = { 0, 1, 2, 1, 3, 1, 4, 1, 5, 5, 1, 1, 6, 6, 7, 7 };
static void M538Sync(void) {
	setprg8(0x6000, (latche >> 1) | 8);
	setprg8(0x8000, M538Banks[latche & 15]);
	setprg8(0xA000, 14);
	setprg8(0xC000, 7);
	setprg8(0xE000, 15);
	setchr8(0);
	setmirror(1);
}

static void M538Power(void) {
	FDSSoundPower();
	LatchPower();
}

void Mapper538_Init(CartInfo *info) {
	Latch_Init(info, M538Sync, 0, 0xC000, 0xCFFF, 1, 0);
	info->Power = M538Power;
}

/* ------------------ A65AS --------------------------- */

/* actually, there is two cart in one... First have extra mirroring
 * mode (one screen) and 32K bankswitching, second one have only
 * 16 bankswitching mode and normal mirroring... But there is no any
 * correlations between modes and they can be used in one mapper code.
 *
 * Submapper 0 - 3-in-1 (N068)
 * Submapper 0 - 3-in-1 (N080)
 * Submapper 1 - 4-in-1 (JY-066)
 */

static int A65ASsubmapper;
static void BMCA65ASSync(void) {
	if (latche & 0x40)
		setprg32(0x8000, (latche >> 1) & 0x0F);
	else {
		if (A65ASsubmapper == 1) {
			setprg16(0x8000, ((latche & 0x30) >> 1) | (latche & 7));
			setprg16(0xC000, ((latche & 0x30) >> 1) | 7);
		} else {
			setprg16(0x8000, latche & 0x0F);
			setprg16(0xC000, latche & 0x0F | 7);
		}
	}
	setchr8(0);
	if (latche & 0x80)
		setmirror(MI_0 + (((latche >> 5) & 1)));
	else
		setmirror(((latche >> 3) & 1) ^ 1);
}

void BMCA65AS_Init(CartInfo *info) {
	A65ASsubmapper = info->submapper;
	Latch_Init(info, BMCA65ASSync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ BMC-11160 ---------------------------*/
/* Simple BMC discrete mapper by TXC */

static void BMC11160Sync(void) {
	uint32 bank = (latche >> 4) & 7;
	setprg32(0x8000, bank);
	setchr8((bank << 2) | (latche & 3));
	setmirror((latche >> 7) & 1);
}

void BMC11160_Init(CartInfo *info) {
	Latch_Init(info, BMC11160Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ BMC-K-3046 ---------------------------*/
/* NES 2.0 mapper 336 is used for an 11-in-1 multicart
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_336 */

static void BMCK3046Sync(void) {
	setprg16(0x8000, latche);
	setprg16(0xC000, latche | 0x07);
	setchr8(0);
}

void BMCK3046_Init(CartInfo *info) {
	Latch_Init(info, BMCK3046Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/* Mapper 429: LIKO BBG-235-8-1B/Milowork FCFC1 */

static void Mapper429_Sync(void) {
	setprg32(0x8000, latche >>2);
	setchr8(latche);
}

static void Mapper429_Reset(void) {
	latche = 4; /* Initial CHR bank 0, initial PRG bank 1 */
	Mapper429_Sync();
}

void Mapper429_Init(CartInfo *info) {
	info->Reset = Mapper429_Reset;
	Latch_Init(info, Mapper429_Sync, 0, 0x8000, 0xFFFF, 0, 0);
}

/*------------------ Mapper 415 ---------------------------*/

static void Mapper415_Sync(void) {
	setprg8(0x6000, latche & 0x0F);
	setprg32(0x8000, ~0);
	setchr8(0);
	setmirror(((latche >> 4) & 1) ^ 1);
}

static void M415Power(void) {
	LatchPower();
	SetReadHandler(0x6000, 0x7FFF, CartBR);
}

void Mapper415_Init(CartInfo *info) {
	Latch_Init(info, Mapper415_Sync, 0, 0x8000, 0xFFFF, 0, 0);
	info->Power = M415Power;
}
#endif
