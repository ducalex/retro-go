#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 * Copyright (C) 2020
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

/* NES 2.0 Mapper 353 is used for the 92 Super Mario Family multicart,
 * consisting of an MMC3 clone ASIC together with a PAL.
 * The PCB code is 81-03-05-C.
 */

#include "mapinc.h"
#include "mmc3.h"
#include "../fds_apu.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8* CHRRAM = NULL;
static uint32 CHRRAMSIZE;
static uint8 PPUCHRBus;
static uint8 MIR[8];

static void M353PPU(uint32 A) {
	A &= 0x1FFF;
	A >>= 10;
	PPUCHRBus = A;
	if (EXPREGS[0] == 0)
		setmirror(MI_0 + MIR[A]);
}

static void M353PW(uint32 A, uint8 V) {
	uint8 bank = V;

	if (EXPREGS[0] == 2) {
		bank &= 0x0F;
		bank |= (DRegBuf[0] & 0x80) ? 0x10 : 0x00;
		bank |= (EXPREGS[0] << 5);
	} else {
		if ((EXPREGS[0] == 3) && !(DRegBuf[0] & 0x80)) {
			switch (A & 0xF000) {
			case 0xC000:
			case 0xE000:
				bank = DRegBuf[6 + ((A >> 13) & 1)] | 0x70;
				break;
			}
		} else {
			bank &= 0x1F;
			bank |= (EXPREGS[0] << 5);
		}
	}

	setprg8(A, bank);
}

static void M353CW(uint32 A, uint8 V) {
	if ((EXPREGS[0] == 2) && (DRegBuf[0] & 0x80))
		setchr8r(0x10, 0);
	else
		setchr1(A, (V & 0x7F) | ((EXPREGS[0]) << 7));

	MIR[A >> 10] = V >> 7;
	if ((EXPREGS[0] == 0) && (PPUCHRBus == (A >> 10)))
		setmirror(MI_0 + (V >> 7));
}

static void M353MW(uint8 V) {
	if (EXPREGS[0] != 0) {
		A000B = V;
		setmirror((V & 1) ^ 1);
	}
}

static DECLFW(M353Write) {
	if (A & 0x80) {
		EXPREGS[0] = (A >> 13) & 0x03;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	} else {
		if (A < 0xC000) {
			MMC3_CMDWrite(A, V);
			FixMMC3PRG(MMC3_cmd);
		} else
			MMC3_IRQWrite(A, V);
	}
}

static void M353Power(void) {
	FDSSoundPower();
	EXPREGS[0] = 0;
	GenMMC3Power();
	SetWriteHandler(0x8000, 0xFFFF, M353Write);
}

static void M353Reset(void) {
	EXPREGS[0] = 0;
	MMC3RegReset();
	FDSSoundReset();
}

static void M353Close(void) {
	GenMMC3Close();
	if (CHRRAM)
		FCEU_gfree(CHRRAM);
	CHRRAM = NULL;
}

void Mapper353_Init(CartInfo* info) {
	GenMMC3_Init(info, 256, 128, 8, info->battery);
	cwrap = M353CW;
	pwrap = M353PW;
	mwrap = M353MW;
	PPU_hook = M353PPU;
	info->Power = M353Power;
	info->Close = M353Close;
	info->Reset = M353Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");

	CHRRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
	CHRRAM = (uint8*)FCEU_gmalloc(CHRRAMSIZE);
#else
	CHRRAM = (uint8*)ahb_calloc(1, CHRRAMSIZE);
#endif
	SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
	AddExState(CHRRAM, CHRRAMSIZE, 0, "CHRR");
}
#endif
