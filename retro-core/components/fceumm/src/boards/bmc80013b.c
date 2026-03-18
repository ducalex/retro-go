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

/* NES 2.0 Mapper 274 is used for the 90-in-1 Hwang Shinwei multicart.
 * Its UNIF board name is BMC-80013-B.
 * https://wiki.nesdev.com/w/index.php/NES_2.0_Mapper_274 */

 /* 2020-03-22 - Update support for Cave Story II, FIXME: Arabian does not work for some reasons... */

#include "mapinc.h"

static uint8 regs[2], mode;

static SFORMAT StateRegs[] =
{
	{ &regs[0], 1, "REG0" },
	{ &regs[1], 1, "REG1" },
	{ &mode, 1, "MODE" },
	{ 0 }
};

static void Sync(void) {
	if (mode & 0x02)
		setprg16(0x8000, (regs[0] & 0x0F) | (regs[1] & 0x70));
	else
		setprg16(0x8000, (regs[0] & ((ROM_size - 1) & 0x0F)) | 0x80);
	setprg16(0xC000, regs[1]);
	setmirror(((regs[0] >> 4) & 1) ^ 1);
}

static DECLFW(BMC80013BWrite) {
	uint8 reg = (A >> 13) & 0x03;
	if (!reg)
		regs[0] = V & 0x1F;
	else {
		regs[1] = V & 0x7F;
		mode = reg;
	}
	Sync();
}

static void BMC80013BPower(void) {
	Sync();
	setchr8(0);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, BMC80013BWrite);
}

static void BMC80013BReset(void) {
	regs[0] = regs[1] = mode = 0;
	Sync();
}

static void BMC80013BRestore(int version) {
	Sync();
}

void BMC80013B_Init(CartInfo *info) {
	info->Power = BMC80013BPower;
	info->Reset = BMC80013BReset;
	GameStateRestore = BMC80013BRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
