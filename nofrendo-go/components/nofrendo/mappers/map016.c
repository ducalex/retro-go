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
** map016.c: Bandai mapper interface
**
*/

#include <nofrendo.h>
#include <mmc.h>
#include <nes.h>

static struct
{
   int counter;
   bool enabled;
} irq;


static void map_write(uint32 address, uint8 value)
{
    int reg = address & 0xF;

    if (reg < 8)
    {
        mmc_bankvrom(1, reg << 10, value);
    }
    else
    {
        switch (address & 0x000F)
        {
        case 0x8:
            mmc_bankrom(16, 0x8000, value);
            break;

        case 0x9:
            switch (value & 3)
            {
            case 0: ppu_setmirroring(PPU_MIRROR_VERT); break;
            case 1: ppu_setmirroring(PPU_MIRROR_HORI); break;
            case 2: ppu_setmirroring(PPU_MIRROR_SCR0); break;
            case 3: ppu_setmirroring(PPU_MIRROR_SCR1); break;
            }
            break;

        case 0xA:
            irq.enabled = (value & 1);
            break;

        case 0xB:
            irq.counter = (irq.counter & 0xFF00) | value;
            break;

        case 0xC:
            irq.counter = (value << 8) | (irq.counter & 0xFF);
            break;

        case 0xD:
            /* eeprom I/O port? */
            break;
        }
    }
}

static void map_hblank(int scanline)
{
    if (irq.enabled && irq.counter)
    {
        if (0 == --irq.counter)
            nes6502_irq();
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = irq.counter & 0xFF;
    state[1] = irq.counter >> 8;
    state[2] = irq.enabled;
}

static void map_setstate(uint8 *state)
{
    irq.counter = (state[1] << 8) | state[0];
    irq.enabled = state[2];
}

static void map_init(rom_t *cart)
{
    mmc_bankrom(16, 0x8000, 0);
    mmc_bankrom(16, 0xC000, MMC_LASTBANK);
    irq.counter = 0;
    irq.enabled = false;
}


mapintf_t map16_intf =
{
    .number     = 16,
    .name       = "Bandai",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = map_getstate,
    .set_state  = map_setstate,
    .mem_read   = {},
    .mem_write  = {
        { 0x6000, 0x600D, map_write },
        { 0x7FF0, 0x7FFD, map_write },
        { 0x8000, 0x800D, map_write },
    },
};
