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
** map231.c: NINA-07 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    int prg_bank = ((value & 0x80) >> 5) | (value & 0x03);
    int chr_bank = (value >> 4) & 0x07;

    mmc_bankrom(32, 0x8000, prg_bank);
    mmc_bankvrom(8, 0x0000, chr_bank);
}

static void map_init(rom_t *cart)
{
   mmc_bankrom(32, 0x8000, MMC_LASTBANK);
}


mapintf_t map231_intf =
{
    .number     = 231,
    .name       = "NINA-07",
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
