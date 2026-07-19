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

#include "Device.hxx"
#include "M6502.hxx"
#include "M6532.hxx"
#include "System.hxx"
#include "TIA.hxx"

// ---------------------------------------------------------------------------
// Global speedup hack which gives 10% or better speedup in core emulation.
// I'm not proud of all the globals... Stella is beautiful code and as such 
// deserves better than some global variable hack but the speedup is needed 
// to bring even more games up to 60 FPS. Sometimes the ends justify the means.
// ---------------------------------------------------------------------------
Int32  gSystemCycles    = 0;

uInt32 gTotalSystemCycles = 0;

// -----------------------------------------------------------------------------
// Allocate page table - since this is in main RAM, the compiler will be able
// to optiize access as the memory will be known at compile time. Again, not
// overly proud of making these things global but speed is critical on the DS.
// -----------------------------------------------------------------------------
PageAccess myPageAccessTable[64] ; 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
System::System(uInt16 n, uInt16 m)
    : myAddressMask((1 << n) - 1),
      myPageShift(m),
      myPageMask((1 << m) - 1),
      myNumberOfPages(1 << (n - m)),
      myNumberOfDevices(0),
      myM6502(0)
{
  // Make sure the arguments are reasonable
  assert((1 <= m) && (m <= n) && (n <= 16));

  // Initialize page access table
  PageAccess access;
  access.directPeekBase = 0;
  access.directPokeBase = 0;
  access.device = &myNullDevice;
  for(int page = 0; page < myNumberOfPages; ++page)
  {
    setPageAccess(page, access);
  }
          
  gTotalSystemCycles = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
System::~System()
{
  // Free the devices attached to me, since I own them
  for(uInt32 i = 0; i < myNumberOfDevices; ++i)
  {
      delete myDevices[i];
  }

  // Free the M6502 that I own
  delete myM6502;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void System::reset()
{
  // Reset system cycle counter
  resetCycles();

  // First we reset the devices attached to myself
  for(uInt32 i = 0; i < myNumberOfDevices; ++i)
  {
    myDevices[i]->reset();
  }

  // Reset the global classes
  theM6532.reset();
  theTIA.reset();

  // Now we reset the processor if it exists
  if(myM6502 != 0)
  {
    myM6502->reset();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void System::attach(Device* device)
{
  // Add device to my collection of devices
  myDevices[myNumberOfDevices++] = device;

  // Ask the device to install itself
  device->install(*this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void System::attach(M6502* m6502)
{
  // Remember the processor
  myM6502 = m6502;

  // Ask the processor to install itself
  myM6502->install(*this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void System::resetCycles()
{
  // First we let all of the device attached to me know about the reset
  for(uInt32 i = 0; i < myNumberOfDevices; ++i)
  {
    myDevices[i]->systemCyclesReset();
  }
  
  // Reset the global classes
  theM6532.systemCyclesReset();
  theTIA.systemCyclesReset();  
  
  gTotalSystemCycles += gSystemCycles;
    
  // Now, we reset cycle count to zero
  gSystemCycles = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PageAccess& System::getPageAccess(uInt16 page)
{
  return myPageAccessTable[page];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
System::System(const System& s)
    : myAddressMask(s.myAddressMask),
      myPageShift(s.myPageShift),
      myPageMask(s.myPageMask),
      myNumberOfPages(s.myNumberOfPages)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
System& System::operator = (const System&)
{
  assert(false);

  return *this;
}
