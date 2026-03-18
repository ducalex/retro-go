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

static uint8 game = 0;

static void Sync(void)
{
    setchr8(game);
	setprg16(0x8000, game);
	setprg16(0xC000, game);
}

static void M60Reset(void)
{
    game = (game + 1) & 3;
    Sync();
}

static void M60Power(void)
{
    game = 0;
    Sync();
    SetReadHandler(0x8000, 0xFFFF, CartBR);
    SetWriteHandler(0x8000, 0xFFFF, CartBW);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper60_Init(CartInfo *info)
{
    info->Power = M60Power;
    info->Reset = M60Reset;
    GameStateRestore = StateRestore;
	AddExState(&game, 1, 0, "GAME");
}
#endif
