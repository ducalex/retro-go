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
#include "CartFASC.hxx"
#include "Random.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeFASC::CartridgeFASC(const uInt8* image)
{
  // Reuse the existing cart buffer...
  myImage = (uInt8 *)image;

  // Initialize RAM with random values
  Random random;
  for(uInt32 i = 0; i < 256; ++i)
  {
    fast_cart_buffer[i] = (myCartInfo.clearRAM ? 0x00:random.next());
  }
}
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeFASC::~CartridgeFASC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeFASC::name() const
{
  return "FASC";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeFASC::reset()
{
  // Upon reset we switch to bank 2
  bank(2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeFASC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1100 & mask) == 0) && ((0x1200 & mask) == 0));

  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FF8 & ~mask); i < 0x2000; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Set the page accessing method for the RAM writing pages
  for(uInt32 j = 0x1000; j < 0x1100; j += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = 0;
    page_access.directPokeBase = &fast_cart_buffer[j & 0x00FF];
    mySystem->setPageAccess(j >> shift, page_access);
  }
 
  // Set the page accessing method for the RAM reading pages
  for(uInt32 k = 0x1100; k < 0x1200; k += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = &fast_cart_buffer[k & 0x00FF];
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(k >> shift, page_access);
  }

  // Install pages for bank 2
  bank(2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeFASC::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF8:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x0FF9:
      // Set the current bank to the middle 4k bank
      bank(1);
      break;

    case 0x0FFA:
      // Set the current bank to the upper 4k bank
      bank(2);
      break;

    default:
      break;
  }

  // NOTE: This does not handle accessing RAM, however, this function
  // should never be called for RAM because of the way page accessing
  // has been setup
  return myImage[myCurrentBank * 4096 + address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeFASC::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF8:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x0FF9:
      // Set the current bank to the middle 4k bank
      bank(1);
      break;

    case 0x0FFA:
      // Set the current bank to the upper 4k bank
      bank(2);
      break;

    default:
      break;
  }

  // NOTE: This does not handle accessing RAM, however, this function
  // should never be called for RAM because of the way page accessing
  // has been setup
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeFASC::bank(uInt16 bank)
{
  // Remember what bank we're in
  myCurrentBank = bank;
  uInt16 offset = myCurrentBank * 4096;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Setup the page access methods for the current bank
  page_access.device = this;
  page_access.directPokeBase = 0;

  // Map ROM image into the system
  for(uInt32 address = 0x1200; address < (0x1FF8U & ~mask);
      address += (1 << shift))
  {
    page_access.directPeekBase = &myImage[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, page_access);
  }
}
