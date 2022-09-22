/* This file is part of Snes9x. See LICENSE file. */

#ifndef _FXEMU_H_
#define _FXEMU_H_ 1

#include "snes9x.h"

/* The FxInfo_s structure, the link between the FxEmulator and the Snes Emulator */
typedef struct
{
   uint8_t*  pvRegisters; /* 768 bytes located in the memory at address 0x3000 */
   uint32_t  nRamBanks;   /* Number of 64kb-banks in GSU-RAM/BackupRAM (banks 0x70-0x73) */
   uint8_t*  pvRam;       /* Pointer to GSU-RAM */
   uint32_t  nRomBanks;   /* Number of 32kb-banks in Cart-ROM */
   uint8_t*  pvRom;       /* Pointer to Cart-ROM */
} FxInit_s;

/* Reset the FxChip */
extern void FxReset(FxInit_s* psFxInfo);

/* Execute until the next stop instruction */
extern int32_t FxEmulate(uint32_t nInstructions);

/* Write access to the cache */
extern void FxFlushCache(void); /* Called when the G flag in SFR is set to zero */

/* SCBR write seen.  We need to update our cached screen pointers */
extern void fx_dirtySCBR(void);

/* Update RamBankReg and RAM Bank pointer */
extern void fx_updateRamBank(uint8_t Byte);

extern void fx_computeScreenPointers(void);
#endif
