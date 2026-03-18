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

/* N625836 PCB */

#include "mapinc.h"
#include "mmc3.h"

static void Mapper455_PRGWrap(uint32 A, uint8 V) {
	int prgAND =EXPREGS[1] &0x01? 0x1F: 0x0F;
	int prgOR  =EXPREGS[0] >>2 &0x07 | EXPREGS[1] <<1 &0x08 | EXPREGS[0] >>2 &0x10;
	if (EXPREGS[0] &0x01) {
		if (EXPREGS[0] &0x02) {
			setprg32(0x8000, prgOR >>1);
		} else {
			setprg16(0x8000, prgOR);
			setprg16(0xC000, prgOR);
		}
	} else {
		prgOR <<=1;
		setprg8(A, V &prgAND | prgOR &~prgAND);
	}
}

static void Mapper455_CHRWrap(uint32 A, uint8 V) {
	int chrAND =EXPREGS[1] &0x02? 0xFF: 0x7F;
	int chrOR  =(EXPREGS[0] >>2 &0x07 | EXPREGS[1] <<1 &0x08 | EXPREGS[0] >>2 &0x10) <<4;
	setchr1(A, V &chrAND | chrOR &~chrAND);
}

static DECLFW(Mapper455_Write) {
	if (A &0x100) {
		EXPREGS[0] =V;
		EXPREGS[1] =A &0xFF;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static void Mapper455_Reset(void) {
	EXPREGS[0] =1;
	EXPREGS[1] =0;
	MMC3RegReset();
}

static void Mapper455_Power(void) {
	EXPREGS[0] =1;
	EXPREGS[1] =0;
	GenMMC3Power();
	SetWriteHandler(0x4100, 0x5FFF, Mapper455_Write);
}

void Mapper455_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 256, 0, 0);
	cwrap = Mapper455_CHRWrap;
	pwrap = Mapper455_PRGWrap;
	info->Power = Mapper455_Power;
	info->Reset = Mapper455_Reset;
	AddExState(EXPREGS, 2, 0, "EXPR");
}
#endif
