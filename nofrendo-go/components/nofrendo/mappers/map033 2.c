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
** map033.c: Taito TC0190 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    int page = (address >> 13) & 3;
    int reg = address & 3;

    switch (page)
    {
    case 0: /* $800X */
        switch (reg)
        {
        case 0:
            mmc_bankrom(8, 0x8000, value);
            break;

        case 1:
            mmc_bankrom(8, 0xA000, value);
            break;

        case 2:
            mmc_bankvrom(2, 0x0000, value);
            break;

        case 3:
            mmc_bankvrom(2, 0x0800, value);
            break;
        }
        break;

    case 1: /* $A00X */
        {
            int loc = 0x1000 + (reg << 10);
            mmc_bankvrom(1, loc, value);
        }
        break;

    case 2: /* $C00X */
    case 3: /* $E00X */
        switch (reg)
        {
        case 0:
            /* irqs maybe ? */
            //break;

        case 1:
            /* this doesn't seem to work just right */
            if (value & 1)
                ppu_setmirroring(PPU_MIRROR_HORI);
            else
                ppu_setmirroring(PPU_MIRROR_VERT);
            break;

        default:
            break;
        }
        break;
    }
}


mapintf_t map33_intf =
{
    .number     = 33,
    .name       = "Taito TC0190",
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
