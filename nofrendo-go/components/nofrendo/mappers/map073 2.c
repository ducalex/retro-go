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
** map073.c: Konami VRC3 mapper interface
** Implemented by Firebug
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
    uint32 counter;
    bool enabled;
} irq;


static void map_hblank(int scanline)
{
    /* Increment the counter if it is enabled and check for strike */
    if (irq.enabled)
    {
        irq.counter += NES_CYCLES_PER_SCANLINE;

        /* Counter triggered on overflow into Q16 */
        if (irq.counter & 0x10000)
        {
                /* Clip to sixteen-bit word */
                irq.counter &= 0xFFFF;

                /* Trigger the IRQ */
                nes6502_irq();

                /* Shut off IRQ counter */
                irq.enabled = false;
        }
    }
}

static void map_write(uint32 address, uint8 value)
{
    switch (address & 0xF000)
    {
        case 0x8000: irq.counter &= 0xFFF0;
                    irq.counter |= (uint32) (value);
                    break;
        case 0x9000: irq.counter &= 0xFF0F;
                    irq.counter |= (uint32) (value << 4);
                    break;
        case 0xA000: irq.counter &= 0xF0FF;
                    irq.counter |= (uint32) (value << 8);
                    break;
        case 0xB000: irq.counter &= 0x0FFF;
                    irq.counter |= (uint32) (value << 12);
                    break;
        case 0xC000: irq.enabled = (value & 0x02);
                    break;
        case 0xF000: mmc_bankrom (16, 0x8000, value);
        default:     break;
    }
}

static void map_init(rom_t *cart)
{
    irq.enabled = false;
    irq.counter = 0x0000;
}


mapintf_t map73_intf =
{
    .number     = 73,
    .name       = "Konami VRC3",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {
        { 0x8000, 0xFFFF, map_write }
    },
};
