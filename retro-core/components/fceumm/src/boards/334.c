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
 *
 *
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 dipswitch;

static void M334PW(uint32 A, uint8 V) {
    setprg32(0x8000, EXPREGS[0] >> 1);
}

static DECLFW(M334Write) {
    if (!(A & 1)) {
        EXPREGS[0] = V;
        FixMMC3PRG(MMC3_cmd);
    }
}

static DECLFR(M334Read) {
    if (A & 2)
        return ((X.DB & 0xFE) | (dipswitch & 1));
    return X.DB;
}

static void M334Reset(void) {
    dipswitch++;
	EXPREGS[0] = 0;
	MMC3RegReset();
}

static void M334Power(void) {
    dipswitch = 0;
	EXPREGS[0] = 0;
	GenMMC3Power();
    SetReadHandler(0x6000, 0x7FFF, M334Read);
	SetWriteHandler(0x6000, 0x7FFF, M334Write);
}

void Mapper334_Init(CartInfo *info) {
	GenMMC3_Init(info, 32, 32, 0, 0);
	pwrap = M334PW;
	info->Power = M334Power;
	info->Reset = M334Reset;
	AddExState(EXPREGS, 1, 0, "EXPR");
}
#endif
