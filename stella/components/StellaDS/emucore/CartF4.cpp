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
#include "CartF4.hxx"
#include "Random.hxx"
#include "System.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF4::CartridgeF4(const uInt8* image)
{
  // Just reuse the existing cart buffer
  myImage = (uInt8 *)image;
    
  // Copy 8K of the ROM image into the fast_cart_buffer[] for a bit of a speed-hack
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    fast_cart_buffer[addr] = image[addr];
  }    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF4::~CartridgeF4()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeF4::name() const
{
  return "F4";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF4::reset()
{
  // Upon reset we switch to bank 0
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF4::install(System& system)
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
  for(uInt32 i = (0x1FF4 & ~mask); i < 0x2000; i += (1 << shift))
  {
      mySystem->setPageAccess(i >> shift, page_access);
  }

  // And setup for this system without any direct peek/poke until we switch banks
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << MY_PAGE_SHIFT))    
  {
      mySystem->setPageAccess(address >> shift, page_access);
  }
    
  // Install pages for bank 0
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeF4::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FF4) && (address <= 0x0FFB))
  {
    bank(address - 0x0FF4);
  }

  return myImage[myCurrentOffset + address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF4::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FF4) && (address <= 0x0FFB))
  {
    bank(address - 0x0FF4);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeF4::bank(uInt16 bank)
{ 
  // Remember what bank we're in
  myCurrentOffset = bank * 4096;

  // Setup the page access methods for the current bank
  uInt16 access_num = 0x1000 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x0000; address < (0x0FF4U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
  {
      myPageAccessTable[access_num++].directPeekBase = &myImage[myCurrentOffset + address];
  }
}
