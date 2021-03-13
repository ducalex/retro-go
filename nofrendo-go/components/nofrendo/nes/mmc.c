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
** nes_mmc.c
**
** NES Memory Management Controller (mapper) emulation
** $Id: nes_mmc.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <nofrendo.h>
#include <mappers.h>
#include <string.h>
#include "ppu.h"
#include "mmc.h"
#include "rom.h"

#define  MMC_8KPRG         (prg_banks * 2)
#define  MMC_16KPRG        (prg_banks)
#define  MMC_32KPRG        (prg_banks / 2)
#define  MMC_8KCHR         (chr_banks)
#define  MMC_4KCHR         (chr_banks * 2)
#define  MMC_2KCHR         (chr_banks * 4)
#define  MMC_1KCHR         (chr_banks * 8)

static map_t mapper;
static rom_t *cart;
static int prg_banks;
static int chr_banks;

/* Map a pointer into the address space */
void mmc_bankptr(int size, uint32 address, int bank, uint8 *ptr)
{
   int page = address >> MEM_PAGESHIFT;
   uint8 *base = ptr;

   if (ptr == NULL)
   {
      MESSAGE_ERROR("MMC: Invalid pointer! Addr: $%04X Bank: %d Size: %d\n", address, bank, size);
      abort();
   }

   switch (size)
   {
   case 8:
      base += ((bank >= 0 ? bank : MMC_8KPRG + bank) % MMC_8KPRG) << 13;
      break;

   case 16:
      base += ((bank >= 0 ? bank : MMC_16KPRG + bank) % MMC_16KPRG) << 14;
      break;

   case 32:
      base += ((bank >= 0 ? bank : MMC_32KPRG + bank) % MMC_32KPRG) << 15;
      break;

   default:
      MESSAGE_ERROR("MMC: Invalid bank size! Addr: $%04X Bank: %d Size: %d\n", address, bank, size);
      abort();
   }

   for (int i = 0; i < (size * 0x400 / MEM_PAGESIZE); i++)
   {
      mem_setpage(page + i, base + i * MEM_PAGESIZE);
   }
}

/* PRG-ROM bankswitching */
void mmc_bankrom(int size, uint32 address, int bank)
{
   mmc_bankptr(size, address, bank, cart->prg_rom);
}

/* PRG-RAM bankswitching */
void mmc_bankwram(int size, uint32 address, int bank)
{
   mmc_bankptr(size, address, bank, cart->prg_ram);
}

/* CHR-ROM/RAM bankswitching */
void mmc_bankvrom(int size, uint32 address, int bank)
{
   uint8 *chr = cart->chr_rom ?: cart->chr_ram;

   switch (size)
   {
   case 1:
      bank = (bank >= 0 ? bank : MMC_1KCHR + bank) % MMC_1KCHR;
      ppu_setpage(1, address >> 10, &chr[bank << 10] - address);
      break;

   case 2:
      bank = (bank >= 0 ? bank : MMC_2KCHR + bank) % MMC_2KCHR;
      ppu_setpage(2, address >> 10, &chr[bank << 11] - address);
      break;

   case 4:
      bank = (bank >= 0 ? bank : MMC_4KCHR + bank) % MMC_4KCHR;
      ppu_setpage(4, address >> 10, &chr[bank << 12] - address);
      break;

   case 8:
      bank = (bank >= 0 ? bank : MMC_8KCHR + bank) % MMC_8KCHR;
      ppu_setpage(8, 0, &chr[bank << 13]);
      break;

   default:
      MESSAGE_ERROR("MMC: Invalid CHR bank size %d\n", size);
      abort();
   }
}

static void mmc_setpages(void)
{
   /* Switch Save RAM into CPU space */
   mmc_bankwram(8, 0x6000, 0);

   /* Switch PRG ROM into CPU space */
   mmc_bankrom(16, 0x8000, 0);
   mmc_bankrom(16, 0xC000, MMC_LASTBANK);

   /* Switch CHR ROM/RAM into CPU space */
   mmc_bankvrom(8, 0x0000, 0);

   if (cart->flags & ROM_FLAG_FOURSCREEN)
      ppu_setmirroring(PPU_MIRROR_FOUR);
   else if (cart->flags & ROM_FLAG_VERTICAL)
      ppu_setmirroring(PPU_MIRROR_VERT);
   else
      ppu_setmirroring(PPU_MIRROR_HORI);
}

/* Mapper initialization routine */
void mmc_reset(void)
{
   mmc_setpages();

   ppu_setlatchfunc(NULL);

   if (mapper.init)
      mapper.init(cart);
}

void mmc_shutdown()
{

}

map_t *mmc_init(rom_t *_cart)
{
   int mapper_number = _cart->mapper_number;

   mapper.number = -1;

   for (int i = 0; mappers[i]; i++)
   {
      if (mappers[i]->number == mapper_number)
      {
         mapper = *mappers[i]; // Copy
         cart = _cart;
         chr_banks = cart->chr_rom ? cart->chr_rom_banks : cart->chr_ram_banks;
         prg_banks = cart->prg_rom_banks;

         MESSAGE_INFO("MMC: Mapper %s (iNES %03d)\n", mapper.name, mapper.number);
         MESSAGE_INFO("MMC: PRG-ROM: %d banks\n", cart->prg_rom_banks);
         MESSAGE_INFO("MMC: %s: %d banks\n", cart->chr_rom ? "CHR-ROM" : "CHR-RAM", chr_banks);

         return &mapper;
      }
   }

   MESSAGE_ERROR("MMC: Unsupported mapper %03d\n", mapper_number);
   return NULL;
}
