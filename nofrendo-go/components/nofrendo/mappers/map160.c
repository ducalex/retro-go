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
** map160.c: Mapper 160 interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static struct
{
    bool enabled, expired;
    int counter;
    int latch_c005, latch_c003;
} irq;


static void map_write(uint32 address, uint8 value)
{
    if (address >= 0x8000 && address <= 0x8003)
    {
        mmc_bankrom(8, 0x8000 + 0x2000 * (address & 3), value);
    }
    else if (address >= 0x9000 && address <= 0x9007)
    {
        mmc_bankvrom(1, 0x400 * (address & 7), value);
    }
    else if (0xC002 == address)
    {
        irq.enabled = false;
        irq.latch_c005 = irq.latch_c003;
    }
    else if (0xC003 == address)
    {
        if (false == irq.expired)
        {
            irq.counter = value;
        }
        else
        {
            irq.expired = false;
            irq.enabled = true;
            irq.counter = irq.latch_c005;
        }
    }
    else if (0xC005 == address)
    {
        irq.latch_c005 = value;
        irq.counter = value;
    }
    else
    {
        MESSAGE_DEBUG("mapper 160: unhandled write $%02X to $%04X\n", value, address);
    }
}

static void map_hblank(int scanline)
{
    if (scanline < 241)
    {
        if (irq.enabled && ppu_enabled())
        {
            if (0 == irq.counter && false == irq.expired)
            {
                irq.expired = true;
                nes6502_irq();
            }
            else
            {
                irq.counter--;
            }
        }
    }
}

static void map_init(rom_t *cart)
{
    irq.enabled = false;
    irq.expired = false;
    irq.counter = 0;
    irq.latch_c003 = irq.latch_c005 = 0;
}


mapintf_t map160_intf =
{
    .number     = 160,
    .name       = "Aladdin (pirate)",
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
