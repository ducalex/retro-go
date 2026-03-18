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

/* Dongda PEC-9588 educational computer. Provides the same 1bpp all-points-addressable graphics mode.
   Its chipset was later used for the Yancheng cy2000-3 PCB, used on most of the games that display "Union Bond" at the start.
   Some of them also use the 1bpp mode for a few screens!
*/

#include "mapinc.h"
#include "eeprom_93C66.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM;
static uint32 WRAMSIZE;
static uint8 reg[8];
static uint8 eeprom_data[512];
static SFORMAT StateRegs[] =
{
        { reg,           8, "REGS" },
        { eeprom_data, 512, "EEPR" },
        { 0 }
};

static void sync()
{
   uint8 prgLow  = (reg[0] &0x0F) | (reg[0] >>1 &0x10);
   uint8 prgHigh = reg[1] <<5;
   switch ((reg[0] >>5 &2) | (reg[0] >>4 &1))
   {
      case 0: /* UNROM-512 */
         setprg16(0x8000, prgHigh | prgLow);
         setprg16(0xC000, prgHigh |   0x1F);
         break;
      case 1: /* Open Bus on Yancheng cy2000-3 PCB, expansion cartridge on the Dongda PEC-9588 */
         break;
      case 2: /* UNROM-448+64. Very strange mode, used for the LOGO program on the Dongda PEC-9588 */
         setprg16(0x8000, prgHigh | prgLow);
         setprg16(0xC000, prgHigh |(prgLow >=0x1C? 0x1C: 0x1E));
         break;
      case 3: /* UNROM-128 or BNROM */
         if (prgLow &0x10)
         {
            setprg16(0x8000, prgHigh | (prgLow <<1 &0x10) | (prgLow &0x0F));
            setprg16(0xC000, prgHigh | (prgLow <<1 &0x10) |         0x0F);
         }
         else
            setprg32(0x8000, prgHigh >>1 | prgLow);
         break;
   }
   setprg8r(0x10, 0x6000, 0);

   setchr8(0);
   PEC586Hack = !!(reg[0] &0x80);

   setmirror(reg[0] &0x10 && ~reg[3] &0x80? MI_H: MI_V);

   eeprom_93C66_write(reg[2] &0x10, reg[2] &0x04, reg[2] &0x01);
}

static DECLFR(readReg)
{
   return eeprom_93C66_read()? 0x00: 0x04;
}

static DECLFW(writeReg)
{
   reg[A >>8 &7] = V;
   sync();
}

static void power(void)
{
   memset(reg, 0, sizeof(reg));
   eeprom_93C66_init();
   sync();
   SetReadHandler (0x5400, 0x57FF, readReg);
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

void Mapper164_Init (CartInfo *info)
{
   info->Power   = power;
   info->Reset   = reset;
   info->Close   = close;

   GameStateRestore = StateRestore;
   AddExState(StateRegs, ~0, 0, 0);

   WRAMSIZE = info->iNES2? (info->PRGRamSize + (info->PRGRamSaveSize &~0x7FF)): 8192;
#ifndef FCEU_NO_MALLOC
   WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
   WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
   SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
   AddExState(WRAM, WRAMSIZE, 0, "WRAM");
   FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);

   eeprom_93C66_storage = eeprom_data;
   info->battery = 1;
   info->SaveGame[0] = eeprom_data;
   info->SaveGameLen[0] = 512;
}
#endif
