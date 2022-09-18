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
** nes/rom.c: NES file loader (ROM, FDS, NSF, SAV, etc)
**
*/

#include "nes.h"
#include "../database.h"

static rom_t rom;

#ifdef USE_SRAM_FILE

/* Save battery-backed RAM */
static void rom_savesram(void)
{
   char fn[sizeof(rom.filename)+8];
   FILE *fp;

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.prg_ram_banks > 0)
   {
      if ((fp = fopen(strcat(strcpy(fn, rom.filename), ".sav"), "wb")))
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
   char fn[sizeof(rom.filename)+8];
   FILE *fp;

   if ((rom.flags & ROM_FLAG_BATTERY) && rom.prg_ram_banks > 0)
   {
      if ((fp = fopen(strcat(strcpy(fn, rom.filename), ".sav"), "rb")))
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

   rom = (rom_t) {
      .data_ptr = data,
      .data_len = size,
      .system = SYS_UNKNOWN,
      .mirroring = PPU_MIRROR_HORI,
   };

   if (!memcmp(data, ROM_NES_MAGIC, 4))
   {
      inesheader_t *header = (inesheader_t *)data;

      MESSAGE_INFO("ROM: Found iNES file of size %d.\n", size);

      rom.prg_rom = data + sizeof(inesheader_t);

      if (header->rom_type & ROM_FLAG_TRAINER)
      {
         MESSAGE_INFO("ROM: Trainer found and skipped.\n");
         rom.prg_rom += 0x200;
      }

      rom.checksum = crc32_le(0, rom.prg_rom, size - (rom.prg_rom - data));
      rom.prg_rom_banks = header->prg_banks * 2;
      rom.chr_rom_banks = header->chr_banks;
      rom.prg_ram_banks = 1; // 8KB. Not specified by iNES
      rom.chr_ram_banks = 1; // 8KB. Not specified by iNES
      rom.flags = header->rom_type;
      rom.mapper_number = header->rom_type >> 4;

      MESSAGE_INFO("ROM: CRC32:  %08X\n", (unsigned)rom.checksum);

      if (header->reserved2 == 0)
      {
         // https://wiki.nesdev.com/w/index.php/INES
         // A general rule of thumb: if the last 4 bytes are not all zero, and the header is
         // not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits
         // of the mapper number or simply refuse to load the ROM.
         rom.mapper_number |= (header->mapper_hinybble & 0xF0);
      }

      if (rom.flags & ROM_FLAG_FOURSCREEN)
         rom.mirroring = PPU_MIRROR_FOUR;
      else if (rom.flags & ROM_FLAG_VERTICAL)
         rom.mirroring = PPU_MIRROR_VERT;

      const db_game_t *entry = games_database;
      while (entry->crc && entry->crc != rom.checksum)
         entry++;

      if (entry->crc == rom.checksum)
      {
         MESSAGE_INFO("ROM: Game found in database.\n");

         rom.system = entry->system;

         if (entry->mapper != rom.mapper_number)
         {
            MESSAGE_WARN("ROM: mapper mismatch! (DB: %d, ROM: %d)\n", entry->mapper, rom.mapper_number);
            rom.mapper_number = entry->mapper;
         }

         if (entry->mirror != rom.mirroring)
         {
            MESSAGE_WARN("ROM: mirroring mismatch! (DB: %d, ROM: %d)\n", entry->mirror, rom.mirroring);
            rom.mirroring = entry->mirror;
         }

         if (entry->prg_rom != rom.prg_rom_banks)
         {
            MESSAGE_WARN("ROM: prg_rom_banks mismatch! (DB: %d, ROM: %d)\n", entry->prg_rom, rom.prg_rom_banks);
            // rom.prg_rom_banks = entry->prg_rom;
         }

         if (entry->prg_ram != rom.prg_ram_banks)
         {
            MESSAGE_WARN("ROM: prg_ram_banks mismatch! (DB: %d, ROM: %d)\n", entry->prg_ram, rom.prg_ram_banks);
            // rom.prg_rom_banks = entry->prg_ram;
         }

         if (entry->chr_rom > -1 && entry->chr_rom != rom.chr_rom_banks)
         {
            MESSAGE_WARN("ROM: chr_rom_banks mismatch! (DB: %d, ROM: %d)\n", entry->chr_rom, rom.chr_rom_banks);
            // rom.chr_rom_banks = entry->chr_rom;
         }

         if (entry->chr_ram > -1 && entry->chr_ram != rom.chr_ram_banks)
         {
            MESSAGE_WARN("ROM: chr_ram_banks mismatch! (DB: %d, ROM: %d)\n", entry->chr_ram, rom.chr_ram_banks);
            // rom.chr_ram_banks = entry->chr_ram;
         }
      }
      else
      {
         MESSAGE_INFO("ROM: Game not found in database.\n");
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

      MESSAGE_INFO("ROM: Mapper: %d, PRG:%dK, CHR:%dK, Flags: %c%c%c%c\n",
                  rom.mapper_number,
                  rom.prg_rom_banks * 8, rom.chr_rom_banks * 8,
                  (rom.flags & ROM_FLAG_VERTICAL) ? 'V' : 'H',
                  (rom.flags & ROM_FLAG_BATTERY) ? 'B' : '-',
                  (rom.flags & ROM_FLAG_TRAINER) ? 'T' : '-',
                  (rom.flags & ROM_FLAG_FOURSCREEN) ? '4' : '-');

      strcpy(rom.filename, "filename.nes");
      return &rom;
   }
   else if (!memcmp(data, ROM_FDS_MAGIC, 4) || !memcmp(data, ROM_FDS_RAW_MAGIC, 15))
   {
      MESSAGE_INFO("ROM: Found FDS file of size %d.\n", size);

      rom.flags = ROM_FLAG_FDS_DISK|ROM_FLAG_BATTERY;
      rom.prg_ram_banks = 4; // The FDS adapter contains 32KB to store game program
      rom.chr_ram_banks = 1; // The FDS adapter contains 8KB
      rom.prg_rom_banks = 1; // This will contain the FDS BIOS
      rom.checksum = crc32_le(0, rom.data_ptr, rom.data_len);
      rom.mapper_number = 20;
      rom.system = SYS_FAMICOM;

      MESSAGE_INFO("ROM: CRC32:  %08X\n", (unsigned)rom.checksum);

      rom.prg_ram = malloc((rom.prg_ram_banks + rom.prg_rom_banks) * ROM_PRG_BANK_SIZE);
      rom.chr_ram = malloc(rom.chr_ram_banks * ROM_CHR_BANK_SIZE);
      // We do it this way because only rom.prg_ram is freed in rom_free
      rom.prg_rom = rom.prg_ram + (rom.prg_ram_banks * ROM_PRG_BANK_SIZE);

      if (!rom.prg_ram || !rom.chr_ram || !rom.prg_rom)
      {
         MESSAGE_ERROR("ROM: Memory allocation failed!\n");
         return NULL;
      }

      strcpy(rom.filename, "filename.fds");
      return &rom;
   }
   else if (!memcmp(data, ROM_NSF_MAGIC, 5))
   {
      MESSAGE_INFO("ROM: Found NSF file of size %d.\n", size);

      rom.prg_ram_banks = 1; // Some songs may need it. I store a bootstrap program there at the moment
      rom.chr_ram_banks = 1; // Not used but some code might assume it will be present...
      rom.prg_rom_banks = 4; // This is actually PRG-RAM but some of our code assumes PRG-ROM to be present...
      rom.checksum = crc32_le(0, rom.data_ptr, rom.data_len);
      rom.mapper_number = 31;

      MESSAGE_INFO("ROM: CRC32:  %08X\n", (unsigned)rom.checksum);

      rom.prg_ram = malloc((rom.prg_ram_banks + rom.prg_rom_banks) * ROM_PRG_BANK_SIZE);
      rom.chr_ram = malloc(rom.chr_ram_banks * ROM_CHR_BANK_SIZE);
      // We do it this way because only rom.prg_ram is freed in rom_free
      rom.prg_rom = rom.prg_ram + (rom.prg_ram_banks * ROM_PRG_BANK_SIZE);

      if (!rom.prg_ram || !rom.chr_ram || !rom.prg_rom)
      {
         MESSAGE_ERROR("ROM: Memory allocation failed!\n");
         return NULL;
      }

      strcpy(rom.filename, "filename.nsf");
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
      if (rom.system == SYS_UNKNOWN)
      {
         if (strstr(filename, "(E)")
            || strstr(filename, "(Europe)")
            || strstr(filename, "(A)")
            || strstr(filename, "(Australia)"))
            rom.system = SYS_NES_PAL;
      }
      rom.flags |= ROM_FLAG_FREE_DATA;
      // This is fine, rom_loadmem zeroes `rom`.
      strncpy(rom.filename, filename, sizeof(rom.filename) - 1);
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
