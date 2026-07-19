//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2024 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// This file has been modified by Dave Bernazzani (wavemotion-dave)
// for optimized execution on the DS/DSi platform. Please seek the
// official Stella source distribution which is far cleaner, newer,
// and better maintained.
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================
#include <nds.h>

#include "M6502.hxx"
#include "Random.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
M6502::M6502(uInt32 systemCyclesPerProcessorCycle)
    : mySystem(0)
{
  uInt16 t;

  // Compute the BCD lookup table
  for(t = 0; t < 256; ++t)
  {
    ourBCDTable[0][t] = ((t >> 4) * 10) + (t & 0x0f);
    ourBCDTable[1][t] = (((t % 100) / 10) << 4) | (t % 10);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
M6502::~M6502()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void M6502::install(System& system)
{
  // Remember which system I'm installed in
  mySystem = &system;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void M6502::reset()
{
  Random random;

  // Set registers to default values
  A = random.next() & 0xFF;
  X = random.next() & 0xFF;
  Y = random.next() & 0xFF;
    
  SP = 0xff;
  PS(0x20);

  // Load PC from the reset vector
  gPC = (uInt16)mySystem->peek(0xfffc) | ((uInt16)mySystem->peek(0xfffd) << 8);
  gPC &= MY_ADDR_MASK;
    
  // Set the data bus back to a known value
  myDataBusState = 0x02;    
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void M6502::stop()
{
  *stack_executionStatus |= StopExecutionBit;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 uInt8 M6502::PS() const
{
  uInt8 ps = 0x20;

  if(N & 0x80)  ps |= 0x80;
  if(V)         ps |= 0x40;
  if(B)         ps |= 0x10;
  if(D)         ps |= 0x08;
  if(I)         ps |= 0x04;
  if(!notZ)     ps |= 0x02;
  if(C)         ps |= 0x01;

  return ps;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void M6502::PS(uInt8 ps)
{
  N = ps & 0x80;
  V = ps & 0x40;
  B = ps & 0x10;
  D = ps & 0x08;
  I = ps & 0x04;
  notZ = !(ps & 0x02);
  C = ps & 0x01;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 M6502::ourBCDTable[2][256];

