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
** map016.c: Bandai mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
   int counter;
   bool enabled;
} irq;

// Shouldn't that be packed? (It wasn't packed in SNSS...)
typedef struct
{
   unsigned char irqCounterLowByte;
   unsigned char irqCounterHighByte;
   unsigned char irqCounterEnabled;
} mapper16Data;


static void map16_init(rom_t *cart)
{
   UNUSED(cart);

   mmc_bankrom(16, 0x8000, 0);
   mmc_bankrom(16, 0xC000, MMC_LASTBANK);
   irq.counter = 0;
   irq.enabled = false;
}

static void map16_write(uint32 address, uint8 value)
{
   int reg = address & 0xF;

   if (reg < 8)
   {
      mmc_bankvrom(1, reg << 10, value);
   }
   else
   {
      switch (address & 0x000F)
      {
      case 0x8:
         mmc_bankrom(16, 0x8000, value);
         break;

      case 0x9:
         switch (value & 3)
         {
         case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
         case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
         case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
         case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
         }
         break;

      case 0xA:
         irq.enabled = (value & 1);
         break;

      case 0xB:
         irq.counter = (irq.counter & 0xFF00) | value;
         break;

      case 0xC:
         irq.counter = (value << 8) | (irq.counter & 0xFF);
         break;

      case 0xD:
         /* eeprom I/O port? */
         break;
      }
   }
}

static void map16_hblank(int scanline)
{
   UNUSED(scanline);

   if (irq.enabled && irq.counter)
   {
      if (0 == --irq.counter)
         nes6502_irq();
   }
}

static void map16_getstate(void *state)
{
   ((mapper16Data*)state)->irqCounterLowByte = irq.counter & 0xFF;
   ((mapper16Data*)state)->irqCounterHighByte = irq.counter >> 8;
   ((mapper16Data*)state)->irqCounterEnabled = irq.enabled;
}

static void map16_setstate(void *state)
{
   irq.counter = (((mapper16Data*)state)->irqCounterHighByte << 8)
                       | ((mapper16Data*)state)->irqCounterLowByte;
   irq.enabled = ((mapper16Data*)state)->irqCounterEnabled;
}

static const mem_write_handler_t map16_memwrite[] =
{
   { 0x6000, 0x600D, map16_write },
   { 0x7FF0, 0x7FFD, map16_write },
   { 0x8000, 0x800D, map16_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map16_intf =
{
   16,               /* mapper number */
   "Bandai",         /* mapper name */
   map16_init,       /* init routine */
   NULL,             /* vblank callback */
   map16_hblank,     /* hblank callback */
   map16_getstate,   /* get state (snss) */
   map16_setstate,   /* set state (snss) */
   NULL,             /* memory read structure */
   map16_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
