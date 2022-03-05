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
** map022.c: VRC2a mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    int reg = address >> 12;

    switch (reg)
    {
    case 0x8:
        mmc_bankrom(8, 0x8000, value);
        break;

    case 0xA:
        mmc_bankrom(8, 0xA000, value);
        break;

    case 0x9:
        switch (value & 3)
        {
            case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
            case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
            case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
            case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
        }
        break;

    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
        {
            int loc = (((reg - 0xB) << 1) + (address & 1)) << 10;
            mmc_bankvrom(1, loc, value >> 1);
        }
        break;

    default:
        break;
    }
}


mapintf_t map22_intf =
{
    .number     = 22,
    .name       = "Konami VRC2 A",
    .init       = NULL,
    .vblank     = NULL,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {
        { 0x8000, 0xFFFF, map_write }
    },
};
