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
#include "CartE0.hxx"
#include "System.hxx"
#include <iostream>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE0::CartridgeE0(const uInt8* image)
{
  myImage = (uInt8 *)fast_cart_buffer;
    
  // Copy the ROM image into my buffer
  for(uInt32 addr = 0; addr < 8192; ++addr)
  {
    myImage[addr] = image[addr];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE0::~CartridgeE0()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeE0::name() const
{
  return "E0";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::reset()
{
  // Setup segments to some default slices
  segmentZero(4);
  segmentOne(5);
  segmentTwo(6);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1000 & mask) == 0) && ((0x1400 & mask) == 0) &&
      ((0x1800 & mask) == 0) && ((0x1C00 & mask) == 0));

  // Set the page acessing methods for the first part of the last segment
  page_access.directPokeBase = 0;
  page_access.device = this;
  for(uInt32 i = 0x1C00; i < (0x1FE0U & ~mask); i += (1 << shift))
  {
    page_access.directPeekBase = &myImage[7168 + (i & 0x03FF)];
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Set the page accessing methods for the hot spots in the last segment
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
  for(uInt32 j = (0x1FE0 & ~mask); j < 0x2000; j += (1 << shift))
  {
    mySystem->setPageAccess(j >> shift, page_access);
  }

  // Just something so we have the right device
  for(uInt32 i = 0x1000; i < 0x1C00; i += (1 << shift))
  {
    page_access.directPeekBase = &myImage[(i & 0x0FFF)];
    mySystem->setPageAccess(i >> shift, page_access);
  }
    
  // Install some default slices for the other segments
  segmentZero(4);
  segmentOne(5);
  segmentTwo(6);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeE0::peek(uInt16 address)
{
  // Switch banks if necessary
  if ((address & 0x0FF8) == 0xFE0)
  {
    segmentZero(address & 0x0007);
  }
  else if ((address & 0x0FF8) == 0xFE8)
  {
    segmentOne(address & 0x0007);
  }
  else if ((address & 0x0FF8) == 0xFF0)
  {
    segmentTwo(address & 0x0007);
  }

  return fast_cart_buffer[(7168 + (address & 0x03FF))];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::poke(uInt16 address, uInt8)
{
  // Switch banks if necessary
  if ((address & 0x0FF8) == 0xFE0)
  {
    segmentZero(address & 0x0007);
  }
  else if ((address & 0x0FF8) == 0xFE8)
  {
    segmentOne(address & 0x0007);
  }
  else if ((address & 0x0FF8) == 0xFF0)
  {
    segmentTwo(address & 0x0007);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentZero(uInt16 slice)
{ 
    uInt32 offset = (slice << 10);
    
    // Setup the page access methods for the current bank - this is as fast as we can do it...
    myPageAccessTable[0x20].directPeekBase = &fast_cart_buffer[offset | 0x0000];
    myPageAccessTable[0x21].directPeekBase = &fast_cart_buffer[offset | 0x0080];
    myPageAccessTable[0x22].directPeekBase = &fast_cart_buffer[offset | 0x0100];
    myPageAccessTable[0x23].directPeekBase = &fast_cart_buffer[offset | 0x0180];
    myPageAccessTable[0x24].directPeekBase = &fast_cart_buffer[offset | 0x0200];
    myPageAccessTable[0x25].directPeekBase = &fast_cart_buffer[offset | 0x0280];
    myPageAccessTable[0x26].directPeekBase = &fast_cart_buffer[offset | 0x0300];
    myPageAccessTable[0x27].directPeekBase = &fast_cart_buffer[offset | 0x0380];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentOne(uInt16 slice)
{ 
    uInt32 offset = (slice << 10);

    // Setup the page access methods for the current bank - this is as fast as we can do it...
    myPageAccessTable[0x28].directPeekBase = &fast_cart_buffer[offset | 0x0000];
    myPageAccessTable[0x29].directPeekBase = &fast_cart_buffer[offset | 0x0080];
    myPageAccessTable[0x2A].directPeekBase = &fast_cart_buffer[offset | 0x0100];
    myPageAccessTable[0x2B].directPeekBase = &fast_cart_buffer[offset | 0x0180];
    myPageAccessTable[0x2C].directPeekBase = &fast_cart_buffer[offset | 0x0200];
    myPageAccessTable[0x2D].directPeekBase = &fast_cart_buffer[offset | 0x0280];
    myPageAccessTable[0x2E].directPeekBase = &fast_cart_buffer[offset | 0x0300];
    myPageAccessTable[0x2F].directPeekBase = &fast_cart_buffer[offset | 0x0380];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentTwo(uInt16 slice)
{ 
    uInt32 offset = (slice << 10);
    
    // Setup the page access methods for the current bank - this is as fast as we can do it...
    myPageAccessTable[0x30].directPeekBase = &fast_cart_buffer[offset | 0x0000];
    myPageAccessTable[0x31].directPeekBase = &fast_cart_buffer[offset | 0x0080];
    myPageAccessTable[0x32].directPeekBase = &fast_cart_buffer[offset | 0x0100];
    myPageAccessTable[0x33].directPeekBase = &fast_cart_buffer[offset | 0x0180];
    myPageAccessTable[0x34].directPeekBase = &fast_cart_buffer[offset | 0x0200];
    myPageAccessTable[0x35].directPeekBase = &fast_cart_buffer[offset | 0x0280];
    myPageAccessTable[0x36].directPeekBase = &fast_cart_buffer[offset | 0x0300];
    myPageAccessTable[0x37].directPeekBase = &fast_cart_buffer[offset | 0x0380];
}
