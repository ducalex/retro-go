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
** map046.c: Mapper #46 (Pelican Game Station)
** Implemented by Firebug with information courtesy of Kevin Horton
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint32 prg_bank;
static uint32 chr_bank;


static void map_set_banks(void)
{
    mmc_bankrom(32, 0x8000, prg_bank);
    mmc_bankvrom(8, 0x0000, chr_bank);
}

static void map_write(uint32 address, uint8 value)
{
    /* $8000-$FFFF: D6-D4 = lower three bits of CHR bank */
    /*              D0    = low bit of PRG bank          */
    /* $6000-$7FFF: D7-D4 = high four bits of CHR bank   */
    /*              D3-D0 = high four bits of PRG bank   */
    if (address & 0x8000)
    {
        prg_bank &= ~0x01; // Keep only the high part
        prg_bank |= value & 0x01;

        chr_bank &= ~0x07; // Keep only the high part
        chr_bank |= (value >> 4) & 0x07;
    }
    else
    {
        prg_bank &= 0x01; // Keep only the low part
        prg_bank |= (value & 0x0F) << 1;

        chr_bank &= 0x07; // Keep only the low part
        chr_bank |= ((value >> 4) & 0x0F) << 3;
    }
    map_set_banks();
}

static void map_init(rom_t *cart)
{
    prg_bank = 0x00;
    chr_bank = 0x00;
    map_set_banks();
}


mapintf_t map46_intf =
{
    .number     = 46,
    .name       = "Pelican Game Station",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {{0x6000, 0xFFFF, map_write}},
};
