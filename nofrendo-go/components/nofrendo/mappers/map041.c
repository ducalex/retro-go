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
** map041.c: Mapper #41 (Caltron 6 in 1)
** Implemented by Firebug with information courtesy of Kevin Horton
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static uint8 register_low;
static uint8 register_high;


static void map_update(void)
{
    mmc_bankvrom(8, 0x0000, ((register_low >> 1) & 0x0C) | (register_high));
}

static void map_low_write(uint32 address, uint8 value)
{
    /* $6000-$67FF: A5    = mirroring (1=horizontal, 0=vertical)      */
    /*              A4-A3 = high two bits of 8K CHR bank              */
    /*              A2    = register 1 enable (0=disabled, 1=enabled) */
    /*              A2-A0 = 32K PRG bank                              */
    register_low = (uint8) (address & 0x3F);
    mmc_bankrom(32, 0x8000, register_low & 0x07);
    map_update();
    if (register_low & 0x20) ppu_setmirroring(PPU_MIRROR_HORI);
    else                     ppu_setmirroring(PPU_MIRROR_VERT);
}

static void map_high_write(uint32 address, uint8 value)
{
    /* $8000-$FFFF: D1-D0 = low two bits of 8K CHR bank */
    if (register_low & 0x04)
    {
        register_high = value & 0x03;
        map_update();
    }
}

static void map_init(rom_t *cart)
{
    /* Both registers set to zero at power on */
    /* TODO: Registers should also be cleared on a soft reset */
    register_low = 0x00;
    register_high = 0x00;
    mmc_bankrom(32, 0x8000, 0x00);
    map_update();
}


mapintf_t map41_intf =
{
    .number     = 41,
    .name       = "Caltron 6 in 1",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {
        { 0x6000, 0x67FF, map_low_write },
        { 0x8000, 0xFFFF, map_high_write },
    },
};
