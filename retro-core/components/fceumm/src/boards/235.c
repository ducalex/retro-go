#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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

#include "mapinc.h"

static uint16 cmdreg;
static uint8 unrom, reg, openbus;

static uint32 PRGROMSize;

static SFORMAT StateRegs[] =
{
	{ &cmdreg,  2 | FCEUSTATE_RLSB, "CREG" },
	{ &unrom,   1, "UNRM" },
	{ &reg,     1, "UNRG" },
	{ &openbus, 1, "OPNB" },
	{ 0 }
};

static void Sync(void) {
	if (unrom) {
		uint8 PRGPageSize = PRGROMSize / 16384;
		setprg16(0x8000, (PRGPageSize & 0xC0) | (reg & 7));
		setprg16(0xC000, (PRGPageSize & 0xC0) | 7);
		setchr8(0);
		setmirror(MI_V);
	} else {
		uint8 bank = ((cmdreg & 0x300) >> 3) | (cmdreg & 0x1F);
		if (cmdreg & 0x400)
			setmirror(MI_0);
		else
			setmirror(((cmdreg >> 13) & 1) ^ 1);
		if (bank >= PRGROMSize / 32768)
			openbus = 1;
		else if (cmdreg & 0x800) {
			setprg16(0x8000, (bank << 1) | ((cmdreg >> 12) & 1));
			setprg16(0xC000, (bank << 1) | ((cmdreg >> 12) & 1));
		} else
			setprg32(0x8000, bank);
		setchr8(0);
	}
}

static DECLFR(M235Read) {
	if (openbus) {
		openbus = 0;
		return X.DB;
	}
	return CartBR(A);
}

static DECLFW(M235Write) {
	cmdreg = A;
	reg = V;
	Sync();
}

static void M235Reset(void) {
	cmdreg = 0;
	reg = 0;
	if (PRGROMSize & 0x20000)
		unrom = (unrom + 1) & 1;
	Sync();
}

static void M235Power(void) {
	SetWriteHandler(0x8000, 0xFFFF, M235Write);
	SetReadHandler(0x8000, 0xFFFF, M235Read);
	cmdreg = 0;
	reg = 0;
	Sync();
}

static void M235Restore(int version) {
	Sync();
}

void Mapper235_Init(CartInfo *info) {
	info->Reset = M235Reset;
	info->Power = M235Power;
	GameStateRestore = M235Restore;
	AddExState(&StateRegs, ~0, 0, 0);
	PRGROMSize = info->PRGRomSize;
}
#endif
