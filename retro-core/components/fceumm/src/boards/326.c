#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
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
 *
 * NES 2.0 Mapper 326 is used for a bootleg version of Contra/Gryzor.
 * as implemented from
 * http://forums.nesdev.org/viewtopic.php?f=9&t=17352&p=218722#p218722
 */

#include "mapinc.h"

static uint8 PRG[3], CHR[8], NTAPage[4];

static SFORMAT StateRegs[] =
{
	{ PRG, 3, "PRG" },
	{ CHR, 8, "CHR" },
	{ NTAPage, 4, "NT" },
	{ 0 }
};

static void SyncPRG(void) {
	setprg8(0x8000, PRG[0]);
	setprg8(0xA000, PRG[1]);
	setprg8(0xC000, PRG[2]);
	setprg8(0xE000, ~0);
}

static void DoCHR(int x, uint8 V) {
	CHR[x] = V;
	setchr1(x << 10, V);
}

static void FixCHR(void) {
	int x;
	for (x = 0; x < 8; x++)
		DoCHR(x, CHR[x]);
}

static void FASTAPASS(2) DoNTARAM(int w, uint8 V) {
	NTAPage[w] = V;
	setntamem(NTARAM + ((V & 1) << 10), 1, w);
}

static void FixNTAR(void) {
	int x;
	for (x = 0; x < 4; x++)
		DoNTARAM(x, NTAPage[x]);
}

static DECLFW(M326Write) {
	switch (A & 0xE010) {
	case 0x8000: PRG[0] = V; SyncPRG(); break;
	case 0xA000: PRG[1] = V; SyncPRG(); break;
	case 0xC000: PRG[2] = V; SyncPRG(); break;
	}

	A &= 0x801F;
	if ((A >= 0x8010) && (A <= 0x8017))
		DoCHR(A - 0x8010, V);
	else if ((A >= 0x8018) && (A <= 0x801B))
		DoNTARAM(A - 0x8018, V);
}

static void M326Power(void) {
	SyncPRG();
	FixCHR();
	FixNTAR();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M326Write);
}

static void StateRestore(int version) {
	SyncPRG();
	FixCHR();
	FixNTAR();
}

void Mapper326_Init(CartInfo *info) {
	info->Power = M326Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
