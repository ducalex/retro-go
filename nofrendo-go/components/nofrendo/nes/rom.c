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
** nes_rom.c
**
** NES ROM loading/saving related functions
** $Id: nes_rom.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <nofrendo.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rom.h"
#include "mmc.h"
#include "ppu.h"
#include "nes.h"

static rom_t rom;

#ifdef USE_SRAM_FILE

/* Save battery-backed RAM */
static void rom_savesram(void)
{
   char fn[PATH_MAX + 1];
   FILE *fp;

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.sram_banks > 0)
   {
      snprintf(fn, PATH_MAX, "%s.sav", rom.filename);
      if ((fp = fopen(fn, "wb")))
      {
         fwrite(rom.sram, ROM_SRAM_BANK_SIZE, rom.sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Wrote battery RAM to %s.\n", fn);
      }
   }
}

/* Load battery-backed RAM from disk */
static void rom_loadsram(void)
{
   char fn[PATH_MAX + 1];
   FILE *fp;

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.sram_banks > 0)
   {
      snprintf(fn, PATH_MAX, "%s.sav", rom.filename);
      if ((fp = fopen(fn, "rb")))
      {
         fread(rom.sram, ROM_SRAM_BANK_SIZE, rom.sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Read battery RAM from %s.\n", fn);
      }
   }
}
#endif

/* Load a ROM from a memory buffer */
rom_t *rom_load_memory(const void *data, size_t length)
{
   return NULL;
}

/* Load a ROM from file */
rom_t *rom_load_file(const char *filename)
{
   inesheader_t header;

   if (!filename)
      return NULL;

   MESSAGE_INFO("ROM: Loading '%s'\n", filename);

   FILE *fp = fopen(filename, "rb");
   if (!fp)
   {
      MESSAGE_ERROR("ROM: Unable to open file\n");
      goto _fail;
   }

   memset(&rom, 0, sizeof(rom_t));

   fseek(fp, 0, SEEK_END);
   rom.size = ftell(fp) - 16;
   fseek(fp, 0, SEEK_SET);
   fread(&header, sizeof(inesheader_t), 1, fp);

   if (memcmp(header.ines_magic, ROM_INES_MAGIC, 4))
   {
      MESSAGE_ERROR("ROM: File is not a valid ROM image\n");
      goto _fail;
   }

   if (rom.size > 0x200000 || header.rom_banks > 0x80)
   {
      MESSAGE_ERROR("ROM: File is too large to be a valid ROM image\n");
      goto _fail;
   }

   if (header.rom_type & ROM_FLAG_TRAINER)
   {
      MESSAGE_INFO("ROM: Reading trainer at $%04X\n", TRAINER_OFFSET);
      fread(rom.sram + TRAINER_OFFSET, TRAINER_LENGTH, 1, fp);
      rom.size -= TRAINER_LENGTH;
   }

   if (!(rom.rom = malloc(rom.size)))
   {
      MESSAGE_ERROR("ROM: Memory allocation failed\n");
      goto _fail;
   }

   if (fread(rom.rom, rom.size, 1, fp) != 1)
   {
      MESSAGE_ERROR("ROM: Read error\n");
      goto _fail;
   }

   fclose(fp);

   strncpy(rom.filename, filename, PATH_MAX);
   rom.checksum = crc32_le(0, rom.rom, rom.size);
   rom.rom_banks = header.rom_banks;
   rom.vrom_banks = header.vrom_banks;
   rom.vram_banks = ROM_VRAM_BANKS;
   rom.sram_banks = ROM_SRAM_BANKS;
   rom.flags = header.rom_type;
   rom.mapper_number = header.rom_type >> 4;

   if (header.reserved2 == 0)
   {
      // https://wiki.nesdev.com/w/index.php/INES
      // A general rule of thumb: if the last 4 bytes are not all zero, and the header is
      // not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits
      // of the mapper number or simply refuse to load the ROM.

      rom.mapper_number |= (header.mapper_hinybble & 0xF0);
   }

   if (rom.vrom_banks)
   {
      rom.vrom = rom.rom + (rom.rom_banks * ROM_PROG_BANK_SIZE);
   }

   MESSAGE_INFO("ROM: Size:   %d\n", rom.size);
   MESSAGE_INFO("ROM: CRC32:  %08X\n", rom.checksum);
   MESSAGE_INFO("ROM: Mapper: %d, PRG:%dK, CHR:%dK, Flags: %c%c%c%c\n",
                rom.mapper_number,
                rom.rom_banks * 16, rom.vrom_banks * 8,
                (rom.flags & ROM_FLAG_VERTICAL) ? 'V' : 'H',
                (rom.flags & ROM_FLAG_BATTERY) ? 'B' : '-',
                (rom.flags & ROM_FLAG_TRAINER) ? 'T' : '-',
                (rom.flags & ROM_FLAG_FOURSCREEN) ? '4' : '-');

#ifdef USE_SRAM_FILE
   rom_loadsram();
#endif

   MESSAGE_INFO("ROM: Loading done.\n");
   return &rom;

_fail:
   if (fp) fclose(fp);
   rom_free();
   return NULL;
}

/* Free a ROM */
void rom_free(void)
{
#ifdef USE_SRAM_FILE
   rom_savesram();
#endif
   free(rom.rom);
}
