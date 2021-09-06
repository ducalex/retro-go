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
** map076.c: NAMCOT-3446 mapper interface
** Implemented by ducalex with the help of nesdevwiki
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint8 reg8000;


static void map_write(uint32 address, uint8 value)
{
    switch (address & 0xE001)
    {
    case 0x8000: // Bank select
        reg8000 = value;
        break;

    case 0x8001: // Bank data
        switch (reg8000 & 0x07)
        {
        case 0: // Select 2 KB CHR bank at PPU $0000-$07FF
            // mmc_bankchr(2, 0x0000, value & 0x1F, CHR_ROM);
            break;

        case 1: // Select 2 KB CHR bank at PPU $0800-$0FFF
            // mmc_bankchr(2, 0x0000, value & 0x1F, CHR_ROM);
            break;

        case 2: // Select 2 KB CHR bank at PPU $0000-$17FF
            mmc_bankchr(2, 0x0000, value & 0x3F, CHR_ROM);
            break;

        case 3: // Select 2 KB CHR bank at PPU $0800-$1000
            mmc_bankchr(2, 0x0800, value & 0x3F, CHR_ROM);
            break;

        case 4: // Select 2 KB CHR bank at PPU $1000-$17FF
            mmc_bankchr(2, 0x1000, value & 0x3F, CHR_ROM);
            break;

        case 5: // Select 2 KB CHR bank at PPU $1800-$2000
            mmc_bankchr(2, 0x1800, value & 0x3F, CHR_ROM);
            break;

        case 6: // Select 8 KB PRG ROM bank at $8000-$9FFF
            mmc_bankprg(8, 0x8000, value & 0xF, PRG_ROM);
            break;

        case 7: // Select 8 KB PRG ROM bank at $A000-$BFFF
            mmc_bankprg(8, 0xA000, value & 0xF, PRG_ROM);
            break;
        }
        break;

    default:
        MESSAGE_DEBUG("map076: unhandled write: address=%p, value=0x%x\n", (void*)address, value);
        break;
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = reg8000;
}

static void map_setstate(uint8 *state)
{
    reg8000 = state[0];
}

static void map_init(rom_t *cart)
{
    // The MMC already mapped the fixed PRG correctly by now, do nothing.
    // mmc_bankprg(8, 0xC000, -2, PRG_ROM);
    // mmc_bankprg(8, 0xE000, -1, PRG_ROM);
    reg8000 = 0;
}


mapintf_t map76_intf =
{
    .number     = 76,
    .name       = "NAMCOT-3446",
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
