#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2019 Libretro Team
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

/* NES 2.0 Mapper 324
 * FARID_UNROM_8-IN-1
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_324
 */

#include "mapinc.h"

static uint8 latch;

static SFORMAT StateRegs[] =
{
	{ &latch, 1, "LATC" },
	{ 0 }
};

static void Sync(void) {
	setchr8(0);
	setprg16(0x8000, ((latch & 0x70) >> 1) | (latch & 0x07));
	setprg16(0xC000, ((latch & 0x70) >> 1) | 0x07 );
}

static DECLFW(FARIDUNROMWrite) {
	V &= CartBR(A);
	if ((V & 0x80) && !(latch & 0x80) && !(latch & 0x08))
		latch = (latch & 0x87) | (V & 0x78);
	latch = (latch & 0x78) | (V & 0x87);
	Sync();
}

static void FARIDUNROMPower(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, FARIDUNROMWrite);
}

static void FARIDUNROMReset(void) {
	latch &= ~0x78;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void FARIDUNROM_Init(CartInfo *info) {
	info->Power = FARIDUNROMPower;
	info->Reset = FARIDUNROMReset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
