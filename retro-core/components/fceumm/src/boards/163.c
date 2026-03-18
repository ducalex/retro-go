#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *  Copyright (C) 2005 CaH4e3
 *  Copyright (C) 2019 Libretro Team
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

/* Nanjing FC-001 PCB */

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM;
static uint32 WRAMSIZE;
static uint8 reg[4];
static SFORMAT StateRegs[] =
{
        { reg, 4, "REGS" },
        { 0 }
};

static void sync()
{
   setprg32(0x8000, (reg[2] <<4) | (reg[0] &0xF) | (reg[3] &0x04? 0x00: 0x03));
   setprg8r(0x10, 0x6000, 0);
   if (~reg[0] &0x80)
      setchr8(0);
}

static void hblank(void) {
   if (reg[0] &0x80 && scanline <239)
   {  /* Actual hardware cannot look at the current scanline number, but instead latches PA09 on PA13 rises. This does not seem possible with the current PPU emulation however. */
      setchr4(0x0000, scanline >=127? 1: 0);
      setchr4(0x1000, scanline >=127? 1: 0);
   }
   else
      setchr8(0);
}

static DECLFR(readReg)
{
   return ~reg[1] &0x04;
}

static DECLFW(writeReg)
{
   uint8 index = A >>8 &3;
   
   /* Swap bits of registers 0-2 again if the "swap bits" bit is set. Exclude register 2 on when PRG-ROM is 1 MiB. */
   if (reg[3] &0x01 && index <= (ROM_size == 64? 1: 2))
      V = (V &~3) | (V >>1 &1) | (V <<1 &2);
   
   if (A &1)
   {
      if (reg[1] &0x01 && ~V &0x01) reg[1] ^=0x04;  /* If A0=1, flip feedback bit on falling edges of D0 */
   }                                                /* If A0=0, write to register */
   else
      reg[index] = V;
   sync();
}

static void power(void)
{
   memset(reg, 0, sizeof(reg));
   sync();
   SetReadHandler (0x5000, 0x57FF, readReg);
   SetWriteHandler(0x5000, 0x57FF, writeReg);
   SetReadHandler (0x6000, 0xFFFF, CartBR);
   SetWriteHandler(0x6000, 0x7FFF, CartBW);
}

static void reset(void)
{
   memset(reg, 0, sizeof(reg));
   sync();
}

static void close(void)
{
   if (WRAM)
      FCEU_gfree(WRAM);
   WRAM = NULL;
}

static void StateRestore(int version)
{
   sync();
}

void Mapper163_Init (CartInfo *info)
{
   info->Power   = power;
   info->Reset   = reset;
   info->Close   = close;
   GameHBIRQHook = hblank;

   GameStateRestore = StateRestore;
   AddExState(StateRegs, ~0, 0, 0);

   WRAMSIZE = info->iNES2? (info->PRGRamSize + info->PRGRamSaveSize): 8192;
#ifndef FCEU_NO_MALLOC
   WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
   WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
   SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
   AddExState(WRAM, WRAMSIZE, 0, "WRAM");
   FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);

   if (info->battery) {
      info->SaveGame[0] = WRAM;
      info->SaveGameLen[0] = WRAMSIZE;
   }
}
#endif
