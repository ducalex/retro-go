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
** map085.c: Konami VRC7 mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>

static struct
{
    unsigned counter, latch;
    bool enabled, wait_state;
} irq;


static void map_write(uint32 address, uint8 value)
{
    switch (address)
    {
    // PRG Select 0
    case 0x8000:
        mmc_bankrom(8, 0x8000, value);
        break;

    // PRG Select 1
    case 0x8008:
    case 0x8010:
        mmc_bankrom(8, 0xA000, value);
        break;

    // PRG Select 2
    case 0x9000:
        mmc_bankrom(8, 0xC000, value);
        break;

    // Sound
    case 0x9010:
    case 0x9030:
        // VRC Sound not implemented
        break;

    // CHR Select 0
    case 0xA000:
        mmc_bankvrom(1, 0x0000, value);
        break;

    // CHR Select 1
    case 0xA008:
    case 0xA010:
        mmc_bankvrom(1, 0x0400, value);
        break;

    // CHR Select 2
    case 0xB000:
        mmc_bankvrom(1, 0x0800, value);
        break;

    // CHR Select 3
    case 0xB008:
    case 0xB010:
        mmc_bankvrom(1, 0x0C00, value);
        break;

    // CHR Select 4
    case 0xC000:
        mmc_bankvrom(1, 0x1000, value);
        break;

    // CHR Select 5
    case 0xC008:
    case 0xC010:
        mmc_bankvrom(1, 0x1400, value);
        break;

    // CHR Select 6
    case 0xD000:
        mmc_bankvrom(1, 0x1800, value);
        break;

    // CHR Select 7
    case 0xD008:
    case 0xD010:
        mmc_bankvrom(1, 0x1C00, value);
        break;

    // Mirroring
    case 0xE000:
        switch (value & 3)
        {
        case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
        case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
        case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
        case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
        }
        break;

    // IRQ Latch
    case 0xE008:
    case 0xE010:
        irq.latch = value;
        break;

    // IRQ Control
    case 0xF000:
        irq.wait_state = (value >> 0) & 1;
        irq.enabled = (value >> 1) & 1;
        // irq.mode = (value >> 2) & 1;
        if (irq.enabled)
            irq.counter = irq.latch;
        break;

    // IRQ Acknowledge
    case 0xF008:
    case 0xF010:
        irq.enabled = irq.wait_state;
        break;

    default:
        MESSAGE_DEBUG("unhandled vrc7 write: $%02X to $%04X\n", value, address);
        break;
    }
}

static void map_hblank(int scanline)
{
    if (!irq.enabled)
        return;

    if (++irq.counter > 0xFF)
    {
        irq.counter = irq.latch;
        nes6502_irq();
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = irq.counter;
    state[1] = irq.latch;
    state[2] = irq.enabled;
    state[3] = irq.wait_state;
    // state[4] = irq.mode;
}

static void map_setstate(uint8 *state)
{
    irq.counter = state[0];
    irq.latch = state[1];
    irq.enabled = state[2];
    irq.wait_state = state[3];
    // irq.mode = state[4];
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(16, 0x8000, 0);
    mmc_bankrom(16, 0xC000, MMC_LASTBANK);
    mmc_bankvrom(8, 0x0000, 0);

    irq.counter = irq.latch = 0;
    irq.enabled = irq.wait_state = 0;
}


mapintf_t map85_intf =
{
    .number     = 85,
    .name       = "Konami VRC7",
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
