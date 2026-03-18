#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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
#include "mmc3.h"

static void UNLA9746PWrap(uint32 A, uint8 V) {
	setprg8(A, EXPREGS[1] <<4 | V &0x0F);
}

static void UNLA9746CWrap(uint32 A, uint8 V) {
	setchr1(A, EXPREGS[1] <<7 | V &0x7F);
}

static DECLFW(UNLA9746WriteOuter) {
	switch(A &1) {
		case 0: EXPREGS[1] =EXPREGS[1] &~1 | V >>3 &1; break;
		case 1: EXPREGS[1] =EXPREGS[1] &~2 | V >>4 &2; break;
	}
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static DECLFW(UNLA9746WriteASIC) {
	int index;
	
	if (A &1)
	{ 	/* Register data */
		if (~EXPREGS[0] &0x20)
		{	/* Scrambled mode inactive */
			MMC3_CMDWrite(A, V);
		}
		else
		{	/* Scrambled mode active */
			if (MMC3_cmd >=0x08 && MMC3_cmd <=0x1F)
			{	/* Scrambled CHR register */
				index = (MMC3_cmd -8) >>2;
				if (MMC3_cmd &1)
				{	/* LSB nibble */
					DRegBuf[index] &=~0x0F;
					DRegBuf[index] |=V >>1 &0x0F;
				}
				else
				{	/* MSB nibble */
					DRegBuf[index] &=~0xF0;
					DRegBuf[index] |=V <<4 &0xF0;
				}
				FixMMC3CHR(MMC3_cmd);
			}
			else
			if (MMC3_cmd >=0x25 && MMC3_cmd <=0x26)
			{	/* Scrambled PRG register */
				DRegBuf[6 | MMC3_cmd &1] =V >>5 &1 | V >>3 &2 | V >>1 &4 | V <<1 &8;
				FixMMC3PRG(MMC3_cmd);
			}
		}
	}
	else
	{	/* Register index */
		MMC3_CMDWrite(A, V);
		if (A &2) EXPREGS[0] =V;
	}
}

static void UNLA9746Power(void) {
	GenMMC3Power();
	SetWriteHandler(0x5000, 0x5FFF, UNLA9746WriteOuter);
	SetWriteHandler(0x8000, 0xBFFF, UNLA9746WriteASIC);
	EXPREGS[0] = 0;
	EXPREGS[1] = 3;
	MMC3RegReset();
}

static void UNLA9746Reset(void) {
	EXPREGS[0] = 0;
	EXPREGS[1] = 3;
	MMC3RegReset();
}

void UNLA9746_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 128, 0, 0);
	pwrap = UNLA9746PWrap;
	cwrap = UNLA9746CWrap;
	info->Power = UNLA9746Power;
	info->Reset = UNLA9746Reset;
	AddExState(EXPREGS, 2, 0, "EXPR");
}

#endif
