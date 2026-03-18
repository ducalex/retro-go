#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) ||                                                    \
    (defined(TARGET_GNW) && defined(ENABLE_EMULATOR_NES) &&                    \
     FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
 *
 */

#include "mapinc.h"

static uint8 reg[2];
static uint8 latch;
static uint8 pad;

static SFORMAT StateRegs[] = {
    {reg, 2, "REG"}, {&latch, 1, "LATC"}, {&pad, 1, "PAD"}, {0}};

static void M319Sync(void) {
  if (reg[1] & 0x40)
    setprg32(0x8000, reg[1] >> 3 & 3);
  else {
    setprg16(0x8000, (reg[1] >> 2 & 6) | (reg[1] >> 5 & 1));
    setprg16(0xC000, (reg[1] >> 2 & 6) | (reg[1] >> 5 & 1));
  }
  setchr8((reg[0] >> 4 & ~(reg[0] << 2 & 4)) |
          (latch << 2 & (reg[0] << 2 & 4)));
  setmirror(reg[1] >> 7);
}

static void StateRestore(int version) { M319Sync(); }

static DECLFR(M319ReadPad) { return pad; }

static DECLFW(M319WriteReg) {
  reg[A >> 2 & 1] = V;
  M319Sync();
}

static DECLFW(M319WriteLatch) {
  latch = V;
  M319Sync();
}

static void M319Reset(void) {
  reg[0] = reg[1] = latch = 0;
  pad ^= 0x40;
  M319Sync();
}

static void M319Power(void) {
  reg[0] = reg[1] = latch = pad = 0;
  M319Sync();
  SetReadHandler(0x5000, 0x5FFF, M319ReadPad);
  SetReadHandler(0x6000, 0xFFFF, CartBR);
  SetWriteHandler(0x6000, 0x7FFF, M319WriteReg);
  SetWriteHandler(0x8000, 0xFFFF, M319WriteLatch);
}

void Mapper319_Init(CartInfo *info) {
  info->Power = M319Power;
  info->Reset = M319Reset;
  GameStateRestore = StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
#endif
