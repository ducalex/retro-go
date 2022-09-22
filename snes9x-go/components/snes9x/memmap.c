/* This file is part of Snes9x. See LICENSE file. */

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "ppu.h"
#include "display.h"
#include "cheats.h"
#include "apu.h"
#include "sa1.h"
#include "dsp1.h"
#include "srtc.h"
#include "sdd1.h"
#include "spc7110.h"
#include "seta.h"

#ifdef __W32_HEAP
#include <malloc.h>
#endif

#ifdef _MSC_VER
/* Necessary to build on MSVC */
#define strnicmp _strnicmp
#endif

#define MAP_HIROM_SRAM_OR_NONE (Memory.SRAMSize == 0 ? (uint8_t*) MAP_NONE : (uint8_t*) MAP_HIROM_SRAM)
#define MAP_LOROM_SRAM_OR_NONE (Memory.SRAMSize == 0 ? (uint8_t*) MAP_NONE : (uint8_t*) MAP_LOROM_SRAM)
#define MAP_RONLY_SRAM_OR_NONE (Memory.SRAMSize == 0 ? (uint8_t*) MAP_NONE : (uint8_t*) MAP_RONLY_SRAM)

#include "fxemu.h"
extern FxInit_s SuperFX;

static int32_t retry_count = 0;
static uint8_t bytes0x2000 [0x2000];
static bool is_bsx(uint8_t*);
static bool bs_name(uint8_t*);

void S9xDeinterleaveType2(bool reset);

extern char* rom_filename;

void S9xDeinterleaveType1(int32_t TotalFileSize, uint8_t* base)
{
   int32_t i;
   uint8_t blocks [256];
   uint8_t *tmp    = NULL;
   int32_t nblocks = TotalFileSize >> 16;

   for (i = 0; i < nblocks; i++)
   {
      blocks [i * 2] = i + nblocks;
      blocks [i * 2 + 1] = i;
   }

   /* DS2 DMA notes: base may or may not be 32-byte aligned */
   tmp = (uint8_t*) malloc(0x8000);

   if (tmp)
   {
      for (i = 0; i < nblocks * 2; i++)
      {
         int32_t j;
         for (j = i; j < nblocks * 2; j++)
         {
            if (blocks [j] == i)
            {
               uint8_t b;

               /* memmove converted: Different mallocs [Neb] */
               memcpy(tmp, &base [blocks [j] * 0x8000], 0x8000);
               /* memmove converted: Different addresses, or identical for blocks[i] == blocks[j] [Neb] */
               /* DS2 DMA notes: Don't do DMA at all if blocks[i] == blocks[j] */
               memcpy(&base [blocks [j] * 0x8000],
                      &base [blocks [i] * 0x8000], 0x8000);
               /* memmove converted: Different mallocs [Neb] */
               memcpy(&base [blocks [i] * 0x8000], tmp, 0x8000);
               b          = blocks [j];
               blocks [j] = blocks [i];
               blocks [i] = b;
               break;
            }
         }
      }
      free(tmp);
   }
}

void S9xDeinterleaveGD24(int32_t TotalFileSize, uint8_t* base)
{
   uint8_t *tmp = NULL;
   if (TotalFileSize != 0x300000)
      return;

   /* DS2 DMA notes: base may or may not be 32-byte aligned */
   tmp = (uint8_t*) malloc(0x80000);

   if (tmp)
   {
      /* memmove converted: Different mallocs [Neb] */
      memcpy(tmp, &base[0x180000], 0x80000);
      /* memmove converted: Different addresses [Neb] */
      memcpy(&base[0x180000], &base[0x200000], 0x80000);
      /* memmove converted: Different addresses [Neb] */
      memcpy(&base[0x200000], &base[0x280000], 0x80000);
      /* memmove converted: Different mallocs [Neb] */
      memcpy(&base[0x280000], tmp, 0x80000);
      free(tmp);

      S9xDeinterleaveType1(TotalFileSize, base);
   }
}

static bool AllASCII(uint8_t* b, int32_t size)
{
   int32_t i;
   for (i = 0; i < size; i++)
      if (b[i] < 32 || b[i] > 126)
         return false;
   return true;
}

static int32_t ScoreHiROM(bool skip_header, int32_t romoff)
{
   int32_t score = 0;
   int32_t o = skip_header ? 0xff00 + 0x200 : 0xff00;

   o += romoff;

   if (Memory.ROM [o + 0xd5] & 0x1)
      score += 2;

   /* Mode23 is SA-1 */
   if (Memory.ROM [o + 0xd5] == 0x23)
      score -= 2;

   if (Memory.ROM [o + 0xd4] == 0x20)
      score += 2;

   if ((Memory.ROM [o + 0xdc] + (Memory.ROM [o + 0xdd] << 8) + Memory.ROM [o + 0xde] + (Memory.ROM [o + 0xdf] << 8)) == 0xffff)
   {
      score += 2;
      if (0 != (Memory.ROM [o + 0xde] + (Memory.ROM [o + 0xdf] << 8)))
         score++;
   }

   if (Memory.ROM [o + 0xda] == 0x33)
      score += 2;
   if ((Memory.ROM [o + 0xd5] & 0xf) < 4)
      score += 2;
   if (!(Memory.ROM [o + 0xfd] & 0x80))
      score -= 6;
   if ((Memory.ROM [o + 0xfc] | (Memory.ROM [o + 0xfd] << 8)) > 0xFFB0)
      score -= 2; /* reduced after looking at a scan by Cowering */
   if (Memory.CalculatedSize > 1024 * 1024 * 3)
      score += 4;
   if ((1 << (Memory.ROM [o + 0xd7] - 7)) > 48)
      score -= 1;
   if (!AllASCII(&Memory.ROM [o + 0xb0], 6))
      score -= 1;
   if (!AllASCII(&Memory.ROM [o + 0xc0], ROM_NAME_LEN - 1))
      score -= 1;

   return score;
}

static int32_t ScoreLoROM(bool skip_header, int32_t romoff)
{
   int32_t score = 0;
   int32_t o = skip_header ? 0x7f00 + 0x200 : 0x7f00;

   o += romoff;

   if (!(Memory.ROM [o + 0xd5] & 0x1))
      score += 3;

   /* Mode23 is SA-1 */
   if (Memory.ROM [o + 0xd5] == 0x23)
      score += 2;

   if ((Memory.ROM [o + 0xdc] + (Memory.ROM [o + 0xdd] << 8) + Memory.ROM [o + 0xde] + (Memory.ROM [o + 0xdf] << 8)) == 0xffff)
   {
      score += 2;
      if (0 != (Memory.ROM [o + 0xde] + (Memory.ROM [o + 0xdf] << 8)))
         score++;
   }

   if (Memory.ROM [o + 0xda] == 0x33)
      score += 2;
   if ((Memory.ROM [o + 0xd5] & 0xf) < 4)
      score += 2;
   if (Memory.CalculatedSize <= 1024 * 1024 * 16)
      score += 2;
   if (!(Memory.ROM [o + 0xfd] & 0x80))
      score -= 6;
   if ((Memory.ROM [o + 0xfc] | (Memory.ROM [o + 0xfd] << 8)) > 0xFFB0)
      score -= 2; /* reduced per Cowering suggestion */
   if ((1 << (Memory.ROM [o + 0xd7] - 7)) > 48)
      score -= 1;
   if (!AllASCII(&Memory.ROM [o + 0xb0], 6))
      score -= 1;
   if (!AllASCII(&Memory.ROM [o + 0xc0], ROM_NAME_LEN - 1))
      score -= 1;

   return score;
}

static char* Safe(const char* s)
{
   static char* safe = NULL;
   static int32_t safe_len = 0;
   int32_t i;
   int32_t len;

   if (s == NULL)
   {
      if (safe)
      {
         free(safe);
         safe = NULL;
      }
      return NULL;
   }

   len = strlen(s);

   if (!safe || len + 1 > safe_len)
   {
      if (safe)
         free(safe);
      safe = (char*) malloc(safe_len = len + 1);
   }

   for (i = 0; i < len; i++)
   {
      if (s [i] >= 32 && s [i] < 127)
         safe [i] = s[i];
      else
         safe [i] = '?';
   }
   safe [len] = 0;
   return safe;
}

/**********************************************************************************************/
/* S9xInitMemory()                                                                                     */
/* This function allocates and zeroes all the memory needed by the emulator                   */
/**********************************************************************************************/
bool S9xInitMemory(void)
{
   /* DS2 DMA notes: These would do well to be allocated with 32 extra bytes
      so they can be 32-byte aligned. [Neb] */
   Memory.RAM   = (uint8_t*) calloc(0x20000, 1);
   Memory.SRAM  = (uint8_t*) calloc(0x20000, 1);
   Memory.VRAM  = (uint8_t*) calloc(0x10000, 1);
   Memory.BSRAM = (uint8_t*) calloc(0x80000, 1);
   /* Don't bother initializing ROM, we will load a game anyway. */
#ifdef DS2_DMA
   Memory.ROM   = (uint8_t*) AlignedMalloc(MAX_ROM_SIZE + 0x200 + 0x8000, 32, &PtrAdj.ROM);
#else
   Memory.ROM   = (uint8_t*) malloc(MAX_ROM_SIZE + 0x200 + 0x8000);
#endif
   Memory.FillRAM = NULL;

   IPPU.TileCache [TILE_2BIT] = (uint8_t*) calloc(MAX_2BIT_TILES, 128);
   IPPU.TileCache [TILE_4BIT] = (uint8_t*) calloc(MAX_4BIT_TILES, 128);
   IPPU.TileCache [TILE_8BIT] = (uint8_t*) calloc(MAX_8BIT_TILES, 128);

   IPPU.TileCached [TILE_2BIT] = (uint8_t*) calloc(MAX_2BIT_TILES, 1);
   IPPU.TileCached [TILE_4BIT] = (uint8_t*) calloc(MAX_4BIT_TILES, 1);
   IPPU.TileCached [TILE_8BIT] = (uint8_t*) calloc(MAX_8BIT_TILES, 1);

   if (!Memory.RAM || !Memory.SRAM || !Memory.VRAM || !Memory.ROM || !Memory.BSRAM || !IPPU.TileCache [TILE_2BIT] || !IPPU.TileCache [TILE_4BIT] || !IPPU.TileCache [TILE_8BIT] || !IPPU.TileCached [TILE_2BIT] || !IPPU.TileCached [TILE_4BIT] ||  !IPPU.TileCached [TILE_8BIT])
   {
      S9xDeinitMemory();
      return false;
   }

   /* FillRAM uses first 32K of ROM image area, otherwise space just
      wasted. Might be read by the SuperFX code. */
   Memory.FillRAM = Memory.ROM;

   /* Add 0x8000 to ROM image pointer to stop SuperFX code accessing
      unallocated memory (can cause crash on some ports). */
   Memory.ROM += 0x8000; /* still 32-byte aligned */

   SuperFX.pvRegisters = &Memory.FillRAM [0x3000];
   SuperFX.nRamBanks = 2; /* Most only use 1.  1 = 64KB, 2 = 128KB = 1024Mb */
   SuperFX.pvRam = Memory.SRAM;
   SuperFX.nRomBanks = (2 * 1024 * 1024) / (32 * 1024);
   SuperFX.pvRom = (uint8_t*) Memory.ROM;

   return true;
}

void S9xDeinitMemory(void)
{
   int t;
   if (Memory.RAM)
   {
      free(Memory.RAM);
      Memory.RAM = NULL;
   }
   if (Memory.SRAM)
   {
      free(Memory.SRAM);
      Memory.SRAM = NULL;
   }
   if (Memory.VRAM)
   {
      free(Memory.VRAM);
      Memory.VRAM = NULL;
   }
   if (Memory.ROM)
   {
      Memory.ROM -= 0x8000;
#ifdef DS2_RAM
      AlignedFree(ROM, PtrAdj.ROM);
#else
      free(Memory.ROM);
#endif
      Memory.ROM = NULL;
   }

   if (Memory.BSRAM)
   {
      free(Memory.BSRAM);
      Memory.BSRAM = NULL;
   }

   for (t = 0; t <= TILE_8BIT; t++)
   {
      if (IPPU.TileCache[t])
      {
         free(IPPU.TileCache[t]);
         IPPU.TileCache[t] = NULL;
      }
      if (IPPU.TileCached[t])
      {
         free(IPPU.TileCached[t]);
         IPPU.TileCached[t] = NULL;
      }
   }

   /* Ensure that we free the static char
    * array allocated by Safe() */
   Safe(NULL);
}

#ifndef LOAD_FROM_MEMORY
static void _makepath(char* path,
      const char* dir, const char* fname, const char* ext)
{
   if (dir && *dir)
   {
      strcpy(path, dir);
      strcat(path, "/");
   }
   else
      *path = 0;

   if (fname)
      strcat(path, fname);

   if (ext && *ext)
   {
      strcat(path, ".");
      strcat(path, ext);
   }
}

static void _splitpath (const char* path,
      char* dir, char* fname, char* ext)
{
   const char* slash = strrchr(path, '/');
   const char* dot   = strrchr(path, '.');

   if (!slash)
      slash = strrchr((char*)path, '\\');

   if (dot && slash && dot < slash)
      dot = NULL;

   if (!slash)
   {
      *dir = 0;
      strcpy(fname, path);
      if (dot)
      {
         fname[dot - path] = 0;
         strcpy(ext, dot + 1);
      }
      else
         *ext = 0;
   }
   else
   {
      strcpy(dir, path);
      dir[slash - path] = 0;
      strcpy(fname, slash + 1);
      if (dot)
      {
         fname[dot - slash - 1] = 0;
         strcpy(ext, dot + 1);
      }
      else
         *ext = 0;
   }
}

/* Read variable size MSB int from a file */
static int32_t ReadInt(RFILE* f, uint32_t nbytes)
{
   int32_t v = 0;
   while (nbytes--)
   {
      int32_t c = filestream_getc(f);
      if (c == EOF)
         return -1;
      v = (v << 8) | (c & 0xFF);
   }
   return v;
}

static const char* S9xGetFilename(const char* in)
{
   static char filename [PATH_MAX + 1];
   char dir [_MAX_DIR + 1];
   char fname [_MAX_FNAME + 1];
   char ext [_MAX_EXT + 1];
   _splitpath(Memory.ROMFilename, dir, fname, ext);
   _makepath(filename, dir, fname, in);
   return filename;
}

#define IPS_EOF 0x00454F46l

static void CheckForIPSPatch(const char* rom_filename, bool header, int32_t* rom_size)
{
   char  dir [_MAX_DIR + 1];
   char  name [_MAX_FNAME + 1];
   char  ext [_MAX_EXT + 1];
   char  fname [_MAX_PATH + 1];
   RFILE *patch_file = NULL;
   int32_t offset = header ? 512 : 0;

   _splitpath(rom_filename, dir, name, ext);
   _makepath(fname, dir, name, "ips");

   patch_file = filestream_open(fname, RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!patch_file)
   {
      patch_file = filestream_open(S9xGetFilename("ips"),
            RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

      if (!patch_file)
         return;
   }

   if (filestream_read(patch_file, fname, 5) != 5 ||
       strncmp(fname, "PATCH", 5) != 0)
   {
      filestream_close(patch_file);
      return;
   }

   int32_t ofs;

   for (;;)
   {
      int32_t len;
      int32_t rlen;
      int32_t rchar;

      ofs = ReadInt(patch_file, 3);
      if (ofs == -1)
         goto err_eof;

      if (ofs == IPS_EOF)
         break;

      ofs -= offset;

      len = ReadInt(patch_file, 2);
      if (len == -1)
         goto err_eof;

      /* Apply patch block */
      if (len)
      {
         if (ofs + len > MAX_ROM_SIZE)
            goto err_eof;

         while (len--)
         {
            rchar = filestream_getc(patch_file);
            if (rchar == EOF)
               goto err_eof;
            Memory.ROM [ofs++] = (uint8_t) rchar;
         }
         if (ofs > *rom_size)
            *rom_size = ofs;
      }
      else
      {
         rlen = ReadInt(patch_file, 2);
         if (rlen == -1)
            goto err_eof;

         rchar = filestream_getc(patch_file);
         if (rchar == EOF)
            goto err_eof;

         if (ofs + rlen > MAX_ROM_SIZE)
            goto err_eof;

         while (rlen--)
            Memory.ROM [ofs++] = (uint8_t) rchar;

         if (ofs > *rom_size)
            *rom_size = ofs;
      }
   }

   /* Check if ROM image needs to be truncated */
   ofs = ReadInt(patch_file, 3);
   if (ofs != -1 && ofs - offset < *rom_size)
      *rom_size = ofs - offset; /* Need to truncate ROM image */
   filestream_close(patch_file);
   return;

err_eof:
   if (patch_file)
      filestream_close(patch_file);
}

static uint32_t FileLoader(uint8_t* buffer, const char* filename, int32_t maxsize)
{
   RFILE* ROMFile = NULL;
   int32_t len = 0;

   char dir [_MAX_DIR + 1];
   char name [_MAX_FNAME + 1];
   char ext [_MAX_EXT + 1];
   char fname [_MAX_PATH + 1];

   uint32_t FileSize = 0;

   _splitpath(filename, dir, name, ext);
   _makepath(fname, dir, name, ext);

#ifdef _WIN32
   /* memmove required: Overlapping addresses [Neb] */
   memmove(&ext [0], &ext[1], 4);
#endif

   ROMFile = filestream_open(fname, RETRO_VFS_FILE_ACCESS_READ,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!ROMFile)
      return 0;

   Memory.HeaderCount = 0;
   uint8_t* ptr = buffer;

   {
      int32_t calc_size;
      FileSize = filestream_read(ROMFile, ptr, maxsize + 0x200 - (ptr - Memory.ROM));
      filestream_close(ROMFile);
      calc_size = FileSize & ~0x1FFF; /* round to the lower 0x2000 */

      if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader)
            || Settings.ForceHeader)
      {
         /* memmove required: Overlapping addresses [Neb] */
         /* DS2 DMA notes: Can be split into 512-byte DMA blocks [Neb] */
#ifdef DS2_DMA
         __dcache_writeback_all();
         {
            uint32_t i;
            for (i = 0; i < calc_size; i += 512)
            {
               ds2_DMAcopy_32Byte(2 /* channel: emu internal */, ptr + i, ptr + i + 512, 512);
               ds2_DMA_wait(2);
               ds2_DMA_stop(2);
            }
         }
#else
         memmove(ptr, ptr + 512, calc_size);
#endif
         Memory.HeaderCount++;
         FileSize -= 512;
      }

      ptr           += FileSize;
   }

   return FileSize;
}
#endif

/**********************************************************************************************/
/* LoadROM()                                                                                  */
/* This function loads a Snes-Backup image                                                    */
/**********************************************************************************************/

bool LoadROM(
#ifdef LOAD_FROM_MEMORY
      const struct retro_game_info* game
#else
      const char* filename
#endif
      )
{
   int32_t hi_score, lo_score;
   int32_t TotalFileSize = 0;
   bool Interleaved = false;
   bool Tales = false;
   const uint8_t* src;

   uint8_t* RomHeader = Memory.ROM;
   Memory.ExtendedFormat = NOPE;

   Del7110Gfx();

   memset(bytes0x2000, 0, 0x2000);
   CPU.TriedInterleavedMode2 = false;

   Memory.CalculatedSize = 0;
   retry_count = 0;

again:
#ifdef LOAD_FROM_MEMORY
   Memory.HeaderCount = 0;
   TotalFileSize = game->size;
   src = game->data;
   Memory.HeaderCount = 0;

   if ((((game->size & 0x1FFF) == 0x200) && !Settings.ForceNoHeader) || Settings.ForceHeader)
   {
      TotalFileSize -= 0x200;
      src += 0x200;
      Memory.HeaderCount = 1;

   }

   if (TotalFileSize > MAX_ROM_SIZE)
      return false;

   memcpy(Memory.ROM, src, TotalFileSize);

#else
   TotalFileSize = FileLoader(Memory.ROM, filename, MAX_ROM_SIZE);

   if (!TotalFileSize)
      return false; /* it ends here */
   CheckForIPSPatch(filename, Memory.HeaderCount != 0, &TotalFileSize);
#endif
   /* fix hacked games here. */
   if ((strncmp("HONKAKUHA IGO GOSEI", (char*)&Memory.ROM[0x7FC0], 19) == 0) && (Memory.ROM[0x7FD5] != 0x31))
   {
      Memory.ROM[0x7FD5] = 0x31;
      Memory.ROM[0x7FD6] = 0x02;
   }

#ifndef NO_SPEEDHACKS
   /* SNESAdvance speed hacks (from the speed-hacks branch of CatSFC) */
   if (strncmp("YOSHI'S ISLAND", (char *) &Memory.ROM[0x7FC0], 14) == 0)
   {
      Memory.ROM[0x0000F4] = 0x42;
      Memory.ROM[0x0000F5] = 0x3B;
   }
   else if (strncmp("SUPER MARIOWORLD", (char *) &Memory.ROM[0x7FC0], 16) == 0)
   {
      Memory.ROM[0x00006D] = 0x42;
   }
   else if (strncmp("ALL_STARS + WORLD", (char *) &Memory.ROM[0x7FC0], 17) == 0)
   {
      Memory.ROM[0x0003D0] = 0x42;
      Memory.ROM[0x0003D1] = 0x5B;
      Memory.ROM[0x018522] = 0x42;
      Memory.ROM[0x018523] = 0x5B;
      Memory.ROM[0x02C804] = 0x42;
      Memory.ROM[0x02C805] = 0xBA;
      Memory.ROM[0x0683B5] = 0x42;
      Memory.ROM[0x0683B6] = 0x5B;
      Memory.ROM[0x0696AC] = 0x42;
      Memory.ROM[0x0696AD] = 0xBA;
      Memory.ROM[0x089233] = 0xDB;
      Memory.ROM[0x089234] = 0x61;
      Memory.ROM[0x0895DF] = 0x42;
      Memory.ROM[0x0895E0] = 0x5B;
      Memory.ROM[0x0A7A9D] = 0x42;
      Memory.ROM[0x0A7A9E] = 0xBA;
      Memory.ROM[0x1072E7] = 0x42;
      Memory.ROM[0x1072E8] = 0xD9;
      Memory.ROM[0x107355] = 0x42;
      Memory.ROM[0x107356] = 0x5B;
      Memory.ROM[0x1073CF] = 0x42;
      Memory.ROM[0x1073D0] = 0x5B;
      Memory.ROM[0x107443] = 0x42;
      Memory.ROM[0x107444] = 0x5B;
      Memory.ROM[0x107498] = 0x42;
      Memory.ROM[0x107499] = 0x5B;
      Memory.ROM[0x107505] = 0x42;
      Memory.ROM[0x107506] = 0x5B;
      Memory.ROM[0x107539] = 0x42;
      Memory.ROM[0x10753A] = 0x5B;
      Memory.ROM[0x107563] = 0x42;
      Memory.ROM[0x107564] = 0x5B;
      Memory.ROM[0x18041D] = 0x42;
      Memory.ROM[0x18041E] = 0x79;
   }
#endif

   hi_score = ScoreHiROM(true, 0);
   lo_score = ScoreLoROM(true, 0);

   if (Memory.HeaderCount == 0 && !Settings.ForceNoHeader && strncmp((char *) &Memory.ROM [0], "BANDAI SFC-ADX", 14) && ((hi_score > lo_score && ScoreHiROM(true, 0) > hi_score) || (hi_score <= lo_score && ScoreLoROM(true, 0) > lo_score)))
   {
#ifdef DS2_DMA
      __dcache_writeback_all();
      {
         uint32_t i;
         for (i = 0; i < TotalFileSize; i += 512)
         {
            ds2_DMAcopy_32Byte(2 /* channel: emu internal */, Memory.ROM + i, Memory.ROM + i + 512, 512);
            ds2_DMA_wait(2);
            ds2_DMA_stop(2);
         }
      }
#else
      /* memmove required: Overlapping addresses [Neb] */
      memmove(Memory.ROM, Memory.ROM + 512, TotalFileSize - 512);
#endif
      TotalFileSize -= 512;
   }

   Memory.CalculatedSize = TotalFileSize & ~0x1FFF; /* round down to lower 0x2000 */
   memset(Memory.ROM + Memory.CalculatedSize, 0, MAX_ROM_SIZE - Memory.CalculatedSize);

   if (Memory.CalculatedSize > 0x400000 &&
         !(Memory.ROM[0x7FD5] == 0x32 && ((Memory.ROM[0x7FD6] & 0xF0) == 0x40)) && /* exclude S-DD1 */
         !(Memory.ROM[0xFFD5] == 0x3A && ((Memory.ROM[0xFFD6] & 0xF0) == 0xF0))) /* exclude SPC7110 */
      Memory.ExtendedFormat = YEAH; /* you might be a Jumbo! */

   /* If both vectors are invalid, it's type 1 LoROM */

   if(Memory.ExtendedFormat == NOPE && strncmp((char *) &Memory.ROM[0], "BANDAI SFC-ADX", 14) && ((Memory.ROM[0x7ffc] | (Memory.ROM[0x7ffd] << 8)) < 0x8000) && ((Memory.ROM[0xfffc] | (Memory.ROM[0xFffd] << 8)) < 0x8000) && !Settings.ForceInterleaved)
     S9xDeinterleaveType1(TotalFileSize, Memory.ROM);

   /* CalculatedSize is now set, so rescore */
   hi_score = ScoreHiROM(false, 0);
   lo_score = ScoreLoROM(false, 0);

   if (Memory.ExtendedFormat != NOPE)
   {
      int32_t loromscore, hiromscore, swappedlorom, swappedhirom;
      loromscore = ScoreLoROM(false, 0);
      hiromscore = ScoreHiROM(false, 0);
      swappedlorom = ScoreLoROM(false, 0x400000);
      swappedhirom = ScoreHiROM(false, 0x400000);

      /* set swapped here. */
      if (MAX(swappedlorom, swappedhirom) >= MAX(loromscore, hiromscore))
      {
         Memory.ExtendedFormat = BIGFIRST;
         hi_score = swappedhirom;
         lo_score = swappedlorom;
         RomHeader = Memory.ROM + 0x400000;
      }
      else
      {
         Memory.ExtendedFormat = SMALLFIRST;
         lo_score = loromscore;
         hi_score = hiromscore;
         RomHeader = Memory.ROM;
      }
   }

   Interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;
   if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
   {
      Memory.LoROM = true;
      Memory.HiROM = false;

      /* Ignore map type byte if not 0x2x or 0x3x */
      if ((RomHeader [0x7fd5] & 0xf0) == 0x20 || (RomHeader [0x7fd5] & 0xf0) == 0x30)
      {
         switch (RomHeader [0x7fd5] & 0xf)
         {
         case 1:
            Interleaved = true;
            break;
         case 5:
            Interleaved = true;
            Tales = true;
            break;
         }
      }
   }
   else
   {
      if ((RomHeader [0xffd5] & 0xf0) == 0x20 || (RomHeader [0xffd5] & 0xf0) == 0x30)
      {
         switch (RomHeader [0xffd5] & 0xf)
         {
         case 0:
         case 3:
            Interleaved = true;
            break;
         }
      }
      Memory.LoROM = false;
      Memory.HiROM = true;
   }

   /* More */
   if (!Settings.ForceHiROM &&
       !Settings.ForceLoROM &&
       !Settings.ForceInterleaved &&
       !Settings.ForceInterleaved2 &&
       !Settings.ForceNotInterleaved &&
       !Settings.ForceSuperFX &&
       !Settings.ForceNoSuperFX &&
       !Settings.ForceDSP1 &&
       !Settings.ForceNoDSP1 &&
       !Settings.ForceSA1 &&
       !Settings.ForceNoSA1 &&
       !Settings.ForceC4 &&
       !Settings.ForceNoC4 &&
       !Settings.ForceSDD1 &&
       !Settings.ForceNoSDD1)
   {
      /* スーファミターボ BIOS読み込み */
      if ((strncmp((char*) &Memory.ROM [0], "BANDAI SFC-ADX", 14) == 0) && !(strncmp((char*) &Memory.ROM [0x10], "SFC-ADX BACKUP", 14) == 0))
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = false;
         Tales = false;
      }
      else if (strncmp ((char *) &Memory.ROM [0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 || strncmp ((char *) &Memory.ROM [0x7fc0], "SP MOMOTAROU DENTETSU2", 22) == 0 || strncmp ((char *) &Memory.ROM [0x7fc0], "SUPER FORMATION SOCCE", 21) == 0)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = false;
      }
      /* BS Zooっと麻雀 */
      else if ((strncmp ((char *) &Memory.ROM [0xffc0], "Ｚｏｏっと麻雀！", 16) == 0)|| (strncmp ((char *) &Memory.ROM [0xffc0], "Zooっと麻雀!IVT", 15) == 0))
      {
         Memory.LoROM = false;
         Memory.HiROM = true;
      }
      /* 再BS探偵倶楽部 */
      else if (strncmp ((char *) &Memory.ROM [0xffc0], "再BS探偵倶楽部", 14) == 0)
      {
         Memory.LoROM = false;
         Memory.HiROM = true;
      }
      /* BATMAN--REVENGE JOKER (USA) */
      else if (strncmp ((char *) &Memory.ROM [0xffc0], "BATMAN--REVENGE JOKER", 21) == 0)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = true;
      }
      /* THE DUEL: TEST DRIVE */
      else if (strncmp ((char *) &Memory.ROM [0x7fc0], "THE DUEL: TEST DRIVE", 20) == 0)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = false;
      }
      /* ポパイ いじわる魔女シーハッグの巻 */
      else if (strncmp ((char *) &Memory.ROM [0x7fc0], "POPEYE IJIWARU MAJO", 19) == 0)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = false;
      }
      /* Pop'nツインビー サンプル版 */
      else if(strncmp ((char *) &Memory.ROM [0x7fc0], "POPN TWINBEE", 12) == 0)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
         Interleaved = false;
      }
      /* Mario Early Years: Fun with Numbers */
      else if ((strncmp ((char *) &Memory.ROM [0x7fc0], "MEY Fun with Numbers", 20) == 0))
      {
         int32_t i;
         for (i = 0x87fc0; i < 0x87fe0; i++)
            Memory.ROM [i] = 0;
      }
      else if(Memory.CalculatedSize == 0x100000 && strncmp ((char *) &Memory.ROM [0xffc0], "WWF SUPER WRESTLEMANIA", 22) == 0)
      {
         int32_t cvcount;
         memcpy(&Memory.ROM[0x100000], Memory.ROM, 0x100000);
         for(cvcount = 0; cvcount < 16; cvcount++)
         {
            memcpy(&Memory.ROM[0x8000 * cvcount], &Memory.ROM[0x10000 * cvcount + 0x100000 + 0x8000], 0x8000);
            memcpy(&Memory.ROM[0x8000 * cvcount + 0x80000], &Memory.ROM[0x10000 * cvcount + 0x100000], 0x8000);
         }
         Memory.LoROM = true;
         Memory.HiROM = false;
         memset(&Memory.ROM[Memory.CalculatedSize], 0, MAX_ROM_SIZE - Memory.CalculatedSize);
      }
   }

   if (!Settings.ForceNotInterleaved && Interleaved)
   {
      CPU.TriedInterleavedMode2 = true;

      if (Tales)
      {
         if (Memory.ExtendedFormat == BIGFIRST)
         {
            S9xDeinterleaveType1(0x400000, Memory.ROM);
            S9xDeinterleaveType1(Memory.CalculatedSize - 0x400000, Memory.ROM + 0x400000);
         }
         else
         {
            S9xDeinterleaveType1(Memory.CalculatedSize - 0x400000, Memory.ROM);
            S9xDeinterleaveType1(0x400000, Memory.ROM + Memory.CalculatedSize - 0x400000);
         }

         Memory.LoROM = false;
         Memory.HiROM = true;
      }
      else if (Settings.ForceInterleaved2)
         S9xDeinterleaveType2(false);
      else if (Memory.CalculatedSize == 0x300000)
      {
         bool t = Memory.LoROM;

         Memory.LoROM = Memory.HiROM;
         Memory.HiROM = t;
         S9xDeinterleaveGD24(Memory.CalculatedSize, Memory.ROM);
      }
      else
      {
         bool t = Memory.LoROM;

         Memory.LoROM = Memory.HiROM;
         Memory.HiROM = t;

         S9xDeinterleaveType1(Memory.CalculatedSize, Memory.ROM);
      }

      hi_score = ScoreHiROM(false, 0);
      lo_score = ScoreLoROM(false, 0);

      if ((Memory.HiROM && (lo_score >= hi_score || hi_score < 0)) || (Memory.LoROM && (hi_score > lo_score || lo_score < 0)))
      {
         if (retry_count == 0)
         {
            Settings.ForceNotInterleaved = true;
            Settings.ForceInterleaved = false;
            retry_count++;
            goto again;
         }
      }
   }

   if (Memory.ExtendedFormat == SMALLFIRST)
      Tales = true;

   InitROM(Tales);
   S9xInitCheatData();
   S9xApplyCheats();
   S9xReset();
   return true;
}

/* compatibility wrapper */
void S9xDeinterleaveMode2(void)
{
   S9xDeinterleaveType2(true);
}

void S9xDeinterleaveType2(bool reset)
{
   uint32_t TmpAdj;
   uint8_t *tmp;
   uint8_t blocks [256];
   int32_t i;
   int32_t nblocks = Memory.CalculatedSize >> 16;
   int32_t step = 64;

   while (nblocks <= step)
      step >>= 1;

   nblocks = step;

   for (i = 0; i < nblocks * 2; i++)
   {
      blocks [i] = (i & ~0xF) | ((i & 3) << 2) | ((i & 12) >> 2);
   }

#ifdef DS2_DMA
   tmp = (uint8_t*) AlignedMalloc(0x10000, 32, &TmpAdj);
#else
   tmp = (uint8_t*) malloc(0x10000);
#endif

   if (tmp)
   {
#ifdef DS2_DMA
      __dcache_writeback_all();
#endif
      for (i = 0; i < nblocks * 2; i++)
      {
         int32_t j;
         for (j = i; j < nblocks * 2; j++)
         {
            if (blocks [j] == i)
            {
               uint8_t b;
#ifdef DS2_DMA
               ds2_DMAcopy_32Byte(2 /* channel: emu internal */, tmp, &Memory.ROM [blocks [j] * 0x10000], 0x10000);
               ds2_DMA_wait(2);
               ds2_DMA_stop(2);
               ds2_DMAcopy_32Byte(2 /* channel: emu internal */, &Memory.ROM [blocks [j] * 0x10000], &Memory.ROM [blocks [i] * 0x10000], 0x10000);
               ds2_DMA_wait(2);
               ds2_DMA_stop(2);
               ds2_DMAcopy_32Byte(2 /* channel: emu internal */, &Memory.ROM [blocks [i] * 0x10000], tmp, 0x10000);
               ds2_DMA_wait(2);
               ds2_DMA_stop(2);
#else
               /* memmove converted: Different mallocs [Neb] */
               memcpy(tmp, &Memory.ROM [blocks [j] * 0x10000], 0x10000);
               /* memmove converted: Different addresses, or identical if blocks[i] == blocks[j] [Neb] */
               memcpy(&Memory.ROM [blocks [j] * 0x10000], &Memory.ROM [blocks [i] * 0x10000], 0x10000);
               /* memmove converted: Different mallocs [Neb] */
               memcpy(&Memory.ROM [blocks [i] * 0x10000], tmp, 0x10000);
#endif
               b          = blocks [j];
               blocks [j] = blocks [i];
               blocks [i] = b;
               break;
            }
         }
      }
      free(tmp);
      tmp = NULL;
   }
   if (reset)
   {
      InitROM(false);
      S9xReset();
   }
}

void ParseSNESHeader(uint8_t* RomHeader)
{
   if(Settings.BS)
   {
      uint32_t size_count;

      Memory.SRAMSize = 0x05;
      strncpy(Memory.ROMName, (char *) &RomHeader[0x10], 17);
      memset(&Memory.ROMName[0x11], 0, ROM_NAME_LEN - 1 - 17);
      Memory.ROMSpeed = RomHeader [0x28];
      Memory.ROMType = 0xe5;
      Memory.ROMSize = 1;

      for(size_count = 0x800; size_count < Memory.CalculatedSize; size_count <<= 1, ++Memory.ROMSize);
   }
   else
   {
      Memory.SRAMSize = RomHeader [0x28];
      strncpy(Memory.ROMName, (char*) &RomHeader[0x10], ROM_NAME_LEN - 1);
      Memory.ROMSpeed = RomHeader [0x25];
      Memory.ROMType = RomHeader [0x26];
      Memory.ROMSize = RomHeader [0x27];
   }

   Memory.ROMChecksum = RomHeader [0x2e] + (RomHeader [0x2f] << 8);
   Memory.ROMComplementChecksum = RomHeader [0x2c] + (RomHeader [0x2d] << 8);
   Memory.ROMRegion = RomHeader[0x29];
   /* memmove converted: Different mallocs [Neb] */
   memcpy(Memory.ROMId, &RomHeader [0x2], 4);
   if (RomHeader[0x2A] == 0x33)
      /* memmove converted: Different mallocs [Neb] */
      memcpy(Memory.CompanyId, &RomHeader [0], 2);
   else
      sprintf(Memory.CompanyId, "%02X", RomHeader[0x2A]);
}

void InitROM(bool Interleaved)
{
   uint8_t* RomHeader;
   uint32_t sum1 = 0;
   uint32_t sum2 = 0;

   SuperFX.nRomBanks = Memory.CalculatedSize >> 15;
   Settings.MultiPlayer5Master = Settings.MultiPlayer5;
   Settings.MouseMaster = Settings.Mouse;
   Settings.SuperScopeMaster = Settings.SuperScope;
   Settings.DSP1Master = Settings.ForceDSP1;
   Settings.SuperFX = false;
   Settings.DSP = 0;
   Settings.SA1 = false;
   Settings.C4 = false;
   Settings.SDD1 = false;
   Settings.SRTC = false;
   Settings.SPC7110 = false;
   Settings.SPC7110RTC = false;
   Settings.BS = false;
   Settings.OBC1 = false;
   Settings.SETA = false;
   s7r.DataRomSize = 0;
   Memory.CalculatedChecksum = 0;

   RomHeader = Memory.ROM + 0x7FB0;

   if (Memory.ExtendedFormat == BIGFIRST)
      RomHeader += 0x400000;

   if (Memory.HiROM)
      RomHeader += 0x8000;

   if (!Settings.BS)
   {
      Settings.BS = is_bsx(Memory.ROM + 0x7FC0);

      if (Settings.BS)
      {
         Memory.LoROM = true;
         Memory.HiROM = false;
      }
      else
      {
         Settings.BS = is_bsx(Memory.ROM + 0xFFC0);
         if (Settings.BS)
         {
            Memory.HiROM = true;
            Memory.LoROM = false;
         }
      }
   }

   memset(Memory.BlockIsRAM, 0, MEMMAP_NUM_BLOCKS);
   memset(Memory.BlockIsROM, 0, MEMMAP_NUM_BLOCKS);

   memset(Memory.ROMId, 0, 5);
   memset(Memory.CompanyId, 0, 3);

   ParseSNESHeader(RomHeader);

   /* Detect and initialize chips - detection codes are compatible with NSRT */

   /* DSP1/2/3/4 */
   if (Memory.ROMType == 0x03)
   {
      if (Memory.ROMSpeed == 0x30)
         Settings.DSP = 4; /* DSP4 */
      else
         Settings.DSP = 1; /* DSP1 */
   }
   else if (Memory.ROMType == 0x05)
   {
      if (Memory.ROMSpeed == 0x20)
         Settings.DSP = 2; /* DSP2 */
      else if (Memory.ROMSpeed == 0x30 && RomHeader[0x2a] == 0xb2)
         Settings.DSP = 3; /* DSP3 */
      else
         Settings.DSP = 1; /* DSP1 */
   }

   switch (Settings.DSP)
   {
      case 1: /* DSP1 */
         SetDSP = &DSP1SetByte;
         GetDSP = &DSP1GetByte;
         break;
      case 2: /* DSP2 */
         SetDSP = &DSP2SetByte;
         GetDSP = &DSP2GetByte;
         break;
      case 3: /* DSP3 */
         /* SetDSP = &DSP3SetByte; */
         /* GetDSP = &DSP3GetByte; */
         break;
      case 4: /* DSP4 */
         SetDSP = &DSP4SetByte;
         GetDSP = &DSP4GetByte;
         break;
      default:
         SetDSP = NULL;
         GetDSP = NULL;
         break;
   }

   if(!Settings.ForceNoDSP1 && Settings.DSP)
      Settings.DSP1Master = true;

   if (Memory.HiROM)
   {
      /* Enable S-RTC (Real Time Clock) emulation for Dai Kaijyu Monogatari 2 */
      Settings.SRTC = ((Memory.ROMType & 0xf0) >> 4) == 5;

      if (((Memory.ROMSpeed & 0x0F) == 0x0A) && ((Memory.ROMType & 0xF0) == 0xF0))
      {
         Settings.SPC7110 = true;
         if ((Memory.ROMType & 0x0F) == 0x09)
            Settings.SPC7110RTC = true;
      }

      if (Settings.SPC7110)
         SPC7110HiROMMap();
      else if ((Memory.ROMSpeed & ~0x10) == 0x25)
         TalesROMMap(Interleaved);
      else
         HiROMMap();
   }
   else
   {
      Settings.SuperFX = Settings.ForceSuperFX;

      if (Memory.ROMType == 0x25)
         Settings.OBC1 = true;

      /* BS-X BIOS */
      if (Memory.ROMType == 0xE5)
         Settings.BS = true;

      if ((Memory.ROMType & 0xf0) == 0x10)
         Settings.SuperFX = !Settings.ForceNoSuperFX;

      /* OBC1 hack ROM */
      if (strncmp(Memory.ROMName, "METAL COMBAT", 12) == 0 && Memory.ROMType == 0x13 && Memory.ROMSpeed == 0x42)
      {
         Settings.OBC1 = true;
         Settings.SuperFX = Settings.ForceSuperFX;
         Memory.ROMSpeed = 0x30;
      }

      Settings.SDD1 = Settings.ForceSDD1;
      if ((Memory.ROMType & 0xf0) == 0x40)
         Settings.SDD1 = !Settings.ForceNoSDD1;

      if (((Memory.ROMType & 0xF0) == 0xF0) & ((Memory.ROMSpeed & 0x0F) != 5))
      {
         Memory.SRAMSize = 2;
         if ((Memory.ROMType & 0x0F) == 6)
         {
            if (Memory.ROM[0x7FD7] == 0x09)
            {
               Settings.SETA = ST_011;
               SetSETA = &S9xSetST011;
               GetSETA = &S9xGetST011;
            }
            else
            {
               Settings.SETA = ST_010;
               SetSETA = &S9xSetST010;
               GetSETA = &S9xGetST010;
            }
         }
         else
            Settings.SETA = ST_018;
      }
      Settings.C4 = Settings.ForceC4;
      if ((Memory.ROMType & 0xf0) == 0xf0 && (strncmp(Memory.ROMName, "MEGAMAN X", 9) == 0 || strncmp(Memory.ROMName, "ROCKMAN X", 9) == 0))
         Settings.C4 = !Settings.ForceNoC4;

      if (Settings.SETA && Settings.SETA != ST_018)
         SetaDSPMap();
      else if (Settings.SuperFX)
      {
         SuperFXROMMap();
         Settings.MultiPlayer5Master = false;
         Settings.DSP1Master = false;
         Settings.SA1 = false;
         Settings.C4 = false;
         Settings.SDD1 = false;
      }
      else if (Settings.ForceSA1 || (!Settings.ForceNoSA1 && (Memory.ROMSpeed & ~0x10) == 0x23 && (Memory.ROMType & 0xf) > 3 && (Memory.ROMType & 0xf0) == 0x30))
      {
         Settings.SA1 = true;
         Settings.DSP1Master = false;
         Settings.C4 = false;
         Settings.SDD1 = false;
         SA1ROMMap();
      }
      else if ((Memory.ROMSpeed & ~0x10) == 0x25)
         TalesROMMap(Interleaved);
      else if (Memory.ExtendedFormat != NOPE)
         JumboLoROMMap(Interleaved);
      else if (strncmp((char*) &Memory.ROM [0x7fc0], "SOUND NOVEL-TCOOL", 17) == 0 || strncmp((char*) &Memory.ROM [0x7fc0], "DERBY STALLION 96", 17) == 0)
      {
         LoROM24MBSMap();
         Settings.DSP1Master = false;
      }
      else if (strncmp((char*) &Memory.ROM [0x7fc0], "THOROUGHBRED BREEDER3", 21) == 0 || strncmp((char*) &Memory.ROM [0x7fc0], "RPG-TCOOL 2", 11) == 0)
      {
         SRAM512KLoROMMap();
         Settings.DSP1Master = false;
      }
      else if (strncmp((char*) &Memory.ROM [0x7fc0], "DEZAEMON  ", 10) == 0)
      {
         SRAM1024KLoROMMap();
         Settings.DSP1Master = false;
      }
      else if (strncmp((char*) &Memory.ROM [0x7fc0], "ADD-ON BASE CASSETE", 19) == 0)
      {
         strncpy(Memory.ROMName, (char *) &Memory.ROM[0x100010], ROM_NAME_LEN - 1);
         Settings.MultiPlayer5Master = false;
         Settings.MouseMaster = false;
         Settings.SuperScopeMaster = false;
         Settings.DSP1Master = false;
         Memory.SRAMSize = 5;
         SufamiTurboLoROMMap();
      }
      else if ((strncmp((char *) &Memory.ROM [0x7fc0], "ROCKMAN X  ", 11) == 0)|| (strncmp((char *) &Memory.ROM [0x7fc0], "MEGAMAN X  ", 11) == 0)|| (strncmp((char *) &Memory.ROM [0x7fc0], "demon's blazon", 14) == 0)|| (strncmp((char *) &Memory.ROM [0x7fc0], "demon's crest", 13) == 0))
         CapcomProtectLoROMMap();
      else if ((Memory.ROMSpeed & ~0x10) == 0x22 && strncmp(Memory.ROMName, "Super Street Fighter", 20) != 0)
         AlphaROMMap();
      else if (strncmp ((char *) &Memory.ROM [0x7fc0], "HITOMI3", 7) == 0)
      {
         Memory.SRAMSize = 3;
         LoROMMap();
      }
      else
         LoROMMap();
   }

   if (Settings.BS)
      Memory.ROMRegion = 0;

   if (!Memory.CalculatedChecksum)
   {
      int32_t i;
      uint32_t remainder;
      int32_t power2 = 0;
      int32_t sub = 0;
      int32_t size = Memory.CalculatedSize;

      while (size >>= 1)
         power2++;

      size      = 1 << power2;
      remainder = Memory.CalculatedSize - size;

      for (i = 0; i < size; i++)
         sum1 += Memory.ROM [i];

      for (i = 0; i < (int32_t) remainder; i++)
         sum2 += Memory.ROM [size + i];

      if (Settings.BS && Memory.ROMType != 0xE5)
      {
         if (Memory.HiROM)
            for (i = 0; i < 48; i++)
               sub += Memory.ROM[0xffb0 + i];
         else if (Memory.LoROM)
            for (i = 0; i < 48; i++)
               sub += Memory.ROM[0x7fb0 + i];
         sum1 -= sub;
      }

      if (remainder)
         sum1 += sum2 * (size / remainder);

      sum1 &= 0xffff;
      Memory.CalculatedChecksum = sum1;
   }

   if (Settings.ForceNTSC)
      Settings.PAL = false;
   else if (Settings.ForcePAL)
      Settings.PAL = true;
   else
   {
      /* Korea refers to South Korea, which uses NTSC */
      switch (Memory.ROMRegion)
      {
      case 13:
      case 1:
      case 0:
         Settings.PAL = false;
         break;
      default:
         Settings.PAL = true;
         break;
      }
   }
   if (Settings.PAL)
   {
      Settings.FrameTime = Settings.FrameTimePAL;
      Memory.ROMFramesPerSecond = 50;
   }
   else
   {
      Settings.FrameTime = Settings.FrameTimeNTSC;
      Memory.ROMFramesPerSecond = 60;
   }

   Memory.ROMName[ROM_NAME_LEN - 1] = 0;
   if (strlen(Memory.ROMName))
   {
      char* p = Memory.ROMName + strlen(Memory.ROMName) - 1;

      while (p > Memory.ROMName && *(p - 1) == ' ')
         p--;
      *p = 0;
   }

   Memory.SRAMMask = Memory.SRAMSize ? ((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;

#ifndef USE_BLARGG_APU
   IAPU.OneCycle = ONE_APU_CYCLE;
#endif
   Settings.Shutdown = true;
   ResetSpeedMap();
   ApplyROMFixes();
   sprintf(Memory.ROMName, "%s", Safe(Memory.ROMName));
   sprintf(Memory.ROMId, "%s", Safe(Memory.ROMId));
   sprintf(Memory.CompanyId, "%s", Safe(Memory.CompanyId));
   Settings.ForceHeader = Settings.ForceHiROM = Settings.ForceLoROM = Settings.ForceInterleaved = Settings.ForceNoHeader = Settings.ForceNotInterleaved = Settings.ForceInterleaved2 = false;
}

void FixROMSpeed(void)
{
   int32_t c;

   if (CPU.FastROMSpeed == 0)
      CPU.FastROMSpeed = SLOW_ONE_CYCLE;

   for (c = 0x800; c < 0x1000; c++)
      if (c & 0x8 || c & 0x400)
         Memory.MemorySpeed [c] = (uint8_t) CPU.FastROMSpeed;
}

void ResetSpeedMap(void)
{
   int32_t i;
   memset(Memory.MemorySpeed, SLOW_ONE_CYCLE, 0x1000);
   for (i = 0; i < 0x400; i += 0x10)
   {
      Memory.MemorySpeed[i + 2] = Memory.MemorySpeed[0x800 + i + 2] = ONE_CYCLE;
      Memory.MemorySpeed[i + 3] = Memory.MemorySpeed[0x800 + i + 3] = ONE_CYCLE;
      Memory.MemorySpeed[i + 4] = Memory.MemorySpeed[0x800 + i + 4] = ONE_CYCLE;
      Memory.MemorySpeed[i + 5] = Memory.MemorySpeed[0x800 + i + 5] = ONE_CYCLE;
   }
   FixROMSpeed();
}

void map_space(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, uint8_t *data)
{
   uint32_t c, i, p;

   for (c = bank_s; c <= bank_e; c++)
   {
      for (i = addr_s; i <= addr_e; i += 0x1000)
      {
         p = (c << 4) | (i >> 12);
         Memory.Map[p] = data;
         Memory.BlockIsROM[p] = false;
         Memory.BlockIsRAM[p] = true;
      }
   }
}

void map_index(uint32_t bank_s, uint32_t bank_e, uint32_t addr_s, uint32_t addr_e, intptr_t index, int32_t type)
{
   uint32_t c, i, p;
   bool isROM, isRAM;

   isROM = !((type == MAP_TYPE_I_O) || (type == MAP_TYPE_RAM));
   isRAM = !((type == MAP_TYPE_I_O) || (type == MAP_TYPE_ROM));

   for (c = bank_s; c <= bank_e; c++)
   {
      for (i = addr_s; i <= addr_e; i += 0x1000)
      {
         p = (c << 4) | (i >> 12);
         Memory.Map[p] = (uint8_t*) index;
         Memory.BlockIsROM[p] = isROM;
         Memory.BlockIsRAM[p] = isRAM;
      }
   }
}

void WriteProtectROM(void)
{
   int32_t c;

   /* memmove converted: Different mallocs [Neb] */
   memcpy(Memory.WriteMap, Memory.Map, sizeof(Memory.Map));
   for (c = 0; c < 0x1000; c++)
      if (Memory.BlockIsROM [c])
         Memory.WriteMap [c] = (uint8_t*) MAP_NONE;
}

void MapRAM(void)
{
   int32_t c, i;

   if (Memory.LoROM && !Settings.SDD1)
   {
      /* Banks 70->7d and f0->fe 0x0000-0x7FFF, S-RAM */
      for (c = 0; c < 0x0f; c++)
      {
         for (i = 0; i < 8; i++)
         {
            Memory.Map [(c << 4) + 0xF00 + i] = Memory.Map [(c << 4) + 0x700 + i] = MAP_LOROM_SRAM_OR_NONE;
            Memory.BlockIsRAM [(c << 4) + 0xF00 + i] = Memory.BlockIsRAM [(c << 4) + 0x700 + i] = true;
            Memory.BlockIsROM [(c << 4) + 0xF00 + i] = Memory.BlockIsROM [(c << 4) + 0x700 + i] = false;
         }
      }
      if(Memory.CalculatedSize <= 0x200000)
      {
         /* Banks 70->7d 0x8000-0xffff S-RAM */
         for (c = 0; c < 0x0e; c++)
         {
            for(i = 8; i < 16; i++)
            {
               Memory.Map [(c << 4) + 0x700 + i] = MAP_LOROM_SRAM_OR_NONE;
               Memory.BlockIsRAM [(c << 4) + 0x700 + i] = true;
               Memory.BlockIsROM [(c << 4) + 0x700 + i] = false;
            }
         }
      }
   }
   else if(Memory.LoROM && Settings.SDD1)
   {
      /* Banks 70->7d 0x0000-0x7FFF, S-RAM */
      for (c = 0; c < 0x0f; c++)
      {
         for (i = 0; i < 8; i++)
         {
            Memory.Map [(c << 4) + 0x700 + i] = MAP_LOROM_SRAM_OR_NONE;
            Memory.BlockIsRAM [(c << 4) + 0x700 + i] = true;
            Memory.BlockIsROM [(c << 4) + 0x700 + i] = false;
         }
      }
   }
   /* Banks 7e->7f, RAM */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x7e0] = Memory.RAM;
      Memory.Map [c + 0x7f0] = Memory.RAM + 0x10000;
      Memory.BlockIsRAM [c + 0x7e0] = true;
      Memory.BlockIsRAM [c + 0x7f0] = true;
      Memory.BlockIsROM [c + 0x7e0] = false;
      Memory.BlockIsROM [c + 0x7f0] = false;
   }
   WriteProtectROM();
}

void MapExtraRAM(void)
{
   int32_t c;

   /* Banks 7e->7f, RAM */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x7e0] = Memory.RAM;
      Memory.Map [c + 0x7f0] = Memory.RAM + 0x10000;
      Memory.BlockIsRAM [c + 0x7e0] = true;
      Memory.BlockIsRAM [c + 0x7f0] = true;
      Memory.BlockIsROM [c + 0x7e0] = false;
      Memory.BlockIsROM [c + 0x7f0] = false;
   }

   /* Banks 70->73, S-RAM */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x700] = Memory.SRAM;
      Memory.Map [c + 0x710] = Memory.SRAM + 0x8000;
      Memory.Map [c + 0x720] = Memory.SRAM + 0x10000;
      Memory.Map [c + 0x730] = Memory.SRAM + 0x18000;

      Memory.BlockIsRAM [c + 0x700] = true;
      Memory.BlockIsROM [c + 0x700] = false;
      Memory.BlockIsRAM [c + 0x710] = true;
      Memory.BlockIsROM [c + 0x710] = false;
      Memory.BlockIsRAM [c + 0x720] = true;
      Memory.BlockIsROM [c + 0x720] = false;
      Memory.BlockIsRAM [c + 0x730] = true;
      Memory.BlockIsROM [c + 0x730] = false;
   }
}

void LoROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      if (Settings.SETA == ST_018)
         Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_SETA_RISC;
      else
         Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      if (Settings.C4)
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_C4;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_C4;
      }
      else if(Settings.OBC1)
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_OBC_RAM;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_OBC_RAM;
      }
      else
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) bytes0x2000 - 0x6000;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) bytes0x2000 - 0x6000;
      }

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 8; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 11) % Memory.CalculatedSize];

      for (i = c + 8; i < c + 16; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
   }

   if (Settings.DSP)
      DSPMap();

   MapRAM();
   WriteProtectROM();
}

void DSPMap(void)
{
   switch (Settings.DSP)
   {
      case 1:
         if (Memory.HiROM)
         {
            map_index(0x00, 0x1f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
            map_index(0x80, 0x9f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
            break;
         }
         else if (Memory.CalculatedSize > 0x100000)
         {
            map_index(0x60, 0x6f, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
            map_index(0xe0, 0xef, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
            break;
         }
         else
         {
            map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
            map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
            break;
         }
      case 2:
         map_index(0x20, 0x3f, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
         map_index(0x20, 0x3f, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
         map_index(0xa0, 0xbf, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
         map_index(0xa0, 0xbf, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
         break;
      case 3:
         map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
         map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
         break;
      case 4:
         map_index(0x30, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
         map_index(0xb0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
         break;
   }
}

void SetaDSPMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) bytes0x2000 - 0x6000;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) bytes0x2000 - 0x6000;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c + 8; i < c + 16; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;

      /* only upper half is ROM */
      for (i = c + 8; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
   }

   memset(Memory.SRAM, 0, 0x1000);
   for (c = 0x600; c < 0x680; c += 0x10)
   {
      for (i = 0; i < 0x08; i++)
      {
         /* Where does the SETA chip access, anyway? Please confirm this. */
         Memory.Map[c + 0x80 + i] = (uint8_t*)MAP_SETA_DSP;
         Memory.BlockIsROM [c + 0x80 + i] = false;
         Memory.BlockIsRAM [c + 0x80 + i] = true;
      }

      for (i = 0; i < 0x04; i++)
      {
         /* and this! */
         Memory.Map[c + i] = (uint8_t*)MAP_SETA_DSP;
         Memory.BlockIsROM [c + i] = false;
      }
   }

   MapRAM();
   WriteProtectROM();
}

void HiROMMap(void)
{
   int32_t i;
   int32_t c;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM. */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [0x306 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [0x307 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [0xb06 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [0xb07 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.BlockIsRAM [0x306 + (c << 4)] = true;
      Memory.BlockIsRAM [0x307 + (c << 4)] = true;
      Memory.BlockIsRAM [0xb06 + (c << 4)] = true;
      Memory.BlockIsRAM [0xb07 + (c << 4)] = true;
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 16; i++)
      {
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   if (Settings.DSP)
      DSPMap();

   MapRAM();
   WriteProtectROM();
}

void TalesROMMap(bool Interleaved)
{
   int32_t c;
   int32_t i;
   int32_t sum = 0;

   uint32_t OFFSET0 = 0x400000;
   uint32_t OFFSET1 = 0x400000;
   uint32_t OFFSET2 = 0x000000;

   if (Interleaved)
   {
      OFFSET0 = 0x000000;
      OFFSET1 = 0x000000;
      OFFSET2 = Memory.CalculatedSize - 0x400000; /* changed to work with interleaved DKJM2. */
   }

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;

      /* makes more sense to map the range here. */
      /* ToP seems to use sram to skip intro??? */
      if (c >= 0x200)
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = MAP_HIROM_SRAM_OR_NONE;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = MAP_HIROM_SRAM_OR_NONE;
         Memory.BlockIsRAM [6 + c] = Memory.BlockIsRAM [7 + c] = Memory.BlockIsRAM [0x806 + c] = Memory.BlockIsRAM [0x807 + c] = true;
      }
      else
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;
      }
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = &Memory.ROM [((c << 12) % (Memory.CalculatedSize - 0x400000)) + OFFSET0];
         Memory.Map [i + 0x800] = &Memory.ROM [((c << 12) % 0x400000) + OFFSET2];
         Memory.BlockIsROM [i] = true;
         Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 8; i++)
      {
         Memory.Map [i + 0x400] = &Memory.ROM [((c << 12) % (Memory.CalculatedSize - 0x400000)) + OFFSET1];
         Memory.Map [i + 0x408] = &Memory.ROM [((c << 12) % (Memory.CalculatedSize - 0x400000)) + OFFSET1];
         Memory.Map [i + 0xc00] = &Memory.ROM [((c << 12) % 0x400000) + OFFSET2];
         Memory.Map [i + 0xc08] = &Memory.ROM [((c << 12) % 0x400000) + OFFSET2];
         Memory.BlockIsROM [i + 0x400] = true;
         Memory.BlockIsROM [i + 0x408] = true;
         Memory.BlockIsROM [i + 0xc00] = true;
         Memory.BlockIsROM [i + 0xc08] = true;
      }
   }

   Memory.ROMChecksum = *(Memory.Map[8] + 0xFFDE) + (*(Memory.Map[8] + 0xFFDF) << 8);
   Memory.ROMComplementChecksum = *(Memory.Map[8] + 0xFFDC) + (*(Memory.Map[8] + 0xFFDD) << 8);

   for (i = 0x40; i < 0x80; i++)
   {
      uint8_t* bank_low = (uint8_t*)Memory.Map[i << 4];
      uint8_t* bank_high = (uint8_t*)Memory.Map[(i << 4) + 0x800];
      for (c = 0; c < 0x10000; c++)
      {
         sum += bank_low[c];
         sum += bank_high[c];
      }
   }

   Memory.CalculatedChecksum = sum & 0xFFFF;
   MapRAM();
   WriteProtectROM();
}

void AlphaROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = true;
      }
   }

   /* Banks 40->7f and c0->ff */

   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 16; i++)
      {
         Memory.Map [i + 0x400] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.Map [i + 0xc00] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   MapRAM();
   WriteProtectROM();
}

void DetectSuperFxRamSize(void)
{
   if (Memory.ROM[0x7FDA] == 0x33)
      Memory.SRAMSize = Memory.ROM[0x7FBD];
   else if (strncmp(Memory.ROMName, "STAR FOX 2", 10) == 0)
      Memory.SRAMSize = 6;
   else
      Memory.SRAMSize = 5;
}

void SuperFXROMMap(void)
{
   int32_t c;
   int32_t i;

   DetectSuperFxRamSize();

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [0x006 + c] = Memory.Map [0x806 + c] = (uint8_t*) Memory.SRAM - 0x6000;
      Memory.Map [0x007 + c] = Memory.Map [0x807 + c] = (uint8_t*) Memory.SRAM - 0x6000;
      Memory.BlockIsRAM [0x006 + c] = Memory.BlockIsRAM [0x007 + c] = Memory.BlockIsRAM [0x806 + c] = Memory.BlockIsRAM [0x807 + c] = true;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 16; i++)
      {
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   /* Banks 7e->7f, RAM */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x7e0] = Memory.RAM;
      Memory.Map [c + 0x7f0] = Memory.RAM + 0x10000;
      Memory.BlockIsRAM [c + 0x7e0] = true;
      Memory.BlockIsRAM [c + 0x7f0] = true;
      Memory.BlockIsROM [c + 0x7e0] = false;
      Memory.BlockIsROM [c + 0x7f0] = false;
   }

   /* Banks 70->71, S-RAM */
   for (c = 0; c < 32; c++)
   {
      Memory.Map [c + 0x700] = Memory.SRAM + (((c >> 4) & 1) << 16);
      Memory.BlockIsRAM [c + 0x700] = true;
      Memory.BlockIsROM [c + 0x700] = false;
   }

   /* Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
      block is repeated twice in each 64K block. */
#ifdef DS2_DMA
   __dcache_writeback_all();
#endif
   for (c = 0; c < 64; c++)
   {
#ifdef DS2_DMA
      ds2_DMAcopy_32Byte(2 /* channel: emu internal */, &ROM [0x200000 + c * 0x10000], &ROM [c * 0x8000], 0x8000);
      ds2_DMAcopy_32Byte(3 /* channel: emu internal 2 */, &ROM [0x208000 + c * 0x10000], &ROM [c * 0x8000], 0x8000);
      ds2_DMA_wait(2);
      ds2_DMA_wait(3);
      ds2_DMA_stop(2);
      ds2_DMA_stop(3);
#else
      /* memmove converted: Different addresses [Neb] */
      memcpy(&Memory.ROM [0x200000 + c * 0x10000], &Memory.ROM [c * 0x8000], 0x8000);
      /* memmove converted: Different addresses [Neb] */
      memcpy(&Memory.ROM [0x208000 + c * 0x10000], &Memory.ROM [c * 0x8000], 0x8000);
#endif
   }

   WriteProtectROM();
}

void SA1ROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) &Memory.FillRAM [0x3000] - 0x3000;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_BWRAM;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_BWRAM;
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 16; i++)
         Memory.Map [i + 0x400] = (uint8_t*) &Memory.SRAM [(c << 12) & 0x1ffff];

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = false;
   }

   /* c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c;  i < c + 16; i++)
      {
         Memory.Map [i + 0xc00] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x7e0] = Memory.RAM;
      Memory.Map [c + 0x7f0] = Memory.RAM + 0x10000;
      Memory.BlockIsRAM [c + 0x7e0] = true;
      Memory.BlockIsRAM [c + 0x7f0] = true;
      Memory.BlockIsROM [c + 0x7e0] = false;
      Memory.BlockIsROM [c + 0x7f0] = false;
   }
   WriteProtectROM();

   /* Now copy the map and correct it for the SA1 CPU. */
   /* memmove converted: Different mallocs [Neb] */
   memcpy((void*) SA1.WriteMap, (void*) Memory.WriteMap, sizeof(Memory.WriteMap));
   /* memmove converted: Different mallocs [Neb] */
   memcpy((void*) SA1.Map, (void*) Memory.Map, sizeof(Memory.Map));

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      SA1.Map [c + 0] = SA1.Map [c + 0x800] = &Memory.FillRAM [0x3000];
      SA1.Map [c + 1] = SA1.Map [c + 0x801] = (uint8_t*) MAP_NONE;
      SA1.WriteMap [c + 0] = SA1.WriteMap [c + 0x800] = &Memory.FillRAM [0x3000];
      SA1.WriteMap [c + 1] = SA1.WriteMap [c + 0x801] = (uint8_t*) MAP_NONE;
   }

   /* Banks 60->6f */
   for (c = 0; c < 0x100; c++)
      SA1.Map [c + 0x600] = SA1.WriteMap [c + 0x600] = (uint8_t*) MAP_BWRAM_BITMAP;

   Memory.BWRAM = Memory.SRAM;
}

void LoROM24MBSMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x200; c += 16)
   {
      Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i + 0x800] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 8; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize];

      for (i = c + 8; i < c + 16; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
   }

   MapExtraRAM();
   WriteProtectROM();
}

void SufamiTurboLoROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [c << 11] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 8; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize];

      for (i = c + 8; i < c + 16; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
   }

   if (Settings.DSP1Master)
   {
      for (c = 0; c < 0x100; c++)
      {
         Memory.Map [c + 0xe00] = (uint8_t*) MAP_DSP;
         Memory.BlockIsROM [c + 0xe00] = false;
      }
   }

   /* Banks 7e->7f, RAM */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [c + 0x7e0] = Memory.RAM;
      Memory.Map [c + 0x7f0] = Memory.RAM + 0x10000;
      Memory.BlockIsRAM [c + 0x7e0] = true;
      Memory.BlockIsRAM [c + 0x7f0] = true;
      Memory.BlockIsROM [c + 0x7e0] = false;
      Memory.BlockIsROM [c + 0x7f0] = false;
   }

   /* Banks 60->67, S-RAM */
   for (c = 0; c < 0x80; c++)
   {
      Memory.Map [c + 0x600] = MAP_LOROM_SRAM_OR_NONE;
      Memory.BlockIsRAM [c + 0x600] = true;
      Memory.BlockIsROM [c + 0x600] = false;
   }

   WriteProtectROM();
}

void SRAM512KLoROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 8; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize];

      for (i = c + 8; i < c + 16; i++)
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [((c << 11) + 0x200000) % Memory.CalculatedSize] - 0x8000;

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
   }

   MapExtraRAM();
   WriteProtectROM();
}

void SRAM1024KLoROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = Memory.BlockIsRAM [c + 0x400] = Memory.BlockIsRAM [c + 0xc00] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = Memory.BlockIsRAM [c + 0x401] = Memory.BlockIsRAM [c + 0xc01] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = Memory.Map [c + 0x402] = Memory.Map [c + 0xc02] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = Memory.Map [c + 0x403] = Memory.Map [c + 0xc03] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = Memory.Map [c + 0x404] = Memory.Map [c + 0xc04] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = Memory.Map [c + 0x405] = Memory.Map [c + 0xc05] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = Memory.Map [c + 0x406] = Memory.Map [c + 0xc06] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = Memory.Map [c + 0x407] = Memory.Map [c + 0xc07] = (uint8_t*) MAP_NONE;
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   MapExtraRAM();
   WriteProtectROM();
}

void CapcomProtectLoROMMap(void)
{
   int32_t c;
   int32_t i;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.Map [c + 0x400] = Memory.Map [c + 0xc00] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.Map [c + 0x401] = Memory.Map [c + 0xc01] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = Memory.BlockIsRAM [c + 0x400] = Memory.BlockIsRAM [c + 0xc00] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = Memory.BlockIsRAM [c + 0x401] = Memory.BlockIsRAM [c + 0xc01] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = Memory.Map [c + 0x402] = Memory.Map [c + 0xc02] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = Memory.Map [c + 0x403] = Memory.Map [c + 0xc03] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = Memory.Map [c + 0x404] = Memory.Map [c + 0xc04] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = Memory.Map [c + 0x405] = Memory.Map [c + 0xc05] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 6] = Memory.Map [c + 0x806] = Memory.Map [c + 0x406] = Memory.Map [c + 0xc06] = (uint8_t*) MAP_NONE;
      Memory.Map [c + 7] = Memory.Map [c + 0x807] = Memory.Map [c + 0x407] = Memory.Map [c + 0xc07] = (uint8_t*) MAP_NONE;
      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 11) % Memory.CalculatedSize] - 0x8000;
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   MapRAM();
   WriteProtectROM();
}

void JumboLoROMMap(bool Interleaved)
{
   int32_t c;
   int32_t i;
   int32_t sum = 0, k, l;

   uint32_t OFFSET0 = 0x400000;
   uint32_t OFFSET2 = 0x000000;

   if (Interleaved)
   {
      OFFSET0 = 0x000000;
      OFFSET2 = Memory.CalculatedSize - 0x400000; /* changed to work with interleaved DKJM2. */
   }
   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;
      if (Settings.DSP1Master)
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_DSP;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_DSP;
      }
      else if (Settings.C4)
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) MAP_C4;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) MAP_C4;
      }
      else
      {
         Memory.Map [c + 6] = Memory.Map [c + 0x806] = (uint8_t*) bytes0x2000 - 0x6000;
         Memory.Map [c + 7] = Memory.Map [c + 0x807] = (uint8_t*) bytes0x2000 - 0x6000;
      }

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = &Memory.ROM [((c << 11) % (Memory.CalculatedSize - 0x400000)) + OFFSET0] - 0x8000;
         Memory.Map [i + 0x800] = &Memory.ROM [((c << 11) % (0x400000)) + OFFSET2] - 0x8000;
         Memory.BlockIsROM [i + 0x800] = Memory.BlockIsROM [i] = true;
      }
   }

   if (Settings.DSP1Master)
   {
      /* Banks 30->3f and b0->bf */
      for (c = 0x300; c < 0x400; c += 16)
      {
         for (i = c + 8; i < c + 16; i++)
         {
            Memory.Map [i + 0x800] = (uint8_t*) MAP_DSP;
            Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = false;
         }
      }
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0x400; c < 0x800; c += 16)
   {
      /* updated mappings to correct A15 mirroring */
      for (i = c; i < c + 8; i++)
      {
         Memory.Map [i] = &Memory.ROM [((c << 11) % (Memory.CalculatedSize - 0x400000)) + OFFSET0];
         Memory.Map [i + 0x800] = &Memory.ROM [((c << 11) % 0x400000) + OFFSET2];
      }

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = &Memory.ROM [((c << 11) % (Memory.CalculatedSize - 0x400000)) + OFFSET0] - 0x8000;
         Memory.Map [i + 0x800] = &Memory.ROM [((c << 11) % 0x400000) + OFFSET2 ] - 0x8000;
      }

      for (i = c; i < c + 16; i++)
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
   }

   /* ROM type has to be 64 Mbit header! */
   for (k = 0; k < 256; k++)
   {
      uint8_t* bank = 0x8000 + Memory.Map[8 + (k << 4)]; /* use upper half of the banks, and adjust for LoROM. */
      for (l = 0; l < 0x8000; l++)
         sum += bank[l];
   }
   Memory.CalculatedChecksum = sum & 0xFFFF;
   MapRAM();
   WriteProtectROM();
}

void SPC7110HiROMMap(void)
{
   int32_t c;
   int32_t i;
   int32_t sum = 0;

   /* Banks 00->3f and 80->bf */
   for (c = 0; c < 0x400; c += 16)
   {
      Memory.Map [c + 0] = Memory.Map [c + 0x800] = Memory.RAM;
      Memory.BlockIsRAM [c + 0] = Memory.BlockIsRAM [c + 0x800] = true;
      Memory.Map [c + 1] = Memory.Map [c + 0x801] = Memory.RAM;
      Memory.BlockIsRAM [c + 1] = Memory.BlockIsRAM [c + 0x801] = true;

      Memory.Map [c + 2] = Memory.Map [c + 0x802] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 3] = Memory.Map [c + 0x803] = (uint8_t*) MAP_PPU;
      Memory.Map [c + 4] = Memory.Map [c + 0x804] = (uint8_t*) MAP_CPU;
      Memory.Map [c + 5] = Memory.Map [c + 0x805] = (uint8_t*) MAP_CPU;

      Memory.Map [c + 6] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [c + 7] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [c + 0x806] = Memory.Map [c + 0x807] = (uint8_t*) MAP_NONE;

      for (i = c + 8; i < c + 16; i++)
      {
         Memory.Map [i] = Memory.Map [i + 0x800] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i] = Memory.BlockIsROM [i + 0x800] = true;
      }
   }

   /* Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM. */
   for (c = 0; c < 16; c++)
   {
      Memory.Map [0x306 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [0x307 + (c << 4)] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map [0xb06 + (c << 4)] = (uint8_t*) MAP_NONE;
      Memory.Map [0xb07 + (c << 4)] = (uint8_t*) MAP_NONE;
      Memory.BlockIsRAM [0x306 + (c << 4)] = true;
      Memory.BlockIsRAM [0x307 + (c << 4)] = true;
   }

   /* Banks 40->7f and c0->ff */
   for (c = 0; c < 0x400; c += 16)
   {
      for (i = c; i < c + 16; i++)
      {
         Memory.Map [i + 0x400] = Memory.Map [i + 0xc00] = &Memory.ROM [(c << 12) % Memory.CalculatedSize];
         Memory.BlockIsROM [i + 0x400] = Memory.BlockIsROM [i + 0xc00] = true;
      }
   }

   for (c = 0; c < 0x10; c++)
   {
      Memory.Map [0x500 + c] = (uint8_t*)MAP_SPC7110_DRAM;
      Memory.BlockIsROM [0x500 + c] = true;
   }

   for (c = 0; c < 0x100; c++)
   {
      Memory.Map [0xD00 + c] = (uint8_t*) MAP_SPC7110_ROM;
      Memory.Map [0xE00 + c] = (uint8_t*) MAP_SPC7110_ROM;
      Memory.Map [0xF00 + c] = (uint8_t*) MAP_SPC7110_ROM;
      Memory.BlockIsROM [0xD00 + c] = Memory.BlockIsROM [0xE00 + c] = Memory.BlockIsROM [0xF00 + c] = true;

   }
   S9xSpc7110Init();

   for (i = 0; i < (int32_t)Memory.CalculatedSize; i++)
      sum += Memory.ROM[i];

   if (Memory.CalculatedSize == 0x300000)
      sum <<= 1;
   Memory.CalculatedChecksum = sum & 0xFFFF;

   MapRAM();
   WriteProtectROM();
}

void SPC7110Sram(uint8_t newstate)
{
   if (newstate & 0x80)
   {
      Memory.Map[6] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map[7] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map[0x306] = MAP_HIROM_SRAM_OR_NONE;
      Memory.Map[0x307] = MAP_HIROM_SRAM_OR_NONE;
   }
   else
   {
      Memory.Map[6] = MAP_RONLY_SRAM_OR_NONE;
      Memory.Map[7] = MAP_RONLY_SRAM_OR_NONE;
      Memory.Map[0x306] = MAP_RONLY_SRAM_OR_NONE;
      Memory.Map[0x307] = MAP_RONLY_SRAM_OR_NONE;
   }
}

const char* TVStandard(void)
{
   return Settings.PAL ? "PAL" : "NTSC";
}

const char* Speed(void)
{
   return Memory.ROMSpeed & 0x10 ? "120ns" : "200ns";
}

const char* MapType(void)
{
   return Memory.HiROM ? "HiROM" : "LoROM";
}

const char* ROMID(void)
{
   return Memory.ROMId;
}

bool match_na(const char* str)
{
   return strcmp(Memory.ROMName, str) == 0;
}

bool match_id(const char* str)
{
   return strncmp(Memory.ROMId, str, strlen(str)) == 0;
}

void ApplyROMFixes(void)
{
   /*
   HACKS NSRT can fix that we hadn't detected before.
   [14:25:13] <@Nach>     case 0x0c572ef0: So called Hook (US)(2648)
   [14:25:13] <@Nach>     case 0x6810aa95: Bazooka Blitzkreig swapped sizes hack -handled
   [14:25:17] <@Nach>     case 0x61E29C06: The Tick region hack
   [14:25:19] <@Nach>     case 0x1EF90F74: Jikkyou Keiba Simulation Stable Star PAL hack
   [14:25:23] <@Nach>     case 0x4ab225b5: So called Krusty's Super Fun House (E)
   [14:25:25] <@Nach>     case 0x77fd806a: Donkey Kong Country 2 (E) v1.1 bad dump -handled
   [14:25:27] <@Nach>     case 0x340f23e5: Donkey Kong Country 3 (U) copier hack - handled
   */

   /* not MAD-1 compliant */
   if (match_na("WANDERERS FROM YS"))
   {
      int32_t c;
      for (c = 0; c < 0xE0; c++)
      {
         Memory.Map[c + 0x700] = MAP_LOROM_SRAM_OR_NONE;
         Memory.BlockIsROM[c + 0x700] = false;
         Memory.BlockIsRAM[c + 0x700] = true;
      }
      WriteProtectROM();
   }

   if (strncmp(Memory.ROMName, "WAR 2410", 8) == 0)
   {
      Memory.Map [0x005] = (uint8_t*) Memory.RAM;
      Memory.BlockIsRAM [0x005] = true;
      Memory.BlockIsROM [0x005] = false;
   }

   /* NMI hacks */
   CPU.NMITriggerPoint = 4;
   if (match_na("CACOMA KNIGHT"))
      CPU.NMITriggerPoint = 25;

   /* Disabling a speed-up:
      Games which spool sound samples between the SNES and sound CPU using
      H-DMA as the sample is playing. */
   if (match_na("EARTHWORM JIM 2") ||
         match_na("PRIMAL RAGE") ||
         match_na("CLAY FIGHTER") ||
         match_na("ClayFighter 2") ||
         strncasecmp(Memory.ROMName, "MADDEN", 6) == 0 ||
         strncmp(Memory.ROMName, "NHL", 3) == 0 ||
         match_na("WeaponLord") ||
         strncmp(Memory.ROMName, "WAR 2410", 8) == 0)
      Settings.Shutdown = false;

   /* APU timing hacks */

#ifndef USE_BLARGG_APU
   /* Stunt Racer FX */
   if (match_id("CQ  ") ||
         /* Illusion of Gaia */
         strncmp(Memory.ROMId, "JG", 2) == 0 ||
         match_na("GAIA GENSOUKI 1 JPN"))
      IAPU.OneCycle = 13;
   else if (strcmp (Memory.ROMName, "UMIHARAKAWASE") == 0)
      IAPU.OneCycle = 20;
   /* RENDERING RANGER R2 */
   else if (match_id("AVCJ") ||
         /* Mark Davis */
         strncmp(Memory.ROMName, "THE FISHING MASTER", 18) == 0 || /* needs >= actual APU timing. (21 is .002 Mhz slower) */
         /* Star Ocean */
         strncmp(Memory.ROMId, "ARF", 3) == 0 ||
         /* Tales of Phantasia */
         strncmp(Memory.ROMId, "ATV", 3) == 0 ||
         /* Act Raiser 1 & 2 */
         strncasecmp(Memory.ROMName, "ActRaiser", 9) == 0 ||
         /* Soulblazer */
         match_na("SOULBLAZER - 1 USA") ||
         match_na("SOULBLADER - 1") ||
         /* Terranigma */
         strncmp(Memory.ROMId, "AQT", 3) == 0 ||
         /* Robotrek */
         strncmp(Memory.ROMId, "E9 ", 3) == 0 ||
         match_na("SLAP STICK 1 JPN") ||
         /* ZENNIHON PURORESU2 */
         strncmp(Memory.ROMId, "APR", 3) == 0 ||
         /* Bomberman 4 */
         strncmp(Memory.ROMId, "A4B", 3) == 0 ||
         /* UFO KAMEN YAKISOBAN */
         strncmp(Memory.ROMId, "Y7 ", 3) == 0 ||
         strncmp(Memory.ROMId, "Y9 ", 3) == 0 ||
         /* Panic Bomber World */
         strncmp(Memory.ROMId, "APB", 3) == 0 ||
         ((strncmp(Memory.ROMName, "Parlor", 6) == 0 ||
         match_na("HEIWA Parlor!Mini8") ||
         strncmp(Memory.ROMName, "SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!", 21) == 0) &&
         strcmp(Memory.CompanyId, "A0") == 0) ||
         match_na("DARK KINGDOM") ||
         match_na("ZAN3 SFC") ||
         match_na("HIOUDEN") ||
         match_na("\xC3\xDD\xBC\xC9\xB3\xC0") || /* Tenshi no Uta */
         match_na("FORTUNE QUEST") ||
         match_na("FISHING TO BASSING") ||
         strncmp(Memory.ROMName, "TokyoDome '95Battle 7", 21) == 0 ||
         match_na("OHMONO BLACKBASS") ||
         strncmp(Memory.ROMName, "SWORD WORLD SFC", 15) == 0 ||
         match_na("MASTERS") || /* Augusta 2 J */
         match_na("SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0") || /* Kamen Rider */
         strncmp(Memory.ROMName, "LETs PACHINKO(", 14) == 0)  /* A set of BS games */
      IAPU.OneCycle = 15;
#endif

   /* Specific game fixes */
   Settings.StarfoxHack = match_na("STAR FOX") || match_na("STAR WING");
   Settings.WinterGold = match_na("FX SKIING NINTENDO 96") || match_na("DIRT RACER") || Settings.StarfoxHack;
   Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

   /* CPU timing hacks */
   Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * Settings.CyclesPercentage) / 100;

   /* A Couple of HDMA related hacks - Lantus */
   if ((match_na("SFX SUPERBUTOUDEN2")) || (match_na("ALIEN vs. PREDATOR")) || (match_na("STONE PROTECTORS")) || (match_na("SUPER BATTLETANK 2")))
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 130) / 100;
   else if (match_na("HOME IMPROVEMENT"))
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 200) / 100;
   else if (match_id("ASRJ") && Settings.CyclesPercentage == 100)
      /* Street Racer */
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 95) / 100;
   /* Power Rangers Fight */
   else if (strncmp(Memory.ROMId, "A3R", 3) == 0 ||
         /* Clock Tower */
         strncmp(Memory.ROMId, "AJE", 3) == 0)
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 103) / 100;
   else if (strncmp(Memory.ROMId, "A3M", 3) == 0 && Settings.CyclesPercentage == 100)
      /* Mortal Kombat 3. Fixes cut off speech sample */
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;
   else if (match_na("\x0bd\x0da\x0b2\x0d4\x0b0\x0bd\x0de") &&
         Settings.CyclesPercentage == 100)
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 101) / 100;
   else if (match_na("WILD TRAX") || match_na("STAR FOX 2") || match_na("YOSSY'S ISLAND") || match_na("YOSHI'S ISLAND"))
      CPU.TriedInterleavedMode2 = true;
   /* Start Trek: Deep Sleep 9 */
   else if (strncmp(Memory.ROMId, "A9D", 3) == 0 && Settings.CyclesPercentage == 100)
      Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;

   /* SA-1 Speedup settings */
   SA1.WaitAddress = NULL;
   SA1.WaitByteAddress1 = NULL;
   SA1.WaitByteAddress2 = NULL;

   if (Settings.SA1)
   {
      /* Itoi Shigesato no Bass Tsuri No.1 (J) */
      if (match_id("ZBPJ"))
      {
         SA1.WaitAddress = SA1.Map [0x0093f1 >> MEMMAP_SHIFT] + 0x93f1;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x304a;
      }
      /* Daisenryaku Expert WWII (J) */
      else if (match_id("AEVJ"))
      {
         SA1.WaitAddress = SA1.Map [0x0ed18d >> MEMMAP_SHIFT] + 0xd18d;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3000;
      }
      /* Derby Jockey 2 (J) */
      else if (match_id("A2DJ"))
         SA1.WaitAddress = SA1.Map [0x008b62 >> MEMMAP_SHIFT] + 0x8b62;
      /* Dragon Ball Z - Hyper Dimension (J) */
      else if (match_id("AZIJ"))
      {
         SA1.WaitAddress = SA1.Map [0x008083 >> MEMMAP_SHIFT] + 0x8083;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3020;
      }
      /* SD Gundam G NEXT (J) */
      else if (match_id("ZX3J"))
      {
         SA1.WaitAddress = SA1.Map [0x0087f2 >> MEMMAP_SHIFT] + 0x87f2;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x30c4;
      }
      /* Shougi no Hanamichi (J) */
      else if (match_id("AARJ"))
      {
         SA1.WaitAddress = SA1.Map [0xc1f85a >> MEMMAP_SHIFT] + 0xf85a;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x0c64;
         SA1.WaitByteAddress2 = Memory.SRAM + 0x0c66;
      }
      /* Asahi Shinbun Rensai Katou Hifumi Kudan Shougi Shingiryu (J) */
      if (match_id("A23J"))
      {
         SA1.WaitAddress = SA1.Map [0xc25037 >> MEMMAP_SHIFT] + 0x5037;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x0c06;
         SA1.WaitByteAddress2 = Memory.SRAM + 0x0c08;
      }
      /* Taikyoku Igo - Idaten (J) */
      else if (match_id("AIIJ"))
      {
         SA1.WaitAddress = SA1.Map [0xc100be >> MEMMAP_SHIFT] + 0x00be;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x1002;
         SA1.WaitByteAddress2 = Memory.SRAM + 0x1004;
      }
      /* Takemiya Masaki Kudan no Igo Taishou (J) */
      else if (match_id("AITJ"))
         SA1.WaitAddress = SA1.Map [0x0080b7 >> MEMMAP_SHIFT] + 0x80b7;
      /* J. League '96 Dream Stadium (J) */
      else if (match_id("AJ6J"))
         SA1.WaitAddress = SA1.Map [0xc0f74a >> MEMMAP_SHIFT] + 0xf74a;
      /* Jumpin' Derby (J) */
      else if (match_id("AJUJ"))
         SA1.WaitAddress = SA1.Map [0x00d926 >> MEMMAP_SHIFT] + 0xd926;
      /* Kakinoki Shougi (J) */
      else if (match_id("AKAJ"))
         SA1.WaitAddress = SA1.Map [0x00f070 >> MEMMAP_SHIFT] + 0xf070;
      /* Hoshi no Kirby 3 (J), Kirby's Dream Land 3 (U) */
      else if (match_id("AFJJ") || match_id("AFJE"))
      {
         SA1.WaitAddress = SA1.Map [0x0082d4 >> MEMMAP_SHIFT] + 0x82d4;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x72a4;
      }
      /* Hoshi no Kirby - Super Deluxe (J) */
      else if (match_id("AKFJ"))
      {
         SA1.WaitAddress = SA1.Map [0x008c93 >> MEMMAP_SHIFT] + 0x8c93;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x300a;
         SA1.WaitByteAddress2 = Memory.FillRAM + 0x300e;
      }
      /* Kirby Super Star (U) */
      else if (match_id("AKFE"))
      {
         SA1.WaitAddress = SA1.Map [0x008cb8 >> MEMMAP_SHIFT] + 0x8cb8;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x300a;
         SA1.WaitByteAddress2 = Memory.FillRAM + 0x300e;
      }
      /* Super Mario RPG (J), (U) */
      else if (match_id("ARWJ") || match_id("ARWE"))
      {
         SA1.WaitAddress = SA1.Map [0xc0816f >> MEMMAP_SHIFT] + 0x816f;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3000;
      }
      /* Marvelous (J) */
      else if (match_id("AVRJ"))
      {
         SA1.WaitAddress = SA1.Map [0x0085f2 >> MEMMAP_SHIFT] + 0x85f2;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3024;
      }
      /* Harukanaru Augusta 3 - Masters New (J) */
      else if (match_id("AO3J"))
      {
         SA1.WaitAddress = SA1.Map [0x00dddb >> MEMMAP_SHIFT] + 0xdddb;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x37b4;
      }
      /* Jikkyou Oshaberi Parodius (J) */
      else if (match_id("AJOJ"))
         SA1.WaitAddress = SA1.Map [0x8084e5 >> MEMMAP_SHIFT] + 0x84e5;
      /* Super Bomberman - Panic Bomber W (J) */
      else if (match_id("APBJ"))
         SA1.WaitAddress = SA1.Map [0x00857a >> MEMMAP_SHIFT] + 0x857a;
      /* Pebble Beach no Hatou New - Tournament Edition (J) */
      else if (match_id("AONJ"))
      {
         SA1.WaitAddress = SA1.Map [0x00df33 >> MEMMAP_SHIFT] + 0xdf33;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x37b4;
      }
      /* PGA European Tour (U) */
      else if (match_id("AEPE"))
      {
         SA1.WaitAddress = SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3102;
      }
      /* PGA Tour 96 (U) */
      else if (match_id("A3GE"))
      {
         SA1.WaitAddress = SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3102;
      }
      /* Power Rangers Zeo - Battle Racers (U) */
      else if (match_id("A4RE"))
      {
         SA1.WaitAddress = SA1.Map [0x009899 >> MEMMAP_SHIFT] + 0x9899;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3000;
      }
      /* SD F-1 Grand Prix (J) */
      else if (match_id("AGFJ"))
         SA1.WaitAddress = SA1.Map [0x0181bc >> MEMMAP_SHIFT] + 0x81bc;
      /* Saikousoku Shikou Shougi Mahjong (J) */
      else if (match_id("ASYJ"))
      {
         SA1.WaitAddress = SA1.Map [0x00f2cc >> MEMMAP_SHIFT] + 0xf2cc;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x7ffe;
         SA1.WaitByteAddress2 = Memory.SRAM + 0x7ffc;
      }
      /* Shougi Saikyou II (J) */
      else if (match_id("AX2J"))
         SA1.WaitAddress = SA1.Map [0x00d675 >> MEMMAP_SHIFT] + 0xd675;
      /* Mini Yonku Shining Scorpion - Let's & Go!! (J) */
      else if (match_id("A4WJ"))
         SA1.WaitAddress = SA1.Map [0xc048be >> MEMMAP_SHIFT] + 0x48be;
      /* Shin Shougi Club (J) */
      else if (match_id("AHJJ"))
      {
         SA1.WaitAddress = SA1.Map [0xc1002a >> MEMMAP_SHIFT] + 0x002a;
         SA1.WaitByteAddress1 = Memory.SRAM + 0x0806;
         SA1.WaitByteAddress2 = Memory.SRAM + 0x0808;
      }
      /* ｼｮｳｷﾞｻｲｷｮｳ */
      else if (match_id("AMSJ"))
         SA1.WaitAddress = SA1.Map [0x00CD6A >> MEMMAP_SHIFT] + 0xCD6A;
      /* ﾊﾌﾞﾒｲｼﾞﾝﾉｵﾓｼﾛｼｮｳｷﾞ */
      else if (match_id("IL"))
         SA1.WaitAddress = SA1.Map [0x008549 >> MEMMAP_SHIFT] + 0x8549;
      /* MASOUKISHIN */
      else if (match_id("ALXJ"))
      {
         SA1.WaitAddress = SA1.Map [0x00EC9C >> MEMMAP_SHIFT] + 0xEC9C;
         SA1.WaitByteAddress1 = Memory.FillRAM + 0x3072;
      }
      /* SUPER SHOGI3 */
      else if (match_id("A3IJ"))
         SA1.WaitAddress = SA1.Map [0x00F669 >> MEMMAP_SHIFT] + 0xF669;
   }

   /* Other */

   /* Additional game fixes by sanmaiwashi ... */
   /* Gundam Knight Story */
   if (match_na("SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1"))
   {
      bytes0x2000 [0xb18] = 0x4c;
      bytes0x2000 [0xb19] = 0x4b;
      bytes0x2000 [0xb1a] = 0xea;
   }

   if(Settings.BS && Memory.LoROM &&
      match_na("F-ZERO") &&
      Memory.ROMChecksum == 0xb10d &&
      Memory.ROMComplementChecksum == 0x4ef2)
     Memory.ROM[0x7fd0] = 0xFF; /* fix memory pack position bits */
}

/* 7FC0h or FFC0h */
/* FFC0h - FFCFh: CartName */
/* FFD0h        : Memory pack location */
/* FFD1h - FFD5 : 00:00:00:00:00 (??) */
/* FFD6h        : Month 10h, 20h, 30h... */
/* FFD7h        : Day   This byte / 8  low 3bits is unknown. */
/* FFD8h        : ROMSpeed */
/* FFD9h        : Satellaview ROM Type */
/* FFDAh        : Maker ID */
/* FFDBh        : ROM Version */

static bool is_bsx(uint8_t *p) /* p == "0xFFC0" or "0x7FC0" ROM offset pointer */
{
   uint32_t c;
   int32_t i;
   bool b = false;
   bool bb = false;

   /* Satellaview ROM Type */
   if (p[0x19] & 0x4f)
      return false;

   /* Maker ID */
   c = p[0x1a];
   if ((c != 0x33) && (c != 0xff)) /* 0x33 = Manufacturer: Nintendo */
      return false;

   /* Month, Day */
   c = (p[0x17] << 8) | p[0x16];
   if ((c != 0x0000) && (c != 0xffff))
   {
      if ((c & 0x040f) != 0)
         return false;
      if ((c & 0xff) > 0xc0)
         return false;
   }

   /* ROMSpeed */
   c = p[0x18];
   if ((c & 0xce) || ((c & 0x30) == 0))
      return false;

   /* Memory pack location */
   if(p[0x10] == 0)
      return false;

   for(i = 0; i < 8; i++)
   {
      if(p[0x10] & (1 << i))
      {
         if(bb)
            return false;
         b = true;
      }
      else if(b)
         bb = true;
   }

   if ((p[0x15] & 0x03) != 0)
      return false;
   c = p[0x13];
   if ((c != 0x00) && (c != 0xff))
      return false;
   if (p[0x14] != 0x00)
      return false;
   return bs_name(p);
}

static bool bs_name(uint8_t* p)
{
   int32_t lcount;
   for(lcount = 16; lcount > 0; lcount--)
   {
      /* null strings */
      if(*p == 0)
      {
         if(lcount == 16)
            return false;
         p++;
      }
      /* SJIS single byte char */
      else if((*p >= 0x20 && *p <= 0x7f) || (*p >= 0xa0 && *p <= 0xdf))
         p++;
      /* SJIS multi byte char */
      else if(lcount >= 2)
      {
         if(((*p >= 0x81 && *p <= 0x9f) || (*p >= 0xe0 && *p <= 0xfc)) && ((*(p + 1) >= 0x40 && *(p + 1) <= 0x7e) || (*(p + 1) >= 0x80 && *(p + 1) <= 0xfc)))
         {
            p += 2;
            lcount--;
         }
         else
            return false;
      }
      else
         return false;
   }
   return true;
}
