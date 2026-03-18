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

static SFORMAT StateRegs[] =
{
	{ regs, 4, "REGS" },
	{ 0 }
};

static void Sync(void)
{
   uint8 mirr;
	int r = 0;

	if ((regs[1]) & 0x20)
		r = 1;
	if ((regs[1] >> 4) & 0x01)
	{
		setprg16(0x8000, (regs[1] & 0x07) | (r << 3));
		setprg16(0xC000, (regs[1] & 0x07) | (r << 3));
		setchr8((regs[0] & 0x07) | (r << 3));
	}
	else
	{
		setprg32(0x8000, ((regs[1] >> 1) & 0x03) | (r << 2));
		setchr8((regs[2] & 0x01) | (r << 3));
	}

	mirr = (((regs[0] >> 4) & 0x1));

	if (mirr)
		setmirror(0);
	else
		setmirror(1);

}

static DECLFW(KG256WriteHi) {
	regs[2] = V;
	Sync();
}

static DECLFW(KG256WriteLo) {
	regs[A & 0x03] = V;
	Sync();
}

static void KG256Power(void) {

	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;
	regs[3] = 0;

	SetWriteHandler(0x8000, 0xFFFF, KG256WriteHi);
	SetWriteHandler(0x6000, 0x7FFF, KG256WriteLo);
	SetReadHandler(0x8000, 0xFFFF, CartBR);

	Sync();

}

static void StateRestore(int version) {
	Sync();
}

static void KG256Reset(void) {
	
	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;

	Sync();
}

void KG256_Init(CartInfo *info) {

	Sync();

	info->Power = KG256Power;
	info->Reset = KG256Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
}



#endif
