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

/* NES 2.0 Mapper 337 - BMC-CTC-12IN1
 * 12-in-1 Game Card multicart
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_337

 * NES 2.0 Mapper 350 - BMC-891227
 * Super 15-in-1 Game Card 
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_350
 */

#include "mapinc.h"

static uint8 latche, m350;

static SFORMAT StateRegs[] =
{
	{ &latche, 1, "LATC" },
	{ &m350, 1, "M350" },
	{ 0 }
};

static void Sync(void) {
	uint8 mirroring = m350 ? ((latche >> 7) & 1) : ((latche >> 5) & 1);
	uint8 mode      = m350 ? ((latche >> 5) & 0x03) : ((latche >> 6) & 0x03);
	uint8 base      = m350 ? ((latche & 0x40) ? (latche & 0x20) : 0) : 0;

	setchr8(0);
	setprg8(0x6000, 1);
	setprg16r(0, 0x8000, base | (latche & 0x1F));
	setprg16r(0, 0xC000, base | ((latche & 0x1F) | ((mode & 2) ? 0x07 : (mode & 1))));
	setmirror(mirroring ^ 1);
}

static DECLFW(BMCCTC12IN1Write8) {
	latche = (latche & 7) | (V & ~7);
	Sync();
}

static DECLFW(BMCCTC12IN1WriteC) {
	latche = (latche & ~7) | (V & 7);
	Sync();
}

static void BMCCTC12IN1Power(void) {
	Sync();
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xBFFF, BMCCTC12IN1Write8);
	SetWriteHandler(0xC000, 0xFFFF, BMCCTC12IN1WriteC);
}

static void BMCCTC12IN1Reset(void) {
	latche = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

/* Mapper 337 - BMC-CTC-12IN1 */
void BMCCTC12IN1_Init(CartInfo *info) {
	m350 = 0;
	info->Power = BMCCTC12IN1Power;
	info->Reset = BMCCTC12IN1Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}

/* Mapper 350 - BMC-891227 */
void BMC891227_Init(CartInfo *info) {
	m350 = 1;
	info->Power = BMCCTC12IN1Power;
	info->Reset = BMCCTC12IN1Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
