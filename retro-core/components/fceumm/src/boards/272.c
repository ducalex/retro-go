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
 * NES 2.0 Mapper 272 is used for a bootleg implementation of
 * 悪魔城 Special: ぼくDracula君 (Akumajō Special: Boku Dracula-kun).
 *
 * as implemented from
 * https://forums.nesdev.org/viewtopic.php?f=9&t=15302&start=60#p205862
 *
 */

#include "mapinc.h"

static uint8 prg[2];
static uint8 chr[8];
static uint8 mirr;
static uint8 pal_mirr;
static uint8 last_pa13;
static uint8 IRQCount;
static uint8 IRQa;

static SFORMAT StateRegs[] =
{
	{ prg, 2, "PRG" },
	{ chr, 8, "CHR" },
	{ &mirr, 1, "MIRR" },
	{ &last_pa13, 1, "PA13" },
	{ &IRQCount, 1, "CNTR" },
	{ &pal_mirr, 1, "PALM" },
	{ &IRQa, 1, "CCLK" },
	{ 0 }
};

/* shifts bit from position `bit` into position `pos` of expression `exp` */
#define shi(exp, bit, pos) \
	((((exp) & (1 << (bit))) >> (bit)) << (pos))

static uint32 vrc_addr_mix(uint32 A) {
	/* this game wires A0 to VRC_A0 and A1 to VRC_A1 */
	return (A & 0xf000) | shi(A, 0, 0) | shi(A, 1, 1);
}

static void Sync(void) {
	uint8 i;
	setprg8(0x8000, prg[0]);
	setprg8(0xa000, prg[1]);
	setprg16(0xc000, -1);	
	for (i = 0; i < 8; ++i)
		setchr1(0x400 * i, chr[i]);
	switch (pal_mirr) {
	case 2: setmirror(MI_0); break;
	case 3: setmirror(MI_1); break;
	default: 
		switch (mirr) {
		case 0: setmirror(MI_V); break;
		case 1: setmirror(MI_H); break;
		}
	}
}

static DECLFW(M272Write) {
	/* writes to VRC chip */
	switch (vrc_addr_mix(A)) {
	case 0x8000:
	case 0x8001:
	case 0x8002:
	case 0x8003:
		prg[0] = V;
		break;
	case 0x9000:
	case 0x9001:
	case 0x9002:
	case 0x9003:
		mirr = V & 1;
		break;
	case 0xA000:
	case 0xA001:
	case 0xA002:
	case 0xA003:
		prg[1] = V;
		break;
	case 0xb000: chr[0] = (chr[0] & 0xF0) | (V & 0xF); break;
	case 0xb001: chr[0] = (chr[0] & 0xF) | ((V & 0xF) << 4); break;
	case 0xb002: chr[1] = (chr[1] & 0xF0) | (V & 0xF); break;
	case 0xb003: chr[1] = (chr[1] & 0xF) | ((V & 0xF) << 4); break;
	case 0xc000: chr[2] = (chr[2] & 0xF0) | (V & 0xF); break;
	case 0xc001: chr[2] = (chr[2] & 0xF) | ((V & 0xF) << 4); break;
	case 0xc002: chr[3] = (chr[3] & 0xF0) | (V & 0xF); break;
	case 0xc003: chr[3] = (chr[3] & 0xF) | ((V & 0xF) << 4); break;
	case 0xd000: chr[4] = (chr[4] & 0xF0) | (V & 0xF); break;
	case 0xd001: chr[4] = (chr[4] & 0xF) | ((V & 0xF) << 4); break;
	case 0xd002: chr[5] = (chr[5] & 0xF0) | (V & 0xF); break;
	case 0xd003: chr[5] = (chr[5] & 0xF) | ((V & 0xF) << 4); break;
	case 0xe000: chr[6] = (chr[6] & 0xF0) | (V & 0xF); break;
	case 0xe001: chr[6] = (chr[6] & 0xF) | ((V & 0xF) << 4); break;
	case 0xe002: chr[7] = (chr[7] & 0xF0) | (V & 0xF); break;
	case 0xe003: chr[7] = (chr[7] & 0xF) | ((V & 0xF) << 4); break;
		
	default:
		break;
	}

	/* writes to PAL chip */
	switch (A & 0xC00C) {
	case 0x8004: pal_mirr = V & 3; break;
	case 0x800c: X6502_IRQBegin(FCEU_IQEXT); break;
	case 0xc004: X6502_IRQEnd(FCEU_IQEXT); break;
	case 0xc008: IRQa = 1; break;
	case 0xc00c: IRQa = 0; IRQCount = 0; X6502_IRQEnd(FCEU_IQEXT); break;
	}

	Sync();
}

static void M272Power(void) {
	prg[0] = prg[1] = 0;
	chr[0] = chr[1] = chr[2] = chr[3] = 0;
	chr[4] = chr[5] = chr[6] = chr[7] = 0;
	mirr = pal_mirr = 0;
	last_pa13 = 0;
	IRQCount = 0;
	IRQa = 0;
	Sync();
	SetWriteHandler(0x8000, 0xFFFF, M272Write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void M272Hook(uint32 A) {
	uint8 pa13 = (A >> 13) & 1;
	if ((last_pa13 == 1) && (pa13 == 0)) {
		if (IRQa) {
			IRQCount++;
			if (IRQCount == 84) {
				IRQCount = 0;
				X6502_IRQBegin(FCEU_IQEXT);
			}
		}
	}
	last_pa13 = pa13;
}

static void M272Reset(void) {
	prg[0] = prg[1] = 0;
	chr[0] = chr[1] = chr[2] = chr[3] = 0;
	chr[4] = chr[5] = chr[6] = chr[7] = 0;
	mirr = pal_mirr = 0;
	last_pa13 = 0;
	IRQCount = 0;
	IRQa = 0;
	Sync();
}

static void StateRestore(int version) {
	Sync();
}

void Mapper272_Init(CartInfo *info) {
	info->Power = M272Power;
	info->Reset = M272Reset;
	PPU_hook = M272Hook;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
