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
** map031.c: NSF player interface
** Implemented by ducalex
**
*/

#include <nofrendo.h>
#include <string.h>
#include <mmc.h>

static const nsfheader_t *header;
static rom_t *cart;
static int current_song = 1;
static bool playing = false;
#define SYNC_TO_VBLANK 1

// Ugly hack to access from the gui until we make a nice
// GUI running on the NES in here.
int nsf_current_song = 0;

static void setup_bank(int bank, int value)
{
    MESSAGE_INFO("Bank %02x %02x\n", bank, value);
    // size_t offset1 = header->load_addr & 0xFFF;
    memcpy(cart->prg_rom + bank * 0x1000, cart->data_ptr + 128 + value * 0x1000, 0x1000);
}

static void setup_song(int song)
{
    song %= (header->total_songs + 1);

    const uint8 program[] = {
        // Setup APU:                                               // $6000
        0xA9, 0x0F,                                                 // LDA #$0F
        0x8D, 0x15, 0x40,                                           // STA $4015
        0xA9, 0x40,                                                 // LDA #$40
        0x8D, 0x17, 0x40,                                           // STA $4017
        // Init song:
        0xA9, song - 1,                                             // LDA #$song
        0xA2, header->region,                                       // LDX #$region
        0x20, header->init_addr & 0xFF, header->init_addr >> 8,     // JSR $init_addr
        // Playback loop:                                           // $6011
#if SYNC_TO_VBLANK
        0xAD, 0x02, 0x20,                                           // LDA $2002
        0x29, 0x80,                                                 // AND #$80
        0x10, 0x03,                                                 // BPL #$03
#endif
        0x20, header->play_addr & 0xFF, header->play_addr >> 8,     // JSR $play_addr
        0x8E, 0x00, 0x58,                                           // STX $5800
        0x4C, 0x11, 0x60,                                           // JMP $6007
    };
    memcpy(cart->prg_ram, program, sizeof(program));

    // Load song data into ROM
    if (*((uint64_t*)header->banks) == 0)
    {
        size_t offset = header->load_addr - 0x8000;
        size_t len = MIN(cart->data_len - 128, 0x7FF8 - offset);
        memcpy(cart->prg_rom + offset, cart->data_ptr + 128, len);
    }
    else
    {
        for (size_t i = 0; i < 8; i++)
            setup_bank(i, header->banks[i]);
    }
    cart->prg_rom[0xFFFC - 0x8000] = 0x00;
    cart->prg_rom[0xFFFD - 0x8000] = 0x60;

    nsf_current_song = song;
    playing = false;

    MESSAGE_INFO("song #%d is ready to play!\n", song);
}

// That whole thing should be written in 6502 assembly above instead but for now I'm lazy
static void map_vblank(void)
{
    if (playing && nes_getptr()->input->state)
    {
        setup_song(++current_song);
        nes6502_reset();
        apu_reset();
        nes6502_burn(NES_CPU_CLOCK / 2);
    }
}

static void map_write(uint32 address, uint8 value)
{
    if (address >= 0x5FF8 && address <= 0x5FFF) // bank switching
    {
        setup_bank(address - 0x5FF8, value);
    }
    else if (address == 0x5800) // playback sync
    {
#if !SYNC_TO_VBLANK
        nes6502_burn(header->ntsc_speed * (NES_CPU_CLOCK / 1000000.f) - 214);
#endif
        playing = true;
    }
}

static void map_init(rom_t *_cart)
{
    cart = _cart;
    header = (const nsfheader_t *)cart->data_ptr;

    MESSAGE_INFO("NSF: Songs:%d (%d), Load:0x%04x, Init:0x%04x, Play:0x%04x\n",
                header->total_songs,
                header->first_song,
                header->load_addr,
                header->init_addr,
                header->play_addr);

    MESSAGE_INFO("Name: '%s'\n", header->name);
    MESSAGE_INFO("Artist: '%s'\n", header->artist);
    MESSAGE_INFO("Copyright: '%s'\n", header->copyright);

    setup_song(header->first_song);
}


mapintf_t map31_intf =
{
    .number     = 31,
    .name       = "NSF Player",
    .init       = map_init,
    .vblank     = map_vblank,
    .hblank     = NULL,
    .get_state  = NULL,
    .set_state  = NULL,
    .mem_read   = {},
    .mem_write  = {
        { 0x5800, 0x5FFF, map_write }
    },
};
