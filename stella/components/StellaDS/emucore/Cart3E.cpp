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
#include <assert.h>
#include "Random.hxx"
#include "Cart3E.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge3E::Cartridge3E(const uInt8* image, uInt32 size)
  : mySize(size)
{
  // Copy the ROM image into my buffer - just reuse the existing buffer
  myImage = (uInt8 *)image;

  // Initialize RAM with random values
  class Random random;
  for(uInt32 i = 0; i < 32768; ++i)
  {
    xl_ram_buffer[i] = random.next();
  }
  
  // Force saving of the extra 32K of RAM
  bSaveStateXL = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge3E::~Cartridge3E()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge3E::name() const
{
  return "3E";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3E::reset()
{
  // We'll map bank 0 into the first segment upon reset
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3E::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1800 & mask) == 0);

  // Set the page accessing methods for the hot spots (for 100% emulation
  // we need to chain any accesses below 0x40 to the TIA. Our poke() method
  // does this via mySystem->tiaPoke(...), at least until we come up with a
  // cleaner way to do it).
  for(uInt32 i = 0x00; i < 0x80; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Setup the second segment to always point to the last ROM slice
  for(uInt32 j = 0x1800; j < 0x2000; j += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = &myImage[(mySize - 2048) + (j & 0x07FF)];
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(j >> shift, page_access);
  }

  // Install pages for bank 0 into the first segment
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge3E::peek(uInt16 address)
{
    // We can only get here if we are address < 0x80 in zpg
    return theTIA.peek(address);   
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3E::poke(uInt16 address, uInt8 value)
{
    // We can only get here if we are address < 0x80 in zpg
  if(address & 0x0FC0)
  {
      theTIA.poke(address, value);    // Pass through to "real" TIA
  }
  else // We are 3F or lower...
  {
      if (address == 0x3F) bank(value);
      else if(address == 0x3E) bank(value + 256);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3E::bank(uInt16 bank)
{ 
  if(bank < 256)
  {
    // Make sure the bank they're asking for is reasonable
    if((uInt32)bank * 2048 < mySize)
    {
      myCurrentBank = bank;
    }
    else
    {
      // Oops, the bank they're asking for isn't valid so let's wrap it
      // around to a valid bank number
      myCurrentBank = bank % (mySize / 2048);
    }
  
    uInt32 offset = myCurrentBank * 2048;
    uInt16 shift = mySystem->pageShift();
  
    // Setup the page access methods for the current bank
    page_access.device = this;
    page_access.directPokeBase = 0;
  
    // Map ROM image into the system
    for(uInt32 address = 0x1000; address < 0x1800; address += (1 << shift))
    {
        page_access.directPeekBase = &myImage[offset + (address & 0x07FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }
  }
  else
  {
    bank -= 256;
    bank %= 32;
    myCurrentBank = bank + 256;

    uInt32 offset = bank * 1024;
    uInt16 shift = mySystem->pageShift();
    uInt32 address;
  
    // Setup the page access methods for the current bank
    page_access.device = this;
    page_access.directPokeBase = 0;
  
    // Map read-port RAM image into the system
    for(address = 0x1000; address < 0x1400; address += (1 << shift))
    {
        page_access.directPeekBase = &xl_ram_buffer[offset + (address & 0x03FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }

   page_access.directPeekBase = 0;

    // Map write-port RAM image into the system
    for(address = 0x1400; address < 0x1800; address += (1 << shift))
    {
        page_access.directPokeBase = &xl_ram_buffer[offset + (address & 0x03FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }
  }
}
