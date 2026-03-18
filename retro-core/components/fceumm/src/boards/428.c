#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *
 *  Copyright (C) 2008 -2020 dragon2snow,loong2snow from www.nesbbs.com
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
 *
 *
 */

#include "mapinc.h"

static uint8 regs[4];
static uint8 hrd_flag;

static SFORMAT StateRegs[] =
{
	{ &hrd_flag, 1, "FLAG" },
	{ regs, 4, "REGS" },
	{ 0 }
};

static void Sync(void) {
	int mask = regs[2] >> 6; /* There is an CNROM mode that takes either two or four inner CHR banks from a CNROM-like latch register at $8000-$FFFF. */

	if (regs[1] & 0x10)
		setprg32(0x8000,(regs[1] & 0xC0) >> 6);
	else
	{
		setprg16(0x8000, (regs[1] & 0xE0) >> 5);
		setprg16(0xC000, (regs[1] & 0xE0) >> 5);
	}

	setchr8(((regs[1] & 0x07) & ~mask) | regs[0] & mask);
	
	setmirror((regs[1] & 0x8) ? 0 : 1);
}

static DECLFW(WriteHi) {
	regs[0] = V;
	Sync();
}

static DECLFW(WriteLo) {
	regs[A & 0x03] = V;
	Sync();
}

static DECLFR(ReadLo) {
	return hrd_flag;
}

static void Power(void) {
	hrd_flag = 0; /* Solder pad, selecting different menus */

	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;

	Sync();
	SetWriteHandler(0x8000, 0xFFFF, WriteHi);
	SetWriteHandler(0x6001, 0x6002, WriteLo);
	SetReadHandler(0x6000, 0x7FFF, ReadLo);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void StateRestore(int version) {
	Sync();
}

static void Reset(void) {
	hrd_flag++;
	hrd_flag &= 3;
	
	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;

	Sync();
}

void Mapper428_Init(CartInfo *info) {
	hrd_flag = 0;

	Sync();

	info->Power = Power;
	info->Reset = Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
}

#endif
