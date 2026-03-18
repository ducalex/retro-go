#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* NES 2.0 Mapper 357 is used for a 4-in-1 multicart (cartridge ID 4602) from Bit Corp.
 * The first game is Bit Corp's hack of the YUNG-08 conversion of Super Mario Brothers 2 (J) named Mr. Mary 2,
 * the other three games are UNROM games.
 *
 * Implementation is modified so reset actually sets the correct dipswitch (or outer banks) for each of the 4 games
 */

#include "mapinc.h"

static uint8 preg[4];
static uint8 dipswitch;
static uint8 IRQa;
static uint16 IRQCount;

static const uint8 banks[8] = { 4, 3, 5, 3, 6, 3, 7, 3 };
static const uint8 outer_bank[4] = { 0x00, 0x08, 0x10, 0x18 };

static SFORMAT StateRegs[] =
{
	{ &IRQCount, 2 | FCEUSTATE_RLSB, "IRQC" },
	{ &IRQa, 1 | FCEUSTATE_RLSB, "IRQA" },
	{ &dipswitch, 1, "DPSW" },
	{ &preg, 4, "REG" },
	{ 0 }
};

static void Sync(void) {
	if (dipswitch == 0) {
		/* SMB2J Mode */
		setprg4(0x5000, 16);
		setprg8(0x6000, preg[1] ? 0 : 2);
		setprg8(0x8000, 1);
		setprg8(0xa000, 0);
		setprg8(0xc000, banks[preg[0]]);
		setprg8(0xe000, preg[1] ? 8 : 10);
	} else {
		/* UNROM Mode */
		setprg16(0x8000, outer_bank[dipswitch] | preg[2]);
		setprg16(0xc000, outer_bank[dipswitch] | 7);
	}
	setchr8(0);
	setmirror(dipswitch == 3 ? MI_H : MI_V);
}

static DECLFW(M357WriteLo) {
	switch (A & 0x71ff) {
		case 0x4022: preg[0] = V & 7; Sync(); break;
		case 0x4120: preg[1] = V & 1; Sync(); break;
	}
}

static DECLFW(M357WriteIRQ) {
	IRQa = V & 1;
	if (!IRQa) {
		IRQCount = 0;
		X6502_IRQEnd(FCEU_IQEXT);
	}
}

static DECLFW(M357WriteUNROM) {
	preg[2] = V & 7;
	Sync();
}

static void M357Power(void) {
	preg[0] = 0;
	preg[1] = 0;
	IRQa = IRQCount = 0;
	Sync();
	SetReadHandler(0x5000, 0xffff, CartBR);
	SetWriteHandler(0x4022, 0x4022, M357WriteLo);
	SetWriteHandler(0x4120, 0x4120, M357WriteLo);
	SetWriteHandler(0x4122, 0x4122, M357WriteIRQ);
	SetWriteHandler(0x8000, 0xffff, M357WriteUNROM);
}

static void M357Reset(void) {
	IRQa = IRQCount = 0;
	dipswitch++;
	dipswitch &= 3;
	Sync();
}

static void FP_FASTAPASS(1) M357IRQHook(int a) {
	if (IRQa) {
		if (IRQCount < 4096)
			IRQCount += a;
		else {
			IRQa = 0;
			X6502_IRQBegin(FCEU_IQEXT);
		}
	}
}

static void StateRestore(int version) {
	Sync();
}

void Mapper357_Init(CartInfo *info) {
	info->Reset = M357Reset;
	info->Power = M357Power;
	MapIRQHook = M357IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
