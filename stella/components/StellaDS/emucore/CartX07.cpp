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

#include <cassert>
#include <iostream>
#include "CartX07.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "M6532.hxx"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeX07::CartridgeX07(const uInt8* image)
{
  // Just reuse the existing cart buffer
  myImage = (uInt8 *)image;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeX07::~CartridgeX07()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeX07::name() const
{
  return "X07";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeX07::reset()
{
  // Upon reset we switch to bank 0
  myCurrentBank = 99;
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeX07::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();

  // Set the page accessing methods for the hot spots
  // The hotspots use addresses below 0x1000, so we simply grab them
  // all and forward the TIA/RIOT calls from the peek and poke methods.
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
  for(uInt32 j = 0x0000; j < 0x1000; j += (1 << shift))
  {
    if (j == 0x80) continue;  
    if (j == 0x180) continue; 
    mySystem->setPageAccess(j >> shift, page_access);
  }

  myCurrentBank = 99;
  // Install pages for bank 0
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeX07::peek(uInt16 address)
{
  uInt8 value = 0;

  // Check for RAM or TIA mirroring
  uInt16 lowAddress = address & 0x3ff;
  if(lowAddress & 0x80)
  {
    value = theM6532.peek(address);      
  }
  else if(!(lowAddress & 0x200))
  {
    value = theTIA.peek(address);
  }

  // Switch banks if necessary
  if((address & 0x180f) == 0x080d)
    bank((address & 0xf0) >> 4);
  else if((address & 0x1880) == 0)
  {
    if((myCurrentBank & 0xe) == 0xe)
      bank(((address & 0x40) >> 6) | 0xe);
  }

  return value;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeX07::poke(uInt16 address, uInt8 value)
{
  // Check for RAM or TIA mirroring
  uInt16 lowAddress = address & 0x3ff;
  if(lowAddress & 0x80)
  {
    theM6532.poke(address, value);
  }
  else if(!(lowAddress & 0x200))
  {
    theTIA.poke(address, value);
  }

  // Switch banks if necessary
  if((address & 0x180f) == 0x080d)
    bank((address & 0xf0) >> 4);
  else if((address & 0x1880) == 0)
  {
    if((myCurrentBank & 0xe) == 0xe)
      bank(((address & 0x40) >> 6) | 0xe);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeX07::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  if (myCurrentBank != (bank & 0x0f))
  {
      myCurrentBank = (bank & 0x0f);
      uInt32 offset = myCurrentBank * 4096;
      uInt16 shift = mySystem->pageShift();

      // Setup the page access methods for the current bank
      page_access.device = this;
      page_access.directPokeBase = 0;

      // Map ROM image into the system
      for(uInt32 address = 0x1000; address < 0x2000; address += (1 << shift))
      {
        page_access.directPeekBase = &myImage[offset + (address & 0x0FFF)];
        mySystem->setPageAccess(address >> shift, page_access);
      }
  }
}
