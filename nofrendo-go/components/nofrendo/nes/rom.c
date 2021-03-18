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
** nes/rom.c: ROM loader
**
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

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.prg_ram_banks > 0)
   {
      snprintf(fn, PATH_MAX, "%s.sav", rom.filename);
      if ((fp = fopen(fn, "wb")))
      {
         fwrite(rom.prg_ram, ROM_PRG_BANK_SIZE, rom.prg_ram_banks, fp);
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

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.prg_ram_banks > 0)
   {
      snprintf(fn, PATH_MAX, "%s.sav", rom.filename);
      if ((fp = fopen(fn, "rb")))
      {
         fread(rom.prg_ram, ROM_PRG_BANK_SIZE, rom.prg_ram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Read battery RAM from %s.\n", fn);
      }
   }
}
#endif

/* Load a ROM from a memory buffer */
rom_t *rom_loadmem(uint8 *data, size_t size)
{
   if (!data || size < 16)
      return NULL;

   if (!memcmp(data, ROM_INES_MAGIC, 4))
   {
      inesheader_t *header = (inesheader_t *)data;

      MESSAGE_INFO("ROM: Found iNES file of size %d.\n", size);

      memset(&rom, 0, sizeof(rom_t));
      rom.data_ptr = data;
      rom.data_len = size;

      rom.prg_rom = data + sizeof(inesheader_t);

      if (header->rom_type & ROM_FLAG_TRAINER)
      {
         MESSAGE_INFO("ROM: Trainer found and skipped.\n");
         rom.prg_rom += TRAINER_LENGTH;
      }

      rom.checksum = crc32_le(0, rom.prg_rom, size - (rom.prg_rom - data));
      rom.prg_rom_banks = header->prg_banks * 2;
      rom.chr_rom_banks = header->chr_banks;
      rom.prg_ram_banks = 1; // 8KB. Not specified by iNES
      rom.chr_ram_banks = 1; // 8KB. Not specified by iNES
      rom.flags = header->rom_type;
      rom.mapper_number = header->rom_type >> 4;

      if (header->reserved2 == 0)
      {
         // https://wiki.nesdev.com/w/index.php/INES
         // A general rule of thumb: if the last 4 bytes are not all zero, and the header is
         // not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits
         // of the mapper number or simply refuse to load the ROM.
         rom.mapper_number |= (header->mapper_hinybble & 0xF0);
      }

      rom.prg_ram = malloc(rom.prg_ram_banks * ROM_PRG_BANK_SIZE);
      rom.chr_ram = malloc(rom.chr_ram_banks * ROM_CHR_BANK_SIZE);

      if (!rom.prg_ram || !rom.chr_ram)
      {
         MESSAGE_ERROR("ROM: Memory allocation failed!\n");
         return NULL;
      }

      if (rom.chr_rom_banks > 0)
      {
         rom.chr_rom = rom.prg_rom + (rom.prg_rom_banks * ROM_PRG_BANK_SIZE);
      }

      MESSAGE_INFO("ROM: CRC32:  %08X\n", rom.checksum);
      MESSAGE_INFO("ROM: Mapper: %d, PRG:%dK, CHR:%dK, Flags: %c%c%c%c\n",
                  rom.mapper_number,
                  rom.prg_rom_banks * 8, rom.chr_rom_banks * 8,
                  (rom.flags & ROM_FLAG_VERTICAL) ? 'V' : 'H',
                  (rom.flags & ROM_FLAG_BATTERY) ? 'B' : '-',
                  (rom.flags & ROM_FLAG_TRAINER) ? 'T' : '-',
                  (rom.flags & ROM_FLAG_FOURSCREEN) ? '4' : '-');

      strncpy(rom.filename, "filename.nes", PATH_MAX);
      return &rom;
   }
   else if (!memcmp(data, FDS_DISK_MAGIC, 15) || !memcmp(data + 16, FDS_DISK_MAGIC, 15))
   {
      MESSAGE_INFO("ROM: Found FDS file of size %d.\n", size);

      memset(&rom, 0, sizeof(rom_t));
      rom.data_ptr = data;
      rom.data_len = size;

      rom.prg_ram_banks = 4;
      rom.chr_ram_banks = 1;
      rom.prg_rom_banks = 1;

      rom.prg_ram = malloc(0x8000 + 0x2000);
      rom.chr_ram = malloc(0x2000);
      rom.prg_rom = rom.prg_ram + 0x8000;

      if (!rom.prg_ram || !rom.chr_ram)
      {
         MESSAGE_ERROR("ROM: Memory allocation failed!\n");
         return NULL;
      }

      rom.checksum = crc32_le(0, rom.data_ptr, rom.data_len);
      rom.mapper_number = 20;

      MESSAGE_INFO("ROM: CRC32:  %08X\n", rom.checksum);

      strncpy(rom.filename, "filename.fds", PATH_MAX);
      return &rom;
   }

   MESSAGE_ERROR("ROM: File is not a valid NES image!\n");
   return NULL;
}

/* Load a ROM from file */
rom_t *rom_loadfile(const char *filename)
{
   uint8 *data = NULL;
   long size = 0;

   if (!filename)
      return NULL;

   FILE *fp = fopen(filename, "rb");
   if (!fp)
   {
      MESSAGE_ERROR("ROM: Unable to open file '%s'\n", filename);
      return NULL;
   }

   MESSAGE_INFO("ROM: Loading file '%s'\n", filename);

   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   if (size < 16 || size > 0x200000)
   {
      MESSAGE_ERROR("ROM: File size error\n");
   }
   else if ((data = malloc(size)) == NULL)
   {
      MESSAGE_ERROR("ROM: Memory allocation failed\n");
   }
   else if (fread(data, size, 1, fp) != 1)
   {
      MESSAGE_ERROR("ROM: Read error\n");
   }
   else if (rom_loadmem(data, size) == NULL)
   {
      MESSAGE_ERROR("ROM: Load error\n");
   }
   else
   {
      fclose(fp);
      strncpy(rom.filename, filename, PATH_MAX);
      rom.flags |= ROM_FLAG_FREE_DATA;
      #ifdef USE_SRAM_FILE
         rom_loadsram();
      #endif
      return &rom;
   }

   fclose(fp);
   free(data);
   return NULL;
}

/* Free a ROM */
void rom_free(void)
{
#ifdef USE_SRAM_FILE
   rom_savesram();
#endif
   if (rom.flags & ROM_FLAG_FREE_DATA)
   {
      free(rom.data_ptr);
      rom.data_ptr = NULL;
   }
   free(rom.prg_ram);
   rom.prg_ram = NULL;
   free(rom.chr_ram);
   rom.chr_ram = NULL;
}
