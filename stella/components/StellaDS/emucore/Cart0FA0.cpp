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
#include "Cart0FA0.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge0FA0::Cartridge0FA0(const uInt8* image)
{
    myImage = fast_cart_buffer;

    // Copy the ROM image into my buffer
    for(uInt32 addr = 0; addr < 8192; ++addr)
    {
        myImage[addr] = image[addr];
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge0FA0::~Cartridge0FA0()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge0FA0::name() const
{
    return "0FA0";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0FA0::reset()
{
    // Upon reset we switch to the starting bank (1)
    bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0FA0::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Get the page accessing methods for the hot spots since they overlap
  // areas within the lower memory that we will need to forward along...
  myHotSpotPageAccess = mySystem->getPageAccess(0x06a0 >> shift);

  // Install pages for the starting bank
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0FA0::checkSwitchBank(uInt16 address)
{
  // Switch banks if necessary
  if ((address & 0x16e0) == 0x06a0)     bank(0);
  if ((address & 0x16e0) == 0x06c0)     bank(1);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge0FA0::peek(uInt16 address)
{
    checkSwitchBank(address);
    if(!(address & 0x1000)) return myHotSpotPageAccess.device->peek(address);
    else return myImage[myCurrentOffset + (address & 0xFFF)];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0FA0::poke(uInt16 address, uInt8 value)
{
    checkSwitchBank(address);
     
    // Check to make sure this isn't an inadvertant cart-write...
    if(!(address & 0x1000))
    {
        myHotSpotPageAccess.device->poke(address, value);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge0FA0::bank(uInt16 bank)
{ 
    // Offset to the 4K Bank in ROM
    myCurrentOffset = bank * 4096;

    // Setup the page access methods for the current bank
    uInt32 access_num = 0x1000 >> MY_PAGE_SHIFT;
    
    // Map ROM image into the system for the desired 4K Bank
    for(uInt32 address = 0x1000; address < 0x2000; address += (1 << MY_PAGE_SHIFT))
    {
        page_access.directPeekBase = &myImage[myCurrentOffset + (address & 0xFFF)];
        mySystem->setPageAccess(access_num++, page_access);
    }
    
    // ------------------------------------------------------------------------
    // Now we need to put back all potential hotspots - some of which may have
    // been overritten by the mapping of ROM into the system directly above...
    // ------------------------------------------------------------------------
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;

    // Map all potential addresses
    // - A11 and A8 are not connected to RIOT
    // - A10, A9 and A7 are the fixed part of the hotspot address
    // - A6 and A5 determine bank
    for(uInt16 a11 = 0; a11 <= 1; ++a11)
    {
        for(uInt16 a8 = 0; a8 <= 1; ++a8)
        {
          const uInt16 addr = (a11 << 11) + (a8 << 8);

          mySystem->setPageAccess((0x06a0 | addr) >> MY_PAGE_SHIFT, page_access);
          mySystem->setPageAccess((0x06c0 | addr) >> MY_PAGE_SHIFT, page_access);
        }
    }
}
