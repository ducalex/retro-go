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
** map040.c: SMB 2j (hack) mapper interface
**
*/

#include "nes/nes.h"

static struct
{
    bool  enabled;
    uint8 counter;
    uint8 reload;
} irq;


static void map_hblank(nes_t *nes)
{
    if (irq.enabled && irq.counter)
    {
        if (0 == --irq.counter)
        {
            nes6502_irq();
            irq.enabled = false;
        }
    }
}

static void map_write(uint32 address, uint8 value)
{
    int range = (address >> 13) - 4;

    switch (range)
    {
    case 0: /* 0x8000-0x9FFF */
        irq.enabled = false;
        irq.counter = irq.reload;
        break;

    case 1: /* 0xA000-0xBFFF */
        irq.enabled = true;
        break;

    case 3: /* 0xE000-0xFFFF */
        mmc_bankrom(8, 0xC000, value & 7);
        break;

    default:
        break;
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = irq.counter;
    state[1] = irq.enabled;
}

static void map_setstate(uint8 *state)
{
    irq.counter = state[0];
    irq.enabled = state[1];
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(8, 0x6000, 6);
    mmc_bankrom(8, 0x8000, 4);
    mmc_bankrom(8, 0xA000, 5);
    mmc_bankrom(8, 0xE000, 7);

    irq.enabled = false;
    irq.reload = 4096 / nes_getptr()->cycles_per_scanline;
    irq.counter = irq.reload;
}


mapintf_t map40_intf =
{
    .number     = 40,
    .name       = "SMB 2j (hack)",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = map_getstate,
    .set_state  = map_setstate,
    .mem_read   = {},
    .mem_write  = {
        { 0x8000, 0xFFFF, map_write }
    },
};
