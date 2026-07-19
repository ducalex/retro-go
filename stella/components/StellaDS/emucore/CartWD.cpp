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

#include <string.h>
#include <assert.h>
#include "CartWD.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include <iostream>

Int32 myCyclesAtBankswitchInit;
uInt8 myPendingBank;

const uInt8 ourBankOrg[16][4] = 
{
                   //             0 1 2 3 4 5 6 7
  { 0, 0, 1, 3 },  // Bank 0,  8  2 1 - 1 - - - -
  { 0, 1, 2, 3 },  // Bank 1,  9  1 1 1 1 - - - -
  { 4, 5, 6, 7 },  // Bank 2, 10  - - - - 1 1 1 1
  { 7, 4, 2, 3 },  // Bank 3, 11  - - 1 1 1 - - 1
  { 0, 0, 6, 7 },  // Bank 4, 12  2 - - - - - 1 1
  { 0, 1, 7, 6 },  // Bank 5, 13  1 1 - - - - 1 1
  { 2, 3, 4, 5 },  // Bank 6, 14  - - 1 1 1 1 - -
  { 6, 0, 5, 1 },  // Bank 7, 15  1 1 - - - 1 1 -

  { 0, 0, 1, 3 },  // Bank 0,  8  2 1 - 1 - - - -
  { 0, 1, 2, 3 },  // Bank 1,  9  1 1 1 1 - - - -
  { 4, 5, 6, 7 },  // Bank 2, 10  - - - - 1 1 1 1
  { 7, 4, 2, 3 },  // Bank 3, 11  - - 1 1 1 - - 1
  { 0, 0, 6, 7 },  // Bank 4, 12  2 - - - - - 1 1
  { 0, 1, 7, 6 },  // Bank 5, 13  1 1 - - - - 1 1
  { 2, 3, 4, 5 },  // Bank 6, 14  - - 1 1 1 1 - -
  { 6, 0, 5, 1 }   // Bank 7, 15  1 1 - - - 1 1 -
};

static uInt8 tmpSlice[1024];
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeWD::CartridgeWD(const uInt8* image, uInt32 size)
    : mySize(size)
{
    myImage = fast_cart_buffer;

    // -----------------------------------------------------------------------------------------------------
    // There is a common "bad dump" of Pink Panther that has banks 2/3 swapped (and is 3 bytes too long).
    // -----------------------------------------------------------------------------------------------------
    if(size == 8195)
    {
        // swap banks 2 & 3 of bad dump and correct size
        memcpy((char *)tmpSlice,          (char *)(image + 2048), 1024);
        memcpy((char *)(image+2048),      (char *)(image + 3072), 1024);
        memcpy((char *)(image+3072),      (char *)tmpSlice,       1024);
        size = 8192;
    }
        
    // ----------------------------------------------------------------------------
    // Copy the ROM image into my buffer. The bad dump extra 3 bytes are ignored.
    // ----------------------------------------------------------------------------
    for(uInt32 addr = 0; addr < size; ++addr)
    {
        myImage[addr] = image[addr];
    }
        
    myCyclesAtBankswitchInit = 0;
    myPendingBank = 0xF0;        
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeWD::~CartridgeWD()
{
    // Nothing to do... 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeWD::name() const
{
  return "WD";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWD::reset()
{
  myCyclesAtBankswitchInit = 0;
  myPendingBank = 0xF0;  // sentinal value... no bank hotswap yet
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWD::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();

  // We're going to map the first 40 hex bytes here... we will chain call into TIA as needed
  for(uInt32 i = 0x00; i < 0x40; i += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // And then map the rest of ROM to our custom peek handler...
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(j >> shift, page_access);
  }
    
  // Leave these zero for faster bank switch
  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;

  // Install pages for bank 0 into the first segment
  myCurrentBank = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeWD::peek(uInt16 address)
{
  // Is it time to do an actual bankswitch?
  if(myPendingBank != 0xF0 && (gSystemCycles > (myCyclesAtBankswitchInit + 3)))
  {
    myCurrentBank = myPendingBank & 0x0F;
    myPendingBank = 0xF0;
  }

  // Hotspots below 0x1000 are also TIA addresses
  if(!(address & 0x1000))   
  {
    // Hotspots at $30 - $3F
    // Note that a hotspot read triggers a bankswitch after at least 3 cycles (see above).
    if((address & 0x00FF) >= 0x30 && (address & 0x00FF) <= 0x3F)
    {
      myCyclesAtBankswitchInit = gSystemCycles;
      myPendingBank = address & 0x000F;
    }
    return theTIA.peek(address);  // Chain the call along to the real TIA
  }
  else  // We are doing a real peek() of our ROM image - take into account the unusual bank switching...
  {
      address = address & 0x0FFF;       // Map down to 4k
      if (address < 0x40)               // Lower 128 bytes of first bank is special RAM (first 64 bytes to READ, next 64 bytes to WRITE)
      {
        return myRAM[128 +(address & 0x3F)];
      }
      else if (address < 0x400)
      {
        return myImage[(1024 * ourBankOrg[myCurrentBank][0]) + (address & 0x3FF)];
      }
      else if (address < 0x800)
      {
        return myImage[(1024 * ourBankOrg[myCurrentBank][1]) + (address & 0x3FF)];
      }
      else if (address < 0xC00)
      {
        return myImage[(1024 * ourBankOrg[myCurrentBank][2]) + (address & 0x3FF)];
      }
      else
      {
        return myImage[(1024 * ourBankOrg[myCurrentBank][3]) + (address & 0x3FF)];
      }
  }    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeWD::poke(uInt16 address, uInt8 value)
{
  if (address & 0x1000)
  {
      if ((address & 0x0FFF) < 0x0080)  // Lower 128 bytes of first bank is special RAM (first 64 bytes to READ, next 64 bytes to WRITE)
      {
        myRAM[128 + (address & 0x3F)] = value;
      }
  }
  else
  {
      if ((address& 0x0FFF) < 0x0040)
      {
        theTIA.poke(address, value);
      }
  }
}
