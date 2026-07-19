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
#include "CartF8.hxx"
#include "M6502.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF8::CartridgeF8(const uInt8* image)
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
CartridgeF8::~CartridgeF8()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeF8::name() const
{
  return "F8";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF8::reset()
{
  // Upon reset we switch to bank 1
  f8_bankbit = 0x1FFF;
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF8::install(System& system)
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
 uInt8 CartridgeF8::peek(uInt16 address)
{
  address = address & 0x1FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x1FF8:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x1FF9:
      // Set the current bank to the upper 4k bank
      bank(1);
      break;
  }

  return myImage[address & f8_bankbit];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void CartridgeF8::poke(uInt16 address, uInt8)
{
  address = address & 0x1FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x1FF8:
      // Set the current bank to the lower 4k bank
      bank(0);
      break;

    case 0x1FF9:
      // Set the current bank to the upper 4k bank
      bank(1);
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void CartridgeF8::bank(uInt16 bank)
{    
  f8_bankbit = (bank ? 0x1FFF:0x0FFF);

  // Setup the page access methods for the current bank
  uInt32 access_num = 0x1000 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x1000; address < (0x1FF8U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
  {
      myPageAccessTable[access_num++].directPeekBase = &fast_cart_buffer[address & f8_bankbit];
  }
}

