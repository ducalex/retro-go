/* This file is part of Snes9x. See LICENSE file. */

#include "fxemu.h"
#include "fxinst.h"
#include "memmap.h"
#include <stdlib.h>
#include <string.h>

#include <retro_inline.h>

/* The FxChip Emulator's internal variables */
FxRegs_s GSU; /* This will be initialized when loading a ROM */

void FxFlushCache(void)
{
   GSU.vCacheBaseReg = 0;
   GSU.bCacheActive = false;
}

void fx_flushCache(void)
{
   GSU.bCacheActive = false;
}

void fx_updateRamBank(uint8_t Byte)
{
   /* Update BankReg and Bank pointer */
   GSU.vRamBankReg = (uint32_t) Byte & (FX_RAM_BANKS - 1);
   GSU.pvRamBank = GSU.apvRamBank[Byte & 0x3];
}

static INLINE void fx_readRegisterSpaceForCheck(void)
{
   R15 = (uint32_t) READ_WORD(&GSU.pvRegisters[30]);
}

static void fx_readRegisterSpaceForUse(void)
{
   static const uint32_t avHeight[] = { 128, 160, 192, 256 };
   static const uint32_t avMult[] = { 16, 32, 32, 64 };
   int32_t i;
   uint8_t* p = GSU.pvRegisters;

   /* Update R0 - R14 */
   for (i = 0; i < 15; i++, p += 2)
      GSU.avReg[i] = (uint32_t) READ_WORD(p);

   /* Update other registers */
   p = GSU.pvRegisters;
   GSU.vStatusReg = (uint32_t) READ_WORD(&GSU.pvRegisters[GSU_SFR]);
   GSU.vPrgBankReg = (uint32_t) GSU.pvRegisters[GSU_PBR];
   GSU.vRomBankReg = (uint32_t)p[GSU_ROMBR];
   GSU.vRamBankReg = ((uint32_t)p[GSU_RAMBR]) & (FX_RAM_BANKS - 1);
   GSU.vCacheBaseReg = (uint32_t) READ_WORD(&p[GSU_CBR]);

   /* Update status register variables */
   GSU.vZero = !(GSU.vStatusReg & FLG_Z);
   GSU.vSign = (GSU.vStatusReg & FLG_S) << 12;
   GSU.vOverflow = (GSU.vStatusReg & FLG_OV) << 16;
   GSU.vCarry = (GSU.vStatusReg & FLG_CY) >> 2;

   /* Set bank pointers */
   GSU.pvRamBank = GSU.apvRamBank[GSU.vRamBankReg & 0x3];
   GSU.pvRomBank = GSU.apvRomBank[GSU.vRomBankReg];
   GSU.pvPrgBank = GSU.apvRomBank[GSU.vPrgBankReg];

   /* Set screen pointers */
   GSU.pvScreenBase = &GSU.pvRam[ USEX8(p[GSU_SCBR]) << 10 ];
   i  =  (int32_t)(!!(p[GSU_SCMR] & 0x04));
   i |= ((int32_t)(!!(p[GSU_SCMR] & 0x20))) << 1;
   GSU.vScreenHeight = GSU.vScreenRealHeight = avHeight[i];
   GSU.vMode = p[GSU_SCMR] & 0x03;
   if (i == 3)
      GSU.vScreenSize = 32768;
   else
      GSU.vScreenSize = GSU.vScreenHeight * 4 * avMult[GSU.vMode];
   if (GSU.vPlotOptionReg & 0x10)
      GSU.vScreenHeight = 256; /* OBJ Mode (for drawing into sprites) */
   if (GSU.pvScreenBase + GSU.vScreenSize > GSU.pvRam + (GSU.nRamBanks * 65536))
      GSU.pvScreenBase =  GSU.pvRam + (GSU.nRamBanks * 65536) - GSU.vScreenSize;
   GSU.pfPlot = fx_apfPlotTable[GSU.vMode];
   GSU.pfRpix = fx_apfPlotTable[GSU.vMode + 5];

   fx_apfOpcodeTable[0x04c] = GSU.pfPlot;
   fx_apfOpcodeTable[0x14c] = GSU.pfRpix;
   fx_apfOpcodeTable[0x24c] = GSU.pfPlot;
   fx_apfOpcodeTable[0x34c] = GSU.pfRpix;

   if(GSU.vMode != GSU.vPrevMode || GSU.vPrevScreenHeight != GSU.vScreenHeight || GSU.vSCBRDirty)
      fx_computeScreenPointers();
}

void fx_dirtySCBR(void)
{
   GSU.vSCBRDirty = true;
}

void fx_computeScreenPointers(void)
{
   int32_t i, j, condition, mask, result;
   uint32_t apvIncrement, vMode, xIncrement;
   GSU.vSCBRDirty = false;

   /* Make a list of pointers to the start of each screen column*/
   vMode = GSU.vMode;
   condition = vMode - 2;
   mask = (condition | -condition) >> 31;
   result = (vMode & mask) | (3 & ~mask);
   vMode = result + 1;
   GSU.x[0] = 0;
   GSU.apvScreen[0] = GSU.pvScreenBase;
   apvIncrement = vMode << 4;

   if(GSU.vScreenHeight == 256)
   {
      GSU.x[16] = vMode << 12;
      GSU.apvScreen[16] = GSU.pvScreenBase + (vMode << 13);
      apvIncrement <<= 4;
      xIncrement = vMode << 4;

      for(i = 1, j = 17 ; i < 16 ; i++, j++)
      {
         GSU.x[i] = GSU.x[i - 1] + xIncrement;
         GSU.apvScreen[i] = GSU.apvScreen[i - 1] + apvIncrement;
         GSU.x[j] = GSU.x[j - 1] + xIncrement;
         GSU.apvScreen[j] = GSU.apvScreen[j - 1] + apvIncrement;
      }
   }
   else
   {
      xIncrement = (vMode * GSU.vScreenHeight) << 1;
      for(i = 1 ; i < 32 ; i++)
      {
         GSU.x[i] = GSU.x[i - 1] + xIncrement;
         GSU.apvScreen[i] = GSU.apvScreen[i - 1] + apvIncrement;
      }
   }
   GSU.vPrevMode = GSU.vMode;
   GSU.vPrevScreenHeight = GSU.vScreenHeight;
}

static INLINE void fx_writeRegisterSpaceAfterCheck(void)
{
   WRITE_WORD(&GSU.pvRegisters[30], R15);
}

static void fx_writeRegisterSpaceAfterUse(void)
{
   int32_t i;
   uint8_t* p = GSU.pvRegisters;
   for (i = 0; i < 15; i++, p += 2)
      WRITE_WORD(p, GSU.avReg[i]);

   /* Update status register */
   if (USEX16(GSU.vZero) == 0)
      SF(Z);
   else
      CF(Z);
   if (GSU.vSign & 0x8000)
      SF(S);
   else
      CF(S);
   if (GSU.vOverflow >= 0x8000 || GSU.vOverflow < -0x8000)
      SF(OV);
   else
      CF(OV);
   if (GSU.vCarry)
      SF(CY);
   else
      CF(CY);

   p = GSU.pvRegisters;
   WRITE_WORD(&p[GSU_SFR], GSU.vStatusReg);
   p[GSU_PBR] = (uint8_t) GSU.vPrgBankReg;
   p[GSU_ROMBR] = (uint8_t)GSU.vRomBankReg;
   p[GSU_RAMBR] = (uint8_t)GSU.vRamBankReg;
   WRITE_WORD(&p[GSU_CBR], GSU.vCacheBaseReg);
}

/* Reset the FxChip */
void FxReset(FxInit_s* psFxInfo)
{
   int32_t i;
   /* Clear all internal variables */
   memset(&GSU, 0, sizeof(FxRegs_s));

   /* Set default registers */
   GSU.pvSreg = GSU.pvDreg = &R0;

   /* Set RAM and ROM pointers */
   GSU.pvRegisters = psFxInfo->pvRegisters;
   GSU.nRamBanks = psFxInfo->nRamBanks;
   GSU.pvRam = psFxInfo->pvRam;
   GSU.nRomBanks = psFxInfo->nRomBanks;
   GSU.pvRom = psFxInfo->pvRom;
   GSU.vPrevScreenHeight = ~0;
   GSU.vPrevMode = ~0;

   /* The GSU can't access more than 2mb (16mbits) */
   if (GSU.nRomBanks > 0x20)
      GSU.nRomBanks = 0x20;

   /* Clear FxChip register space */
   memset(GSU.pvRegisters, 0, 0x300);

   /* Set FxChip version Number */
   GSU.pvRegisters[0x3b] = 0;

   /* Make ROM bank table */
   for (i = 0; i < 256; i++)
   {
      uint32_t b = i & 0x7f;
      if (b >= 0x40)
      {
         if (GSU.nRomBanks > 2)
            b %= GSU.nRomBanks;
         else
            b &= 1;

         GSU.apvRomBank[i] = &GSU.pvRom[b << 16];
      }
      else
      {
         b %= GSU.nRomBanks * 2;
         GSU.apvRomBank[i] = &GSU.pvRom[(b << 16) + 0x200000];
      }
   }

   /* Make RAM bank table */
   for (i = 0; i < 4; i++)
   {
      GSU.apvRamBank[i] = &GSU.pvRam[(i % GSU.nRamBanks) << 16];
      GSU.apvRomBank[0x70 + i] = GSU.apvRamBank[i];
   }

   /* Start with a nop in the pipe */
   GSU.vPipe = 0x01;

   fx_readRegisterSpaceForCheck();
   fx_readRegisterSpaceForUse();
}

static bool fx_checkStartAddress(void)
{
   /* Check if we start inside the cache */
   if (GSU.bCacheActive && R15 >= GSU.vCacheBaseReg && R15 < (GSU.vCacheBaseReg + 512))
      return true;

   /*  Check if we're in an unused area */
   if (GSU.vPrgBankReg >= 0x60 && GSU.vPrgBankReg <= 0x6f)
      return false;
   if (GSU.vPrgBankReg >= 0x74)
      return false;

   /* Check if we're in RAM and the RAN flag is not set */
   if (GSU.vPrgBankReg >= 0x70 && !(SCMR & (1 << 3)))
      return false;

   /* If not, we're in ROM, so check if the RON flag is set */
   if (!(SCMR & (1 << 4)))
      return false;

   return true;
}

/* Execute until the next stop instruction */
int32_t FxEmulate(uint32_t nInstructions)
{
   uint32_t vCount;

   /* Read registers and initialize GSU session */
   fx_readRegisterSpaceForCheck();

   /* Check if the start address is valid */
   if (!fx_checkStartAddress())
   {
      CF(G);
      fx_writeRegisterSpaceAfterCheck();
      return 0;
   }

   fx_readRegisterSpaceForUse();

   /* Execute GSU session */
   CF(IRQ);

   vCount = fx_run(nInstructions);

   /* Store GSU registers */
   fx_writeRegisterSpaceAfterCheck();
   fx_writeRegisterSpaceAfterUse();

   /* Check for error code */
   return vCount;
}
