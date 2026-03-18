#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *  Copyright (C) 2005 CaH4e3
 *  Copyright (C) 2019 Libretro Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "mapinc.h"
#include "mmc3.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

void 	(*sync)(void);
static uint8	allowExtendedMirroring;

static uint8	mode[4];
static uint8*	WRAM = NULL;
static uint32	WRAMSIZE;

static uint8	irqControl;
static uint8	irqEnabled;
static uint8	irqPrescaler;
static uint8	irqCounter;
static uint8	irqXor;
static uint32	lastPPUAddress;

static uint8	prg[4];
static uint16	chr[8];
static uint16	nt[4];
static uint8	latch[2];
static uint8	mul[2];
static uint8	adder;
static uint8	test;
static uint8    dipSwitch;

// On the G&W we can't afford using 256kB of memory for this
// this mapper is hacked to build but it may not work correctly
// because of that
#ifndef TARGET_GNW
static uint8 cpuWriteHandlersSet;
static writefunc cpuWriteHandlers[0x10000]; /* Actual write handlers for CPU write trapping as a method fo IRQ clocking */
#endif

static SFORMAT JYASIC_stateRegs[] = {
	{ &irqControl,   1,                  "IRQM" },
	{ &irqPrescaler, 1,                  "IRQP" },
	{ &irqCounter,   1,                  "IRQC" },
	{ &irqXor,       1,                  "IRQX" },
	{ &irqEnabled,   1,                  "IRQA" },
	{ mul,           2,                  "MUL"  },
	{ &test,         1,                  "REGI" },
	{ mode ,         4,                  "TKCO" },
	{ prg,           4,                  "PRGB" },
	{ latch,         2,                  "CLTC" },
	{ chr,           8*2,                "CHRB" },
	{ &nt[0],        2 | FCEUSTATE_RLSB, "NMS0" },
	{ &nt[1],        2 | FCEUSTATE_RLSB, "NMS1" },
	{ &nt[2],        2 | FCEUSTATE_RLSB, "NMS2" },
	{ &nt[3],        2 | FCEUSTATE_RLSB, "NMS3" },
	{ &dipSwitch,    1,                  "TEKR" },
	{ &adder,        1,                  "ADDE" },
	{ 0 }
};

static uint8 rev (uint8_t val)
{
	return ((val <<6) &0x40) | ((val <<4) &0x20) | ((val <<2) &0x10) | (val &0x08) | ((val >>2) &0x04) | ((val >>4) &0x02) | ((val >>6) &0x01);
}

static void syncPRG (int AND, int OR)
{
	uint8_t prgLast =mode[0] &0x04? prg[3]: 0xFF;
	uint8_t prg6000 =0;
	switch (mode[0] &0x03)
   {
      case 0:
         setprg32(0x8000, prgLast &AND >>2 |OR >>2);
         prg6000 =prg[3] <<2 |3;
         break;
      case 1:
         setprg16(0x8000, prg[1]  &AND >>1 |OR >>1);
         setprg16(0xC000, prgLast &AND >>1 |OR >>1);
         prg6000 =prg[3] <<1 |1;
         break;
      case 2:
         setprg8(0x8000, prg[0] &AND |OR);
         setprg8(0xA000, prg[1] &AND |OR);
         setprg8(0xC000, prg[2] &AND |OR);
         setprg8(0xE000, prgLast   &AND |OR);
         prg6000 =prg[3];
         break;
      case 3:
         setprg8(0x8000, rev(prg[0]) &AND |OR);
         setprg8(0xA000, rev(prg[1]) &AND |OR);
         setprg8(0xC000, rev(prg[2]) &AND |OR);
         setprg8(0xE000, rev(  prgLast) &AND |OR);
         prg6000 =rev(prg[3]);
         break;
   }
	if (mode[0] &0x80) /* Map ROM */
		setprg8 (0x6000, prg6000 &AND |OR);
	else
      if (WRAMSIZE)   /* Otherwise map WRAM if it exists */
         setprg8r(0x10, 0x6000, 0);
}

static void syncCHR (int AND, int OR)
{
   /* MMC4 mode[0] with 4 KiB CHR mode[0] */
   if (mode[3] &0x80 && (mode[0] &0x18) ==0x08)
   {
      int chrBank;
      for (chrBank =0; chrBank <8; chrBank +=4)
         setchr4(0x400 *chrBank, chr[latch[chrBank /4]&2 | chrBank] &AND >>2 | OR >>2);
   }
   else
   {
      int chrBank;
      switch(mode[0] &0x18)
      {
         case 0x00: /* 8 KiB CHR mode[0] */
            setchr8(chr[0] &AND >>3 | OR >>3);
            break;
         case 0x08: /* 4 KiB CHR mode[0] */
            for (chrBank =0; chrBank <8; chrBank +=4)
               setchr4(0x400 *chrBank, chr[chrBank] &AND >>2 | OR >>2);
            break;
         case 0x10:
            for (chrBank =0; chrBank <8; chrBank +=2)
               setchr2(0x400 *chrBank, chr[chrBank] &AND >>1 | OR >>1);
            break;
         case 0x18:
            for (chrBank =0; chrBank <8; chrBank +=1)
               setchr1(0x400 *chrBank, chr[chrBank] &AND     | OR    );
            break;
      }
   }

   PPUCHRRAM = (mode[2] & 0x40) ? 0xFF: 0x00; /* Write-protect or write-enable CHR-RAM */
}

static void syncNT (int AND, int OR)
{
	if (mode[0] &0x20 || mode[1] &0x08)
   {
      /* ROM nametables or extended mirroring */
      /* First, set normal CIRAM pages using extended registers ... */
      setmirrorw(nt[0] &1, nt[1] &1, nt[2] &1, nt[3] &1);

      if (mode[0] &0x20)
      {
         int ntBank;
         for (ntBank =0; ntBank <4; ntBank++)
         {
            /* Then replace with ROM nametables if such are generally enabled */
            int vromHere =(nt[ntBank] &0x80) ^(mode[2] &0x80) |(mode[0] &0x40);
            /* ROM nametables are used either when globally enabled via D000.6 or per-bank via B00x.7 vs. D002.7 */
            if (vromHere)
               setntamem(CHRptr[0] +0x400*((nt[ntBank] &AND | OR) & CHRmask1[0]), 0, ntBank);
         }
      }
   }
   else
      switch (mode[1] &0x03)
      {
         /* Regularly mirrored CIRAM */
         case 0:
            setmirror(MI_V);
            break;
         case 1:
            setmirror(MI_H);
            break;
         case 2:
            setmirror(MI_0);
            break;
         case 3:
            setmirror(MI_1);
            break;
      }
}

static void clockIRQ (void)
{
	uint8_t mask =irqControl &0x04? 0x07: 0xFF;
	if (irqEnabled)
      switch (irqControl &0xC0)
      {
         case 0x40:
            {
               uint8_t next = irqPrescaler + 1;
               irqPrescaler = (irqPrescaler & ~mask) | (next & mask);
            }
            if ((irqPrescaler &mask) ==0x00 && (irqControl &0x08? irqCounter: ++irqCounter) ==0x00)
               X6502_IRQBegin(FCEU_IQEXT);
            break;
         case 0x80:
            {
               uint8_t next = irqPrescaler - 1;
               irqPrescaler = (irqPrescaler & ~mask) | (next & mask);
            }
            if ((irqPrescaler &mask) ==mask && (irqControl &0x08? irqCounter: --irqCounter) ==0xFF)
               X6502_IRQBegin(FCEU_IQEXT);
            break;
      }
}

#ifndef TARGET_GNW
static DECLFW(trapCPUWrite)
{
	if ((irqControl &0x03) ==0x03)
      clockIRQ(); /* Clock IRQ counter on CPU writes */
	cpuWriteHandlers[A](A, V);
}
#endif

static void FP_FASTAPASS(1) trapPPUAddressChange (uint32 A)
{
   if ((irqControl &0x03) ==0x02 && lastPPUAddress !=A)
   {
      int i;
      for (i =0; i <2; i++)
         clockIRQ(); /* Clock IRQ counter on PPU "reads" */
   }
   if (mode[3] &0x80 && (mode[0] &0x18) ==0x08 && ((A &0x2FF0) ==0xFD0 || (A &0x2FF0) ==0xFE0))
   {
      /* If MMC4 mode[0] is enabled, and CHR mode[0] is 4 KiB, and tile FD or FE is being fetched ... */
      latch[A >>12 &1] =(A >>10 &4) | (A >>4 &2); /* ... switch the left or right pattern table's latch to 0 (FD) or 2 (FE), being used as an offset for the CHR register index. */
      sync();
   }
   lastPPUAddress =A;
}

static void ppuScanline(void)
{
	if ((irqControl &0x03) ==0x01)
   {
      int i;
      for (i =0; i <8; i++)
         clockIRQ(); /* Clock IRQ counter on A12 rises (eight per scanline). This should be done in trapPPUAddressChange, but would require more accurate PPU emulation for that. */
   }
}

static void FP_FASTAPASS(1) cpuCycle(int a)
{
   if ((irqControl &0x03) ==0x00)
      while (a--)
         clockIRQ(); /* Clock IRQ counter on M2 cycles */
}

static DECLFR(readALU_DIP)
{
   if ((A &0x3FF) ==0 && A !=0x5800) /* 5000, 5400, 5C00: read solder pad setting */
      return dipSwitch | X.DB &0x3F;

   if (A &0x800)
      switch (A &3)
      {
         /* 5800-5FFF: read ALU */
         case 0:
            return (mul[0] *mul[1]) &0xFF;
         case 1:
            return (mul[0] *mul[1]) >>8;
         case 2:
            return adder;
         case 3:
            return test;
      }
   /* all others */
   return X.DB;
}

static DECLFW(writeALU)
{
	switch (A &3)
   {
      case 0:
         mul[0] =V;
         break;
      case 1:
         mul[1] =V;
         break;
      case 2:
         adder +=V;
         break;
      case 3:
         test  = V;
         adder = 0;
         break;
   }
}

static DECLFW(writePRG)
{
	prg[A &3] = V;
	sync();	
}

static DECLFW(writeCHRLow)
{
	chr[A &7] =chr[A &7] &0xFF00 | V;
	sync();
}

static DECLFW(writeCHRHigh)
{
	chr[A &7] =chr[A &7] &0x00FF | V <<8;
	sync();
}

static DECLFW(writeNT)
{
	if (~A &4)
		nt[A &3] =nt[A &3] &0xFF00 | V;
	else
		nt[A &3] =nt[A &3] &0x00FF | V <<8;
	sync();
}

static DECLFW(writeIRQ)
{
	switch (A &7)
   {
      case 0:
         irqEnabled =!!(V &1);
         if (!irqEnabled)
         {
            irqPrescaler =0;
            X6502_IRQEnd(FCEU_IQEXT);
         }
         break;
      case 1:
         irqControl =V;
         break;
      case 2:
         irqEnabled =0;
         irqPrescaler =0;
         X6502_IRQEnd(FCEU_IQEXT);
         break;
      case 3:
         irqEnabled =1;
         break;
      case 4:
         irqPrescaler =V ^irqXor;
         break;
      case 5:
         irqCounter =V ^irqXor;
         break;
      case 6:
         irqXor =V;
         break;
   }
}

static DECLFW(writeMode)
{
	switch (A &3)
   {
      case 0:
         mode[0] =V;
         if (!allowExtendedMirroring)
            mode[0] &=~0x20;
         break;
      case 1:
         mode[1] =V;
         if (!allowExtendedMirroring)
            mode[1] &=~0x08;
         break;
      case 2:
         mode[2] =V;
         break;
      case 3:
         mode[3] =V;
         break;
   }
	sync();
}

static void JYASIC_restoreWriteHandlers(void)
{
#ifndef TARGET_GNW
   int i;
   if (cpuWriteHandlersSet) 
   {
	   for (i =0; i <0x10000; i++) SetWriteHandler(i, i, cpuWriteHandlers[i]);
	   cpuWriteHandlersSet =0;
   }
#endif
}

static void JYASIC_power(void)
{
#ifndef TARGET_GNW
   unsigned int i;
#endif

   SetWriteHandler(0x5000, 0x5FFF, writeALU);
   SetWriteHandler(0x6000, 0x7fff, CartBW);
   SetWriteHandler(0x8000, 0x87FF, writePRG);     /* 8800-8FFF ignored */
   SetWriteHandler(0x9000, 0x97FF, writeCHRLow);  /* 9800-9FFF ignored */
   SetWriteHandler(0xA000, 0xA7FF, writeCHRHigh); /* A800-AFFF ignored */
   SetWriteHandler(0xB000, 0xB7FF, writeNT);      /* B800-BFFF ignored */
   SetWriteHandler(0xC000, 0xCFFF, writeIRQ);
   SetWriteHandler(0xD000, 0xD7FF, writeMode);    /* D800-DFFF ignored */

   JYASIC_restoreWriteHandlers();
#ifndef TARGET_GNW
   for (i =0; i <0x10000; i++) cpuWriteHandlers[i] =GetWriteHandler(i);
   SetWriteHandler(0x0000, 0xFFFF, trapCPUWrite); /* Trap all CPU writes for IRQ clocking purposes */
   cpuWriteHandlersSet =1;
#endif

   SetReadHandler(0x5000, 0x5FFF, readALU_DIP);
   SetReadHandler(0x6000, 0xFFFF, CartBR);

   mul[0]     = mul[1]    = adder   = test    = dipSwitch = 0;
   mode[0]    = mode[1]   = mode[2] = mode[3] = 0;
   irqControl =irqEnabled = irqPrescaler =irqCounter = irqXor = lastPPUAddress = 0;
   memset(prg, 0, sizeof(prg));
   memset(chr, 0, sizeof(chr));
   memset(nt, 0, sizeof(nt));
   latch[0] =0;
   latch[1] =4;

   sync();
}

static void JYASIC_reset (void)
{
	dipSwitch = (dipSwitch +0x40) &0xC0;
}

static void JYASIC_close (void)
{
	if (WRAM)
      FCEU_gfree(WRAM);
   WRAM = NULL;
}

static void JYASIC_restore (int version)
{
	sync();
}

void JYASIC_init (CartInfo *info)
{
#ifndef TARGET_GNW
   cpuWriteHandlersSet =0;
#endif
   info->Reset = JYASIC_reset;
   info->Power = JYASIC_power;
   info->Close = JYASIC_close;
   PPU_hook = trapPPUAddressChange;
   MapIRQHook = cpuCycle;
   GameHBIRQHook2 = ppuScanline;
   AddExState(JYASIC_stateRegs, ~0, 0, 0);
   GameStateRestore = JYASIC_restore;

   /* WRAM is present only in iNES mapper 35, or in mappers with numbers above 255 that require NES 2.0, which explicitly denotes WRAM size */
   if (info->iNES2)
      WRAMSIZE =info->PRGRamSize + info->PRGRamSaveSize;
   else
      WRAMSIZE =info->mapper ==35? 8192: 0;

   if (WRAMSIZE)
   {
#ifndef FCEU_NO_MALLOC
      WRAM = (uint8*)FCEU_gmalloc(WRAMSIZE);
#else
      WRAM = (uint8*)ahb_calloc(1, WRAMSIZE);
#endif
      SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
      FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
   }
}

static void syncSingleCart (void)
{
   syncPRG(0x3F, mode[3] <<5 &~0x3F);
   if (mode[3] &0x20)
   {
      syncCHR(0x1FF,                        mode[3] <<6 &0x600);
      syncNT (0x1FF,                        mode[3] <<6 &0x600);
   }
   else
   {
      syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<6 &0x600);
      syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<6 &0x600);
   }
}
void Mapper35_Init(CartInfo *info)
{
   /* Basically mapper 90/209/211 with WRAM */
	allowExtendedMirroring =1;
	sync =syncSingleCart;
	JYASIC_init(info);
}
void Mapper90_Init(CartInfo *info)
{
   /* Single cart, extended mirroring and ROM nametables disabled */
	allowExtendedMirroring =0;
	sync =syncSingleCart;
	JYASIC_init(info);
}

void Mapper209_Init(CartInfo *info)
{
   /* Single cart, extended mirroring and ROM nametables enabled */
	allowExtendedMirroring =1;
	sync =syncSingleCart;
	JYASIC_init(info);
}

void Mapper211_Init(CartInfo *info)
{
   /* Duplicate of mapper 209 */
	allowExtendedMirroring =1;
	sync =syncSingleCart;
	JYASIC_init(info);
}

static void sync281 (void)
{
   syncPRG(0x1F, mode[3] <<5);
   syncCHR(0xFF, mode[3] <<8);
   syncNT (0xFF, mode[3] <<8);
}

void Mapper281_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync281;
	JYASIC_init(info);
}

static void sync282 (void)
{
	syncPRG(0x1F, mode[3] <<4 &~0x1F);
	if (mode[3] &0x20)
   {
		syncCHR(0x1FF,                            mode[3] <<6 &0x600);
		syncNT (0x1FF,                            mode[3] <<6 &0x600);
	}
   else
   {
		syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<6 &0x600);
		syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<6 &0x600);
	}
}

void Mapper282_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync282;
	JYASIC_init(info);
}

void sync295 (void)
{
   syncPRG(0x0F, mode[3] <<4);
   syncCHR(0x7F, mode[3] <<7);
   syncNT (0x7F, mode[3] <<7);
}

void Mapper295_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync295;
	JYASIC_init(info);
}

void sync358 (void)
{
   syncPRG(0x1F, mode[3] <<4 &~0x1F);
   if (mode[3] &0x20)
   {
      syncCHR(0x1FF, mode[3] <<7 &0x600);
      syncNT (0x1FF, mode[3] <<7 &0x600);
   }
   else
   {
      syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
      syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
   }
}

void Mapper358_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync358;
	JYASIC_init(info);
}

void sync386 (void)
{
   syncPRG(0x1F, mode[3] <<4 &0x20 | mode[3] <<3 &0x40);
   if (mode[3] &0x20)
   {
      syncCHR(0x1FF, mode[3] <<7 &0x600);
      syncNT (0x1FF, mode[3] <<7 &0x600);
   }
   else
   {
      syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
      syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
   }
}

void Mapper386_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync386;
	JYASIC_init(info);
}
	
void sync387(void)
{
	syncPRG(0x0F, mode[3] <<3 &0x10 | mode[3] <<2 &0x20);
	if (mode[3] &0x20)
   {
		syncCHR(0x1FF, mode[3] <<7 &0x600);
		syncNT (0x1FF, mode[3] <<7 &0x600);
	}
   else
   {
		syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
		syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<7 &0x600);
	}
}

void Mapper387_Init(CartInfo *info)
{
   /* Multicart */
   allowExtendedMirroring =1;
   sync =sync387;
   JYASIC_init(info);
}

void sync388 (void)
{
   syncPRG(0x1F, mode[3] <<3 &0x60);

   if (mode[3] &0x20)
   {
      syncCHR(0x1FF, mode[3] <<8 &0x200);
      syncNT (0x1FF, mode[3] <<8 &0x200);
   }
   else
   {
      syncCHR(0x0FF, mode[3] <<8 &0x100 | mode[3] <<8 &0x200);
      syncNT (0x0FF, mode[3] <<8 &0x100 | mode[3] <<8 &0x200);
   }
}

void Mapper388_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =0;
	sync =sync388;
	JYASIC_init(info);
}

void sync397 (void)
{
	syncPRG(0x1F, mode[3] <<4 &~0x1F);
	syncCHR(0x7F, mode[3] <<7);
	syncNT (0x7F, mode[3] <<7);
}

void Mapper397_Init(CartInfo *info)
{
   /* Multicart */
   allowExtendedMirroring =1;
   sync =sync397;
   JYASIC_init(info);
}

void sync421 (void)
{
   if (mode[3] &0x04)
      syncPRG(0x3F, mode[3] <<4 &~0x3F);
   else
      syncPRG(0x1F, mode[3] <<4 &~0x1F);
   syncCHR(0x1FF, mode[3] <<8 &0x300);
   syncNT (0x1FF, mode[3] <<8 &0x300);
}

void Mapper421_Init(CartInfo *info)
{
   /* Multicart */
	allowExtendedMirroring =1;
	sync =sync421;
	JYASIC_init(info);
}

/* Mapper 394: HSK007 circuit board that can simulate J.Y. ASIC, MMC3, and NROM. */
static uint8 HSK007Reg[4];
void sync394 (void) /* Called when J.Y. ASIC is active */
{
	int prgOR  =HSK007Reg[3] <<1 &0x010 | HSK007Reg[1] <<5 &0x020;
	int chrOR  =HSK007Reg[3] <<1 &0x080 | HSK007Reg[1] <<8 &0x100;
	syncPRG(0x1F, prgOR);
	syncCHR(0xFF, chrOR);
	syncNT (0xFF, chrOR);	
}
static void Mapper394_PWrap(uint32 A, uint8 V)
{
	int prgAND =HSK007Reg[3] &0x10? 0x1F: 0x0F;
	int prgOR  =HSK007Reg[3] <<1 &0x010 | HSK007Reg[1] <<5 &0x020;
	if (HSK007Reg[1] &0x08)
		setprg8(A, V &prgAND | prgOR &~prgAND);
	else
	if (A ==0x8000)
		setprg32(A, (prgOR | HSK007Reg[3] <<1 &0x0F) >>2);
	
}
static void Mapper394_CWrap(uint32 A, uint8 V)
{
	int chrAND =HSK007Reg[3] &0x80? 0xFF: 0x7F;
	int chrOR  =HSK007Reg[3] <<1 &0x080 | HSK007Reg[1] <<8 &0x100;
	setchr1(A, V &chrAND | chrOR &~chrAND);
}
static DECLFW(Mapper394_Write)
{
	uint8 oldMode =HSK007Reg[1];
	A &=3;
	HSK007Reg[A] =V;
	if (A ==1)
	{		
		if (~oldMode &0x10 &&  V &0x10) JYASIC_power();
		if ( oldMode &0x10 && ~V &0x10)
		{
			JYASIC_restoreWriteHandlers();
			GenMMC3Power();
		}
	}
	else
	{
		if (HSK007Reg[1] &0x10)
			sync();
		else
		{
			FixMMC3PRG(MMC3_cmd);
			FixMMC3CHR(MMC3_cmd);
		}
			
	}
}
static void Mapper394_restore (int version)
{
#ifndef TARGET_GNW
	int i;
#endif
	JYASIC_restoreWriteHandlers();
	if (HSK007Reg[1] &0x10)
	{		
		SetWriteHandler(0x5000, 0x5FFF, writeALU);
		SetWriteHandler(0x6000, 0x7fff, CartBW);
		SetWriteHandler(0x8000, 0x87FF, writePRG);     /* 8800-8FFF ignored */
		SetWriteHandler(0x9000, 0x97FF, writeCHRLow);  /* 9800-9FFF ignored */
		SetWriteHandler(0xA000, 0xA7FF, writeCHRHigh); /* A800-AFFF ignored */
		SetWriteHandler(0xB000, 0xB7FF, writeNT);      /* B800-BFFF ignored */
		SetWriteHandler(0xC000, 0xCFFF, writeIRQ);
		SetWriteHandler(0xD000, 0xD7FF, writeMode);    /* D800-DFFF ignored */
		
#ifndef TARGET_GNW
		for (i =0; i <0x10000; i++) cpuWriteHandlers[i] =GetWriteHandler(i);
		SetWriteHandler(0x0000, 0xFFFF, trapCPUWrite); /* Trap all CPU writes for IRQ clocking purposes */
		cpuWriteHandlersSet =1;
#endif
		
		SetReadHandler(0x5000, 0x5FFF, readALU_DIP);
		SetReadHandler(0x6000, 0xFFFF, CartBR);
		sync();
	}
	else
	{
		SetWriteHandler(0x8000, 0xBFFF, MMC3_CMDWrite);
		SetWriteHandler(0xC000, 0xFFFF, MMC3_IRQWrite);
		SetReadHandler(0x8000, 0xFFFF, CartBR);
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
	}
}
static void Mapper394_power(void)
{
	HSK007Reg[0] =0x00;
	HSK007Reg[1] =0x0F;
	HSK007Reg[2] =0x00;
	HSK007Reg[3] =0x10;
	GenMMC3Power();
	SetWriteHandler(0x5000, 0x5FFF, Mapper394_Write);
}

void Mapper394_Init(CartInfo *info)
{
	allowExtendedMirroring =1;
	sync =sync394;
	JYASIC_init(info);
	GenMMC3_Init(info, 128, 128, 0, 0);
	pwrap =Mapper394_PWrap;
	cwrap =Mapper394_CWrap;
	info->Reset = Mapper394_power;
	info->Power = Mapper394_power;
	AddExState(HSK007Reg, 4, 0, "HSK ");
	GameStateRestore = Mapper394_restore;
}
#endif
