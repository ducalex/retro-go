#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2008 CaH4e3
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

/* NES 2.0 Mapper 436: 820401/T-217 */

#include "mapinc.h"
#include "mmc3.h"

static void Mapper436_PWrap(uint32 A, uint8 V) {
	if (EXPREGS[0] &0x01)
		setprg8(A, V &0x0F | EXPREGS[0] >>2 &0x30);
	else
	if (A == 0x8000)
		setprg32(A, (EXPREGS[0] >>4));
}

static void Mapper436_CWrap(uint32 A, uint8 V) {
	setchr1(A, V &0x7F | EXPREGS[0] <<1 &~0x7F);
}

static DECLFW(Mapper436_Write) {
	EXPREGS[0] = A &0xFF;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void Mapper436_Reset(void) {
	EXPREGS[0] = 0;
	MMC3RegReset();
}

static void Mapper436_Power(void) {
	EXPREGS[0] = 0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, Mapper436_Write);
}

void Mapper436_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 128, 8, 0);
	pwrap = Mapper436_PWrap;
	cwrap = Mapper436_CWrap;
	info->Power = Mapper436_Power;
	info->Reset = Mapper436_Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");
}
#endif
