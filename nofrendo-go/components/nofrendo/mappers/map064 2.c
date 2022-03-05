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
** map064.c: Tengen RAMBO-1 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
    uint16 counter, latch;
    bool enabled, reset;
} irq;

static uint16 command = 0;
static uint16 vrombase = 0x0000;


static void map_hblank(int scanline)
{
    if (scanline >= 241)
        return;

    irq.reset = false;

    if (ppu_enabled())
    {
        if (0 == irq.counter--)
        {
            irq.counter = irq.latch;

            if (irq.enabled)
                nes6502_irq();

            irq.reset = true;
        }
    }
}

static void map_write(uint32 address, uint8 value)
{
    switch (address & 0xE001)
    {
    case 0x8000:
        command = value;
        vrombase = (value & 0x80) ? 0x1000 : 0x0000;
        break;

    case 0x8001:
        switch (command & 0xF)
        {
        case 0:
            mmc_bankvrom(1, 0x0000 ^ vrombase, value);
            mmc_bankvrom(1, 0x0400 ^ vrombase, value);
            break;

        case 1:
            mmc_bankvrom(1, 0x0800 ^ vrombase, value);
            mmc_bankvrom(1, 0x0C00 ^ vrombase, value);
            break;

        case 2:
            mmc_bankvrom(1, 0x1000 ^ vrombase, value);
            break;

        case 3:
            mmc_bankvrom(1, 0x1400 ^ vrombase, value);
            break;

        case 4:
            mmc_bankvrom(1, 0x1800 ^ vrombase, value);
            break;

        case 5:
            mmc_bankvrom(1, 0x1C00 ^ vrombase, value);
            break;

        case 6:
            mmc_bankrom(8, (command & 0x40) ? 0xA000 : 0x8000, value);
            break;

        case 7:
            mmc_bankrom(8, (command & 0x40) ? 0xC000 : 0xA000, value);
            break;

        case 8:
            mmc_bankvrom(1, 0x0400, value);
            break;

        case 9:
            mmc_bankvrom(1, 0x0C00, value);
            break;

        case 15:
            mmc_bankrom(8, (command & 0x40) ? 0x8000 : 0xC000, value);
            break;

        default:
            MESSAGE_DEBUG("mapper 64: unknown command #%d", command & 0xF);
            break;
        }
        break;

    case 0xA000:
        if (value & 1)
            ppu_setmirroring(PPU_MIRROR_HORI);
        else
            ppu_setmirroring(PPU_MIRROR_VERT);
        break;

    case 0xC000:
        //irq.counter = value;
        irq.latch = value;
        break;

    case 0xC001:
        //irq.latch = value;
        irq.reset = true;
        break;

    case 0xE000:
        //irq.counter = irq.latch;
        irq.enabled = false;
        break;

    case 0xE001:
        irq.enabled = true;
        break;

    default:
        MESSAGE_DEBUG("mapper 64: Wrote $%02X to $%04X", value, address);
        break;
    }

    if (true == irq.reset)
        irq.counter = irq.latch;
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(8, 0x8000, MMC_LASTBANK);
    mmc_bankrom(8, 0xA000, MMC_LASTBANK);
    mmc_bankrom(8, 0xC000, MMC_LASTBANK);
    mmc_bankrom(8, 0xE000, MMC_LASTBANK);

    irq.counter = irq.latch = 0;
    irq.reset = irq.enabled = false;
}


mapintf_t map64_intf =
{
    .number     = 64,
    .name       = "Tengen RAMBO-1",
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
