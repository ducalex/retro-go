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

/* added 2020-1-28 - */
/* NES 2.0 Mapper 372 is used for a revision of the Rockman I-VI multicart (PCB ID SFC-12).
 * It is INES Mapper 045 but with one bit of outer bank register #2 working as a CHR-ROM/RAM switch.
 */

#include "mapinc.h"
#include "mmc3.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 WRAM[0x2000];
static uint32 CHRRAMSIZE;
static uint8 *CHRRAM;

static void M372CW(uint32 A, uint8 V) {
	if (!UNIFchrrama) {
		uint32 NV = V;
		if (EXPREGS[2] & 8)
			NV &= (1 << ((EXPREGS[2] & 7) + 1)) - 1;
		else if (EXPREGS[2])
			NV &= 0;	/* hack ;( don't know exactly how it should be */
		NV |= EXPREGS[0] | ((EXPREGS[2] & 0xF0) << 4);
		if (EXPREGS[2] & 0x20)
			setchr1r(0x10, A, V);
		else
			setchr1(A, NV);
	} else
		/* setchr8(0); */		/* i don't know what cart need this, but a new one need other lol */
		setchr1(A, V);
}

static void M372PW(uint32 A, uint8 V) {
	uint32 MV = V & ((EXPREGS[3] & 0x3F) ^ 0x3F);
	MV |= EXPREGS[1];
	if(UNIFchrrama)
		MV |= ((EXPREGS[2] & 0x40) << 2);
	setprg8(A, MV);
/*	FCEU_printf("1:%02x 2:%02x 3:%02x A=%04x V=%03x\n",EXPREGS[1],EXPREGS[2],EXPREGS[3],A,MV); */
}

static DECLFW(M372Write) {
	if (EXPREGS[3] & 0x40) {
		WRAM[A - 0x6000] = V;
		return;
	}
	EXPREGS[EXPREGS[4]] = V;
	EXPREGS[4] = (EXPREGS[4] + 1) & 3;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static DECLFR(M372Read) {
	uint32 addr = 1 << (EXPREGS[5] + 4);
	if (A & (addr | (addr - 1)))
		return X.DB | 1;
	else
		return X.DB;
}

static void M372Reset(void) {
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = EXPREGS[4] = 0;
	EXPREGS[5]++;
	EXPREGS[5] &= 7;
	MMC3RegReset();
}

static void M372Power(void) {
	GenMMC3Power();
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = EXPREGS[4] = EXPREGS[5] = 0;
	SetWriteHandler(0x5000, 0x7FFF, M372Write);
	SetReadHandler(0x5000, 0x5FFF, M372Read);
}

static void M372Close(void) {
	GenMMC3Close();
	if (CHRRAM)
		FCEU_gfree(CHRRAM);
	CHRRAM = NULL;
}

void Mapper372_Init(CartInfo *info) {
	GenMMC3_Init(info, 512, 256, 8, info->battery);
	cwrap = M372CW;
	pwrap = M372PW;
	info->Reset = M372Reset;
	info->Power = M372Power;
	info->Close = M372Close;
	CHRRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	CHRRAM = (uint8*)FCEU_gmalloc(CHRRAMSIZE);
#else
	CHRRAM = (uint8*)ahb_calloc(1, CHRRAMSIZE);
#endif
	SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
	AddExState(CHRRAM, CHRRAMSIZE, 0, "CHRR");
	AddExState(EXPREGS, 5, 0, "EXPR");
}
#endif
