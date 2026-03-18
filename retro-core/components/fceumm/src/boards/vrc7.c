#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) ||                                                    \
    (defined(TARGET_GNW) && defined(ENABLE_EMULATOR_NES) &&                    \
     FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
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

#include "fceu-emu2413.h"
#include "mapinc.h"

#ifdef FCEU_NO_MALLOC
#include "gw_malloc.h"
#endif

#ifndef TARGET_GNW
static int32 dwave = 0;
#endif
static OPLL *VRC7Sound = NULL;
static uint8 vrc7idx, preg[3], creg[8], mirr;
static uint8 IRQLatch, IRQa, IRQd;
static int32 IRQCount, CycleCount;
static uint8 *WRAM = NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[] = {{&vrc7idx, 1, "VRCI"},    {preg, 3, "PREG"},
                              {creg, 8, "CREG"},        {&mirr, 1, "MIRR"},
                              {&IRQa, 1, "IRQA"},       {&IRQd, 1, "IRQD"},
                              {&IRQLatch, 1, "IRQL"},   {&IRQCount, 4, "IRQC"},
                              {&CycleCount, 4, "CYCC"}, {0}};

/* VRC7 Sound */

#ifndef TARGET_GNW
void DoVRC7Sound(void) {
  int32 z, a;
  if (FSettings.soundq >= 1)
    return;
  z = ((SOUNDTS << 16) / soundtsinc) >> 4;
  a = z - dwave;
  OPLL_FCEU_fillbuf(VRC7Sound, &Wave[dwave], a, 1);
  dwave += a;
}

void UpdateOPLNEO(int32 *Wave, int Count) {
  OPLL_FCEU_fillbuf(VRC7Sound, Wave, Count, 4);
}
#endif

void UpdateOPL(int Count) {
#ifndef TARGET_GNW
  int32 z, a;
  z = ((SOUNDTS << 16) / soundtsinc) >> 4;
  a = z - dwave;
  if (VRC7Sound && a)
    OPLL_FCEU_fillbuf(VRC7Sound, &Wave[dwave], a, 1);
  dwave = 0;
#else
  OPLL_FCEU_fillbuf(VRC7Sound, Wave, (FSettings.SndRate / (PAL ? 50 : 60)), 1);
#endif
}

static void VRC7SC(void) {
#ifndef TARGET_GNW
  if (VRC7Sound)
    OPLL_FCEU_set_rate(VRC7Sound, FSettings.SndRate);
#else
  OPLL_FCEU_set_rate(VRC7Sound, FSettings.SndRate);
#endif
}

#ifndef TARGET_GNW
static void VRC7SKill(void) {
  if (VRC7Sound)
    OPLL_delete(VRC7Sound);
  VRC7Sound = NULL;
}
#endif

static void VRC7_ESI(void) {
  GameExpSound.RChange = VRC7SC;
#ifndef TARGET_GNW
  GameExpSound.Kill = VRC7SKill;
  VRC7Sound =
      OPLL_FCEU_new(3579545, FSettings.SndRate ? FSettings.SndRate : 44100);
#else
  VRC7Sound =
      OPLL_FCEU_new(3579545, FSettings.SndRate ? FSettings.SndRate : 18000);
#endif
  OPLL_FCEU_reset(VRC7Sound);

  GameExpSound.Fill = UpdateOPL;
#ifndef TARGET_GNW
  GameExpSound.NeoFill = UpdateOPLNEO;
#endif
}

/* VRC7 Sound */

static void Sync(void) {
  uint8 i;
  setprg8r(0x10, 0x6000, 0);
  setprg8(0x8000, preg[0]);
  setprg8(0xA000, preg[1]);
  setprg8(0xC000, preg[2]);
  setprg8(0xE000, ~0);
  for (i = 0; i < 8; i++)
    setchr1(i << 10, creg[i]);
  switch (mirr & 3) {
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

static DECLFW(VRC7SW) {
#ifndef TARGET_GNW
  if (FSettings.SndRate)
#endif
  {
#ifndef TARGET_GNW
    OPLL_FCEU_writeReg(VRC7Sound, vrc7idx, V);
#else
    OPLL_FCEU_writeReg(VRC7Sound, vrc7idx, V);
#endif
  }
}

static DECLFW(VRC7Write) {
  A |= (A & 8) << 1; /* another two-in-oooone */
  if (A >= 0xA000 && A <= 0xDFFF) {
    A &= 0xF010;
    creg[((A >> 4) & 1) | ((A - 0xA000) >> 11)] = V;
    Sync();
  } else if (A == 0x9030) {
    VRC7SW(A, V);
  } else
    switch (A & 0xF010) {
    case 0x8000:
      preg[0] = V;
      Sync();
      break;
    case 0x8010:
      preg[1] = V;
      Sync();
      break;
    case 0x9000:
      preg[2] = V;
      Sync();
      break;
    case 0x9010:
      vrc7idx = V;
      break;
    case 0xE000:
      mirr = V & 3;
      Sync();
      break;
    case 0xE010:
      IRQLatch = V;
      X6502_IRQEnd(FCEU_IQEXT);
      break;
    case 0xF000:
      IRQa = V & 2;
      IRQd = V & 1;
      if (V & 2)
        IRQCount = IRQLatch;
      CycleCount = 0;
      X6502_IRQEnd(FCEU_IQEXT);
      break;
    case 0xF010:
      IRQa = IRQd;
      X6502_IRQEnd(FCEU_IQEXT);
      break;
    }
}

static void VRC7Power(void) {
  Sync();
  SetWriteHandler(0x6000, 0x7FFF, CartBW);
  SetReadHandler(0x6000, 0xFFFF, CartBR);
  SetWriteHandler(0x8000, 0xFFFF, VRC7Write);
  FCEU_CheatAddRAM(WRAMSIZE >> 10, 0x6000, WRAM);
}

static void VRC7Close(void) {
  if (WRAM)
    FCEU_gfree(WRAM);
  WRAM = NULL;
}

static void VRC7IRQHook(int a) {
  if (IRQa) {
    CycleCount += a * 3;
    while (CycleCount >= 341) {
      CycleCount -= 341;
      IRQCount++;
      if (IRQCount == 0x100) {
        IRQCount = IRQLatch;
        X6502_IRQBegin(FCEU_IQEXT);
      }
    }
  }
}

static void StateRestore(int version) {
  Sync();

#ifndef GEKKO
#ifndef TARGET_GNW
  OPLL_FCEU_forceRefresh(VRC7Sound);
#else
  OPLL_FCEU_forceRefresh(VRC7Sound);
#endif
#endif
}

void Mapper85_Init(CartInfo *info) {
  info->Power = VRC7Power;
  info->Close = VRC7Close;
  MapIRQHook = VRC7IRQHook;
  WRAMSIZE = 8192;
#ifndef FCEU_NO_MALLOC
  WRAM = (uint8 *)FCEU_gmalloc(WRAMSIZE);
#else
  WRAM = (uint8 *)ahb_calloc(1, WRAMSIZE);
#endif
  SetupCartPRGMapping(0x10, WRAM, WRAMSIZE, 1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");
  if (info->battery) {
    info->SaveGame[0] = WRAM;
    info->SaveGameLen[0] = WRAMSIZE;
  }
  GameStateRestore = StateRestore;
  VRC7_ESI();
  AddExState(&StateRegs, ~0, 0, 0);

/* Ignoring these sound state files for Wii since it causes states unable to
 * load */
#ifndef GEKKO
  /* Sound states */
  AddExState(&VRC7Sound->adr, sizeof(VRC7Sound->adr), 0, "ADDR");
  AddExState(&VRC7Sound->out, sizeof(VRC7Sound->out), 0, "OUT0");
  AddExState(&VRC7Sound->realstep, sizeof(VRC7Sound->realstep), 0, "RTIM");
  AddExState(&VRC7Sound->oplltime, sizeof(VRC7Sound->oplltime), 0, "TIME");
  AddExState(&VRC7Sound->opllstep, sizeof(VRC7Sound->opllstep), 0, "STEP");
  AddExState(&VRC7Sound->prev, sizeof(VRC7Sound->prev), 0, "PREV");
  AddExState(&VRC7Sound->next, sizeof(VRC7Sound->next), 0, "NEXT");
  AddExState(&VRC7Sound->LowFreq, sizeof(VRC7Sound->LowFreq), 0, "LFQ0");
  AddExState(&VRC7Sound->HiFreq, sizeof(VRC7Sound->HiFreq), 0, "HFQ0");
  AddExState(&VRC7Sound->InstVol, sizeof(VRC7Sound->InstVol), 0, "VOLI");
  AddExState(&VRC7Sound->CustInst, sizeof(VRC7Sound->CustInst), 0, "CUSI");
  AddExState(&VRC7Sound->slot_on_flag, sizeof(VRC7Sound->slot_on_flag), 0,
             "FLAG");
  AddExState(&VRC7Sound->pm_phase, sizeof(VRC7Sound->pm_phase), 0, "PMPH");
  AddExState(&VRC7Sound->lfo_pm, sizeof(VRC7Sound->lfo_pm), 0, "PLFO");
  AddExState(&VRC7Sound->am_phase, sizeof(VRC7Sound->am_phase), 0, "AMPH");
  AddExState(&VRC7Sound->lfo_am, sizeof(VRC7Sound->lfo_am), 0, "ALFO");
  AddExState(&VRC7Sound->patch_number, sizeof(VRC7Sound->patch_number), 0,
             "PNUM");
  AddExState(&VRC7Sound->key_status, sizeof(VRC7Sound->key_status), 0, "KET");
  AddExState(&VRC7Sound->mask, sizeof(VRC7Sound->mask), 0, "MASK");
  AddExState((uint8 *)VRC7Sound->slot, sizeof(VRC7Sound->slot), 0, "SLOT");
#endif
}

void NSFVRC7_Init(void) {
  SetWriteHandler(0x9010, 0x901F, VRC7Write);
  SetWriteHandler(0x9030, 0x903F, VRC7Write);
  VRC7_ESI();
}
#endif
