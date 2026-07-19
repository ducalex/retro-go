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
#include <iostream>
#include "CartEFSC.hxx"
#include "Random.hxx"
#include "System.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeEFSC::CartridgeEFSC(const uInt8* image)
{
  // Cart is already in cart_buffer[]
    
  // Initialize RAM with random values
  Random random;
  for(uInt32 i = 0; i < 128; ++i)
  {
    myRAM[128 + i] = (myCartInfo.clearRAM ? 0x00:random.next());
  }    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeEFSC::~CartridgeEFSC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeEFSC::name() const
{
  return "EFSC";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeEFSC::reset()
{
  // Upon reset we switch to bank 1
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeEFSC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
  
  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FE0 & ~mask); i < 0x2000; i += (1 << shift))
  {
    mySystem->setPageAccess(i >> shift, page_access);
  }
    
  // Set the page accessing method for the RAM writing pages
  for(uInt32 j = 0x1000; j < 0x1080; j += (1 << shift))
  {
    page_access.directPokeBase = &myRAM[128 + (j & 0x007F)];
    page_access.directPeekBase = 0;
    mySystem->setPageAccess(j >> shift, page_access);
  }

  // Set the page accessing method for the RAM reading pages
  for(uInt32 k = 0x1080; k < 0x1100; k += (1 << shift))
  {
    page_access.directPeekBase = &myRAM[128 + (k & 0x007F)];
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(k >> shift, page_access);
  }
    
  // Leave these at zero for faster bank switch
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;

  // Install pages for bank 1
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeEFSC::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FEF))
  {
    bank(address - 0x0FE0);
  }

  // NOTE: This does not handle accessing RAM, however, this function 
  // should never be called for RAM because of the way page accessing 
  // has been setup
  return cart_buffer[myCurrentOffset32 + address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeEFSC::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FEF))
  {
    bank(address - 0x0FE0);
  }
    
  // NOTE: This does not handle accessing RAM, however, this function 
  // should never be called for RAM because of the way page accessing 
  // has been setup
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeEFSC::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentOffset32 = bank * 4096;

  // Setup the page access methods for the current bank
  uInt32 access_num = 0x1100 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x0100; address < (0x0FE0U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
  {
      myPageAccessTable[access_num++].directPeekBase = &cart_buffer[myCurrentOffset32 + address];
  }
}
