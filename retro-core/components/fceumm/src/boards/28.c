#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/*
 * Copyright (C) 2012-2017 FCEUX team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

/* added 2019-5-23
 * Mapper 28 - Action 53
 * http://wiki.nesdev.com/w/index.php/INES_Mapper_028 */

#include "mapinc.h"

static uint8 prg_mask_16k;
static uint8 reg, chr, prg, mode, outer;

static SFORMAT StateRegs[] = {
	{&reg, 1, "REG"},
	{&chr, 1, "CHR"},
	{&prg, 1, "PRG"},
	{&mode, 1, "MODE"},
	{&outer, 1, "OUTR"},
	{0}
};

void SyncMirror() {
	switch (mode & 3) {
	case 0: setmirror(MI_0); break;
	case 1: setmirror(MI_1); break;
	case 2: setmirror(MI_V); break;
	case 3: setmirror(MI_H); break;
	}
}

void Mirror(uint8 value)
{
	if ((mode & 2) == 0) {
		mode &= 0xfe;
		mode |= value >> 4 & 1;
	}
	SyncMirror();
}


static void Sync() {
	uint8 prglo = 0;
	uint8 prghi = 0;

	uint8 outb = outer << 1;

	/* this can probably be rolled up, but i have no motivation to do so
	 * until it's been tested */
	switch (mode & 0x3c) {
	/* 32K modes */
	case 0x00:
	case 0x04:
		prglo = outb;
		prghi = outb | 1;
		break;
	case 0x10:
	case 0x14:
		prglo = (outb & ~2) | (prg << 1 & 2);
		prghi = (outb & ~2) | (prg << 1 & 2) | 1;
		break;
	case 0x20:
	case 0x24:
		prglo = (outb & ~6) | (prg << 1 & 6);
		prghi = (outb & ~6) | (prg << 1 & 6) | 1;
		break;
	case 0x30:
	case 0x34:
		prglo = (outb & ~14) | (prg << 1 & 14);
		prghi = (outb & ~14) | (prg << 1 & 14) | 1;
		break;
	/* bottom fixed modes */
	case 0x08:
		prglo = outb;
		prghi = outb | (prg & 1);
		break;
	case 0x18:
		prglo = outb;
		prghi = (outb & ~2) | (prg & 3);
		break;
	case 0x28:
		prglo = outb;
		prghi = (outb & ~6) | (prg & 7);
		break;
	case 0x38:
		prglo = outb;
		prghi = (outb & ~14) | (prg & 15);
		break;
	/* top fixed modes */
	case 0x0c:
		prglo = outb | (prg & 1);
		prghi = outb | 1;
		break;
	case 0x1c:
		prglo = (outb & ~2) | (prg & 3);
		prghi = outb | 1;
		break;
	case 0x2c:
		prglo = (outb & ~6) | (prg & 7);
		prghi = outb | 1;
		break;
	case 0x3c:
		prglo = (outb & ~14) | (prg & 15);
		prghi = outb | 1;
		break;
	}

	prglo &= prg_mask_16k;
	prghi &= prg_mask_16k;

	setprg16(0x8000, prglo);
	setprg16(0xC000, prghi);
	setchr8(chr);
}

static DECLFW(WriteEXP) {
	reg = V & 0x81;
}

static DECLFW(WritePRG) {
	switch (reg) {
	case 0x00:
		chr = V & 3;
		Mirror(V);
		Sync();
		break;
	case 0x01:
		prg = V & 15;
		Mirror(V);
		Sync();
		break;
	case 0x80:
		mode = V & 63;
		SyncMirror();
		Sync();
		break;
	case 0x81:
		outer = V & 63;
		Sync();
		break;
	}
}

static void M28Power(void) {
	outer = 63;
	prg = 15;
	Sync();
	prg_mask_16k = PRGsize[0] - 1;
	SetWriteHandler(0x5000,0x5FFF,WriteEXP);
	SetWriteHandler(0x8000,0xFFFF,WritePRG);
	SetReadHandler(0x8000,0xFFFF,CartBR);
	SetReadHandler(0x6000,0x7FFF,CartBR);
	SetWriteHandler(0x6000,0x7FFF,CartBW);
}

static void M28Reset(void) {
	outer = 63;
	prg = 15;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper28_Init(CartInfo* info) {
	info->Power=M28Power;
	info->Reset=M28Reset;
	GameStateRestore=StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
