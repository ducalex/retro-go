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

/* NES 2.0 Mapper 380 denotes the 970630C circuit board,
 * used on a 512 KiB multicart having 42 to 80,000 listed NROM and UNROM games. */

#include "mapinc.h"

static uint16 latche;
static uint8 dipswitch;
static uint8 isKN35A;
static uint8 is4K1;

static SFORMAT StateRegs[] = {
   { &latche, 2 | FCEUSTATE_RLSB, "LATC" },
   { &dipswitch, 1, "DPSW" },
   { 0 }
};

static void Sync(void)
{
   if (latche & 0x200)
   {
      if (latche & 1) /* NROM 128 */
      {
         setprg16(0x8000, latche >> 2);
         setprg16(0xC000, latche >> 2);
      }
      else /* NROM-256 */
         setprg32(0x8000, latche >> 3);
   }
   else /* UxROM */
   {
      setprg16(0x8000, latche >> 2);
      setprg16(0xC000, (latche >> 2) | 7 | (isKN35A && latche &0x100? 8: 0));
   }
   setmirror(latche &(is4K1? 0x040: 0x002)? MI_H: MI_V);
}

static DECLFR(M380Read)
{
   if (latche & 0x100 && !isKN35A)
      return CartBR(A | dipswitch);
   return CartBR(A);
}

static DECLFW(M380Write)
{
   latche = A;
   Sync();
}

static void M380Reset(void)
{
   dipswitch = (dipswitch + 1) & 0xF;
   latche = 0;
   Sync();
}

static void M380Power(void)
{
   dipswitch = 0;
   latche = 0;
   Sync();
   setchr8(0);
   SetReadHandler(0x8000, 0xFFFF, M380Read);
   SetWriteHandler(0x8000, 0xFFFF, M380Write);
}

static void StateRestore(int version)
{
   Sync();
}

void Mapper380_Init(CartInfo *info)
{
   isKN35A = info->iNES2 && info->submapper == 1;
   is4K1 = info->iNES2 && info->submapper == 2;
   info->Power = M380Power;
   info->Reset = M380Reset;
   GameStateRestore = StateRestore;
   AddExState(&StateRegs, ~0, 0, 0);
}
#endif
