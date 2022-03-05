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
** map001.c: MMC1 mapper interface
** Implemented by ducalex
**
*/

#include <nofrendo.h>
#include <mmc.h>

static uint8 regs[4];
static uint8 latch = 0;
static int bitcount = 0;
static int prg_banks = 0;


static void update_mirror()
{
    switch (regs[0] & 3)
    {
        case 0: ppu_setmirroring(PPU_MIRROR_SCR0); break;
        case 1: ppu_setmirroring(PPU_MIRROR_SCR1); break;
        case 2: ppu_setmirroring(PPU_MIRROR_VERT); break;
        case 3: ppu_setmirroring(PPU_MIRROR_HORI); break;
    }
}

static void update_chr()
{
    if (regs[0] & 0x10)
    {
        mmc_bankvrom(4, 0x0000, regs[1]);
        mmc_bankvrom(4, 0x1000, regs[2]);
    }
    else
    {
        mmc_bankvrom(8, 0x0000, regs[1] >> 1);
    }
}

static void update_prg()
{
    int offset = prg_banks >= 0x10 ? (regs[1] & 0x10) : 0;
    int bank = regs[3] & 0xF;
    int mode = (regs[0] >> 2) & 3;

    // Note: "fixed" banks do respect the offset!

    switch (mode)
    {
    case 0:
    case 1:
        // switch 32 KB at $8000, ignoring low bit of bank number
        mmc_bankrom(32, 0x8000, offset + (bank >> 1));
        break;

    case 2:
        // fix first bank at $8000 and switch 16 KB bank at $C000
        mmc_bankrom(16, 0x8000, offset);
        mmc_bankrom(16, 0xC000, offset + bank);
        break;

    case 3:
        // fix last bank at $C000 and switch 16 KB bank at $8000
        mmc_bankrom(16, 0x8000, offset + bank);
        mmc_bankrom(16, 0xC000, offset + ((prg_banks - 1) & 0xF));
        break;
    }
}

static void map_write(uint32 address, uint8 value)
{
    // MESSAGE_INFO("MMC1 write: $%02X to $%04X\n", value, address);

    // Reset
    if (value & 0x80)
    {
        regs[0] |= 0x0C;
        bitcount = 0;
        latch = 0;
        update_prg();
        return;
    }

    // Serial data in
    latch |= ((value & 1) << bitcount++);

    /* Wait until we received 5 bits */
    if (bitcount != 5)
        return;

    // Only matters on fifth write
    int regnum = (address >> 13) & 0x3;

    regs[regnum] = latch;
    bitcount = 0;
    latch = 0;

    switch (regnum)
    {
    case 0:
        // Register 0: Control
        update_mirror();
        update_prg();
        update_chr();
        break;

    case 1:
        // Register 1: CHR bank 0
        update_chr();
        update_prg();
        break;

    case 2:
        // Register 2: CHR bank 1
        update_chr();
        break;

    case 3:
        // Register 3: PRG bank
        update_prg();
        break;
    }
}

static void map_getstate(uint8 *state)
{
    state[0] = regs[0];
    state[1] = regs[1];
    state[2] = regs[2];
    state[3] = regs[3];
    state[4] = bitcount;
    state[5] = latch;
}

static void map_setstate(uint8 *state)
{
    regs[0]  = state[0];
    regs[1]  = state[1];
    regs[2]  = state[2];
    regs[3]  = state[3];
    bitcount = state[4];
    latch    = state[5];
}

static void map_init(rom_t *cart)
{
    prg_banks = cart->prg_rom_banks / 2;
    bitcount = 0;
    latch = 0;

    regs[0] = 0x1F;
    regs[1] = 0x00;
    regs[2] = 0x00;
    regs[3] = 0x00;

    update_mirror();
    update_chr();
    update_prg();
}


mapintf_t map1_intf =
{
    .number     = 1,
    .name       = "MMC1",
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
