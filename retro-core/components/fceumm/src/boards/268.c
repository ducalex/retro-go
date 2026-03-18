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

static uint8 *CHRRAM =NULL;
static uint8 submapper;

static void Mapper268_PRGWrap(uint32 A, uint8 V) {
	int prgMaskMMC3, prgMaskGNROM, prgOffset;
	
	prgMaskMMC3 =(EXPREGS[3] &0x10? 0x00: 0x0F) /* PRG A13-A16 */
	            |(EXPREGS[0] &0x40? 0x00: 0x10) /* PRG A17     */
	            |(EXPREGS[1] &0x80? 0x00: 0x20) /* PRG A18     */
	            |(EXPREGS[1] &0x40? 0x40: 0x00) /* PRG A19     */
	            |(EXPREGS[1] &0x20? 0x80: 0x00) /* PRG A20     */
	;
	switch(submapper &~1) {
	default:	/* Original implementation */
		prgMaskGNROM =EXPREGS[3] &0x10? (EXPREGS[1] &0x02? 0x03: 0x01): 0x00;
		prgOffset    =(EXPREGS[3]     &0x00E)
		             |(EXPREGS[0] <<4 &0x070)
			     |(EXPREGS[1] <<3 &0x080)
			     |(EXPREGS[1] <<6 &0x300)
			     |(EXPREGS[0] <<6 &0xC00);
		break;
	case 2:		/* Later revision with different arrangement of register 1 */
		prgMaskGNROM =(EXPREGS[3] &0x10)? (EXPREGS[1] &0x10? 0x01: 0x03): 0x00;
		prgOffset    =(EXPREGS[3]     &0x00E)
		             |(EXPREGS[0] <<4 &0x070)
			     |(EXPREGS[1] <<4 &0x080)
			     |(EXPREGS[1] <<6 &0x100)
			     |(EXPREGS[1] <<8 &0x200)
			     |(EXPREGS[0] <<6 &0xC00);
		break;
	case 4:		/* LD622D: PRG A20-21 moved to register 0 */
		prgMaskGNROM =EXPREGS[3] &0x10? (EXPREGS[1] &0x02? 0x03: 0x01): 0x00;
		prgOffset    =(EXPREGS[3]     &0x00E)
		             |(EXPREGS[0] <<4 &0x070)
			     |(EXPREGS[0] <<3 &0x180);
		break;
	case 6:		/* J-852C: CHR A17 selects between two PRG chips */
		prgMaskGNROM =EXPREGS[3] &0x10? (EXPREGS[1] &0x02? 0x03: 0x01): 0x00;
		prgOffset    =(EXPREGS[3]     &0x00E)
		             |(EXPREGS[0] <<4 &0x070)
			     |(EXPREGS[1] <<3 &0x080)
			     |(EXPREGS[1] <<6 &0x300)
			     |(EXPREGS[0] <<6 &0xC00);
		prgOffset &=ROM_size -1;
		if (EXPREGS[0] &0x80? !!(EXPREGS[0] &0x08): !!(DRegBuf[0] &0x80)) prgOffset |=ROM_size;
		break;
	}
	prgOffset &=~(prgMaskMMC3 | prgMaskGNROM);
	setprg8(A, (V &prgMaskMMC3) | prgOffset | (A >>13 &prgMaskGNROM));

	/* CHR-RAM write protect on submapper 8/9) */
	SetupCartCHRMapping(0, CHRptr[0], CHRsize[0], (submapper &~1) ==8 && EXPREGS[0] &0x10? 0: 1);
}

static void Mapper268_CHRWrap(uint32 A, uint8 V) {
	int chrMaskMMC3, chrMaskGNROM, chrOffset;
	
	chrMaskMMC3  =EXPREGS[3] &0x10? 0x00: EXPREGS[0] &0x80? 0x7F: 0xFF;
	chrMaskGNROM =EXPREGS[3] &0x10? 0x07: 0x00;
	chrOffset    =(EXPREGS[0] <<4 &0x380) | (EXPREGS[2] <<3 &0x078);
	chrOffset   &=~(chrMaskMMC3 | chrMaskGNROM);
	
	setchr1r(CHRRAM && EXPREGS[4] &0x01 && (V &0xFE) ==(EXPREGS[4] &0xFE)? 0x10: 0x00, A, (V &chrMaskMMC3) | chrOffset | (A >>10 &chrMaskGNROM));
}

static DECLFR(Mapper268_ReadWRAM) {
	return A001B &0xA0? CartBR(A): X.DB;
}

static DECLFW(Mapper268_WriteWRAM) {
	if ((A001B &0x80 && ~A001B &0x40) || (A001B &0x20)) CartBW(A, V);
}

static DECLFW(Mapper268_WriteReg) {
	int index =A &7;
	if (~submapper &1) Mapper268_WriteWRAM(A, V);
	if (~EXPREGS[3] &0x80 || index ==2) {
		if (index ==2) {
			if (EXPREGS[2] &0x80) V =(V &0x0F) | (EXPREGS[2] &~0x0F);
			V &=(~EXPREGS[2] >>3 &0xE) |0xF1;
		}		
		EXPREGS[index] =V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static void Mapper268_Reset(void) {
	EXPREGS[0] =EXPREGS[1] =EXPREGS[2] =EXPREGS[3] =EXPREGS[4] =EXPREGS[5] =0;
	MMC3RegReset();
}

static void Mapper268_Power(void) {	
	EXPREGS[0] =EXPREGS[1] =EXPREGS[2] =EXPREGS[3] =EXPREGS[4] =EXPREGS[5] =0;
	GenMMC3Power();
	SetReadHandler(0x6000, 0x7FFF, Mapper268_ReadWRAM);
	if (submapper &1) {
		SetWriteHandler(0x5000, 0x5FFF, Mapper268_WriteReg);
		SetWriteHandler(0x6000, 0x7FFF, Mapper268_WriteWRAM);
	} else
		SetWriteHandler(0x6000, 0x7FFF, Mapper268_WriteReg);
}

static void Mapper268_close(void) {
	if (CHRRAM) FCEU_gfree(CHRRAM);
	CHRRAM =NULL;
	GenMMC3Close();
}

void Mapper268_Init(CartInfo *info) {
	submapper = info->submapper;
	GenMMC3_Init(info, 512, 256, (info->PRGRamSize +info->PRGRamSaveSize) >>10, info->battery);
	cwrap = Mapper268_CHRWrap;
	pwrap = Mapper268_PRGWrap;
	info->Power = Mapper268_Power;
	info->Reset = Mapper268_Reset;
	info->Close = Mapper268_close;
	AddExState(EXPREGS, 8, 0, "EXPR");
	
	if (info->CHRRomSize && info->CHRRamSize + info->CHRRamSaveSize) {
#ifndef FCEU_NO_MALLOC
		CHRRAM =(uint8 *)FCEU_gmalloc(info->CHRRamSize + info->CHRRamSaveSize);
#else
		CHRRAM =(uint8 *)ahb_calloc(1, info->CHRRamSize + info->CHRRamSaveSize);
#endif
		SetupCartCHRMapping(0x10, CHRRAM, info->CHRRamSize + info->CHRRamSaveSize, 1);
		AddExState(CHRRAM, info->CHRRamSize + info->CHRRamSaveSize, 0, "CRAM");
	}
}

void COOLBOY_Init(CartInfo *info) {
	info->submapper =0;
	Mapper268_Init(info);
}

void MINDKIDS_Init(CartInfo *info) {
	info->submapper =1;
	Mapper268_Init(info);
}
#endif
