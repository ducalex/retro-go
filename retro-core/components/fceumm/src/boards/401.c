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
 *
 * NES 2.0 - Mapper 401 (reference from NewRisingSun)
 * Super 19-in-1 (VIP19) (crc 0x2F497313)
 *
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 dipswitch = 0;

static void M401CW(uint32 A, uint8 V) {
	uint32 mask = (0xFF >> (~EXPREGS[2] & 0xF));
	uint32 bank  = (EXPREGS[0] | ((EXPREGS[2] << 4) & 0xF00));
	setchr1(A, (V & mask) | bank);
}

static void M401PW(uint32 A, uint8 V) {
	if ((dipswitch & 1) && (EXPREGS[1] & 0x80)) {
		/* openbus */
	} else {
		uint32 mask = (~EXPREGS[3] & 0x1F);
		uint32 bank  = (EXPREGS[1] & 0x1F) | (EXPREGS[2] & 0x80) |
			((dipswitch & 2) ? (EXPREGS[2] & 0x20) : ((EXPREGS[1] >> 1) & 0x20)) |
			((dipswitch & 4) ? (EXPREGS[2] & 0x40) : ((EXPREGS[1] << 1) & 0x40));
		setprg8(A, (V & mask) | bank);
	}
}

static DECLFR(M401Read) {
	if ((dipswitch & 1) && (EXPREGS[1] & 0x80))
		return X.DB;
	return CartBR(A);
}

static DECLFW(M401Write) {
	/* FCEU_printf("Wr A:%04x V:%02x index:%d\n", A, V, EXPREGS[4]); */
	if (!(EXPREGS[3] & 0x40)) {
		EXPREGS[EXPREGS[4]] = V;
		EXPREGS[4] = (EXPREGS[4] + 1) & 3;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
	CartBW(A, V);
}

static void M401Reset(void) {
	dipswitch = (dipswitch + 1) & 7;
	FCEU_printf("dipswitch = %d\n", dipswitch);
	EXPREGS[0] = 0x00;
	EXPREGS[1] = 0x00;
	EXPREGS[2] = 0x0F;
	EXPREGS[3] = 0x00;
	EXPREGS[4] = 0x00;
	MMC3RegReset();
}

static void M401Power(void) {
	dipswitch = 7;
	EXPREGS[0] = 0x00;
	EXPREGS[1] = 0x00;
	EXPREGS[2] = 0x0F;
	EXPREGS[3] = 0x00;
	EXPREGS[4] = 0x00;
	GenMMC3Power();
	SetReadHandler(0x8000, 0xFFFF, M401Read);
	SetWriteHandler(0x6000, 0x7FFF, M401Write);
}

void Mapper401_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 256, 8, 0);
	cwrap = M401CW;
	pwrap = M401PW;
	info->Power = M401Power;
	info->Reset = M401Reset;
	AddExState(EXPREGS, 5, 0, "EXPR");
	AddExState(&dipswitch, 1, 0, "DPSW");
}
#endif
