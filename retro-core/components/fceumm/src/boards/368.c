#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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

static uint8 preg;
static uint8 latch;
static uint8 IRQa;
static uint16 IRQCount;

static const uint8 banks[8] = { 4, 3, 5, 3, 6, 3, 7, 3 };

static SFORMAT StateRegs[] =
{
	{ &IRQCount, 2 | FCEUSTATE_RLSB, "IRQC" },
	{ &IRQa, 1, "IRQA" },
	{ &latch, 1, "LATC" },
	{ &preg, 1, "REG" },
	{ 0 }
};

static void Sync(void) {
	setprg8(0x6000, 2);
	setprg8(0x8000, 1);
	setprg8(0xa000, 0);
	setprg8(0xc000, banks[preg]);
	setprg8(0xe000, 8);
	setchr8(0);
}

static DECLFW(M368WritePRG) {
    preg = V & 7;
    Sync();
}

static DECLFW(M368WriteIRQ) {
	latch = V & 0x53;
    IRQa = V & 1;
	if (!IRQa) {
		IRQCount = 0;
		X6502_IRQEnd(FCEU_IQEXT);
	}
}

static DECLFR(M368Read) {
    return (latch | 0xBA);
}

static void M368Power(void) {
	preg = 0;
	latch = 0;
	IRQa = IRQCount = 0;
	Sync();
    SetReadHandler(0x4122, 0x4122, M368Read);
	SetReadHandler(0x6000, 0x7FFF, CartBR);
    SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4022, 0x4022, M368WritePRG);
	SetWriteHandler(0x4120, 0x4120, M368WritePRG);
	SetWriteHandler(0x4122, 0x4122, M368WriteIRQ);
}

static void M368Reset(void) {
	IRQa = IRQCount = 0;
	Sync();
}

static void FP_FASTAPASS(1) M368IRQHook(int a) {
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

void Mapper368_Init(CartInfo *info) {
	info->Reset = M368Reset;
	info->Power = M368Power;
	MapIRQHook = M368IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
