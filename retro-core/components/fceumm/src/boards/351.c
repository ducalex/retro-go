#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022 NewRisingSun
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

static uint8 reg[4], dip;
static uint8 MMC1_reg[4], MMC1_shift, MMC1_count, MMC1_filter;
static uint8 MMC3_reg[8], MMC3_index, MMC3_mirroring, MMC3_wram, MMC3_reload, MMC3_count, MMC3_irq;
static uint8 VRC4_prg[2];
static uint8 VRC4_mirroring;
static uint8 VRC4_misc;
static uint16 VRC4_chr[8];
static uint8 VRCIRQ_latch;
static uint8 VRCIRQ_mode;
static uint8 VRCIRQ_count;
static signed short int VRCIRQ_cycles;
static uint8 *CHRRAM =NULL;
static uint8 *PRGCHR =NULL;

static SFORMAT stateRegs[] = {
	{ reg,             4, "REGS" },
	{ &dip,            1, "DIPS" },
	{ MMC1_reg,        4, "MMC1" },
	{ &MMC1_shift,     1, "M1SH" },
	{ &MMC1_count,     1, "M1CN" },
	{ &MMC1_filter,    1, "M1FI" },
	{ MMC3_reg,        1, "MMC3" },
	{ &MMC3_index,     1, "M3IX" },
	{ &MMC3_mirroring, 1, "M3MI" },
	{ &MMC3_wram,      1, "M3WR" },
	{ &MMC3_reload,    1, "M3RL" },
	{ &MMC3_count,     1, "M3CN" },
	{ &MMC3_irq,       1, "M3IQ" },
	{ VRC4_prg,        2, "V4PR" },
	{ &VRC4_mirroring, 1, "V4MI" },
	{ &VRC4_misc,      1, "V4MS" },
	{ VRC4_chr,       16, "V4CH" },
	{ &VRCIRQ_latch,   1, "VILA" },
	{ &VRCIRQ_mode,    1, "VIMO" },
	{ &VRCIRQ_count,   1, "VICO" },
	{ &VRCIRQ_cycles,  2, "VICY" },
	{ 0 }
};


static void sync () {
	int chrAND;
	int chrOR;
	int prgAND =reg[2] &0x04? 0x0F: 0x1F;
	int prgOR  =reg[1] >>1;
	int chip   =reg[2] &0x01 && CHRRAM? 0x10: 0x00;
	
	if (reg[2] &0x10) { /* NROM mode */
		if (reg[2] &0x08) { /* NROM-64 */
			setprg8r(chip, 0x8000, prgOR);
			setprg8r(chip, 0xA000, prgOR);
			setprg8r(chip, 0xC000, prgOR);
			setprg8r(chip, 0xE000, prgOR);
		} else
		if (reg[2] &0x04) { /* NROM-128 */
			setprg16r(chip, 0x8000, prgOR >>1);
			setprg16r(chip, 0xC000, prgOR >>1);
		} else      /* NROM-256 */
			setprg32r(chip, 0x8000, prgOR >>2);
	} else
	if (~reg[0] &0x02) { /* MMC3 mode */
		setprg8r(chip, 0x8000 ^(MMC3_index <<8 &0x4000), MMC3_reg[6] &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xA000,                           MMC3_reg[7] &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xC000 ^(MMC3_index <<8 &0x4000),        0xFE &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xE000,                                  0xFF &prgAND | prgOR &~prgAND);
	} else
	if (reg[0] &0x01) { /* VRC4 mode */
		setprg8r(chip, 0x8000 ^(VRC4_misc <<13 &0x4000), VRC4_prg[0] &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xA000,                           VRC4_prg[1] &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xC000 ^(VRC4_misc <<13 &0x4000),        0xFE &prgAND | prgOR &~prgAND);
		setprg8r(chip, 0xE000,                                  0xFF &prgAND | prgOR &~prgAND);
	} else { /* MMC1 mode */
		prgAND >>=1;
		prgOR  >>=1;
		if (MMC1_reg[0] &0x8) { /* 16 KiB mode */
			if (MMC1_reg[0] &0x04) { /* OR logic */
				setprg16r(chip, 0x8000, MMC1_reg[3] &prgAND | prgOR &~prgAND);
				setprg16r(chip, 0xC000,        0xFF &prgAND | prgOR &~prgAND);
			} else {                 /* AND logic */
				setprg16r(chip, 0x8000,        0x00 &prgAND | prgOR &~prgAND);
				setprg16r(chip, 0xC000, MMC1_reg[3] &prgAND | prgOR &~prgAND);
			}
		} else
			setprg32(0x8000, (MMC1_reg[3] &prgAND | prgOR &~prgAND) >>1);
	}
	
	chrAND =reg[2] &0x10 && ~reg[2] &0x20? 0x1F: reg[2] &0x20? 0x7F: 0xFF;
	chrOR  =reg[0] <<1;
	if (reg[2] &0x01)  /* CHR RAM mode */
		setchr8r(0x10, 0);
	else
	if (reg[2] &0x40)  /* CNROM mode */
		setchr8(chrOR >>3);
	else
	if (~reg[0] &0x02) { /* MMC3 mode */
		setchr1(0x0000 ^(MMC3_index <<5 &0x1000),(MMC3_reg[0] &0xFE)&chrAND | chrOR &~chrAND);
		setchr1(0x0400 ^(MMC3_index <<5 &0x1000),(MMC3_reg[0] |0x01)&chrAND | chrOR &~chrAND);
		setchr1(0x0800 ^(MMC3_index <<5 &0x1000),(MMC3_reg[1] &0xFE)&chrAND | chrOR &~chrAND);
		setchr1(0x0C00 ^(MMC3_index <<5 &0x1000),(MMC3_reg[1] |0x01)&chrAND | chrOR &~chrAND);
		setchr1(0x1000 ^(MMC3_index <<5 &0x1000), MMC3_reg[2]       &chrAND | chrOR &~chrAND);
		setchr1(0x1400 ^(MMC3_index <<5 &0x1000), MMC3_reg[3]       &chrAND | chrOR &~chrAND);
		setchr1(0x1800 ^(MMC3_index <<5 &0x1000), MMC3_reg[4]       &chrAND | chrOR &~chrAND);
		setchr1(0x1C00 ^(MMC3_index <<5 &0x1000), MMC3_reg[5]       &chrAND | chrOR &~chrAND);
	} else
	if (reg[0] &0x01) { /* VRC4 mode */
		setchr1(0x0000, VRC4_chr[0] &chrAND | chrOR &~chrAND);
		setchr1(0x0400, VRC4_chr[1] &chrAND | chrOR &~chrAND);
		setchr1(0x0800, VRC4_chr[2] &chrAND | chrOR &~chrAND);
		setchr1(0x0C00, VRC4_chr[3] &chrAND | chrOR &~chrAND);
		setchr1(0x1000, VRC4_chr[4] &chrAND | chrOR &~chrAND);
		setchr1(0x1400, VRC4_chr[5] &chrAND | chrOR &~chrAND);
		setchr1(0x1800, VRC4_chr[6] &chrAND | chrOR &~chrAND);
		setchr1(0x1C00, VRC4_chr[7] &chrAND | chrOR &~chrAND);
	} else { /* MMC1 mode */
		chrAND >>=2;
		chrOR  >>=2;
		if (MMC1_reg[0] &0x10) { /* 4 KiB mode */
			setchr4(0x0000, MMC1_reg[1] &chrAND | chrOR &~chrAND);
			setchr4(0x1000, MMC1_reg[2] &chrAND | chrOR &~chrAND);
		} else                   /* 8 KiB mode */
			setchr8((MMC1_reg[1] &chrAND |chrOR &~chrAND) >>1);		
	}
	
	if (~reg[0] &0x02)  /* MMC3 mode */
		setmirror(MMC3_mirroring &1 ^1);
	else
	if ( reg[0] &0x01) /* VRC4 mode */
		setmirror(VRC4_mirroring &3 ^(VRC4_mirroring &2? 0: 1));
	else               /* MMC1 mode */
		setmirror(MMC1_reg[0] &3 ^3);
}

static DECLFW(writeMMC3) {
	switch(A &0xE001) {
	case 0x8000: MMC3_index =V;              sync();    break;
	case 0x8001: MMC3_reg[MMC3_index &7] =V; sync();    break;
	case 0xA000: MMC3_mirroring =V;          sync();    break;
	case 0xA001: MMC3_wram =V;               sync();    break;
	case 0xC000: MMC3_reload =V;                        break;
	case 0xC001: MMC3_count =0;                         break;
	case 0xE000: MMC3_irq =0; X6502_IRQEnd(FCEU_IQEXT); break;
	case 0xE001: MMC3_irq =1;                           break;
	}
}

static DECLFW(writeMMC1) {
	if (V &0x80) {
		MMC1_shift =MMC1_count =0;
		MMC1_reg[0] |=0x0C;
		sync();
	} else
	if (!MMC1_filter) {
		MMC1_shift |=(V &1) <<MMC1_count++;
		if (MMC1_count ==5) {
			MMC1_reg[A >>13 &3] =MMC1_shift;
			MMC1_count =0;
			MMC1_shift =0;
			sync();
		}
	}
	MMC1_filter =2;
}

static DECLFW(writeVRC4) {
	uint8 index;
	A =A &0xF000 | (A &0x800? ((A &8? 1: 0) | (A &4? 2: 0)): ((A &4? 1: 0) | (A &8? 2: 0)));
	switch (A &0xF000) {
	case 0x8000: case 0xA000:
		VRC4_prg[A >>13 &1] =V;
		sync();
		break;
	case 0x9000:
		if (~A &2)
			VRC4_mirroring =V;
		else
		if (~A &1)
			VRC4_misc =V;
		sync();
		break;
	case 0xF000:
		switch (A &3) {
		case 0: VRCIRQ_latch =VRCIRQ_latch &0xF0 | V &0x0F; break;
		case 1: VRCIRQ_latch =VRCIRQ_latch &0x0F | V <<4;   break;
		case 2: VRCIRQ_mode =V;
		        if (VRCIRQ_mode &0x02) {
				VRCIRQ_count =VRCIRQ_latch;
				VRCIRQ_cycles =341;
			}
			X6502_IRQEnd(FCEU_IQEXT);
			break;
		case 3: VRCIRQ_mode =VRCIRQ_mode &~0x02 | VRCIRQ_mode <<1 &0x02;
			X6502_IRQEnd(FCEU_IQEXT);
			break;
		}
		break;
	default:
		index =(A -0xB000) >>11 | A >>1 &1;
		if (A &1)
			VRC4_chr[index] =VRC4_chr[index] & 0x0F | V <<4;
		else
			VRC4_chr[index] =VRC4_chr[index] &~0x0F | V &0x0F;
		sync();
		break;
	}
}

static void FP_FASTAPASS(1) cpuCycle(int a) {
	if ((reg[0] &3) ==3) while (a--) { /* VRC4 mode */
		if (VRCIRQ_mode &0x02 && (VRCIRQ_mode &0x04 || (VRCIRQ_cycles -=3) <=0)) {
			if (~VRCIRQ_mode &0x04) VRCIRQ_cycles +=341;
			if (!++VRCIRQ_count) {
				VRCIRQ_count =VRCIRQ_latch;
				X6502_IRQBegin(FCEU_IQEXT);
			}
		}
	}
	if (MMC1_filter) MMC1_filter--;
}
	
static void horizontalBlanking(void) {
	if (~reg[0] &2) { /* MMC3 mode */
		MMC3_count = (MMC3_count == 0) ? MMC3_reload : (MMC3_count - 1);
		if (!MMC3_count && MMC3_irq) X6502_IRQBegin(FCEU_IQEXT);
	}
}

static void applyMode() {
	switch (reg[0] &3) {
	case 0:
	case 1: SetWriteHandler(0x8000, 0xFFFF, writeMMC3); break;
	case 2: SetWriteHandler(0x8000, 0xFFFF, writeMMC1); break;	
	case 3: SetWriteHandler(0x8000, 0xFFFF, writeVRC4); break;
	}
}

static void Mapper351_restore (int version) {
	applyMode();
	sync();
}

static DECLFR(readDIP) {
	return dip;
}

static DECLFW(writeReg) {
	uint8 previousMode =reg[0] &3;
	reg[A &3] =V;
	if ((reg[0] &3) !=previousMode) applyMode();
	sync();
}

static DECLFW(writeFDSMirroring) {
	MMC3_mirroring =V >>3 &1;
	sync();
}

static void Mapper351_power(void) {
	int i;
	for (i =0; i <4; i++) reg[i] =0;
	for (i =0; i <4; i++) MMC1_reg[i] =0;
	for (i =0; i <8; i++) MMC3_reg[i] =0;
	for (i =0; i <2; i++) VRC4_prg[i] =0;
	for (i =0; i <8; i++) VRC4_chr[i] =0;
	MMC1_shift =MMC1_count =MMC1_filter =0;
	MMC1_reg[0] =0x0C;
	MMC3_index =MMC3_mirroring =MMC3_wram =MMC3_reload =MMC3_count =MMC3_irq =0;	
	VRC4_mirroring =VRC4_misc =VRCIRQ_latch =VRCIRQ_mode =VRCIRQ_count =VRCIRQ_cycles =0;
	dip =0;
	
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetReadHandler(0x5000, 0x5FFF, readDIP);
	SetWriteHandler(0x5000, 0x5FFF, writeReg);
	SetWriteHandler(0x4025, 0x4025, writeFDSMirroring);
	applyMode();
	sync();
}

static void Mapper351_reset (void) {
	int i;
	for (i =0; i <4; i++) reg[i] =0;
	dip++;
	applyMode();
	sync();
}

static void Mapper351_close(void) {
	if (CHRRAM) FCEU_gfree(CHRRAM);
	if (PRGCHR) FCEU_gfree(PRGCHR);
	CHRRAM =NULL;
	PRGCHR =NULL;
}

void Mapper351_Init (CartInfo *info) {
	int CHRRAMSIZE =info->CHRRamSize + info->CHRRamSaveSize;
	
	info->Reset = Mapper351_reset;
	info->Power = Mapper351_power;
	info->Close = Mapper351_close;
	MapIRQHook = cpuCycle;
	GameHBIRQHook = horizontalBlanking;
	GameStateRestore = Mapper351_restore;
	AddExState(stateRegs, ~0, 0, 0);
	
	if (CHRRAMSIZE) {
#ifndef FCEU_NO_MALLOC
		CHRRAM =(uint8 *)FCEU_gmalloc(CHRRAMSIZE);
#else
		CHRRAM =(uint8 *)ahb_calloc(1, CHRRAMSIZE);
#endif
		SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
		AddExState(CHRRAM, CHRRAMSIZE, 0, "CRAM");
		
		/* This crazy thing can map CHR-ROM into CPU address space. Allocate a combined PRG+CHR address space and treat it a second "chip". */
#ifndef FCEU_NO_MALLOC
		PRGCHR =(uint8 *)FCEU_gmalloc(PRGsize[0] +CHRsize[0]);
#else
		PRGCHR =(uint8 *)ahb_calloc(1, PRGsize[0] +CHRsize[0]);
#endif
		memcpy(PRGCHR, PRGptr[0], PRGsize[0]);
		memcpy(PRGCHR +PRGsize[0], CHRptr[0], CHRsize[0]);
		SetupCartPRGMapping(0x10, PRGCHR, PRGsize[0] +CHRsize[0], 0);
	}
}

#endif
