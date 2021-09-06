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
** map071.c: Clone of 002 (UNROM) for Codemasters/Camerica games
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    // SCR0/1 mirroring only used by Fire Hawk
    if ((address & 0xF000) == 0x9000)
        ppu_setmirroring((value & 4) ? PPU_MIRROR_SCR1 : PPU_MIRROR_SCR0);
    else
        mmc_bankrom(16, 0x8000, value);
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(16, 0xc000, MMC_LASTBANK);
    mmc_bankrom(16, 0x8000, 0);
}


mapintf_t map71_intf =
{
    .number     = 71,
    .name       = "Mapper 71",
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
