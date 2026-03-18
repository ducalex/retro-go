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
 */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint16 latche, latcheinit;
static uint16 addrreg0, addrreg1;
static uint8 dipswitch;
static void (*WSync)(void);
static readfunc defread;
static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;
static uint32 hasBattery;
static uint32 submapper = 0;

static DECLFW(LatchWrite) {
	latche = A;
	WSync();
}

static void LatchReset(void) {
	latche = latcheinit;
	WSync();
}

static void LatchPower(void) {
	latche = latcheinit;
	WSync();
	if (WRAM) {
		SetReadHandler(0x6000, 0xFFFF, CartBR);
		SetWriteHandler(0x6000, 0x7FFF, CartBW);
		FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
	} else
		SetReadHandler(0x6000, 0xFFFF, defread);
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

static void Latch_Init(CartInfo *info, void (*proc)(void), readfunc func, uint16 linit, uint16 adr0, uint16 adr1, uint8 wram) {
	latcheinit = linit;
	addrreg0 = adr0;
	addrreg1 = adr1;
	WSync = proc;
	hasBattery = 0;
	if (func != NULL)
		defread = func;
	else
		defread = CartBROB;
	info->Power = LatchPower;
	info->Reset = LatchReset;
	info->Close = LatchClose;
	if (wram) {
		WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
		WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
		WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
		SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
		if (info->battery) {
			hasBattery = 1;
			info->SaveGame[0] = WRAM;
			info->SaveGameLen[0] = WRAMSIZE;
		}
		AddExState(WRAM, WRAMSIZE, 0, "WRAM");
	}
	GameStateRestore = StateRestore;
	AddExState(&latche, 2, 0, "LATC");
}

/*------------------ BMCD1038 ---------------------------*/

static void BMCD1038Sync(void) {
	if (latche & 0x80) {
		setprg16(0x8000, (latche & 0x70) >> 4);
		setprg16(0xC000, (latche & 0x70) >> 4);
	} else
		setprg32(0x8000, (latche & 0x60) >> 5);
	setchr8(latche & 7);
	setmirror(((latche & 8) >> 3) ^ 1);
}

static DECLFR(BMCD1038Read) {
	if (latche & 0x100)
		return dipswitch;
	else
		return CartBR(A);
}

static DECLFW(BMCD1038Write) {
	/* Only recognize the latch write if the lock bit has not been set. Needed for NT-234 "Road Fighter" */
	if (~latche & 0x200)
		LatchWrite(A, V);
}

static void BMCD1038Reset(void) {
	dipswitch++;
	dipswitch &= 3;
	
	/* Always reset to menu */
	latche = 0;
	BMCD1038Sync();
}

static void BMCD1038Power(void) {
	LatchPower();
	
	/* Trap latch writes to enforce the "Lock" bit */
	SetWriteHandler(0x8000, 0xFFFF, BMCD1038Write);
}

void BMCD1038_Init(CartInfo *info) {
	Latch_Init(info, BMCD1038Sync, BMCD1038Read, 0x0000, 0x8000, 0xFFFF, 0);
	info->Reset = BMCD1038Reset;
	info->Power = BMCD1038Power;
	AddExState(&dipswitch, 1, 0, "DIPSW");
}

/*------------------ Map 058 ---------------------------*/

static void M58Sync(void) {
	if (latche & 0x40) {
		setprg16(0x8000, latche & 7);
		setprg16(0xC000, latche & 7);
	} else
		setprg32(0x8000, (latche >> 1) & 3);
	setchr8((latche >> 3) & 7);
	setmirror(((latche & 0x80) >> 7) ^ 1);
}

void Mapper58_Init(CartInfo *info) {
	Latch_Init(info, M58Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 059 ---------------------------*/
/* One more forgotten mapper */
/* Formerly, an incorrect implementation of BMC-T3H53 */
/*static void M59Sync(void) {
	setprg32(0x8000, (latche >> 4) & 7);
	setchr8(latche & 0x7);
	setmirror((latche >> 3) & 1);
}

static DECLFR(M59Read) {
	if (latche & 0x100)
		return 0;
	else
		return CartBR(A);
}

void Mapper59_Init(CartInfo *info) {
	Latch_Init(info, M59Sync, M59Read, 0x0000, 0x8000, 0xFFFF, 0);
}*/

/*------------------ Map 061 ---------------------------*/
static void M61Sync(void) {
	if (((latche & 0x10) << 1) ^ (latche & 0x20)) {
		setprg16(0x8000, ((latche & 0xF) << 1) | (((latche & 0x20) >> 4)));
		setprg16(0xC000, ((latche & 0xF) << 1) | (((latche & 0x20) >> 4)));
	} else
		setprg32(0x8000, latche & 0xF);
	setchr8(0);
	setmirror(((latche >> 7) & 1) ^ 1);
}

void Mapper61_Init(CartInfo *info) {
	Latch_Init(info, M61Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 063 ---------------------------*/
/* added 2019-5-23
 * Mapper 63 NTDEC-Multicart
 * http://wiki.nesdev.com/w/index.php/INES_Mapper_063
 * - Powerful 250-in-1
 * - Hello Kitty 255-in-1
 * Submapper 1:
 * - NTDEC 82-in-1 */

static uint16 openBus;

static DECLFR(M63Read) {
	if (openBus)
		return X.DB;
	return CartBR(A);
}

static void M63Sync(void) {
	uint16 prg =latche >>2 &(submapper ==1? 0x7F: 0xFF);
	if (latche &2)
		setprg32(0x8000, prg >>1);
	else {
		setprg16(0x8000, prg);
		setprg16(0xC000, prg);
	}
	openBus =prg >=ROM_size;
	SetupCartCHRMapping(0, CHRptr[0], 0x2000, latche &(submapper ==1? 0x200: 0x400)? 0: 1);
	setchr8(0);
	setmirror(latche &1? MI_H: MI_V);
}

void Mapper63_Init(CartInfo *info) {
	submapper = info->submapper;
	Latch_Init(info, M63Sync, M63Read, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 092 ---------------------------*/
/* Another two-in-one mapper, two Jaleco carts uses similar
 * hardware, but with different wiring.
 * Original code provided by LULU
 * Additionally, PCB contains DSP extra sound chip, used for voice samples (unemulated)
 */

static void M92Sync(void) {
	uint8 reg = latche & 0xF0;
	setprg16(0x8000, 0);
	if (latche >= 0x9000) {
		switch (reg) {
		case 0xD0: setprg16(0xc000, latche & 15); break;
		case 0xE0: setchr8(latche & 15); break;
		}
	} else {
		switch (reg) {
		case 0xB0: setprg16(0xc000, latche & 15); break;
		case 0x70: setchr8(latche & 15); break;
		}
	}
}

void Mapper92_Init(CartInfo *info) {
	Latch_Init(info, M92Sync, NULL, 0x80B0, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 200 ---------------------------*/

static void M200Sync(void) {
	setprg16(0x8000, latche);
	setprg16(0xC000, latche);
	setchr8(latche);
	setmirror(latche &(submapper ==1? 4: 8)? MI_H: MI_V);
}

void Mapper200_Init(CartInfo *info) {
	submapper = info->submapper;
	Latch_Init(info, M200Sync, NULL, 0xFFFF, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 201 ---------------------------*/
/* 2020-3-6 - Support for 21-in-1 (CF-043) (2006-V) (Unl) [p1].nes which has mixed mirroring
 * found at the time labeled as submapper 15
 * 0x05658DED 128K PRG, 32K CHR */
static void M201Sync(void) {
	if (latche & 8 || submapper == 15) {
		setprg32(0x8000, latche & 3);
		setchr8(latche & 3);
	} else {
		setprg32(0x8000, 0);
		setchr8(0);
	}
	if (submapper == 15)
		setmirror(((latche & 0x07) == 0x07) ? MI_V : MI_H);
}

void Mapper201_Init(CartInfo *info) {
	submapper = info->submapper;
	Latch_Init(info, M201Sync, NULL, 0xFFFF, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 202 ---------------------------*/

static void M202Sync(void) {
	/* According to more carefull hardware tests and PCB study */
	int32 mirror = latche & 1;
	int32 bank = (latche >> 1) & 0x7;
	int32 select = (mirror & (bank >> 2));
	setprg16(0x8000, select ? (bank & 6) | 0 : bank);
	setprg16(0xc000, select ? (bank & 6) | 1 : bank);
	setmirror(mirror ^ 1);
	setchr8(bank);
}

void Mapper202_Init(CartInfo *info) {
	Latch_Init(info, M202Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 204 ---------------------------*/

static void M204Sync(void) {
	int32 tmp2 = latche & 0x6;
	int32 tmp1 = tmp2 + ((tmp2 == 0x6) ? 0 : (latche & 1));
	setprg16(0x8000, tmp1);
	setprg16(0xc000, tmp2 + ((tmp2 == 0x6) ? 1 : (latche & 1)));
	setchr8(tmp1);
	setmirror(((latche >> 4) & 1) ^ 1);
}

void Mapper204_Init(CartInfo *info) {
	Latch_Init(info, M204Sync, NULL, 0xFFFF, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 212 ---------------------------*/

static DECLFR(M212Read) {
	uint8 ret = CartBROB(A);
	if ((A & 0xE010) == 0x6000)
		ret |= 0x80;
	return ret;
}

static void M212Sync(void) {
	if (latche & 0x4000) {
		setprg32(0x8000, (latche >> 1) & 3);
	} else {
		setprg16(0x8000, latche & 7);
		setprg16(0xC000, latche & 7);
	}
	setchr8(latche & 7);
	setmirror(((latche >> 3) & 1) ^ 1);
}

void Mapper212_Init(CartInfo *info) {
	Latch_Init(info, M212Sync, M212Read, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 213 ---------------------------*/

/*                SEE MAPPER 58                         */

/*------------------ Map 214 ---------------------------*/

static void M214Sync(void) {
	setprg16(0x8000, (latche >> 2) & 3);
	setprg16(0xC000, (latche >> 2) & 3);
	setchr8(latche & 3);
}

void Mapper214_Init(CartInfo *info) {
	Latch_Init(info, M214Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 217 ---------------------------*/

static void M217Sync(void) {
	setprg32(0x8000, (latche >> 2) & 0x03);
	setchr8(latche & 0x0F);
}

void Mapper217_Init(CartInfo *info) {
	Latch_Init(info, M217Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 227 ---------------------------*/

static void M227Sync(void) {
	uint32 S = latche & 1;
	uint32 p = ((latche >> 2) & 0x1F) + ((latche & 0x100) >> 3);
	uint32 L = (latche >> 9) & 1;

	if ((latche >> 7) & 1) {
		if (S) {
			setprg32(0x8000, p >> 1);
		} else {
			setprg16(0x8000, p);
			setprg16(0xC000, p);
		}
	} else {
		if (S) {
			if (L) {
				setprg16(0x8000, p & 0x3E);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p & 0x3E);
				setprg16(0xC000, p & 0x38);
			}
		} else {
			if (L) {
				setprg16(0x8000, p);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p);
				setprg16(0xC000, submapper ==2? 0: p & 0x38);
			}
		}
	}

	if (!hasBattery && (latche & 0x80) == 0x80)
		/* CHR-RAM write protect hack, needed for some multicarts */
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 0);
	else
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 1);

	setmirror(((latche >> 1) & 1) ^ 1);
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
}

static DECLFR(M227Read) {
	if (latche &0x0400)
		return CartBR(A | dipswitch);
	else
		return CartBR(A);
}

static void Mapper227_Reset(void) {
	dipswitch++;
	dipswitch &= 15;
	latche = 0;
	M227Sync();
}

void Mapper227_Init(CartInfo *info) {
	submapper =info->submapper;
	Latch_Init(info, M227Sync, M227Read, 0x0000, 0x8000, 0xFFFF, info->iNES2 && (info->PRGRamSize || info->PRGRamSaveSize) || info->battery);
	info->Reset = Mapper227_Reset;
	AddExState(&dipswitch, 1, 0, "DIPSW");
}

/*------------------ Map 229 ---------------------------*/

static void M229Sync(void) {
	setchr8(latche);
	if (!(latche & 0x1e))
		setprg32(0x8000, 0);
	else {
		setprg16(0x8000, latche & 0x1F);
		setprg16(0xC000, latche & 0x1F);
	}
	setmirror(((latche >> 5) & 1) ^ 1);
}

void Mapper229_Init(CartInfo *info) {
	Latch_Init(info, M229Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 231 ---------------------------*/

static void M231Sync(void) {
	setchr8(0);
	if (latche & 0x20)
		setprg32(0x8000, (latche >> 1) & 0x0F);
	else {
		setprg16(0x8000, latche & 0x1E);
		setprg16(0xC000, latche & 0x1E);
	}
	setmirror(((latche >> 7) & 1) ^ 1);
}

void Mapper231_Init(CartInfo *info) {
	Latch_Init(info, M231Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 242 ---------------------------*/
static uint8 M242TwoChips;
static void M242Sync(void) {
	uint32 S = latche & 1;
	uint32 p = (latche >> 2) & 0x1F;
	uint32 L = (latche >> 9) & 1;
	
	if (M242TwoChips) {
		if (latche &0x600)
		{	/* First chip */
			p &= 0x1F; 
		}
		else
		{	/* Second chip */
			p &= 0x07;
			p += 0x20;
		}
	}

	if ((latche >> 7) & 1) {
		if (S) {
			setprg32(0x8000, p >> 1);
		} else {
			setprg16(0x8000, p);
			setprg16(0xC000, p);
		}
	} else {
		if (S) {
			if (L) {
				setprg16(0x8000, p & 0x3E);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p & 0x3E);
				setprg16(0xC000, p & 0x38);
			}
		} else {
			if (L) {
				setprg16(0x8000, p);
				setprg16(0xC000, p | 7);
			} else {
				setprg16(0x8000, p);
				setprg16(0xC000, p & 0x38);
			}
		}
	}

	if (!hasBattery && (latche & 0x80) == 0x80 && (ROM_size * 16) > 256)
		/* CHR-RAM write protect hack, needed for some multicarts */
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 0);
	else
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 1);

	setmirror(((latche >> 1) & 1) ^ 1);
	setchr8(0);
	setprg8r(0x10, 0x6000, 0);
}

static DECLFR(M242Read) {
	if (latche &0x0100)
		return CartBR(A | dipswitch);
	else
		return CartBR(A);
}

static void Mapper242_Reset(void) {
	dipswitch++;
	dipswitch &= 31;
	latche = 0;
	M242Sync();
}

void Mapper242_Init(CartInfo *info) {
	M242TwoChips = info->PRGRomSize &0x20000 && info->PRGRomSize >0x20000;
	Latch_Init(info, M242Sync, M242Read, 0x0000, 0x8000, 0xFFFF,  info->iNES2 && (info->PRGRamSize || info->PRGRamSaveSize) || info->battery);
	info->Reset = Mapper242_Reset;
	AddExState(&dipswitch, 1, 0, "DIPSW");
}

/*------------------ Map 288 ---------------------------*/
/* NES 2.0 Mapper 288 is used for two GKCX1 21-in-1 multicarts
 * - 21-in-1 (GA-003)
 * - 64-in-1 (CF-015)
 */
static void M288Sync(void) {
	setchr8(latche & 7);
	setprg32(0x8000, (latche >> 3) & 3);
}

static DECLFR(M288Read) {
	uint8 ret = CartBR(A);
	if (latche & 0x20)
		ret |= (dipswitch << 2);
	return ret;
}

static void M288Reset(void) {
	dipswitch++;
	dipswitch &= 3;
	M288Sync();
}

void Mapper288_Init(CartInfo *info) {
	dipswitch = 0;
	Latch_Init(info, M288Sync, M288Read, 0x0000, 0x8000, 0xFFFF, 0);
	info->Reset = M288Reset;
	AddExState(&dipswitch, 1, 0, "DIPSW");
}

/*------------------ Map 385 ---------------------------*/

static void M385Sync(void) {
	int32 mirror = latche & 1;
	int32 bank = (latche >> 1) & 0x7;
	setprg16(0x8000, bank);
	setprg16(0xc000, bank);
	setmirror(mirror ^ 1);
	setchr8(0);
}

void Mapper385_Init(CartInfo *info) {
	Latch_Init(info, M385Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*------------------ Map 541 ---------------------------*/
/* LittleCom 160-in-1 multicart */
static void M541Sync(void) {
	if (latche & 2) {
		/* NROM-128 */
		setprg16(0x8000, latche >> 2);
		setprg16(0xC000, latche >> 2);
	} else {
		/* NROM=256 */
		setprg32(0x8000, latche >> 3);
	}
	setchr8(0);
	setmirror(latche & 1);
}

void Mapper541_Init(CartInfo *info) {
	Latch_Init(info, M541Sync, NULL, 0x0000, 0xC000, 0xFFFF, 0);
}

/*------------------ 190in1 ---------------------------*/

static void BMC190in1Sync(void) {
	setprg16(0x8000, (latche >> 2) & 7);
	setprg16(0xC000, (latche >> 2) & 7);
	setchr8((latche >> 2) & 7);
	setmirror((latche & 1) ^ 1);
}

void BMC190in1_Init(CartInfo *info) {
	Latch_Init(info, BMC190in1Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*-------------- BMC810544-C-A1 ------------------------*/

static void BMC810544CA1Sync(void) {
	uint32 bank = latche >> 7;
	if (latche & 0x40)
		setprg32(0x8000, bank);
	else {
		setprg16(0x8000, (bank << 1) | ((latche >> 5) & 1));
		setprg16(0xC000, (bank << 1) | ((latche >> 5) & 1));
	}
	setchr8(latche & 0x0f);
	setmirror(((latche >> 4) & 1) ^ 1);
}

void BMC810544CA1_Init(CartInfo *info) {
	Latch_Init(info, BMC810544CA1Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*-------------- BMCNTD-03 ------------------------*/

static void BMCNTD03Sync(void) {
	/* 1PPP Pmcc spxx xccc
	 * 1000 0000 0000 0000 v
	 * 1001 1100 0000 0100 h
	 * 1011 1010 1100 0100
	 */
	uint32 prg = ((latche >> 10) & 0x1e);
	uint32 chr = ((latche & 0x0300) >> 5) | (latche & 7);
	if (latche & 0x80) {
		setprg16(0x8000, prg | ((latche >> 6) & 1));
		setprg16(0xC000, prg | ((latche >> 6) & 1));
	} else
		setprg32(0x8000, prg >> 1);
	setchr8(chr);
	setmirror(((latche >> 10) & 1) ^ 1);
}

void BMCNTD03_Init(CartInfo *info) {
	Latch_Init(info, BMCNTD03Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*-------------- BMCG-146 ------------------------*/

static void BMCG146Sync(void) {
	setchr8(0);
	if (latche & 0x800) {		/* UNROM mode */
		setprg16(0x8000, (latche & 0x1F) | (latche & ((latche & 0x40) >> 6)));
		setprg16(0xC000, (latche & 0x18) | 7);
	} else {
		if (latche & 0x40) {	/* 16K mode */
			setprg16(0x8000, latche & 0x1F);
			setprg16(0xC000, latche & 0x1F);
		} else {
			setprg32(0x8000, (latche >> 1) & 0x0F);
		}
	}
	setmirror(((latche & 0x80) >> 7) ^ 1);
}

void BMCG146_Init(CartInfo *info) {
	Latch_Init(info, BMCG146Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*-------------- BMC-TJ-03 ------------------------*/
/* NES 2.0 mapper 341 is used for a simple 4-in-1 multicart */

static void BMCTJ03Sync(void) {
	uint8 mirr = latche &(PRGsize[0] &0x40000? 0x800: 0x200)? MI_H: MI_V;
	uint8 bank = latche >> 8;

	setprg32(0x8000, bank);
	setchr8(bank);

	setmirror(mirr);
}

void BMCTJ03_Init(CartInfo *info) {
	Latch_Init(info, BMCTJ03Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/*-------------- BMC-SA005-A ------------------------*/
/* NES 2.0 mapper 338 is used for a 16-in-1 and a 200/300/600/1000-in-1 multicart.
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_338 */

static void BMCSA005ASync(void) {
	setprg16(0x8000, latche & 0x0F);
	setprg16(0xC000, latche & 0x0F);
	setchr8(latche & 0x0F);
	setmirror((latche >> 3) & 1);
}

void BMCSA005A_Init(CartInfo *info) {
	Latch_Init(info, BMCSA005ASync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/* -------------- 831019C J-2282 ------------------------ */

static void J2282Sync(void)
{
    setchr8(0);

    if ((latche & 0x40)) {
        uint8 bank = (latche >> 0) & 0x1F;
        setprg16(0x8000, bank);
        setprg16(0xC000, bank);
    }
    else
    {
        uint8 bank;
        if (latche & 0x800)
	{
            setprg8(0x6000, ((latche << 1) & 0x3F) | 3);
        }
        bank = (latche >> 1) & 0x1F;
        setprg32(0x8000, bank);
    }

    if (latche & 0x80)
        setmirror(0);
    else
        setmirror(1);
}

void J2282_Init(CartInfo *info)
{
    Latch_Init(info, J2282Sync, NULL, 0x0000, 0x8000, 0xFFFF, 0);
}

/* -------------- Mapper 409 ------------------------ */
static void M409Sync(void) {
	setprg16(0x8000, latche);
	setprg16(0xC000, ~0);
	setchr8(0);
}

void Mapper409_Init(CartInfo *info) {
	Latch_Init(info, M409Sync, NULL, 0x0000, 0xC000, 0xCFFF, 0);
}

/*------------------ Map 435 ---------------------------*/
static void M435Sync(void) {
	int p =latche >>2 &0x1F | latche >>3 &0x20 | latche >>4 &0x40;
	if (latche &0x200) {
		if (latche &0x001) {
			setprg16(0x8000, p);
			setprg16(0xC000, p);
		} else {
			setprg32(0x8000, p >> 1);
		}
	} else {
		setprg16(0x8000, p);
		setprg16(0xC000, p | 7);
	}

	if (latche &0x200)
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 0);
	else
		SetupCartCHRMapping(0, CHRptr[0], 0x2000, 1);

	setmirror(latche &0x002? MI_H: MI_V);
	setchr8(0);
}

void Mapper435_Init(CartInfo *info) {
	Latch_Init(info, M435Sync, NULL, 0x0000, 0x8000, 0xFFFF, 1);
}

/*------------------ Map 459 ---------------------------*/
static void M459Sync(void) {
	int p =latche >>5;
	int c =latche &0x03 | latche >>2 &0x04 | latche >>4 &0x08;
	if (latche &0x04) {
		setprg32(0x8000, p);
	} else {
		setprg16(0x8000, p <<1);
		setprg16(0xC000, p <<1 |7);
	}
	setchr8(c &(latche &0x08? 0x0F: 0x08));
	setmirror(latche &0x100? MI_H: MI_V);
}

void Mapper459_Init(CartInfo *info) {
	Latch_Init(info, M459Sync, NULL, 0x0000, 0x8000, 0xFFFF, 1);
}

/*------------------ Map 461 ---------------------------*/
static void M461Sync(void) {
	int p =latche <<1 | latche >>5 &1;
	int c =latche >>8;
	if (latche &0x10) {
		setprg16(0x8000, p);
		setprg16(0xC000, p);
	} else {
		setprg32(0x8000, p >>1);
	}
	setchr8(c);
	setmirror(latche &0x80? MI_H: MI_V);
}

void Mapper461_Init(CartInfo *info) {
	Latch_Init(info, M461Sync, NULL, 0x0000, 0x8000, 0xFFFF, 1);
}

/*------------------ Map 464 ---------------------------*/
static void M464Sync(void) {
	int p =latche >>7;
	int c =latche &0x1F;
	if (latche &0x40) {
		setprg32(0x8000, p >> 1);
	} else {
		setprg16(0x8000, p);
		setprg16(0xC000, p);
	}
	setchr8(c);
	setmirror(latche &0x20? MI_H: MI_V);
}

void Mapper464_Init(CartInfo *info) {
	Latch_Init(info, M464Sync, NULL, 0x0000, 0x8000, 0xFFFF, 1);
}
#endif
