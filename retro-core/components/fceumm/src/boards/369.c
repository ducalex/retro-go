#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
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

/* Mapper 369 (BMC-N49C-300) - Super Mario Bros. Party multicart */

#include "mapinc.h"
#include "mmc3.h"

static uint8 mode;
static uint8 mmc3_count, mmc3_latch, mmc3_enabled, mmc3_reload;
static uint8 smb2_reg;
static uint8 smb2j_enabled;
static uint16 smb2j_count;

static SFORMAT StateRegs[] = {
	{ &mode, 1, "MODE" },
	{ &mmc3_reload, 1, "IRQR" },
	{ &mmc3_count, 1, "IRQC" },
	{ &mmc3_latch, 1, "IRQL" },
	{ &mmc3_enabled, 1, "IRQA" },
	{ &smb2_reg, 1, "MBRG" },
	{ &smb2j_enabled, 1, "MIRQ" },
	{ &smb2j_count, 2 | FCEUSTATE_RLSB, "MIQC" },
	{ 0 }
};

static void SyncPRG(uint32 A, uint8 V) {
	switch (mode) {
	case 0x00:
	case 0x01: /* NROM */
		setprg32(0x8000, mode & 0x01);
		break;
	case 0x13: /* Mapper 40 */
		setprg8r(0, 0x6000, 0x0E);
		setprg8(0x8000, 0x0C);
		setprg8(0xa000, 0x0D);
		setprg8(0xc000, smb2_reg | 0x08);
		setprg8(0xe000, 0x0F);
		break;
	case 0x37: /* MMC3 128 PRG */
		setprg8r(0x10, 0x6000, 0);
		setprg8(A, (V & 0x0F) | 0x10);
		break;
	case 0xFF: /* MMC3 256 PRG */
		setprg8r(0x10, 0x6000, 0);
		setprg8(A, (V & 0x1F) | 0x20);
		break;
	}
}

static void SyncCHR(uint32 A, uint8 V) {
	switch (mode) {
	case 0x00:
	case 0x01: /* NROM */
	case 0x13: /* Mapper 40 */
		setchr8(mode & 0x03);
		break;
	case 0x37: /* MMC3 128 CHR */
		setchr1(A, (V & 0x7F) | 0x80);
		break;
	case 0xFF: /* MMC3 256 CHR */
		setchr1(A, (V & 0xFF) | 0x100);
		break;
	}
}

static DECLFW(M369WriteLo) {
	if ((A & 0xC100) == 0x4100) {
		mode = V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}

static DECLFW(M369Write) {
	if (mode == 0x13) {
		switch (A & 0xE000) {
		case 0x8000:
			smb2j_enabled = 0;
			smb2j_count = 0;
			X6502_IRQEnd(FCEU_IQEXT);
			break;
		case 0xA000:
			smb2j_enabled = 1;
			break;
		case 0xE000:
			smb2_reg = V & 7;
			FixMMC3PRG(MMC3_cmd);
			FixMMC3CHR(MMC3_cmd);
			break;
		}
	} else {
		switch (A & 0xE001) {
		case 0x8000:
		case 0x8001:
		case 0xA000:
		case 0xA001:
			MMC3_CMDWrite(A, V);
			FixMMC3PRG(MMC3_cmd);
			FixMMC3CHR(MMC3_cmd);
			break;
		case 0xC000:
			mmc3_latch = V;
			break;
		case 0xC001:
			mmc3_reload = 1;
			break;
		case 0xE000:
			X6502_IRQEnd(FCEU_IQEXT);
			mmc3_enabled = 0;
			break;
		case 0xE001:
			mmc3_enabled = 1;
			break;
		}
	}
}

static void FP_FASTAPASS(1) SMB2JIRQHook(int a) {
	if (mode != 0x13)
		return;

	if (smb2j_enabled) {
		if (smb2j_count < 4096)
			smb2j_count += a;
		else {
			smb2j_enabled = 0;
			X6502_IRQBegin(FCEU_IQEXT);
		}
	}
}

static void MMC3IRQHook(void) {
	int32 count = mmc3_count;

	if (mode == 0x13)
		return;
	
	if (!count || mmc3_reload) {
		mmc3_count = mmc3_latch;
		mmc3_reload = 0;
	} else
		mmc3_count--;
	if (count && !mmc3_count && mmc3_enabled)
		X6502_IRQBegin(FCEU_IQEXT);
}

static void M369Reset(void) {
	mode = 0;
	smb2_reg = 0;
	smb2j_enabled = 0;
	smb2j_count = 0;
	mmc3_count = mmc3_latch = mmc3_enabled = 0;
	MMC3RegReset();
}

static void M369Power(void) {
	mode = 0;
	smb2_reg = 0;
	smb2j_enabled = 0;
	smb2j_count = 0;
	mmc3_count = mmc3_latch = mmc3_enabled = 0;
	GenMMC3Power();
	SetWriteHandler(0x4100, 0x4FFF, M369WriteLo);
	SetWriteHandler(0x8000, 0xFFFF, M369Write);
}

void Mapper369_Init(CartInfo *info) {
	GenMMC3_Init(info, 512, 384, 8, info->battery);
	pwrap = SyncPRG;
	cwrap = SyncCHR;
	info->Power = M369Power;
	info->Reset = M369Reset;
	MapIRQHook = SMB2JIRQHook;
	GameHBIRQHook = MMC3IRQHook;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
