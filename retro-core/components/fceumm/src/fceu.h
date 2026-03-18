#ifndef _FCEUH
#define _FCEUH

#include "fceu-types.h"
#include <stddef.h>

#ifndef RG_ATTR_EXT_RAM
#ifdef ESP_PLATFORM
#include <esp_attr.h>
#define RG_ATTR_EXT_RAM EXT_RAM_BSS_ATTR
#else
#define RG_ATTR_EXT_RAM __attribute__((section(".ext_ram.bss")))
#endif
#endif

extern int fceuindbg;

/* Overclocking-related */
#ifndef FCEU_NO_OVERCLOCKING
extern unsigned overclock_enabled;
extern unsigned overclocked;
extern unsigned skip_7bit_overclocking;
extern unsigned DMC_7bit;
#endif
extern unsigned totalscanlines;
extern unsigned normal_scanlines;
#ifndef FCEU_NO_OVERCLOCKING
extern unsigned extrascanlines;
extern unsigned vblankscanlines;
#endif

/* Region selection */
extern unsigned dendy;

/* Audio mods*/
#ifndef TARGET_GNW
extern unsigned swapDuty; /* Swap bits 6 & 7 of $4000/$4004 to mimic bug
                           * found on some famiclones/Dendy models.
                           */
#endif

void ResetGameLoaded(void);

#define DECLFR(x) uint8 FP_FASTAPASS(1) x(uint32 A)
#define DECLFW(x) void FP_FASTAPASS(2) x(uint32 A, uint8 V)

void FCEU_MemoryRand(uint8 *ptr, uint32 size);
void FASTAPASS(3) SetReadHandler(int32 start, int32 end, readfunc func);
void FASTAPASS(3) SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc FASTAPASS(1) GetWriteHandler(int32 a);
readfunc FASTAPASS(1) GetReadHandler(int32 a);
#ifdef FCEU_LOW_RAM
uint8 fceu_read(int32 a);
void fceu_write(int32 a, uint8 v);
#endif

int AllocGenieRW(void);
void FlushGenieRW(void);

void FCEU_ResetVidSys(void);

void ResetMapping(void);
void ResetNES(void);
void PowerNES(void);

void FCEU_OpenGenie(void);
void FCEU_CloseGenie(void);
void FCEUI_GeniePower(void);

// Memory Table Allocation
void FCEU_AllocMemoryTables(void);

#include "gw_malloc.h"

extern uint64 timestampbase;
extern uint32 MMC5HackVROMMask;
extern uint8 *MMC5HackExNTARAMPtr;
extern int MMC5Hack, PEC586Hack;
extern uint8 *MMC5HackVROMPTR;
extern uint8 MMC5HackCHRMode;
extern uint8 MMC5HackSPMode;
extern uint8 MMC50x5130;
extern uint8 MMC5HackSPScroll;
extern uint8 MMC5HackSPPage;

extern uint8 RAM[0x800];

#ifndef FCEU_LOW_RAM
extern readfunc *ARead;
extern writefunc *BWrite;
#endif

extern void (*GameInterface)(int h);
extern void (*GameStateRestore)(int version);

#define GI_RESETM2 1
#define GI_POWER 2
#define GI_CLOSE 3

#include "git.h"
extern FCEUGI *GameInfo;

extern uint8 PAL;

#include "driver.h"

typedef struct {
  int PAL;
  int SoundVolume;
  int TriangleVolume;
  int SquareVolume[2];
  int NoiseVolume;
  int PCMVolume;
  int GameGenie;

  /* Current first and last rendered scanlines. */
  int FirstSLine;
  int LastSLine;

  /* Driver code(user)-specified first and last rendered scanlines.
   * Usr*SLine[0] is for NTSC, Usr*SLine[1] is for PAL.
   */
  int UsrFirstSLine[2];
  int UsrLastSLine[2];
  uint32 SndRate;
  int soundq;
  int lowpass;
} FCEUS;

extern FCEUS FSettings;

void FCEU_PrintError(char *format, ...);
void FCEU_printf(char *format, ...);

void SetNESDeemph(uint8 d, int force);
void DrawTextTrans(uint8 *dest, uint32 width, uint8 *textmsg, uint8 fgcolor);
void FCEU_PutImage(void);
#if defined(FRAMESKIP) || defined(TARGET_GNW)
void FCEU_PutImageDummy(void);
#endif

extern uint8 Exit;
extern uint8 pale;
extern uint8 vsdip;

#define JOY_A 0x01
#define JOY_B 0x02
#define JOY_SELECT 0x04
#define JOY_START 0x08
#define JOY_UP 0x10
#define JOY_DOWN 0x20
#define JOY_LEFT 0x40
#define JOY_RIGHT 0x80

#endif
