#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCEUmm - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
 *  Copyright (C) 2020
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

/*	Mappers:
	176 - Standard
	523 - Jncota KT-xxx, re-release of 封神榜꞉ 伏魔三太子: 1 KiB->2 KiB, 2 KiB->4 KiB CHR, hard-wired nametable mirroring)

	Submappers:	
	0 - Standard
	1 - FK-xxx
	2 - 外星 FS005/FS006
	3 - JX9003B
	4 - GameStar Smart Genius Deluxe
	5 - HST-162
	
	Verified on real hardware:
	"Legend of Kage" sets CNROM latch 1 and switches between CHR bank 0 and 1 using 5FF2, causing the wrong bank (1 instead of 0) during gameplay.
	
	Heuristic for detecting whether the DIP switch should be changed on every soft reset:
	The first write to the $5xxx range is to $501x           => ROM always addresses $501x; changing the DIP switch on reset would break the emulation after reset, so don't do it.
	The first write to the $5xxx range is to $5020 or higher => ROM either uses a DIP switch or writes to $5FFx for safety; changing the DIP switch on reset is possible.
	Exclude the $5FF3 address as well as $5000-$500F from this heuristic.
*/

#include "mapinc.h"
#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

static uint8 *WRAM            = NULL;
static uint8 *CHRRAM          = NULL;
static uint32 WRAMSIZE        = 0;
static uint32 CHRRAMSIZE      = 0;

static uint8 fk23_regs[8]     = { 0 }; /* JX9003B has eight registers, all others have four */
static uint8 mmc3_regs[12]    = { 0 };
static uint8 mmc3_ctrl        = 0;
static uint8 mmc3_mirr        = 0;
static uint8 mmc3_wram        = 0;
static uint8 irq_count        = 0;
static uint8 irq_latch        = 0;
static uint8 irq_enabled      = 0;
static uint8 irq_reload       = 0;
static uint8 latch            = 0;
static uint8 dipswitch        = 0;
static uint8 subType          = 0; /* NES 2.0 Submapper, denoting PCB variants */
static uint8 jncota523        = 0; /* Jncota board with unusual wiring that turns 1 KiB CHR banks into 2 KiB banks, and has hard-wired nametable mirroring. */
static uint8 dipsw_enable     = 0; /* Change the address mask on every reset? */
static uint8 after_power      = 0; /* Used for detecting whether a DIP switch is used or not (see above) */

extern uint32 ROM_size;
extern uint32 VROM_size;

static SFORMAT StateRegs[] = {
   { fk23_regs,               8, "EXPR" },
   { mmc3_regs,              12, "M3RG" },
   { &latch,                  1, "LATC" },
   { &dipswitch,              1, "DPSW" },
   { &mmc3_ctrl,              1, "M3CT" },
   { &mmc3_mirr,              1, "M3MR" },
   { &mmc3_wram,              1, "M3WR" },
   { &irq_reload,             1, "IRQR" },
   { &irq_count,              1, "IRQC" },
   { &irq_latch,              1, "IRQL" },
   { &irq_enabled,            1, "IRQA" },
   { &subType,                1, "SUBT" },
   { 0 }
};

#define INVERT_PRG          !!(mmc3_ctrl & 0x40)
#define INVERT_CHR          !!(mmc3_ctrl & 0x80)
#define WRAM_ENABLED        !!(mmc3_wram & 0x80)
#define WRAM_EXTENDED      (!!(mmc3_wram & 0x20) && subType == 2)    /* Extended A001 register. Only available on FS005 PCB. */
#define FK23_ENABLED       (!!(mmc3_wram & 0x40) || !WRAM_EXTENDED)  /* Enable or disable registers in the $5xxx range. Only available on FS005 PCB. */
#define PRG_MODE              ( fk23_regs[0] & 0x07)
#define MMC3_EXTENDED       !!( fk23_regs[3] & 0x02)                 /* Extended MMC3 mode, adding extra registers for switching the normally-fixed PRG banks C and E and for eight independent 1 KiB CHR banks. Only available on FK- and FS005 PCBs. */
#define CHR_8K_MODE         !!( fk23_regs[0] & 0x40)                 /* MMC3 CHR registers are ignored, apply outer bank only, and CNROM latch if it exists */
#define CHR_CNROM_MODE        (~fk23_regs[0] & 0x20 && (subType == 1 || subType == 5)) /* Only subtypes 1 and 5 have a CNROM latch, which can be disabled */
#define CHR_OUTER_BANK_SIZE !!( fk23_regs[0] & 0x10)                 /* Switch between 256 and 128 KiB CHR, or 32 and 16 KiB CHR in CNROM mode */
#define CHR_MIXED           !!(WRAM_EXTENDED && mmc3_wram &0x04)     /* First 8 KiB of CHR address space are RAM, then ROM */

static void cwrap(uint32 A, uint32 V)
{
   int bank = 0;

   if (jncota523)
   {
      if (~A &0x0400) setchr2r(bank, A, V);
   }
   else
   {
      /* some workaround for chr rom / ram access */
      if (!VROM_size)
         bank = 0;
      else if (CHRRAMSIZE && fk23_regs[0] & 0x20)
         bank = 0x10;
      
      if (CHR_MIXED && V < 8) bank = 0x10; /* first 8K of chr bank is RAM */
      
      setchr1r(bank, A, V);
   }
}

static void SyncCHR(void)
{
   uint32 outer = fk23_regs[2] | (subType == 3? (fk23_regs[6] << 8): 0);    /* Outer 8 KiB CHR bank. Subtype 3 has an MSB register providing more bits. */
   if (CHR_8K_MODE)
   {
      uint32 mask = (CHR_CNROM_MODE? (CHR_OUTER_BANK_SIZE? 0x01: 0x03): 0x00);
      /* In Submapper 1, address bits come either from outer bank or from latch. In Submapper 5, they are OR'd. Both verified on original hardware. */
      uint32 bank = ((subType ==5? outer: (outer & ~mask)) | (latch & mask)) << 3;

      cwrap(0x0000, bank + 0);
      cwrap(0x0400, bank + 1);
      cwrap(0x0800, bank + 2);
      cwrap(0x0C00, bank + 3);

      cwrap(0x1000, bank + 4);
      cwrap(0x1400, bank + 5);
      cwrap(0x1800, bank + 6);
      cwrap(0x1C00, bank + 7);
   }
   else
   {
      uint32 cbase = (INVERT_CHR? 0x1000: 0);
      uint32 mask  = (CHR_OUTER_BANK_SIZE? 0x7F: 0xFF);
             outer = (outer << 3) & ~mask; /* From 8 KiB to 1 KiB banks. Address bits are never OR'd; they either come from the outer bank or from the MMC3. */

      if (MMC3_EXTENDED)
      {
         cwrap(cbase ^ 0x0000, mmc3_regs[0]  &mask | outer);
         cwrap(cbase ^ 0x0400, mmc3_regs[10] &mask | outer);
         cwrap(cbase ^ 0x0800, mmc3_regs[1]  &mask | outer);
         cwrap(cbase ^ 0x0c00, mmc3_regs[11] &mask | outer);

         cwrap(cbase ^ 0x1000, mmc3_regs[2]  &mask | outer);
         cwrap(cbase ^ 0x1400, mmc3_regs[3]  &mask | outer);
         cwrap(cbase ^ 0x1800, mmc3_regs[4]  &mask | outer);
         cwrap(cbase ^ 0x1c00, mmc3_regs[5]  &mask | outer);
      }
      else
      {
         cwrap(cbase ^ 0x0000,(mmc3_regs[0] & 0xFE) & mask | outer);
         cwrap(cbase ^ 0x0400,(mmc3_regs[0] | 0x01) & mask | outer);
         cwrap(cbase ^ 0x0800,(mmc3_regs[1] & 0xFE) & mask | outer);
         cwrap(cbase ^ 0x0C00,(mmc3_regs[1] | 0x01) & mask | outer);

         cwrap(cbase ^ 0x1000, mmc3_regs[2]         & mask | outer);
         cwrap(cbase ^ 0x1400, mmc3_regs[3]         & mask | outer);
         cwrap(cbase ^ 0x1800, mmc3_regs[4]         & mask | outer);
         cwrap(cbase ^ 0x1c00, mmc3_regs[5]         & mask | outer);
      }
   }
}

static void SyncPRG(void)
{
   uint32 mask = 0x3F >> PRG_MODE;        /* For PRG modes 0-2, the mode# decides how many bits of the inner 8 KiB bank are used. This is greatly relevant to map the correct bank that contains the reset vectors. */
   uint32 prg_base = fk23_regs[1] & 0x7F; /* The bits for the first 2 MiB are the same between all the variants. */
   switch (subType)
   {
      case 1: /* FK-xxx */
         if (PRG_MODE == 0) mask = 0xFF;  /* Mode 0 allows the MMC3 to address 2 MiB rather than the usual 512 KiB. */
	 break;
      case 2: /* FS005 */
         prg_base |= fk23_regs[0] << 4 & 0x080 | fk23_regs[0] << 1 & 0x100 | fk23_regs[2] << 3 & 0x600 | fk23_regs[2] << 6 & 0x800;   
	 break;
      case 3: /* JX9003B */
         if (PRG_MODE == 0) mask = 0xFF;  /* Mode 0 allows the MMC3 to address 2 MiB rather than the usual 512 KiB. */
         prg_base |= fk23_regs[5] << 7;
	 break;
      case 4: /* GameStar Smart Genius Deluxe */
         prg_base |= fk23_regs[2] & 0x80;
	 break;
      case 5: /* HST-162 */
         prg_base = prg_base &0x1F | fk23_regs[5] <<5;
	 break;
   }

   switch (PRG_MODE)
   {
      case 0: /* MMC3 with 512 KiB or 2 MiB addressable */
      case 1: /* MMC3 with 256 KiB addressable */
      case 2: /* MMC3 with 128 KiB addressable */
      {
         uint32 cbase = (INVERT_PRG ? 0x4000 : 0);
      
         prg_base =(prg_base << 1) & ~mask; /* from 16 to 8 KiB. Address bits are never OR'd; they either come from the outer bank or from the MMC3.  */
      
         if (MMC3_EXTENDED)
         {
            setprg8(0x8000 ^ cbase, mmc3_regs[6] & mask | prg_base);
            setprg8(0xA000,         mmc3_regs[7] & mask | prg_base);
            setprg8(0xC000 ^ cbase, mmc3_regs[8] & mask | prg_base);
            setprg8(0xE000,         mmc3_regs[9] & mask | prg_base);
         }
         else
         {
            setprg8(0x8000 ^ cbase, (mmc3_regs[6] & mask) | prg_base);
            setprg8(0xA000,         (mmc3_regs[7] & mask) | prg_base);
            setprg8(0xC000 ^ cbase, (0xFE         & mask) | prg_base);
            setprg8(0xE000,         (0xFF         & mask) | prg_base);
         }
         break;
      }
      case 3: /* NROM-128 */
         setprg16(0x8000, prg_base);
         setprg16(0xC000, prg_base);
         break;
      case 4: /* NROM-256 */
         setprg32(0x8000, (prg_base >> 1));
         break;
      case 5: /* UNROM */
         setprg16(0x8000, latch & 0x07 | prg_base &~0x07);
	 setprg16(0xC000,         0x07 | prg_base       );
	 break;
   }
}

static void SyncWRAM(void)
{
   /* TODO: WRAM Protected  mode when not in extended mode */
   if (WRAM_ENABLED || WRAM_EXTENDED)
   {
      if (WRAM_EXTENDED)
      {
         setprg8r(0x10, 0x4000, (mmc3_wram & 0x03) + 1);
         setprg8r(0x10, 0x6000, mmc3_wram & 0x03);
      }
      else
         setprg8r(0x10, 0x6000, 0);
   }
}

static void SyncMIR(void)
{
   if (jncota523) /* Jncota board has hard-wired mirroring */
      return;
   else
   switch (mmc3_mirr & (subType == 2? 0x03 : 0x01))
   {
      case 0: setmirror(MI_V); break;
      case 1: setmirror(MI_H); break;
      case 2: setmirror(MI_0); break;
      case 3: setmirror(MI_1); break;
   }
}

static void Sync(void)
{
   SyncPRG();
   SyncCHR();
   SyncWRAM();
   SyncMIR();
}

static DECLFW(Write4800) /* Only used by submapper 5 (HST-162) */
{
   fk23_regs[5] = V; /* Register 4800 is a separate register, but we use one of the ASIC registers that is otherwise unused in submapper 5 */
   SyncPRG();
}
static DECLFW(Write5000)
{
   if (after_power && A > 0x5010 && A != 0x5FF3) /* Ignore writes from $5000-$500F, in particular to $5008, but not $5FF3 */
   { 
      after_power = 0;
      dipsw_enable = A >= 0x5020;   /* The DIP switch change on soft-reset is enabled if the first write after power-on is not to $501x */
   }
   if (FK23_ENABLED && (A & (0x10 << dipswitch)))
   {
      fk23_regs[A & (subType == 3? 7: 3)] = V;
      SyncPRG();
      SyncCHR();
   }
   else
      /* FK23C Registers disabled, $5000-$5FFF maps to the second 4 KiB of the 8 KiB WRAM bank 2 */
      CartBW(A, V);
}

static DECLFW(Write8000)
{
   latch = V;
   if (CHR_8K_MODE && CHR_CNROM_MODE) SyncCHR(); /* CNROM latch updated */
   if (PRG_MODE == 5) SyncPRG(); /* UNROM latch has been updated */
   
   switch (A & 0xE001)
   {
      case 0x8000:
      {
         uint8 old_ctrl;
         if (A & 2) return; /* Confirmed on real hardware: writes to 8002 and 8003, or 9FFE and 9FFF, are ignored. Needed for Dr. Mario on some of the "bouncing ball" multis. */
         old_ctrl = mmc3_ctrl;
      
         /* Subtype 2, 8192 or more KiB PRG-ROM, no CHR-ROM: Like Subtype 0,
          * but MMC3 registers $46 and $47 swapped. */
         if (subType == 2)
         {
            if (V == 0x46)
               V = 0x47;
            else if (V == 0x47)
               V = 0x46;
         }
      
         mmc3_ctrl = V;
      
         if (INVERT_PRG != (old_ctrl & 0x40))
            SyncPRG();
      
         if (INVERT_CHR != (old_ctrl & 0x80))
            SyncCHR();
      
         break;
      }
      case 0x8001:
      {
         uint8 ctrl_mask;
         if (A & 2) return; /* Confirmed on real hardware: writes to 8002 and 8003, or 9FFE and 9FFF, are ignored. Needed for Dr. Mario on some of the "bouncing ball" multis. */
         ctrl_mask = MMC3_EXTENDED ? 0x0F : 0x07;
      
         if ((mmc3_ctrl & ctrl_mask) < 12)
         {
            mmc3_regs[mmc3_ctrl & ctrl_mask] = V;
      
            if (((mmc3_ctrl & ctrl_mask) < 6) || ((mmc3_ctrl & ctrl_mask) >= 10))
               SyncCHR();
            else
               SyncPRG();
         }
         break;
      }
      case 0xA000:
         mmc3_mirr = V;
         SyncMIR();
         break;
      case 0xA001:
         /* ignore bits when ram config register is disabled */
         if ((V & 0x20) == 0)
            V &= 0xC0;
         mmc3_wram = V;
         Sync();
         break;
      case 0xC000:
         irq_latch = V;
         break;
      case 0xC001:
         irq_reload = 1;
         break;
      case 0xE000:
         X6502_IRQEnd(FCEU_IQEXT);
         irq_enabled = 0;
         break;
      case 0xE001:
         irq_enabled = 1;
         break;
      default:
         break;
   }
}

static void IRQHook(void)
{
   if (!irq_count || irq_reload)
      irq_count = irq_latch;
   else
      irq_count--;

   if (!irq_count && irq_enabled)
      X6502_IRQBegin(FCEU_IQEXT);

   irq_reload = 0;
}

static void Reset(void)
{
   /* this little hack makes sure that we try all the dip switch settings eventually, if we reset enough */
   if (dipsw_enable) {
      dipswitch = (dipswitch + 1) & 7;
      FCEU_printf("BMCFK23C dipswitch set to $%04x\n",0x5000|0x10 << dipswitch);
   }

   fk23_regs[0]   = fk23_regs[1] = fk23_regs[2] = fk23_regs[3] = fk23_regs[4] = fk23_regs[5] = fk23_regs[6] = fk23_regs[7] = 0;
   mmc3_regs[0]   = 0;
   mmc3_regs[1]   = 2;
   mmc3_regs[2]   = 4;
   mmc3_regs[3]   = 5;
   mmc3_regs[4]   = 6;
   mmc3_regs[5]   = 7;
   mmc3_regs[6]   = 0;
   mmc3_regs[7]   = 1;
   mmc3_regs[8]   = ~1;
   mmc3_regs[9]   = ~0;
   mmc3_regs[10]  = ~0;
   mmc3_regs[11]  = ~0;
   mmc3_wram      = 0x80;
   mmc3_ctrl      = mmc3_mirr = irq_count = irq_latch = irq_enabled = 0;

   Sync();
}

static void Power(void)
{
   fk23_regs[0]   = fk23_regs[1] = fk23_regs[2] = fk23_regs[3] = fk23_regs[4] = fk23_regs[5] = fk23_regs[6] = fk23_regs[7] = 0;
   mmc3_regs[0]   = 0;
   mmc3_regs[1]   = 2;
   mmc3_regs[2]   = 4;
   mmc3_regs[3]   = 5;
   mmc3_regs[4]   = 6;
   mmc3_regs[5]   = 7;
   mmc3_regs[6]   = 0;
   mmc3_regs[7]   = 1;
   mmc3_regs[8]   = ~1;
   mmc3_regs[9]   = ~0;
   mmc3_regs[10]  = ~0;
   mmc3_regs[11]  = ~0;
   mmc3_wram      = 0x80;
   mmc3_ctrl      = mmc3_mirr = irq_count = irq_latch = irq_enabled = 0;

   Sync();

   SetReadHandler(0x8000, 0xFFFF, CartBR);
   SetWriteHandler(0x5000, 0x5FFF, Write5000);
   SetWriteHandler(0x8000, 0xFFFF, Write8000);

   if (subType == 5)
      SetWriteHandler(0x4800, 0x4FFF, Write4800);

   if (WRAMSIZE)
   {
      SetReadHandler(0x6000, 0x7FFF, CartBR);
      SetWriteHandler(0x6000, 0x7FFF, CartBW);
      FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
   }
}

static void Close(void)
{
   if (WRAM)
      FCEU_gfree(WRAM);
   WRAM = NULL;

   if (CHRRAM)
      FCEU_gfree(CHRRAM);
   CHRRAM = NULL;
}

static void StateRestore(int version)
{
   Sync();
}

void Init(CartInfo *info)
{
   /* Initialization for iNES and UNIF. subType and dipsw_enable must have been set. */
   info->Power       = Power;
   info->Reset       = Reset;
   info->Close       = Close;
   GameHBIRQHook     = IRQHook;
   GameStateRestore  = StateRestore;
   AddExState(StateRegs, ~0, 0, 0);

   if (CHRRAMSIZE)
   {
#ifndef FCEU_NO_MALLOC
      CHRRAM = (uint8 *)FCEU_gmalloc(CHRRAMSIZE);
#else
      CHRRAM = (uint8 *)ahb_calloc(1, CHRRAMSIZE);
#endif
      SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
      AddExState(CHRRAM, CHRRAMSIZE, 0, "CRAM");
   }

   if (WRAMSIZE)
   {
#ifndef FCEU_NO_MALLOC
		WRAM = (uint8 *)FCEU_gmalloc(WRAMSIZE);
#else
		WRAM = (uint8 *)ahb_calloc(1, WRAMSIZE);
#endif
      SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
      AddExState(WRAM, WRAMSIZE, 0, "WRAM");

      if (info->battery)
      {
         info->SaveGame[0] = WRAM;
         if (info->iNES2 && info->PRGRamSaveSize)
            info->SaveGameLen[0] = info->PRGRamSaveSize;
         else
            info->SaveGameLen[0] = WRAMSIZE;
      }
   }

}

void Mapper176_Init(CartInfo *info) { /* .NES file */
   dipsw_enable = 0;
   jncota523 = 0;
   if (info->iNES2)
   {
      subType = info->submapper;
      after_power = subType != 2;  /* FS005 never has DIP switches, the others may have one, so use the heuristic. */
      CHRRAMSIZE = info->CHRRamSize + info->CHRRamSaveSize;
      WRAMSIZE = info->PRGRamSize + info->PRGRamSaveSize;
   }
   else
   {
      /* Waixing boards have 32K battery backed wram */
      if (info->battery)
      {
         subType = 2;
         after_power = 0;
         WRAMSIZE = 32 * 1024;
      }
      else
      {
         /* Always enable WRAM for iNES-headered files */
         WRAMSIZE = 8 * 1024;
	 
	 /* Distinguishing subType 1 from subType 0 is important for the correct reset vector location.
	    It is safe to assume subType 1 except for the following-sized ROMs. */
         subType = (ROM_size ==128 && VROM_size ==256 ||  /* 2048+2048 */
                    ROM_size ==128 && VROM_size ==128 ||  /* 2048+1024 */
                    ROM_size ==128 && VROM_size ==64  ||  /* 2048+512 */
                    ROM_size ==128 && VROM_size ==0   ||  /* 2048+0 */
                    ROM_size ==64  && VROM_size ==64)?    /* 1024+512 */
                    0: 1;
		    
         /* Detect heuristically whether the address mask should be changed on every soft reset */
         after_power = 1;
      }
   }
   Init(info);
}

void BMCFK23C_Init(CartInfo *info)	/* UNIF FK23C. Also includes mislabelled WAIXING-FS005, recognizable by their PRG-ROM size. */
{
   if (!UNIFchrrama)
   {
      /* Rockman I-VI uses mixed chr rom/ram */
      if ((ROM_size * 16) == 2048 && (VROM_size * 8) == 512)
         CHRRAMSIZE = 8 * 1024;
   }
   WRAMSIZE = 8 * 1024;

   dipsw_enable = 0;
   after_power = 1;
   jncota523 = 0;
   subType =ROM_size *16 >=4096? 2: ROM_size == 64 && VROM_size == 128? 1: 0;
   if (subType == 2) 
      CHRRAMSIZE = 256 * 1024;
   
   Init(info);
}

void BMCFK23CA_Init(CartInfo *info)	/* UNIF FK23CA. Also includes mislabelled WAIXING-FS005, recognizable by their PRG-ROM size. */
{
   WRAMSIZE = 8 * 1024;

   dipsw_enable = 0;
   after_power = 1;
   jncota523 = 0;
   subType =ROM_size *16 >=2048? 2: 1;
   if (subType == 2) 
      CHRRAMSIZE = 256 * 1024;
   
   Init(info);
}

void Super24_Init(CartInfo *info)	/* UNIF BMC-Super24in1SC03 */
{
   CHRRAMSIZE = 8 * 1024;
   dipsw_enable = 0;
   after_power = 0;
   jncota523 = 0;
   subType = 0;
   Init(info);
}

void WAIXINGFS005_Init(CartInfo *info)	/* UNIF WAIXING-FS005 */
{
   CHRRAMSIZE = 8 * 1024;
   WRAMSIZE = 32 * 1024;
   dipsw_enable = 0;
   after_power = 0;
   jncota523 = 0;
   subType = 2;
   Init(info);
}

void Mapper523_Init(CartInfo *info)	/* Jncota Fengshengban */
{
   WRAMSIZE = 8 * 1024;
   dipsw_enable = 0;
   after_power = 0;
   jncota523 = 1;
   subType = 1;
   Init(info);
}
#endif
