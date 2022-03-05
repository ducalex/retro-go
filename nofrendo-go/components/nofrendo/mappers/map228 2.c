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
** map228.c: Cheetahmen II mapper interface
** Implemented by ducalex
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint8 mram[4];


static void map_update(uint32 address, uint8 value)
{
    uint16 bank1 = ((address >> 6) & 0x7E) + (((address >> 6) & 1) & ((address >> 5) & 1));
    uint16 bank2 = bank1 + (((address >> 5) & 1) ^ 1);
    uint16 vbank = (((address & 0xF) << 2)) | (value & 0x3);

    // Chip 2 doesn't exist
    if ((bank1 & 0x60) == 0x60) {
        bank1 -= 0x20;
        bank2 -= 0x20;
    }

    mmc_bankrom(16, 0x8000, bank1);
    mmc_bankrom(16, 0xc000, bank2);
    mmc_bankvrom(8, 0x0000, vbank);

    if ((address & 0x2000) == 0)
        ppu_setmirroring(PPU_MIRROR_VERT);
    else
        ppu_setmirroring(PPU_MIRROR_HORI);
}

static uint8 map_read(uint32 address)
{
    return mram[address & 3] & 0xF;
}

static void map_write(uint32 address, uint8 value)
{
    mram[address & 3] = value;
}

static void map_init(rom_t *cart)
{
    map_update(0x8000, 0x00);
}


mapintf_t map228_intf =
{
    .number     = 228,
    .name       = "Mapper 228",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {
        { 0x4020, 0x5FFF, map_read },
    },
    .mem_write  = {
        { 0x8000, 0xFFFF, map_update },
        { 0x4020, 0x5FFF, map_write },
    },
};
