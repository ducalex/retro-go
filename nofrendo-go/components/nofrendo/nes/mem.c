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
IRAM_ATTR void mem_setpage(uint32 page, uint8 *ptr)
{
   ASSERT(page < 32);
   ASSERT(ptr);

   mem.pages[page] = ptr - (page * MEM_PAGESIZE);

   if (!MEM_PAGE_HAS_HANDLERS(mem.pages_read[page]))
   {
      mem.pages_read[page] = mem.pages[page];
   }

   if (!MEM_PAGE_HAS_HANDLERS(mem.pages_write[page]))
   {
      mem.pages_write[page] = mem.pages[page];
   }
}

/* Get 2KB memory page */
uint8 *mem_getpage(uint32 page)
{
   ASSERT(page < 32);

   uint8 *page_ptr = mem.pages[page];

   if (MEM_PAGE_IS_VALID_PTR(page_ptr))
   {
      return page_ptr + (page * MEM_PAGESIZE);
   }

   return page_ptr;
}

/* read a byte of 6502 memory space */
IRAM_ATTR uint8 mem_getbyte(uint32 address)
{
   uint8 *page = mem.pages_read[address >> MEM_PAGESHIFT];

   /* Special memory handlers */
   if (MEM_PAGE_HAS_HANDLERS(page))
   {
      for (mem_read_handler_t *mr = mem.read_handlers; mr->read_func != NULL; mr++)
      {
         if (address >= mr->min_range && address <= mr->max_range)
            return mr->read_func(address);
      }
      page = mem.pages[address >> MEM_PAGESHIFT];
   }

   /* Unmapped region */
   if (page == NULL)
   {
      MESSAGE_DEBUG("Read unmapped region: $%4X\n", address);
      return 0xFF;
   }
   /* Paged memory */
   else
   {
      return page[address];
   }
}

/* write a byte of data to 6502 memory space */
IRAM_ATTR void mem_putbyte(uint32 address, uint8 value)
{
   uint8 *page = mem.pages_write[address >> MEM_PAGESHIFT];

   /* Special memory handlers */
   if (MEM_PAGE_HAS_HANDLERS(page))
   {
      for (mem_write_handler_t *mw = mem.write_handlers; mw->write_func != NULL; mw++)
      {
         if (address >= mw->min_range && address <= mw->max_range)
         {
            mw->write_func(address, value);
            return;
         }
      }
      page = mem.pages[address >> MEM_PAGESHIFT];
   }

   /* Unmapped region */
   if (page == NULL)
   {
      MESSAGE_DEBUG("Write to unmapped region: $%2X to $%4X\n", address, value);
   }
   /* Paged memory */
   else
   {
      page[address] = value;
   }
}

IRAM_ATTR uint32 mem_getword(uint32 address)
{
   return mem_getbyte(address + 1) << 8 | mem_getbyte(address);
}

void mem_reset(void)
{
   memset(&mem, 0, sizeof(mem));

   mem_setpage(0, mem.ram);
   mem_setpage(1, mem.ram); // $800 - $FFF mirror
   mem_setpage(2, mem.ram); // $1000 - $07FF mirror
   mem_setpage(3, mem.ram); // $1800 - $1FFF mirror

   int num_read_handlers = 0, num_write_handlers = 0;
   mapper_t *mapper = nes_getptr()->mapper;

   // NES cartridge handlers
   if (mapper)
   {
      for (int i = 0, rc = 0, wc = 0; i < 4; i++)
      {
         if (mapper->mem_read[rc].read_func)
            mem.read_handlers[num_read_handlers++] = mapper->mem_read[rc++];

         if (mapper->mem_write[wc].write_func)
            mem.write_handlers[num_write_handlers++] = mapper->mem_write[wc++];
      }
   }

   // NES hardware handlers
   for (int i = 0, rc = 0, wc = 0; i < MEM_HANDLERS_MAX; i++)
   {
      if (read_handlers[rc].read_func)
         mem.read_handlers[num_read_handlers++] = read_handlers[rc++];

      if (write_handlers[wc].write_func)
         mem.write_handlers[num_write_handlers++] = write_handlers[wc++];
   }

   // Mark pages if they contain handlers (used for fast access in nes6502)
   for (mem_read_handler_t *mr = mem.read_handlers; mr->read_func != NULL; mr++)
   {
      for (int i = mr->min_range; i < mr->max_range; i++)
         mem.pages_read[i >> MEM_PAGESHIFT] = MEM_PAGE_USE_HANDLERS;
   }

   for (mem_write_handler_t *mw = mem.write_handlers; mw->write_func != NULL; mw++)
   {
      for (int i = mw->min_range; i < mw->max_range; i++)
         mem.pages_write[i >> MEM_PAGESHIFT] = MEM_PAGE_USE_HANDLERS;
   }

   ASSERT(num_read_handlers <= MEM_HANDLERS_MAX);
   ASSERT(num_write_handlers <= MEM_HANDLERS_MAX);
}

mem_t *mem_init(void)
{
   // memset(&mem, 0, sizeof(mem));
   mem_reset();
   return &mem;
}

void mem_shutdown(void)
{
   //
}
