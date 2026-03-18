#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
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

static uint8 reg[2];
static uint8 dip;
static uint8 chrramvariant;
static SFORMAT StateRegs[] =
{
	{ reg,  2, "REG " },
	{ &dip, 1, "DIP " },
	{ 0 }
};

static void Sync(void)
{
	int prg;
	int chr;
	if (chrramvariant)
	{
		prg = (reg[1] &7) | (reg[0] <<3);
		chr = 0;
	}
	else
	{
		prg = reg[1] & 0x0F;
		chr = reg[0] & 0x0F;
	}
	switch (reg[1] >>4 &3)
	{
		case 0:
		case 1:
			setprg16(0x8000, prg);
			setprg16(0xC000, prg |7);
			break;
		case 2:
			setprg32(0x8000, prg >>1);
			break;
		case 3:
			setprg16(0x8000, prg);
			setprg16(0xC000, prg);
			break;
	}
	setchr8(chr);
	setmirror((reg[0] >>5 &1) ^1);
}

static DECLFR(M236Read)
{
	if ((reg[1] >>4 &3) ==1)
		return CartBR((A &~0xF) | (dip &0xF));
	else
		return CartBR(A);
}

static DECLFW(M236WriteReg)
{
	reg[A >>14 &1] =A &0xFF;
	Sync();
}

static void M236Power(void)
{
	dip = 0;
	reg[0] = 0;
	reg[1] = 0;
	Sync();
	SetWriteHandler(0x8000, 0xFFFF, M236WriteReg);
	SetReadHandler(0x8000, 0xFFFF, M236Read);
}

static void M236Reset(void)
{
	++dip;
	/* Soft-reset returns to menu */
	reg[0] = 0;
	reg[1] = 0;
	Sync();
}

static void StateRestore(int version)
{
	Sync();
}

void Mapper236_Init(CartInfo *info)
{
	info->Power = M236Power;
	info->Reset = M236Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
	chrramvariant = info->CHRRomSize == 0;
}
#endif
