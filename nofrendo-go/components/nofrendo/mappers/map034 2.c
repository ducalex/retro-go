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


static void map_write(uint32 address, uint8 value)
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

static void map_init(rom_t *cart)
{
    mmc_bankrom(32, 0x8000, MMC_LASTBANK);
}


mapintf_t map34_intf =
{
    .number     = 34,
    .name       = "Nina-1",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {{0x7FFD, 0xFFFF, map_write}},
};
