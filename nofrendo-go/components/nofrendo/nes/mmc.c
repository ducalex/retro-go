/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes/mmc.c: Mapper emulation
**
*/

#include <mappers.h>
#include "nes.h"

static mapper_t mapper;
static rom_t *cart;

/* PRG-ROM/RAM bankswitching */
void mmc_bankprg(unsigned size, unsigned address, int bank, uint8 *base)
{
   size_t banks = 16;

   // if (base == PRG_ANY)
   //    base = cart->prg_rom_banks ? PRG_ROM : PRG_RAM;

   if (base == PRG_ROM)
   {
      base = cart->prg_rom;
      banks = cart->prg_rom_banks;
   }
   else if (base == PRG_RAM)
   {
      base = cart->prg_ram;
      banks = cart->prg_ram_banks;
   }

   if (base == NULL)
   {
      MESSAGE_ERROR("MMC: Invalid pointer! Addr: $%04X Bank: %d Size: %d\n", address, bank, size);
      abort();
   }

   switch (size)
   {
   case 8:
      base += ((bank >= 0 ? bank : (banks) + bank) % (banks)) << 13;
      break;

   case 16:
      base += ((bank >= 0 ? bank : (banks / 2) + bank) % (banks / 2)) << 14;
      break;

   case 32:
      base += ((bank >= 0 ? bank : (banks / 4) + bank) % (banks / 4)) << 15;
      break;

   default:
      MESSAGE_ERROR("MMC: Invalid bank size! Addr: $%04X Bank: %d Size: %d\n", address, bank, size);
      abort();
   }

   for (int i = 0; i < (size * 0x400 / MEM_PAGESIZE); i++)
   {
      mem_setpage((address >> MEM_PAGESHIFT) + i, base + i * MEM_PAGESIZE);
   }
}

/* CHR-ROM/RAM bankswitching */
void mmc_bankchr(unsigned size, unsigned address, int bank, uint8 *base)
{
   size_t banks = 128;

   if (base == CHR_ANY)
      base = cart->chr_rom_banks ? CHR_ROM : CHR_RAM;

   if (base == CHR_ROM)
   {
      base = cart->chr_rom;
      banks = cart->chr_rom_banks;
   }
   else if (base == CHR_RAM)
   {
      base = cart->chr_ram;
      banks = cart->chr_ram_banks;
   }

   switch (size)
   {
   case 1:
      bank = (bank >= 0 ? bank : (banks * 8) + bank) % (banks * 8);
      ppu_setpage(1, address >> 10, &base[bank << 10] - address);
      break;

   case 2:
      bank = (bank >= 0 ? bank : (banks * 4) + bank) % (banks * 4);
      ppu_setpage(2, address >> 10, &base[bank << 11] - address);
      break;

   case 4:
      bank = (bank >= 0 ? bank : (banks * 2) + bank) % (banks * 2);
      ppu_setpage(4, address >> 10, &base[bank << 12] - address);
      break;

   case 8:
      bank = (bank >= 0 ? bank : (banks) + bank) % (banks);
      ppu_setpage(8, 0, &base[bank << 13]);
      break;

   default:
      MESSAGE_ERROR("MMC: Invalid CHR bank size %d\n", size);
      abort();
   }
}

/* Mapper initialization routine */
void mmc_reset(void)
{
   /* Switch PRG-RAM into CPU space */
   if (cart->prg_ram_banks > 0)
      mmc_bankprg(8, 0x6000, 0, PRG_RAM);

   /* Switch PRG-ROM into CPU space */
   if (cart->prg_rom_banks > 1)
   {
      mmc_bankprg(16, 0x8000,  0, PRG_ROM);
      mmc_bankprg(16, 0xC000, -1, PRG_ROM);
   }
   else if (cart->prg_rom_banks == 1)
   {
      mmc_bankprg(8, 0xE000,  0, PRG_ROM);
   }

   /* Switch CHR-ROM/RAM into PPU space */
   mmc_bankchr(8, 0x0000, 0, CHR_ANY);

   /* Setup name table mirroring */
   ppu_setmirroring(cart->mirroring);

   /* Clear special callbacks */
   ppu_setlatchfunc(NULL);
   ppu_setvreadfunc(NULL);
   nes_settimer(NULL, 0);

   /* The mapper's init will undo all we've just done, oh well :) */
   if (mapper.init)
      mapper.init(cart);
}

void mmc_shutdown()
{
   //
}

mapper_t *mmc_init(rom_t *_cart)
{
   int mapper_number = _cart->mapper_number;

   mapper.number = -1;

   for (int i = 0; mappers[i]; i++)
   {
      if (mappers[i]->number == mapper_number)
      {
         mapper = *mappers[i]; // Copy
         cart = _cart;

         MESSAGE_INFO("MMC: Mapper %s (iNES %03d)\n", mapper.name, mapper.number);
         MESSAGE_INFO("MMC: PRG-ROM: %d banks\n", cart->prg_rom_banks);
         MESSAGE_INFO("MMC: PRG-RAM: %d banks\n", cart->prg_ram_banks);
         if (cart->chr_rom_banks > 0)
            MESSAGE_INFO("MMC: CHR-ROM: %d banks\n", cart->chr_rom_banks);
         else
            MESSAGE_INFO("MMC: CHR-RAM: %d banks\n", cart->chr_ram_banks);

         return &mapper;
      }
   }

   MESSAGE_ERROR("MMC: Unsupported mapper %03d\n", mapper_number);
   return NULL;
}
