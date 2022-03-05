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
** map193.c: Mapper 193 interface (War in the Gulf)
** Implemented by ducalex
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    switch (address & 0x03)
    {
        case 0:
            mmc_bankvrom(4, 0x0000, value >> 2);
            break;

        case 1:
            mmc_bankvrom(2, 0x1000, value >> 1);
            break;

        case 2:
            mmc_bankvrom(2, 0x1800, value >> 1);
            break;

        case 3:
            mmc_bankrom(8, 0x8000, value);
            break;
    }
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(8, 0xA000, 0xD);
    mmc_bankrom(8, 0xC000, 0xE);
    mmc_bankrom(8, 0xE000, 0xF);
}


mapintf_t map193_intf =
{
    .number     = 193,
    .name       = "Mapper 193",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {{0x6000, 0x6004, map_write}},
};
