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

#include "mapinc.h"
#include "mmc3.h"

static void Mapper376CW(uint32 A, uint8 V) {
	uint32 base = (EXPREGS[0] &0x40? 0x080: 0x000) | (EXPREGS[1] &0x01? 0x100: 0x000);
	setchr1(A, base | (V & 0x7F));
}

static void Mapper376PW(uint32 A, uint8 V) {
	uint32 base = (EXPREGS[0] &0x07) | (EXPREGS[0] &0x40? 0x08: 0x00) | (EXPREGS[1] &0x01? 0x10: 0x00);
	if (EXPREGS[0] & 0x80) {
		if (EXPREGS[0] &0x20) {
			if (A ==0x8000) setprg32(A, base >>1);
		} else {
			if (A ==0x8000 || A ==0xC000) setprg16(A, base);
		}
	} else
		setprg8(A, (base << 1) | (V & 0x0F));
}

static DECLFW(Mapper376Write) {
	EXPREGS[A & 1] = V;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void Mapper376Reset(void) {
	EXPREGS[0] = 0;
	EXPREGS[1] = 0;
	MMC3RegReset();
}

static void Mapper376Power(void) {
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, Mapper376Write);
}

void Mapper376_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 256, 1, 0);
	pwrap = Mapper376PW;
	cwrap = Mapper376CW;
	info->Power = Mapper376Power;
	info->Reset = Mapper376Reset;
	AddExState(EXPREGS, 2, 0, "EXPR");
}
#endif
