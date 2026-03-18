#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
 *  Copyright (C) 2002 Xodnizel
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

/* DS-9-27. Absolutely insane PCB that overlays 8 KiB of WRAM into a selectable position between $8000 and $E000. */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM;
static uint32 WRAMSIZE;
static uint16 latch[2];
static uint8 submapper;

static void SetWRAM(uint16 A) {
	setprg8r(0x10, A, 0);
	SetWriteHandler(A, A |0x1FFF, CartBW);
}

static DECLFW(Mapper452_WriteLatch);
static void Mapper452_Sync(void) {
	SetWriteHandler(0x8000, 0xFFFF, Mapper452_WriteLatch);
	if (submapper ==1) {
		switch(latch[0] &0xF000) {
		case 0xA000:
			setprg16(0x8000, latch[0] >>1);
			setprg8 (0xC000, 0);
			SetWRAM(latch[0] <<4 &0x6000 |0x8000);
			break;
		case 0xC000:
			setprg16(0x8000, latch[0] >>1 |0);
			setprg16(0xC000, latch[0] >>1 |1);
			SetWRAM(latch[0] <<4 &0x6000 |0x8000);
			break;
		case 0xD000:
			setprg8(0x8000, latch[0]);
			setprg8(0xA000, latch[0]);
			setprg8(0xC000, latch[0]);
			setprg8(0xE000, latch[0]);
			SetWRAM(latch[0] <<4 &0x2000 |0x8000);
			SetWRAM(latch[0] <<4 &0x2000 |0xC000);
			break;
		case 0xE000:
			setprg16(0x8000, latch[0] >>1);
			setprg16(0xC000, latch[0] &0x100? (latch[0] >>1 |7): 0);
			SetWRAM(latch[0] <<4 &0x6000 |0x8000);
			break;
		default:
			setprg16(0x8000, latch[0] >>1);
			setprg16(0xC000, 0);
			break;
		}
		setchr8(0);
		setmirror(latch[0] &0x800? MI_H: MI_V);
	} else {
		uint8 wramBank = latch[1] >>3 &6 |8;
		if (latch[1] &2) {
			setprg8(0x8000, latch[0] >>1);
			setprg8(0xA000, latch[0] >>1);
			setprg8(0xC000, latch[0] >>1);
			setprg8(0xE000, latch[0] >>1);
			SetWRAM((wramBank ^4) <<12);
		} else
		if (latch[1] &8) {
			setprg8(0x8000, latch[0] >>1 &~1 |0);
			setprg8(0xA000, latch[0] >>1 &~1 |1);
			setprg8(0xC000, latch[0] >>1 &~1 |2);
			setprg8(0xE000, latch[0] >>1 &~1 |3 | latch[1] &4 | (latch[1] &0x04 && latch[1] &0x40? 8: 0));
		} else {
			setprg16(0x8000, latch[0] >>2);
			setprg16(0xC000, 0);
		}
		SetWRAM(wramBank <<12);
		setchr8(0);
		setmirror(latch [1] &1 ^1);
	}
}

static DECLFW(Mapper452_WriteLatch) {
	CartBW(A, V);
	latch[0] =A &0xFFFF;
	latch[1] =V;
	Mapper452_Sync();
}

static void Mapper452_Reset(void) {
	latch[0] =latch[1] =0;
	Mapper452_Sync();
}

static void Mapper452_Power(void) {
	latch[0] =latch[1] =0;
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	Mapper452_Sync();
	FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
}

static void Mapper452_Close(void) {
	if (WRAM)
		FCEU_gfree(WRAM);
	WRAM = NULL;
}

static void StateRestore(int version) {
	Mapper452_Sync();
}

void Mapper452_Init(CartInfo *info) {
	submapper =info->submapper;
	info->Reset = Mapper452_Reset;
	info->Power = Mapper452_Power;
	info->Close = Mapper452_Close;
	GameStateRestore = StateRestore;

	WRAMSIZE =8192;
#ifndef FCEU_NO_MALLOC
	WRAM =(uint8*) FCEU_gmalloc(WRAMSIZE);
#else
	WRAM =(uint8*)ahb_calloc(1, WRAMSIZE);
#endif
	SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");
	AddExState(&latch, 4, 0, "LATC");
}
#endif
