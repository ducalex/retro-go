#ifndef _FCEU_CART_H
#define _FCEU_CART_H

#include "fceu.h"

typedef struct {
  /* Set by mapper/board code: */
  void (*Power)(void);
  void (*Reset)(void);
  void (*Close)(void);
  uint8 *SaveGame[4];    /* Pointers to memory to save/load. */
  uint32 SaveGameLen[4]; /* How much memory to save/load. */

  /* Set by iNES/UNIF loading code. */
  int iNES2;                          /* iNES version */
  int mapper;                         /* mapper used */
  int submapper; /* submapper used */ /* TODO: */
  int mirror;                         /* As set in the header or chunk.
                                       * iNES/UNIF specific.  Intended
                                       * to help support games like "Karnov"
                                       * that are not really MMC3 but are
                                       * set to mapper 4.
                                       */
  int battery;                        /* Presence of an actual battery. */
  int PRGRomSize;                     /* prg rom size in bytes */
  int CHRRomSize;                     /* chr rom size in bytes */
  int PRGRamSize;                     /* prg ram size in bytes (volatile) */
  int CHRRamSize;                     /* chr ram size in bytes (volatile) */
  int PRGRamSaveSize; /* prg ram size in bytes (non-volatile or battery backed)
                       */
  int CHRRamSaveSize; /* chr ram size in bytes (non-volatile or battery backed)
                       */
  int region;         /* video system timing (ntsc, pal, dendy */

  uint8 MD5[16];
  uint32 PRGCRC32;
  uint32 CHRCRC32;
  uint32 CRC32; /* Should be set by the iNES/UNIF loading
                 * code, used by mapper/board code, maybe
                 * other code in the future.
                 */
} CartInfo;

extern uint8 *Page[32], *VPage[8], *MMC5SPRVPage[8], *MMC5BGVPage[8];

void ResetCartMapping(void);
void SetupCartPRGMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartCHRMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartMirroring(int m, int hard, uint8 *extra);

DECLFR(CartBROB);
DECLFR(CartBR);
DECLFW(CartBW);

extern uint8 *PRGptr[32];
extern uint8 *CHRptr[32];

extern uint32 PRGsize[32];
extern uint32 CHRsize[32];

extern uint32 PRGmask2[32];
extern uint32 PRGmask4[32];
extern uint32 PRGmask8[32];
extern uint32 PRGmask16[32];
extern uint32 PRGmask32[32];

extern uint32 CHRmask1[32];
extern uint32 CHRmask2[32];
extern uint32 CHRmask4[32];
extern uint32 CHRmask8[32];

void FASTAPASS(2) setprg2(uint32 A, uint32 V);
void FASTAPASS(2) setprg4(uint32 A, uint32 V);
void FASTAPASS(2) setprg8(uint32 A, uint32 V);
void FASTAPASS(2) setprg16(uint32 A, uint32 V);
void FASTAPASS(2) setprg32(uint32 A, uint32 V);

void FASTAPASS(3) setprg2r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setprg4r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setprg8r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setprg16r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setprg32r(int r, uint32 A, uint32 V);

void FASTAPASS(3) setchr1r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setchr2r(int r, uint32 A, uint32 V);
void FASTAPASS(3) setchr4r(int r, uint32 A, uint32 V);
void FASTAPASS(2) setchr8r(int r, uint32 V);

void FASTAPASS(2) setchr1(uint32 A, uint32 V);
void FASTAPASS(2) setchr2(uint32 A, uint32 V);
void FASTAPASS(2) setchr4(uint32 A, uint32 V);
void FASTAPASS(2) setchr8(uint32 V);

void FASTAPASS(1) setmirror(int t);
void setmirrorw(int a, int b, int c, int d);
void FASTAPASS(3) setntamem(uint8 *p, int ram, uint32 b);

#define MI_H 0
#define MI_V 1
#define MI_0 2
#define MI_1 3

#ifdef FCEU_ENABLE_GAMEGENIE_ROM
extern int geniestage;

void FCEU_GeniePower(void);

void FCEU_OpenGenie(void);
void FCEU_CloseGenie(void);
void FCEU_KillGenie(void);
#endif

#endif
