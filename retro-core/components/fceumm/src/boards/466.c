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

static uint8 regs[3];

static SFORMAT StateRegs[] =
{
   { regs, 3, "EXPR" },
   { 0 }
};

static DECLFR(Mapper466_ReadOB)
{
   return X.DB;
}

static void Mapper466_Sync(void)
{
   int prg =regs[1] <<5 | regs[0] <<1 &0x1E | regs[0] >>5 &1;
   
   /* Return open bus when selecting unpopulated PRG chip */
   if (prg &0x20 && PRGsize[0] <1024*1024)
	   SetReadHandler(0x8000, 0xFFFF, Mapper466_ReadOB);
   else
	   SetReadHandler(0x8000, 0xFFFF, CartBR);
   
   if (regs[0] &0x40)
   {
      if (regs[0] &0x10)
      {
         setprg16(0x8000, prg);
         setprg16(0xC000, prg);
      }
      else
      {
         setprg32(0x8000, prg >>1);
      }
   }
   else
   {
      setprg16(0x8000, prg &~7 | regs[2] &7);
      setprg16(0xC000, prg &~7 |          7);
   }
   setprg8r(0x10, 0x6000, 0);
   setchr8(0);
   setmirror(regs[0] &0x80? MI_H: MI_V);
}

static DECLFW(Mapper466_Write5000)
{
   regs[A >>11 &1] =A &0xFF;
   Mapper466_Sync();
}

static DECLFW(Mapper466_WriteLatch)
{
   regs[2] =V;
   Mapper466_Sync();
}

static void Mapper466_Reset(void)
{
   regs[0] =regs[1] =0;
   Mapper466_Sync();
}

static void Mapper466_Power(void)
{
   regs[0] =regs[1] =0;
   Mapper466_Sync();
   SetWriteHandler(0x5000, 0x5FFF, Mapper466_Write5000);
   SetWriteHandler(0x8000, 0xFFFF, Mapper466_WriteLatch);
   SetReadHandler(0x6000, 0xFFFF, CartBR);
}

void Mapper466_Init(CartInfo *info)
{
   info->Power       = Mapper466_Power;
   info->Reset       = Mapper466_Reset;
   AddExState(StateRegs, ~0, 0, 0);
}
#endif
