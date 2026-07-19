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
#include "CartF6.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF6::CartridgeF6(const uInt8* image)
{
  // Just reuse the existing cart buffer
  myImage = (uInt8 *)image;
    
  // Copy half the ROM image into the fast_cart_buffer[] for a bit of a speed-hack
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    fast_cart_buffer[addr] = image[addr];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF6::~CartridgeF6()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeF6::name() const
{
  return "F6";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6::reset()
{
  // Upon reset we switch to bank 0
  myCurrentOffset = 0 * 4096;
  bank(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Set the page accessing methods for the hot spots
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
  for(uInt32 i = (0x1FF6 & ~mask); i < 0x2000; i += (1 << shift))
  {
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // And setup for this system without any direct peek/poke until we switch banks
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << MY_PAGE_SHIFT))    
  {
      mySystem->setPageAccess(address >> shift, page_access);
  }
  
  // Upon install we'll setup bank 0
  myCurrentOffset = 0 * 4096;
  bank(0);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeF6::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF6:
      // Set the current bank to the first 4k bank
      myCurrentOffset = 0 * 4096;
      bank(0);
      break;

    case 0x0FF7:
      // Set the current bank to the second 4k bank
      myCurrentOffset = 1 * 4096;
      bank(1);
      break;

    case 0x0FF8:
      // Set the current bank to the third 4k bank
      myCurrentOffset = 2 * 4096;
      bank(2);
      break;

    case 0x0FF9:
      // Set the current bank to the forth 4k bank
      myCurrentOffset = 3 * 4096;
      bank(3);
      break;
  }

  return myImage[myCurrentOffset | address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF6::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch banks if necessary
  switch(address)
  {
    case 0x0FF6:
      // Set the current bank to the first 4k bank
      myCurrentOffset = 0 * 4096;
      bank(0);
      break;

    case 0x0FF7:
      // Set the current bank to the second 4k bank
      myCurrentOffset = 1 * 4096;
      bank(1);
      break;

    case 0x0FF8:
      // Set the current bank to the third 4k bank
      myCurrentOffset = 2 * 4096;
      bank(2);
      break;

    case 0x0FF9:
      // Set the current bank to the forth 4k bank
      myCurrentOffset = 3 * 4096;
      bank(3);
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline  void CartridgeF6::bank(uInt16 bank)
{ 
  // Setup the page access methods for the current bank
  uInt16 access_num = 0x1000 >> MY_PAGE_SHIFT;

  if (bank < 2)
  {
      // Map ROM image into the system - here we can use the fast_cart_buffer[]
      for(uInt32 address = 0x0000; address < (0x0FF6U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
      {
          myPageAccessTable[access_num++].directPeekBase = &fast_cart_buffer[myCurrentOffset + address];
      }
  }
  else
  {
      // Map ROM image into the system - here we use the normal cart_buffer[]
      for(uInt32 address = 0x0000; address < (0x0FF6U & ~MY_PAGE_MASK); address += (1 << MY_PAGE_SHIFT))
      {
          myPageAccessTable[access_num++].directPeekBase = &myImage[myCurrentOffset + address];
      }
  }
}

