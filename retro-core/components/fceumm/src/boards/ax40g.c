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

/* NES 2.0 Mapper 527 is used for a bootleg version of
 * Taito's 不動明王伝 (Fudō Myōō Den).
 * Its UNIF board name is UNL-AX-40G. The original INES Mapper 207 is
 * replaced with a VRC2 clone (A0/A1, i.e. VRC2b) while retaining
 * Mapper 207's extended mirroring.
 */

#include "mapinc.h"

static uint8 preg[2], creg[8], NT[2];

static SFORMAT StateRegs[] =
{
	{ preg, 2, "PREG" },
	{ creg, 8, "CREG" },
	{ NT, 2, "NMT" },
	{ 0 }
};

static void Sync(void) {
	uint8 i;
	setprg8(0x8000, preg[0]);
	setprg8(0xA000, preg[1]);
	setprg8(0xC000, 0x1E);
	setprg8(0xE000, 0x1F);
	for (i = 0; i < 8; i++)
		setchr1(i << 10, creg[i]);
	setmirrorw(NT[0], NT[0], NT[1], NT[1]);
}

static DECLFW(UNLAX40GWrite8) {
	A &= 0xF003;
	preg[0] = V & 0x1F;
	Sync();
}

static DECLFW(UNLAX40GWriteA) {
	A &= 0xF003;
	preg[1] = V & 0x1F;
	Sync();
}

static DECLFW(UNLAX40GWriteB) {
	uint16 i, shift;
	A &= 0xF003;
	i = ((A >> 1) & 1) | ((A - 0xB000) >> 11);
	shift = ((A & 1) << 2);
	creg[i] = (creg[i] & (0xF0 >> shift)) | ((V & 0xF) << shift);
	if (i < 2)
		NT[i] = (creg[i] & 0x80) >> 7;
	Sync();
}

static void UNLAX40GPower(void) {
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0x8FFF, UNLAX40GWrite8);
	SetWriteHandler(0xA000, 0xAFFF, UNLAX40GWriteA);
	SetWriteHandler(0xB000, 0xEFFF, UNLAX40GWriteB);
}

static void StateRestore(int version) {
	Sync();
}

void UNLAX40G_Init(CartInfo *info) {
	info->Power = UNLAX40GPower;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
