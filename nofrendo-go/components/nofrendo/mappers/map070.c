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
** map070.c: mapper 70 interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static bool fourscreen;

/* mapper 70: Arkanoid II, Kamen Rider Club, etc. */
/* ($8000-$FFFF) D6-D4 = switch $8000-$BFFF */
/* ($8000-$FFFF) D3-D0 = switch PPU $0000-$1FFF */
/* ($8000-$FFFF) D7 = switch mirroring */
static void map70_write(uint32 address, uint8 value)
{
   UNUSED(address);

   mmc_bankrom(16, 0x8000, (value >> 4) & 0x07);
   mmc_bankvrom(8, 0x0000, value & 0x0F);

   /* Argh! FanWen used the 4-screen bit to determine
   ** whether the game uses D7 to switch between
   ** horizontal and vertical mirroring, or between
   ** one-screen mirroring from $2000 or $2400.
   */
   if (fourscreen)
   {
      if (value & 0x80)
         ppu_setmirroring(PPU_MIRROR_HORI);
      else
         ppu_setmirroring(PPU_MIRROR_VERT);
   }
   else
   {
      if (value & 0x80)
         ppu_setmirroring(PPU_MIRROR_SCR1);
      else
         ppu_setmirroring(PPU_MIRROR_SCR0);
   }
}

static void map70_init(rom_t *cart)
{
   fourscreen = cart->flags & ROM_FLAG_FOURSCREEN;
}

static const mem_write_handler_t map70_memwrite[] =
{
   { 0x8000, 0xFFFF, map70_write },
   LAST_MEMORY_HANDLER
};

mapintf_t map70_intf =
{
   70,               /* mapper number */
   "Mapper 70",      /* mapper name */
   map70_init,       /* init routine */
   NULL,             /* vblank callback */
   NULL,             /* hblank callback */
   NULL,             /* get state (snss) */
   NULL,             /* set state (snss) */
   NULL,             /* memory read structure */
   map70_memwrite,   /* memory write structure */
   NULL              /* external sound device */
};
