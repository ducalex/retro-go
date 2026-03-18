#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301	USA
 */
 
/*	晶太 YY840708C PCB
	Solely used for the "1995 Soccer 6-in-1 足球小将專輯 (JY-014)" multicart.
	MMC3+PAL16L8 combination, resulting in a bizarre mapper that switches banks in part upon *reads*.
*/ 

#include "mapinc.h"
#include "mmc3.h"

#define A15    EXPREGS[0]
#define A16    EXPREGS[1]
#define A17A18 EXPREGS[2]

static void M383PRGWrap (uint32 A, uint8 V)
{
	switch(A17A18)
	{
		case 0x00:
			/* "Setting 0 provides a round-about means of dividing the first 128 KiB bank into two 32 KiB and one 64 KiB bank." */
			setprg8(A, V &(A16? 0x07: 0x03) | (A16? 0x00: A15) | A16 | A17A18);
			break;
		case 0x30:
			/* "Setting 3 provides 128 KiB MMC3 banking with the CPU A14 line fed to the MMC3 clone reversed.
			   This is used for the game Tecmo Cup: Soccer Game (renamed "Tecmo Cup Soccer"),
			   originally an MMC1 game with the fixed bank at $8000-$BFFF and the switchable bank at $C000-$FFFF,
			   a configuration that could not be reproduced with an MMC3 alone." */
			setprg8(A ^0x4000, V &0x0F | A17A18);
			
			/* "It is also used for the menu,
			   which in part executes from PRG-ROM mapped to the CPU $6000-$7FFF address range on the MMC3 clone's fixed banks alone,
			   as no MMC3 PRG bank register is written to before JMPing to this address range." */
			if (A ==0xE000) setprg8(A ^0x8000, V &0x0B | A17A18);
			break;
		default:
			/* "Settings 1 and 2 provide normal 128 KiB MMC3 banking." */
			setprg8(A, V &0x0F | A17A18);
			break;
	}
}

static void M383CHRWrap (uint32 A, uint8 V)
{
	setchr1(A, V &0x7F | A17A18 <<3);
}

static DECLFR(M383Read)
{
	if (A17A18 ==0x00)
	{	/* "PAL PRG A16 is updated with the content of the corresponding MMC3 PRG bank bit by reading from the respective address range,
		   which in turn will then be applied across the entire ROM address range." */
		A16 =DRegBuf[0x06 | A >>13 &0x01] &0x08;
		FixMMC3PRG(MMC3_cmd);
	}
	return CartBR(A);
}

static DECLFW(M383Write)
{
	if (A &0x0100)
	{
		A15    =A >>11 &0x04;
		A17A18 =A &0x30;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);		
	}
	if (A &0x4000)
		MMC3_IRQWrite(A, V);
	else
		MMC3_CMDWrite(A, V);
}

static void M383Reset (void) {
	EXPREGS[0] = 0;
	EXPREGS[1] = 0;
	EXPREGS[2] = 0;
	MMC3RegReset();
}

static void M383Power (void) {
	GenMMC3Power();
	SetReadHandler(0x8000, 0xBFFF, M383Read);
	SetWriteHandler(0x8000, 0xFFFF, M383Write);
}

void Mapper383_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 128, 8, 0);
	pwrap = M383PRGWrap;
	cwrap = M383CHRWrap;
	info->Power = M383Power;
	info->Reset = M383Reset;
	AddExState(EXPREGS, 3, 0, "EXPR");
}
#endif
