#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020 NewRisingSun
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

/* Mapper 422: "Normal" version of the mapper. Represents UNIF boards BS-400R and BS-4040R.
   Mapper 126: Power Joy version of the mapper, connecting CHR A18 and A19 in reverse order.
   Mapper 534: Waixing version of the mapper, inverting the reload value of the MMC3 scanline counter.
*/

#include "mapinc.h"
#include "mmc3.h"

static uint8 reverseCHR_A18_A19;
static uint8 invertC000;
static uint8 dipSwitch;

static void wrapPRG(uint32 A, uint8 V) {
	int prgAND = EXPREGS[0] &0x40? 0x0F: 0x1F; /* 128 KiB or 256 KiB inner PRG bank selection */
	int prgOR  =((EXPREGS[0] <<4 &0x70) | (EXPREGS[0] <<3 &0x180)) &~prgAND; /* outer PRG bank */
	switch(EXPREGS[3] &3) {
		case 0: /* MMC3 PRG mode */
			break;
		case 1:
		case 2: /* NROM-128 mode: MMC3 register 6 applies throughout $8000-$FFFF, MMC3 A13 replaced with CPU A13. */
			V =(DRegBuf[6] &~1) | (A >>13 &1);
			setprg8(A ^0x4000,  (V     &prgAND) | prgOR); /* wrapPRG is only called with A containing the switchable banks, so we need to manually switch the normally fixed banks in this mode as well. */
			break;
		case 3:	/* NROM-256 mode: MMC3 register 6 applies throughout $8000-$FFFF, MMC3 A13-14 replaced with CPU A13-14. */
			V =(DRegBuf[6] &~3) | (A >>13 &3);
			setprg8(A ^0x4000, ((V ^2) &prgAND) | prgOR); /* wrapPRG is only called with A containing the switchable banks, so we need to manually switch the normally fixed banks in this mode as well. */
			break;
	}
	setprg8(A, (V &prgAND) | prgOR);	
}

static void wrapCHR(uint32 A, uint8 V) {
	int chrAND = EXPREGS[0] &0x80? 0x7F: 0xFF; /* 128 KiB or 256 KiB innter CHR bank selection */
	int chrOR; /* outer CHR bank */
	if (reverseCHR_A18_A19) /* Mapper 126 swaps CHR A18 and A19 */
		chrOR =((EXPREGS[0] <<4 &0x080) | (EXPREGS[0] <<3 &0x100) | (EXPREGS[0] <<5 &0x200)) &~chrAND;
	else
		chrOR =EXPREGS[0] <<4 &0x380 &~chrAND;
	
	if (EXPREGS[3] &0x10) /* CNROM mode: 8 KiB inner CHR bank comes from outer bank register #2 */
		setchr8((EXPREGS[2] &(chrAND >>3)) | chrOR >>3);
	else /* MMC3 CHR mode */
		setchr1(A, (V & chrAND) | chrOR);
}

static DECLFW(writeWRAM) {
	if (~EXPREGS[3] &0x80) {
		/* Lock bit clear: Update any outer bank register */
		EXPREGS[A &3] =V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	} else
	if ((A &3) ==2) {
		/* Lock bit set: Only update the bottom one or two bits of the CNROM bank */
		int latchMask =EXPREGS[2] &0x10? 1: 3; /* 16 or 32 KiB inner CHR bank selection */
		EXPREGS[2] &=~latchMask;
		EXPREGS[2] |= V &latchMask;
		FixMMC3CHR(MMC3_cmd);
	}
	CartBW(A, V);
}

static DECLFR(readDIP) {
	uint8 result =CartBR(A);
	if (EXPREGS[1] &1) result = (result &~3) | (dipSwitch &3); /* Replace bottom two bits with solder pad or DIP switch setting if so selected */
	return result;
}

static DECLFW(writeIRQ) {
	MMC3_IRQWrite(A, V ^0xFF);
}

static void reset(void) {
	dipSwitch++; /* Soft-resetting cycles through solder pad or DIP switch settings */
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = 0;
	MMC3RegReset();
}

static void power(void) {
	dipSwitch =0;
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = 0;
	GenMMC3Power();
	SetWriteHandler(0x6000, 0x7FFF, writeWRAM);
	SetReadHandler(0x8000, 0xFFFF, readDIP);
	if (invertC000) SetWriteHandler(0xC000, 0xDFFF, writeIRQ); /* Mapper 534 inverts the MMC3 scanline counter reload value */
}

static void init(CartInfo *info) {
	GenMMC3_Init(info, 512, 256, 8, info->battery);
	cwrap = wrapCHR;
	pwrap = wrapPRG;

	info->Power = power;
	info->Reset = reset;

	AddExState(EXPREGS, 4, 0, "EXPR");
	AddExState(&dipSwitch, 1, 0, "DPSW");
}

void Mapper126_Init(CartInfo *info) {
	reverseCHR_A18_A19 = 1;
	invertC000 = 0;
	init(info);
}

void Mapper422_Init(CartInfo *info) {
	reverseCHR_A18_A19 = 0;
	invertC000 = 0;
	init(info);
}

void Mapper534_Init(CartInfo *info) {
	reverseCHR_A18_A19 = 0;
	invertC000 = 1;
	init(info);
}
#endif
