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
 *  Copyright (C) 1998 BERO
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

#include <stdlib.h>
#include <string.h>

#include "fceu-endian.h"
#include "fceu-memory.h"
#include "fceu-sound.h"
#include "fceu-types.h"
#include "fceu.h"
#include "general.h"
#include "nsf.h"
#include "ppu.h"
#include "x6502.h"

#include "fceu-cart.h"
#include "fceu-state.h"
#include "input.h"
#include "palette.h"
#include "video.h"

#define VBlankON (PPU[0] & 0x80) /* Generate VBlank NMI */
#define Sprite16 (PPU[0] & 0x20) /* Sprites 8x16/8x8 */
#define BGAdrHI (PPU[0] & 0x10)  /* BG pattern adr $0000/$1000 */
#define SpAdrHI (PPU[0] & 0x08)  /* Sprite pattern adr $0000/$1000 */
#define INC32 (PPU[0] & 0x04)    /* auto increment 1/32 */

#define SpriteON (PPU[1] & 0x10) /* Show Sprite */
#define ScreenON (PPU[1] & 0x08) /* Show screen */
#define GRAYSCALE                                                              \
  (PPU[1] & 0x01) /* Grayscale (AND palette entries with 0x30) */

#define PPU_status (PPU[2])

#define Pal (PALRAM)

static void FetchSpriteData(void);
static void RefreshLine(int lastpixel);
static void RefreshSprites(void);
static void CopySprites(uint8 *target);

static void Fixit1(void);
static uint32 ppulut1[256];
static uint32 ppulut2[256];
static uint32 ppulut3[128];

static void makeppulut(void) {
  int x;
  int y;
  int cc, xo, pixel;

  for (x = 0; x < 256; x++) {
    ppulut1[x] = 0;
    for (y = 0; y < 8; y++)
      ppulut1[x] |= ((x >> (7 - y)) & 1) << (y * 4);
    ppulut2[x] = ppulut1[x] << 1;
  }

  for (cc = 0; cc < 16; cc++) {
    for (xo = 0; xo < 8; xo++) {
      ppulut3[xo | (cc << 3)] = 0;
      for (pixel = 0; pixel < 8; pixel++) {
        int shiftr;
        shiftr = (pixel + xo) / 8;
        shiftr *= 2;
        ppulut3[xo | (cc << 3)] |= ((cc >> shiftr) & 3) << (2 + pixel * 4);
      }
    }
  }
}

static uint8 ppudead = 1;
static uint8 kook = 0;
int fceuindbg = 0;

int MMC5Hack = 0, PEC586Hack = 0;
uint32 MMC5HackVROMMask = 0;
uint8 *MMC5HackExNTARAMPtr = 0;
uint8 *MMC5HackVROMPTR = 0;
uint8 MMC5HackCHRMode = 0;
uint8 MMC5HackSPMode = 0;
uint8 MMC50x5130 = 0;
uint8 MMC5HackSPScroll = 0;
uint8 MMC5HackSPPage = 0;

uint8 VRAMBuffer = 0, PPUGenLatch = 0;
uint8 *vnapage[4];
uint8 PPUNTARAM = 0;
uint8 PPUCHRRAM = 0;

/* Color deemphasis emulation.  Joy... */
static uint8 deemp = 0;
static int deempcnt[8];

void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);
void FP_FASTAPASS(1) (*PPU_hook)(uint32 A);

uint8 vtoggle = 0;
uint8 XOffset = 0;

uint32 TempAddr = 0, RefreshAddr = 0;

static int maxsprites = 8;

/* scanline is equal to the current visible scanline we're on. */
int scanline;
static uint32 scanlines_per_frame;

uint8 PPU[4];
uint8 PPUSPL;
uint8 NTARAM[0x800] __attribute__((aligned(16))),
    PALRAM[0x20] __attribute__((aligned(16))),
    SPRAM[0x100] __attribute__((aligned(16))),
    SPRBUF[0x100] __attribute__((aligned(16)));
uint8 UPALRAM[0x03] __attribute__((aligned(16)));

#define MMC5SPRVRAMADR(V) &MMC5SPRVPage[(V) >> 10][(V)]
#define VRAMADR(V) &VPage[(V) >> 10][(V)]

uint8 *MMC5BGVRAMADR(uint32 V) {
  if (!Sprite16) {
    extern uint8 mmc5ABMode; /* A=0, B=1 */
    if (mmc5ABMode == 0)
      return MMC5SPRVRAMADR(V);
    else
      return &MMC5BGVPage[(V) >> 10][(V)];
  } else
    return &MMC5BGVPage[(V) >> 10][(V)];
}

DECLFR(A2002) {
  uint8 ret;

  FCEUPPU_LineUpdate();
  ret = PPU_status;
  ret |= PPUGenLatch & 0x1F;

#ifdef FCEUDEF_DEBUGGER
  if (!fceuindbg)
#endif
  {
    vtoggle = 0;
    PPU_status &= 0x7F;
    PPUGenLatch = ret;
  }

  return ret;
}

DECLFR(A200x) { /* Not correct for $2004 reads. */
  FCEUPPU_LineUpdate();
  return PPUGenLatch;
}

DECLFR(A2007) {
  uint8 ret;
  uint32 tmp = RefreshAddr & 0x3FFF;

  FCEUPPU_LineUpdate();

  if (tmp >= 0x3F00) { /* Palette RAM tied directly to the output data, without
                          VRAM buffer */
    if (!(tmp & 3)) {
      if (!(tmp & 0xC))
        ret = PALRAM[0x00];
      else
        ret = UPALRAM[((tmp & 0xC) >> 2) - 1];
    } else
      ret = PALRAM[tmp & 0x1F];
    if (GRAYSCALE)
      ret &= 0x30;
#ifdef FCEUDEF_DEBUGGER
    if (!fceuindbg)
#endif
    {
      if ((tmp - 0x1000) < 0x2000)
        VRAMBuffer = VPage[(tmp - 0x1000) >> 10][tmp - 0x1000];
      else
        VRAMBuffer =
            vnapage[((tmp - 0x1000) >> 10) & 0x3][(tmp - 0x1000) & 0x3FF];
      if (PPU_hook)
        PPU_hook(tmp);
    }
  } else {
    ret = VRAMBuffer;
#ifdef FCEUDEF_DEBUGGER
    if (!fceuindbg)
#endif
    {
      if (PPU_hook)
        PPU_hook(tmp);
      PPUGenLatch = VRAMBuffer;
      if (tmp < 0x2000) {
        VRAMBuffer = VPage[tmp >> 10][tmp];
      } else if (tmp < 0x3F00)
        VRAMBuffer = vnapage[(tmp >> 10) & 0x3][tmp & 0x3FF];
    }
  }

#ifdef FCEUDEF_DEBUGGER
  if (!fceuindbg)
#endif
  {
    if ((ScreenON || SpriteON) && (scanline < 240)) {
      uint32 rad = RefreshAddr;
      if ((rad & 0x7000) == 0x7000) {
        rad ^= 0x7000;
        if ((rad & 0x3E0) == 0x3A0)
          rad ^= 0xBA0;
        else if ((rad & 0x3E0) == 0x3e0)
          rad ^= 0x3e0;
        else
          rad += 0x20;
      } else
        rad += 0x1000;
      RefreshAddr = rad;
    } else {
      if (INC32)
        RefreshAddr += 32;
      else
        RefreshAddr++;
    }
    if (PPU_hook)
      PPU_hook(RefreshAddr & 0x3fff);
  }
  return ret;
}

DECLFW(B2000) {
  FCEUPPU_LineUpdate();
  PPUGenLatch = V;

  if (!(PPU[0] & 0x80) && (V & 0x80) && (PPU_status & 0x80))
    TriggerNMI2();

  PPU[0] = V;
  TempAddr &= 0xF3FF;
  TempAddr |= (V & 3) << 10;
}

DECLFW(B2001) {
  FCEUPPU_LineUpdate();
  PPUGenLatch = V;
  PPU[1] = V;
  if (V & 0xE0)
    deemp = V >> 5;
}

DECLFW(B2002) { PPUGenLatch = V; }

DECLFW(B2003) {
  PPUGenLatch = V;
  PPU[3] = V;
  PPUSPL = V & 0x7;
}

DECLFW(B2004) {
  PPUGenLatch = V;
  if (PPUSPL >= 8) {
    if (PPU[3] >= 8)
      SPRAM[PPU[3]] = V;
  } else {
    SPRAM[PPUSPL] = V;
  }
  PPU[3]++;
  PPUSPL++;
}

DECLFW(B2005) {
  uint32 tmp = TempAddr;
  FCEUPPU_LineUpdate();
  PPUGenLatch = V;
  if (!vtoggle) {
    tmp &= 0xFFE0;
    tmp |= V >> 3;
    XOffset = V & 7;
  } else {
    tmp &= 0x8C1F;
    tmp |= ((V & ~0x7) << 2);
    tmp |= (V & 7) << 12;
  }
  TempAddr = tmp;
  vtoggle ^= 1;
}

DECLFW(B2006) {
  FCEUPPU_LineUpdate();

  PPUGenLatch = V;
  if (!vtoggle) {
    TempAddr &= 0x00FF;
    TempAddr |= (V & 0x3f) << 8;
  } else {
    TempAddr &= 0xFF00;
    TempAddr |= V;

    RefreshAddr = TempAddr;
    if (PPU_hook)
      PPU_hook(RefreshAddr);
  }
  vtoggle ^= 1;
}

DECLFW(B2007) {
  uint32 tmp = RefreshAddr & 0x3FFF;
  PPUGenLatch = V;
  if (tmp < 0x2000) {
    if (PPUCHRRAM & (1 << (tmp >> 10)))
      VPage[tmp >> 10][tmp] = V;
  } else if (tmp < 0x3F00) {
    if (PPUNTARAM & (1 << ((tmp & 0xF00) >> 10)))
      vnapage[((tmp & 0xF00) >> 10)][tmp & 0x3FF] = V;
  } else {
    if (!(tmp & 3)) {
      if (!(tmp & 0xC))
        PALRAM[0x00] = PALRAM[0x04] = PALRAM[0x08] = PALRAM[0x0C] = V & 0x3F;
      else
        UPALRAM[((tmp & 0xC) >> 2) - 1] = V & 0x3F;
    } else
      PALRAM[tmp & 0x1F] = V & 0x3F;
  }
  if (INC32)
    RefreshAddr += 32;
  else
    RefreshAddr++;
  if (PPU_hook)
    PPU_hook(RefreshAddr & 0x3fff);
}

DECLFW(B4014) {
  uint32 t = V << 8;
  int x;

  for (x = 0; x < 256; x++)
    X6502_DMW(0x2004, X6502_DMR(t + x));
}

#define PAL(c) ((c) + cc)

#define GETLASTPIXEL                                                           \
  (PAL ? ((timestamp * 48 - linestartts) / 15)                                 \
       : ((timestamp * 48 - linestartts) >> 4))

static uint8 *Pline, *Plinef;
static int firsttile;
static int linestartts;
static int tofix = 0;

static void ResetRL(uint8 *target) {
  memset(target, 0xFF, 256);
  if (InputScanlineHook)
    InputScanlineHook(0, 0, 0, 0);
  Plinef = target;
  Pline = target;
  firsttile = 0;
  linestartts = timestamp * 48 + X.count;
  tofix = 0;
  FCEUPPU_LineUpdate();
  tofix = 1;
}

static uint8 sprlinebuf[256 + 8];

IRAM_ATTR void FCEUPPU_LineUpdate(void) {
#ifdef FCEUDEF_DEBUGGER
  if (!fceuindbg)
#endif
    if (Pline) {
      int l = GETLASTPIXEL;
      RefreshLine(l);
    }
}

static int rendis = 0;

void FCEUI_SetRenderDisable(int sprites, int bg) {
  if (sprites >= 0) {
    if (sprites == 2)
      rendis ^= 1;
    else
      rendis = (rendis & ~1) | sprites ? 1 : 0;
  }
  if (bg >= 0) {
    if (bg == 2)
      rendis ^= 2;
    else
      rendis = (rendis & ~2) | bg ? 2 : 0;
  }
}

static void CheckSpriteHit(int p);

static void EndRL(void) {
  RefreshLine(272);
  if (tofix)
    Fixit1();
  CheckSpriteHit(272);
  Pline = 0;
}

static int32 sphitx;
static uint8 sphitdata;

static void CheckSpriteHit(int p) {
  int l = p - 16;
  int x;

  if (sphitx == 0x100)
    return;

  for (x = sphitx; x < (sphitx + 8) && x < l; x++) {
    if ((sphitdata & (0x80 >> (x - sphitx))) && !(Plinef[x] & 64) && x < 255) {
      PPU_status |= 0x40;
      sphitx = 0x100;
      break;
    }
  }
}

/* spork the world.  Any sprites on this line? Then this will be set to 1.
 * Needed for zapper emulation and *gasp* sprite emulation.
 */
static int spork = 0;
static uint8 spr_min_x, spr_max_x;
static uint8 active_sprites[64];
static uint8 num_active_sprites;

static void BuildActiveSpriteList(void);

/* lasttile is really "second to last tile." */
static IRAM_ATTR void RefreshLine(int lastpixel) {
  static uint32 pshift[2];
  static uint32 atlatch;
  uint32 smorkus = RefreshAddr;

#define RefreshAddr smorkus
  uint32 vofs;
  int X1;

  register uint8 *P = Pline;
  int lasttile = lastpixel >> 3;
  int numtiles;
  static int norecurse = 0; /* Yeah, recursion would be bad.
                             * PPU_hook() functions can call
                             * mirroring/chr bank switching functions,
                             * which call FCEUPPU_LineUpdate, which call this
                             * function.
                             */
  if (norecurse)
    return;

  if (sphitx != 0x100 && !(PPU_status & 0x40)) {
    if ((sphitx < (lastpixel - 16)) && !(sphitx < ((lasttile - 2) * 8)))
      lasttile++;
  }

  if (lasttile > 34)
    lasttile = 34;
  numtiles = lasttile - firsttile;

  if (numtiles <= 0)
    return;

  P = Pline;

  vofs = 0;

  if (PEC586Hack)
    vofs = ((RefreshAddr & 0x200) << 3) | ((RefreshAddr >> 12) & 7);
  else
    vofs = ((PPU[0] & 0x10) << 8) | ((RefreshAddr >> 12) & 7);

  if (!ScreenON && !SpriteON) {
    uint32 tem;
    tem = Pal[0] | (Pal[0] << 8) | (Pal[0] << 16) | (Pal[0] << 24);
    tem |= 0x40404040;
    FCEU_dwmemset(Pline, tem, numtiles * 8);
    P += numtiles * 8;
    Pline = P;

    firsttile = lasttile;

#define TOFIXNUM (272 - 0x4)
    if (lastpixel >= TOFIXNUM && tofix) {
      Fixit1();
      tofix = 0;
    }

    if (InputScanlineHook && (lastpixel - 16) >= 0) {
      InputScanlineHook(Plinef, spork ? sprlinebuf : 0, linestartts,
                        lasttile * 8 - 16);
    }
    return;
  }

  /* Priority bits, needed for sprite emulation. */
  Pal[0] |= 64;
  Pal[4] |= 64;
  Pal[8] |= 64;
  Pal[0xC] |= 64;

  /* This high-level graphics MMC5 emulation code was written for MMC5 carts in
   * "CL" mode. It's probably not totally correct for carts in "SL" mode.
   */

#define PPUT_MMC5
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
  if (MMC5Hack && geniestage != 1) {
#else
  if (MMC5Hack) {
#endif
    if (MMC5HackCHRMode == 0 && (MMC5HackSPMode & 0x80)) {
      int tochange = MMC5HackSPMode & 0x1F;
      tochange -= firsttile;
      for (X1 = firsttile; X1 < lasttile; X1++) {
        if ((tochange <= 0 && MMC5HackSPMode & 0x40) ||
            (tochange > 0 && !(MMC5HackSPMode & 0x40))) {
#define PPUT_MMC5SP
#include "pputile.h"
#undef PPUT_MMC5SP
        } else {
#include "pputile.h"
        }
        tochange--;
      }
    } else if (MMC5HackCHRMode == 1 && (MMC5HackSPMode & 0x80)) {
      int tochange = MMC5HackSPMode & 0x1F;
      tochange -= firsttile;

#define PPUT_MMC5SP
#define PPUT_MMC5CHR1
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
#undef PPUT_MMC5CHR1
#undef PPUT_MMC5SP
    } else if (MMC5HackCHRMode == 1) {
#define PPUT_MMC5CHR1
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
#undef PPUT_MMC5CHR1
    } else {
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
    }
  }
#undef PPUT_MMC5
  else if (PPU_hook) {
    norecurse = 1;
#define PPUT_HOOK
    if (PEC586Hack) {
#define PPU_BGFETCH
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
#undef PPU_BGFETCH
    } else {
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
    }
#undef PPUT_HOOK
    norecurse = 0;
  } else {
    if (PEC586Hack) {
#define PPU_BGFETCH
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
#undef PPU_BGFETCH
    } else {
      for (X1 = firsttile; X1 < lasttile; X1++) {
#include "pputile.h"
      }
    }
  }

#undef vofs
#undef RefreshAddr

  /* Reverse changes made before. */
  Pal[0] &= 63;
  Pal[4] &= 63;
  Pal[8] &= 63;
  Pal[0xC] &= 63;

  RefreshAddr = smorkus;
  if (firsttile <= 2 && 2 < lasttile && !(PPU[1] & 2)) {
    uint32 tem;
    tem = Pal[0] | (Pal[0] << 8) | (Pal[0] << 16) | (Pal[0] << 24);
    tem |= 0x40404040;
    *(uint32 *)Plinef = *(uint32 *)(Plinef + 4) = tem;
  }

  if (!ScreenON) {
    uint32 tem;
    int tstart, tcount;
    tem = Pal[0] | (Pal[0] << 8) | (Pal[0] << 16) | (Pal[0] << 24);
    tem |= 0x40404040;

    tcount = lasttile - firsttile;
    tstart = firsttile - 2;
    if (tstart < 0) {
      tcount += tstart;
      tstart = 0;
    }
    if (tcount > 0)
      FCEU_dwmemset(Plinef + tstart * 8, tem, tcount * 8);
  }

  if (lastpixel >= TOFIXNUM && tofix) {
    Fixit1();
    tofix = 0;
  }

  /* This only works right because of a hack earlier in this function. */
  CheckSpriteHit(lastpixel);

  if (InputScanlineHook && (lastpixel - 16) >= 0) {
    InputScanlineHook(Plinef, spork ? sprlinebuf : 0, linestartts,
                      lasttile * 8 - 16);
  }
  Pline = P;
  firsttile = lasttile;
}

static INLINE void Fixit2(void) {
  if (ScreenON || SpriteON) {
    uint32 rad = RefreshAddr;
    rad &= 0xFBE0;
    rad |= TempAddr & 0x041f;
    RefreshAddr = rad;
  }
}

static void Fixit1(void) {
  if (ScreenON || SpriteON) {
    uint32 rad = RefreshAddr;

    if ((rad & 0x7000) == 0x7000) {
      rad ^= 0x7000;
      if ((rad & 0x3E0) == 0x3A0)
        rad ^= 0xBA0;
      else if ((rad & 0x3E0) == 0x3e0)
        rad ^= 0x3e0;
      else
        rad += 0x20;
    } else
      rad += 0x1000;
    RefreshAddr = rad;
  }
}

void MMC5_hb(int); /* Ugh ugh ugh. */
static void IRAM_ATTR DoLine(void) {
  int x;
  uint8 *target = NULL;
#ifndef TARGET_GNW
  int colour_emphasis;
  uint8 *dtarget = NULL;
#endif

  if (scanline >= 240 && scanline != totalscanlines) {
    X6502_Run(256 + 69);
    scanline++;
    X6502_Run(16);
    return;
  }

  target = XBuf + ((scanline < 240 ? scanline : 240) << 8);
#ifndef TARGET_GNW
  dtarget = XDBuf + ((scanline < 240 ? scanline : 240) << 8);
#endif

  if (MMC5Hack && (ScreenON || SpriteON))
    MMC5_hb(scanline);

  X6502_Run(256);
  EndRL();

  if (rendis & 2) { /* User asked to not display background data. */
    uint32 tem;
    tem = Pal[0] | (Pal[0] << 8) | (Pal[0] << 16) | (Pal[0] << 24);
    tem |= 0x40404040;
    FCEU_dwmemset(target, tem, 256);
  }

  if (SpriteON)
    CopySprites(target);

  if (ScreenON || SpriteON) { /* Yes, very el-cheapo. */
    if (PPU[1] & 0x01) {
      for (x = 63; x >= 0; x--)
        *(uint32 *)&target[x << 2] = (*(uint32 *)&target[x << 2]) & 0x30303030;
    }
  }
  if ((PPU[1] >> 5) == 0x7) {
    for (x = 63; x >= 0; x--)
      *(uint32 *)&target[x << 2] =
          ((*(uint32 *)&target[x << 2]) & 0x3f3f3f3f) | 0xc0c0c0c0;
  } else if (PPU[1] & 0xE0)
    for (x = 63; x >= 0; x--)
      *(uint32 *)&target[x << 2] = (*(uint32 *)&target[x << 2]) | 0x40404040;
  else
    for (x = 63; x >= 0; x--)
      *(uint32 *)&target[x << 2] =
          ((*(uint32 *)&target[x << 2]) & 0x3f3f3f3f) | 0x80808080;

  /* write the actual colour emphasis */
#ifndef TARGET_GNW
  colour_emphasis = ((PPU[1] >> 5) << 24) | ((PPU[1] >> 5) << 16) |
                    ((PPU[1] >> 5) << 8) | ((PPU[1] >> 5) << 0);
  for (x = 63; x >= 0; x--)
    *(uint32 *)&dtarget[x << 2] = colour_emphasis;
#endif

  sphitx = 0x100;

  if (ScreenON || SpriteON)
    if (scanline == 0) {
      BuildActiveSpriteList();
    }

  FetchSpriteData();

  if (GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0] & 0x38) != 0x18)) {
    X6502_Run(6);
    Fixit2();
    X6502_Run(4);
    GameHBIRQHook();
    X6502_Run(85 - 16 - 10);
  } else {
    X6502_Run(6); /* Tried 65, caused problems with Slalom(maybe others) */
    Fixit2();
    X6502_Run(85 - 6 - 16);

    /* A semi-hack for Star Trek: 25th Anniversary */
    if (GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0] & 0x38) != 0x18))
      GameHBIRQHook();
  }

  if (SpriteON)
    RefreshSprites();
  if (GameHBIRQHook2 && (ScreenON || SpriteON))
    GameHBIRQHook2();
  scanline++;
  if (scanline < 240) {
    ResetRL(XBuf + (scanline << 8));
  }
  X6502_Run(16);
}

#define V_FLIP 0x80
#define H_FLIP 0x40
#define SP_BACK 0x20

typedef struct {
  uint8 y, no, atr, x;
} SPR;

typedef struct {
  uint8 ca[2], atr, x;
} SPRB;

void FCEUI_DisableSpriteLimitation(int a) { maxsprites = a ? 64 : 8; }

static uint8 numsprites, SpriteBlurp;
static void IRAM_ATTR BuildActiveSpriteList(void) {
  SPR *spr = (SPR *)SPRAM;
  uint8 count = 0;
  for (int n = 63; n >= 0; n--, spr++) {
    if (spr->y < 240) {
      active_sprites[count++] = (uint8)n;
    }
  }
  num_active_sprites = count;
}

static void IRAM_ATTR FetchSpriteData(void) {
  uint8 ns, sb;
  SPR *spr;
  uint8 H;
  int n;
  int vofs;
  uint8 P0 = PPU[0];

  spr = (SPR *)SPRAM;
  H = 8;

  ns = sb = 0;

  vofs = (uint32)(P0 & 0x8 & (((P0 & 0x20) ^ 0x20) >> 2)) << 9;
  H += (P0 & 0x20) >> 2;

  if (!PPU_hook)
    for (int i = 0; i < num_active_sprites; i++) {
      n = active_sprites[i];
      spr = (SPR *)SPRAM + (63 - n); // SPRAM is indexed 0 to 63
      if ((uint32)(scanline - spr->y) >= H)
        continue;
      if (ns < maxsprites) {
        if (n == 63)
          sb = 1;

        {
          SPRB dst;
          uint8 *C;
          int t;
          uint32 vadr;

          t = (int)scanline - (spr->y);

          if (Sprite16)
            vadr = ((spr->no & 1) << 12) + ((spr->no & 0xFE) << 4);
          else
            vadr = (spr->no << 4) + vofs;

          if (spr->atr & V_FLIP) {
            vadr += 7;
            vadr -= t;
            vadr += (P0 & 0x20) >> 1;
            vadr -= t & 8;
          } else {
            vadr += t;
            vadr += t & 8;
          }

        /* Fix this geniestage hack */
#ifdef FCEU_ENABLE_GAMEGENIE_ROM
          if (MMC5Hack && geniestage != 1)
#else
          if (MMC5Hack)
#endif
            C = MMC5SPRVRAMADR(vadr);
          else
            C = VRAMADR(vadr);

          dst.ca[0] = C[0];
          dst.ca[1] = C[8];
          dst.x = spr->x;
          dst.atr = spr->atr;

          *(uint32 *)&SPRBUF[ns << 2] = *(uint32 *)&dst;
        }

        ns++;
      } else {
        PPU_status |= 0x20;
        break;
      }
    }
  else
    for (int i = 0; i < num_active_sprites; i++) {
      n = active_sprites[i];
      spr = (SPR *)SPRAM + (63 - n);
      if ((uint32)(scanline - spr->y) >= H)
        continue;

      if (ns < maxsprites) {
        if (n == 63)
          sb = 1;

        {
          SPRB dst;
          uint8 *C;
          int t;
          uint32 vadr;

          t = (int)scanline - (spr->y);

          if (Sprite16)
            vadr = ((spr->no & 1) << 12) + ((spr->no & 0xFE) << 4);
          else
            vadr = (spr->no << 4) + vofs;

          if (spr->atr & V_FLIP) {
            vadr += 7;
            vadr -= t;
            vadr += (P0 & 0x20) >> 1;
            vadr -= t & 8;
          } else {
            vadr += t;
            vadr += t & 8;
          }

          if (MMC5Hack)
            C = MMC5SPRVRAMADR(vadr);
          else
            C = VRAMADR(vadr);
          dst.ca[0] = C[0];
          if (ns < 8) {
            PPU_hook(0x2000);
            PPU_hook(vadr);
          }
          dst.ca[1] = C[8];
          dst.x = spr->x;
          dst.atr = spr->atr;

          *(uint32 *)&SPRBUF[ns << 2] = *(uint32 *)&dst;
        }

        ns++;
      } else {
        PPU_status |= 0x20;
        break;
      }
    }

  /* Handle case when >8 sprites per scanline option is enabled. */
  if (ns > 8)
    PPU_status |= 0x20;
  else if (PPU_hook) {
    for (n = 0; n < (8 - ns); n++) {
      PPU_hook(0x2000);
      PPU_hook(vofs);
    }
  }
  numsprites = ns;
  SpriteBlurp = sb;
}

static IRAM_ATTR void RefreshSprites(void) {
  int n;
  SPRB *spr;

  spork = 0;
  if (!numsprites)
    return;

  /* Optimized: Only clear the previous dirty range if it's small,
     otherwise clear everything. Actually, FCEU_dwmemset is fast. */
  FCEU_dwmemset(sprlinebuf, 0x80808080, 256);

  numsprites--;
  spr = (SPRB *)SPRBUF + numsprites;

  spr_min_x = 255;
  spr_max_x = 0;

  for (n = numsprites; n >= 0; n--, spr--) {
    register uint32 pixdata;
    register uint8 J, atr;

    int x = spr->x;
    uint8 *C;
    uint8 *VB;

    pixdata = ppulut1[spr->ca[0]] | ppulut2[spr->ca[1]];
    J = spr->ca[0] | spr->ca[1];
    atr = spr->atr;

    if (J) {
      if (n == 0 && SpriteBlurp && !(PPU_status & 0x40)) {
        sphitx = x;
        sphitdata = J;
        if (atr & H_FLIP)
          sphitdata = ((J << 7) & 0x80) | ((J << 5) & 0x40) |
                      ((J << 3) & 0x20) | ((J << 1) & 0x10) |
                      ((J >> 1) & 0x08) | ((J >> 3) & 0x04) |
                      ((J >> 5) & 0x02) | ((J >> 7) & 0x01);
      }

      if (x < spr_min_x)
        spr_min_x = x;
      if (x > spr_max_x)
        spr_max_x = x;

      C = sprlinebuf + x;
      VB = (PALRAM + 0x10) + ((atr & 3) << 2);

      if (atr & H_FLIP) {
        uint8 p0 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p1 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p2 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p3 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p4 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p5 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p6 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p7 = VB[pixdata & 3];

        if (atr & SP_BACK) {
          if (J & 0x80)
            C[7] = p0 | 0x40;
          if (J & 0x40)
            C[6] = p1 | 0x40;
          if (J & 0x20)
            C[5] = p2 | 0x40;
          if (J & 0x10)
            C[4] = p3 | 0x40;
          if (J & 0x08)
            C[3] = p4 | 0x40;
          if (J & 0x04)
            C[2] = p5 | 0x40;
          if (J & 0x02)
            C[1] = p6 | 0x40;
          if (J & 0x01)
            C[0] = p7 | 0x40;
        } else {
          if (J & 0x80)
            C[7] = p0;
          if (J & 0x40)
            C[6] = p1;
          if (J & 0x20)
            C[5] = p2;
          if (J & 0x10)
            C[4] = p3;
          if (J & 0x08)
            C[3] = p4;
          if (J & 0x04)
            C[2] = p5;
          if (J & 0x02)
            C[1] = p6;
          if (J & 0x01)
            C[0] = p7;
        }
      } else {
        uint8 p0 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p1 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p2 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p3 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p4 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p5 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p6 = VB[pixdata & 3];
        pixdata >>= 4;
        uint8 p7 = VB[pixdata & 3];

        if (atr & SP_BACK) {
          if (J & 0x80)
            C[0] = p0 | 0x40;
          if (J & 0x40)
            C[1] = p1 | 0x40;
          if (J & 0x20)
            C[2] = p2 | 0x40;
          if (J & 0x10)
            C[3] = p3 | 0x40;
          if (J & 0x08)
            C[4] = p4 | 0x40;
          if (J & 0x04)
            C[5] = p5 | 0x40;
          if (J & 0x02)
            C[6] = p6 | 0x40;
          if (J & 0x01)
            C[7] = p7 | 0x40;
        } else {
          if (J & 0x80)
            C[0] = p0;
          if (J & 0x40)
            C[1] = p1;
          if (J & 0x20)
            C[2] = p2;
          if (J & 0x10)
            C[3] = p3;
          if (J & 0x08)
            C[4] = p4;
          if (J & 0x04)
            C[5] = p5;
          if (J & 0x02)
            C[6] = p6;
          if (J & 0x01)
            C[7] = p7;
        }
      }
    }
  }
  SpriteBlurp = 0;
  spork = 1;
}

static void CopySprites(uint8 *target) {
  uint8 *P = target;

  if (!spork)
    return;
  spork = 0;

  if (rendis & 1)
    return; /* User asked to not display sprites. */

  if (spr_min_x > spr_max_x)
    return;

  /* Optimized: Only iterate through the range where sprites are actually
   * present. */
  int n = spr_min_x & ~3;
  int limit = (spr_max_x + 8 + 3) & ~3;
  if (limit > 256)
    limit = 256;

  for (int i = n; i < limit; i += 4) {
    uint32 t = *(uint32 *)(sprlinebuf + i);

    if (t != 0x80808080) {
#ifdef MSB_FIRST
      if (!(t & 0x80000000)) {
        if (!(t & 0x40000000) || (P[i] & 64))
          P[i] = sprlinebuf[i];
      }
      if (!(t & 0x800000)) {
        if (!(t & 0x400000) || (P[i + 1] & 64))
          P[i + 1] = (sprlinebuf + 1)[i];
      }
      if (!(t & 0x8000)) {
        if (!(t & 0x4000) || (P[i + 2] & 64))
          P[i + 2] = (sprlinebuf + 2)[i];
      }
      if (!(t & 0x80)) {
        if (!(t & 0x40) || (P[i + 3] & 64))
          P[i + 3] = (sprlinebuf + 3)[i];
      }
#else
      if (!(t & 0x80)) {
        if (!(t & 0x40) || (P[i] & 0x40))
          P[i] = sprlinebuf[i];
      }
      if (!(t & 0x8000)) {
        if (!(t & 0x4000) || (P[i + 1] & 0x40))
          P[i + 1] = sprlinebuf[i + 1];
      }
      if (!(t & 0x800000)) {
        if (!(t & 0x400000) || (P[i + 2] & 0x40))
          P[i + 2] = sprlinebuf[i + 2];
      }
      if (!(t & 0x80000000)) {
        if (!(t & 0x40000000) || (P[i + 3] & 0x40))
          P[i + 3] = sprlinebuf[i + 3];
      }
#endif
    }
  }
}

void FCEUPPU_SetVideoSystem(int w) {
  if (w) {
    scanlines_per_frame = dendy ? 262 : 312;
    FSettings.FirstSLine = FSettings.UsrFirstSLine[1];
    FSettings.LastSLine = FSettings.UsrLastSLine[1];
  } else {
    scanlines_per_frame = 262;
    FSettings.FirstSLine = FSettings.UsrFirstSLine[0];
    FSettings.LastSLine = FSettings.UsrLastSLine[0];
  }
}

void FCEUPPU_Init(void) { makeppulut(); }

void FCEUPPU_Reset(void) {
  VRAMBuffer = PPU[0] = PPU[1] = PPU_status = PPU[3] = 0;
  PPUSPL = 0;
  PPUGenLatch = 0;
  RefreshAddr = TempAddr = 0;
  vtoggle = 0;
  ppudead = 2;
  kook = 0;
}

void FCEUPPU_Power(void) {
#ifndef FCEU_LOW_RAM
  int x;
#endif

  memset(NTARAM, 0x00, 0x800);
  memset(PALRAM, 0x00, 0x20);
  memset(UPALRAM, 0x00, 0x03);
  memset(SPRAM, 0x00, 0x100);
  FCEUPPU_Reset();
#ifndef FCEU_LOW_RAM
  for (x = 0x2000; x < 0x4000; x += 8) {
    ARead[x] = A200x;
    BWrite[x] = B2000;
    ARead[x + 1] = A200x;
    BWrite[x + 1] = B2001;
    ARead[x + 2] = A2002;
    BWrite[x + 2] = B2002;
    ARead[x + 3] = A200x;
    BWrite[x + 3] = B2003;
    ARead[x + 4] = A200x; /* A2004; */
    BWrite[x + 4] = B2004;
    ARead[x + 5] = A200x;
    BWrite[x + 5] = B2005;
    ARead[x + 6] = A200x;
    BWrite[x + 6] = B2006;
    ARead[x + 7] = A2007;
    BWrite[x + 7] = B2007;
  }

  BWrite[0x4014] = B4014;
#endif
}

#include <esp_attr.h>

IRAM_ATTR int FCEUPPU_Loop(int skip) {
  /* Needed for Knight Rider, possibly others. */
  if (ppudead) {
    memset(XBuf, 0x80, 256 * 240);
    X6502_Run(scanlines_per_frame * (256 + 85));
    ppudead--;
  } else {
    X6502_Run(256 + 85);
    PPU_status |= 0x80;

    /* Not sure if this is correct.  According to Matt Conte and my own tests,
     * it is. Timing is probably off, though. NOTE:  Not having this here breaks
     * a Super Donkey Kong game.
     */
    PPU[3] = PPUSPL = 0;

    /* I need to figure out the true nature and length of this delay. */
    X6502_Run(12);
#if !defined(TARGET_GNW) || defined(NES_MAPPER_NSF)
    if (GameInfo->type == GIT_NSF)
      DoNSFFrame();
    else
#endif
    {
      if (VBlankON)
        TriggerNMI();
    }
    X6502_Run((scanlines_per_frame - 242) * (256 + 85) - 12);
#ifndef FCEU_NO_OVERCLOCKING
    if (overclock_enabled && vblankscanlines) {
      if (!DMC_7bit || !skip_7bit_overclocking) {
        overclocked = 1;
        X6502_Run(vblankscanlines * (256 + 85) - 12);
        overclocked = 0;
      }
    }
#endif
    PPU_status &= 0x1f;
    X6502_Run(256);

    {
      int x;

      if (ScreenON || SpriteON) {
        if (GameHBIRQHook && ((PPU[0] & 0x38) != 0x18))
          GameHBIRQHook();
        if (PPU_hook)
          for (x = 0; x < 42; x++) {
            PPU_hook(0x2000);
            PPU_hook(0);
          }
        if (GameHBIRQHook2)
          GameHBIRQHook2();
      }
      X6502_Run(85 - 16);
      if (ScreenON || SpriteON) {
        RefreshAddr = TempAddr;
        if (PPU_hook)
          PPU_hook(RefreshAddr & 0x3fff);
      }

      /* Clean this stuff up later. */
      spork = numsprites = 0;
      ResetRL(XBuf);

      X6502_Run(16 - kook);
      kook ^= 1;
    }
#if !defined(TARGET_GNW) || defined(NES_MAPPER_NSF)
    if (GameInfo->type == GIT_NSF)
      X6502_Run((256 + 85) * normal_scanlines);
    else
#endif
#if defined(FRAMESKIP) || defined(TARGET_GNW)
        if (skip) {
      int y;

      y = SPRAM[0];
      y++;

      PPU_status |= 0x20; /* Fixes "Bee 52".  Does it break anything? */
      if (GameHBIRQHook) {
        X6502_Run(256);
        for (scanline = 0; scanline < 240; scanline++) {
          if (ScreenON || SpriteON)
            GameHBIRQHook();
          if (scanline == y && SpriteON)
            PPU_status |= 0x40;
          X6502_Run((scanline == 239) ? 85 : (256 + 85));
        }
      } else if (y < 240) {
        X6502_Run((256 + 85) * y);
        if (SpriteON)
          PPU_status |= 0x40; /* Quick and very dirty hack. */
        X6502_Run((256 + 85) * (240 - y));
      } else
        X6502_Run((256 + 85) * 240);
    } else
#endif
    {
      int x, max, maxref;

      deemp = PPU[1] >> 5;

      /* manual samples can't play correctly with overclocking */
#ifndef FCEU_NO_OVERCLOCKING
      if (DMC_7bit && skip_7bit_overclocking)
        totalscanlines = normal_scanlines;
      else
        totalscanlines =
            normal_scanlines + (overclock_enabled ? extrascanlines : 0);
#else
      totalscanlines = normal_scanlines;
#endif
      for (scanline = 0;
           scanline < totalscanlines;) { /* scanline is incremented in  DoLine.
                                            Evil. :/ */
        deempcnt[deemp]++;
        DoLine();
#ifndef FCEU_NO_OVERCLOCKING
        if (scanline < normal_scanlines || scanline == totalscanlines) {
          overclocked = 0;
        } else {
          if (DMC_7bit &&
              skip_7bit_overclocking) /* 7bit sample started after 240th line */
            break;
          overclocked = 1;
        }
#endif
      }

#ifndef FCEU_NO_OVERCLOCKING
      DMC_7bit = 0;
#endif
      if (MMC5Hack && (ScreenON || SpriteON))
        MMC5_hb(scanline);
      for (x = 1, max = 0, maxref = 0; x < 7; x++) {
        if (deempcnt[x] > max) {
          max = deempcnt[x];
          maxref = x;
        }
        deempcnt[x] = 0;
      }
      SetNESDeemph(maxref, 0);
    }
  }

#if defined(FRAMESKIP) || defined(TARGET_GNW)
  if (skip) {
    FCEU_PutImageDummy();
    return (0);
  } else
#endif
  {
    FCEU_PutImage();
    return (1);
  }
}

static uint16 TempAddrT, RefreshAddrT;

void FCEUPPU_LoadState(int version) {
  TempAddr = TempAddrT;
  RefreshAddr = RefreshAddrT;
}

SFORMAT FCEUPPU_STATEINFO[] = {{NTARAM, 0x800, "NTAR"},
                               {PALRAM, 0x20, "PRAM"},
                               {SPRAM, 0x100, "SPRA"},
                               {PPU, 0x4, "PPUR"},
                               {&kook, 1, "KOOK"},
                               {&ppudead, 1, "DEAD"},
                               {&PPUSPL, 1, "PSPL"},
                               {&XOffset, 1, "XOFF"},
                               {&vtoggle, 1, "VTGL"},
                               {&RefreshAddrT, 2 | FCEUSTATE_RLSB, "RADD"},
                               {&TempAddrT, 2 | FCEUSTATE_RLSB, "TADD"},
                               {&VRAMBuffer, 1, "VBUF"},
                               {&PPUGenLatch, 1, "PGEN"},
                               {0}};

void FCEUPPU_SaveState(void) {
  TempAddrT = TempAddr;
  RefreshAddrT = RefreshAddr;
}
#endif