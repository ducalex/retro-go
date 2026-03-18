#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) ||                                                    \
    (defined(TARGET_GNW) && defined(ENABLE_EMULATOR_NES) &&                    \
     FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2002 Xodnizel
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

// FORCE ENABLE MAPPERS (Manual Override)
#define NES_MAPPER_000 // NROM
#define NES_MAPPER_001 // MMC1 (Zelda, Metroid, etc.)
#define NES_MAPPER_002 // UNROM (Castlevania, Contra, etc.)
#define NES_MAPPER_003 // CNROM
#define NES_MAPPER_004 // MMC3 (Mario 3, Kirby, etc.)
#define NES_MAPPER_005 // MMC5 (Castlevania 3)
#define NES_MAPPER_007 // AxROM (Battletoads)
#define NES_MAPPER_009 // MMC2 (Punch-Out!!)
#define NES_MAPPER_010 // MMC4
#define NES_MAPPER_011 // Color Dreams
#define NES_MAPPER_013 // CPROM
#define NES_MAPPER_015 // 100-in-1
#define NES_MAPPER_016 // Bandai (Dragon Ball Z)
#define NES_MAPPER_018 // Jaleco SS88006
#define NES_MAPPER_019 // Namco 163
#define NES_MAPPER_021 // VRC4
#define NES_MAPPER_022 // VRC2
#define NES_MAPPER_023 // VRC2/4
#define NES_MAPPER_024 // VRC6 (Akumajou Densetsu)
#define NES_MAPPER_025 // VRC4
#define NES_MAPPER_026 // VRC6
#define NES_MAPPER_032 // Irem G-101
#define NES_MAPPER_033 // Taito TC0190
#define NES_MAPPER_034 // BNROM/NINA-001
#define NES_MAPPER_040 // SMB2j FDS
#define NES_MAPPER_041 // Caltron 6-in-1
#define NES_MAPPER_042 // Bio Miracle FDS
#define NES_MAPPER_043 // SMB2j FDS
#define NES_MAPPER_048 // Taito TC190V
#define NES_MAPPER_050 // SMB2j FDS
#define NES_MAPPER_051 // 11-in-1
#define NES_MAPPER_052 // Mario 7-in-1
#define NES_MAPPER_057 // GK 6-in-1
#define NES_MAPPER_058 // 68-in-1
#define NES_MAPPER_060 // Reset-based 4-in-1
#define NES_MAPPER_061 // 20-in-1
#define NES_MAPPER_062 // 700-in-1
#define NES_MAPPER_064 // RAMBO-1
#define NES_MAPPER_065 // Irem H-3001
#define NES_MAPPER_066 // GNROM
#define NES_MAPPER_067 // Sunsoft-3
#define NES_MAPPER_068 // Sunsoft-4
#define NES_MAPPER_069 // FME-7 (Batman Return of the Joker)
#define NES_MAPPER_070 // Bandai
#define NES_MAPPER_071 // Camerica
#define NES_MAPPER_072 // Jaleco
#define NES_MAPPER_073 // VRC3
#define NES_MAPPER_075 // VRC1
#define NES_MAPPER_076 // Namco 109
#define NES_MAPPER_077 // Napoleon Senki
#define NES_MAPPER_078 // Irem 74HC161/32
#define NES_MAPPER_079 // NINA-03
#define NES_MAPPER_080 // Taito X-005
#define NES_MAPPER_082 // Taito X1-17
#define NES_MAPPER_085 // VRC7 (Lagrange Point)
#define NES_MAPPER_086 // Jaleco
#define NES_MAPPER_087 // Jaleco
#define NES_MAPPER_088 // Namco 118
#define NES_MAPPER_089 // Sunsoft-2
#define NES_MAPPER_090 // Tekken 2 / Mortal Kombat
#define NES_MAPPER_091 // Street Fighter III
#define NES_MAPPER_092 // Jaleco
#define NES_MAPPER_093 // Sunsoft-2
#define NES_MAPPER_094 // UN1ROM
#define NES_MAPPER_095 // Namco 118
#define NES_MAPPER_097 // Kaiketsu Yanchamaru
#define NES_MAPPER_099 // VS Unisystem
#define NES_MAPPER_105 // Nintendo World Championship
#define NES_MAPPER_112 // Asder
#define NES_MAPPER_113 // NINA-06
#define NES_MAPPER_114 // Super Donkey Kong
#define NES_MAPPER_115 // Yuu Yuu Hakusho Final
#define NES_MAPPER_118 // TxSROM
#define NES_MAPPER_119 // TQROM
#define NES_MAPPER_140 // Bio Senshi Dan
#define NES_MAPPER_144 // Death Race
#define NES_MAPPER_151 // VS Unisystem
#define NES_MAPPER_152 // Arkanoid 2
#define NES_MAPPER_153 // Bandai
#define NES_MAPPER_154 // Namco
#define NES_MAPPER_155 // MMC1A
#define NES_MAPPER_156 // Buzz & Waldog
#define NES_MAPPER_157 // Datach
#define NES_MAPPER_159 // Bandai
#define NES_MAPPER_163 // Nanjing
#define NES_MAPPER_164 // Waixing
#define NES_MAPPER_165 // Waixing
#define NES_MAPPER_180 // Crazy Climber
#define NES_MAPPER_184 // Sunsoft-1
#define NES_MAPPER_185 // CNROM-Protection
#define NES_MAPPER_188 // Karaoke Studio
#define NES_MAPPER_189 // Street Fighter 2
#define NES_MAPPER_193 // Fighting Hero
#define NES_MAPPER_206 // DxROM
#define NES_MAPPER_207 // Fudou Myouou Den
#define NES_MAPPER_209 // Shin Samurai Spirits 2
#define NES_MAPPER_210 // Namco 163
#define NES_MAPPER_225 // 52-in-1
#define NES_MAPPER_226 // 76-in-1
#define NES_MAPPER_227 // 1200-in-1
#define NES_MAPPER_228 // Action 52
#define NES_MAPPER_229 // 31-in-1
#define NES_MAPPER_230 // 22-in-1
#define NES_MAPPER_231 // 20-in-1
#define NES_MAPPER_232 // Quattro Arcade
#define NES_MAPPER_233 // 42-in-1
#define NES_MAPPER_234 // Maxi 15
#define NES_MAPPER_235 // 150-in-1
#define NES_MAPPER_240 // Gen Ke Xin Zhuan
#define NES_MAPPER_241 // Education
#define NES_MAPPER_242 // Wai Xing Zhan Shi
#define NES_MAPPER_243 // Poker III
#define NES_MAPPER_244 // Decathlon
#define NES_MAPPER_245 // Yong Zhe Dou E Long
#define NES_MAPPER_246 // Fong Shen Bang
#define NES_MAPPER_255 // 110-in-1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fceu-cart.h"
#include "fceu-types.h"
#include "fceu.h"
#include "ppu.h"
#include "x6502.h"

#include "fceu-memory.h"
#include "fceu-state.h"
#include "file.h"
#include "general.h"
#include "ines.h"
#include "unif.h"

#ifndef TARGET_GNW
#include "fceu-crc32.h"
#else
#include "crc32.h"
#include <esp_rom_crc.h>
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif
#ifndef LINUX_EMU
#include "build/mappers.h"
#endif
#endif
#include "cheat.h"
#include "md5.h"
#include "vsuni.h"

#ifndef TARGET_GNW
extern SFORMAT FCEUVSUNI_STATEINFO[];
#endif

uint8 *trainerpoo = NULL;
uint8 *ROM = NULL;
uint8 *VROM = NULL;
uint8 *ExtraNTARAM = NULL;
iNES_HEADER head = {0};

CartInfo iNESCart = {0};

uint32 ROM_size = 0;
uint32 VROM_size = 0;

static int CHRRAMSize = -1;

static int iNES_Init(int num);

static DECLFR(TrainerRead) { return (trainerpoo[A & 0x1FF]); }

static void iNES_ExecPower() {
  if (iNESCart.Power)
    iNESCart.Power();

  if (trainerpoo) {
    int x;
    for (x = 0; x < 512; x++) {
      X6502_DMW(0x7000 + x, trainerpoo[x]);
      if (X6502_DMR(0x7000 + x) != trainerpoo[x]) {
        SetReadHandler(0x7000, 0x71FF, TrainerRead);
        break;
      }
    }
  }
}

static void iNESGI(int h) {
  switch (h) {
  case GI_RESETM2:
    if (iNESCart.Reset)
      iNESCart.Reset();
    break;
  case GI_POWER:
    iNES_ExecPower();
    break;
  case GI_CLOSE:
    if (iNESCart.Close)
      iNESCart.Close();
    if (ROM) {
      free(ROM);
      ROM = NULL;
    }
    if (VROM) {
      free(VROM);
      VROM = NULL;
    }
    if (trainerpoo) {
      free(trainerpoo);
      trainerpoo = NULL;
    }
    if (ExtraNTARAM) {
      free(ExtraNTARAM);
      ExtraNTARAM = NULL;
    }
    break;
  }
}

struct CRCMATCH {
  uint32 crc;
  char *name;
};

struct INPSEL {
  uint32 crc32;
  int input1;
  int input2;
  int inputfc;
};

static void SetInput(void) {
  static struct INPSEL moo[] = {
      {0x19b0a9f1, SI_GAMEPAD, SI_ZAPPER,
       SIFC_NONE}, /* 6-in-1 (MGC-023)(Unl)[!] */
      {0x29de87af, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB},                                /* Aerobics Studio */
      {0xd89e5a67, SI_UNSET, SI_UNSET, SIFC_ARKANOID}, /* Arkanoid (J) */
      {0x0f141525, SI_UNSET, SI_UNSET, SIFC_ARKANOID}, /* Arkanoid 2(J) */
      {0x32fb0583, SI_UNSET, SI_ARKANOID, SIFC_NONE},  /* Arkanoid(NES) */
      {0x60ad090a, SI_GAMEPAD, SI_GAMEPAD, SIFC_FTRAINERA}, /* Athletic World */
      {0x48ca0ee1, SI_GAMEPAD, SI_GAMEPAD, SIFC_BWORLD},    /* Barcode World */
      {0x4318a2f8, SI_UNSET, SI_ZAPPER,
       SIFC_NONE}, /* Barker Bill's Trick Shooting */
      {0x6cca1c1f, SI_GAMEPAD, SI_GAMEPAD, SIFC_FTRAINERB}, /* Dai Undoukai */
      {0x24598791, SI_UNSET, SI_ZAPPER, SIFC_NONE},         /* Duck Hunt */
      {0xd5d6eac4, SI_UNSET, SI_UNSET, SIFC_SUBORKB},       /* Edu (As) */
      {0xe9a7fe9e, SI_UNSET, SI_MOUSE,
       SIFC_NONE}, /* Educational Computer 2000 */
      {0x8f7b1669, SI_UNSET, SI_UNSET,
       SIFC_SUBORKB}, /* FP BASIC 3.3 by maxzhou88 */
      {0xf7606810, SI_UNSET, SI_UNSET, SIFC_FKB}, /* Family BASIC 2.0A */
      {0x895037bc, SI_UNSET, SI_UNSET, SIFC_FKB}, /* Family BASIC 2.1a */
      {0xb2530afc, SI_UNSET, SI_UNSET, SIFC_FKB}, /* Family BASIC 3.0 */
      {0xea90f3e2, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB}, /* Family Trainer:  Running Stadium */
      {0xbba58be5, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB}, /* Family Trainer: Manhattan Police */
      {0x3e58a87e, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Freedom Force */
      {0xd9f45be9, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_QUIZKING}, /* Gimme a Break ... */
      {0x1545bd13, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_QUIZKING},                              /* Gimme a Break ... 2 */
      {0x4e959173, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Gotcha! - The Sport! */
      {0xbeb8ab01, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Gumshoe */
      {0xff24d794, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Hogan's Alley */
      {0x21f85681, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_HYPERSHOT}, /* Hyper Olympic (Gentei Ban) */
      {0x980be936, SI_GAMEPAD, SI_GAMEPAD, SIFC_HYPERSHOT}, /* Hyper Olympic */
      {0x915a53a7, SI_GAMEPAD, SI_GAMEPAD, SIFC_HYPERSHOT}, /* Hyper Sports */
      {0x9fae4d46, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_MAHJONG}, /* Ide Yousuke Meijin no Jissen Mahjong */
      {0x7b44fb2a, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_MAHJONG}, /* Ide Yousuke Meijin no Jissen Mahjong 2 */
      {0x2f128512, SI_GAMEPAD, SI_GAMEPAD, SIFC_FTRAINERA}, /* Jogging Race */
      {0xbb33196f, SI_UNSET, SI_UNSET, SIFC_FKB},     /* Keyboard Transformer */
      {0x8587ee00, SI_UNSET, SI_UNSET, SIFC_FKB},     /* Keyboard Transformer */
      {0x543ab532, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* LIKO Color Lines */
      {0x368c19a8, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* LIKO Study Cartridge */
      {0x5ee6008e, SI_UNSET, SI_ZAPPER, SIFC_NONE},   /* Mechanized Attack */
      {0x370ceb65, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB}, /* Meiro Dai Sakusen */
      {0x3a1694f9, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_4PLAYER}, /* Nekketsu Kakutou Densetsu */
      {0x9d048ea4, SI_GAMEPAD, SI_GAMEPAD, SIFC_OEKAKIDS}, /* Oeka Kids */
      {0x2a6559a1, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Operation Wolf (J) */
      {0xedc3662b, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Operation Wolf */
      {0x912989dc, SI_UNSET, SI_UNSET, SIFC_FKB},   /* Playbox BASIC */
      {0x9044550e, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERA}, /* Rairai Kyonshizu */
      {0xea90f3e2, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB},                               /* Running Stadium */
      {0x851eb9be, SI_GAMEPAD, SI_ZAPPER, SIFC_NONE}, /* Shooting Range */
      {0x6435c095, SI_GAMEPAD, SI_POWERPADB,
       SIFC_UNSET}, /* Short Order/Eggsplode */
      {0xc043a8df, SI_UNSET, SI_MOUSE,
       SIFC_NONE}, /* Shu Qi Yu - Shu Xue Xiao Zhuan Yuan (Ch) */
      {0x2cf5db05, SI_UNSET, SI_MOUSE,
       SIFC_NONE}, /* Shu Qi Yu - Zhi Li Xiao Zhuan Yuan (Ch) */
      {0xad9c63e2, SI_GAMEPAD, SI_UNSET, SIFC_SHADOW},    /* Space Shadow */
      {0x61d86167, SI_GAMEPAD, SI_POWERPADB, SIFC_UNSET}, /* Street Cop */
      {0xabb2f974, SI_UNSET, SI_UNSET,
       SIFC_SUBORKB}, /* Study and Game 32-in-1 */
      {0x41ef9ac4, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* Subor */
      {0x8b265862, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* Subor */
      {0x82f1fb96, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* Subor 1.0 Russian */
      {0x9f8f200a, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERA}, /* Super Mogura Tataki!! - Pokkun Moguraa */
      {0xd74b2719, SI_GAMEPAD, SI_POWERPADB, SIFC_UNSET}, /* Super Team Games */
      {0x74bea652, SI_GAMEPAD, SI_ZAPPER, SIFC_NONE},     /* Supergun 3-in-1 */
      {0x5e073a1b, SI_UNSET, SI_UNSET,
       SIFC_SUBORKB}, /* Supor English (Chinese) */
      {0x589b6b0d, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* SuporV20 */
      {0x41401c6d, SI_UNSET, SI_UNSET, SIFC_SUBORKB}, /* SuporV40 */
      {0x23d17f5e, SI_GAMEPAD, SI_ZAPPER, SIFC_NONE}, /* The Lone Ranger */
      {0xc3c0811d, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_OEKAKIDS}, /* The two "Oeka Kids" games */
      {0xde8fd935, SI_UNSET, SI_ZAPPER, SIFC_NONE},        /* To the Earth */
      {0x47232739, SI_GAMEPAD, SI_GAMEPAD, SIFC_TOPRIDER}, /* Top Rider */
      {0x8a12a7d9, SI_GAMEPAD, SI_GAMEPAD,
       SIFC_FTRAINERB}, /* Totsugeki Fuuun Takeshi Jou */
      {0xb8b9aca3, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Wild Gunman */
      {0x5112dc21, SI_UNSET, SI_ZAPPER, SIFC_NONE}, /* Wild Gunman */
      {0xaf4010ea, SI_GAMEPAD, SI_POWERPADB,
       SIFC_UNSET}, /* World Class Track Meet */
      {0xb3cc4d26, SI_GAMEPAD, SI_UNSET,
       SIFC_SHADOW}, /* 2-in-1 Uzi Lightgun (MGC-002) */

      {0x00000000, SI_UNSET, SI_UNSET, SIFC_UNSET}};
  int x = 0;

  while (moo[x].input1 >= 0 || moo[x].input2 >= 0 || moo[x].inputfc >= 0) {
    if (moo[x].crc32 == iNESCart.CRC32) {
      GameInfo->input[0] = moo[x].input1;
      GameInfo->input[1] = moo[x].input2;
      GameInfo->inputfc = moo[x].inputfc;
      break;
    }
    x++;
  }
}

#define INESB_INCOMPLETE 1
#define INESB_CORRUPT 2
#define INESB_HACKED 4

struct BADINF {
  uint64 md5partial;
  uint8 *name;
  uint32 type;
};

static struct BADINF BadROMImages[] = {
#include "ines-bad.h"
};

static void CheckBad(uint64 md5partial) {
  int32 x = 0;
  while (BadROMImages[x].name) {
    if (BadROMImages[x].md5partial == md5partial) {
      FCEU_PrintError("The copy game you have loaded, \"%s\", is bad, and will "
                      "not work properly in FCE Ultra.",
                      BadROMImages[x].name);
      return;
    }
    x++;
  }
}

struct CHINF {
  uint32 crc32;
  int32 mapper;
  int32 submapper;
  int32 mirror;
  int32 battery;
  int32 prgram; /* ines2 prgram format */
  int32 chrram; /* ines2 chrram format */
  int32 region;
  int32 extra;
};

static void CheckHInfo(void) {
#define DEFAULT (-1)
#define NOEXTRA (-1)

  /* used for mirroring special overrides */
#define MI_4 2 /* forced 4-screen mirroring */
#define DFAULT8                                                                \
  8 /* anything but hard-wired (4-screen) mirroring, mapper-controlled */

  /* tv system/region */
#define PAL 1
#define MULTI 2
#define DENDY 3

  static struct CHINF moo[] = {
#include "ines-correct.h"
  };
  int32 tofix = 0, x;
  uint64 partialmd5 = 0;
  int32 current_mapper = 0;
  int32 cur_mirr = 0;

  for (x = 0; x < 8; x++)
    partialmd5 |= (uint64)iNESCart.MD5[15 - x] << (x * 8);
  CheckBad(partialmd5);

  x = 0;
  do {
    if (moo[x].crc32 == iNESCart.CRC32) {
      if (moo[x].mapper >= 0) {
        if (moo[x].extra >= 0 && moo[x].extra == 0x800 && VROM_size) {
          VROM_size = 0;
#ifndef TARGET_GNW
          free(VROM);
#endif
          VROM = NULL;
          tofix |= 8;
        }
        if (iNESCart.mapper != (moo[x].mapper & 0xFFF)) {
          tofix |= 1;
          current_mapper = iNESCart.mapper;
          iNESCart.mapper = moo[x].mapper & 0xFFF;
        }
      }
      if (moo[x].submapper >= 0) {
        iNESCart.iNES2 = 1;
        if (moo[x].submapper != iNESCart.submapper) {
          iNESCart.submapper = moo[x].submapper;
        }
      }
      if (moo[x].mirror >= 0) {
        cur_mirr = iNESCart.mirror;
        if (moo[x].mirror == 8) {
          if (iNESCart.mirror ==
              2) { /* Anything but hard-wired(four screen). */
            tofix |= 2;
            iNESCart.mirror = 0;
          }
        } else if (iNESCart.mirror != moo[x].mirror) {
          if (iNESCart.mirror != (moo[x].mirror & ~4))
            if ((moo[x].mirror & ~4) <= 2) /* Don't complain if one-screen
                                         mirroring needs to be set(the iNES
                                         header can't hold this information).
                                         */
              tofix |= 2;
          iNESCart.mirror = moo[x].mirror;
        }
      }
      if (moo[x].battery >= 0) {
        if (!(head.ROM_type & 2) && (moo[x].battery != 0)) {
          tofix |= 4;
          head.ROM_type |= 2;
        }
      }
      if (moo[x].region >= 0) {
        if (iNESCart.region != moo[x].region) {
          tofix |= 16;
          iNESCart.region = moo[x].region;
        }
      }

      if (moo[x].prgram >= 0) {
        tofix |= 32;
        iNESCart.iNES2 = 1;
        iNESCart.PRGRamSize =
            (moo[x].prgram & 0x0F) ? (64 << ((moo[x].prgram >> 0) & 0xF)) : 0;
        iNESCart.PRGRamSaveSize =
            (moo[x].prgram & 0xF0) ? (64 << ((moo[x].prgram >> 4) & 0xF)) : 0;
      }

      if (moo[x].chrram >= 0) {
        tofix |= 32;
        iNESCart.iNES2 = 1;
        iNESCart.CHRRamSize =
            (moo[x].chrram & 0x0F) ? (64 << ((moo[x].chrram >> 0) & 0xF)) : 0;
        iNESCart.CHRRamSaveSize =
            (moo[x].chrram & 0xF0) ? (64 << ((moo[x].chrram >> 4) & 0xF)) : 0;
      }

      break;
    }
    x++;
  } while (moo[x].mirror >= 0 || moo[x].mapper >= 0);

  /* Games that use these iNES mappers tend to have the four-screen bit set
     when it should not be.
     */
  if ((iNESCart.mapper == 118 || iNESCart.mapper == 24 ||
       iNESCart.mapper == 26) &&
      (iNESCart.mirror == 2)) {
    iNESCart.mirror = 0;
    tofix |= 2;
  }

  /* Four-screen mirroring implicitly set. */
  if (iNESCart.mapper == 99)
    iNESCart.mirror = 2;

  if (tofix) {
    size_t gigastr_len;
    char gigastr[768];
    strcpy(gigastr, " The iNES header contains incorrect information.  For "
                    "now, the information will be corrected in RAM. ");
    gigastr_len = strlen(gigastr);
    if (tofix & 1)
      sprintf(
          gigastr + gigastr_len,
          "Current mapper # is %ld. The mapper number should be set to %d. ",
          current_mapper, iNESCart.mapper);
    if (tofix & 2) {
      uint8 *mstr[3] = {(uint8_t *)"Horizontal", (uint8_t *)"Vertical",
                        (uint8_t *)"Four-screen"};
      sprintf(gigastr + gigastr_len,
              "Current mirroring is %s. Mirroring should be set to \"%s\". ",
              mstr[cur_mirr & 3], mstr[iNESCart.mirror & 3]);
    }
    if (tofix & 4)
      strcat(gigastr, "The battery-backed bit should be set.  ");
    if (tofix & 8)
      strcat(gigastr, "This game should not have any CHR ROM.  ");
    if (tofix & 16) {
      uint8 *rstr[4] = {(uint8 *)"NTSC", (uint8 *)"PAL", (uint8 *)"Multi",
                        (uint8 *)"Dendy"};
      sprintf(gigastr + gigastr_len,
              "This game should run with \"%s\" timings.",
              rstr[iNESCart.region]);
    }
    if (tofix & 32) {
      unsigned PRGRAM = iNESCart.PRGRamSize + iNESCart.PRGRamSaveSize;
      unsigned CHRRAM = iNESCart.CHRRamSize + iNESCart.CHRRamSaveSize;
      if (PRGRAM || CHRRAM) {
        if (iNESCart.PRGRamSaveSize == 0)
          sprintf(gigastr + gigastr_len, "workram: %d KB, ", PRGRAM / 1024);
        else if (iNESCart.PRGRamSize == 0)
          sprintf(gigastr + gigastr_len, "saveram: %d KB, ", PRGRAM / 1024);
        else
          sprintf(gigastr + gigastr_len,
                  "workram: %d KB (%dKB battery-backed), ", PRGRAM / 1024,
                  iNESCart.PRGRamSaveSize / 1024);
        sprintf(gigastr + gigastr_len, "chrram: %d KB.",
                (CHRRAM + iNESCart.CHRRamSaveSize) / 1024);
      }
    }
    strcat(gigastr, "\n");
    FCEU_printf("%s\n", gigastr);
  }

#undef DEFAULT
#undef NOEXTRA
#undef DFAULT8
#undef MI_4
#undef PAL
#undef DENDY
}

typedef struct {
  int32 mapper;
  void (*init)(CartInfo *);
} NewMI;

typedef struct {
  uint8 *name;
  int32 number;
  void (*init)(CartInfo *);
} BMAPPINGLocal;

#define INES_BOARD_BEGIN() static BMAPPINGLocal bmap[] = {
#define INES_BOARD_END()                                                       \
  {                                                                            \
    (uint8_t *)"", 0, NULL                                                     \
  }                                                                            \
  }                                                                            \
  ;
#define INES_BOARD(a, b, c) {(uint8_t *)a, b, c},

INES_BOARD_BEGIN()
#if defined(LINUX_EMU) || defined(NES_MAPPER_000)
INES_BOARD("NROM", 0, NROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_001)
INES_BOARD("MMC1", 1, Mapper1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_002)
INES_BOARD("UNROM", 2, UNROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_003)
INES_BOARD("CNROM", 3, CNROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_004)
INES_BOARD("MMC3", 4, Mapper4_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_005)
INES_BOARD("MMC5", 5, Mapper5_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_006)
INES_BOARD("FFE Rev. A", 6, Mapper6_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_007)
INES_BOARD("ANROM", 7, ANROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_008)
INES_BOARD("", 8, Mapper8_Init) /* no games, it's worthless */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_009)
INES_BOARD("MMC2", 9, Mapper9_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_010)
INES_BOARD("MMC4", 10, Mapper10_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_011)
INES_BOARD("Color Dreams", 11, Mapper11_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_012)
INES_BOARD("REX DBZ 5", 12, Mapper12_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_013)
INES_BOARD("CPROM", 13, CPROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_014)
INES_BOARD("REX SL-1632", 14, UNLSL1632_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_015)
INES_BOARD("100-in-1", 15, Mapper15_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_016)
INES_BOARD("BANDAI 24C02", 16, Mapper16_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_017)
INES_BOARD("FFE Rev. B", 17, Mapper17_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_018)
INES_BOARD("JALECO SS880006", 18,
           Mapper18_Init) /* JF-NNX (EB89018-30007) boards */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_019)
INES_BOARD("Namcot 106", 19, Mapper19_Init)
#endif
/*    INES_BOARD( "",                         20, Mapper20_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_021)
INES_BOARD("Konami VRC2/VRC4 A", 21, Mapper21_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_022)
INES_BOARD("Konami VRC2/VRC4 B", 22, Mapper22_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_023)
INES_BOARD("Konami VRC2/VRC4 C", 23, Mapper23_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_024)
INES_BOARD("Konami VRC6 Rev. A", 24, Mapper24_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_025)
INES_BOARD("Konami VRC2/VRC4 D", 25, Mapper25_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_026)
INES_BOARD("Konami VRC6 Rev. B", 26, Mapper26_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_027)
INES_BOARD("CC-21 MI HUN CHE", 27,
           UNLCC21_Init) /* Former dupe for VRC2/VRC4 mapper, redefined with crc
                            to mihunche boards */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_028)
INES_BOARD("Action 53", 28, Mapper28_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_029)
INES_BOARD("", 29, Mapper29_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_030)
INES_BOARD("UNROM 512", 30, UNROM512_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_031)
INES_BOARD("infineteNesLives-NSF", 31, Mapper31_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_032)
INES_BOARD("IREM G-101", 32, Mapper32_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_033)
INES_BOARD("TC0190FMC/TC0350FMR", 33, Mapper33_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_034)
INES_BOARD("IREM I-IM/BNROM", 34, Mapper34_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_035)
INES_BOARD("EL870914C", 35,
           Mapper35_Init) // Modified to fit in the G&W, can cause bugs.
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_036)
INES_BOARD("TXC Policeman", 36, Mapper36_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_037)
INES_BOARD("PAL-ZZ SMB/TETRIS/NWC", 37, Mapper37_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_038)
INES_BOARD("Bit Corp.", 38, Mapper38_Init) /* Crime Busters */
#endif
/*    INES_BOARD( "",                         39, Mapper39_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_040)
INES_BOARD("SMB2j FDS", 40, Mapper40_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_041)
INES_BOARD("CALTRON 6-in-1", 41, Mapper41_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_042)
INES_BOARD("BIO MIRACLE FDS", 42, Mapper42_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_043)
INES_BOARD("FDS SMB2j LF36", 43, Mapper43_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_044)
INES_BOARD("MMC3 BMC PIRATE A", 44, Mapper44_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_045)
INES_BOARD("MMC3 BMC PIRATE B", 45, Mapper45_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_046)
INES_BOARD("RUMBLESTATION 15-in-1", 46, Mapper46_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_047)
INES_BOARD("NES-QJ SSVB/NWC", 47, Mapper47_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_048)
INES_BOARD("TAITO TCxxx", 48, Mapper48_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_049)
INES_BOARD("MMC3 BMC PIRATE C", 49, Mapper49_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_050)
INES_BOARD("SMB2j FDS Rev. A", 50, Mapper50_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_051)
INES_BOARD("11-in-1 BALL SERIES", 51, Mapper51_Init) /* 1993 year version */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_052)
INES_BOARD("MMC3 BMC PIRATE D", 52, Mapper52_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_053)
INES_BOARD("SUPERVISION 16-in-1", 53, Supervision16_Init)
#endif
/*    INES_BOARD( "",                         54, Mapper54_Init ) */
/*    INES_BOARD( "",                         55, Mapper55_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_056)
INES_BOARD("UNLKS202", 56, UNLKS202_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_057)
INES_BOARD("SIMBPLE BMC PIRATE A", 57, Mapper57_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_058)
INES_BOARD("SIMBPLE BMC PIRATE B", 58, Mapper58_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_059)
INES_BOARD("BMC T3H53/D1038", 59, BMCD1038_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_060)
INES_BOARD("Reset-based NROM-128 ", 60, Mapper60_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_061)
INES_BOARD("20-in-1 KAISER Rev. A", 61, Mapper61_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_062)
INES_BOARD("700-in-1", 62, Mapper62_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_063)
INES_BOARD("", 63, Mapper63_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_064)
INES_BOARD("TENGEN RAMBO1", 64, Mapper64_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_065)
INES_BOARD("IREM-H3001", 65, Mapper65_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_066)
INES_BOARD("MHROM", 66, MHROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_067)
INES_BOARD("SUNSOFT-FZII", 67, Mapper67_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_068)
INES_BOARD("Sunsoft Mapper #4", 68, Mapper68_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_069)
INES_BOARD("SUNSOFT-5/FME-7", 69, Mapper69_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_070)
INES_BOARD("BA KAMEN DISCRETE", 70, Mapper70_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_071)
INES_BOARD("CAMERICA BF9093", 71, Mapper71_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_072)
INES_BOARD("JALECO JF-17", 72, Mapper72_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_073)
INES_BOARD("KONAMI VRC3", 73, Mapper73_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_074)
INES_BOARD("TW MMC3+VRAM Rev. A", 74, Mapper74_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_075)
INES_BOARD("KONAMI VRC1", 75, Mapper75_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_076)
INES_BOARD("NAMCOT 108 Rev. A", 76, Mapper76_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_077)
INES_BOARD("IREM LROG017", 77, Mapper77_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_078)
INES_BOARD("Irem 74HC161/32", 78, Mapper78_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_079)
INES_BOARD("AVE/C&E/TXC BOARD", 79, Mapper79_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_080)
INES_BOARD("TAITO X1-005 Rev. A", 80, Mapper80_Init)
#endif
/*    INES_BOARD( "",                             81, Mapper81_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_082)
INES_BOARD("TAITO X1-017", 82, Mapper82_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_083)
INES_BOARD("YOKO VRC Rev. B", 83, Mapper83_Init)
#endif
/*    INES_BOARD( "",                            84, Mapper84_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_085)
INES_BOARD("KONAMI VRC7", 85, Mapper85_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_086)
INES_BOARD("JALECO JF-13", 86, Mapper86_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_087)
INES_BOARD("74*139/74 DISCRETE", 87, Mapper87_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_088)
INES_BOARD("NAMCO 3433", 88, Mapper88_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_089)
INES_BOARD("SUNSOFT-3", 89, Mapper89_Init) /* SUNSOFT-2 mapper */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_090)
INES_BOARD("HUMMER/JY BOARD", 90, Mapper90_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_091)
INES_BOARD("EARLY HUMMER/JY BOARD", 91, Mapper91_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_092)
INES_BOARD("JALECO JF-19", 92, Mapper92_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_093)
INES_BOARD(
    "SUNSOFT-3R", 93,
    SUNSOFT_UNROM_Init) /* SUNSOFT-2 mapper with VRAM, different wiring */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_094)
INES_BOARD("HVC-UN1ROM", 94, Mapper94_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_095)
INES_BOARD("NAMCOT 108 Rev. B", 95, Mapper95_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_096)
INES_BOARD("BANDAI OEKAKIDS", 96, Mapper96_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_097)
INES_BOARD("IREM TAM-S1", 97, Mapper97_Init)
#endif
/*    INES_BOARD( "",                            98, Mapper98_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_099)
INES_BOARD("VS Uni/Dual- system", 99, Mapper99_Init)
#endif
/*    INES_BOARD( "",                            100, Mapper100_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_101)
INES_BOARD("", 101, Mapper101_Init)
#endif
/*    INES_BOARD( "",                            102, Mapper102_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_103)
INES_BOARD("FDS DOKIDOKI FULL", 103, Mapper103_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_104)
INES_BOARD("CAMERICA GOLDENFIVE", 104, Mapper104_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_105)
INES_BOARD("NES-EVENT NWC1990", 105, Mapper105_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_106)
INES_BOARD("SMB3 PIRATE A", 106, Mapper106_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_107)
INES_BOARD("MAGIC CORP A", 107, Mapper107_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_108)
INES_BOARD("FDS UNROM BOARD", 108, Mapper108_Init)
#endif
/*    INES_BOARD( "",                            109, Mapper109_Init ) */
/*    INES_BOARD( "",                            110, Mapper110_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_111)
// Can't fit in the G&W due to amount of ram requested
INES_BOARD("Cheapocabra", 111, Mapper111_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_112)
INES_BOARD("ASDER/NTDEC BOARD", 112, Mapper112_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_113)
INES_BOARD("HACKER/SACHEN BOARD", 113, Mapper113_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_114)
INES_BOARD("MMC3 SG PROT. A", 114, Mapper114_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_115)
INES_BOARD("MMC3 PIRATE A", 115, Mapper115_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_116)
INES_BOARD("MMC1/MMC3/VRC PIRATE", 116, UNLSL12_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_117)
INES_BOARD("FUTURE MEDIA BOARD", 117, Mapper117_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_118)
INES_BOARD("TSKROM", 118, TKSROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_119)
INES_BOARD("NES-TQROM", 119, Mapper119_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_120)
INES_BOARD("FDS TOBIDASE", 120, Mapper120_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_121)
INES_BOARD("MMC3 PIRATE PROT. A", 121, Mapper121_Init)
#endif
/*    INES_BOARD( "",                            122, Mapper122_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_123)
INES_BOARD("MMC3 PIRATE H2288", 123, UNLH2288_Init)
#endif
/*    INES_BOARD( "",                            124, Mapper124_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_125)
INES_BOARD("FDS LH32", 125, LH32_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_126)
INES_BOARD("PowerJoy 84-in-1 PJ-008", 126, Mapper126_Init)
#endif
/*    INES_BOARD( "",                            127, Mapper127_Init ) */
/*    INES_BOARD( "",                            128, Mapper128_Init ) */
/*    INES_BOARD( "",                            129, Mapper129_Init ) */
/*    INES_BOARD( "",                            130, Mapper130_Init ) */
/*    INES_BOARD( "",                            131, Mapper131_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_132)
INES_BOARD("TXC/UNL-22211", 132, Mapper132_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_133)
INES_BOARD("SA72008", 133, SA72008_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_134)
INES_BOARD("MMC3 BMC PIRATE", 134, Mapper134_Init)
#endif
/*    INES_BOARD( "",                            135, Mapper135_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_136)
INES_BOARD("Sachen 3011", 136, Mapper136_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_137)
INES_BOARD("S8259D", 137, S8259D_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_138)
INES_BOARD("S8259B", 138, S8259B_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_139)
INES_BOARD("S8259C", 139, S8259C_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_140)
INES_BOARD("JALECO JF-11/14", 140, Mapper140_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_141)
INES_BOARD("S8259A", 141, S8259A_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_142)
INES_BOARD("UNLKS7032", 142, UNLKS7032_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_143)
INES_BOARD("TCA01", 143, TCA01_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_144)
INES_BOARD("AGCI 50282", 144, Mapper144_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_145)
INES_BOARD("SA72007", 145, SA72007_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_146)
INES_BOARD("SA0161M", 146, SA0161M_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_147)
INES_BOARD("Sachen 3018 board", 147, Mapper147_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_148)
INES_BOARD("SA0037", 148, SA0037_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_149)
INES_BOARD("SA0036", 149, SA0036_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_150)
INES_BOARD("SA-015/SA-630", 150, S74LS374N_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_151)
INES_BOARD("", 151, Mapper151_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_152)
INES_BOARD("", 152, Mapper152_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_153)
INES_BOARD("BANDAI SRAM", 153,
           Mapper153_Init) /* Bandai board 16 with SRAM instead of EEPROM */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_154)
INES_BOARD("", 154, Mapper154_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_155)
INES_BOARD("", 155, Mapper155_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_156)
INES_BOARD("", 156, Mapper156_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_157)
INES_BOARD("BANDAI BARCODE", 157, Mapper157_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_158)
INES_BOARD("TENGEN 800037", 158, Mapper158_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_159)
INES_BOARD("BANDAI 24C01", 159,
           Mapper159_Init) /* Different type of EEPROM on the  bandai board */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_160)
INES_BOARD("SA009", 160, SA009_Init) // Sylver : not working
#endif
/*    INES_BOARD( "",                            161, Mapper161_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_162)
INES_BOARD("Waixing FS304", 162, Mapper162_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_163)
INES_BOARD("", 163, Mapper163_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_164)
INES_BOARD("", 164, Mapper164_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_165)
INES_BOARD("", 165, Mapper165_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_166)
INES_BOARD("SUBOR Rev. A", 166, Mapper166_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_167)
INES_BOARD("SUBOR Rev. B", 167, Mapper167_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_168)
INES_BOARD("", 168, Mapper168_Init)
#endif
/*    INES_BOARD( "",                            169, Mapper169_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_170)
INES_BOARD("", 170, Mapper170_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_171)
INES_BOARD("", 171, Mapper171_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_172)
INES_BOARD("Super Mega P-4070", 172, Mapper172_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_173)
INES_BOARD("Idea-Tek ET.xx", 173, Mapper173_Init)
#endif
/*    INES_BOARD( "",                            174, Mapper174_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_175)
INES_BOARD("", 175, Mapper175_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_176)
INES_BOARD("BMCFK23C", 176, Mapper176_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_177)
INES_BOARD("", 177, Mapper177_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_178)
INES_BOARD("Waixing FS305", 178, Mapper178_Init)
#endif
/*    INES_BOARD( "",                            179, Mapper179_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_180)
INES_BOARD("", 180, Mapper180_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_181)
INES_BOARD("", 181, Mapper181_Init)
#endif
/*    INES_BOARD( "",                            182, Mapper182_Init ) */ /* Deprecated,
                                                                             dupe
                                                                             of
                                                                             Mapper
                                                                             114
                                                                           */
#if defined(LINUX_EMU) || defined(NES_MAPPER_183)
INES_BOARD("", 183, Mapper183_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_184)
INES_BOARD("", 184, Mapper184_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_185)
INES_BOARD("", 185, Mapper185_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_186)
INES_BOARD("", 186, Mapper186_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_187)
INES_BOARD("", 187, Mapper187_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_188)
INES_BOARD("", 188, Mapper188_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_189)
INES_BOARD("", 189, Mapper189_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_190)
INES_BOARD("", 190, Mapper190_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_191)
INES_BOARD("", 191, Mapper191_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_192)
INES_BOARD("Waixing FS308", 192, Mapper192_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_193)
INES_BOARD("NTDEC TC-112", 193, Mapper193_Init) /* War in the Gulf */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_194)
INES_BOARD("TW MMC3+VRAM Rev. C", 194, Mapper194_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_195)
INES_BOARD("Waixing FS303", 195, Mapper195_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_196)
INES_BOARD("", 196, Mapper196_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_197)
INES_BOARD("", 197, Mapper197_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_198)
INES_BOARD("TW MMC3+VRAM Rev. E", 198, Mapper198_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_199)
INES_BOARD("Waixing FS309", 199, Mapper199_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_200)
INES_BOARD("", 200, Mapper200_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_201)
INES_BOARD("21-in-1", 201, Mapper201_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_202)
INES_BOARD("", 202, Mapper202_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_203)
INES_BOARD("", 203, Mapper203_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_204)
INES_BOARD("", 204, Mapper204_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_205)
INES_BOARD("BMC 15-in-1/3-in-1", 205, Mapper205_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_206)
INES_BOARD("Nintendo DE(1)ROM", 206, Mapper206_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_207)
INES_BOARD("TAITO X1-005 Rev. B", 207, Mapper207_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_208)
INES_BOARD("", 208, Mapper208_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_209)
INES_BOARD("HUMMER/JY BOARD", 209, Mapper209_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_210)
INES_BOARD("", 210, Mapper210_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_211)
INES_BOARD("HUMMER/JY BOARD", 211, Mapper211_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_212)
INES_BOARD("", 212, Mapper212_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_213)
INES_BOARD("", 213, Mapper58_Init) /* in mapper 58 */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_214)
INES_BOARD("", 214, Mapper214_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_215)
INES_BOARD("UNL-8237", 215, UNL8237_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_216)
INES_BOARD("", 216, Mapper216_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_217)
INES_BOARD("", 217, Mapper217_Init) /* Redefined to a new Discrete BMC mapper */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_218)
INES_BOARD("Magic Floor", 218, Mapper218_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_219)
INES_BOARD("UNLA9746", 219, UNLA9746_Init)
#endif
/*	INES_BOARD( "Debug Mapper",             220, Mapper220_Init         ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_221)
INES_BOARD("UNLN625092", 221, UNLN625092_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_222)
INES_BOARD("", 222, Mapper222_Init)
#endif
/*    INES_BOARD( "",                            223, Mapper223_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_224)
INES_BOARD("KT-008", 224,
           MINDKIDS_Init) /* The KT-008 board contains the MINDKIDS chipset */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_225)
INES_BOARD("", 225, Mapper225_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_226)
INES_BOARD("BMC 22+20-in-1", 226, Mapper226_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_227)
INES_BOARD("", 227, Mapper227_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_228)
INES_BOARD("", 228, Mapper228_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_229)
INES_BOARD("", 229, Mapper229_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_230)
INES_BOARD("BMC Contra+22-in-1", 230, Mapper230_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_231)
INES_BOARD("", 231, Mapper231_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_232)
INES_BOARD("BMC QUATTRO", 232, Mapper232_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_233)
INES_BOARD("BMC 22+20-in-1 RST", 233, Mapper233_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_234)
INES_BOARD("BMC MAXI", 234, Mapper234_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_235)
INES_BOARD("Golden Game", 235, Mapper235_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_236)
INES_BOARD("Realtec 8031/8155/8099/8106", 236, Mapper236_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_237)
INES_BOARD("Teletubbies / Y2K", 237, Mapper237_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_238)
INES_BOARD("UNL6035052", 238, UNL6035052_Init)
#endif
/*    INES_BOARD( "",                            239, Mapper239_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_240)
INES_BOARD("", 240, Mapper240_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_241)
INES_BOARD("", 241, Mapper241_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_242)
INES_BOARD("43272", 242, Mapper242_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_243)
INES_BOARD("SA-020A", 243, S74LS374N_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_244)
INES_BOARD("DECATHLON", 244, Mapper244_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_245)
INES_BOARD("", 245, Mapper245_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_246)
INES_BOARD("FONG SHEN BANG", 246, Mapper246_Init)
#endif
/*    INES_BOARD( "",                            247, Mapper247_Init ) */
/*    INES_BOARD( "",                            248, Mapper248_Init ) */
#if defined(LINUX_EMU) || defined(NES_MAPPER_249)
INES_BOARD("", 249, Mapper249_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_250)
INES_BOARD("", 250, Mapper250_Init)
#endif
/*    INES_BOARD( "",                            251, Mapper251_Init ) */ /* No
                                                                             good
                                                                             dumps
                                                                             for
                                                                             this
                                                                             mapper,
                                                                             use
                                                                             UNIF
                                                                             version
                                                                           */
#if defined(LINUX_EMU) || defined(NES_MAPPER_252)
INES_BOARD("SAN GUO ZHI PIRATE", 252, Mapper252_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_253)
INES_BOARD("DRAGON BALL PIRATE", 253, Mapper253_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_254)
INES_BOARD("", 254, Mapper254_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_255)
INES_BOARD("", 255, Mapper255_Init) /* Duplicate of M225? */
#endif

/* NES 2.0 MAPPERS */

#if defined(LINUX_EMU) || defined(NES_MAPPER_256)
INES_BOARD("OneBus", 256, UNLOneBus_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_258)
INES_BOARD("158B", 258, UNL8237_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_259)
INES_BOARD("F-15", 259, BMCF15_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_260)
INES_BOARD("HPxx / HP2018-A", 260, BMCHPxx_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_261)
INES_BOARD("810544-C-A1", 261, BMC810544CA1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_262)
INES_BOARD("SHERO", 262, UNLSHeroes_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_263)
INES_BOARD("KOF97", 263, UNLKOF97_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_264)
INES_BOARD("YOKO", 264, UNLYOKO_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_265)
INES_BOARD("T-262", 265, BMCT262_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_266)
INES_BOARD("CITYFIGHT", 266, UNLCITYFIGHT_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_267)
INES_BOARD("8-in-1 JY-119", 267, Mapper267_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_268)
INES_BOARD(
    "COOLBOY/MINDKIDS", 268,
    Mapper268_Init) /* Submapper distinguishes between COOLBOY and MINDKIDS */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_269)
INES_BOARD("Games Xplosion 121-in-1", 269, Mapper269_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_271)
INES_BOARD("MGC-026", 271, Mapper271_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_272)
INES_BOARD("Akumajō Special: Boku Dracula-kun", 272, Mapper272_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_274)
INES_BOARD("80013-B", 274, BMC80013B_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_281)
INES_BOARD("YY860417C", 281, Mapper281_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_282)
INES_BOARD("860224C", 282, Mapper282_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_283)
INES_BOARD("GS-2004/GS-2013", 283, Mapper283_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_285)
INES_BOARD("A65AS", 285, BMCA65AS_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_286)
INES_BOARD("BS-5", 286, BMCBS5_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_287)
INES_BOARD("411120-C, 811120-C", 287, BMC411120C_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_288)
INES_BOARD("GKCX1", 288, Mapper288_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_289)
INES_BOARD("60311C", 289, BMC60311C_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_290)
INES_BOARD("NTD-03", 290, BMCNTD03_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_291)
INES_BOARD("Kasheng 2-in-1 ", 291, Mapper291_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_292)
INES_BOARD("DRAGONFIGHTER", 292, UNLBMW8544_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_293)
INES_BOARD("NewStar 12-in-1/7-in-1", 293, Mapper293_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_294)
INES_BOARD("63-1601 ", 294, Mapper294_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_295)
INES_BOARD("YY860216C", 295, Mapper295_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_297)
INES_BOARD("TXC 01-22110-000", 297, Mapper297_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_298)
INES_BOARD("TF1201", 298, UNLTF1201_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_299)
INES_BOARD("11160", 299, BMC11160_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_300)
INES_BOARD("190in1", 300, BMC190in1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_301)
INES_BOARD("8157", 301, UNL8157_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_302)
INES_BOARD("KS7057", 302, UNLKS7057_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_303)
INES_BOARD("KS7017", 303, UNLKS7017_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_304)
INES_BOARD("SMB2J", 304, UNLSMB2J_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_305)
INES_BOARD("KS7031", 305, UNLKS7031_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_306)
INES_BOARD("KS7016", 306, UNLKS7016_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_307)
INES_BOARD("KS7037", 307, UNLKS7037_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_308)
INES_BOARD("TH2131-1", 308, UNLTH21311_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_309)
INES_BOARD("LH51", 309, LH51_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_310)
INES_BOARD("K-1053", 310, Mapper310_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_312)
INES_BOARD("KS7013B", 312, UNLKS7013B_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_313)
INES_BOARD("RESET-TXROM", 313, BMCRESETTXROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_314)
INES_BOARD("64in1NoRepeat", 314, BMC64in1nr_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_315)
INES_BOARD("830134C", 315, BMC830134C_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_319)
INES_BOARD("HP898F", 319, Mapper319_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_320)
INES_BOARD("830425C-4391T", 320, BMC830425C4391T_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_322)
INES_BOARD("K-3033", 322, BMCK3033_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_323)
INES_BOARD("FARID_SLROM_8-IN-1", 323, FARIDSLROM8IN1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_324)
INES_BOARD("FARID_UNROM_8-IN-1", 324, FARIDUNROM_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_325)
INES_BOARD("MALISB", 325, UNLMaliSB_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_326)
INES_BOARD("Contra/Gryzor", 326, Mapper326_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_327)
INES_BOARD("10-24-C-A1", 327, BMC1024CA1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_328)
INES_BOARD("RT-01", 328, UNLRT01_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_329)
INES_BOARD("EDU2000", 329, UNLEDU2000_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_330)
INES_BOARD("Sangokushi II: Haō no Tairiku", 330, Mapper330_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_331)
INES_BOARD("12-IN-1", 331, BMC12IN1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_332)
INES_BOARD("WS", 332, BMCWS_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_333)
INES_BOARD("NEWSTAR-GRM070-8IN1", 333, BMC8IN1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_334)
INES_BOARD("5/20-in-1 1993 Copyright", 334, Mapper334_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_335)
INES_BOARD("CTC-09", 335, BMCCTC09_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_336)
INES_BOARD("K-3046", 336, BMCK3046_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_337)
INES_BOARD("CTC-12IN1", 337, BMCCTC12IN1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_338)
INES_BOARD("SA005-A", 338, BMCSA005A_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_339)
INES_BOARD("K-3006", 339, BMCK3006_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_340)
INES_BOARD("K-3036", 340, BMCK3036_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_341)
INES_BOARD("TJ-03", 341, BMCTJ03_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_342)
INES_BOARD("COOLGIRL", 342, COOLGIRL_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_344)
INES_BOARD("GN-26", 344, BMCGN26_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_345)
INES_BOARD("L6IN1", 345, BMCL6IN1_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_346)
INES_BOARD("KS7012", 346, UNLKS7012_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_347)
INES_BOARD("KS7030", 347, UNLKS7030_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_348)
INES_BOARD("830118C", 348, BMC830118C_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_349)
INES_BOARD("G-146", 349, BMCG146_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_350)
INES_BOARD("891227", 350, BMC891227_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_351)
INES_BOARD("Techline XB", 351, Mapper351_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_353)
INES_BOARD("Super Mario Family", 353, Mapper353_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_354)
INES_BOARD("FAM250/81-01-39-C/SCHI-24", 354, Mapper354_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_355)
INES_BOARD("3D-BLOCK", 355, UNL3DBlock_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_356)
INES_BOARD("7-in-1 Rockman (JY-208)", 356, Mapper356_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_357)
INES_BOARD("Bit Corp 4-in-1", 357, Mapper357_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_358)
INES_BOARD("YY860606C", 358, Mapper358_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_359)
INES_BOARD("SB-5013/GCL8050/841242C", 359, Mapper359_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_360)
INES_BOARD("Bitcorp 31-in-1", 360, Mapper360_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_361)
INES_BOARD("OK-411", 361,
           GN45_Init) /* OK-411 is emulated together with GN-45 */
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_364)
INES_BOARD("JY830832C", 364, Mapper364_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_366)
INES_BOARD("GN-45", 366, GN45_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_368)
INES_BOARD("Yung-08", 368, Mapper368_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_369)
INES_BOARD("N49C-300", 369, Mapper369_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_370)
INES_BOARD("Golden Mario Party II - Around the World 6-in-1", 370,
           Mapper370_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_372)
INES_BOARD("MMC3 PIRATE SFC-12", 372, Mapper372_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_374)
INES_BOARD("95/96 Super HiK 4-in-1", 374, Mapper374_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_375)
INES_BOARD("135-in-1", 375, Mapper375_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_376)
INES_BOARD("YY841155C", 376, Mapper376_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_377)
INES_BOARD("JY-111/JY-112", 377, Mapper377_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_380)
INES_BOARD("42 to 80,000 (970630C)", 380, Mapper380_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_381)
INES_BOARD("KN-42", 381, Mapper381_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_382)
INES_BOARD("830928C", 382, Mapper382_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_383)
INES_BOARD("YY840708C", 383, Mapper383_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_385)
INES_BOARD("NTDEC 2779", 385, Mapper385_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_386)
INES_BOARD("YY860729C", 386, Mapper386_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_387)
INES_BOARD("YY850735C", 387, Mapper387_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_388)
INES_BOARD("YY850835C", 388, Mapper388_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_389)
INES_BOARD("Caltron 9-in-1", 389, Mapper389_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_390)
INES_BOARD("Realtec 8031", 390, Mapper390_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_391)
INES_BOARD("BS-110", 391, Mapper391_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_393)
INES_BOARD("820720C", 393, Mapper393_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_394)
INES_BOARD("HSK007", 394, Mapper394_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_395)
INES_BOARD("Realtec 8210", 395, Mapper395_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_396)
INES_BOARD("YY850437C", 396, Mapper396_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_397)
INES_BOARD("YY850439C", 397, Mapper397_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_401)
INES_BOARD("BMC Super 19-in-1 (VIP19)", 401, Mapper401_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_402)
INES_BOARD("831019C J-2282", 402, J2282_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_403)
INES_BOARD("89433", 403, Mapper403_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_404)
INES_BOARD("JY012005", 404, Mapper404_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_409)
INES_BOARD("retroUSB DPCMcart", 409, Mapper409_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_410)
INES_BOARD("JY-302", 410, Mapper410_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_411)
INES_BOARD("A88S-1", 411, Mapper411_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_414)
INES_BOARD("9999999-in-1", 414, Mapper414_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_415)
INES_BOARD("0353", 415, Mapper415_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_416)
INES_BOARD("4-in-1/N-32", 416, Mapper416_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_417)
INES_BOARD("", 417, Mapper417_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_421)
INES_BOARD("SC871115C", 421, Mapper421_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_422)
INES_BOARD("BS-400R/BS-4040", 422, Mapper422_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_428)
INES_BOARD("AB-G1L/WELL-NO-DG450", 428, Mapper428_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_429)
INES_BOARD("LIKO BBG-235-8-1B", 429, Mapper429_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_431)
INES_BOARD("Realtek GN-91B", 431, Mapper431_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_432)
INES_BOARD("Realtec 8090", 432, Mapper432_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_433)
INES_BOARD("NC-20MB", 433, Mapper433_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_434)
INES_BOARD("S-009", 434, Mapper434_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_435)
INES_BOARD("F-1002", 435, Mapper435_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_436)
INES_BOARD("820401/T-217", 436, Mapper436_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_437)
INES_BOARD("NTDEC TH2348", 437, Mapper437_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_438)
INES_BOARD("K-3071", 438, Mapper438_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_441)
INES_BOARD("850335C", 441, Mapper441_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_443)
INES_BOARD("NC-3000M", 443, Mapper443_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_444)
INES_BOARD("NC-7000M/NC-8000M", 444, Mapper444_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_449)
INES_BOARD("22-in-1 King Series", 449, Mapper449_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_452)
INES_BOARD("DS-9-27", 452, Mapper452_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_455)
INES_BOARD("N625836", 455, Mapper455_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_456)
INES_BOARD("K6C3001A", 456, Mapper456_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_459)
INES_BOARD("8-in-1", 459, Mapper459_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_460)
INES_BOARD("FC-29-40/K-3101", 460, Mapper460_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_461)
INES_BOARD("0324", 461, Mapper461_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_463)
INES_BOARD("YH810X1", 463, Mapper463_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_464)
INES_BOARD("NTDEC 9012", 464, Mapper464_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_465)
INES_BOARD("ET-120", 465, Mapper465_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_466)
INES_BOARD("Keybyte Computer", 466, Mapper466_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_467)
INES_BOARD("47-2", 467, Mapper467_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_468)
INES_BOARD("BlazePro CPLD", 468, Mapper468_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_513)
INES_BOARD("SA-9602B", 513, SA9602B_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_516)
INES_BOARD("Brilliant Com Cocoma Pack", 516, Mapper516_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_518)
INES_BOARD("DANCE2000", 518, UNLD2000_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_519)
INES_BOARD("EH8813A", 519, UNLEH8813A_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_521)
INES_BOARD("DREAMTECH01", 521, DreamTech01_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_522)
INES_BOARD("LH10", 522, LH10_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_523)
INES_BOARD("Jncota KT-???", 523, Mapper523_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_524)
INES_BOARD("900218", 524, BTL900218_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_525)
INES_BOARD("KS7021A", 525, UNLKS7021A_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_526)
INES_BOARD("BJ-56", 526, UNLBJ56_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_527)
INES_BOARD("AX-40G", 527, UNLAX40G_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_529)
INES_BOARD("T-230", 529, UNLT230_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_530)
INES_BOARD("AX5705", 530, UNLAX5705_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_533)
INES_BOARD("Sachen 3014", 533, Mapper533_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_534)
INES_BOARD("NJ064", 534, Mapper534_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_535)
INES_BOARD("LH53", 535, LH53_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_538)
INES_BOARD("60-1064-16L (FDS)", 538, Mapper538_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_539)
INES_BOARD("Kid Ikarus (FDS)", 539, Mapper539_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_540)
INES_BOARD("82112C", 540, Mapper540_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_541)
INES_BOARD("LittleCom 160-in-1", 541, Mapper541_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_543)
INES_BOARD("5-in-1 (CH-501)", 543, Mapper543_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_553)
INES_BOARD("SACHEN 3013", 553, Mapper553_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_554)
INES_BOARD("KS-7010", 554, Mapper554_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_550)
INES_BOARD("", 550, Mapper550_Init)
#endif
#if defined(LINUX_EMU) || defined(NES_MAPPER_558)
INES_BOARD("YC-03-09", 558, Mapper558_Init)
#endif
INES_BOARD_END()

static uint32 iNES_get_mapper_id(void) {
  /* If byte 7 AND $0C = $08, and the size taking into account byte 9 does not
   * exceed the actual size of the ROM image, then NES 2.0. If byte 7 AND $0C =
   * $00, and bytes 12-15 are all 0, then iNES. Otherwise, archaic iNES. -
   * nesdev*/
  uint32 ret;
  switch (head.ROM_type2 & 0x0C) {
  case 0x08: /* header version is NES 2.0 */
    ret = (((uint32)head.ROM_type3 << 8) & 0xF00) | (head.ROM_type2 & 0xF0) |
          (head.ROM_type >> 4);
    break;
  case 0x00: /* header version is iNES */
    ret = (head.ROM_type2 & 0xF0) | (head.ROM_type >> 4);
    break;
  default: /* any other value is Archaic iNes, byte 7-15 not used */
    ret = (head.ROM_type >> 4);
    break;
  }
  return ret;
}

static void iNES_read_header_info(void) {
  ROM_size = head.ROM_size;
  VROM_size = head.VROM_size;
  iNESCart.mirror = (head.ROM_type & 8) ? 2 : (head.ROM_type & 1);
  iNESCart.battery = (head.ROM_type & 2) ? 1 : 0;
  iNESCart.mapper = iNES_get_mapper_id();
  iNESCart.iNES2 = (head.ROM_type2 & 0x0C) == 0x08;

  if (iNESCart.iNES2) {
    ROM_size |= ((head.upper_PRG_CHR_size >> 0) & 0xF) << 8;
    VROM_size |= ((head.upper_PRG_CHR_size >> 4) & 0xF) << 8;
    iNESCart.submapper = (head.ROM_type3 >> 4) & 0x0F;
    iNESCart.region = head.Region & 3;
    if (head.PRGRAM_size & 0x0F)
      iNESCart.PRGRamSize = 64 << ((head.PRGRAM_size >> 0) & 0x0F);
    if (head.PRGRAM_size & 0xF0)
      iNESCart.PRGRamSaveSize = 64 << ((head.PRGRAM_size >> 4) & 0x0F);
    if (head.CHRRAM_size & 0x0F)
      iNESCart.CHRRamSize = 64 << ((head.CHRRAM_size >> 0) & 0x0F);
    if (head.CHRRAM_size & 0xF0)
      iNESCart.CHRRamSaveSize = 64 << ((head.CHRRAM_size >> 4) & 0x0F);
  }
}

#ifdef TARGET_GNW
int iNESLoad(const char *name, const uint8_t *rom, uint32_t rom_size) {
  struct md5_context md5;
  uint64 filesize = rom_size; /* size of file including header */
  uint64 romSize = 0;         /* size of PRG + CHR rom */
  /* used for malloc and cart mapping */
  uint32 offset = 0;
  uint32 rom_size_pow2 = 0;
  uint32 vrom_size_pow2 = 0;

  memcpy(&head, rom, 16);
  offset += 16;

  if (memcmp(&head, "NES\x1a", 4)) {
    FCEU_PrintError("Not an iNES file!\n");
    return 0;
  }

  memset(&iNESCart, 0, sizeof(iNESCart));

  if (!memcmp((char *)(&head) + 0x7, "DiskDude", 8))
    memset((char *)(&head) + 0x7, 0, 0x9);

  if (!memcmp((char *)(&head) + 0x7, "demiforce", 9))
    memset((char *)(&head) + 0x7, 0, 0x9);

  if (!memcmp((char *)(&head) + 0xA, "Ni03", 4)) {
    if (!memcmp((char *)(&head) + 0x7, "Dis", 3))
      memset((char *)(&head) + 0x7, 0, 0x9);
    else
      memset((char *)(&head) + 0xA, 0, 0x6);
  }

  iNES_read_header_info();

  if (!ROM_size)
    ROM_size = 256;

  filesize -= 16; /* remove header size from total size */

  /* Trainer */
  if (head.ROM_type & 4) {
    trainerpoo = (uint8 *)FCEU_gmalloc(512);
    memcpy(trainerpoo, rom + 16, 512);
    offset += 512;

    filesize -= 512;
  }

  iNESCart.PRGRomSize =
      ROM_size >= 0xF00
          ? (pow(2, head.ROM_size >> 2) * ((head.ROM_size & 3) * 2 + 1))
          : (ROM_size * 0x4000);
  iNESCart.CHRRomSize =
      VROM_size >= 0xF00
          ? (pow(2, head.VROM_size >> 2) * ((head.VROM_size & 3) * 2 + 1))
          : (VROM_size * 0x2000);
  ;

  romSize = iNESCart.PRGRomSize + iNESCart.CHRRomSize;

  if (romSize > filesize) {
    FCEU_PrintError(" File length is too short to contain all data reported "
                    "from header by %llu\n",
                    romSize - filesize);
  } else if (romSize < filesize)
    FCEU_PrintError(" File contains %llu bytes of unused data\n",
                    filesize - romSize);

  rom_size_pow2 = uppow2(iNESCart.PRGRomSize);

  ROM = (uint8 *)(rom + offset);
  offset += iNESCart.PRGRomSize;

  if (iNESCart.CHRRomSize) {
    vrom_size_pow2 = uppow2(iNESCart.CHRRomSize);

    VROM = (uint8 *)(rom + offset);
    offset += iNESCart.CHRRomSize;
  }

  iNESCart.PRGCRC32 = esp_rom_crc32_le(0, ROM, iNESCart.PRGRomSize);
  iNESCart.CHRCRC32 = esp_rom_crc32_le(0, VROM, iNESCart.CHRRomSize);
  iNESCart.CRC32 =
      esp_rom_crc32_le(iNESCart.PRGCRC32, VROM, iNESCart.CHRRomSize);

  md5_starts(&md5);
  md5_update(&md5, ROM, iNESCart.PRGRomSize);
  if (iNESCart.CHRRomSize)
    md5_update(&md5, VROM, iNESCart.CHRRomSize);
  md5_finish(&md5, iNESCart.MD5);

  memcpy(&GameInfo->MD5, &iNESCart.MD5, sizeof(iNESCart.MD5));

  if (iNESCart.iNES2 == 0) {
    if (strstr(name, "(E)") || strstr(name, "(e)") ||
        strstr(name, "(Europe)") || strstr(name, "(PAL)") ||
        strstr(name, "(F)") || strstr(name, "(f)") || strstr(name, "(G)") ||
        strstr(name, "(g)") || strstr(name, "(I)") || strstr(name, "(i)") ||
        strstr(name, "(S)") || strstr(name, "(s)") ||
        strstr(name, "(France)") || strstr(name, "(Germany)") ||
        strstr(name, "(Italy)") || strstr(name, "(Spain)") ||
        strstr(name, "(Sweden)") || strstr(name, "(Sw)") ||
        strstr(name, "(Australia)") || strstr(name, "(A)") ||
        strstr(name, "(a)")) {
      iNESCart.region = 1;
    }
  }

  ResetCartMapping();
  ResetExState(0, 0);

  SetupCartPRGMapping(0, ROM, rom_size_pow2, 0);

  SetInput();

  if (iNESCart.iNES2 < 1)
    CheckHInfo();

  /* Must remain here because above functions might change value of
   * VROM_size and free(VROM).
   */
  if (VROM_size)
    SetupCartCHRMapping(0, VROM, vrom_size_pow2, 0);

  if (iNESCart.mirror == 2) {
#ifndef FCEU_NO_MALLOC
    ExtraNTARAM = (uint8 *)FCEU_gmalloc(2048);
#else
    ExtraNTARAM = (uint8 *)itc_calloc(1, 2048);
#endif
    SetupCartMirroring(4, 1, ExtraNTARAM);
  } else if (iNESCart.mirror >= 0x10)
    SetupCartMirroring(2 + (iNESCart.mirror & 1), 1, 0);
  else
    SetupCartMirroring(iNESCart.mirror & 1, (iNESCart.mirror & 4) >> 2, 0);

  iNESCart.battery = (head.ROM_type & 2) ? 1 : 0;

  if (!iNES_Init(iNESCart.mapper)) {
    FCEU_printf("\n");
    FCEU_PrintError(" iNES mapper #%d is not supported at all.\n",
                    iNESCart.mapper);
    return 0;
  }

  GameInterface = iNESGI;

  /* 0: RP2C02 ("NTSC NES")
   * 1: RP2C07 ("Licensed PAL NES")
   * 2: Multiple-region
   * 3: UMC 6527P ("Dendy") */
  if (iNESCart.region == 3)
    dendy = 1;
  FCEUI_SetVidSystem((iNESCart.region == 1) ? 1 : 0);

  return 1;
}
#else
int iNESLoad(const char *name, FCEUFILE *fp) {
  const char *tv_region[] = {"NTSC", "PAL", "Multi-region", "Dendy"};
  struct md5_context md5;
#ifdef DEBUG
  char *mappername = NULL;
  uint32 mappertest = 0;
#endif
  uint64 filesize = FCEU_fgetsize(fp); /* size of file including header */
  uint64 romSize = 0;                  /* size of PRG + CHR rom */
  /* used for malloc and cart mapping */
  uint32 rom_size_pow2 = 0;
  uint32 vrom_size_pow2 = 0;

  if (FCEU_fread(&head, 1, 16, fp) != 16)
    return 0;

  if (memcmp(&head, "NES\x1a", 4)) {
    FCEU_PrintError("Not an iNES file!\n");
    return 0;
  }

  memset(&iNESCart, 0, sizeof(iNESCart));

  if (!memcmp((char *)(&head) + 0x7, "DiskDude", 8))
    memset((char *)(&head) + 0x7, 0, 0x9);

  if (!memcmp((char *)(&head) + 0x7, "demiforce", 9))
    memset((char *)(&head) + 0x7, 0, 0x9);

  if (!memcmp((char *)(&head) + 0xA, "Ni03", 4)) {
    if (!memcmp((char *)(&head) + 0x7, "Dis", 3))
      memset((char *)(&head) + 0x7, 0, 0x9);
    else
      memset((char *)(&head) + 0xA, 0, 0x6);
  }

  iNES_read_header_info();

  if (!ROM_size)
    ROM_size = 256;

  filesize -= 16; /* remove header size from total size */

  /* Trainer */
  if (head.ROM_type & 4) {
    trainerpoo = (uint8 *)FCEU_gmalloc(512);
    FCEU_fread(trainerpoo, 512, 1, fp);
    filesize -= 512;
  }

  iNESCart.PRGRomSize =
      ROM_size >= 0xF00
          ? (pow(2, head.ROM_size >> 2) * ((head.ROM_size & 3) * 2 + 1))
          : (ROM_size * 0x4000);
  iNESCart.CHRRomSize =
      VROM_size >= 0xF00
          ? (pow(2, head.VROM_size >> 2) * ((head.VROM_size & 3) * 2 + 1))
          : (VROM_size * 0x2000);
  ;

  romSize = iNESCart.PRGRomSize + iNESCart.CHRRomSize;

  if (romSize > filesize) {
    FCEU_PrintError(" File length is too short to contain all data reported "
                    "from header by %llu\n",
                    romSize - filesize);
  } else if (romSize < filesize)
    FCEU_PrintError(" File contains %llu bytes of unused data\n",
                    filesize - romSize);

  rom_size_pow2 = uppow2(iNESCart.PRGRomSize);

  if ((ROM = (uint8 *)FCEU_malloc(rom_size_pow2)) == NULL)
    return 0;

  memset(ROM, 0xFF, rom_size_pow2);
  FCEU_fread(ROM, 1, iNESCart.PRGRomSize, fp);

  if (iNESCart.CHRRomSize) {
    vrom_size_pow2 = uppow2(iNESCart.CHRRomSize);

    if ((VROM = (uint8 *)FCEU_malloc(vrom_size_pow2)) == NULL) {
      free(ROM);
      ROM = NULL;
      return 0;
    }

    memset(VROM, 0xFF, vrom_size_pow2);
    FCEU_fread(VROM, 1, iNESCart.CHRRomSize, fp);
  }

  iNESCart.PRGCRC32 = CalcCRC32(0, ROM, iNESCart.PRGRomSize);
  iNESCart.CHRCRC32 = CalcCRC32(0, VROM, iNESCart.CHRRomSize);
  iNESCart.CRC32 = CalcCRC32(iNESCart.PRGCRC32, VROM, iNESCart.CHRRomSize);

  md5_starts(&md5);
  md5_update(&md5, ROM, iNESCart.PRGRomSize);
  if (iNESCart.CHRRomSize)
    md5_update(&md5, VROM, iNESCart.CHRRomSize);
  md5_finish(&md5, iNESCart.MD5);

  memcpy(&GameInfo->MD5, &iNESCart.MD5, sizeof(iNESCart.MD5));

#ifdef DEBUG
  mappername = "Not Listed";

  for (mappertest = 0; mappertest < (sizeof bmap / sizeof bmap[0]) - 1;
       mappertest++) {
    if (bmap[mappertest].number == iNESCart.mapper) {
      mappername = (char *)bmap[mappertest].name;
      break;
    }
  }
#endif

  if (iNESCart.iNES2 == 0) {
    if (strstr(name, "(E)") || strstr(name, "(e)") ||
        strstr(name, "(Europe)") || strstr(name, "(PAL)") ||
        strstr(name, "(F)") || strstr(name, "(f)") || strstr(name, "(G)") ||
        strstr(name, "(g)") || strstr(name, "(I)") || strstr(name, "(i)") ||
        strstr(name, "(S)") || strstr(name, "(s)") ||
        strstr(name, "(France)") || strstr(name, "(Germany)") ||
        strstr(name, "(Italy)") || strstr(name, "(Spain)") ||
        strstr(name, "(Sweden)") || strstr(name, "(Sw)") ||
        strstr(name, "(Australia)") || strstr(name, "(A)") ||
        strstr(name, "(a)")) {
      iNESCart.region = 1;
    }
  }

#ifdef DEBUG
  FCEU_printf(" PRG-ROM CRC32:  0x%08X\n", iNESCart.PRGCRC32);
  FCEU_printf(" PRG+CHR CRC32:  0x%08X\n", iNESCart.CRC32);
  FCEU_printf(" PRG+CHR MD5:    0x%s\n", md5_asciistr(iNESCart.MD5));
  FCEU_printf(" PRG-ROM:  %6d KiB\n", iNESCart.PRGRomSize >> 10);
  FCEU_printf(" CHR-ROM:  %6d KiB\n", iNESCart.CHRRomSize >> 10);
  FCEU_printf(" Mapper #: %3d\n", iNESCart.mapper);
  FCEU_printf(" Mapper name: %s\n", mappername);
  FCEU_printf(" Mirroring: %s\n", iNESCart.mirror == 2 ? "None (Four-screen)"
                                  : iNESCart.mirror    ? "Vertical"
                                                       : "Horizontal");
  FCEU_printf(" Battery: %s\n", (head.ROM_type & 2) ? "Yes" : "No");
  FCEU_printf(" System: %s\n", tv_region[iNESCart.region]);
  FCEU_printf(" Trained: %s\n", (head.ROM_type & 4) ? "Yes" : "No");

  if (iNESCart.iNES2) {
    unsigned PRGRAM = iNESCart.PRGRamSize + iNESCart.PRGRamSaveSize;
    unsigned CHRRAM = iNESCart.CHRRamSize + iNESCart.CHRRamSaveSize;

    FCEU_printf(" NES 2.0 extended iNES.\n");
    FCEU_printf(" Sub Mapper #: %3d\n", iNESCart.submapper);
    if (PRGRAM || CHRRAM) {
      if (head.ROM_type & 0x02) {
        FCEU_printf(" PRG RAM: %d KB (%d KB battery-backed)\n", PRGRAM / 1024,
                    iNESCart.PRGRamSaveSize / 1024);
        FCEU_printf(" CHR RAM: %d KB (%d KB battery-backed)\n", CHRRAM / 1024,
                    iNESCart.CHRRamSaveSize / 1024);
      } else {
        FCEU_printf(" PRG RAM: %d KB\n", PRGRAM / 1024);
        FCEU_printf(" CHR RAM: %d KB\n", CHRRAM / 1024);
      }
    }
  }
#endif

  ResetCartMapping();
  ResetExState(0, 0);

  SetupCartPRGMapping(0, ROM, rom_size_pow2, 0);

  SetInput();

  if (iNESCart.iNES2 < 1)
    CheckHInfo();

  {
    int x;
    uint64 partialmd5 = 0;
    int mapper = iNESCart.mapper;
    int mirroring = iNESCart.mirror;

    for (x = 0; x < 8; x++)
      partialmd5 |= (uint64)iNESCart.MD5[7 - x] << (x * 8);

    FCEU_VSUniCheck(partialmd5, &mapper, &mirroring);

    if ((mapper != iNESCart.mapper) || (mirroring != iNESCart.mirror)) {
      FCEU_PrintError("\n");
      FCEU_PrintError(" Incorrect VS-Unisystem header information!\n");
      if (mapper != iNESCart.mapper)
        FCEU_PrintError(" Mapper:    %d\n", mapper);
      if (mirroring != iNESCart.mirror)
        FCEU_PrintError(" Mirroring: %s\n", (mirroring == 2)
                                                ? "None (Four-screen)"
                                            : mirroring ? "Vertical"
                                                        : "Horizontal");
      iNESCart.mapper = mapper;
      iNESCart.mirror = mirroring;
    }
  }

  /* Must remain here because above functions might change value of
   * VROM_size and free(VROM).
   */
  if (VROM_size)
    SetupCartCHRMapping(0, VROM, vrom_size_pow2, 0);

  if (iNESCart.mirror == 2) {
    ExtraNTARAM = (uint8 *)FCEU_gmalloc(2048);
    SetupCartMirroring(4, 1, ExtraNTARAM);
  } else if (iNESCart.mirror >= 0x10)
    SetupCartMirroring(2 + (iNESCart.mirror & 1), 1, 0);
  else
    SetupCartMirroring(iNESCart.mirror & 1, (iNESCart.mirror & 4) >> 2, 0);

  iNESCart.battery = (head.ROM_type & 2) ? 1 : 0;

  if (!iNES_Init(iNESCart.mapper)) {
    FCEU_printf("\n");
    FCEU_PrintError(" iNES mapper #%d is not supported at all.\n",
                    iNESCart.mapper);
    return 0;
  }

  GameInterface = iNESGI;

  /* 0: RP2C02 ("NTSC NES")
   * 1: RP2C07 ("Licensed PAL NES")
   * 2: Multiple-region
   * 3: UMC 6527P ("Dendy") */
  if (iNESCart.region == 3)
    dendy = 1;
  FCEUI_SetVidSystem((iNESCart.region == 1) ? 1 : 0);

  return 1;
}
#endif

static int iNES_Init(int num) {
  BMAPPINGLocal *tmp = bmap;

  CHRRAMSize = -1;

#ifndef TARGET_GNW
  if (GameInfo->type == GIT_VSUNI)
    AddExState(FCEUVSUNI_STATEINFO, ~0, 0, 0);
#endif

  while (tmp->init) {
    if (num == tmp->number) {
#ifndef TARGET_GNW
      UNIFchrrama = 0; /* need here for compatibility with UNIF mapper code */
#endif
      if (!VROM_size) {
        if (iNESCart.iNES2) {
          CHRRAMSize = iNESCart.CHRRamSize + iNESCart.CHRRamSaveSize;
          if (CHRRAMSize == 0)
            CHRRAMSize = iNESCart.CHRRamSize = 8 * 8192;
        } else {
          switch (num) { /* FIXME, mapper or game data base with the board
                            parameters and ROM/RAM sizes */
          case 13:
            CHRRAMSize = 16 * 1024;
            break;
          case 6:
          case 28:
          case 29:
          case 30:
          case 45:
          case 96:
          case 513:
            CHRRAMSize = 32 * 1024;
            break;
          case 176:
            CHRRAMSize = 128 * 1024;
            break;
          case 268:
            CHRRAMSize = 256 * 1024;
            break;
          default:
            CHRRAMSize = 8 * 1024;
            break;
          }
          iNESCart.CHRRamSize = CHRRAMSize;
        }
        if (CHRRAMSize >
            0) { /* TODO: CHR-RAM are sometimes handled in mappers e.g. MMC1
                    using submapper 1/2/4 and CHR-RAM can be zero here */
#ifndef FCEU_NO_MALLOC
          if ((VROM = (uint8 *)malloc(CHRRAMSize)) == NULL)
            return 0;
#else
          if ((VROM = (uint8 *)ahb_calloc(1, CHRRAMSize)) == NULL)
            return 0;
#endif
          FCEU_MemoryRand(VROM, CHRRAMSize);
#ifndef TARGET_GNW
          UNIFchrrama = VROM;
#endif
          SetupCartCHRMapping(0, VROM, CHRRAMSize, 1);
          AddExState(VROM, CHRRAMSize, 0, "CHRR");
          /* FCEU_printf(" CHR-RAM:  %3d KiB\n", CHRRAMSize / 1024); */
        }
      }
      if (head.ROM_type & 8) {
        if (ExtraNTARAM != NULL) {
          AddExState(ExtraNTARAM, 2048, 0, "EXNR");
        }
      }
      FCEU_printf("init found mapper %ld\n", tmp->number);

      tmp->init(&iNESCart);
      return 1;
    }
    tmp++;
  }
  return 0;
}
#endif