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
** map185.c: CNROM with copy protection mapper interface
** Implemented by ducalex with the help of nesdevwiki
**
*/

#include <nofrendo.h>
#include <string.h>
#include <mmc.h>

static uint8 reg = 0;


static void map_write(uint32 address, uint8 value)
{
    // From https://www.nesdev.org/wiki/INES_Mapper_185:
    //      If value AND $0F is nonzero, and if C does not equal $13, then CHR is enabled, otherwise CHR is disabled.
    //      This works with all games except for Seicross (v2).
    if ((value & 0xF) && value != 0x13)
        mmc_bankchr(8, 0x0000, 0, CHR_ROM);
    else
        mmc_bankchr(8, 0x0000, 0, CHR_RAM);
    reg = value;
}

static void map_getstate(uint8 *state)
{
    state[0] = reg;
}

static void map_setstate(uint8 *state)
{
    map_write(0x8000, state[0]);
}

static void map_init(rom_t *cart)
{
    memset(cart->chr_ram, 0xFF, 8192);
    mmc_bankprg(16, 0x8000,  0, PRG_ROM);
    mmc_bankprg(16, 0xC000, -1, PRG_ROM);
    map_write(0x8000, 0);
}


mapintf_t map185_intf =
{
    .number     = 185,
    .name       = "Mapper 185",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = map_getstate,
    .set_state  = map_setstate,
    .mem_read   = {},
    .mem_write  = {
        { 0x8000, 0xFFFF, map_write }
    },
};
