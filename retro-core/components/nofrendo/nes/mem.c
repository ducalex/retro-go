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
** nes/mem.c: Memory emulation
**
*/
#pragma GCC diagnostic ignored "-Warray-bounds="

#include "nes.h"

static mem_t mem;


static void dummy_write(uint32 address, uint8 value)
{
   UNUSED(address);
   UNUSED(value);
}

/* read/write handlers for standard NES */
static const mem_read_handler_t read_handlers[] =
{
   { 0x2000, 0x3FFF, ppu_read   },
   { 0x4000, 0x4015, apu_read   },
   { 0x4016, 0x4017, input_read },
   LAST_MEMORY_HANDLER
};

static const mem_write_handler_t write_handlers[] =
{
   { 0x2000, 0x3FFF, ppu_write   },
   { 0x4014, 0x4014, ppu_write   },
   { 0x4016, 0x4016, input_write },
   { 0x4000, 0x4017, apu_write   },
   { 0x8000, 0xFFFF, dummy_write },
   LAST_MEMORY_HANDLER
};

/* Set 2KB memory page */
void mem_setpage(uint32 page, uint8 *ptr)
{
   if (page >= MEM_PAGECOUNT)
   {
      MESSAGE_ERROR("Invalid CPU page #%d (max: %d) !\n", (int)page, MEM_PAGECOUNT);
      return;
   }

   if (ptr == MEM_PAGE_NOT_MAPPED)
   {
      mem.pages[page] = mem.dummy - (page * MEM_PAGESIZE);
      mem.flags[page] &= ~MEM_PAGE_HAS_MEMORY;
   }
   else
   {
      mem.pages[page] = ptr - (page * MEM_PAGESIZE);
      mem.flags[page] |= MEM_PAGE_HAS_MEMORY;
   }
}

/* Get 2KB memory page */
uint8 *mem_getpage(uint32 page)
{
   if (page >= MEM_PAGECOUNT)
   {
      MESSAGE_ERROR("Invalid CPU page #%d (max: %d) !\n", (int)page, MEM_PAGECOUNT);
      return MEM_PAGE_NOT_MAPPED;
   }

   if (mem.flags[page] & MEM_PAGE_HAS_MEMORY)
   {
      return mem.pages[page] + (page * MEM_PAGESIZE);
   }

   return MEM_PAGE_NOT_MAPPED;
}

/* read a byte of 6502 memory space */
uint8 mem_getbyte(uint32 address)
{
   uint32 flags = mem.flags[address >> MEM_PAGESHIFT];

   if (flags & MEM_PAGE_HAS_READ_HANDLER)
   {
      for (mem_read_handler_t *mr = mem.read_handlers; mr->handler != NULL; mr++)
      {
         if (address >= mr->min_range && address <= mr->max_range)
            return mr->handler(address);
      }
   }

   if (flags & MEM_PAGE_HAS_MEMORY)
   {
      return mem.pages[address >> MEM_PAGESHIFT][address];
   }

   MESSAGE_DEBUG("Read unmapped region: $%4X\n", address);
   return 0xFF;
}

/* write a byte of data to 6502 memory space */
void mem_putbyte(uint32 address, uint8 value)
{
   uint32 flags = mem.flags[address >> MEM_PAGESHIFT];

   if (flags & MEM_PAGE_HAS_WRITE_HANDLER)
   {
      for (mem_write_handler_t *mw = mem.write_handlers; mw->handler != NULL; mw++)
      {
         if (address >= mw->min_range && address <= mw->max_range)
         {
            mw->handler(address, value);
            return;
         }
      }
   }

   if (flags & MEM_PAGE_HAS_MEMORY)
   {
      mem.pages[address >> MEM_PAGESHIFT][address] = value;
      return;
   }

   MESSAGE_DEBUG("Write to unmapped region: $%2X to $%4X\n", address, value);
}

uint32 mem_getword(uint32 address)
{
   return mem_getbyte(address + 1) << 8 | mem_getbyte(address);
}

void mem_reset(void)
{
   memset(mem.ram, 0, MEM_RAMSIZE);

   mem_setpage(0, mem.ram); // $000 - $7FF
   mem_setpage(1, mem.ram); // $800 - $FFF mirror
   mem_setpage(2, mem.ram); // $1000 - $07FF mirror
   mem_setpage(3, mem.ram); // $1800 - $1FFF mirror

   for (size_t i = 4; i < MEM_PAGECOUNT; ++i)
   {
      mem_setpage(i, MEM_PAGE_NOT_MAPPED);
   }

   mem_read_handler_t *mem_r = mem.read_handlers;
   mem_write_handler_t *mem_w = mem.write_handlers;
   mapper_t *mapper = nes_getptr()->mapper;

   // NES cartridge handlers
   if (mapper)
   {
      for (size_t i = 0; i < 4 && mapper->mem_read[i].handler; i++)
         *mem_r++ = mapper->mem_read[i];
      for (size_t i = 0; i < 4 && mapper->mem_write[i].handler; i++)
         *mem_w++ = mapper->mem_write[i];
   }

   // NES hardware handlers
   for (size_t i = 0; i < MEM_HANDLERS_MAX - 5 && read_handlers[i].handler; i++)
      *mem_r++ = read_handlers[i];
   for (size_t i = 0; i < MEM_HANDLERS_MAX - 5 && write_handlers[i].handler; i++)
      *mem_w++ = write_handlers[i];

   mem_r->handler = NULL;
   mem_w->handler = NULL;

   // Mark pages if they contain handlers (used for fast access in nes6502)
   for (mem_read_handler_t *mr = mem.read_handlers; mr < mem_r; mr++)
   {
      for (int i = mr->min_range; i < mr->max_range; i++)
         mem.flags[i >> MEM_PAGESHIFT] |= MEM_PAGE_HAS_READ_HANDLER;
   }
   for (mem_write_handler_t *mw = mem.write_handlers; mw < mem_w; mw++)
   {
      for (int i = mw->min_range; i < mw->max_range; i++)
         mem.flags[i >> MEM_PAGESHIFT] |= MEM_PAGE_HAS_WRITE_HANDLER;
   }
}

mem_t *mem_init_(void)
{
   // Don't care if the alloc fails, dummy is just a safety net
   mem.dummy = malloc(MEM_PAGESIZE);
   mem_reset();
   return &mem;
}

void mem_shutdown(void)
{
   free(mem.dummy);
   mem.dummy = NULL;
}
