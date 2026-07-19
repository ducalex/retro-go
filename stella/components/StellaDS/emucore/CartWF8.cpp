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
#include "CartWF8.hxx"
#include "System.hxx"
#include <iostream>

extern uInt16 f8_bankbit;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeWF8::CartridgeWF8(const uInt8* image)
{
  myImage = fast_cart_buffer;    
  // Copy the ROM image into my buffer
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    myImage[addr] = image[addr];
  }
  f8_bankbit = 0x1FFF;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeWF8::~CartridgeWF8()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeWF8::name() const
{
  return "WF8";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWF8::reset()
{
  // Upon reset we switch to bank 1
  f8_bankbit = 0x1FFF;
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWF8::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
    
  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FF8 & ~mask); i < 0x2000; i += (1 << shift))
  {
    mySystem->setPageAccess(i >> shift, page_access);
  }
  
  // And setup for this system without any direct peek/poke until we switch banks
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << MY_PAGE_SHIFT))    
  {
      mySystem->setPageAccess(address >> shift, page_access);
  }

  // Install pages for bank 1
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeWF8::peek(uInt16 address)
{
  return fast_cart_buffer[myCurrentOffset + (address & 0xFFF)];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWF8::poke(uInt16 address, uInt8 data)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF8:
      // Set the current bank to the lower 4k bank
      bank((data & 0x04) ? 1:0);
      break;
  }
}

extern uInt16 f8_bankbit;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWF8::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentOffset = bank * 4096;
    
  f8_bankbit = (bank ? 0x1FFF:0x0FFF);

  // Setup the page access methods for the current bank
  uInt32 access_num = 0x1000 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x0000; address < (0x0FF8U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
  {
      myPageAccessTable[access_num++].directPeekBase = &fast_cart_buffer[myCurrentOffset + address];
  }
}

