#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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

#include "mapinc.h"
#include "mmc3.h"

static void M291CW(uint32 A, uint8 V) {
	setchr1(A, V | ((EXPREGS[0] << 2) & 0x100));
}

static void M291PW(uint32 A, uint8 V) {
	if (EXPREGS[0] & 0x20)
		setprg32(0x8000, ((EXPREGS[0] >> 1) & 3) | ((EXPREGS[0] >> 4) & 4));
	else
		setprg8(A, (V & 0x0F) |  ((EXPREGS[0] >> 2) & 0x10));
}

static DECLFW(M291Write) {
	EXPREGS[0] = V;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void M291Reset(void) {
	EXPREGS[0] = 0;
	MMC3RegReset();
}

static void M291Power(void) {
	EXPREGS[0] = 0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, M291Write);
}

void Mapper291_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 512, 0, 0);
	cwrap = M291CW;
	pwrap = M291PW;
	info->Power = M291Power;
	info->Reset = M291Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");
}
#endif
