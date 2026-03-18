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

#include <string.h>

#include "fceu-state.h"
#include "fceu-types.h"
#include "fceu.h"
#include "input.h"
#include "vsuni.h"
#include "x6502.h"


#define IOPTION_GUN 0x01
#define IOPTION_SWAPDIRAB 0x02
#define IOPTION_PREDIP 0x10

typedef struct {
  char *name;
  int gameid;
  uint64 md5partial;
  int mapper;
  int mirroring;
  int ppu;
  int ioption;
  int predip;
} VSUNIENTRY;

VSUNIENTRY *curvs;

static uint8 DIPS = 0;
extern uint8 vsdip;

void FCEUI_VSUniToggleDIPView(void) { DIPS = !DIPS; }

void FCEU_VSUniToggleDIP(int w) { vsdip ^= 1 << w; }

void FCEUI_VSUniSetDIP(int w, int state) {
  if (((vsdip >> w) & 1) != state)
    FCEUI_VSUniToggleDIP(w);
}

uint8 FCEUI_VSUniGetDIPs(void) { return (vsdip); }

static uint8 secdata[2][32] = {
    {0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f, 0x6f, 0x6b, 0xeb,
     0xa9, 0xb1, 0x90, 0x94, 0x14, 0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9,
     0xb1, 0x90, 0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x6F, 0x00,
     0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static uint8 *secptr;

static uint8 VSindex;

static DECLFR(VSSecRead) {
  switch (A) {
  case 0x5e00:
    VSindex = 0;
    return X.DB;
  case 0x5e01:
    return (secptr[(VSindex++) & 0x1F]);
  }
  return (0x00);
}
uint8 coinon = 0;

void FCEU_VSUniCoin(void) { coinon = 6; }

static int curppu;
static int64 curmd5;

#define RP2C04_0001 1
#define RP2C04_0002 2
#define RP2C04_0003 3
#define RP2C04_0004 4
#define RCP2C03B 5
#define RC2C05_01 6
#define RC2C05_02 7
#define RC2C05_03 8
#define RC2C05_04 9

static readfunc OldReadPPU;
static writefunc OldWritePPU[2];

static DECLFR(A2002_Gumshoe) { return ((OldReadPPU(A) & ~0x3F) | 0x1C); }

static DECLFR(A2002_Topgun) { return ((OldReadPPU(A) & ~0x3F) | 0x1B); }

static DECLFR(A2002_MBJ) { /* Mighty Bomb Jack */
  return ((OldReadPPU(A) & ~0x3F) | 0x3D);
}

static DECLFW(B2000_2001_2C05) { OldWritePPU[(A & 1) ^ 1](A ^ 1, V); }
static uint8 xevselect = 0;
static DECLFR(XevRead) {
  if (A == 0x54FF) {
    return (0x5);
  } else if (A == 0x5678) {
    return (xevselect ? 0 : 1);
  } else if (A == 0x578F) {
    return (xevselect ? 0xd1 : 0x89);
  } else if (A == 0x5567) {
    xevselect ^= 1;
    return (xevselect ? 0x37 : 0x3E);
  }
  return (X.DB);
}

void FCEU_VSUniSwap(uint8 *j0, uint8 *j1) {
  if (curvs->ioption & IOPTION_SWAPDIRAB) {
    uint16 t = *j0;
    *j0 = (*j0 & 0xC) | (*j1 & 0xF3);
    *j1 = (*j1 & 0xC) | (t & 0xF3);
  }
}

void FCEU_VSUniPower(void) {
  coinon = 0;
  VSindex = 0;

  if (secptr)
    SetReadHandler(0x5e00, 0x5e01, VSSecRead);

  if (curppu == RC2C05_04) {
    OldReadPPU = GetReadHandler(0x2002);
    SetReadHandler(0x2002, 0x2002, A2002_Topgun);
  } else if (curppu == RC2C05_03) {
    OldReadPPU = GetReadHandler(0x2002);
    SetReadHandler(0x2002, 0x2002, A2002_Gumshoe);
  } else if (curppu == RC2C05_02) {
    OldReadPPU = GetReadHandler(0x2002);
    SetReadHandler(0x2002, 0x2002, A2002_MBJ);
  }
  if (curppu == RC2C05_04 || curppu == RC2C05_01 || curppu == RC2C05_03 ||
      curppu == RC2C05_02) {
    OldWritePPU[0] = GetWriteHandler(0x2000);
    OldWritePPU[1] = GetWriteHandler(0x2001);
    SetWriteHandler(0x2000, 0x2001, B2000_2001_2C05);
  }
  if (curmd5 == 0x2d396247cf58f9faLL) { /* Super Xevious */
    SetReadHandler(0x5400, 0x57FF, XevRead);
  }
}

/* Games that will probably not be supported ever(or for a long time), since
they require dual-system:

 Balloon Fight
 VS Mahjong
 VS Tennis
 Wrecking Crew
*/

/* Games/PPU list.  Information copied from MAME.  ROMs are exchangable, so
don't take this list as "this game must use this PPU".

RP2C04-0001:
- Baseball
- Freedom Force
- Gradius
- Hogan's Alley
- Mach Rider (Japan, Fighting Course)
- Pinball
- Platoon
- Super Xevious

RP2C04-0002:
- Castlevania
- Ladies Golf
- Mach Rider (Endurance Course)
- Raid on Bungeling Bay (Japan)
- Slalom
- Stroke N' Match Golf
- Wrecking Crew

RP2C04-0003:
- Dr. Mario
- Excite Bike
- Goonies
- Soccer
- TKO Boxing

RP2C04-0004:
- Clu Clu Land
- Excite Bike (Japan)
- Ice Climber
- Ice Climber Dual (Japan)
- Super Mario Bros.

RCP2C03B:
- Battle City
- Duck Hunt
- Mahjang
- Pinball (Japan)
- Rbi Baseball
- Star Luster
- Stroke and Match Golf (Japan)
- Super Skykid
- Tennis
- Tetris

RC2C05-01:
- Ninja Jajamaru Kun (Japan)

RC2C05-02:
- Mighty Bomb Jack (Japan)

RC2C05-03:
- Gumshoe

RC2C05-04:
- Top Gun
*/

VSUNIENTRY VSUniGames[] = {
    {"Baseball", VS_BASEBALL, 0x691d4200ea42be45LL, 99, 2, RP2C04_0001, 0, 0},
    {"Battle City", VS_BATTLECITY, 0x8540949d74c4d0ebLL, 99, 2, RP2C04_0001, 0,
     0},
    {"Battle City(Bootleg)", VS_BATTLECITY, 0x8093cbe7137ac031LL, 99, 2,
     RP2C04_0001, 0, 0},
    {"Clu Clu Land", VS_CLUCLULAND, 0x1b8123218f62b1eeLL, 99, 2, RP2C04_0004,
     IOPTION_SWAPDIRAB, 0},
    {"Dr Mario", VS_DRMARIO, 0xe1af09c477dc0081LL, 1, 0, RP2C04_0003,
     IOPTION_SWAPDIRAB, 0},
    {"Duck Hunt", VS_DUCKHUNT, 0x47735d1e5f1205bbLL, 99, 2, RCP2C03B,
     IOPTION_GUN, 0},
    {"Excitebike", VS_EXITEBIKE, 0x3dcd1401bcafde77LL, 99, 2, RP2C04_0003, 0,
     0},
    {"Excitebike (J)", VS_EXITEBIKE, 0x7ea51c9d007375f0LL, 99, 2, RP2C04_0004,
     0, 0},
    {"Freedom Force", VS_FREEDOMFORCE, 0xed96436bd1b5e688LL, 4, 0, RP2C04_0001,
     IOPTION_GUN, 0}, /* Wrong color in game select screen? */
    {"Stroke and Match Golf", VS_STROKEANDMATCHGOLF, 0x612325606e82bc66LL, 99,
     2, RP2C04_0002, IOPTION_SWAPDIRAB | IOPTION_PREDIP, 0x01},
    {"Goonies", VS_GOONIES, 0xb4032d694e1d2733LL, 151, 1, RP2C04_0003, 0, 0},
    {"Gradius", VS_GRADIUS, 0x50687ae63bdad976LL, 151, 1, RP2C04_0001,
     IOPTION_SWAPDIRAB, 0},
    {"Gumshoe", VS_GUMSHOE, 0x87161f8ee37758d3LL, 99, 2, RC2C05_03, IOPTION_GUN,
     0},
    {"Gumshoe", VS_GUMSHOE, 0xb8500780bf69ce29LL, 99, 2, RC2C05_03, IOPTION_GUN,
     0},
    {"Gumshoe", VS_GUMSHOE, 0xa6bf132ba11d0a8cLL, 99, 2, RC2C05_03, IOPTION_GUN,
     0}, /* Gumshoe (VS).nes added: 2017-9-5*/
    {"Hogan's Alley", VS_HOGANSALLEY, 0xd78b7f0bb621fb45LL, 99, 2, RP2C04_0001,
     IOPTION_GUN, 0},
    {"Ice Climber", VS_ICECLIMBER, 0xd21e999513435e2aLL, 99, 2, RP2C04_0004,
     IOPTION_SWAPDIRAB, 0},
    {"Ladies Golf", VS_LADIESGOLF, 0x781b24be57ef6785LL, 99, 2, RP2C04_0002,
     IOPTION_SWAPDIRAB | IOPTION_PREDIP, 0x1},
    {"Mach Rider", VS_MACHRIDER, 0x015672618af06441LL, 99, 2, RP2C04_0002, 0,
     0},
    {"Mach Rider (J)", VS_MACHRIDER, 0xa625afb399811a8aLL, 99, 2, RP2C04_0001,
     0, 0},
    {"Mighty Bomb Jack", VS_MIGHTYBOMBJACK, 0xe6a89f4873fac37bLL, 0, 2,
     RC2C05_02, 0, 0},
    {"Ninja Jajamaru Kun", VS_JAJAMARU, 0xb26a2c31474099c0LL, 99, 2, RC2C05_01,
     IOPTION_SWAPDIRAB, 0},
    {"Pinball", VS_PINBALL, 0xc5f49d3de7f2e9b8LL, 99, 2, RP2C04_0001,
     IOPTION_PREDIP, 0x01},
    {"Pinball (J)", VS_PINBALL, 0x66ab1a3828cc901cLL, 99, 2, RCP2C03B,
     IOPTION_PREDIP, 0x1},
    {"Platoon", VS_PLATOON, 0x160f237351c19f1fLL, 68, 1, RP2C04_0001, 0, 0},
    {"RBI Baseball", VS_RBIBASEBALL, 0x6a02d345812938afLL, 4, 1, RP2C04_0001,
     IOPTION_SWAPDIRAB, 0},
    {"Soccer", VS_SOCCER, 0xd4e7a9058780eda3LL, 99, 2, RP2C04_0003,
     IOPTION_SWAPDIRAB, 0},
    {"Star Luster", VS_STARLUSTER, 0x8360e134b316d94cLL, 99, 2, RCP2C03B, 0, 0},
    {"Stroke and Match Golf (J)", VS_STROKEANDMATCHGOLF, 0x869bb83e02509747LL,
     99, 2, RCP2C03B, IOPTION_SWAPDIRAB | IOPTION_PREDIP, 0x1},
    {"Super Sky Kid", VS_SUPERSKYKID, 0x78d04c1dd4ec0101LL, 4, 1, RCP2C03B,
     IOPTION_SWAPDIRAB | IOPTION_PREDIP, 0x20},
    {"Super Xevious", VS_SUPERXEVIOUS, 0x2d396247cf58f9faLL, 206, 0,
     RP2C04_0001, 0, 0},
    {"Tetris", VS_TETRIS, 0x531a5e8eea4ce157LL, 99, 2, RCP2C03B, IOPTION_PREDIP,
     0x20},
    {"Top Gun", VS_TOPGUN, 0xf1dea36e6a7b531dLL, 2, 0, RC2C05_04, 0, 0},
    {"VS Castlevania", VS_CASTLEVANIA, 0x92fd6909c81305b9LL, 2, 1, RP2C04_0002,
     0, 0},
    {"VS Slalom", VS_SLALOM, 0x4889b5a50a623215LL, 0, 1, RP2C04_0002, 0, 0},
    {"VS Super Mario Bros", VS_SMB, 0x39d8cfa788e20b6cLL, 99, 2, RP2C04_0004, 0,
     0},
    {"VS Super Mario Bros [a1]", VS_SMB, 0xfc182e5aefbce14dLL, 99, 2,
     RP2C04_0004, 0, 0},
    {"VS TKO Boxing", VS_TKOBOXING, 0x6e1ee06171d8ce3aLL, 206, 1, RP2C04_0003,
     IOPTION_PREDIP, 0x00},
    {"VS TKO Boxing [a1]", VS_TKOBOXING, 0xa1c694ce147bc1edLL, 206, 1,
     RP2C04_0003, IOPTION_PREDIP, 0x00},
    {0}};

void FCEU_VSUniCheck(uint64 md5partial, int *MapperNo, int *Mirroring) {
  VSUNIENTRY *vs = VSUniGames;

  while (vs->name) {
    if (md5partial == vs->md5partial) {
      if (vs->ppu < RCP2C03B)
        pale = vs->ppu;
      else
        pale = 5;
      *MapperNo = vs->mapper;
      *Mirroring = vs->mirroring;
      GameInfo->type = GIT_VSUNI;
      GameInfo->cspecial = SIS_VSUNISYSTEM;
      GameInfo->inputfc = SIFC_NONE;
      GameInfo->gameid = vs->gameid;
      curppu = vs->ppu;
      curmd5 = md5partial;

      FCEU_printf(" System: VS-UniSystem\n");
      FCEU_printf(" Name: %s\n", vs->name);

      secptr = 0;

      if (vs->gameid == VS_TKOBOXING)
        secptr = secdata[0];
      if (vs->gameid == VS_RBIBASEBALL)
        secptr = secdata[1];

      vsdip = 0x0;
      if (vs->ioption & IOPTION_PREDIP) {
        vsdip = vs->predip;
      }
      if (vs->ioption & IOPTION_GUN) {
        GameInfo->input[0] = SI_ZAPPER;
        GameInfo->input[1] = SI_NONE;
      } else {
        GameInfo->input[0] = GameInfo->input[1] = SI_GAMEPAD;
      }
      curvs = vs;
      return;
    }
    vs++;
  }
}

void FCEU_VSUniDraw(uint8 *XBuf) {
  uint32 *dest;
  int y, x;

  if (!DIPS)
    return;

  dest = (uint32 *)(XBuf + 256 * 12 + 164);
  for (y = 24; y; y--, dest += (256 - 72) >> 2) {
    for (x = 72 >> 2; x; x--, dest++)
      *dest = 0;
  }

  dest = (uint32 *)(XBuf + 256 * (12 + 4) + 164 + 6);
  for (y = 16; y; y--, dest += (256 >> 2) - 16)
    for (x = 8; x; x--) {
      *dest = 0x01010101;
      dest += 2;
    }

  dest = (uint32 *)(XBuf + 256 * (12 + 4) + 164 + 6);
  for (x = 0; x < 8; x++, dest += 2) {
    uint32 *da = dest + (256 >> 2);

    if (!((vsdip >> x) & 1))
      da += (256 >> 2) * 10;
    for (y = 4; y; y--, da += 256 >> 2)
      *da = 0;
  }
}

SFORMAT FCEUVSUNI_STATEINFO[] = {
    {&vsdip, 1, "vsdp"}, {&coinon, 1, "vscn"}, {&VSindex, 1, "vsin"}, {0}};
