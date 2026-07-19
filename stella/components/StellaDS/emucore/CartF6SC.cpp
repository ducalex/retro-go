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
#include <assert.h>
#include "CartF6SC.hxx"
#include "Random.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF6SC::CartridgeF6SC(const uInt8* image)
{
  // Just reuse the existing cart buffer
  myImage = (uInt8 *)image;

  // Initialize RAM with random values 
  Random random;
  for(uInt32 i = 0; i < 128; ++i)
  {
    myRAM[128+i] = (myCartInfo.clearRAM ? 0x00:random.next());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF6SC::~CartridgeF6SC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeF6SC::name() const
{
  return "F6SC";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6SC::reset()
{
  // Upon reset we switch to bank 0
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6SC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  for(uInt32 i = (0x1FF6 & ~mask); i < 0x2000; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Set the page accessing method for the RAM writing pages
  for(uInt32 j = 0x1000; j < 0x1080; j += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = 0;
    page_access.directPokeBase = &myRAM[128 + (j & 0x007F)];
    mySystem->setPageAccess(j >> shift, page_access);
  }

  // Set the page accessing method for the RAM reading pages
  for(uInt32 k = 0x1080; k < 0x1100; k += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = &myRAM[128 + (k & 0x007F)];
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(k >> shift, page_access);
  }
    
  // Leave these at zero for faster bank switch
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
    
  // And setup for this system without any direct peek/poke until we switch banks
  for(uInt32 address = 0x1100; address < 0x2000; address += (1 << MY_PAGE_SHIFT))    
  {
      mySystem->setPageAccess(address >> shift, page_access);
  }    

  // Install pages for bank 0
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 uInt8 CartridgeF6SC::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF6:
      // Set the current bank to the first 4k bank
      bank(0);
      break;

    case 0x0FF7:
      // Set the current bank to the second 4k bank
      bank(1);
      break;

    case 0x0FF8:
      // Set the current bank to the third 4k bank
      bank(2);
      break;

    case 0x0FF9:
      // Set the current bank to the forth 4k bank
      bank(3);
      break;

    default:
      break;
  }
  
  // NOTE: This does not handle accessing RAM, however, this function
  // should never be called for RAM because of the way page accessing
  // has been setup
  return myImage[myCurrentOffset | address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6SC::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF6:
      // Set the current bank to the first 4k bank
      bank(0);
      break;

    case 0x0FF7:
      // Set the current bank to the second 4k bank
      bank(1);
      break;

    case 0x0FF8:
      // Set the current bank to the third 4k bank
      bank(2);
      break;

    case 0x0FF9:
      // Set the current bank to the forth 4k bank
      bank(3);
      break;

    default:
      break;
  }

  // NOTE: This does not handle accessing RAM, however, this function
  // should never be called for RAM because of the way page accessing
  // has been setup
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void CartridgeF6SC::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentOffset = bank * 4096;

  // Setup the page access methods for the current bank
  uInt16 access_num = 0x1100 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x0100; address < (0x0FF6U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
  {
      myPageAccessTable[access_num++].directPeekBase = &myImage[myCurrentOffset + address];
  }
}
