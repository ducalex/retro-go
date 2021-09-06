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
** map009.c: MMC2 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint8 latch[2];
static uint8 regs[4];


/* Used when tile $FD/$FE is accessed */
static void mmc9_latchfunc(uint32 address, uint8 value)
{
    int reg;

    if (address)
    {
        latch[1] = value;
        reg = 2 + (value - 0xFD);
    }
    else
    {
        latch[0] = value;
        reg = value - 0xFD;
    }

    mmc_bankvrom(4, address, regs[reg]);
}

static void map_write(uint32 address, uint8 value)
{
    switch ((address & 0xF000) >> 12)
    {
    case 0xA:
        mmc_bankrom(8, 0x8000, value);
        break;

    case 0xB:
        regs[0] = value;
        if (0xFD == latch[0])
            mmc_bankvrom(4, 0x0000, value);
        break;

    case 0xC:
        regs[1] = value;
        if (0xFE == latch[0])
            mmc_bankvrom(4, 0x0000, value);
        break;

    case 0xD:
        regs[2] = value;
        if (0xFD == latch[1])
            mmc_bankvrom(4, 0x1000, value);
        break;

    case 0xE:
        regs[3] = value;
        if (0xFE == latch[1])
            mmc_bankvrom(4, 0x1000, value);
        break;

    case 0xF:
        if (value & 1)
            ppu_setmirroring(PPU_MIRROR_HORI);
        else
            ppu_setmirroring(PPU_MIRROR_VERT);
        break;

    default:
        break;
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = latch[0];
    state[1] = latch[1];
    state[2] = regs[0];
    state[3] = regs[1];
    state[4] = regs[2];
    state[5] = regs[3];
}

static void map_setstate(uint8 *state)
{
    latch[0] = state[0];
    latch[1] = state[1];
    regs[0]  = state[2];
    regs[1]  = state[3];
    regs[2]  = state[4];
    regs[3]  = state[5];
}

static void map_init(rom_t *cart)
{
    regs[0] = regs[1] = regs[2] = regs[3] = 0x00;
    latch[0] = latch[1] = 0xFE;

    mmc_bankrom(8, 0x8000, 0);
    mmc_bankrom(8, 0xA000, (cart->prg_rom_banks) - 3);
    mmc_bankrom(8, 0xC000, (cart->prg_rom_banks) - 2);
    mmc_bankrom(8, 0xE000, (cart->prg_rom_banks) - 1);

    ppu_setlatchfunc(mmc9_latchfunc);
}


mapintf_t map9_intf =
{
    .number     = 9,
    .name       = "MMC2",
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
