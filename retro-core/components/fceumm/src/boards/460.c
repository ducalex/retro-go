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

static DECLFR(Mapper460_ReadOB)
{
   return X.DB;
}

static void Mapper460_PRGWrap(uint32 A, uint8 V) {
	int prgAND =0x0F;
	int prgOR  =EXPREGS[0] <<4;
	if (EXPREGS[0] &0x20 && (EXPREGS[0] !=0x20 || ~EXPREGS[1] &1)) { /* Menu selection by selectively connecting CPU D7 to reg or not */
		if (~A &0x4000) {
			setprg8(A,         (EXPREGS[0] &0x10? ~2: ~0) &V &prgAND | prgOR &~prgAND);
			setprg8(A |0x4000, (EXPREGS[0] &0x10?  2:  0) |V &prgAND | prgOR &~prgAND);
		}
	} else
		setprg8(A, V &prgAND | prgOR &~prgAND);
	
	/* Menu selection by selectively connecting reg's D7 to PRG /CE or not */
	if (EXPREGS[0] &0x80 && EXPREGS[1] &1)
		SetReadHandler(0x8000, 0xFFFF, Mapper460_ReadOB);
	else
		SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void Mapper460_CHRWrap(uint32 A, uint8 V) {
	if (EXPREGS[0] &0x04) {
		setchr2(0x0000, DRegBuf[0] &0xFE);
		setchr2(0x0800, DRegBuf[0] |0x01);
		setchr2(0x1000, DRegBuf[2]);
		setchr2(0x1800, DRegBuf[5]);
	} else
		setchr8r(0x10, 0);
}

static DECLFW(Mapper460_WriteExtra) {
	if (A001B &0x80 && ~A001B &0x40) EXPREGS[0] =A &0xFF;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void Mapper460_Reset(void) {
	EXPREGS[0] =0;
	EXPREGS[1]++;
	MMC3RegReset();
}

static void Mapper460_Power(void) {
	EXPREGS[0] =0;
	EXPREGS[1] =0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, Mapper460_WriteExtra);
}

static void Mapper460_close(void) {
	if (CHRRAM) FCEU_gfree(CHRRAM);
	CHRRAM =NULL;
}

void Mapper460_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 512, 0, 0);
	cwrap = Mapper460_CHRWrap;
	pwrap = Mapper460_PRGWrap;
	info->Power = Mapper460_Power;
	info->Reset = Mapper460_Reset;
	info->Close = Mapper460_close;
	AddExState(EXPREGS, 2, 0, "EXPR");
	
#ifndef FCEU_NO_MALLOC
	CHRRAM =(uint8 *)FCEU_gmalloc(8192);
#else
	CHRRAM =(uint8 *)ahb_calloc(1, 8192);
#endif
	SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);
	AddExState(CHRRAM, 8192, 0, "CRAM");
}
#endif
