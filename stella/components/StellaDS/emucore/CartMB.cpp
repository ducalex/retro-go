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
#include "CartMB.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMB::CartridgeMB(const uInt8* image)
{
  // Image is already in cart_buffer[]
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMB::~CartridgeMB()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeMB::name() const
{
  return "F0/MB";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::reset()
{
  // Upon reset we switch to bank 1
  myCurrentBank = 0;
  incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FF0 & ~mask); i < 0x2000; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Install pages for bank 1
  myCurrentBank = 0;
  incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeMB::peek(uInt16 address)
{
  address = address & 0x0FFF;

  // Switch to next bank
  if(address == 0x0FF0) incbank();

  return cart_buffer[myCurrentBank * 4096 + address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::poke(uInt16 address, uInt8)
{
  address = address & 0x0FFF;

  // Switch to next bank
  if(address == 0x0FF0) incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::incbank()
{
  // Remember what bank we're in
  myCurrentBank ++;
  myCurrentBank &= 0x0F;
  uInt16 offset = myCurrentBank * 4096;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Setup the page access methods for the current bank
  page_access.device = this;
  page_access.directPokeBase = 0;

  // Map ROM image into the system
  for(uInt32 address = 0x1000; address < (0x1FF0U & ~mask);
      address += (1 << shift))
  {
    page_access.directPeekBase = &cart_buffer[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, page_access);
  }
}
