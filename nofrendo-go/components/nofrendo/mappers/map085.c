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
** map085.c: Konami VRC7 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
   int counter, latch;
   int wait_state;
   bool enabled;
} irq;


static void map85_write(uint32 address, uint8 value)
{
   uint8 bank = address >> 12;
   uint8 reg = (address & 0x10) | ((address & 0x08) << 1);

   switch (bank)
   {
   case 0x08:
      if (0x10 == reg)
         mmc_bankrom(8, 0xA000, value);
      else
         mmc_bankrom(8, 0x8000, value);
      break;

   case 0x09:
      /* 0x10 & 0x30 should be trapped by sound emulation */
      mmc_bankrom(8, 0xC000, value);
      break;

   case 0x0A:
      if (0x10 == reg)
         mmc_bankvrom(1, 0x0400, value);
      else
         mmc_bankvrom(1, 0x0000, value);
      break;

   case 0x0B:
      if (0x10 == reg)
         mmc_bankvrom(1, 0x0C00, value);
      else
         mmc_bankvrom(1, 0x0800, value);
      break;

   case 0x0C:
      if (0x10 == reg)
         mmc_bankvrom(1, 0x1400, value);
      else
         mmc_bankvrom(1, 0x1000, value);
      break;

   case 0x0D:
      if (0x10 == reg)
         mmc_bankvrom(1, 0x1C00, value);
      else
         mmc_bankvrom(1, 0x1800, value);
      break;

   case 0x0E:
      if (0x10 == reg)
      {
         irq.latch = value;
      }
      else
      {
         switch (value & 3)
         {
         case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
         case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
         case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
         case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
         }
      }
      break;

   case 0x0F:
      if (0x10 == reg)
      {
         irq.enabled = irq.wait_state;
      }
      else
      {
         irq.wait_state = value & 0x01;
         irq.enabled = (value & 0x02) ? true : false;
         if (true == irq.enabled)
            irq.counter = irq.latch;
      }
      break;

   default:
      MESSAGE_DEBUG("unhandled vrc7 write: $%02X to $%04X\n", value, address);
      break;
   }
}

static void map85_hblank(int scanline)
{
   UNUSED(scanline);

   if (irq.enabled)
   {
      if (++irq.counter > 0xFF)
      {
         irq.counter = irq.latch;
         nes6502_irq();

         //return;
      }
      //irq.counter++;
   }
}

static const mem_write_handler_t map85_memwrite[] =
{
   { 0x8000, 0xFFFF, map85_write },
   LAST_MEMORY_HANDLER
};

static void map85_init(rom_t *cart)
{
   UNUSED(cart);

   mmc_bankrom(16, 0x8000, 0);
   mmc_bankrom(16, 0xC000, MMC_LASTBANK);

   mmc_bankvrom(8, 0x0000, 0);

   irq.counter = irq.latch = 0;
   irq.wait_state = 0;
   irq.enabled = false;
}

mapintf_t map85_intf =
{
   85,               /* mapper number */
   "Konami VRC7",    /* mapper name */
   map85_init,       /* init routine */
   NULL,             /* vblank callback */
   map85_hblank,     /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map85_memwrite,   /* memory write structure */
   NULL
};
