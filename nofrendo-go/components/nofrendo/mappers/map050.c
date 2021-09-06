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
** map050.c: SMB2j - 3rd discovered variation
** Implemented by Firebug with information courtesy of Kevin Horton
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
    uint16 enabled;
    uint16 counter;
} irq;


static void map_irq_reset(void)
{
    /* Turn off IRQs */
    irq.enabled = false;
    irq.counter = 0x0000;
}

static void map_hblank(int scanline)
{
    /* Increment the counter if it is enabled and check for strike */
    if (irq.enabled)
    {
        irq.counter += NES_CYCLES_PER_SCANLINE;

        /* IRQ line is hooked to Q12 of the counter */
        if (irq.counter & 0x1000)
        {
            nes6502_irq();
            map_irq_reset();
        }
    }
}

static void map_write(uint32 address, uint8 value)
{
    uint8 selectable_bank;

    /* For address to be decoded, A5 must be high and A6 low */
    if ((address & 0x60) != 0x20) return;

    /* A8 low  = $C000-$DFFF page selection */
    /* A8 high = IRQ timer toggle */
    if (address & 0x100)
    {
        /* IRQ settings */
        if (value & 0x01) irq.enabled = true;
        else              map_irq_reset();
    }
    else
    {
        /* Stupid data line swapping */
        selectable_bank = 0x00;
        if (value & 0x08) selectable_bank |= 0x08;
        if (value & 0x04) selectable_bank |= 0x02;
        if (value & 0x02) selectable_bank |= 0x01;
        if (value & 0x01) selectable_bank |= 0x04;
        mmc_bankrom (8, 0xC000, selectable_bank);
    }
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(8, 0x6000, 0x0F);
    mmc_bankrom(8, 0x8000, 0x08);
    mmc_bankrom(8, 0xA000, 0x09);
    mmc_bankrom(8, 0xE000, 0x0B);
    map_irq_reset();
}


mapintf_t map50_intf =
{
    .number     = 50,
    .name       = "SMB2j (fds hack)",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {{0x4000, 0x5FFF, map_write}},
};
