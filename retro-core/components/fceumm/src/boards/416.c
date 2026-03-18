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



#include "mapinc.h"

static uint8 reg;
static uint8 smb2j_reg;
static uint8 IRQa;
static uint16 IRQCount;

static void Sync(void) {
	if (reg & 8) {
		uint8 prg = ((reg >> 5) & 1) | ((reg >> 6) & 2) | ((reg >> 1) & 4);
		switch ((reg >> 6) & 3) {
		case 0:
			setprg8(0x8000, prg << 1);
			setprg8(0xA000, prg << 1);
			setprg8(0xC000, prg << 1);
			setprg8(0xE000, prg << 1);
			break;
		case 1:
			setprg16(0x8000, prg);
			setprg16(0xC000, prg);
			break;
		case 2:
		case 3:
			setprg32(0x8000, prg >> 1);
			break;
		}
	} else {
		setprg8(0x8000, 0x0);
		setprg8(0xA000, 0x1);
		setprg8(0xC000, smb2j_reg);
		setprg8(0xE000, 0x3);
	}
	setprg8(0x6000, 0x7);
	setchr8((reg >> 1) & 3);
	setmirror(((reg >> 2) & 1) ^ 1);
}

static DECLFW(M416Write4) {
	switch (A & 0xD160) {
	case 0x4120:
		IRQa = V & 1;
		if (!IRQa)
			IRQCount = 0;
		X6502_IRQEnd(FCEU_IQEXT);
		break;
	case 0x4020:
		smb2j_reg = ((V & 1) << 2) | ((V & 6) >> 1);
		Sync();
		break;
	}
}

static DECLFW(M416Write8) {
	reg = V;
	Sync();
}

static void M416Power(void) {
	reg = smb2j_reg = IRQa = IRQCount = 0;
	Sync();
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x4020, 0x5FFF, M416Write4);
	SetWriteHandler(0x8000, 0x8000, M416Write8);
}

static void FP_FASTAPASS(1) M416IRQHook(int a) {
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

void Mapper416_Init(CartInfo *info) {
	info->Power = M416Power;
	MapIRQHook  = M416IRQHook;
	GameStateRestore = StateRestore;
	AddExState(&reg, 1, 0, "REGS");
	AddExState(&smb2j_reg, 1, 0, "SMBJ");
	AddExState(&IRQa, 1, 0, "IRQa");
	AddExState(&IRQCount, 2, 0, "IRQC");
}
#endif
