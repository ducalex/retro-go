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
** map040.c: SMB 2j (hack) mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

// Shouldn't that be packed? (It wasn't packed in SNSS...)
typedef struct
{
   unsigned char irqCounter;
   unsigned char irqCounterEnabled;
} mapper40Data;

#define  MAP40_IRQ_PERIOD  (4096 / NES_CYCLES_PER_SCANLINE)

static struct
{
   bool  enabled;
   uint8 counter;
} irq;


static void map40_init(rom_t *cart)
{
   UNUSED(cart);

   mmc_bankrom(8, 0x6000, 6);
   mmc_bankrom(8, 0x8000, 4);
   mmc_bankrom(8, 0xA000, 5);
   mmc_bankrom(8, 0xE000, 7);

   irq.enabled = false;
   irq.counter = MAP40_IRQ_PERIOD;
}

static void map40_hblank(int scanline)
{
   UNUSED(scanline);

   if (irq.enabled && irq.counter)
   {
      if (0 == --irq.counter)
      {
         nes6502_irq();
         irq.enabled = false;
      }
   }
}

static void map40_write(uint32 address, uint8 value)
{
   int range = (address >> 13) - 4;

   switch (range)
   {
   case 0: /* 0x8000-0x9FFF */
      irq.enabled = false;
      irq.counter = MAP40_IRQ_PERIOD;
      break;

   case 1: /* 0xA000-0xBFFF */
      irq.enabled = true;
      break;

   case 3: /* 0xE000-0xFFFF */
      mmc_bankrom(8, 0xC000, value & 7);
      break;

   default:
      break;
   }
}

static void map40_getstate(void *state)
{
   ((mapper40Data*)state)->irqCounter = irq.counter;
   ((mapper40Data*)state)->irqCounterEnabled = irq.enabled;
}

static void map40_setstate(void *state)
{
   irq.counter = ((mapper40Data*)state)->irqCounter;
   irq.enabled = ((mapper40Data*)state)->irqCounterEnabled;
}

static const mem_write_handler_t map40_memwrite[] =
{
   { 0x8000, 0xFFFF, map40_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map40_intf =
{
   40,               /* mapper number */
   "SMB 2j (hack)",  /* mapper name */
   map40_init,       /* init routine */
   NULL,             /* vblank callback */
   map40_hblank,     /* hblank callback */
   map40_getstate,   /* get state (snss) */
   map40_setstate,   /* set state (snss) */
   NULL,             /* memory read structure */
   map40_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
