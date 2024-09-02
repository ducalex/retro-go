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
** nes/rom.h: ROM loader header
**
*/

#pragma once

#define ROM_NES_MAGIC          "NES\x1A"
#define ROM_NSF_MAGIC          "NESM\x1A"
#define ROM_FDS_MAGIC          "FDS\x1A"
#define ROM_FDS_RAW_MAGIC      "\x01*NINTENDO-HVC*"

#define ROM_FLAG_FOURSCREEN     0x08
#define ROM_FLAG_TRAINER        0x04
#define ROM_FLAG_BATTERY        0x02
#define ROM_FLAG_VERTICAL       0x01

#define ROM_PRG_BANK_SIZE       0x2000
#define ROM_CHR_BANK_SIZE       0x2000

typedef enum
{
   ROM_TYPE_INVALID = 0,
   ROM_TYPE_INES,
   ROM_TYPE_FDS,
   ROM_TYPE_NSF,
} rom_type_t;

typedef struct __attribute__((packed))
{
   uint8 magic[4];
   uint8 prg_banks;
   uint8 chr_banks;
   uint8 rom_type;
   uint8 mapper_hinybble;
   uint32 reserved1;
   uint32 reserved2;
} inesheader_t;

typedef struct __attribute__((packed))
{
   uint8 magic[4];
   uint8 sides;
   uint8 reserved[11];
} fdsheader_t;

typedef struct __attribute__((packed))
{
   uint8 magic[5];
   uint8 version;
   uint8 total_songs;
   uint8 first_song;
   uint16 load_addr;
   uint16 init_addr;
   uint16 play_addr;
   char name[32];
   char artist[32];
   char copyright[32];
   uint16 ntsc_speed;
   uint8 banks[8];
   uint16 pal_speed;
   uint8 region;
   uint8 expansion;
} nsfheader_t;

typedef struct
{
   rom_type_t type;
   uint8 mapper_number;
   uint8 mirroring;
   uint8 system;
   uint32 checksum;

   uint8 *data_ptr;
   size_t data_len;

   uint8 *prg_rom;
   uint8 *chr_rom;
   uint8 *prg_ram;
   uint8 *chr_ram;
   uint8 *trainer;

   bool free_data_ptr;
   bool free_prg_rom;
   bool free_chr_rom;
   bool free_trainer;

   bool battery;
   bool disksystem;
   bool fourscreen;
   bool vertical;

   int prg_rom_banks;
   int chr_rom_banks;
   int prg_ram_banks;
   int chr_ram_banks;

   char *filename;
} rom_t;


rom_t *rom_loadfile(const char *filename);
rom_t *rom_loadmem(uint8 *data, size_t size);
void rom_free(void);

void rom_savesram(const char *filename);
void rom_loadsram(const char *filename);
