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
#include "Cart03E0.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge03E0::Cartridge03E0(const uInt8* image)
{
    myImage = fast_cart_buffer;

    // Copy the ROM image into my buffer
    for(uInt32 addr = 0; addr < 8192; ++addr)
    {
        myImage[addr] = image[addr];
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge03E0::~Cartridge03E0()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge03E0::name() const
{
    return "03E0";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge03E0::reset()
{
    // Upon reset we switch to the starting banks
    bank(4, 0);
    bank(5, 1);
    bank(6, 2);  
    bank(7, 3);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge03E0::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Get the page accessing methods for the hot spots since they overlap
  // areas within the TIA we'll need to forward requests to the TIA
  myHotSpotPageAccess = mySystem->getPageAccess(0x0380 >> shift);

  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
    
  // Set the page accessing methods for the hot spots
  mySystem->setPageAccess(0x0380 >> shift, page_access);

  // Install pages for the starting bank 
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge03E0::checkSwitchBank(uInt16 address)
{
    // Yes, it's possible to switch more than one bank/segment
    if((address & 0x10) == 0) bank(address & 0x0007, 0);
    if((address & 0x20) == 0) bank(address & 0x0007, 1);
    if((address & 0x40) == 0) bank(address & 0x0007, 2);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge03E0::peek(uInt16 address)
{
    checkSwitchBank(address);
    return myHotSpotPageAccess.device->peek(address & 0x3FF);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge03E0::poke(uInt16 address, uInt8 value)
{
    // Check to make sure this isn't an inadvertant cart-write...
    if(!(address & 0x1000))
    {
        checkSwitchBank(address);
        myHotSpotPageAccess.device->poke(address & 0x3FF, value);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge03E0::bank(uInt16 bank, uInt16 segment)
{ 
    // Offset to the 1K Bank in ROM
    myCurrentOffset = bank * 1024;

    // Setup the page access methods for the current bank/segment
    uInt32 access_num = (0x1000 + (segment * 1024)) >> MY_PAGE_SHIFT;
    
    // Map ROM image into the system for the desired 1K Segment
    for(uInt32 address = 0x1000 + (segment * 1024); address < (uInt32)(0x1000 + ((segment+1) * 1024)); address += (1 << MY_PAGE_SHIFT))
    {
        page_access.directPeekBase = &myImage[myCurrentOffset + (address & 0x3FF)];
        mySystem->setPageAccess(access_num++, page_access);
    }
}
