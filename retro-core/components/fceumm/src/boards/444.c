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

/* NC7000M PCB, with incorrect UNIF MAPR BS-110 due to a mix-up. Submapper bits 0 and 1. denote the setting of two solder pads that configure CHR banking. */
/* NC8000M PCB, indicated by submapper bit 2. */

#include "mapinc.h"
#include "mmc3.h"

static uint8 pads;
static uint8 dip;

static void Mapper444_PRGWrap(uint32 A, uint8 V) {
	int prgAND =pads &4 && EXPREGS[0] &0x02? 0x1F: 0x0F;
	int prgOR  =EXPREGS[0] <<4;
	if (EXPREGS[0] &0x04) {
		if (~A &0x4000) {
			setprg8(A,         (~EXPREGS[0] &0x08? ~2: ~0) &V &prgAND | prgOR &~prgAND);
			setprg8(A |0x4000, (~EXPREGS[0] &0x08?  2:  0) |V &prgAND | prgOR &~prgAND);
		}
	} else
		setprg8(A, V &prgAND | prgOR &~prgAND);
}

static void Mapper444_CHRWrap(uint32 A, uint8 V) {
	int chrAND =pads &1? 0xFF: 0x7F;
	int chrOR  =EXPREGS[0] <<7 &(pads &1? 0x00: 0x80) | EXPREGS[0] <<(pads &2? 4: 7) &0x100;
	setchr1(A, V &chrAND | chrOR &~chrAND);
}

static DECLFR(Mapper444_Read) {
	return (EXPREGS[0] &0x0C) ==0x08? dip: CartBR(A);
}

static DECLFW(Mapper444_Write) {
	EXPREGS[0] =A &0xFF;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void Mapper444_Reset(void) {
	dip++;
	dip &= 3;
	EXPREGS[0] =0;
	MMC3RegReset();
}

static void Mapper444_Power(void) {
	dip =0;
	EXPREGS[0] =0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, Mapper444_Write);
	SetReadHandler(0x8000, 0xFFFF, Mapper444_Read);
}

void Mapper444_Init(CartInfo *info) {
	pads = info->submapper; /* UNIF represents submapper 0 */
	GenMMC3_Init(info, 256, 256, 0, 0);
	cwrap = Mapper444_CHRWrap;
	pwrap = Mapper444_PRGWrap;
	info->Power = Mapper444_Power;
	info->Reset = Mapper444_Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");
	AddExState(&dip, 1, 0, "DIPS");
}
#endif
