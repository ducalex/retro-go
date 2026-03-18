#ifdef TARGET_GNW
#include "build/config.h"
#include "build/mappers.h"
#endif

#if !defined(TARGET_GNW) ||                                                    \
    (defined(TARGET_GNW) && defined(ENABLE_EMULATOR_NES) &&                    \
     FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fceu-endian.h"
#include "fceu-memory.h"
#include "fceu-sound.h"
#include "fceu-types.h"
#include "fceu.h"
#include "general.h"
#include "ppu.h"
#include "x6502.h"
#include <rg_system.h>

#include "cheat.h"
#include "fceu-cart.h"
#include "fceu-state.h"
#include "fds.h"
#include "fds_apu.h"
#include "file.h"
#include "ines.h"
#include "input.h"
#include "nsf.h"
#include "palette.h"
#include "unif.h"
#include "video.h"
#include "vsuni.h"

uint64 timestampbase;

FCEUGI *GameInfo = NULL;
void (*GameInterface)(int h);

void (*GameStateRestore)(int version);

#ifdef TARGET_GNW
static FCEUGI gameinfo_global;
#endif

// FCEU_LOW_RAM define allows to use a less RAM consuming method (but less CPU
// optimized) for memory handlers (allows to save about 512KBytes of RAM)
#ifndef FCEU_LOW_RAM
readfunc *ARead = NULL;
writefunc *BWrite = NULL;
#else
typedef struct {
  uint32 min_range, max_range;
  readfunc read_func;
} mem_read_handler_t;

typedef struct {
  uint32 min_range, max_range;
  writefunc write_func;
} mem_write_handler_t;

#if CHEAT_CODES == 1
#define MAX_MEM_HANDLER_READ_COUNT 20
#define MAX_MEM_HANDLER_WRITE_COUNT 20
#else
#define MAX_MEM_HANDLER_READ_COUNT 10
#define MAX_MEM_HANDLER_WRITE_COUNT 10
#endif
mem_read_handler_t MemRead[MAX_MEM_HANDLER_READ_COUNT];
mem_write_handler_t MemWrite[MAX_MEM_HANDLER_WRITE_COUNT];
static uint8 memRead_index;
static uint8 memWrite_index;
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
mem_read_handler_t MemReadG[MAX_MEM_HANDLER_READ_COUNT];
mem_write_handler_t MemWriteG[MAX_MEM_HANDLER_WRITE_COUNT];
static uint8 memReadG_index;
static uint8 memWriteG_index;
#endif
#endif

#ifdef FCEU_ENABLE_GAMEGENIE_ROM
#ifndef FCEU_LOW_RAM
static readfunc *AReadG = NULL;
static writefunc *BWriteG = NULL;
#endif
static int RWWrap = 0;
#endif
uint8 RAM[0x800];
uint8 PAL = 0;
uint8 Exit = 0;
uint8 pale = 0;
uint8 vsdip = 0;

unsigned vblankscanlines = 0;
unsigned skip_7bit_overclocking = 1;
unsigned overclocked = 0;
unsigned dendy = 0;
unsigned normal_scanlines = 240;
unsigned overclock_enabled = 0;
unsigned extrascanlines = 0;
unsigned totalscanlines = 240;
unsigned DMC_7bit = 0;

static DECLFW(BNull) {}

static DECLFR(ANull) { return (X.DB); }

static DECLFW(BRAML) { RAM[A] = V; }

static DECLFR(ARAML) { return RAM[A]; }

static DECLFW(BRAMH) { RAM[A & 0x7FF] = V; }

static DECLFR(ARAMH) { return RAM[A & 0x7FF]; }

#ifndef FCEU_LOW_RAM
void FCEU_AllocMemoryTables(void) {
  if (!ARead) {
    ARead =
        (readfunc *)rg_alloc(0x10000 * sizeof(readfunc), MEM_SLOW | MEM_8BIT);
  }
  if (!BWrite) {
    BWrite =
        (writefunc *)rg_alloc(0x10000 * sizeof(writefunc), MEM_SLOW | MEM_8BIT);
  }
  if (!ARead || !BWrite) {
    RG_PANIC("Failed to allocate FCEU memory tables");
  }
}
#endif

#ifdef FCEU_ENABLE_GAMEGENIE_ROM
int AllocGenieRW(void) {
#ifndef FCEU_LOW_RAM
  if (!AReadG) {
    if (!(AReadG = (readfunc *)FCEU_malloc(0x8000 * sizeof(readfunc))))
      return 0;
  } else
    memset(AReadG, 0, 0x8000 * sizeof(readfunc));

  if (!BWriteG) {
    if (!(BWriteG = (writefunc *)FCEU_malloc(0x8000 * sizeof(writefunc))))
      return 0;
  } else
    memset(BWriteG, 0, 0x8000 * sizeof(writefunc));
#endif

  RWWrap = 1;
  return 1;
}

void FlushGenieRW(void) {
  int32 x;

  if (RWWrap) {
#ifndef FCEU_LOW_RAM
    for (x = 0; x < 0x8000; x++) {
      ARead[x + 0x8000] = AReadG[x];
      BWrite[x + 0x8000] = BWriteG[x];
    }
    free(AReadG);
    free(BWriteG);
    AReadG = NULL;
    BWriteG = NULL;
#else
    bool merged = false;
    for (int i = 0; i < memReadG_index; i++) {
      merged = false;
      for (int j = 0; j < memRead_index; j++) {
        // We check if it can be merged to an existing range
        if ((MemRead[j].max_range == MemReadG[i].min_range + 0x7fff) &&
            (MemRead[j].read_func == MemReadG[i].read_func)) {
          merged = true;
          MemRead[j].max_range = MemReadG[i].max_range + 0x8000;
          break;
        }
      }
      if (!merged) {
        MemRead[memRead_index].min_range = MemReadG[i].min_range + 0x8000;
        MemRead[memRead_index].max_range = MemReadG[i].max_range + 0x8000;
        MemRead[memRead_index].read_func = MemReadG[i].read_func;
        memRead_index++;
      }
    }
    memReadG_index = 0;

    for (int i = 0; i < memWriteG_index; i++) {
      MemWrite[memWrite_index].min_range = MemWriteG[i].min_range + 0x8000;
      MemWrite[memWrite_index].max_range = MemWriteG[i].max_range + 0x8000;
      MemWrite[memWrite_index].write_func = MemWriteG[i].write_func;
      memWrite_index++;
    }
    memWriteG_index = 0;
#endif
  }
  RWWrap = 0;
}
#endif

readfunc FASTAPASS(1) GetReadHandler(int32 a) {
#ifndef FCEU_LOW_RAM
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (a >= 0x8000 && RWWrap)
    return AReadG[a - 0x8000];
  else
#endif
    return ARead[a];
#else
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap && a >= 0x8000) {
    /* Search in dynamically assigned functions */
    for (int i = memReadG_index - 1; i >= 0; i--) {
      if ((a >= MemReadG[i].min_range) && (a <= MemReadG[i].max_range)) {
        return MemReadG[i].read_func;
      }
    }
  }
#endif
  /* Search in dynamically assigned functions */
  for (int i = memRead_index - 1; i >= 0; i--) {
    if ((a >= MemRead[i].min_range) && (a <= MemRead[i].max_range)) {
      return MemRead[i].read_func;
    }
  }

  switch (a >> 11) {
  case 0: /* 0-0x7FF */
    return ARAML;
  case 1: /* 0x800-0x1FFF */
  case 2:
  case 3:
    return ARAMH;
  case 4: /* 0x2000-0x3FFF */
  case 5:
  case 6:
  case 7:
    switch (a % 8) {
    case 2:
      return A2002;
    case 7:
      return A2007;
    default:
      return A200x;
    }
  case 8: /* 0x4000 - 0x47FF */
    if (a == 0x4015)
      return StatusRead;
    if ((a == 0x4016) || (a == 0x4017))
      return JPRead;
    else if (a == 0x4030)
      return FDSRead4030;
    else if (a == 0x4031)
      return FDSRead4031;
    else if (a == 0x4032)
      return FDSRead4032;
    else if (a == 0x4033)
      return FDSRead4033;
    else if ((a >= 0x4040) && (a < 0x4080))
      return FDSWaveRead;
    else if ((a >= 0x4090) && (a <= 0x4092))
      return FDSSRead;
  }

  return ANull;
#endif
}

#ifdef FCEU_LOW_RAM
uint8 fceu_read(int32 a) {
  /* Search in dynamically assigned functions */
  for (int i = memRead_index - 1; i >= 0; i--) {
    if ((a >= MemRead[i].min_range) && (a <= MemRead[i].max_range)) {
      return MemRead[i].read_func(a);
    }
  }

  switch (a >> 11) {
  case 0: /* 0-0x7FF */
    return RAM[a];
  case 1: /* 0x800-0x1FFF */
  case 2:
  case 3:
    return RAM[a & 0x7FF];
  case 4: /* 0x2000-0x3FFF */
  case 5:
  case 6:
  case 7:
    switch (a % 8) {
    case 2:
      return A2002(a);
    case 7:
      return A2007(a);
    default:
      return A200x(a);
    }
  case 8: /* 0x4000 - 0x47FF */
    if (a == 0x4015)
      return StatusRead(a);
    if ((a == 0x4016) || (a == 0x4017))
      return JPRead(a);
    else if (a == 0x4030)
      return FDSRead4030(a);
    else if (a == 0x4031)
      return FDSRead4031(a);
    else if (a == 0x4032)
      return FDSRead4032(a);
    else if (a == 0x4033)
      return FDSRead4033(a);
    else if ((a >= 0x4040) && (a < 0x4080))
      return FDSWaveRead(a);
    else if ((a >= 0x4090) && (a <= 0x4092))
      return FDSSRead(a);
  }

  return (X.DB);
}
#endif

void FASTAPASS(3) SetReadHandler(int32 start, int32 end, readfunc func) {
#if !defined(FCEU_LOW_RAM) || defined(FCEU_ENABLE_GAMEGENIE_ROM)
  int32 x;
#endif
  // printf("SetReadHandler %lx-%lx %lx\n",start,end,(int32)func);
  if (!func)
    func = ANull;

#ifndef FCEU_LOW_RAM
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap)
    for (x = end; x >= start; x--) {
      if (x >= 0x8000)
        AReadG[x - 0x8000] = func;
      else
        ARead[x] = func;
    }
  else
#endif
    for (x = end; x >= start; x--)
      ARead[x] = func;
#else
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap) {
    if (end < 0x8000) {
      goto read_range;
    }
    if (start >= 0x8000) {
      MemReadG[memReadG_index].min_range = start - 0x8000;
      MemReadG[memReadG_index].max_range = end - 0x8000;
      MemReadG[memReadG_index].read_func = func;
      memReadG_index++;
    }
    if ((start < 0x8000) && (end >= 0x8000)) {
      MemReadG[memReadG_index].min_range = 0;
      MemReadG[memReadG_index].max_range = end - 0x8000;
      MemReadG[memReadG_index].read_func = func;
      memReadG_index++;

      end = 0x7fff;
      goto read_range;
    }
  } else
  read_range:
#endif
  {
    // Check if entry is already existing
    for (int i = 0; i < memRead_index; i++) {
      if ((MemRead[i].min_range == start) && (MemRead[i].max_range == end)) {
        MemRead[i].read_func = func;
        return;
      }
    }
    if (memRead_index == MAX_MEM_HANDLER_READ_COUNT) {
      printf("SetReadHandler increase MAX_MEM_HANDLER_READ_COUNT\n");
      while (1) {
      };
    }
    MemRead[memRead_index].min_range = start;
    MemRead[memRead_index].max_range = end;
    MemRead[memRead_index].read_func = func;
    memRead_index++;
  }
//	printf("memRead_index %d memReadG_index
//%d\n",memRead_index,memReadG_index);

#endif
}

writefunc FASTAPASS(1) GetWriteHandler(int32 a) {
#ifndef FCEU_LOW_RAM
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap && a >= 0x8000)
    return BWriteG[a - 0x8000];
  else
#endif
    return BWrite[a];
#else
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap && a >= 0x8000) {
    /* Search in dynamically assigned functions */
    for (int i = memWriteG_index - 1; i >= 0; i--) {
      if ((a >= MemWriteG[i].min_range) && (a <= MemWriteG[i].max_range)) {
        return MemWriteG[i].write_func;
      }
    }
  }
#endif
  /* Search in dynamically assigned functions */
  for (int i = memWrite_index - 1; i >= 0; i--) {
    if ((a >= MemWrite[i].min_range) && (a <= MemWrite[i].max_range)) {
      return MemWrite[i].write_func;
    }
  }
  switch (a >> 11) {
  case 0:
    return BRAML;
  case 1:
  case 2:
  case 3:
    return BRAMH;
  case 4:
  case 5:
  case 6:
  case 7:
    switch (a % 8) {
    case 0:
      return B2000;
    case 1:
      return B2001;
    case 2:
      return B2002;
    case 3:
      return B2003;
    case 4:
      return B2004;
    case 5:
      return B2005;
    case 6:
      return B2006;
    case 7:
      return B2007;
    }
  case 8: /* 0x4000 - 0x47FF */
    if (a < 0x4010) {
      return Write_PSG;
    } else if (a < 0x4014) {
      return Write_DMCRegs;
    } else if (a == 0x4014) {
      return B4014;
    } else if (a == 0x4015) {
      return StatusWrite;
    } else if (a == 0x4016) {
      return B4016;
    } else if (a == 0x4017) {
      return Write_IRQFM;
    } else if ((a >= 0x4040) && (a < 0x4080)) {
      return FDSWaveWrite;
    } else if ((a >= 0x4080) && (a <= 0x408A)) {
      return FDSSWrite;
    }
  }

  return BNull;
#endif
}

#ifdef FCEU_LOW_RAM
void fceu_write(int32 a, uint8 v) {
  for (int i = memWrite_index - 1; i >= 0; i--) {
    if ((a >= MemWrite[i].min_range) && (a <= MemWrite[i].max_range)) {
      MemWrite[i].write_func(a, v);
      return;
    }
  }
  switch (a >> 11) {
  case 0:
    RAM[a] = v;
    return;
  case 1:
  case 2:
  case 3:
    RAM[a & 0x7FF] = v;
    return;
  case 4:
  case 5:
  case 6:
  case 7:
    switch (a % 8) {
    case 0:
      B2000(a, v);
      return;
    case 1:
      B2001(a, v);
      return;
    case 2:
      B2002(a, v);
      return;
    case 3:
      B2003(a, v);
      return;
    case 4:
      B2004(a, v);
      return;
    case 5:
      B2005(a, v);
      return;
    case 6:
      B2006(a, v);
      return;
    case 7:
      B2007(a, v);
      return;
    }
  case 8: /* 0x4000 - 0x47FF */
    if (a < 0x4010) {
      Write_PSG(a, v);
      return;
    } else if (a < 0x4014) {
      Write_DMCRegs(a, v);
      return;
    } else if (a == 0x4014) {
      B4014(a, v);
      return;
    } else if (a == 0x4015) {
      StatusWrite(a, v);
      return;
    } else if (a == 0x4016) {
      return B4016(a, v);
    } else if (a == 0x4017) {
      Write_IRQFM(a, v);
      return;
    } else if ((a >= 0x4040) && (a < 0x4080)) {
      FDSWaveWrite(a, v);
      return;
    } else if ((a >= 0x4080) && (a <= 0x408A)) {
      FDSSWrite(a, v);
      return;
    }
  }
}
#endif

void FASTAPASS(3) SetWriteHandler(int32 start, int32 end, writefunc func) {
#if !defined(FCEU_LOW_RAM) || defined(FCEU_ENABLE_GAMEGENIE_ROM)
  int32 x;
#endif

  // printf("SetWriteHandler %lx-%lx %lx\n",start,end,(int32)func);
  if (!func)
    func = BNull;

#ifndef FCEU_LOW_RAM
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap)
    for (x = end; x >= start; x--) {
      if (x >= 0x8000)
        BWriteG[x - 0x8000] = func;
      else
        BWrite[x] = func;
    }
  else
#endif
    for (x = end; x >= start; x--)
      BWrite[x] = func;
#else
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (RWWrap) {
    if (end < 0x8000) {
      goto write_range;
    }
    if (start >= 0x8000) {
      MemWriteG[memWriteG_index].min_range = start - 0x8000;
      MemWriteG[memWriteG_index].max_range = end - 0x8000;
      MemWriteG[memWriteG_index].write_func = func;
      memWriteG_index++;
    }
    if ((start < 0x8000) && (end >= 0x8000)) {
      MemWriteG[memWriteG_index].min_range = 0;
      MemWriteG[memWriteG_index].max_range = end - 0x8000;
      MemWriteG[memWriteG_index].write_func = func;
      memWriteG_index++;

      end = 0x7fff;
      goto write_range;
    }
  } else
  write_range:
#endif
  {
    // Check if entry is already existing
    for (int i = 0; i < memWrite_index; i++) {
      if ((MemWrite[i].min_range == start) && (MemWrite[i].max_range == end)) {
        MemWrite[i].write_func = func;
        return;
      }
    }
    if (memWrite_index == MAX_MEM_HANDLER_WRITE_COUNT) {
      printf("SetWriteHandler increase MAX_MEM_HANDLER_WRITE_COUNT\n");
      while (1) {
      };
    }
    MemWrite[memWrite_index].min_range = start;
    MemWrite[memWrite_index].max_range = end;
    MemWrite[memWrite_index].write_func = func;
    memWrite_index++;
  }
#endif
}

void FCEUI_CloseGame(void) {
  // printf("FCEUI_CloseGame\n");
  if (!GameInfo)
    return;

#ifndef TARGET_GNW
  if (GameInfo->name)
    free(GameInfo->name);
#endif
  GameInfo->name = 0;
#if CHEAT_CODES == 1
#if !defined(TARGET_GNW) || defined(NES_MAPPER_NSF)
  if (GameInfo->type != GIT_NSF)
#endif
    FCEU_FlushGameCheats();
#endif
  GameInterface(GI_CLOSE);
  ResetExState(0, 0);
#ifndef TARGET_GNW
  FCEU_CloseGenie();
  free(GameInfo);
#elif defined(FCEU_ENABLE_GAMEGENIE_ROM)
  FCEU_CloseGenie();
#endif
  GameInfo = 0;
}

void ResetGameLoaded(void) {
  if (GameInfo)
    FCEUI_CloseGame();

  GameStateRestore = NULL;
  PPU_hook = NULL;
  GameHBIRQHook = NULL;

  if (GameExpSound.Kill)
    GameExpSound.Kill();
  memset(&GameExpSound, 0, sizeof(GameExpSound));

  MapIRQHook = NULL;
  MMC5Hack = 0;
  PEC586Hack = 0;
  PAL &= 1;
  pale = 0;
}

#ifdef TARGET_GNW
int iNESLoad(const char *name, const uint8_t *rom, uint32_t rom_size);
int FDSLoad(const char *name, const char *rom, uint32_t rom_size);
int NSFLoad(const char *name, const char *rom, uint32_t rom_size);
#else
int UNIFLoad(const char *name, FCEUFILE *fp);
int iNESLoad(const char *name, FCEUFILE *fp);
int FDSLoad(const char *name, FCEUFILE *fp);
int NSFLoad(FCEUFILE *fp);
#endif

#ifdef TARGET_GNW
FCEUGI *
FCEUI_LoadGame(const char *name, const uint8_t *databuf, size_t databufsize,
               frontend_post_load_init_cb_t frontend_post_load_init_cb) {
  ResetGameLoaded();

  GameInfo = &gameinfo_global;
  memset(GameInfo, 0, sizeof(FCEUGI));

  GameInfo->soundchan = 0;
  GameInfo->soundrate = 0;
  GameInfo->name = 0;
  GameInfo->type = GIT_CART;
  GameInfo->vidsys = GIV_USER;
  GameInfo->input[0] = GameInfo->input[1] = -1;
  GameInfo->inputfc = -1;
  GameInfo->cspecial = 0;

  if (iNESLoad(name, (const uint8_t *)databuf, databufsize))
    goto endlseq;
#ifdef NES_MAPPER_NSF
  if (NSFLoad(name, (const char *)databuf, databufsize))
    goto endlseq;
#endif
  if (FDSLoad(name, (const char *)databuf, databufsize))
    goto endlseq;

  FCEU_PrintError("An error occurred while loading the file.\n");
  return NULL;

endlseq:
  if (frontend_post_load_init_cb)
    (*frontend_post_load_init_cb)();

  FCEU_ResetVidSys();
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
#ifdef NES_MAPPER_NSF
  if (GameInfo->type != GIT_NSF)
#endif
    FCEU_OpenGenie();
#endif

  PowerNES();

#if CHEAT_CODES == 1
#ifdef NES_MAPPER_NSF
  if (GameInfo->type != GIT_NSF)
#endif
    FCEU_LoadGameCheats();
#endif

  FCEU_ResetPalette();

  return (GameInfo);
}
#else
FCEUGI *
FCEUI_LoadGame(const char *name, const uint8_t *databuf, size_t databufsize,
               frontend_post_load_init_cb_t frontend_post_load_init_cb) {
  FCEUFILE *fp;

  ResetGameLoaded();

  GameInfo = malloc(sizeof(FCEUGI));
  memset(GameInfo, 0, sizeof(FCEUGI));

  GameInfo->soundchan = 0;
  GameInfo->soundrate = 0;
  GameInfo->name = 0;
  GameInfo->type = GIT_CART;
  GameInfo->vidsys = GIV_USER;
  GameInfo->input[0] = GameInfo->input[1] = -1;
  GameInfo->inputfc = -1;
  GameInfo->cspecial = 0;

  fp = FCEU_fopen(name, databuf, databufsize);

  if (!fp) {
    FCEU_PrintError("Error opening \"%s\"!", name);

    free(GameInfo);
    GameInfo = NULL;

    return NULL;
  }

  if (iNESLoad(name, fp))
    goto endlseq;
  if (NSFLoad(fp))
    goto endlseq;
  if (UNIFLoad(NULL, fp))
    goto endlseq;
  if (FDSLoad(NULL, fp))
    goto endlseq;

  FCEU_PrintError("An error occurred while loading the file.\n");
  FCEU_fclose(fp);

  if (GameInfo->name)
    free(GameInfo->name);
  GameInfo->name = NULL;
  free(GameInfo);
  GameInfo = NULL;

  return NULL;

endlseq:
  FCEU_fclose(fp);

  if (frontend_post_load_init_cb)
    (*frontend_post_load_init_cb)();

  FCEU_ResetVidSys();
  if (GameInfo->type != GIT_NSF)
    if (FSettings.GameGenie)
      FCEU_OpenGenie();

  PowerNES();

  if (GameInfo->type != GIT_NSF) {
    FCEU_LoadGamePalette();
    FCEU_LoadGameCheats();
  }

  FCEU_ResetPalette();

  return (GameInfo);
}
#endif

int FCEUI_Initialize(void) {
  if (!FCEU_InitVirtualVideo())
    return 0;
  memset(&FSettings, 0, sizeof(FSettings));
  FSettings.UsrFirstSLine[0] = 8;
  FSettings.UsrFirstSLine[1] = 0;
  FSettings.UsrLastSLine[0] = 231;
  FSettings.UsrLastSLine[1] = 239;
  FSettings.SoundVolume = 100;
  FCEUPPU_Init();
  X6502_Init();
#ifndef FCEU_LOW_RAM
  FCEU_AllocMemoryTables();
#endif
  return 1;
}

void FCEUI_Kill(void) {
  FCEU_KillVirtualVideo();
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  FCEU_KillGenie();
#endif
}

void FCEUI_Emulate(uint8 **pXBuf, int32 **SoundBuf, int32 *SoundBufSize,
                   int skip) {
  int ssize;

  FCEU_UpdateInput();
#if CHEAT_CODES == 1
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (geniestage != 1)
    FCEU_ApplyPeriodicCheats();
#else
  FCEU_ApplyPeriodicCheats();
#endif
#endif
  FCEUPPU_Loop(skip);

  ssize = FlushEmulateSound();

  timestampbase += timestamp;

  timestamp = 0;
  sound_timestamp = 0;

  *pXBuf = XBuf;
  *SoundBuf = WaveFinal;
  *SoundBufSize = ssize;
}

void ResetNES(void) {
  if (!GameInfo)
    return;
  GameInterface(GI_RESETM2);
  FCEUSND_Reset();
  FCEUPPU_Reset();
  X6502_Reset();
}

int option_ramstate = 0;

void FCEU_MemoryRand(uint8 *ptr, uint32 size) {
  int x = 0;
  while (size) {
#if 0
		*ptr = (x & 4) ? 0xFF : 0x00;	/* Huang Di DEBUG MODE enabled by default */
										/* Cybernoid NO MUSIC by default */
		*ptr = (x & 4) ? 0x7F : 0x00;	/* Huang Di DEBUG MODE enabled by default */
										/* Minna no Taabou no Nakayoshi Daisakusen DOESN'T BOOT */
										/* Cybernoid NO MUSIC by default */
		*ptr = (x & 1) ? 0x55 : 0xAA;	/* F-15 Sity War HISCORE is screwed... */
										/* 1942 SCORE/HISCORE is screwed... */
#endif
    uint8_t v = 0;
    switch (option_ramstate) {
    case 0:
      v = 0xff;
      break;
    case 1:
      v = 0x00;
      break;
    case 2:
      v = (uint8_t)rand();
      break;
    }
    *ptr = v;
    x++;
    size--;
    ptr++;
  }
}

void hand(X6502 *X, int type, uint32 A) {}

void PowerNES(void) {
  FCEU_printf("PowerNES\n");
  if (!GameInfo)
    return;

#ifdef FCEU_LOW_RAM
  memRead_index = 0;
  memWrite_index = 0;
#endif

#if CHEAT_CODES == 1
  FCEU_CheatResetRAM();
  FCEU_CheatAddRAM(2, 0, RAM);
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  RWWrap = 0;
  FCEU_GeniePower();
#endif
#endif

  FCEU_MemoryRand(RAM, 0x800);

#ifndef FCEU_LOW_RAM
  SetReadHandler(0x0000, 0xFFFF, ANull);
  SetWriteHandler(0x0000, 0xFFFF, BNull);

  SetReadHandler(0, 0x7FF, ARAML);
  SetWriteHandler(0, 0x7FF, BRAML);

  SetReadHandler(0x800, 0x1FFF, ARAMH);  /* Part of a little */
  SetWriteHandler(0x800, 0x1FFF, BRAMH); /* hack for a small speed boost. */
#endif

  InitializeInput();
  FCEUSND_Power();
  FCEUPPU_Power();

  /* Have the external game hardware "powered" after the internal NES stuff.
          Needed for the NSF code and VS System code.
  */
  if (GameInterface)
    GameInterface(GI_POWER);
#ifndef TARGET_GNW
  if (GameInfo->type == GIT_VSUNI)
    FCEU_VSUniPower();
#endif

  timestampbase = 0;
  X6502_Power();
#if CHEAT_CODES == 1
  FCEU_PowerCheats();
#endif
}

void FCEU_ResetVidSys(void) {
  int w;

  if (GameInfo->vidsys == GIV_NTSC)
    w = 0;
  else if (GameInfo->vidsys == GIV_PAL) {
    w = 1;
    dendy = 0;
  } else
    w = FSettings.PAL;

  PAL = w ? 1 : 0;

  if (PAL)
    dendy = 0;

  normal_scanlines = dendy ? 290 : 240;
#ifndef FCEU_NO_OVERCLOCKING
  totalscanlines = normal_scanlines + (overclock_enabled ? extrascanlines : 0);
#else
  totalscanlines = normal_scanlines;
#endif
  FCEUPPU_SetVideoSystem(w || dendy);
  SetSoundVariables();
}

FCEUS FSettings;

void FCEU_printf(char *format, ...) {
  char temp[2048];

  va_list ap;

  va_start(ap, format);
  vsprintf(temp, format, ap);
  FCEUD_Message(temp);

  va_end(ap);
}

void FCEU_PrintError(char *format, ...) {
  char temp[2048];

  va_list ap;

  va_start(ap, format);
  vsprintf(temp, format, ap);
  FCEUD_PrintError(temp);

  va_end(ap);
}

void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall) {
  FSettings.UsrFirstSLine[0] = ntscf;
  FSettings.UsrLastSLine[0] = ntscl;
  FSettings.UsrFirstSLine[1] = palf;
  FSettings.UsrLastSLine[1] = pall;
  if (PAL || dendy) {
    FSettings.FirstSLine = FSettings.UsrFirstSLine[1];
    FSettings.LastSLine = FSettings.UsrLastSLine[1];
  } else {
    FSettings.FirstSLine = FSettings.UsrFirstSLine[0];
    FSettings.LastSLine = FSettings.UsrLastSLine[0];
  }
}

void FCEUI_SetVidSystem(int a) {
  FSettings.PAL = a ? 1 : 0;

  if (!GameInfo)
    return;

  FCEU_ResetVidSys();
  FCEU_ResetPalette();
}

int FCEUI_GetCurrentVidSystem(int *slstart, int *slend) {
  if (slstart)
    *slstart = FSettings.FirstSLine;
  if (slend)
    *slend = FSettings.LastSLine;
  return (PAL);
}

void FCEUI_SetGameGenie(int a) { FSettings.GameGenie = a ? 1 : 0; }

int32 FCEUI_GetDesiredFPS(void) {
  if (PAL || dendy)
    return (838977920); /* ~50.007 */
  else
    return (1008307711); /* ~60.1 */
}
#endif