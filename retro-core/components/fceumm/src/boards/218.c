#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 * Copyright (C) 2020
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

static void M218Power(void) {
	setchr8(0);
	setprg32(0x8000, 0);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

void Mapper218_Init(CartInfo* info) {
	if (head.ROM_type & 0x08)
		SetupCartMirroring(MI_0 + (head.ROM_type & 0x01), 1,  NULL);
	SetupCartCHRMapping(0, NTARAM, 2048, 1);
	info->Power = M218Power;
}
#endif
