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
#include "CartUA.hxx"
#include "System.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeUA::CartridgeUA(const uInt8* image, uInt8 bSwap)
{
  myImage = fast_cart_buffer;
  // Copy the ROM image into my buffer
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    myImage[addr] = image[addr];
  }
  bUAswapped = bSwap;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeUA::~CartridgeUA()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeUA::name() const
{
  return "UA";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeUA::reset()
{
  // Upon reset we switch to bank 0
  bank(bUAswapped ? 1:0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeUA::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Get the page accessing methods for the hot spots since they overlap
  // areas within the TIA we'll need to forward requests to the TIA
  myHotSpotPageAccess  = mySystem->getPageAccess(0x0220 >> shift);
  myHotSpotPageAccess2 = mySystem->getPageAccess(0x02A0 >> shift);

  // Set the page accessing methods for the hot spots
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
    
  for(uInt16 a11 = 0; a11 <= 1; ++a11)
    for(uInt16 a10 = 0; a10 <= 1; ++a10)
      for(uInt16 a8 = 0; a8 <= 1; ++a8)
        for(uInt16 a7 = 0; a7 <= 1; ++a7)
        {
          const uInt16 addr = (a11 << 11) + (a10 << 10) + (a8 << 8) + (a7 << 7);

          mySystem->setPageAccess((0x0220 | addr) >> shift, page_access);
          mySystem->setPageAccess((0x0240 | addr) >> shift, page_access);
        }

  // Install pages for bank 0
  bank(bUAswapped ? 1:0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeUA::peek(uInt16 address)
{
  address = address & 0x1FFF;

  // Switch banks if necessary
  switch(address & 0x1260)
  {
    case 0x0220:
      // Set the current bank to the lower 4k bank
      bank(bUAswapped ? 1:0);
      break;

    case 0x0240:
      // Set the current bank to the upper 4k bank
      bank(bUAswapped ? 0:1);
      break;

    default:
      break;
  }

  if(!(address & 0x1000))
  {
      if (address & 0x80)
          return myHotSpotPageAccess2.device->peek(address);
      else
          return myHotSpotPageAccess.device->peek(address);
  }
  else
  {
    return 0;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeUA::poke(uInt16 address, uInt8 value)
{
  address = address & 0x1FFF;

  // Switch banks if necessary
  switch(address & 0x1260)
  {
    case 0x0220:
      // Set the current bank to the lower 4k bank
      bank(bUAswapped ? 1:0);
      break;

    case 0x0240:
      // Set the current bank to the upper 4k bank
      bank(bUAswapped ? 0:1);
      break;

    default:
      break;
  }

  if(!(address & 0x1000))
  {
      if (address & 0x80)
          myHotSpotPageAccess2.device->poke(address, value);
      else
          myHotSpotPageAccess.device->poke(address, value);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeUA::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentBank = bank;
  uInt16 offset = myCurrentBank * 4096;
  uInt16 shift = mySystem->pageShift();
//  uInt16 mask = mySystem->pageMask();

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
