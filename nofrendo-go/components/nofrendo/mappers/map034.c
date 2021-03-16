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
** map034.c: Nina-1 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map34_init(rom_t *cart)
{
   UNUSED(cart);

   mmc_bankrom(32, 0x8000, MMC_LASTBANK);
}

static void map34_write(uint32 address, uint8 value)
{
   if ((address & 0x8000) || (0x7FFD == address))
   {
      mmc_bankrom(32, 0x8000, value);
   }
   else if (0x7FFE == address)
   {
      mmc_bankvrom(4, 0x0000, value);
   }
   else if (0x7FFF == address)
   {
      mmc_bankvrom(4, 0x1000, value);
   }
}

static const mem_write_handler_t map34_memwrite[] =
{
   { 0x7FFD, 0xFFFF, map34_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map34_intf =
{
   34,               /* mapper number */
   "Nina-1",         /* mapper name */
   map34_init,       /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map34_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
