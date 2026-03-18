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
 */

/* added 2019-5-23
 * UNIF: BMC-60311C:
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_289
 */

#include "mapinc.h"

static uint8 latch, reg[2];

static SFORMAT StateRegs[] =
{
	{ &latch, 1, "LATC" },
	{ reg, 2, "REGS" },
	{ 0 }
};

static void Sync(void) {
	if (reg[0] &2) { /* UNROM */
		setprg16(0x8000, latch &7 | reg[1] &~7);
		setprg16(0xC000,        7 | reg[1] &~7);
	} else
	if (reg[0] &1)   /* NROM-256 */
		setprg32(0x8000, reg[1] >>1);
	else {           /* NROM-128 */
		setprg16(0x8000, reg[1]);
		setprg16(0xC000, reg[1]);
	}
	SetupCartCHRMapping(0, CHRptr[0], 0x2000, !(reg[0] &4)); /* CHR-RAM write-protect */
	setchr8(0);
	setmirror(!(reg[0] &8));
}

static DECLFW(WriteReg) {
	reg[A &1] =V;
	Sync();
}

static DECLFW(WriteLatch) {
	latch = V;
	Sync();
}

static void BMC60311CPower(void) {
	latch =reg[0] =reg[1] =0;
	Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x6001, WriteReg);
	SetWriteHandler(0x8000, 0xFFFF, WriteLatch);
}

static void BMC60311CReset(void) {
	latch =reg[0] =reg[1] =0;
	Sync();
}

static void BMC60311CRestore(int version) {
	Sync();
}

void BMC60311C_Init(CartInfo *info) {
	info->Power = BMC60311CPower;
	info->Reset = BMC60311CReset;
	GameStateRestore = BMC60311CRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
