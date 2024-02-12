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
** map019.c: Namco 129/163 mapper interface
**
*/

#include "nes/nes.h"

#define RAM_AUTO_INCREMENT 0x80
#define RAM_ADDR_MASK 0x7F
#define SOUND_STEP 256

typedef struct
{
    uint32 frequency;
    uint32 length;
    uint32 phase;
    uint32 offset;
    uint32 volume;
    uint32 period;
    uint32 enable;
} chan_t;

typedef struct map019_s
{
    rom_t *cart;
    uint8 ram[0x80];
    uint8 address;
    struct {
        uint16 counter;
        uint16 enabled;
    } irq;
    struct {
        chan_t channels[8];
        uint8 changed;
        uint8 enable;
    } sound;
    uint8 nt_map[4];
} map019_t;

static map019_t *map019;


static void sound_prepare(void)
{
    const nes_t *nes = nes_getptr();
    uint32 chan_enable = ((map019->ram[0x7F] >> 4) & 7) + 1;
    uint32 z = ((uint64)nes->apu->sample_rate * (chan_enable * 15 * 32 * 32768) * SOUND_STEP) / nes->cpu_clock;

    for (size_t i = 0; i < 8; i++)
    {
        chan_t *chan = &map019->sound.channels[i];
        uint8 *regs = &map019->ram[0x40 + i * 8];

        chan->frequency = regs[0] | (regs[2] << 8) | ((regs[4] & 3) << 16);
        chan->length = 256 - (regs[4] & 0xFC);
        chan->phase = regs[1] | (regs[3] << 8) | (regs[5] << 16);
        chan->offset = regs[6];
        chan->volume = regs[7] & 0xF;
        chan->period = chan->frequency ? (z / chan->frequency) : (1);
        chan->enable = (chan->frequency != 0 && i >= (8 - chan_enable));
    }
    map019->sound.changed = false;
}

static int sound_process(void)
{
    int active = 0;
    int sample = 0;

    for (size_t i = 0; i < 8; i++)
    {
        chan_t *chan = &map019->sound.channels[i];

        if (!chan->enable)
            continue;

        active++;

        for (size_t j = 0; j < 16; ++j)
        {
            int index = chan->offset + ((chan->phase / chan->period) % chan->length);
            int value = map019->ram[(index >> 1) & 0x3F];
            if (index & 1)
                value >>= 4;
            sample += ((value & 0xF) - 8) * chan->volume;
            chan->phase += SOUND_STEP;
        }

        uint8 *regs = &map019->ram[0x40 + i * 8];
        regs[1] = chan->phase;
        regs[3] = chan->phase >> 8;
        regs[5] = chan->phase >> 16;
    }

    if (!active)
        return 0;

    return sample << 1; // / active;
}

static void sound_reset(void)
{
    memset(map019->ram, 0, 128);
    sound_prepare();
}

static uint8 ram_read(uint32 address)
{
    uint8 temp = map019->ram[map019->address & RAM_ADDR_MASK];
    if (map019->address & RAM_AUTO_INCREMENT)
        map019->address = (map019->address + 1) | RAM_AUTO_INCREMENT;
    return temp;
}

static void ram_write(uint32 address, uint8 value)
{
    map019->ram[map019->address & RAM_ADDR_MASK] = value;
    if (map019->address & RAM_AUTO_INCREMENT)
        map019->address = (map019->address + 1) | RAM_AUTO_INCREMENT;
    map019->sound.changed = true;
}

static void set_nametable(uint8 i, uint8 value)
{
    if (value < 0xE0)
        ppu_setpage(8 + (i & 3), &map019->cart->chr_rom[(value % (map019->cart->chr_rom_banks * 8)) << 10]);
    else
        ppu_setnametable(i & 3, value & 1);
    map019->nt_map[i & 3] = value;
}

static void map_write(uint32 address, uint8 value)
{
    uint8 reg = address >> 11;

    switch (reg)
    {
    case 0x0A: // IRQ Counter (low) ($5000-$57FF)
        map019->irq.counter = (map019->irq.counter & 0x7F00) | value;
        nes6502_irq_clear();
        break;

    case 0x0B: // IRQ Counter (high) / IRQ Enable ($5800-$5FFF)
        map019->irq.counter = (map019->irq.counter & 0x00FF) | ((value & 0x7F) << 8);
        map019->irq.enabled = (value & 0x80);
        nes6502_irq_clear();
        break;

    case 0x10: // CHR and NT Select ($8000-$BFFF)
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
        mmc_bankvrom(1, (reg & 7) << 10, value);
        break;

    case 0x18: // CHR and NT Select ($C000-$DFFF)
    case 0x19:
    case 0x1A:
    case 0x1B:
        set_nametable(reg & 3, value);
        break;

    case 0x1C: // PRG Select 1 ($E000-$E7FF)
        mmc_bankrom(8, 0x8000, value & 0x3F);
        // map019->sound.enable = (value & 0x40) == 0x00;
        break;

    case 0x1D: // PRG Select 2 / CHR-RAM Enable ($E800-$EFFF)
        mmc_bankrom(8, 0xA000, value);
        break;

    case 0x1E: // PRG Select 3 ($F000-$F7FF)
        mmc_bankrom(8, 0xC000, value);
        break;

    case 0x1F: // Address Port ($F800-$FFFF)
        map019->address = value;
        break;

    default:
        break;
    }
}

static uint8 map_read(uint32 address)
{
    uint8 reg = address >> 11;

    switch (reg)
    {
    case 0xA: // IRQ Counter (low) ($5000-$57FF)
        return map019->irq.counter & 0xFF;

    case 0xB: // IRQ Counter (high) / IRQ Enable ($5800-$5FFF)
        return map019->irq.counter >> 8;

    default:
        return 0xFF;
    }
}

static void map_hblank(nes_t *nes)
{
    if (map019->irq.enabled)
    {
        map019->irq.counter += nes->cycles_per_scanline;

        if (map019->irq.counter >= 0x7FFF)
        {
            nes6502_irq();
            map019->irq.counter = 0x7FFF;
            map019->irq.enabled = 0;
        }
    }
    if (map019->sound.changed)
        sound_prepare();
}

static void map_getstate(uint8 *state)
{
    state[0] = map019->irq.counter & 0xFF;
    state[1] = map019->irq.counter >> 8;
    state[2] = map019->irq.enabled;
    state[3] = map019->address;
    state[4] = map019->nt_map[0];
    state[5] = map019->nt_map[1];
    state[6] = map019->nt_map[2];
    state[7] = map019->nt_map[3];
    memcpy(&state[0x80], map019->ram, 0x80);
}

static void map_setstate(uint8 *state)
{
    map019->irq.counter = (state[1] << 8) | state[0];
    map019->irq.enabled = state[2];
    map019->address = state[3];
    set_nametable(0, state[4]);
    set_nametable(1, state[5]);
    set_nametable(2, state[6]);
    set_nametable(3, state[7]);
    memcpy(map019->ram, &state[0x80], 0x80);
}

static const apuext_t sound_ext = {
    .reset = sound_reset,
    .process = sound_process,
};

static void map_init(rom_t *cart)
{
    if (!map019)
        map019 = malloc(sizeof(map019_t));
    memset(map019, 0, sizeof(map019_t));
    map019->cart = cart;
    map019->irq.counter = 0;
    map019->irq.enabled = 0;
    map019->nt_map[0] = 0;
    map019->nt_map[1] = 0;
    map019->nt_map[2] = 1;
    map019->nt_map[3] = 1;
    apu_setext(&sound_ext);
}

mapintf_t map19_intf =
{
    .number     = 19,
    .name       = "Namco 129/163",
    .init       = map_init,
    .vblank     = NULL,
    .hblank     = map_hblank,
    .get_state  = map_getstate,
    .set_state  = map_setstate,
    .mem_read   = {
        { 0x4800, 0x4FFF, ram_read },
        { 0x5000, 0x5FFF, map_read },
    },
    .mem_write  = {
        { 0x4800, 0x4FFF, ram_write },
        { 0x5000, 0x5FFF, map_write },
        { 0x8000, 0xFFFF, map_write },
    },
};
