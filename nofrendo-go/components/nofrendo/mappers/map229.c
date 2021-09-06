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
** map229.c: 31 in 1 mapper interface
** Implemented by Firebug with information courtesy of Mark Knibbs
**
*/

#include <nofrendo.h>
#include <mmc.h>


static void map_write(uint32 address, uint8 value)
{
    /* A4-A0 sets 8K CHR page */
    mmc_bankvrom(8, 0x0000, (uint8) (address & 0x1F));

    /* If A4-A1 are all low then select the first 32K,     */
    /* otherwise select a 16K bank at both $8000 and $C000 */
    if ((address & 0x1E) == 0x00)
    {
        mmc_bankrom (32, 0x8000, 0x00);
    }
    else
    {
        mmc_bankrom (16, 0x8000, (uint8) (address & 0x1F));
        mmc_bankrom (16, 0xC000, (uint8) (address & 0x1F));
    }

    /* A5: mirroring (low = vertical, high = horizontal) */
    if (address & 0x20) ppu_setmirroring(PPU_MIRROR_HORI);
    else                ppu_setmirroring(PPU_MIRROR_VERT);
}

static void map_init(rom_t *cart)
{
    /* On reset, PRG is set to first 32K and CHR to first 8K */
    mmc_bankrom(32, 0x8000, 0x00);
    mmc_bankvrom(8, 0x0000, 0x00);
}


mapintf_t map229_intf =
{
    .number     = 229,
    .name       = "BMC 31-IN-1",
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
