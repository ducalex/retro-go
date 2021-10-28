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


static void setup_song(int song)
{
    song %= (header->total_songs + 1);

    const uint8 program[] = {
        // Init song $6000:
        0xA9, song - 1,                                             // LDA #$song
        0xA2, header->region,                                       // LDX #$region
        0x20, header->init_addr & 0xFF, header->init_addr >> 8,     // JSR $init_addr
        0xEA,                                                       // NOP
        // Play song $6008:
        0x20, header->play_addr & 0xFF, header->play_addr >> 8,     // JSR $play_addr
        // Latch input
        0xA2, 0x01,                                                 // LDX #$01
        0x8E, 0x16, 0x40,                                           // STX $4016
        0xA2, 0x00,                                                 // LDX #$01
        0x8E, 0x16, 0x40,                                           // STX $4016
        // Read input to control playback
        // ... not written yet
        // busy loop to tick length
        // ... not written yet so call the C code below
        0x8E, 0x51, 0x51,                                           // STX 5151
        // Loop back to playback
        0x4C, 0x08, 0x60,                                           // JMP $6000
    };

    memcpy(cart->prg_ram, program, sizeof(program));
    memcpy(cart->prg_rom + (header->load_addr - 0x8000), cart->data_ptr + 128, cart->data_len - 128);

    // set reset vector
    cart->prg_rom[0xFFFC - 0x8000] = 0x00;
    cart->prg_rom[0xFFFD - 0x8000] = 0x60;

    playing = false;

    printf("song #%d is ready to play!\n", song);
}

// That whole thing should be written in 6502 assembly above instead but for now I'm lazy
static void map_vblank(void)
{
    if (!playing)
        return;

    int pressed = false;
    for (int i = 0; i < 8; ++i)
    {
        pressed = pressed || (mem_getbyte(0x4016) & 1);
    }

    if (pressed)
    {
        setup_song(++current_song);
        nes6502_reset();
        apu_reset();
        nes6502_burn(NES_CPU_CLOCK / 2);
    }
}

static void map_write(uint32 address, uint8 value)
{
    nes6502_burn(header->ntsc_speed * (NES_CPU_CLOCK / 1000000.f) - 214);
    playing = true;
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
    .mem_write  = {
        { 0x5000, 0x5FFF, map_write }
    },
};
