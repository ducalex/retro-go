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

/* Mapper 389 - Caltron 9-in-1 multicart */

#include "mapinc.h"

static uint8 regs[3];

static SFORMAT StateRegs[] = {
   { &regs, 3, "REGS" },
   { 0 }
};

static void Sync(void)
{
   if (regs[1] & 0x02)
   {
      /* UNROM-064 */
      setprg16(0x8000, (regs[0] >> 2) | ((regs[2] >> 2) & 0x03));
      setprg16(0xC000, (regs[0] >> 2) | 0x03);
   }
   else
   {
      /* NROM-256 */
      setprg32(0x8000, regs[0] >> 3);
   }
   setchr8(((regs[1] >> 1) & 0x1C) | (regs[2] & 0x03));
   setmirror((regs[0] & 0x01) ^ 1);
}

static DECLFW(M389Write)
{
   switch (A & 0xF000)
   {
   case 0x8000: regs[0] = (A & 0xFF); Sync(); break;
   case 0x9000: regs[1] = (A & 0xFF); Sync(); break;
   default:     regs[2] = (A & 0xFF); Sync(); break;
   }
}

static void M389Reset(void)
{
   regs[0] = regs[1] = regs[2] = 0;
   Sync();
}

static void M389Power(void)
{
   regs[0] = regs[1] = regs[2] = 0;
   Sync();
   SetReadHandler(0x8000, 0xFFFF, CartBR);
   SetWriteHandler(0x8000, 0xFFFF, M389Write);
}

static void StateRestore(int version)
{
   Sync();
}

void Mapper389_Init(CartInfo *info)
{
   info->Power = M389Power;
   info->Reset = M389Reset;
   GameStateRestore = StateRestore;
   AddExState(&StateRegs, ~0, 0, 0);
}
#endif
