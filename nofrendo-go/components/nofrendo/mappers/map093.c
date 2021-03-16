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
** map093.c: Mapper 93 interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map93_write(uint32 address, uint8 value)
{
   UNUSED(address);

   /* ($8000-$FFFF) D7-D4 = switch $8000-$BFFF D0: mirror */
   mmc_bankrom(16, 0x8000, value >> 4);

   if (value & 1)
      ppu_setmirroring(PPU_MIRROR_VERT);
   else
      ppu_setmirroring(PPU_MIRROR_HORI);
}

static const mem_write_handler_t map93_memwrite[] =
{
   { 0x8000, 0xFFFF, map93_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map93_intf =
{
   93,               /* mapper number */
   "Mapper 93",      /* mapper name */
   NULL,             /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map93_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
