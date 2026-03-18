#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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

#include "mapinc.h"

static uint8 preg[4];
static uint8 creg[8];
static uint8 nt[4];
static uint8 IRQa;
static uint16 IRQCount;

static SFORMAT StateRegs[] = {
	{ preg, 4, "PREG" },
    { creg, 8, "CREG" },
    { nt, 4, "NREG" },
    { &IRQa, 1, "IRQA" },
    { &IRQCount, 2, "IRQC" },
	{ 0 }
};

static void Sync(void) {
    int i;
    setprg8(0x8000, preg[0]);
    setprg8(0xA000, preg[1]);
    setprg8(0xC000, preg[2]);
    setprg8(0xE000, ~0);
    for (i = 0; i < 8; i++) setchr1(i << 10, creg[i]);
    setmirrorw(nt[0] & 1, nt[1] & 1, nt[2] & 1, nt[3] & 1);
}

static DECLFW(M417Write) {
    switch ((A >> 4) & 7) {
    case 0: preg[A & 3] = V; Sync(); break;
    case 1: creg[0 | (A & 3)] = V; Sync(); break;
    case 2: creg[4 | (A & 3)] = V; Sync(); break;break;
    case 3: IRQCount = 0; IRQa = 1; break;
    case 4: IRQa = 0; X6502_IRQEnd(FCEU_IQEXT); break;
    case 5: nt[A & 3] = V; Sync(); break;
    }
}

static void M417Power(void) {
	Sync();
    SetReadHandler(0x8000, 0xFFFF, CartBR);
    SetWriteHandler(0x8000, 0xFFFF, M417Write);
}

static void M417IRQHook(int a) {
    IRQCount += a;
    if (IRQa && IRQCount > 1024)
        X6502_IRQBegin(FCEU_IQEXT);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper417_Init(CartInfo *info) {
	info->Power = M417Power;
    MapIRQHook = M417IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
