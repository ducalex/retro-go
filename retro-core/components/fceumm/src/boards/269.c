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

/* NES 2.0 Mapper 269
 *   Games Xplosion 121-in-1
 *   15000-in-1
 *   18000-in-1
 */

#include "mapinc.h"
#include "mmc3.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *CHRROM;
static uint32 CHRROMSIZE;

static void M269CW(uint32 A, uint8 V) {
	uint16 NV = V;
	if (EXPREGS[2] & 8)
		NV &= (1 << ((EXPREGS[2] & 7) + 1)) - 1;
	NV |= EXPREGS[0] | ((EXPREGS[2] & 0xF0) << 4);
	setchr1(A, NV);
}

static void M269PW(uint32 A, uint8 V) {
	uint16 MV = V & ((EXPREGS[3] & 0x3F) ^ 0x3F);
	MV |= EXPREGS[1];
	MV |= ((EXPREGS[3] & 0x40) << 2);
	setprg8(A, MV);
}

static DECLFW(M269Write5) {
	if (!(EXPREGS[3] & 0x80)) {
		EXPREGS[EXPREGS[4]] = V;
		EXPREGS[4] = (EXPREGS[4] + 1) & 3;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static void M269Close(void) {
	GenMMC3Close();
	if (CHRROM)
		FCEU_free(CHRROM);
	CHRROM = NULL;
}

static void M269Reset(void) {
	EXPREGS[0] = EXPREGS[1] = EXPREGS[3] = EXPREGS[4] = 0;
	EXPREGS[2] = 0x0F;
	MMC3RegReset();
}

static void M269Power(void) {
	EXPREGS[0] = EXPREGS[1] = EXPREGS[3] = EXPREGS[4] = 0;
	EXPREGS[2] = 0x0F;
	GenMMC3Power();
	SetWriteHandler(0x5000, 0x5FFF, M269Write5);
}

static uint8 unscrambleCHR(uint8 data) {
	return 	((data & 0x01) << 6) | ((data & 0x02) << 3) | ((data & 0x04) << 0) | ((data & 0x08) >> 3) |
			((data & 0x10) >> 3) | ((data & 0x20) >> 2) | ((data & 0x40) >> 1) | ((data & 0x80) << 0);
}

void Mapper269_Init(CartInfo *info) {
	uint32 i;
	GenMMC3_Init(info, 512, 0, 8, 0);
	cwrap = M269CW;
	pwrap = M269PW;
	info->Power = M269Power;
	info->Reset = M269Reset;
	info->Close = M269Close;
	AddExState(EXPREGS, 5, 0, "EXPR");

	CHRROMSIZE = PRGsize[0];
#ifndef FCEU_NO_MALLOC
	CHRROM = (uint8*)FCEU_gmalloc(CHRROMSIZE);
#else
	CHRROM = (uint8*)ahb_calloc(1, CHRROMSIZE);
#endif
	/* unscramble CHR data from PRG */
	for (i = 0; i < CHRROMSIZE; i++)
		CHRROM[i] = unscrambleCHR(PRGptr[0][i]);
	SetupCartCHRMapping(0, CHRROM, CHRROMSIZE, 0);
	AddExState(CHRROM, CHRROMSIZE, 0, "_CHR");
}
#endif
