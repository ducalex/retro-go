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
** map007.c: AOROM mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static bool is_battletoads = 0;


static void map_write(uint32 address, uint8 value)
{
    mmc_bankrom(32, 0x8000, value & 0xF);

    if (value & 0x10)
       // ppu_setmirroring(PPU_MIRROR_SCR1);
       ppu_setnametables(1, is_battletoads ? 0 : 1, 1, 1);
    else
       ppu_setmirroring(PPU_MIRROR_SCR0);
}

static void map_init(rom_t *cart)
{
    is_battletoads = (cart->checksum == 0x279710DC);

    if (is_battletoads)
       MESSAGE_INFO("Enabled Battletoads mirroring hack\n");

    map_write(0x8000, 0);
}


mapintf_t map7_intf =
{
    .number     = 7,
    .name       = "AOROM",
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
