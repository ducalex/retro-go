#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2019 Libretro Team
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

/* NES 2.0 Mapper 322
 * BMC-K-3033
 * 35-in-1 (K-3033)
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_322
 */

#include "mapinc.h"
#include "mmc3.h"

static void BMCK3033CW(uint32 A, uint8 V) {
	if (EXPREGS[2]) {
		if (EXPREGS[3]) {
/*			FCEU_printf("MMC3-256: A:%04x V:%02x R1:%02x\n", A, V, EXPREGS[1]); */
			setchr1(A, (EXPREGS[1] << 8) | (V & 0xFF));
		} else {
/*			FCEU_printf("MMC3-128: A:%04x V:%02x R1:%02x\n", A, V, EXPREGS[1]); */
			setchr1(A, (EXPREGS[1] << 7) | (V & 0x7F));
		}
	} else {
/*		FCEU_printf("NROM: A:%04x V:%02x R1:%02x\n", A, V, EXPREGS[1]); */
		setchr1(A, (V & 0x7F));
	}
}

static void BMCK3033PW(uint32 A, uint8 V) {
	if (EXPREGS[2]) {
		if (EXPREGS[3] ) {
/*			FCEU_printf("MMC3-256 A:%04x V:%02x chip:%02x\n", A, V, EXPREGS[1] & ~0x01); */
			setprg8(A, (EXPREGS[1] << 5) | (V & 0x1F));
		} else {
/*			FCEU_printf("MMC3-128 A:%04x V:%02x chip:%02x\n", A, V, EXPREGS[1]); */
			setprg8(A, (EXPREGS[1] << 4) | (V & 0x0F));
		}
	} else {
		uint32 base = (EXPREGS[1] << 3);
		if (EXPREGS[0] & 0x03) {
/*			FCEU_printf("NROM-256 base:%02x chip:%02x\n", EXPREGS[0] >> 1, EXPREGS[1]); */
			setprg32(0x8000, base | EXPREGS[0] >> 1);
		} else {
/*			FCEU_printf("NROM-128 base:%02x chip:%02x\n", EXPREGS[0], EXPREGS[1]); */
			setprg16(0x8000, base | EXPREGS[0]);
			setprg16(0xC000, base | EXPREGS[0]);
		}
	}
}

static DECLFW(BMCK3033Write) {
	EXPREGS[0] = (A & 0x07);
	EXPREGS[1] = ((A & 0x18) >> 3) | ((A & 0x40) >> 4);
	EXPREGS[2] = (A & 0x20);
	EXPREGS[3] = (A & 0x80);
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void BMCK3033Power(void) {
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, BMCK3033Write);
}

static void BMCK3033Reset(void) {
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = 0;
	MMC3RegReset();
}

void BMCK3033_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 256, 1, 0);
	pwrap = BMCK3033PW;
	cwrap = BMCK3033CW;
	info->Power = BMCK3033Power;
	info->Reset = BMCK3033Reset;
	AddExState(EXPREGS, 4, 0, "EXPR");
}
#endif
