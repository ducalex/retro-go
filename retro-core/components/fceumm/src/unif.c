/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
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

/* TODO:  Battery backup file saving, mirror force    */
/* **INCOMPLETE**             */
/* Override stuff: CHR RAM instead of CHR ROM,   mirroring. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fceu-cart.h"
#include "fceu-crc32.h"
#include "fceu-endian.h"
#include "fceu-memory.h"
#include "fceu-state.h"
#include "fceu-types.h"
#include "fceu.h"
#include "file.h"
#include "general.h"
#include "ines.h"
#include "input.h"
#include "md5.h"
#include "unif.h"

#include "string/stdstring.h"

typedef struct {
  char ID[4];
  uint32 info;
} UNIF_HEADER;

typedef struct {
  char *name;
  int ines_mapper;
  void (*init)(CartInfo *);
  int flags;
} BMAPPING;

typedef struct {
  char *name;
  int (*init)(FCEUFILE *fp);
} BFMAPPING;

CartInfo UNIFCart;

static int vramo;
static int mirrortodo;
static int submapper;
static int cspecial;
static uint8 *boardname;
static uint8 *sboardname;

static uint32 CHRRAMSize;
uint8 *UNIFchrrama = 0;

static UNIF_HEADER unhead;
static UNIF_HEADER uchead;

static uint8 *malloced[32];
static uint32 mallocedsizes[32];
/* used to preserve the rom order as found in the rom file
 * at least one mapper has bank 4 at the beginning for e.g. */
static uint32 prg_idx[16];
static uint32 chr_idx[16];

static uint32 prg_chip_count;
static uint32 chr_chip_count;

static uint64 UNIF_PRGROMSize, UNIF_CHRROMSize;

static int FixRomSize(uint32 size, uint32 minimum) {
  uint32 x = 1;

  if (size < minimum)
    return minimum;
  while (x < size)
    x <<= 1;
  return x;
}

static void FreeUNIF(void) {
  int x;
  if (UNIFchrrama) {
    free(UNIFchrrama);
    UNIFchrrama = 0;
  }
  if (boardname) {
    free(boardname);
    boardname = 0;
  }
  for (x = 0; x < 32; x++) {
    if (malloced[x]) {
      free(malloced[x]);
      malloced[x] = 0;
    }
  }
  if (ROM) {
    free(ROM);
    ROM = 0;
  }
  if (VROM) {
    free(VROM);
    VROM = 0;
  }
}

static void ResetUNIF(void) {
  int x;
  for (x = 0; x < 32; x++)
    malloced[x] = 0;
  for (x = 0; x < 16; x++)
    prg_idx[x] = chr_idx[x] = 0;
  vramo = 0;
  boardname = 0;
  mirrortodo = 0;
  memset(&UNIFCart, 0, sizeof(UNIFCart));
  UNIFchrrama = 0;
  prg_chip_count = 0;
  chr_chip_count = 0;
  UNIF_PRGROMSize = 0;
  UNIF_CHRROMSize = 0;
  ROM_size = 0;
  VROM_size = 0;
}

static void Cleanup(void) {
  FreeUNIF();
  ResetUNIF();
}

static uint8 exntar[2048];

static void MooMirroring(void) {
  if (mirrortodo < 0x4)
    /* 06-22-19 Allow override when using vertical/horizontal mirroring. */
    SetupCartMirroring(mirrortodo, (mirrortodo >> 1) & 1, 0);
  else if (mirrortodo == 0x4) {
    SetupCartMirroring(4, 1, exntar);
    AddExState(exntar, 2048, 0, "EXNR");
  } else
    SetupCartMirroring(0, 0, 0);
}

static int DoMirroring(FCEUFILE *fp) {
  int t;
  uint32 i;
  if (uchead.info == 1) {
    if ((t = FCEU_fgetc(fp)) == EOF)
      return (0);
    mirrortodo = t;
    {
      static char *stuffo[6] = {
          "Horizontal", "Vertical",        "$2000",
          "$2400",      "\"Four-screen\"", "Controlled by Mapper Hardware"};
      if (t < 6)
        FCEU_printf(" Name/Attribute Table Mirroring: %s\n", stuffo[t]);
    }
  } else {
    FCEU_printf(" Incorrect Mirroring Chunk Size (%d). Data is:", uchead.info);
    for (i = 0; i < uchead.info; i++) {
      if ((t = FCEU_fgetc(fp)) == EOF)
        return (0);
      FCEU_printf(" %02x", t);
    }
    FCEU_printf("\n Default Name/Attribute Table Mirroring: Horizontal\n",
                uchead.info);
    mirrortodo = 0;
  }
  return (1);
}

static int NAME(FCEUFILE *fp) {
  char namebuf[100];
  int index;
  int t;

  /* FCEU_printf(" Name: "); */
  index = 0;

  while ((t = FCEU_fgetc(fp)) > 0)
    if (index < 99)
      namebuf[index++] = t;

  namebuf[index] = 0;
  FCEU_printf(" Name: %s\n", namebuf);

  if (!GameInfo->name) {
    GameInfo->name = malloc(strlen(namebuf) + 1);
    strcpy((char *)GameInfo->name, namebuf);
  }
  return (1);
}

static int DINF(FCEUFILE *fp) {
  char name[100], method[100];
  uint8 d, m;
  uint16 y;
  int t;

  if (FCEU_fread(name, 1, 100, fp) != 100)
    return (0);
  if ((t = FCEU_fgetc(fp)) == EOF)
    return (0);
  d = t;
  if ((t = FCEU_fgetc(fp)) == EOF)
    return (0);
  m = t;
  if ((t = FCEU_fgetc(fp)) == EOF)
    return (0);
  y = t;
  if ((t = FCEU_fgetc(fp)) == EOF)
    return (0);
  y |= t << 8;
  if (FCEU_fread(method, 1, 100, fp) != 100)
    return (0);
  name[99] = method[99] = 0;
  FCEU_printf(" Dumped by: %s\n", name);
  FCEU_printf(" Dumped with: %s\n", method);
  {
    char *months[12] = {"January",   "February", "March",    "April",
                        "May",       "June",     "July",     "August",
                        "September", "October",  "November", "December"};
    FCEU_printf(" Dumped on: %s %d, %d\n", months[(m - 1) % 12], d, y);
  }
  return (1);
}

static int CTRL(FCEUFILE *fp) {
  int t;
  uint32 i;
  if (uchead.info == 1) {
    if ((t = FCEU_fgetc(fp)) == EOF)
      return (0);
    /* The information stored in this byte isn't very helpful, but it's
    better than nothing...maybe.
    */

    if (t & 1)
      GameInfo->input[0] = GameInfo->input[1] = SI_GAMEPAD;
    else
      GameInfo->input[0] = GameInfo->input[1] = SI_NONE;
    if (t & 2)
      GameInfo->input[1] = SI_ZAPPER;
  } else {
    FCEU_printf(" Incorrect Control Chunk Size (%d). Data is:", uchead.info);
    for (i = 0; i < uchead.info; i++) {
      t = FCEU_fgetc(fp);
      FCEU_printf(" %02x", t);
    }
    FCEU_printf("\n");
    GameInfo->input[0] = GameInfo->input[1] = SI_GAMEPAD;
  }
  return (1);
}

static int TVCI(FCEUFILE *fp) {
  int t;
  if ((t = FCEU_fgetc(fp)) == EOF)
    return (0);
  if (t <= 2) {
    char *stuffo[3] = {"NTSC", "PAL", "NTSC and PAL"};
    if (t == 0)
      GameInfo->vidsys = GIV_NTSC;
    else if (t == 1)
      GameInfo->vidsys = GIV_PAL;
    FCEU_printf(" TV Standard Compatibility: %s\n", stuffo[t]);
  }
  return (1);
}

static int EnableBattery(FCEUFILE *fp) {
  int ret = FCEU_fgetc(fp);
  UNIFCart.battery = (ret > 0) ? 1 : 0;
  if (UNIFCart.battery)
    FCEU_printf(" Battery-backed.\n");
  return (1);
}

static int LoadPRG(FCEUFILE *fp) {
  int z, t;
  z = uchead.ID[3] - '0';

  if (z < 0 || z > 15)
    return (0);
  FCEU_printf(" PRG ROM %d size: %d\n", z, (int)uchead.info);
  if (malloced[z])
    free(malloced[z]);
  t = uchead.info;
  if (!(malloced[z] = (uint8 *)FCEU_malloc(t)))
    return (0);
  mallocedsizes[z] = t;
  memset(malloced[z] + uchead.info, 0xFF, t - uchead.info);
  if (FCEU_fread(malloced[z], 1, uchead.info, fp) != uchead.info) {
    FCEU_printf("Read Error!\n");
    return (0);
  }

  UNIF_PRGROMSize += t;
  prg_idx[prg_chip_count] = z;
  prg_chip_count++;

  return (1);
}

static int SetBoardName(FCEUFILE *fp) {
  if (!(boardname = (uint8 *)FCEU_malloc(uchead.info + 1)))
    return (0);
  FCEU_fread(boardname, 1, uchead.info, fp);
  boardname[uchead.info] = 0;
  /* strip whitespaces */
  boardname = (uint8 *)string_trim_whitespace((char *const)boardname);
  FCEU_printf(" Board name: %s\n", boardname);
  sboardname = boardname;
  if (!memcmp(boardname, "NES-", 4) || !memcmp(boardname, "UNL-", 4) ||
      !memcmp(boardname, "HVC-", 4) || !memcmp(boardname, "BTL-", 4) ||
      !memcmp(boardname, "BMC-", 4))
    sboardname += 4;

  return (1);
}

static int LoadCHR(FCEUFILE *fp) {
  int z, t;
  z = uchead.ID[3] - '0';
  if (z < 0 || z > 15)
    return (0);
  FCEU_printf(" CHR ROM %d size: %d\n", z, (int)uchead.info);
  if (malloced[16 + z])
    free(malloced[16 + z]);
  t = uchead.info;
  if (!(malloced[16 + z] = (uint8 *)FCEU_malloc(t)))
    return (0);
  mallocedsizes[16 + z] = t;
  memset(malloced[16 + z] + uchead.info, 0xFF, t - uchead.info);
  if (FCEU_fread(malloced[16 + z], 1, uchead.info, fp) != uchead.info) {
    FCEU_printf("Read Error!\n");
    return (0);
  }

  UNIF_CHRROMSize += t;
  chr_idx[chr_chip_count] = z;
  chr_chip_count++;

  return (1);
}

#define NO_BUSC 1

struct _unif_db {
  uint64 partialMD5;
  char *boardname;
  int submapper;
  int mirroring;
  int special; /* TODO: for bus conflicts, set 1 for no bus_conflict */
};

static struct _unif_db unif_db[] = {
    {0x03ed6963ca50e1d8ULL, "A65AS", 1, -1, -1},
    {0x616851e56946893bULL, "RESETNROM-XIN1", 0, MI_V,
     -1}, /* Sheng Tian 2-in-1(Unl,ResetBase)[p1].unf */
    {0x4cd729b5ae23a3cfULL, "RESETNROM-XIN1", 0, MI_H,
     -1}, /* Sheng Tian 2-in-1(Unl,ResetBase)[p2].unf */

    {0, NULL, -1, -1, -1} /* end of the line */
};

static void CheckHashInfo(void) {
  unsigned x = 0;
  uint64 partialMD5 = 0;

  for (x = 0; x < 8; x++)
    partialMD5 |= (uint64)UNIFCart.MD5[15 - x] << (x * 8);

  x = 0;
  do {
    if (partialMD5 == unif_db[x].partialMD5) {
      FCEU_printf("\n");
      FCEU_PrintError(" The UNIF header contains incorrect information.\n");
      FCEU_PrintError(" For now, the information will be corrected in RAM.\n");
      if (unif_db[x].boardname != NULL &&
          strcmp((char *)unif_db[x].boardname, (char *)sboardname) != 0) {
        FCEU_printf(" Boardname should be set to %s\n", unif_db[x].boardname);
        sboardname = (uint8 *)unif_db[x].boardname;
      }
      if (unif_db[x].submapper >= 0 && unif_db[x].submapper != submapper) {
        FCEU_PrintError(" Submapper should be set to %d\n",
                        unif_db[x].submapper);
        submapper = unif_db[x].submapper;
      }
      if (unif_db[x].mirroring >= 0 && unif_db[x].mirroring != mirrortodo) {
        static char *stuffo[6] = {
            "Horizontal", "Vertical",        "$2000",
            "$2400",      "\"Four-screen\"", "Controlled by Mapper Hardware"};
        FCEU_PrintError(" Mirroring should be set to %s\n",
                        stuffo[unif_db[x].mirroring]);
        mirrortodo = unif_db[x].mirroring;
      }
      if (unif_db[x].special >= 0 && unif_db[x].special != cspecial) {
        if (!(strcmp((char *)sboardname, "CNROM"))) {
          FCEU_PrintError(" Special flags applied, No bus conflict.\n");
          cspecial = unif_db[x].special;
        }
      }
      /* todo special case aka, dipswitches, busc-like in fk23c/a, etc */
      FCEU_printf("\n");
    }
    x++;
  } while (unif_db[x].partialMD5 > 0);
}

#define NO_INES -1
#define BMCFLAG_FORCE4 0x01
#define BMCFLAG_16KCHRR 0x02
#define BMCFLAG_32KCHRR 0x04
#define BMCFLAG_128KCHRR 0x08
#define BMCFLAG_256KCHRR 0x10

static BMAPPING bmap[] = {
    {"11160", 299, BMC11160_Init, 0},
    {"12-IN-1", 331, BMC12IN1_Init, 0},
    {"13in1JY110", 295, Mapper295_Init, 0},
    {"190in1", 300, BMC190in1_Init, 0},
    {"22211", 132, Mapper132_Init, 0},
    {"3D-BLOCK", 355, UNL3DBlock_Init, 0},
    {"411120-C", 287, BMC411120C_Init, 0},
    {"42in1ResetSwitch", 233, Mapper233_Init, 0},
    {"43272", 242, Mapper242_Init, 0},
    {"603-5052", 238, UNL6035052_Init, 0},
    {"64in1NoRepeat", 314, BMC64in1nr_Init, 0},
    {"70in1", 236, Mapper236_Init, 0},
    {"70in1B", 236, Mapper236_Init, 0},
    {"810544-C-A1", 261, BMC810544CA1_Init, 0},
    {"8157", 301, UNL8157_Init, 0},
    {"8237", 215, UNL8237_Init, 0},
    {"8237A", 215, UNL8237A_Init, 0},
    {"830118C", 348, BMC830118C_Init, 0},
    {"A65AS", 285, BMCA65AS_Init, 0},
    {"AB-G1L", 428, Mapper428_Init, 0},
    {"WELL-NO-DG450", 428, Mapper428_Init, 0},
    {"TF2740", 428, Mapper428_Init, 0},
    {"AC08", 42, AC08_Init, 0},
    {"ANROM", 7, ANROM_Init, 0},
    {"AX5705", 530, UNLAX5705_Init, 0},
    {"BB", 108, UNLBB_Init, 0},
    {"BS-110", 444, Mapper444_Init,
     0}, /* Due to a mix-up, UNIF MAPR BMC-BS-110 is actually the NC7000M PCB
            and refers to NES 2.0 Mapper 444 instead. */
    {"BS-5", 286, BMCBS5_Init, 0},
    {"CC-21", 27, UNLCC21_Init, 0},
    {"CITYFIGHT", 266, UNLCITYFIGHT_Init, 0},
    {"10-24-C-A1", 327, BMC1024CA1_Init, 0},
    {"CNROM", 3, CNROM_Init, 0},
    {"CPROM", 13, CPROM_Init, BMCFLAG_16KCHRR},
    {"D1038", 59, BMCD1038_Init, 0},
    {"T3H53", 59, BMCD1038_Init, 0},
    {"DANCE", 256, UNLOneBus_Init, 0},
    {"DANCE2000", 518, UNLD2000_Init, 0},
    {"DREAMTECH01", 521, DreamTech01_Init, 0},
    {"EDU2000", 329, UNLEDU2000_Init, 0},
    {"EKROM", 5, EKROM_Init, 0},
    {"ELROM", 5, ELROM_Init, 0},
    {"ETROM", 5, ETROM_Init, 0},
    {"EWROM", 5, EWROM_Init, 0},
    {"FK23C", 176, BMCFK23C_Init, BMCFLAG_256KCHRR},
    {"FK23CA", 176, BMCFK23CA_Init, BMCFLAG_256KCHRR},
    {"FS304", 162, Mapper162_Init, 0},
    {"G-146", 349, BMCG146_Init, 0},
    {"GK-192", 58, Mapper58_Init, 0},
    {"GS-2004", 283, Mapper283_Init, 0},
    {"GS-2013", 283, Mapper283_Init, 0},
    {"Ghostbusters63in1", 226, Mapper226_Init, 0},
    {"G631", 226, Mapper226_Init, 0}, /* duplicate, probably wrong name */
    {"H2288", 123, UNLH2288_Init, 0},
    {"HKROM", 4, HKROM_Init, 0},
    {"KOF97", 263, UNLKOF97_Init, 0},
    /*	{ "KONAMI-QTAI",            NO_INES, Mapper190_Init,        0 }, */
    {"KS7012", 346, UNLKS7012_Init, 0},
    {"KS7013B", 312, UNLKS7013B_Init, 0},
    {"KS7016", 306, UNLKS7016_Init, 0},
    {"KS7017", 303, UNLKS7017_Init, 0},
    {"KS7030", 347, UNLKS7030_Init, 0},
    {"KS7031", 305, UNLKS7031_Init, 0},
    {"KS7032", 142, UNLKS7032_Init, 0},
    {"KS7037", 307, UNLKS7037_Init, 0},
    {"KS7057", 302, UNLKS7057_Init, 0},
    {"LE05", NO_INES, LE05_Init, 0},
    {"LH10", 522, LH10_Init, 0},
    {"LH32", 125, LH32_Init, 0},
    {"LH53", 535, LH53_Init, 0},
    {"MALISB", 325, UNLMaliSB_Init, 0},
    {"MARIO1-MALEE2", 42, MALEE_Init, 0},
    {"MHROM", 66, MHROM_Init, 0},
    {"N625092", 221, UNLN625092_Init, 0},
    {"NROM", 0, NROM_Init, 0},
    {"NROM-128", 0, NROM_Init, 0},
    {"NROM-256", 0, NROM_Init, 0},
    {"NTBROM", 68, Mapper68_Init, 0},
    {"NTD-03", 290, BMCNTD03_Init, 0},
    {"NovelDiamond9999999in1", 201, Novel_Init, 0},
    {"OneBus", 256, UNLOneBus_Init, 0},
    {"PEC-586", NO_INES, UNLPEC586Init, 0},
    {"RROM", 0, NROM_Init, 0},
    {"RROM-128", 0, NROM_Init, 0},
    {"SA-002", 136, Mapper136_Init, 0},
    {"SA-0036", 149, SA0036_Init, 0},
    {"SA-0037", 148, SA0037_Init, 0},
    {"SA-009", 160, SA009_Init, 0},
    {"SA-016-1M", 146, SA0161M_Init, 0},
    {"SA-72007", 145, SA72007_Init, 0},
    {"SA-72008", 133, SA72008_Init, 0},
    {"SA-9602B", 513, SA9602B_Init, BMCFLAG_32KCHRR},
    {"SA-NROM", 143, TCA01_Init, 0},
    {"SAROM", 1, SAROM_Init, 0},
    {"SBROM", 1, SBROM_Init, 0},
    {"SC-127", 35, Mapper35_Init, 0},
    {"SCROM", 1, SCROM_Init, 0},
    {"SEROM", 1, SEROM_Init, 0},
    {"SGROM", 1, SGROM_Init, 0},
    {"SHERO", 262, UNLSHeroes_Init, 0},
    {"SKROM", 1, SKROM_Init, 0},
    {"SL12", 116, UNLSL12_Init, 0},
    {"SL1632", 14, UNLSL1632_Init, 0},
    {"SL1ROM", 1, SL1ROM_Init, 0},
    {"SLROM", 1, SLROM_Init, 0},
    {"SMB2J", 304, UNLSMB2J_Init, 0},
    {"SNROM", 1, SNROM_Init, 0},
    {"SOROM", 1, SOROM_Init, 0},
    {"SSS-NROM-256", NO_INES, SSSNROM_Init, 0},
    {"SUNSOFT_UNROM", 93, SUNSOFT_UNROM_Init,
     0}, /* fix me, real pcb name, real pcb type */
    {"Sachen-74LS374N", 150, S74LS374N_Init, 0},
    {"Sachen-74LS374NA", 150, S74LS374N_Init,
     0}, /* seems to be custom mapper */
    {"Sachen-8259A", 141, S8259A_Init, 0},
    {"Sachen-8259B", 138, S8259B_Init, 0},
    {"Sachen-8259C", 139, S8259C_Init, 0},
    {"Sachen-8259D", 137, S8259D_Init, 0},
    {"Super24in1SC03", 176, Super24_Init, 0},
    {"SuperHIK8in1", 45, Mapper45_Init, 0},
    {"Supervision16in1", 53, Supervision16_Init, 0},
    {"T-227-1", NO_INES, BMCT2271_Init, 0},
    {"T-230", 529, UNLT230_Init, 0},
    {"T-262", 265, BMCT262_Init, 0},
    {"TBROM", 4, TBROM_Init, 0},
    {"TC-U01-1.5M", 147, Mapper147_Init, 0},
    {"TEK90", 90, Mapper90_Init, 0},
    {"TEROM", 4, TEROM_Init, 0},
    {"TF1201", 298, UNLTF1201_Init, 0},
    {"TFROM", 4, TFROM_Init, 0},
    {"TGROM", 4, TGROM_Init, 0},
    {"TKROM", 4, TKROM_Init, 0},
    {"TKSROM", 118, TKSROM_Init, 0},
    {"TLROM", 4, TLROM_Init, 0},
    {"TLSROM", 118, TLSROM_Init, 0},
    {"TQROM", 119, TQROM_Init, 0},
    {"TR1ROM", 4, TFROM_Init, BMCFLAG_FORCE4},
    {"TSROM", 4, TSROM_Init, 0},
    {"TVROM", 4, TLROM_Init, BMCFLAG_FORCE4},
    {"Transformer", NO_INES, Transformer_Init, 0},
    {"UNROM", 2, UNROM_Init, 0},
    {"UNROM-512-8", 30, UNROM512_Init, 0},
    {"UNROM-512-16", 30, UNROM512_Init, BMCFLAG_16KCHRR},
    {"UNROM-512-32", 30, UNROM512_Init, BMCFLAG_32KCHRR},
    {"UOROM", 2, UNROM_Init, 0},
    {"VRC7", 85, UNLVRC7_Init, 0},
    {"YOKO", 264, UNLYOKO_Init, 0},
    {"COOLBOY", 268, COOLBOY_Init, BMCFLAG_256KCHRR},
    {"MINDKIDS", 268, MINDKIDS_Init, BMCFLAG_256KCHRR},
    {"158B", 258, UNL8237_Init, 0},
    {"DRAGONFIGHTER", 292, UNLBMW8544_Init, 0},
    {"EH8813A", 519, UNLEH8813A_Init, 0},
    {"HP898F", 319, BMCHP898F_Init, 0},
    {"F-15", 259, BMCF15_Init, 0},
    {"RT-01", 328, UNLRT01_Init, 0},
    {"81-01-31-C", NO_INES, BMC810131C_Init, 0},
    {"8-IN-1", 333, BMC8IN1_Init, 0},
    {"RET-CUFROM", 29, Mapper29_Init, BMCFLAG_32KCHRR},
    {"60311C", 289, BMC60311C_Init, 0},
    {"WS", 332, BMCWS_Init, 0},
    {"HPxx", 260, BMCHPxx_Init, 0},
    {"HP2018-A", 260, BMCHPxx_Init, 0},
    {"CHINA_ER_SAN2", 19, Mapper19_Init, 0},
    {"WAIXING-FW01", 227, Mapper227_Init, 0},
    {"WAIXING-FS005", 176, WAIXINGFS005_Init, 0},
    {"80013-B", 274, BMC80013B_Init, 0},
    {"TH2131-1", 308, UNLTH21311_Init, 0},
    {"LH51", 309, LH51_Init, 0},
    {"RESETNROM-XIN1", NO_INES, BMCRESETNROMXIN1_Init, 0}, /* split roms */
    {"RESET-TXROM", 313, BMCRESETTXROM_Init, 0},
    {"K-3088", 287, BMCK3088_Init, 0},
    {"FARID_SLROM_8-IN-1", 323, FARIDSLROM8IN1_Init, 0},
    {"830425C-4391T", 320, BMC830425C4391T_Init, 0},
    {"TJ-03", 341, BMCTJ03_Init, 0},
    {"CTC-09", 335, BMCCTC09_Init, 0},
    {"K-3046", 336, BMCK3046_Init, 0},
    {"SA005-A", 338, BMCSA005A_Init, 0},
    {"K-3006", 339, BMCK3006_Init, 0},
    {"K-3036", 340, BMCK3036_Init, 0},
    {"KS7021A", 525, UNLKS7021A_Init, 0},
    {"KS106C", NO_INES, BMCKS106C_Init, 0}, /* split roms */
    {"900218", 524, BTL900218_Init, 0},
    {"JC-016-2", 205, Mapper205_Init, 0},
    {"AX-40G", 527, UNLAX40G_Init, 0},
    {"STREETFIGTER-GAME4IN1", NO_INES, BMCSFGAME4IN1_Init,
     0}, /* mapper 49? submapper 1*/
    {"BJ-56", 526, UNLBJ56_Init, 0},
    {"L6IN1", 345, BMCL6IN1_Init, 0},
    {"CTC-12IN1", 337, BMCCTC12IN1_Init, 0},
    {"891227", 350, BMC891227_Init, 0},
    {"NEWSTAR-GRM070-8IN1", 333, BMC8IN1_Init, 0},
    {"FARID_UNROM_8-IN-1", 324, FARIDUNROM_Init, 0},
    {"K-3033", 322, BMCK3033_Init, 0},
    {"830134C", 315, BMC830134C_Init, 0},
    {"GN-26", 344, BMCGN26_Init, 0},
    {"KG256", NO_INES, KG256_Init, 0},
    {"T4A54A", 134, Mapper134_Init, 0},
    {"WX-KB4K", 134, Mapper134_Init, 0},
    {"SB-5013", 359, Mapper359_Init, 0},
    {"82112C", 540, Mapper540_Init, 0},
    {"N49C-300", 369, Mapper369_Init, 0},
    {"830752C", 396, Mapper396_Init, 0},

    {"BS-400R", 422, Mapper422_Init, 0},
    {"BS-4040R", 422, Mapper422_Init, 0},
    {"22026", 271, Mapper271_Init, 0},
    {"COOLGIRL", 342, COOLGIRL_Init, BMCFLAG_256KCHRR},

    {NULL, NO_INES, NULL, 0}};

static BFMAPPING bfunc[] = {{"CTRL", CTRL},          {"TVCI", TVCI},
                            {"BATR", EnableBattery}, {"MIRR", DoMirroring},
                            {"PRG", LoadPRG},        {"CHR", LoadCHR},
                            {"NAME", NAME},          {"MAPR", SetBoardName},
                            {"DINF", DINF},          {NULL, NULL}};

int LoadUNIFChunks(FCEUFILE *fp) {
  int x;
  int t;
  for (;;) {
    t = FCEU_fread(&uchead, 1, 4, fp);
    if (t < 4) {
      if (t > 0)
        return 0;
      return 1;
    }
    if (!(FCEU_read32le(&uchead.info, fp)))
      return 0;
    t = 0;
    x = 0;
    while (bfunc[x].name) {
      if (!memcmp(&uchead, bfunc[x].name, strlen(bfunc[x].name))) {
        if (!bfunc[x].init(fp))
          return 0;
        t = 1;
        break;
      }
      x++;
    }
    if (!t)
      if (FCEU_fseek(fp, uchead.info, SEEK_CUR))
        return (0);
  }
}

static int InitializeBoard(void) {
  int x = 0;

  if (!sboardname)
    return (0);

  while (bmap[x].name) {
    /* ignore case during board name comparing */
    if (string_is_equal_noncase((const char *)sboardname,
                                (const char *)bmap[x].name)) {

      if (VROM_size == 0) {
        if (bmap[x].flags & BMCFLAG_16KCHRR)
          CHRRAMSize = 16;
        else if (bmap[x].flags & BMCFLAG_32KCHRR)
          CHRRAMSize = 32;
        else if (bmap[x].flags & BMCFLAG_128KCHRR)
          CHRRAMSize = 128;
        else if (bmap[x].flags & BMCFLAG_256KCHRR)
          CHRRAMSize = 256;
        else
          CHRRAMSize = 8;
        CHRRAMSize <<= 10;
        if ((UNIFchrrama = (uint8 *)FCEU_malloc(CHRRAMSize))) {
          SetupCartCHRMapping(0, UNIFchrrama, CHRRAMSize, 1);
          AddExState(UNIFchrrama, CHRRAMSize, 0, "CHRR");
        } else
          return (-1);
      }
      if (bmap[x].flags & BMCFLAG_FORCE4)
        mirrortodo = 4;
      MooMirroring();

      UNIFCart.mapper = bmap[x].ines_mapper;
      UNIFCart.submapper = submapper;
      GameInfo->cspecial = cspecial;

      bmap[x].init(&UNIFCart);
      return (1);
    }
    x++;
  }
  FCEU_printf("\n");
  FCEU_PrintError(" Board type not supported, '%s'.\n", boardname);
  return (0);
}

static void UNIFGI(int h) {
  switch (h) {
  case GI_RESETM2:
    if (UNIFCart.Reset)
      UNIFCart.Reset();
    break;
  case GI_POWER:
    if (UNIFCart.Power)
      UNIFCart.Power();
    if (UNIFchrrama)
      memset(UNIFchrrama, 0, 8192);
    break;
  case GI_CLOSE:
    if (UNIFCart.Close)
      UNIFCart.Close();
    FreeUNIF();
    break;
  }
}

int UNIFLoad(const char *name, FCEUFILE *fp) {
  struct md5_context md5;
  uint64 prg_size_bytes = 0, chr_size_bytes = 0;
  int x = 0;

  FCEU_fseek(fp, 0, SEEK_SET);
  FCEU_fread(&unhead, 1, 4, fp);
  if (memcmp(&unhead, "UNIF", 4))
    return 0;

  ResetCartMapping();

  ResetExState(0, 0);
  ResetUNIF();
  if (!FCEU_read32le(&unhead.info, fp)) {
    Cleanup();
    return 0;
  }
  if (FCEU_fseek(fp, 0x20, SEEK_SET) < 0) {
    Cleanup();
    return 0;
  }
  if (!LoadUNIFChunks(fp)) {
    Cleanup();
    return 0;
  }

  ROM_size = (UNIF_PRGROMSize / 0x1000) + ((UNIF_PRGROMSize % 0x1000) ? 1 : 0);
  ROM_size = (ROM_size >> 2) + ((ROM_size & 3) ? 1 : 0);
  if (UNIF_CHRROMSize) {
    VROM_size = (UNIF_CHRROMSize / 0x400) + ((UNIF_CHRROMSize % 0x400) ? 1 : 0);
    VROM_size = (VROM_size >> 3) + ((VROM_size & 7) ? 1 : 0);
  }

  UNIF_PRGROMSize = FixRomSize(UNIF_PRGROMSize, 2048);
  if (UNIF_CHRROMSize)
    UNIF_CHRROMSize = FixRomSize(UNIF_CHRROMSize, 8192);

  /* Note: Use rounded size for memory allocations and board mapping */

  if (!(ROM = (uint8 *)malloc(UNIF_PRGROMSize))) {
    Cleanup();
    return 0;
  }
  if (UNIF_CHRROMSize) {
    if (!(VROM = (uint8 *)malloc(UNIF_CHRROMSize))) {
      Cleanup();
      return 0;
    }
  }

  /* combine multiple prg/chr blocks into single blocks and free memory used. */

  for (x = 0; x < 16; x++) {
    int p = prg_idx[x];
    int c = 16 + chr_idx[x];
    if (malloced[p]) {
      memcpy(ROM + prg_size_bytes, malloced[p], mallocedsizes[p]);
      prg_size_bytes += mallocedsizes[p];
      free(malloced[p]);
      malloced[p] = 0;
    }

    if (malloced[c]) {
      memcpy(VROM + chr_size_bytes, malloced[c], mallocedsizes[c]);
      chr_size_bytes += mallocedsizes[c];
      free(malloced[c]);
      malloced[c] = 0;
    }
  }

  /* Note: Use raw size in bytes for checksums */

  UNIFCart.PRGRomSize = prg_size_bytes;
  UNIFCart.CHRRomSize = chr_size_bytes;

  UNIFCart.PRGCRC32 = CalcCRC32(0, ROM, prg_size_bytes);
  UNIFCart.CHRCRC32 = CalcCRC32(0, VROM, chr_size_bytes);
  UNIFCart.CRC32 = CalcCRC32(UNIFCart.PRGCRC32, VROM, chr_size_bytes);

  md5_starts(&md5);
  md5_update(&md5, ROM, prg_size_bytes);
  if (chr_size_bytes)
    md5_update(&md5, VROM, chr_size_bytes);
  md5_finish(&md5, (uint8 *)UNIFCart.MD5);
  memcpy(GameInfo->MD5, UNIFCart.MD5, sizeof(UNIFCart.MD5));

  CheckHashInfo();

  /* Note: Use rounded size for board mappings */

  SetupCartPRGMapping(0, ROM, UNIF_PRGROMSize, 0);
  if (UNIF_CHRROMSize)
    SetupCartCHRMapping(0, VROM, UNIF_CHRROMSize, 0);

  FCEU_printf(" PRG-ROM CRC32: 0x%08X\n", UNIFCart.PRGCRC32);
  FCEU_printf(" PRG+CHR CRC32: 0x%08X\n", UNIFCart.CRC32);
  FCEU_printf(" PRG+CHR MD5  : 0x%s\n", md5_asciistr((uint8 *)UNIFCart.MD5));

  if (!InitializeBoard()) {
    Cleanup();
    return 0;
  }

  FCEU_printf(" [UNIF] PRG ROM: %u KiB\n", UNIFCart.PRGRomSize / 1024);
  FCEU_printf(" [UNIF] CHR ROM: %u KiB\n", UNIFCart.CHRRomSize / 1024);
  FCEU_printf(" [UNIF] iNES Mapper: %d\n", UNIFCart.mapper);
  FCEU_printf(" [UNIF] SubMapper: %d\n", UNIFCart.submapper);

  GameInterface = UNIFGI;

  return 1;
}
