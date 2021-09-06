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

static bool fourscreen = false;


/* ($8000-$FFFF) D6-D4 = switch $8000-$BFFF */
/* ($8000-$FFFF) D3-D0 = switch PPU $0000-$1FFF */
/* ($8000-$FFFF) D7 = switch mirroring */
static void map_write(uint32 address, uint8 value)
{
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

static void map_init(rom_t *cart)
{
    fourscreen = cart->flags & ROM_FLAG_FOURSCREEN;
}


mapintf_t map70_intf =
{
    .number     = 70,
    .name       = "Mapper 70",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {
        { 0x8000, 0xFFFF, map_write }
    },
};
