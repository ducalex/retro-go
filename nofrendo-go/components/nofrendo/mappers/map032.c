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
** map032.c: Irem G-101 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static int select_c000 = 0;


static void map32_write(uint32 address, uint8 value)
{
   switch (address >> 12)
   {
   case 0x08:
      if (select_c000)
         mmc_bankrom(8, 0xC000, value);
      else
         mmc_bankrom(8, 0x8000, value);
      break;

   case 0x09:
      if (value & 1)
         ppu_setmirroring(PPU_MIRROR_HORI);
      else
         ppu_setmirroring(PPU_MIRROR_VERT);

      select_c000 = (value & 0x02);
      break;

   case 0x0A:
      mmc_bankrom(8, 0xA000, value);
      break;

   case 0x0B:
      {
         int loc = (address & 0x07) << 10;
         mmc_bankvrom(1, loc, value);
      }
      break;

   default:
      break;
   }
}

static const mem_write_handler_t map32_memwrite[] =
{
   { 0x8000, 0xFFFF, map32_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map32_intf =
{
   32,               /* mapper number */
   "Irem G-101",     /* mapper name */
   NULL,             /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map32_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
