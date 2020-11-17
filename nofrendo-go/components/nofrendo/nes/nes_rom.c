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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nofrendo.h>
#include "nes_rom.h"
#include "nes_mmc.h"
#include "nes_ppu.h"
#include "nes.h"
#include <osd.h>


#ifdef USE_SRAM_FILE

/* Save battery-backed RAM */
static void rom_savesram(rominfo_t *rominfo)
{
   FILE *fp;
   char fn[PATH_MAX + 1];

   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_BATTERY)
   {
      strncpy(fn, rominfo->filename, PATH_MAX + 1);
      osd_newextension(fn, ".sav");

      if ((fp = fopen(fn, "wb")))
      {
         fwrite(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Wrote battery RAM to %s.\n", fn);
      }
   }
}

/* Load battery-backed RAM from disk */
static void rom_loadsram(rominfo_t *rominfo)
{
   FILE *fp;
   char fn[PATH_MAX + 1];

   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_BATTERY)
   {
      strncpy(fn, rominfo->filename, PATH_MAX);
      osd_newextension(fn, ".sav");

      if ((fp = fopen(fn, "rb")))
      {
         fread(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         MESSAGE_INFO("ROM: Read battery RAM from %s.\n", fn);
      }
   }
}
#endif

static int rom_getheader(unsigned char **rom, rominfo_t *rominfo)
{
   inesheader_t head;

   ASSERT(rom);
   ASSERT(*rom);
   ASSERT(rominfo);

   /* Read in the header */
	memcpy(&head, *rom, sizeof(head));
	*rom += sizeof(head);

   if (memcmp(head.ines_magic, ROM_INES_MAGIC, 4))
   {
      MESSAGE_ERROR("ROM: %s is not a valid ROM image\n", rominfo->filename);
      return -1;
   }

   rominfo->rom_banks = head.rom_banks;
   rominfo->vrom_banks = head.vrom_banks;
   /* iNES assumptions */
   rominfo->sram_banks = 8; /* 1kB banks, so 8KB */
   rominfo->vram_banks = 1; /* 8kB banks, so 8KB */
   rominfo->flags = head.rom_type;
   rominfo->mapper_number = head.rom_type >> 4;

   if (head.mapper_hinybble & 1)
      rominfo->flags |= ROM_FLAG_VERSUS;

   if (head.reserved2 == 0)
   {
      // https://wiki.nesdev.com/w/index.php/INES
      // A general rule of thumb: if the last 4 bytes are not all zero, and the header is
      // not marked for NES 2.0 format, an emulator should either mask off the upper 4 bits
      // of the mapper number or simply refuse to load the ROM.

      rominfo->mapper_number |= (head.mapper_hinybble & 0xF0);
   }

   return 0;
}

/* Load a ROM image into memory */
rominfo_t *rom_load(const char *filename)
{
   rominfo_t *rominfo;
   unsigned char *rom, *rom_ptr;
   size_t filesize;

   rominfo = calloc(sizeof(rominfo_t), 1);
   if (NULL == rominfo)
      goto _fail;

   filesize = osd_getromdata(&rom);
   if (NULL == rom)
      goto _fail;

   rom_ptr = rom;

   strncpy(rominfo->filename, filename, sizeof(rominfo->filename) - 1);
   rominfo->checksum = osd_getromcrc();
   // rominfo->checksum = crc32_le(0, rom, filesize);

   MESSAGE_INFO("ROM: Loading '%s'\n", rominfo->filename);
   MESSAGE_INFO("ROM: Size:   %d\n", filesize);
   MESSAGE_INFO("ROM: CRC32:  %08X\n", rominfo->checksum);

   /* Get the header and stick it into rominfo struct */
	if (rom_getheader(&rom_ptr, rominfo))
      goto _fail;

   MESSAGE_INFO("ROM: Header: Mapper:%d, PRG:%dK, CHR:%dK, Mirror:%c, Flags: %c%c%c\n",
      rominfo->mapper_number,
      rominfo->rom_banks * 16, rominfo->vrom_banks * 8,
      (rominfo->flags & ROM_FLAG_VERTICAL) ? 'V' : 'H',
      (rominfo->flags & ROM_FLAG_BATTERY) ? 'B' : '-',
      (rominfo->flags & ROM_FLAG_TRAINER) ? 'T' : '-',
      (rominfo->flags & ROM_FLAG_FOURSCREEN) ? '4' : '-'
   );

   /* iNES format doesn't tell us if we need SRAM, so
   ** we have to always allocate it -- bleh!
   */
   rominfo->sram = calloc(SRAM_BANK_LENGTH, rominfo->sram_banks);
   if (NULL == rominfo->sram)
   {
      MESSAGE_ERROR("ROM: Could not allocate space for battery RAM\n");
      goto _fail;
   }

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
   else
   {
      rominfo->vram = calloc(VRAM_BANK_LENGTH, rominfo->vram_banks);
      if (NULL == rominfo->vram)
      {
         MESSAGE_ERROR("ROM: Could not allocate space for VRAM\n");
         goto _fail;
      }
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
void rom_free(rominfo_t *rominfo)
{
   if (rominfo)
   {
   #ifdef USE_SRAM_FILE
      rom_savesram(rominfo);
   #endif
      if (rominfo->sram) free(rominfo->sram);
      if (rominfo->rom)  free(rominfo->rom);
      if (rominfo->vrom) free(rominfo->vrom);
      if (rominfo->vram) free(rominfo->vram);

      free(rominfo);
   }
}
