#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022 NewRisingSun
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
 */

#include "mapinc.h"

static uint8 regData[2];
static uint8 regAddr;

static SFORMAT K1053_state[] =
{
	{ regData, 2, "REGD" },
	{&regAddr, 1, "REGA" },
	{ 0 }
};

static void K1053_sync (void) {
	int prg =regData[0] &0x3F | regAddr <<4 &~0x3F;
	int chrWritable;
	switch(regAddr &3) {
		case 0:	setprg32(0x8000, prg >>1);
			chrWritable =0;
			break;
		case 1:	setprg16(0x8000, prg);
			setprg16(0xC000, prg |7);
			chrWritable =1;
			break;
		case 2:	prg =prg <<1 | regData[0] >>7;
			setprg8(0x8000, prg);
			setprg8(0xA000, prg);
			setprg8(0xC000, prg);
			setprg8(0xE000, prg);
			chrWritable =1;
			break;
		case 3:	setprg16(0x8000, prg);
			setprg16(0xC000, prg);
			chrWritable =0;
			break;			
	}
	SetupCartCHRMapping(0, CHRptr[0], 0x8000, chrWritable);
	setchr8(regData[1]);
	setmirror(regData[0] &0x40? MI_H: MI_V);
}

static void K1053_restore(int version) {
	K1053_sync();
}

static DECLFW(K1053_write) {
	regData[A >>14 &1] =V;
	if (A &0x4000) regAddr=A &0xFF;
	K1053_sync();
}

static void K1053_reset(void) {
	regData[0] =regData[1] =regAddr =0;
	K1053_sync();
}

static void K1053_power(void) {
	K1053_reset();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, K1053_write);
}

void Mapper310_Init(CartInfo *info) {
	info->Power = K1053_power;
	info->Reset = K1053_reset;
	GameStateRestore = K1053_restore;
	AddExState(&K1053_state, ~0, 0, 0);
}
#endif
