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

/* NES 2.0 Mapper 410 is a variant of mapper 45 where the
 * ASIC's PRG A21/CHR A20 output (set by bit 6 of the third write to $6000)
 * selects between regularly-banked CHR-ROM (=0) and 8 KiB of unbanked CHR-RAM (=1).
 * It is used solely for the Super 8-in-1 - 98格鬥天王＋熱血 (JY-302) multicart.
 */

#include "mapinc.h"
#include "mmc3.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *CHRRAM;

static void M410CW(uint32 A, uint8 V) {
	if (!(EXPREGS[2] & 0x40)) {
		uint32 NV = V;
		NV &= (1 << ((EXPREGS[2] & 7) + 1)) - 1;
		NV |= EXPREGS[0] | ((EXPREGS[2] & 0xF0) << 4);
		setchr1(A, NV);
	} else
		setchr8r(0x10, 0);
}

static void M410PW(uint32 A, uint8 V) {
	uint32 MV = V & ((EXPREGS[3] & 0x3F) ^ 0x3F);
	MV |= EXPREGS[1];
	MV |= ((EXPREGS[2] & 0x40) << 2);
	setprg8(A, MV);
/*	FCEU_printf("1:%02x 2:%02x 3:%02x A=%04x V=%03x\n",EXPREGS[1],EXPREGS[2],EXPREGS[3],A,MV); */
}

static DECLFW(M410Write) {
	EXPREGS[EXPREGS[4]] = V;
	EXPREGS[4] = (EXPREGS[4] + 1) & 3;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void M410Close(void) {
	GenMMC3Close();
	if (CHRRAM)
		FCEU_free(CHRRAM);
	CHRRAM = NULL;
}

static void M410Reset(void) {
	EXPREGS[0] = EXPREGS[1] = EXPREGS[3] = EXPREGS[4] = 0;
	EXPREGS[2] = 0x0F;
	MMC3RegReset();
}

static void M410Power(void) {
	GenMMC3Power();
	EXPREGS[0] = EXPREGS[1] = EXPREGS[3] = EXPREGS[4] = 0;
	EXPREGS[2] = 0x0F;
	SetWriteHandler(0x6000, 0x7FFF, M410Write);
}

void Mapper410_Init(CartInfo *info) {
	GenMMC3_Init(info, 512, 256, 8, info->battery);
	cwrap = M410CW;
	pwrap = M410PW;
	info->Reset = M410Reset;
	info->Power = M410Power;
	info->Close = M410Close;
	AddExState(EXPREGS, 5, 0, "EXPR");

#ifndef FCEU_NO_MALLOC
	CHRRAM = (uint8*)FCEU_gmalloc(8192);
#else
	CHRRAM = (uint8*)ahb_calloc(1, 8192);
#endif
	SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);
	AddExState(CHRRAM, 8192, 0, "CRAM");
}
#endif
