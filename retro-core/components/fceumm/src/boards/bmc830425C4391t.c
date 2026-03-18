#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright (C) 2019 Libretro Team
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

/* NES 2.0 Mapper 320 is used for the Super HiK 6-in-1 A-030 multicart.
 * Basically UxROM with an address-latch-based outer bank register.
 * UNIF board name is BMC-830425C-4391T. Mirroring is hard-wired.
 * http://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_320
 */

#include "mapinc.h"

static uint8 bank_size;
static uint8 inner_bank;
static uint8 outer_bank;

static SFORMAT StateRegs[] =
{
	{ &inner_bank, 1, "INNB" },
	{ &outer_bank, 1, "OUTB" },
	{ &bank_size, 1, "SIZE" },
	{ 0 }
};

static void Sync(void) {
	setprg16(0x8000, (outer_bank << 3) | (inner_bank & bank_size));
	setprg16(0xC000, (outer_bank << 3) | bank_size);
	setchr8(0);
}

static DECLFW(M320Write) {
	/* address mask is inconsistent with that is in the wiki. Mask should be
	 * 0xFFE0 or Mermaid game will not work. */
	if ((A & 0xFFE0) == 0xF0E0) {
		outer_bank = (A & 0x0F);
		bank_size = (A & 0x10) ? 0x07 : 0x0F;
	}
	inner_bank = (V & 0x0F);
	Sync();
}

static void M320Power(void) {
	bank_size = 0x0F;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M320Write);
}

static void M320Reset(void) {
	inner_bank = outer_bank = 0;
	bank_size = 0x0F;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void BMC830425C4391T_Init(CartInfo *info) {
	info->Power = M320Power;
	info->Reset = M320Reset;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
