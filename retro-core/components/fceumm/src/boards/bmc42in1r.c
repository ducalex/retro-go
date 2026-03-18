#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
 *  Copyright (C) 2009 qeed
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

/* Updated 2019-07-12
 * Mapper 226 - Updated and combine UNIF Ghostbusters63in1 board (1.5 MB carts), different bank order
 * - some 1MB carts can switch game lists using Select
 */

#include "mapinc.h"

static uint8 reorder_banks = 0;
static uint8 latche[2], reset;
static uint8 banks[4] = { 0, 0, 1, 2 };
static SFORMAT StateRegs[] =
{
	{ &reset, 1, "RST" },
	{ latche, 2, "LATC" },
	{ 0 }
};

static void Sync(void) {
	uint8 bank = 0;
	uint8 base = ((latche[0] & 0x80) >> 7) | ((latche[1] & 1) << 1);

	if (reorder_banks) /* for 1536 KB prg roms */
		base = banks[base];
	bank = (base << 5) | (latche[0] & 0x1f);

	if (!(latche[0] & 0x20))
		setprg32(0x8000, bank >> 1);
	else {
		setprg16(0x8000, bank);
		setprg16(0xC000, bank);
	}
	setmirror((latche[0] >> 6) & 1);
	setchr8(0);
}

static DECLFW(M226Write) {
	latche[A & 1] = V;
	Sync();
}

static void M226Power(void) {
	latche[0] = latche[1] = 0;
	Sync();
	SetWriteHandler(0x8000, 0xFFFF, M226Write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void StateRestore(int version) {
	Sync();
}

static void M226Reset(void) {
	latche[0] = latche[1] = 0;
	Sync();
}

void Mapper226_Init(CartInfo *info) {
	/* 1536KiB PRG roms have different bank order */
	reorder_banks = ((info->PRGRomSize / 1024) == 1536) ? 1 : 0;
	info->Power = M226Power;
	info->Reset = M226Reset;
	AddExState(&StateRegs, ~0, 0, 0);
	GameStateRestore = StateRestore;
}
#endif
