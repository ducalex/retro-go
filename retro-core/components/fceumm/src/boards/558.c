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

/* Yancheng YC-03-09/Waixing FS??? PCB */

#include "mapinc.h"
#include "eeprom_93C66.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM;
static uint32 WRAMSIZE;
static uint8 reg[4];
static uint8 haveEEPROM;
static uint8 eeprom_data[512];
static SFORMAT StateRegs[] =
{
        { reg, 4, "REGS" },
        { 0 }
};

static void sync()
{
   setprg32(0x8000, reg[1] <<4 | reg[0] &0xF | (reg[3] &0x04? 0x00: 0x03));
   setprg8r(0x10, 0x6000, 0);
   if (~reg[0] &0x80)
      setchr8(0);

   if (haveEEPROM)
      eeprom_93C66_write(reg[2] &0x04, reg[2] &0x02, reg[2] &0x01);
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
   if (haveEEPROM)
      return eeprom_93C66_read()? 0x04: 0x00;
   else
      return reg[2] &0x04;
}

static DECLFW(writeReg)
{
   uint8 index = A >>8 &3;
   
   /* D0 and D1 are connected to the ASIC in reverse order, so swap once */
   V = V &~3 | V >>1 &1 | V <<1 &2;
   
   /* Swap bits of registers 0-2 again if the "swap bits" bit is set. Exclude register 2 on when PRG-ROM is 1 MiB. */
   if (reg[3] &0x02 && index <= (ROM_size == 64? 1: 2))
      V = V &~3 | V >>1 &1 | V <<1 &2;
   
   reg[index] = V;
   sync();
}

static void power(void)
{
   memset(reg, 0, sizeof(reg));
   if (haveEEPROM)
      eeprom_93C66_init();
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

void Mapper558_Init (CartInfo *info)
{
   info->Power   = power;
   info->Reset   = reset;
   info->Close   = close;
   GameHBIRQHook = hblank;

   GameStateRestore = StateRestore;
   AddExState(StateRegs, ~0, 0, 0);

   WRAMSIZE = info->PRGRamSize + (info->PRGRamSaveSize &~0x7FF);
#ifndef FCEU_NO_MALLOC
   WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
   WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
   SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
   AddExState(WRAM, WRAMSIZE, 0, "WRAM");
   FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
   haveEEPROM =!!(info->PRGRamSaveSize &0x200);
   if (haveEEPROM)
   {
      eeprom_93C66_storage = eeprom_data;
      info->battery = 1;
      info->SaveGame[0] = eeprom_data;
      info->SaveGameLen[0] = 512;
   }
   else
   if (info->battery)
   {
      info->SaveGame[0] = WRAM;
      info->SaveGameLen[0] = (info->PRGRamSaveSize &~0x7FF);
   }
}
#endif
