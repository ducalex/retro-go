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
** nes_rom.h
**
** NES ROM loading/saving related defines / prototypes
** $Id: nes_rom.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _NES_ROM_H_
#define _NES_ROM_H_

#define  ROM_INES_MAGIC          "NES\x1A"
#define  ROM_FDS_MAGIC           "FDS\x1A"

#define  ROM_FLAG_FOURSCREEN     0x08
#define  ROM_FLAG_TRAINER        0x04
#define  ROM_FLAG_BATTERY        0x02
#define  ROM_FLAG_VERTICAL       0x01

#define  ROM_VRAM_BANKS          1 // non-ines flags
#define  ROM_SRAM_BANKS          8 // non-ines flags

#define  ROM_PROG_BANK_SIZE      0x4000
#define  ROM_VROM_BANK_SIZE      0x2000
#define  ROM_VRAM_BANK_SIZE      0x2000
#define  ROM_SRAM_BANK_SIZE      0x400

#define  TRAINER_OFFSET          0x1000
#define  TRAINER_LENGTH          0x200

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

typedef union
{
   inesheader_t ines;
   fdsheader_t  fds;
} fileheader_t;

typedef struct
{
   uint8 *data_ptr; // Top of our allocation
   size_t data_len; // Size of our allocation
   bool data_const; // Prevents nofrendo from modifying or freeing data_ptr

   char filename[PATH_MAX + 1];

   uint8 *prg_rom;
   uint8 *chr_rom;
   uint8 prg_ram[ROM_SRAM_BANKS * ROM_SRAM_BANK_SIZE];
   uint8 chr_ram[ROM_VRAM_BANKS * ROM_VRAM_BANK_SIZE];

   int prg_rom_banks;
   int chr_rom_banks;
   int prg_ram_banks;
   int chr_ram_banks;

   uint32 mapper_number;
   uint32 flags;
   uint32 checksum;
} rom_t;


extern rom_t *rom_loadfile(const char *filename);
extern rom_t *rom_loadmem(uint8 *data, size_t size);
extern void rom_free(void);

#endif /* _NES_ROM_H_ */
