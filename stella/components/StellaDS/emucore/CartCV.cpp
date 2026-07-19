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
#include "CartCV.hxx"
#include "Random.hxx"
#include "System.hxx"
#include <iostream>

// ---------------------------------------------------------------------------------------------
// We use the fast_cart_buffer[] here for both the cartridge 2K of ROM and the 1K Commavid RAM
// which takes a full 2K to address for both read/write. Therefore, 4K of fast buffer is used.
// ---------------------------------------------------------------------------------------------


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCV::CartridgeCV(const uInt8* image, uInt32 size)
{
  uInt32 addr;
  
  if (size <= 2048)
  {
    // Copy the ROM image into my buffer. ROM is in the upper 2K half (RAM in the lower)
    for(uInt32 addr = 0; addr < 2048; ++addr)
    {
        fast_cart_buffer[2048+addr] = image[addr];
    }

    // Initialize RAM with random values
    Random random;
    for(uInt32 i = 0; i < 1024; ++i)
    {
        fast_cart_buffer[i] = random.next();
    }
  }
  else if (size == 4096)
  {
    // The game has something saved in the RAM
    // Useful for MagiCard program listings

    // Copy the ROM and RAM into my buffer
    for(addr = 0; addr < 4096; ++addr)
    {
        fast_cart_buffer[addr] = image[addr];
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCV::~CartridgeCV()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeCV::name() const
{
  return "CV";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCV::reset()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCV::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1800 & mask) == 0);

  page_access.directPokeBase = 0;
  page_access.directPeekBase = 0;
  page_access.device = this;

  // Map ROM image into the system
  for(uInt32 address = 0x1800; address < 0x2000; address += (1 << shift))
  {
    page_access.directPeekBase = &fast_cart_buffer[2048 + (address & 0x07FF)];
    mySystem->setPageAccess(address >> shift, page_access);
  }

  // Set the page accessing method for the RAM writing pages
  for(uInt32 j = 0x1400; j < 0x1800; j += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = &fast_cart_buffer[(j & 0x03FF)];
    mySystem->setPageAccess(j >> shift, page_access);
  }

  // Set the page accessing method for the RAM reading pages
  for(uInt32 k = 0x1000; k < 0x1400; k += (1 << shift))
  {
    page_access.directPeekBase = &fast_cart_buffer[(k & 0x03FF)];
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(k >> shift, page_access);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeCV::peek(uInt16 address)
{
  return fast_cart_buffer[address & 0x0FFF];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCV::poke(uInt16, uInt8)
{
  // This is ROM so poking has no effect :-)
}
