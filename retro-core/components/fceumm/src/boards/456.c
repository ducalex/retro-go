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

#include "mapinc.h"
#include "mmc3.h"

static void Mapper456_PRGWrap(uint32 A, uint8 V) {
	setprg8(A, V &0x0F | EXPREGS[0] <<4);
}

static void Mapper456_CHRWrap(uint32 A, uint8 V) {
	setchr1(A, V &0x7F | EXPREGS[0] <<7);
}

static DECLFW(Mapper456_Write) {
	if (A &0x100) {
		EXPREGS[0] =V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static void Mapper456_Reset(void) {
	EXPREGS[0] =0;
	MMC3RegReset();
}

static void Mapper456_Power(void) {
	EXPREGS[0] =0;
	GenMMC3Power();
	SetWriteHandler(0x4020, 0x5FFF, Mapper456_Write);
}

void Mapper456_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 128, 8, 0);
	cwrap = Mapper456_CHRWrap;
	pwrap = Mapper456_PRGWrap;
	info->Power = Mapper456_Power;
	info->Reset = Mapper456_Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");
}
#endif
