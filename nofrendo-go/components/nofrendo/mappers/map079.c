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
** map078.c: NINA-03/06 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map79_write(uint32 address, uint8 value)
{
   if ((address & 0x5100) == 0x4100)
   {
      mmc_bankrom(32, 0x8000, (value >> 3) & 1);
      mmc_bankvrom(8, 0x0000, value & 7);
   }
}

static void map79_init(rom_t *cart)
{
   UNUSED(cart);

   mmc_bankrom(32, 0x8000, 0);
   mmc_bankvrom(8, 0x0000, 0);
}

static const mem_write_handler_t map79_memwrite[] =
{
   { 0x4100, 0x5FFF, map79_write }, /* ????? incorrect range ??? */
   LAST_MEMORY_HANDLER
};

mapintf_t map79_intf =
{
   79,               /* mapper number */
   "NINA-03/06",     /* mapper name */
   map79_init,       /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map79_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
