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
** map078.c: mapper 78 interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static bool fourscreen;

/* mapper 78: Holy Diver, Cosmo Carrier */
/* ($8000-$FFFF) D2-D0 = switch $8000-$BFFF */
/* ($8000-$FFFF) D7-D4 = switch PPU $0000-$1FFF */
/* ($8000-$FFFF) D3 = switch mirroring */
static void map78_write(uint32 address, uint8 value)
{
   UNUSED(address);

   mmc_bankrom(16, 0x8000, value & 7);
   mmc_bankvrom(8, 0x0000, (value >> 4) & 0x0F);

   /* Ugh! Same abuse of the 4-screen bit as with Mapper #70 */
   if (fourscreen)
   {
      if (value & 0x08)
         ppu_setmirroring(PPU_MIRROR_VERT);
      else
         ppu_setmirroring(PPU_MIRROR_HORI);
   }
   else
   {
      if ((value >> 3) & 1)
         ppu_setmirroring(PPU_MIRROR_SCR1);
      else
         ppu_setmirroring(PPU_MIRROR_SCR0);
   }
}

static void map78_init(rom_t *cart)
{
   fourscreen = cart->flags & ROM_FLAG_FOURSCREEN;
}

static const mem_write_handler_t map78_memwrite[] =
{
   { 0x8000, 0xFFFF, map78_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map78_intf =
{
   70,               /* mapper number */
   "Mapper 78",      /* mapper name */
   map78_init,       /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map78_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
