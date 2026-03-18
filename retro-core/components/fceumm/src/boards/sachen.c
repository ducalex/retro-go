#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *  Copyright (C) 2020
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

/* 2020-2-3 - updated mapper 150/243 */

#include "mapinc.h"

static uint8 cmd, dip;
static uint8 latch[8];
static uint8 mapperNum;

static void S74LS374MSync(uint8 mirr) {
	switch (mirr & 3) {
	case 0: setmirror(MI_V); break;
	case 1: setmirror(MI_H); break;
	case 2: setmirrorw(0, 1, 1, 1); break;
	case 3: setmirror(MI_0); break;
	}
}

static int type;
static void S8259Synco(void) {
	int x;
	setprg32(0x8000, latch[5] & 7);

	if (!UNIFchrrama) {		/* No CHR RAM?  Then BS'ing is ok. */
		for (x = 0; x < 4; x++) {
			int bank;
			if (latch[7] & 1)
				bank = (latch[0] & 0x7) | ((latch[4] & 7) << 3);
			else
				bank = (latch[x] & 0x7) | ((latch[4] & 7) << 3);
			switch (type) {
			case 00: bank = (bank << 1) | (x & 1); setchr2(0x800 * x, bank); break;
			case 01: setchr2(0x800 * x, bank); break;
			case 02: bank = (bank << 2) | (x & 3); setchr2(0x800 * x, bank); break;
			case 03:
				bank = latch[x] & 7;
				switch (x & 3) {
				case 01: bank |= (latch[4] & 1) << 4; break;
				case 02: bank |= (latch[4] & 2) << 3; break;
				case 03: bank |= ((latch[4] & 4) << 2) | ((latch[6] & 1) << 3); break;
				}
				setchr1(0x400 * x, bank);
				setchr4(0x1000, ~0);
				break;
			}
		}
	}
	if (!(latch[7] & 1))
		S74LS374MSync(latch[7] >> 1);
	else
		setmirror(MI_V);
}

static DECLFW(S8259Write) {
	A &= 0x4101;
	if (A == 0x4100)
		cmd = V;
	else {
		latch[cmd & 7] = V;
		S8259Synco();
	}
}

static void S8259Reset(void) {
	int x;
	cmd = 0;

	for (x = 0; x < 8; x++) latch[x] = 0;
	setchr8(0);

	S8259Synco();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4100, 0x7FFF, S8259Write);
}

static void S8259Restore(int version) {
	S8259Synco();
}

void S8259A_Init(CartInfo *info) {	/* Kevin's Horton 141 mapper */
	info->Power = S8259Reset;
	GameStateRestore = S8259Restore;
	AddExState(latch, 8, 0, "LATC");
	AddExState(&cmd, 1, 0, "CMD");
	type = 0;
}

void S8259B_Init(CartInfo *info) {	/* Kevin's Horton 138 mapper */
	info->Power = S8259Reset;
	GameStateRestore = S8259Restore;
	AddExState(latch, 8, 0, "LATC");
	AddExState(&cmd, 1, 0, "CMD");
	type = 1;
}

void S8259C_Init(CartInfo *info) {	/* Kevin's Horton 139 mapper */
	info->Power = S8259Reset;
	GameStateRestore = S8259Restore;
	AddExState(latch, 8, 0, "LATC");
	AddExState(&cmd, 1, 0, "CMD");
	type = 2;
}

void S8259D_Init(CartInfo *info) {	/* Kevin's Horton 137 mapper */
	info->Power = S8259Reset;
	GameStateRestore = S8259Restore;
	AddExState(latch, 8, 0, "LATC");
	AddExState(&cmd, 1, 0, "CMD");
	type = 3;
}

static void (*WSync)(void);

static DECLFW(SAWrite) {
	if (A & 0x100) {
		latch[0] = V;
		WSync();
	}
}

static void SAPower(void) {
	latch[0] = 0;
	WSync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4100, 0x5FFF, SAWrite);
}

static void SARestore(int version) {
	WSync();
}

static DECLFW(SADWrite) {
	latch[0] = V;
	WSync();
}

static void SADPower(void) {
	latch[0] = 0;
	WSync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, SADWrite);
}

static void SA0161MSynco(void) {
	setprg32(0x8000, (latch[0] >> 3) & 1);
	setchr8(latch[0] & 7);
}

static void SA72007Synco(void) {
	setprg32(0x8000, 0);
	setchr8(latch[0] >> 7);
}

static void SA009Synco(void) {
	setprg32(0x8000, 0);
	setchr8(latch[0] & 1);
}

static void SA72008Synco(void) {
	setprg32(0x8000, (latch[0] >> 2) & 1);
	setchr8(latch[0] & 3);
}

void SA0161M_Init(CartInfo *info) {
	WSync = SA0161MSynco;
	GameStateRestore = SARestore;
	info->Power = SAPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

void SA72007_Init(CartInfo *info) {
	WSync = SA72007Synco;
	GameStateRestore = SARestore;
	info->Power = SAPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

void SA72008_Init(CartInfo *info) {
	WSync = SA72008Synco;
	GameStateRestore = SARestore;
	info->Power = SAPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

void SA009_Init(CartInfo *info) {
	WSync = SA009Synco;
	GameStateRestore = SARestore;
	info->Power = SAPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

void SA0036_Init(CartInfo *info) {
	WSync = SA72007Synco;
	GameStateRestore = SARestore;
	info->Power = SADPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

void SA0037_Init(CartInfo *info) {
	WSync = SA0161MSynco;
	GameStateRestore = SARestore;
	info->Power = SADPower;
	AddExState(&latch[0], 1, 0, "LATC");
}

/* --------------------------------------------- */

static DECLFR(TCA01Read) {
	uint8 ret;
	if ((A & 0x4100) == 0x4100)
		ret = (X.DB & 0xC0) | ((~A) & 0x3F);
	else
		ret = X.DB;
	return ret;
}

static void TCA01Power(void) {
	setprg16(0x8000, 0);
	setprg16(0xC000, 1);
	setchr8(0);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, TCA01Read);
}

void TCA01_Init(CartInfo *info) {
	info->Power = TCA01Power;
}

/* ------------------ Mapper 150 --------------------- */
/* ------------------ Mapper 243 --------------------- */

/* Mapper 150 - SA-015 / SA-630 / Unif UNL-Sachen-74LS374N */
/* Mapper 243 - SA-020A */

static void S74LS374NSynco(void) {
	uint32 chrBank;
	if (mapperNum == 150)
		chrBank = (latch[6] & 3) | ((latch[4] << 2) & 4) | (latch[2] << 3);
	else
		chrBank = (latch[2] & 1) | ((latch[4] << 1) & 2) | (latch[6] << 2);

	setprg32(0x8000, (latch[2] & 1) | latch[5]);
	setchr8(chrBank);

	switch ((latch[7] >> 1) & 3) {
	case 0: setmirrorw(0, 1, 1, 1); break;
	case 1: setmirror(MI_H); break;
	case 2: setmirror(MI_V); break;
	case 3: setmirror(MI_0); break;
	}
}

static DECLFR(S74LS374NRead) {
	uint8 ret;
	if ((A & 0xC101) == 0x4101) {
		if (dip & 1)
			ret = (latch[cmd] & 3) | (X.DB & 0xFC);
		else
			ret = (latch[cmd] & 7) | (X.DB & 0xF8);
	} else {
		ret = X.DB;
	}
	return ret;
}

static DECLFW(S74LS374NWrite) {
	if (dip & 1)
		V |= 4;
	switch (A & 0xC101) {
	case 0x4100:
		cmd = V & 7;
		break;
	case 0x4101:
		latch[cmd] = V & 7;
		S74LS374NSynco();
		break;
	}
}

static void S74LS374NRestore(int version) {
	S74LS374NSynco();
}

static void S74LS374NReset(void) {
	dip ^= 1;
	latch[0] = latch[1] = latch[2] = latch[3] = 0;
	latch[4] = latch[5] = latch[6] = latch[7] = 0;
	S74LS374NSynco();
}

static void S74LS374NPower(void) {
	dip = 0;
	latch[0] = latch[1] = latch[2] = latch[3] = 0;
	latch[4] = latch[5] = latch[6] = latch[7] = 0;
	S74LS374NSynco();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x4100, 0x7FFF, S74LS374NWrite);
	if (mapperNum == 150)
		SetReadHandler(0x4100, 0x7FFF, S74LS374NRead);
}

void S74LS374N_Init(CartInfo *info) {
	mapperNum = info->mapper;
	info->Power = S74LS374NPower;
	info->Reset = S74LS374NReset;
	GameStateRestore = S74LS374NRestore;
	AddExState(latch, 8, 0, "LATC");
	AddExState(&cmd, 1, 0, "CMD");
}

static DECLFR(Mapper553Read) {
	return 0x3A;
}

static void Mapper553Power(void) {
	setprg16(0xC000, 0);
	setchr8(0);
	SetReadHandler(0x8000, 0xBFFF, Mapper553Read);
	SetReadHandler(0xC000, 0xFFFF, CartBR);
}

void Mapper553_Init(CartInfo *info) {
	info->Power = Mapper553Power;
}
#endif
