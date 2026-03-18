#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2022
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

static uint8 regs[4]          = { 0 };
static uint8 dipswitch        = 0;

static SFORMAT StateRegs[] = {
   { regs,       4, "EXPR" },
   { &dipswitch, 1, "DPSW" },
   { 0 }
};

static void Mapper463_Sync(void)
{
   if (regs[0] &4)
   {
      setprg16(0x8000, regs[1]);
      setprg16(0xC000, regs[1]);
   }
   else
   {
      setprg32(0x8000, regs[1] >>1);
   }
   setchr8(regs[2]);
   setmirror(regs[0] &1? MI_H: MI_V);
}

static DECLFW(Mapper463_Write5000)
{
   if (A &(0x10 << dipswitch))
   {
      regs[A &3] =V;
      Mapper463_Sync();
   }
}

static void Mapper463_Reset(void)
{
   dipswitch =(dipswitch +1) &7;
   regs[0] =regs[1] =regs[2] =regs[3] =0;
   Mapper463_Sync();
}

static void Mapper463_Power(void)
{
   dipswitch =0;
   regs[0] =regs[1] =regs[2] =regs[3] =0;
   Mapper463_Sync();
   SetWriteHandler(0x5000, 0x5FFF, Mapper463_Write5000);
   SetReadHandler(0x6000, 0xFFFF, CartBR);
}

void Mapper463_Init(CartInfo *info)
{
   info->Power       = Mapper463_Power;
   info->Reset       = Mapper463_Reset;
   AddExState(StateRegs, ~0, 0, 0);
}
#endif
