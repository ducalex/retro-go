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

#ifdef USE_SRAM_FILE

/* Save battery-backed RAM */
static void rom_savesram(rom_t *rominfo)
{
   FILE *fp;
   char fn[PATH_MAX + 1];

   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_BATTERY)
   {
      strncpy(fn, rominfo->filename, PATH_MAX + 1);
      char *ext = strrchr(fn, '.');
      if (ext)
         strcpy(ext, ".sav");

      if ((fp = fopen(fn, "wb")))
      {
         fwrite(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Wrote battery RAM to %s.\n", fn);
      }
   }
}

/* Load battery-backed RAM from disk */
static void rom_loadsram(rom_t *rominfo)
{
   FILE *fp;
   char fn[PATH_MAX + 1];

   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_BATTERY)
   {
      strncpy(fn, rominfo->filename, PATH_MAX);
      char *ext = strrchr(fn, '.');
      if (ext)
         strcpy(ext, ".sav");

      if ((fp = fopen(fn, "rb")))
      {
         fread(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Read battery RAM from %s.\n", fn);
      }
   }
}
#endif

/* Load a ROM image into memory */
rom_t *rom_load(const char *filename)
{
   rom_t *rominfo = calloc(sizeof(rom_t), 1);
   uint8_t *rom_ptr = osd_getromdata();
   size_t rom_size = osd_getromsize();

   if (!rominfo || !rom_ptr)
      goto _fail;

   strncpy(rominfo->filename, filename, sizeof(rominfo->filename) - 1);

   MESSAGE_INFO("ROM: Loading '%s'\n", rominfo->filename);
   MESSAGE_INFO("ROM: Size:   %d\n", rom_size);

   /* Read in the header */
   memcpy(&rominfo->header, rom_ptr, 16);
   rom_ptr += 16;

   if (memcmp(rominfo->header.ines_magic, ROM_INES_MAGIC, 4))
   {
      MESSAGE_ERROR("ROM: %s is not a valid ROM image\n", rominfo->filename);
      goto _fail;
   }

   rominfo->checksum = crc32_le(0, rom_ptr, rom_size - 16);
   MESSAGE_INFO("ROM: CRC32:  %08X\n", rominfo->checksum);

   /* Assumed values */
   rominfo->sram_banks = 8; /* 1kB banks, so 8KB */
   rominfo->vram_banks = 1; /* 8kB banks, so 8KB */

   /* Valid values */
   rominfo->rom_banks = rominfo->header.rom_banks;
   rominfo->vrom_banks = rominfo->header.vrom_banks;
   rominfo->flags = rominfo->header.rom_type;
   rominfo->mapper_number = rominfo->header.rom_type >> 4;

   if (rominfo->header.reserved2 == 0)
   {
      // https://wiki.nesdev.com/w/index.php/INES
      // A general rule of thumb: if the last 4 bytes are not all zero, and the header is
      // not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits
      // of the mapper number or simply refuse to load the ROM.

      rominfo->mapper_number |= (rominfo->header.mapper_hinybble & 0xF0);
   }

   MESSAGE_INFO("ROM: Header: Mapper:%d, PRG:%dK, CHR:%dK, Mirror:%c, Flags: %c%c%c\n",
                rominfo->mapper_number,
                rominfo->rom_banks * 16, rominfo->vrom_banks * 8,
                (rominfo->flags & ROM_FLAG_VERTICAL) ? 'V' : 'H',
                (rominfo->flags & ROM_FLAG_BATTERY) ? 'B' : '-',
                (rominfo->flags & ROM_FLAG_TRAINER) ? 'T' : '-',
                (rominfo->flags & ROM_FLAG_FOURSCREEN) ? '4' : '-');

   if (rominfo->flags & ROM_FLAG_TRAINER)
   {
      memcpy(rominfo->sram + TRAINER_OFFSET, rom_ptr, TRAINER_LENGTH);
      rom_ptr += TRAINER_LENGTH;
      MESSAGE_INFO("ROM: Read in trainer at $7000\n");
   }

   rominfo->rom = rom_ptr;
   rom_ptr += ROM_BANK_LENGTH * rominfo->rom_banks;

   if (rominfo->vrom_banks)
   {
      rominfo->vrom = rom_ptr;
      rom_ptr += VROM_BANK_LENGTH * rominfo->vrom_banks;
   }

#ifdef USE_SRAM_FILE
   rom_loadsram(rominfo);
#endif

   MESSAGE_INFO("ROM: Loading done.\n");

   return rominfo;

_fail:
   MESSAGE_ERROR("ROM: Loading failed.\n");
   rom_free(rominfo);
   return NULL;
}

/* Free a ROM */
void rom_free(rom_t *rominfo)
{
   if (rominfo)
   {
#ifdef USE_SRAM_FILE
      rom_savesram(rominfo);
#endif
      free(rominfo->rom);
      free(rominfo);
   }
}
