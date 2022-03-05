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
** map065.c: Irem H-3001 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static struct
{
    uint16 counter;
    uint16 cycles;
    uint8 low, high;
    bool enabled;
} irq;


/* TODO: shouldn't there be some kind of HBlank callback??? */

static void map_write(uint32 address, uint8 value)
{
    int range = address & 0xF000;
    int reg = address & 7;

    switch (range)
    {
    case 0x8000:
    case 0xA000:
    case 0xC000:
        mmc_bankrom(8, range, value);
        break;

    case 0xB000:
        mmc_bankvrom(1, reg << 10, value);
        break;

    case 0x9000:
        switch (reg)
        {
        case 4:
            irq.enabled = (value & 0x01) ? false : true;
            break;

        case 5:
            irq.high = value;
            irq.cycles = (irq.high << 8) | irq.low;
            irq.counter = (uint8)(irq.cycles / 128);
            break;

        case 6:
            irq.low = value;
            irq.cycles = (irq.high << 8) | irq.low;
            irq.counter = (uint8)(irq.cycles / 128);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

static void map_init(rom_t *cart)
{
    irq.counter = 0;
    irq.enabled = false;
    irq.low = irq.high = 0;
    irq.cycles = 0;
}


mapintf_t map65_intf =
{
    .number     = 65,
    .name       = "Irem H-3001",
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
