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
#include "Random.hxx"
#include "Cart3EPlus.hxx"
#include "System.hxx"
#include "TIA.hxx"

uInt16 myCurrentBanks[4] = {0,0,0};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge3EPlus::Cartridge3EPlus(const uInt8* image, uInt32 size)
  : mySize(size)
{
  // Copy the ROM image into my buffer - just reuse the existing cart_buffer[] to save memory
  myImage = (uInt8 *)image;

  // Initialize RAM with random values
  class Random random;
  for(uInt32 i = 0; i < 32768; ++i)
  {
    xl_ram_buffer[i] = random.next();
  }
  
  // ----------------------------------------------------------------------
  // According to online docs, for 3E+ the only thing that really matters 
  // is that the first (0th) bank of the image is slotted into segment 3
  // but we will initalize all segments 0-3 with this starting bank...
  // ----------------------------------------------------------------------
  statingBank = 0;
  
  // Force saving of the extra 32K of RAM
  bSaveStateXL = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cartridge3EPlus::~Cartridge3EPlus()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* Cartridge3EPlus::name() const
{
  return "3E+";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3EPlus::reset()
{
  // Restore the original starting bank to all segments...
  segment(0, statingBank);
  segment(1, statingBank);
  segment(2, statingBank);
  segment(3, statingBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3EPlus::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();

  // Set the page accessing methods for the hot spots (for 100% emulation
  // we need to chain any accesses below 0x40 to the TIA. Our poke() method
  // does this via mySystem->tiaPoke(...) to chain the access.
  for(uInt32 i = 0x00; i < 0x80; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }

  // Start with nothing mapped for ROM access - we'll install segments below
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
  {
    page_access.device = this;
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    mySystem->setPageAccess(j >> shift, page_access);
  }
    
  // Install pages for all segments of the ROM
  segment(0, statingBank);
  segment(1, statingBank);
  segment(2, statingBank);
  segment(3, statingBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 Cartridge3EPlus::peek(uInt16 address)
{
    // We can only get here if the peek() is address in the lower 0x80 memory
  return theTIA.peek(address);   
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cartridge3EPlus::poke(uInt16 address, uInt8 value)
{
  // ----------------------------------------------------------------------------
  // Switch banks if necessary. 3F is the ROM hotspot and 3E is the RAM hotspot
  // ----------------------------------------------------------------------------
  if (address == 0x3F)
  {
    uInt8 seg = (value >> 6) & 0x03;
    segment(seg, value & 0x3F);         // Map ROM bank into desired segment
  }
  else if (address == 0x3E)
  {
    uInt8 seg = (value >> 6) & 0x03;
    segment(seg, (value & 0x3F) + 128); // Map RAM bank into desired segment
  }
  
  theTIA.poke(address, value);         // Pass through to "real" TIA
}

// ---------------------------------------------------------------------------
// Map a bank of ROM or RAM into the desired segment (0-3) of the memory map.
//
// Segment 0 is 0xF000 and is 1K bytes in lenth (0x400 bytes)
// Segment 1 is 0xF400 and is 1K bytes in lenth (0x400 bytes)
// Segment 2 is 0xF800 and is 1K bytes in lenth (0x400 bytes)
// Segment 3 is 0xFC00 and is 1K bytes in lenth (0x400 bytes)
//
// ROM will map into the entire segment where as RAM will be split
// as 0x200 bytes of memory space for Read Access and 0x200 bytes for write.
// ---------------------------------------------------------------------------
void Cartridge3EPlus::segment(uInt8 seg, uInt8 bank)
{
  if(bank < 128)  // ROM bank
  {
    myCurrentBanks[seg] = bank;
    uInt32 offset = bank * 0x400;
    uInt16 shift = mySystem->pageShift();

    // Setup the page access methods for the current bank
    page_access.device = this;
    page_access.directPokeBase = 0;

    // Map ROM image into the system
    for(uInt32 address = 0x1000+((uInt32)0x400*seg); address < 0x1400+((uInt32)0x400*seg); address += (1 << shift))
    {
        page_access.directPeekBase = &myImage[offset + (address & 0x03FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }
  }
  else // RAM bank
  {
    myCurrentBanks[seg] = bank;
    bank -= 128;

    uInt32 offset = bank * 0x200;
    uInt16 shift = mySystem->pageShift();

    // Setup the page access methods for the current bank
    page_access.device = this;
    page_access.directPokeBase = 0;

    // Map read-port RAM image into the system
    for(uInt32 address = 0x1000+((uInt32)0x400*seg); address < 0x1200+((uInt32)0x400*seg); address += (1 << shift))
    {
        page_access.directPeekBase = &xl_ram_buffer[offset + (address & 0x01FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }

    page_access.directPeekBase = 0;

    // Map write-port RAM image into the system - this is shifted up 0x200 bytes
    for(uInt32 address = 0x1200+((uInt32)0x400*seg); address < 0x1400+((uInt32)0x400*seg); address += (1 << shift))
    {
        page_access.directPokeBase = &xl_ram_buffer[offset + (address & 0x01FF)];
        mySystem->setPageAccess(address >> shift, page_access);
    }
  }    
}
