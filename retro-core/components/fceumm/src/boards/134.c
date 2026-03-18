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

/* Chipset used on various PCBs named WX-KB4K, T4A54A, BS-5652... */
/* "Rockman 3" on YH-322 and "King of Fighters 97" on "Super 6-in-1" enable interrupts without initializing the frame IRQ register and therefore freeze on real hardware.
   They can run if another game is selected that does initialize the frame IRQ register, then soft-resetting to the menu and selecting the previously-freezing games. */

#include "mapinc.h"
#include "mmc3.h"

static uint8 dip;

static void Mapper134_PRGWrap(uint32 A, uint8 V) {
	int prgAND =EXPREGS[1] &0x04? 0x0F: 0x1F;
	int prgOR  =(EXPREGS[1] <<4 &0x30) | (EXPREGS[0] <<2 &0x40);
	if (EXPREGS[1] &0x80) { /* NROM mode */
		if (EXPREGS[1] &0x08) { /* NROM-128 mode */
			setprg8(0x8000, (((DRegBuf[6] &~1) |0) &prgAND) | (prgOR &~prgAND));
			setprg8(0xA000, (((DRegBuf[6] &~1) |1) &prgAND) | (prgOR &~prgAND));
			setprg8(0xC000, (((DRegBuf[6] &~1) |0) &prgAND) | (prgOR &~prgAND));
			setprg8(0xE000, (((DRegBuf[6] &~1) |1) &prgAND) | (prgOR &~prgAND));
		} else {                /* NROM-256 mode */
			setprg8(0x8000, (((DRegBuf[6] &~3) |0) &prgAND) | (prgOR &~prgAND));
			setprg8(0xA000, (((DRegBuf[6] &~3) |1) &prgAND) | (prgOR &~prgAND));
			setprg8(0xC000, (((DRegBuf[6] &~3) |2) &prgAND) | (prgOR &~prgAND));
			setprg8(0xE000, (((DRegBuf[6] &~3) |3) &prgAND) | (prgOR &~prgAND));
		}
	} else
		setprg8(A, (V &prgAND) | (prgOR &~prgAND));
}

static void Mapper134_CHRWrap(uint32 A, uint8 V) {
	int chrAND =EXPREGS[1] &0x40? 0x7F: 0xFF;
	int chrOR  =(EXPREGS[1] <<3 &0x180) | (EXPREGS[0] <<4 &0x200);
	if (EXPREGS[0] &0x08) V =(EXPREGS[2] <<3) | (A >>10 &7); /* In CNROM mode, outer bank register 2 replaces the MMC3's CHR registers, and CHR A10-A12 are PPU A10-A12. */
	setchr1(A, (V &chrAND) | (chrOR &~chrAND));
}

static DECLFR(Mapper134_Read) {
	return EXPREGS[0] &0x40? dip: CartBR(A);
}

static DECLFW(Mapper134_Write) {
	if (~EXPREGS[0] &0x80) {
		EXPREGS[A &3] =V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	} else
	if ((A &3) ==2) {
		EXPREGS[A &3] = (EXPREGS[A &3] &~3) | (V &3);
		FixMMC3CHR(MMC3_cmd);
	}
	CartBW(A, V);
}

static void Mapper134_Reset(void) {
	dip++;
	dip &= 15;
	EXPREGS[0] =EXPREGS[1] =EXPREGS[2] =EXPREGS[3] =0;
	MMC3RegReset();
}

static void Mapper134_Power(void) {
	dip =0;
	EXPREGS[0] =EXPREGS[1] =EXPREGS[2] =EXPREGS[3] =0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, Mapper134_Write);
	SetReadHandler(0x8000, 0xFFFF, Mapper134_Read);
}

void Mapper134_Init(CartInfo *info) {
	GenMMC3_Init(info, 256, 256, info->iNES2? (info->PRGRamSize + info->PRGRamSaveSize) /1024: 8, info->battery);
	cwrap = Mapper134_CHRWrap;
	pwrap = Mapper134_PRGWrap;
	info->Power = Mapper134_Power;
	info->Reset = Mapper134_Reset;
	AddExState(EXPREGS, 4, 0, "EXPR");
	AddExState(&dip, 1, 0, "DIPS");
}
#endif
