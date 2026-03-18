#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2020
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

/* NES 2.0 Mapper 516 - Brilliant Com Cocoma Pack */

#include "mapinc.h"
#include "mmc3.h"

static void M516CW(uint32 A, uint8 V) {
/*    FCEU_printf("CHR: A:%04x V:%02x R0:%02x\n", A, V, EXPREGS[0]); */
	setchr1(A, (V & 0x7F) | ((EXPREGS[0] << 5) & 0x180));
}

static void M516PW(uint32 A, uint8 V) {
/*    FCEU_printf("PRG: A:%04x V:%02x R0:%02x\n", A, V, EXPREGS[0]); */
	setprg8(A, (V & 0x0F) | ((EXPREGS[0] << 4) & 0x30));
}

static DECLFW(M516Write) {
/*    FCEU_printf("Wr: A:%04x V:%02x R0:%02x\n", A, V, EXPREGS[0]); */
	if (A & 0x10) {
		EXPREGS[0] = A & 0xF;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
	MMC3_CMDWrite(A, V);
}

static void M516Power(void) {
	EXPREGS[0] = 0;
	GenMMC3Power();
	SetWriteHandler(0x8000, 0xFFFF, M516Write);
}

void Mapper516_Init(CartInfo *info) {
	GenMMC3_Init(info, 128, 128, 0, 0);
	cwrap = M516CW;
	pwrap = M516PW;
	info->Power = M516Power;
	AddExState(EXPREGS, 4, 0, "EXPR");
}
#endif
