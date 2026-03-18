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

/* NES 2.0 Mapper 315
 * BMC-830134C
 * Used for multicarts using 820732C- and 830134C-numbered PCBs such as 4-in-1 Street Blaster 5
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_315
 */

#include "mapinc.h"
#include "mmc3.h"

static void BMC830134CCW(uint32 A, uint8 V) {
	setchr1(A, (V & 0xFF) | ((EXPREGS[0] & 0x01) << 8) | ((EXPREGS[0] & 0x02) << 6) | ((EXPREGS[0] & 0x08) << 3));
}

static void BMC830134CPW(uint32 A, uint8 V) {
	if ((EXPREGS[0] & 0x06) == 0x06) {
		if (A == 0x8000) {
			setprg8(A, (V & 0x0F) | ((EXPREGS[0] & 0x06) << 3));
			setprg8(0xC000, (V & 0x0F) | 0x32);
		} else if (A == 0xA000) {
			setprg8(A, (V & 0x0F) | ((EXPREGS[0] & 0x06) << 3));
			setprg8(0xE000, (V & 0x0F) | 0x32);
		}
	} else
		setprg8(A, (V & 0x0F) | ((EXPREGS[0] & 0x06) << 3));
}

static DECLFW(BMC830134CWrite) {
	EXPREGS[0] = V;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void BMC830134CReset(void) {
	EXPREGS[0] = 0;
	MMC3RegReset();
}

static void BMC830134CPower(void) {
	GenMMC3Power();
	SetWriteHandler(0x6800, 0x68FF, BMC830134CWrite);
}

void BMC830134C_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 256, 1, 0);
	pwrap = BMC830134CPW;
	cwrap = BMC830134CCW;
	info->Power = BMC830134CPower;
	info->Reset = BMC830134CReset;
	AddExState(EXPREGS, 1, 0, "EXPR");
}
#endif
