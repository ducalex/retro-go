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
** map021.c: VRC4 mapper interface
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
} mapper21Data;

#define VRC_VBANK(bank, value, high) \
{ \
   if ((high)) \
      highnybbles[(bank)] = (value) & 0x0F; \
   else \
      lownybbles[(bank)] = (value) & 0x0F; \
   mmc_bankvrom(1, (bank) << 10, (highnybbles[(bank)] << 4)+lownybbles[(bank)]); \
}

static struct
{
   bool enabled, wait_state;
   int counter, latch;
} irq;

static bool select_c000 = 0;
static uint8 lownybbles[8];
static uint8 highnybbles[8];


static void vrc4_init(rom_t *cart)
{
   UNUSED(cart);

   irq.enabled = irq.wait_state = 0;
   irq.counter = irq.latch = 0;
}

static void vrc4_write(uint32 address, uint8 value)
{
   switch (address)
   {
   case 0x8000:
      if (select_c000)
         mmc_bankrom(8, 0xC000,value);
      else
         mmc_bankrom(8, 0x8000,value);
      break;

   case 0x9000:
      switch (value & 3)
      {
         case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
         case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
         case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
         case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
      }
      break;
   case 0x9002: select_c000=(value&0x02)>>1; break;
   case 0xA000: mmc_bankrom(8, 0xA000,value); break;

   case 0xB000: VRC_VBANK(0,value,0); break;
   case 0xB002:
   case 0xB040: VRC_VBANK(0,value,1); break;
   case 0xB001:
   case 0xB004:
   case 0xB080: VRC_VBANK(1,value,0); break;
   case 0xB003:
   case 0xB006:
   case 0xB0C0: VRC_VBANK(1,value,1); break;
   case 0xC000: VRC_VBANK(2,value,0); break;
   case 0xC002:
   case 0xC040: VRC_VBANK(2,value,1); break;
   case 0xC001:
   case 0xC004:
   case 0xC080: VRC_VBANK(3,value,0); break;
   case 0xC003:
   case 0xC006:
   case 0xC0C0: VRC_VBANK(3,value,1); break;
   case 0xD000: VRC_VBANK(4,value,0); break;
   case 0xD002:
   case 0xD040: VRC_VBANK(4,value,1); break;
   case 0xD001:
   case 0xD004:
   case 0xD080: VRC_VBANK(5,value,0); break;
   case 0xD003:
   case 0xD006:
   case 0xD0C0: VRC_VBANK(5,value,1); break;
   case 0xE000: VRC_VBANK(6,value,0); break;
   case 0xE002:
   case 0xE040: VRC_VBANK(6,value,1); break;
   case 0xE001:
   case 0xE004:
   case 0xE080: VRC_VBANK(7,value,0); break;
   case 0xE003:
   case 0xE006:
   case 0xE0C0: VRC_VBANK(7,value,1); break;

   case 0xF000:
      irq.latch &= 0xF0;
      irq.latch |= (value & 0x0F);
      break;
   case 0xF002:
   case 0xF040:
      irq.latch &= 0x0F;
      irq.latch |= ((value & 0x0F) << 4);
      break;
   case 0xF004:
   case 0xF001:
   case 0xF080:
      irq.enabled = (value >> 1) & 0x01;
      irq.wait_state = value & 0x01;
      irq.counter = irq.latch;
      break;
   case 0xF006:
   case 0xF003:
   case 0xF0C0:
      irq.enabled = irq.wait_state;
      break;

   default:
      MESSAGE_DEBUG("wrote $%02X to $%04X", value, address);
      break;
   }
}

static void vrc4_hblank(int scanline)
{
   UNUSED(scanline);

   if (irq.enabled)
   {
      if (256 == ++irq.counter)
      {
         irq.counter = irq.latch;
         nes6502_irq();
         //irq.enabled = false;
         irq.enabled = irq.wait_state;
      }
   }
}

static void vrc4_getstate(void *state)
{
   ((mapper21Data*)state)->irqCounter = irq.counter;
   ((mapper21Data*)state)->irqCounterEnabled = irq.enabled;
}

static void vrc4_setstate(void *state)
{
   irq.counter = ((mapper21Data*)state)->irqCounter;
   irq.enabled = ((mapper21Data*)state)->irqCounterEnabled;
}

static const mem_write_handler_t vrc4_memwrite[] =
{
   { 0x8000, 0xFFFF, vrc4_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map21_intf =
{
   21,               /* mapper number */
   "Konami VRC4 A",  /* mapper name */
   vrc4_init,        /* init routine */
   NULL,             /* vblank callback */
   vrc4_hblank,      /* hblank callback */
   vrc4_getstate,    /* get state (snss) */
   vrc4_setstate,    /* set state (snss) */
   NULL,             /* memory read structure */
   vrc4_memwrite,    /* memory write structure */
   NULL              /* external sound device */
};

mapintf_t map25_intf =
{
   25,               /* mapper number */
   "Konami VRC4 B",  /* mapper name */
   NULL,             /* init routine */
   NULL,             /* vblank callback */
   vrc4_hblank,      /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   vrc4_memwrite,    /* memory write structure */
   NULL              /* external sound device */
};
