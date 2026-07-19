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
#include "Cart0840.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge0840::Cartridge0840(const uInt8* image)
{
  myImage = fast_cart_buffer;
    
  // Copy the ROM image into my buffer
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    myImage[addr] = image[addr];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge0840::~Cartridge0840()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge0840::name() const
{
  return "0840";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0840::reset()
{
  // Upon reset we switch to the starting bank 
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0840::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Get the page accessing methods for the hot spots since they overlap
  // areas within the TIA we'll need to forward requests to the TIA
  myHotSpotPageAccess[0]  = mySystem->getPageAccess(0x0800 >> shift);
  myHotSpotPageAccess[1]  = mySystem->getPageAccess(0x0880 >> shift);
  myHotSpotPageAccess[2]  = mySystem->getPageAccess(0x0900 >> shift);
  myHotSpotPageAccess[3]  = mySystem->getPageAccess(0x0980 >> shift);
  myHotSpotPageAccess[4]  = mySystem->getPageAccess(0x0A00 >> shift);
  myHotSpotPageAccess[5]  = mySystem->getPageAccess(0x0A80 >> shift);
  myHotSpotPageAccess[6]  = mySystem->getPageAccess(0x0B00 >> shift);
  myHotSpotPageAccess[7]  = mySystem->getPageAccess(0x0B80 >> shift);
  myHotSpotPageAccess[8]  = mySystem->getPageAccess(0x0C00 >> shift);
  myHotSpotPageAccess[9]  = mySystem->getPageAccess(0x0C80 >> shift);
  myHotSpotPageAccess[10] = mySystem->getPageAccess(0x0D00 >> shift);
  myHotSpotPageAccess[11] = mySystem->getPageAccess(0x0D80 >> shift);
  myHotSpotPageAccess[12] = mySystem->getPageAccess(0x0E00 >> shift);
  myHotSpotPageAccess[13] = mySystem->getPageAccess(0x0E80 >> shift);
  myHotSpotPageAccess[14] = mySystem->getPageAccess(0x0F00 >> shift);
  myHotSpotPageAccess[15] = mySystem->getPageAccess(0x0F80 >> shift);

  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
    
  // Set the page accessing methods for the hot spots
  mySystem->setPageAccess(0x0800 >> shift, page_access);
  mySystem->setPageAccess(0x0880 >> shift, page_access);
  mySystem->setPageAccess(0x0900 >> shift, page_access);
  mySystem->setPageAccess(0x0980 >> shift, page_access);
  mySystem->setPageAccess(0x0A00 >> shift, page_access);
  mySystem->setPageAccess(0x0A80 >> shift, page_access);
  mySystem->setPageAccess(0x0B00 >> shift, page_access);
  mySystem->setPageAccess(0x0B80 >> shift, page_access);
  mySystem->setPageAccess(0x0C00 >> shift, page_access);
  mySystem->setPageAccess(0x0C80 >> shift, page_access);
  mySystem->setPageAccess(0x0D00 >> shift, page_access);
  mySystem->setPageAccess(0x0D80 >> shift, page_access);
  mySystem->setPageAccess(0x0E00 >> shift, page_access);
  mySystem->setPageAccess(0x0E80 >> shift, page_access);
  mySystem->setPageAccess(0x0F00 >> shift, page_access);
  mySystem->setPageAccess(0x0F80 >> shift, page_access);

  // Install pages for the starting bank 
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge0840::peek(uInt16 address)
{
  address = address & 0x1840;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0800:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x0840:
      // Set the current bank to the upper 4k bank
      bank(1);
      break;
  }

  // Because of the way we've set up accessing above, we can only
  // get here when the addresses are from 0x800 - 0xFFF
  if(!(address & 0x1000))
  {
      const int hotspot = ((address & 0x0F80) >> 7) - 16;
      return myHotSpotPageAccess[hotspot].device->peek(address);
  }
  return 0x00;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0840::poke(uInt16 address, uInt8 value)
{
  address = address & 0x1840;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0800:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x0840:
      // Set the current bank to the upper 4k bank
      bank(1);
      break;
  }
    
  // Because of the way accessing is set up, we will may get here by
  // doing a write to 0x800 - 0xFFF or cart; we ignore the cart write
  if(!(address & 0x1000))
  {
    const int hotspot = ((address & 0x0F80) >> 7) - 16;
    myHotSpotPageAccess[hotspot].device->poke(address, value);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0840::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentOffset = bank * 4096;

  // Setup the page access methods for the current bank
  uInt32 access_num = 0x1000 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << MY_PAGE_SHIFT))
  {
      page_access.directPeekBase = &myImage[myCurrentOffset + (address & 0xFFF)];
      mySystem->setPageAccess(access_num++, page_access);
  }
}

