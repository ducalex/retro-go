#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020
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

/* Mapper 370 - F600
 * Golden Mario Party II - Around the World (6-in-1 multicart)
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 PPUCHRBus;
static uint8 mirr[8];

static void FP_FASTAPASS(1) M370PPU(uint32 A) {
	if ((EXPREGS[0] & 7) == 1) {
		A &= 0x1FFF;
		A >>= 10;
		PPUCHRBus = A;
		setmirror(MI_0 + mirr[A]);
	}
}

static void M370CW(uint32 A, uint8 V) {
	uint8 mask = (EXPREGS[0] & 4) ? 0x7F : 0xFF;
	/* FIXME: Mario VII, mask is reversed? */
	if ((EXPREGS[0] & 7) == 6 && V & 0x80)
		mask = 0xFF;
	mirr[A >> 10] = V >> 7;
	setchr1(A, (V & mask) | ((EXPREGS[0] & 7) << 7));
	if (((EXPREGS[0] & 7) == 1) && (PPUCHRBus == (A >> 10)))
		setmirror(MI_0 + (V >> 7));
}

static void M370PW(uint32 A, uint8 V) {
	uint8 mask = EXPREGS[0] & 0x20 ? 0x0F : 0x1F;
	setprg8(A, (V & mask) | ((EXPREGS[0] & 0x38) << 1));
}

static void M370MW(uint8 V) {
	A000B = V;
	if ((EXPREGS[0] & 7) != 1)
		setmirror((V & 1) ^ 1);
}

static DECLFR(M370Read) {;
	return (EXPREGS[1] << 7);
}

static DECLFW(M370Write) {
	EXPREGS[0] = (A & 0xFF);
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void M370Reset(void) {
	EXPREGS[0] = 0;
	EXPREGS[1] ^= 1;
	FCEU_printf("solderpad=%02x\n", EXPREGS[1]);
	MMC3RegReset();
}

static void M370Power(void) {
	EXPREGS[0] = 0;
	EXPREGS[1] = 1; /* start off with the 6-in-1 menu */
	GenMMC3Power();
	SetReadHandler(0x5000, 0x5FFF, M370Read);
	SetWriteHandler(0x5000, 0x5FFF, M370Write);
}

void Mapper370_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 256, 8, 0);
	cwrap = M370CW;
	pwrap = M370PW;
	mwrap = M370MW;
	PPU_hook = M370PPU;
	info->Power = M370Power;
	info->Reset = M370Reset;
	AddExState(EXPREGS, 2, 0, "EXPR");
}
#endif
