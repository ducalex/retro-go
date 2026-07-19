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
#include <iostream>
#include "CartDPC.hxx"
#include "System.hxx"

// The 2K display ROM image of the cartridge
uInt8 myDisplayImage[2048];

// The flag registers for the data fetchers
uInt8 myFlags[8];

// The random number generator register
uInt8 myRandomNumber;

// The music mode DF5, DF6, & DF7 enabled flags
uInt8 myMusicMode[3];

// System cycle count when the last update to music data fetchers occurred
uInt32 myMusicCycles  = 0;

static const uInt8 musicAmplitudes[8] =
{
  0x00, 0x04, 0x05, 0x09, 0x06, 0x0a, 0x0b, 0x0f
};
CartridgeDPC *myCartDPC ;

extern uInt16 f8_bankbit;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPC::CartridgeDPC(const uInt8* image, uInt32 size)
{
  uInt16 addr;

  // Copy the program ROM image into my buffer
  for(addr = 0; addr < 8192; ++addr)
  {
    fast_cart_buffer[addr] = image[addr];
  }

  // Copy the display ROM image into my buffer
  for(addr = 0; addr < 2048; ++addr)
  {
    myDisplayImage[addr] = (uInt16)image[8192+2047 - addr]; // Yes, load it in backwards - it helps with speed when indexing later...
  }

  // Initialize the DPC data fetcher registers
  for(uInt16 i = 0; i < 8; ++i)
  {
    myTops[i] = myBottoms[i] = myCounters[i] = myFlags[i] = 0;
  }
    
  // None of the data fetchers are in music mode
  myMusicMode[0] = myMusicMode[1] = myMusicMode[2] = false;

  // Initialize the DPC's random number generator register (must be non-zero)
  myRandomNumber = 1;

  // Initialize the system cycles counter & fractional clock values
  myMusicCycles = 0;
    
  myCartDPC = this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPC::~CartridgeDPC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeDPC::name() const
{
  return "DPC";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::reset()
{
  // Update cycles to the current system cycles
  myMusicCycles = mySystem->cycles();

  // Upon reset we switch to bank 1
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = mySystem->cycles();

  // Adjust the cycle counter so that it reflects the new value
  myMusicCycles -= cycles;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  page_access.directPeekBase = 0;
  page_access.directPokeBase = 0;
  page_access.device = this;
  
  // Set page accessing method for the DPC reading & writing pages
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
  {
    mySystem->setPageAccess(j >> shift, page_access);
  }
    
  // Install pages for bank 1
  bank(1);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPC::bank(uInt16 bank)
{ 
    f8_bankbit = (bank ? 0x1FFF : 0x0FFF);
}

#define DPC_MUSIC_PITCH 5966  // 59.66Hz

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::updateMusicModeDataFetchers(uInt32 delta)
{
  Int32 wholeClocks = (delta*100) / DPC_MUSIC_PITCH;

  // Let's update counters and flags of the music mode data fetchers
  for(int x = 5; x <= 7; ++x)
  {
    // Update only if the data fetcher is in music mode
    if(myMusicMode[x - 5])
    {
      Int32 top = myTops[x] + 1;
      Int32 newLow = (Int32)(myCounters[x] & 0x00ff);

      if(myTops[x] != 0)
      {
        newLow -= (wholeClocks % top);
        if(newLow < 0)
        {
          newLow += top;
        }
      }
      else
      {
        newLow = 0;
      }

      // Update flag register for this data fetcher
      if(newLow <= (Int32)myBottoms[x])
      {
        myFlags[x] = 0x00;
      }
      else if(newLow <= (Int32)myTops[x])
      {
        myFlags[x] = 0xff;
      }

      myCounters[x] = (myCounters[x] & 0x0700) | (uInt16)newLow;
    }
  }
}

uInt8 CartridgeDPC::peek(uInt16 address)
{
    return fast_cart_buffer[address & f8_bankbit];  // Will only be called twice at the start of the world
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeDPC::peek_fetch(uInt16 address)
{
    uInt8 result = 0;
    
    // Get the index of the data fetcher that's being accessed
    uInt8 index = address & 0x07;
    uInt8 function = (address >> 3);

    // Update flag register for selected data fetcher
    if((myCounters[index] & 0x00ff) == myTops[index])
    {
      myFlags[index] = 0xff;
    }
    else if((myCounters[index] & 0x00ff) == myBottoms[index])
    {
      myFlags[index] = 0x00;
    }
    
    switch(function)
    {
      case 0x00:
        if (index < 4) 
        {
            static const u8 DPCRandom[] = {37,127,18,65,203,255,9,188};
            result = DPCRandom[++myRandomNumber & 7];   // Not truly random but good enough to flash the eel in Pitfall II (which is all this is ever used for)
        }
        else    // No... it's a music fetcher
        {
          if (myCartInfo.soundQuality == SOUND_WAVE)
          {
              uInt32 delta = gSystemCycles - myMusicCycles;
              if ((delta*100) >= DPC_MUSIC_PITCH)
              {              
                  // Update the music data fetchers (counter & flag)
                  updateMusicModeDataFetchers(delta);
                  myMusicCycles = gSystemCycles - ((((delta*100) % DPC_MUSIC_PITCH)+50) / 100);
              }

              uInt8 i = 0;
              if(myMusicMode[0] && myFlags[5])
              {
                i |= 0x01;
              }
              if(myMusicMode[1] && myFlags[6])
              {
                i |= 0x02;
              }
              if(myMusicMode[2] && myFlags[7])
              {
                i |= 0x04;
              }

              result = musicAmplitudes[i];
          }
          else
          {
              result = 0x00;
          }
        }
        break;
            
      // DFx display data read
      case 0x01:
      {
        result = myDisplayImage[myCounters[index]];
        break;
      }

      // DFx display data read AND'd w/flag
      case 0x02:
      {
        result = myDisplayImage[myCounters[index]] & myFlags[index];
        break;
      } 

      // DFx flag
      case 0x07:
      {
        result = myFlags[index];
        break;
      }
    }

    // Clock the selected data fetcher's counter if needed
    if((index < 5) || ((index >= 5) && (!myMusicMode[index - 5])))
    {
      myCounters[index] = (myCounters[index] - 1) & 0x07ff;
    }
      
    return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::poke(uInt16 address, uInt8 value)
{
    address = address & 0x0FFF;

    // Get the index of the data fetcher that's being accessed
    uInt32 index = address & 0x07;    
    uInt32 function = (address >> 3) & 0x07;

    switch(function)
    {
      // DFx top count
      case 0x00:
      {
        myTops[index] = value;
        myFlags[index] = 0x00;
        break;
      }

      // DFx bottom count
      case 0x01:
      {
        myBottoms[index] = value;
        break;
      }

      // DFx counter low
      case 0x02:
      {
        if((index >= 5) && myMusicMode[index - 5])
        {
          // Data fecther is in music mode so its low counter value
          // should be loaded from the top register not the poked value
          myCounters[index] = (myCounters[index] & 0x0700) | (uInt16)myTops[index];
        }
        else
        {
            // Data fecther is either not a music mode data fecther or it
            // isn't in music mode so it's low counter value should be loaded
            // with the poked value
            myCounters[index] = (myCounters[index] & 0x0700) | (uInt16)value;
        }
        break;
      }

      // DFx counter high
      case 0x03:
      {
        myCounters[index] = (((uInt16)value & 0x07) << 8) | (myCounters[index] & 0x00ff);
        // Execute special code for music mode data fetchers
        if(index >= 5)
        {
            myMusicMode[index - 5] = (value & 0x10);
        }
        break;
      }
    } 
}
