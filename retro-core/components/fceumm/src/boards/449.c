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

static uint16 latchAddr;
static uint8  latchData;
static uint8  dipswitch;
static uint8  dipselect;

static SFORMAT StateRegs[] =
{
   { &latchAddr, 2, "ADDR" },
   { &latchData, 1, "DATA" },
   { &latchData, 1, "DIPS" },
   { &dipselect, 1, "DSEL" },
   { 0 }
};

static void Mapper449_Sync(void)
{
   int prg =latchAddr >>2 &0x1F | latchAddr >>3 &0x20;
   if (~latchAddr &0x080)
   {
      setprg16(0x8000, prg);
      setprg16(0xC000, prg |7);
   }
   else
   {
      if (latchAddr &0x001)
      {
         setprg32(0x8000, prg >>1);
      }
      else
      {
         setprg16(0x8000, prg);
         setprg16(0xC000, prg);
      }
   }
   setchr8(latchData);
   setmirror(latchAddr &0x002? MI_H: MI_V);
}

static DECLFR(Mapper449_Read)
{
   if (dipselect)
      return dipswitch &0x3;
   else
   if (latchAddr &0x200)
      return CartBR(A | dipswitch &0xF);
   else
      return CartBR(A);
}

static DECLFW(Mapper449_WriteDIPSelect)
{
   dipselect =V &1;
}

static DECLFW(Mapper449_WriteLatch)
{
   latchData =V;
   latchAddr =A &0xFFFF;
   Mapper449_Sync();
}

static void Mapper449_Reset(void)
{
   dipswitch++;
   latchAddr =latchData =0;
   Mapper449_Sync();
}

static void Mapper449_Power(void)
{
   dipselect =dipswitch =latchAddr =latchData =0;
   Mapper449_Sync();
   SetWriteHandler(0x6000, 0x7FFF, Mapper449_WriteDIPSelect);
   SetWriteHandler(0x8000, 0xFFFF, Mapper449_WriteLatch);
   SetReadHandler(0x6000, 0x7FFF, CartBR);
   SetReadHandler(0x8000, 0xFFFF, Mapper449_Read);
}

void Mapper449_Init(CartInfo *info)
{
   info->Power       = Mapper449_Power;
   info->Reset       = Mapper449_Reset;
   AddExState(StateRegs, ~0, 0, 0);
}
#endif
