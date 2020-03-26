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

/* TODO: make this a generic ROM loading routine */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <noftypes.h>
#include <nofrendo.h>
#include "nes_rom.h"
#include "nes_mmc.h"
#include "nes_ppu.h"
#include "nes.h"
#include <log.h>
#include <osd.h>
#include <rom/crc.h>


/* Max length for displayed filename */
#define  ROM_DISP_MAXLEN   20


#ifdef ZLIB
#include <zlib.h>
#define  _fopen            gzopen
#define  _fclose           gzclose
#define  _fread(B,N,L,F)   gzread((F),(B),(L)*(N))
#else
#define  _fopen            fopen
#define  _fclose           fclose
#define  _fread(B,N,L,F)   fread((B),(N),(L),(F))
#endif

#define  ROM_FOURSCREEN    0x08
#define  ROM_TRAINER       0x04
#define  ROM_BATTERY       0x02
#define  ROM_MIRRORTYPE    0x01
#define  ROM_INES_MAGIC    "NES\x1A"

#define  TRAINER_OFFSET    0x1000
#define  TRAINER_LENGTH    0x200
#define  VRAM_LENGTH       0x2000

#define  ROM_BANK_LENGTH   0x4000
#define  VROM_BANK_LENGTH  0x2000

#define  SRAM_BANK_LENGTH  0x0400
#define  VRAM_BANK_LENGTH  0x2000

/* Save battery-backed RAM */
static void rom_savesram(rominfo_t *rominfo)
{
   FILE *fp;
   char fn[PATH_MAX + 1];

   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_BATTERY)
   {
      strncpy(fn, rominfo->filename, PATH_MAX);
      osd_newextension(fn, ".sav");

      fp = fopen(fn, "wb");
      if (NULL != fp)
      {
         fwrite(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         log_printf("Wrote battery RAM to %s.\n", fn);
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

      fp = fopen(fn, "rb");
      if (NULL != fp)
      {
         fread(rominfo->sram, SRAM_BANK_LENGTH, rominfo->sram_banks, fp);
         fclose(fp);
         log_printf("Read battery RAM from %s.\n", fn);
      }
   }
}

/* Allocate space for SRAM */
static int rom_allocsram(rominfo_t *rominfo)
{
   /* Load up SRAM */
   rominfo->sram = calloc(SRAM_BANK_LENGTH, rominfo->sram_banks);
   if (NULL == rominfo->sram)
   {
      printf("Could not allocate space for battery RAM");
      return -1;
   }
   return 0;
}

/* If there's a trainer, load it in at $7000 */
static void rom_loadtrainer(unsigned char **rom, rominfo_t *rominfo)
{
   ASSERT(rom);
   ASSERT(rominfo);

   if (rominfo->flags & ROM_FLAG_TRAINER)
   {
//      fread(rominfo->sram + TRAINER_OFFSET, TRAINER_LENGTH, 1, fp);
      memcpy(rominfo->sram + TRAINER_OFFSET, *rom, TRAINER_LENGTH);
      *rom += TRAINER_LENGTH;
      log_printf("Read in trainer at $7000\n");
   }
}

static int rom_loadrom(unsigned char **rom, rominfo_t *rominfo)
{
   ASSERT(rom);
   ASSERT(rominfo);

   /* Allocate ROM space, and load it up! */
/*
   rominfo->rom = malloc((rominfo->rom_banks * ROM_BANK_LENGTH));
   if (NULL == rominfo->rom)
   {
      gui_sendmsg(GUI_RED, "Could not allocate space for ROM image");
      return -1;
   }
   _fread(rominfo->rom, ROM_BANK_LENGTH, rominfo->rom_banks, fp);
*/
   rominfo->rom = *rom;
   *rom += ROM_BANK_LENGTH * rominfo->rom_banks;


   /* If there's VROM, allocate and stuff it in */
   if (rominfo->vrom_banks)
   {
/*
      rominfo->vrom = malloc((rominfo->vrom_banks * VROM_BANK_LENGTH));
      if (NULL == rominfo->vrom)
      {
         gui_sendmsg(GUI_RED, "Could not allocate space for VROM");
         return -1;
      }
      _fread(rominfo->vrom, VROM_BANK_LENGTH, rominfo->vrom_banks, fp);
*/
      rominfo->vrom = *rom;
      *rom += VROM_BANK_LENGTH * rominfo->vrom_banks;
   }
   else
   {
      rominfo->vram = calloc(VRAM_LENGTH, 1);
      if (NULL == rominfo->vram)
      {
         printf("Could not allocate space for VRAM");
         return -1;
      }
   }

   return 0;
}

static FILE *rom_findrom(const char *filename, rominfo_t *rominfo)
{
   FILE *fp;

   ASSERT(rominfo);

   if (NULL == filename)
      return NULL;

   /* Make a copy of the name so we can extend it */
   osd_fullname(rominfo->filename, filename);

   fp = _fopen(rominfo->filename, "rb");
   if (NULL == fp)
   {
      /* Didn't find the file?  Maybe the .NES extension was omitted */
      if (NULL == strrchr(rominfo->filename, '.'))
         strncat(rominfo->filename, ".nes", PATH_MAX - strlen(rominfo->filename));

      /* this will either return NULL or a valid file pointer */
      fp = _fopen(rominfo->filename, "rb");
   }

   return fp;
}

/* return 0 if this *is* an iNES file */
int rom_checkmagic(const char *filename)
{
   inesheader_t head;
   rominfo_t rominfo;
   FILE *fp;

   fp = rom_findrom(filename, &rominfo);
   if (NULL == fp)
      return -1;

   _fread(&head, 1, sizeof(head), fp);

   _fclose(fp);

   if (0 == memcmp(head.ines_magic, ROM_INES_MAGIC, 4))
      /* not an iNES file */
      return 0;

   return -1;
}

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
      printf("rom_getheader: %s is not a valid ROM image\n", rominfo->filename);
      return -1;
   }

   rominfo->rom_banks = head.rom_banks;
   rominfo->vrom_banks = head.vrom_banks;
   /* iNES assumptions */
   rominfo->sram_banks = 8; /* 1kB banks, so 8KB */
   rominfo->vram_banks = 1; /* 8kB banks, so 8KB */
   rominfo->mirror = (head.rom_type & ROM_MIRRORTYPE) ? MIRROR_VERT : MIRROR_HORIZ;
   rominfo->flags = 0;
   if (head.rom_type & ROM_BATTERY)
      rominfo->flags |= ROM_FLAG_BATTERY;
   if (head.rom_type & ROM_TRAINER)
      rominfo->flags |= ROM_FLAG_TRAINER;
   if (head.rom_type & ROM_FOURSCREEN)
      rominfo->flags |= ROM_FLAG_FOURSCREEN;
   if (head.mapper_hinybble & 1)
      rominfo->flags |= ROM_FLAG_VERSUS;

   /* TODO: fourscreen a mirroring type? */
   rominfo->mapper_number = head.rom_type >> 4;

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

/* Build the info string for ROM display */
char *rom_getinfo(rominfo_t *rominfo)
{
   static char info[PATH_MAX + 1];
   char romname[PATH_MAX + 1], temp[PATH_MAX + 1];

   /* Look to see if we were given a path along with filename */
   /* TODO: strip extensions */
   if (strrchr(rominfo->filename, PATH_SEP))
      strncpy(romname, strrchr(rominfo->filename, PATH_SEP) + 1, PATH_MAX);
   else
      strncpy(romname, rominfo->filename, PATH_MAX);

   /* If our filename is too long, truncate our displayed filename */
   if (strlen(romname) > ROM_DISP_MAXLEN)
   {
      strncpy(info, romname, ROM_DISP_MAXLEN - 3);
      strcpy(info + (ROM_DISP_MAXLEN - 3), "...");
   }
   else
   {
      strcpy(info, romname);
   }

   sprintf(temp, " [%d] %dk/%dk %c", rominfo->mapper_number,
           rominfo->rom_banks * 16, rominfo->vrom_banks * 8,
           (rominfo->mirror == MIRROR_VERT) ? 'V' : 'H');

   /* Stick it on there! */
   strncat(info, temp, PATH_MAX - strlen(info));

   if (rominfo->flags & ROM_FLAG_BATTERY)
      strncat(info, "B", PATH_MAX - strlen(info));
   if (rominfo->flags & ROM_FLAG_TRAINER)
      strncat(info, "T", PATH_MAX - strlen(info));
   if (rominfo->flags & ROM_FLAG_FOURSCREEN)
      strncat(info, "4", PATH_MAX - strlen(info));

   return info;
}

/* Load a ROM image into memory */
rominfo_t *rom_load(const char *filename)
{
   rominfo_t *rominfo;
   unsigned char *rom;
   size_t filesize;

   rominfo = calloc(sizeof(rominfo_t), 1);
   if (NULL == rominfo)
      goto _fail;

   filesize = osd_getromdata(&rom);
   if (NULL == rom)
      goto _fail;

   strncpy(rominfo->filename, filename, sizeof(rominfo->filename));
   // rominfo->checksum = crc32_le(0, rom + 16, filesize - 16);
   rominfo->checksum = crc32_le(0, rom, filesize);

   printf("rom_load: filename='%s'\n", rominfo->filename);
   printf("rom_load: filesize=%d\n", filesize);
   printf("rom_load: checksum='%8X'\n", rominfo->checksum);

   /* Get the header and stick it into rominfo struct */
	if (rom_getheader(&rom, rominfo))
      goto _fail;

   /* Make sure we really support the mapper */
   if (false == mmc_peek(rominfo->mapper_number))
   {
      nofrendo_notify("Mapper %d not yet implemented", rominfo->mapper_number);
      printf("rom_load: Mapper %d not yet implemented\n", rominfo->mapper_number);
      goto _fail;
   }

   /* iNES format doesn't tell us if we need SRAM, so
   ** we have to always allocate it -- bleh!
   ** UNIF, TAKE ME AWAY!  AAAAAAAAAA!!!
   */
   if (rom_allocsram(rominfo))
      goto _fail;

   rom_loadtrainer(&rom, rominfo);

	if (rom_loadrom(&rom, rominfo))
      goto _fail;

   // rom_loadsram(rominfo);

   nofrendo_notify("ROM loaded: %s", rom_getinfo(rominfo));

   return rominfo;

_fail:
   nofrendo_notify("ROM not loaded.");
   printf("rom_load: Rom loading failed\n");
   rom_free(&rominfo);
   return NULL;
}

/* Free a ROM */
void rom_free(rominfo_t **rominfo)
{
   if (NULL == *rominfo)
      return;

   rom_savesram(*rominfo);

   if ((*rominfo)->sram)
      free((*rominfo)->sram);
   if ((*rominfo)->rom)
      free((*rominfo)->rom);
   if ((*rominfo)->vrom)
      free((*rominfo)->vrom);
   if ((*rominfo)->vram)
      free((*rominfo)->vram);

   free(*rominfo);
}
