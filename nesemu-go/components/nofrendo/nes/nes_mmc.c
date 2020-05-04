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

#include <string.h>
#include <noftypes.h>
#include <nes6502.h>
#include <libsnss.h>
#include <log.h>
#include "nes_ppu.h"
#include "nes_mmc.h"
#include "nes_rom.h"
#include "mmc_list.h"

#define  MMC_8KPRG         (mmc.prg_banks * 2)
#define  MMC_16KPRG        (mmc.prg_banks)
#define  MMC_32KPRG        (mmc.prg_banks / 2)
#define  MMC_8KCHR         (mmc.chr_banks)
#define  MMC_4KCHR         (mmc.chr_banks * 2)
#define  MMC_2KCHR         (mmc.chr_banks * 4)
#define  MMC_1KCHR         (mmc.chr_banks * 8)

#define  MMC_LAST8KPRG     (MMC_8KPRG - 1)
#define  MMC_LAST16KPRG    (MMC_16KPRG - 1)
#define  MMC_LAST32KPRG    (MMC_32KPRG - 1)
#define  MMC_LAST8KCHR     (MMC_8KCHR - 1)
#define  MMC_LAST4KCHR     (MMC_4KCHR - 1)
#define  MMC_LAST2KCHR     (MMC_2KCHR - 1)
#define  MMC_LAST1KCHR     (MMC_1KCHR - 1)

static mmc_t mmc;

rominfo_t *mmc_getinfo(void)
{
   return mmc.cart;
}

void mmc_setcontext(mmc_t *src_mmc)
{
   ASSERT(src_mmc);

   mmc = *src_mmc;
}

void mmc_getcontext(mmc_t *dest_mmc)
{
   *dest_mmc = mmc;
}

/* VROM bankswitching */
void mmc_bankvrom(int size, uint32 address, int bank)
{
   // printf("mmc_bankvrom: Addr: 0x%x Size: 0x%x Bank:%d\n", address, size, bank);

   switch (size)
   {
   case 1:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST1KCHR;
      ppu_setpage(1, address >> 10, &mmc.chr[(bank % MMC_1KCHR) << 10] - address);
      break;

   case 2:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST2KCHR;
      ppu_setpage(2, address >> 10, &mmc.chr[(bank % MMC_2KCHR) << 11] - address);
      break;

   case 4:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST4KCHR;
      ppu_setpage(4, address >> 10, &mmc.chr[(bank % MMC_4KCHR) << 12] - address);
      break;

   case 8:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST8KCHR;
      ppu_setpage(8, 0, &mmc.chr[(bank % MMC_8KCHR) << 13]);
      break;

   default:
      printf("invalid CHR bank size %d\n", size);
      //abort();
      break;
   }
}

/* ROM bankswitching */
void mmc_bankrom(int size, uint32 address, int bank)
{
   // printf("mmc_bankrom: Addr: 0x%x Size: 0x%x Bank:%d\n", address, size, bank);

   nes6502_context mmc_cpu;

   nes6502_getcontext(&mmc_cpu);

   switch (size)
   {
   case 8:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST8KPRG;
      {
         int page = address >> NES6502_BANKSHIFT;
         mmc_cpu.mem_page[page] = &mmc.prg[(bank % MMC_8KPRG) << 13];
         mmc_cpu.mem_page[page + 1] = mmc_cpu.mem_page[page] + 0x1000;
      }
      break;

   case 16:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST16KPRG;
      {
         int page = address >> NES6502_BANKSHIFT;
         mmc_cpu.mem_page[page] = &mmc.prg[(bank % MMC_16KPRG) << 14];
         mmc_cpu.mem_page[page + 1] = mmc_cpu.mem_page[page] + 0x1000;
         mmc_cpu.mem_page[page + 2] = mmc_cpu.mem_page[page] + 0x2000;
         mmc_cpu.mem_page[page + 3] = mmc_cpu.mem_page[page] + 0x3000;
      }
      break;

   case 32:
      if (bank == MMC_LASTBANK)
         bank = MMC_LAST32KPRG;

      mmc_cpu.mem_page[8] = &mmc.prg[(bank % MMC_32KPRG) << 15];
      mmc_cpu.mem_page[9] = mmc_cpu.mem_page[8] + 0x1000;
      mmc_cpu.mem_page[10] = mmc_cpu.mem_page[8] + 0x2000;
      mmc_cpu.mem_page[11] = mmc_cpu.mem_page[8] + 0x3000;
      mmc_cpu.mem_page[12] = mmc_cpu.mem_page[8] + 0x4000;
      mmc_cpu.mem_page[13] = mmc_cpu.mem_page[8] + 0x5000;
      mmc_cpu.mem_page[14] = mmc_cpu.mem_page[8] + 0x6000;
      mmc_cpu.mem_page[15] = mmc_cpu.mem_page[8] + 0x7000;
      break;

   default:
      printf("invalid PRG bank size %d\n", size);
      //abort();
      break;
   }

   nes6502_setcontext(&mmc_cpu);
}

/* Check to see if this mapper is supported */
bool mmc_peek(int map_num)
{
   mapintf_t **map_ptr = mappers;

   while (NULL != *map_ptr)
   {
      if ((*map_ptr)->number == map_num)
         return true;
      map_ptr++;
   }

   return false;
}

static void mmc_setpages(void)
{
   printf("setting up mapper %d\n", mmc.intf->number);

   /* Switch PRG and CHR into CPU space */
   mmc_bankrom(16, 0x8000, 0);
   mmc_bankrom(16, 0xC000, MMC_LASTBANK);
   mmc_bankvrom(8, 0x0000, 0);

   if (mmc.cart->flags & ROM_FLAG_FOURSCREEN)
   {
      ppu_mirror(0, 1, 2, 3);
   }
   else
   {
      if (MIRROR_VERT == mmc.cart->mirror)
         ppu_mirror(0, 1, 0, 1);
      else
         ppu_mirror(0, 0, 1, 1);
   }

   // ppu_mirrorhipages();
}

/* Mapper initialization routine */
void mmc_reset(void)
{
   mmc_setpages();

   ppu_setlatchfunc(NULL);
   ppu_setvromswitch(NULL);

   if (mmc.intf->init)
      mmc.intf->init();

   log_printf("reset memory mapper\n");
}


void mmc_destroy(mmc_t **nes_mmc)
{
   if (*nes_mmc)
      free(*nes_mmc);
}

mmc_t *mmc_create(rominfo_t *rominfo)
{
   mmc_t *temp;
   mapintf_t **map_ptr;

   for (map_ptr = mappers; (*map_ptr)->number != rominfo->mapper_number; map_ptr++)
   {
      if (NULL == *map_ptr)
         return NULL; /* Should *never* happen */
   }

   temp = calloc(sizeof(mmc_t), 1);
   if (NULL == temp)
      return NULL;

   temp->intf = *map_ptr;
   temp->cart = rominfo;
   temp->prg = rominfo->rom;
   temp->prg_banks = rominfo->rom_banks;

   if (rominfo->vrom_banks)
   {
      temp->chr = rominfo->vrom;
      temp->chr_banks = rominfo->vrom_banks;
   }
   else
   {
      temp->chr = rominfo->vram;
      temp->chr_banks = rominfo->vram_banks;
   }

   mmc_setcontext(temp);

   log_printf("created memory mapper: %s\n", (*map_ptr)->name);

   return temp;
}
