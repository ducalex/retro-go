#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
 *  Copyright (C) 2019 Libretro Team
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
 *
 * TXC/Micro Genius simplified mapper
 * updated 06-2019 http://wiki.nesdev.com/w/index.php/INES_Mapper_036
 *
 * Known games:
 * - Strike Wolf (Asia) (Unl)
 * - Policeman (Gluk Video) (unl)
 * - F-15 City War (Spain) (Gluk Video) (Unl)
 *
 * TXC mappers, originally much complex banksitching
 *
 * 01-22111-000 (05-00002-010) (132, 22211) - MGC-001 Qi Wang
 * 01-22110-000 (52S         )              - MGC-002 2-in-1 Gun
 * 01-22111-100 (02-00002-010) (173       ) - MGC-008 Mahjong Block
 *                             (079       ) - MGC-012 Poke Block
 * 01-22110-200 (05-00002-010) (036       ) - MGC-014 Strike Wolf
 * 01-22000-400 (05-00002-010) (036       ) - MGC-015 Policeman
 * 01-22017-000 (05-PT017-080) (189       ) - MGC-017 Thunder Warrior
 * 01-11160-000 (04-02310-000) (   , 11160) - MGC-023 6-in-1
 * 01-22270-000 (05-00002-010) (132, 22211) - MGC-xxx Creatom
 * 01-22200-400 (------------) (079       ) - ET.03   F-15 City War
 *                             (172       ) -         1991 Du Ma Racing
 *
 */

/* added 2020-2-16
 * Updated based on latest source
 * Mappers 36, 132, 173
 * Mappers 136, 147, 172
 */

#include "mapinc.h"

typedef struct {
	uint8 mask;
	uint8 isJV001;
	uint8 accumulator;
	uint8 inverter;
	uint8 staging;
	uint8 output;
	uint8 increase;
	uint8 Y;
	uint8 invert;
} TXC;

static TXC txc;

static void Dummyfunc(void) { }
static void (*WSync)(void) = Dummyfunc;

static SFORMAT StateRegs[] =
{
	{ &txc.accumulator, 1, "ACC0" },
	{ &txc.inverter,    1, "INVR" },
	{ &txc.staging,     1, "STG0" },
	{ &txc.output,      1, "OUT0" },
	{ &txc.increase,    1, "INC0" },
	{ &txc.Y,        1, "YFLG" },
	{ &txc.invert,      1, "INVT" },
	{ 0 }
};

static uint8 TXC_CMDRead(void) {
	uint8 ret = ((txc.accumulator & txc.mask) | ((txc.inverter ^ txc.invert) & ~txc.mask));
	txc.Y = !txc.invert || ((ret & 0x10) != 0);
	WSync();
	return ret;
}

static DECLFW(TXC_CMDWrite) {
	if (A & 0x8000) {
	  if (txc.isJV001)
		 txc.output = (txc.accumulator & 0x0F) | (txc.inverter & 0xF0);
	  else
		 txc.output = (txc.accumulator & 0x0F) | ((txc.inverter << 1) & 0x10);
	} else {
	  switch (A & 0x103) {
	  case 0x100:
		 if (txc.increase)
			txc.accumulator++;
		 else
			txc.accumulator = ((txc.accumulator & ~txc.mask) | ((txc.staging ^ txc.invert) & txc.mask));
		 break;
	  case 0x101:
		 txc.invert = (V & 0x01) ? 0xFF : 0x00;
		 break;
	  case 0x102:
		 txc.staging = V & txc.mask;
		 txc.inverter = V & ~txc.mask;
		 break;
	  case 0x103:
		 txc.increase = ((V & 0x01) != 0);
		 break;
	  }
	}
	txc.Y = !txc.invert || ((V & 0x10) != 0);
	WSync();
}

static void TXCRegReset(void) {
	txc.output      = 0;
	txc.accumulator = 0;
	txc.inverter    = 0;
	txc.staging     = 0;
	txc.increase    = 0;
	txc.Y       = 0;
	txc.mask        = txc.isJV001 ? 0x0F : 0x07;
	txc.invert      = txc.isJV001 ? 0xFF : 0x00;

	WSync();
}

static void GenTXCPower(void) {
	TXCRegReset();
}

static void StateRestore(int version) {
	WSync();
}

static void GenTXC_Init(CartInfo *info, void (*proc)(void), uint32 jv001) {
	txc.isJV001 = jv001;
	WSync   = proc;
	GameStateRestore = StateRestore;
	AddExState(StateRegs, ~0, 0, 0);
}

static int CheckHash(CartInfo *info) {
	int x = 0;
	uint64 partialmd5 = 0;

	/* These carts do not work with new mapper implementation.
	* This is a hack to use previous mapper implementation for such carts. */
	for (x = 0; x < 8; x++)
	  partialmd5 |= (uint64)info->MD5[15 - x] << (x * 8);
	switch (partialmd5) {
	case 0x2dd8f958850f21f4LL: /* Jin Gwok Sei Chuen Saang (Ch) [U][!] */
	  FCEU_printf(" WARNING: Using alternate mapper implementation.\n");
	  UNL22211_Init(info);
	  return 1;
	}
	return 0;
}

/* --------------- Mapper 36 --------------- */

static uint8 creg = 0;

static void M36Sync(void) {
	setprg32(0x8000, txc.output & 0x03);
	setchr8(creg & 0x0F);
}

static DECLFW(M36Write) {
	if ((A & 0xF200) == 0x4200) creg = V;
	TXC_CMDWrite(A, (V >> 4) & 0x03);
}

static DECLFR(M36Read) {
	uint8 ret = X.DB;
	if ((A & 0x103) == 0x100)
	  ret = (X.DB & 0xCF) | ((TXC_CMDRead() << 4) & 0x30);
	return ret;
}

static void M36Power(void) {
	creg = 0;
	GenTXCPower();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, M36Read);
	SetWriteHandler(0x4100, 0xFFFF, M36Write);
}

void Mapper36_Init(CartInfo *info) {
	GenTXC_Init(info, M36Sync, 0);
	info->Power = M36Power;
	AddExState(&creg, 1, 0, "CREG");
}

/* --------------- Mapper 132 --------------- */

static void M132Sync(void) {
	setprg32(0x8000, (txc.output >> 2) & 0x01);
	setchr8(txc.output & 0x03);
}

static DECLFW(M132Write) {
	TXC_CMDWrite(A, V & 0x0F);
}

static DECLFR(M132Read) {
	uint8 ret = X.DB;
	if ((A & 0x103) == 0x100)
	  ret = ((X.DB & 0xF0) | (TXC_CMDRead() & 0x0F));
	return ret;
}

static void M132Power(void) {
	GenTXCPower();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, M132Read);
	SetWriteHandler(0x4100, 0xFFFF, M132Write);
}

void Mapper132_Init(CartInfo *info) {
	if (CheckHash(info) != 0) return;
	GenTXC_Init(info, M132Sync, 0);
	info->Power = M132Power;
}

/* --------------- Mapper 173 --------------- */

static void M173Sync(void) {
	setprg32(0x8000, 0);
	if (CHRsize[0] > 0x2000)
	  setchr8(((txc.output & 0x01) | (txc.Y ? 0x02 : 0x00) | ((txc.output & 2) << 0x01)));
	else
	  setchr8(0);
}

void Mapper173_Init(CartInfo *info) {
	GenTXC_Init(info, M173Sync, 0);
	info->Power = M132Power;
}

/* ---------------- Joy/Van ----------------- */

/* --------------- Mapper 136 --------------- */

static void M136Sync(void) {
	setprg32(0x8000, (txc.output >> 4) & 0x01);
	setchr8(txc.output & 0x07);
}

static DECLFW(M136Write) {
	TXC_CMDWrite(A, V & 0x3F);
}

static DECLFR(M136Read) {
	uint8 ret = X.DB;
	if ((A & 0x103) == 0x100)
	  ret = ((X.DB & 0xC0) | (TXC_CMDRead() & 0x3F));
	return ret;
}

static void M136Power(void) {
	GenTXCPower();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, M136Read);
	SetWriteHandler(0x4100, 0xFFFF, M136Write);
}

void Mapper136_Init(CartInfo *info) {
	GenTXC_Init(info, M136Sync, 1);
	info->Power = M136Power;
}

/* --------------- Mapper 147 --------------- */

static void M147Sync(void) {
	setprg32(0x8000, ((txc.output >> 4) & 0x02) | (txc.output & 0x01));
	setchr8((txc.output >> 1) & 0x0F);
}

static DECLFW(M147Write) {
	TXC_CMDWrite(A, ((V >> 2) & 0x3F) | ((V << 6) & 0xC0));
}

static DECLFR(M147Read) {
	uint8 ret = X.DB;
	if ((A & 0x103) == 0x100) {
	  uint8 value = TXC_CMDRead();
	  ret = ((value << 2) & 0xFC) | ((value >> 6) & 0x03);
	}
	return ret;
}

static void M147Power(void) {
	GenTXCPower();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, M147Read);
	SetWriteHandler(0x4100, 0xFFFF, M147Write);
}

void Mapper147_Init(CartInfo *info) {
	GenTXC_Init(info, M147Sync, 1);
	info->Power = M147Power;
}

/* --------------- Mapper 172 --------------- */

static void M172Sync(void) {
	setprg32(0x8000, 0);
	setchr8(txc.output);
	setmirror(txc.invert ? 1 : 0);
}

static uint8 GetValue(uint8 value) {
	return (((value << 5) & 0x20) | ((value << 3) & 0x10) | ((value << 1) & 0x08) |
		 ((value >> 1) & 0x04) | ((value >> 3) & 0x02) | ((value >> 5) & 0x01));
}

static DECLFW(M172Write) {
	TXC_CMDWrite(A, GetValue(V));
}

static DECLFR(M172Read) {
	uint8 ret = X.DB;
	if ((A & 0x103) == 0x100)
	  ret = (X.DB & 0xC0) | GetValue(TXC_CMDRead());
	return ret;
}

static void M172Power(void) {
	GenTXCPower();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x5FFF, M172Read);
	SetWriteHandler(0x4100, 0xFFFF, M172Write);
}

void Mapper172_Init(CartInfo *info) {
	GenTXC_Init(info, M172Sync, 1);
	info->Power = M172Power;
}

/* === LEGACY MAPPER IMPLEMENTATION === */

static uint8 reg[4], cmd, is172, is173;

static SFORMAT UNL22211StateRegs[] =
{
	{ reg, 4, "REGS" },
	{ &cmd, 1, "CMD" },
	{ 0 }
};

static void UNL22211Sync(void) {
	setprg32(0x8000, (reg[2] >> 2) & 1);
	if (is172)
	  setchr8((((cmd ^ reg[2]) >> 3) & 2) | (((cmd ^ reg[2]) >> 5) & 1));	/* 1991 DU MA Racing probably CHR bank sequence is WRONG, so it is possible to
														  * rearrange CHR banks for normal UNIF board and mapper 172 is unneccessary */
	else
	  setchr8(reg[2] & 3);
}

static DECLFW(UNL22211WriteLo) {
/*	FCEU_printf("bs %04x %02x\n",A,V); */
	reg[A & 3] = V;
}

static DECLFW(UNL22211WriteHi) {
/*	FCEU_printf("bs %04x %02x\n",A,V); */
	cmd = V;
	UNL22211Sync();
}

static DECLFR(UNL22211ReadLo) {
	return (reg[1] ^ reg[2]) | (is173 ? 0x01 : 0x40);
#if 0
	if(reg[3])
	  return reg[2];
	else
	  return X.DB;
#endif
}

static void UNL22211Power(void) {
	UNL22211Sync();
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetReadHandler(0x4100, 0x4100, UNL22211ReadLo);
	SetWriteHandler(0x4100, 0x4103, UNL22211WriteLo);
	SetWriteHandler(0x8000, 0xFFFF, UNL22211WriteHi);
}

static void UNL22211StateRestore(int version) {
	UNL22211Sync();
}

void UNL22211_Init(CartInfo *info) {
	is172 = 0;
	is173 = 0;
	info->Power = UNL22211Power;
	GameStateRestore = UNL22211StateRestore;
	AddExState(&UNL22211StateRegs, ~0, 0, 0);
}
#endif
