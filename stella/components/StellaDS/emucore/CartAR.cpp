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
#include <string.h>
#include "Cart.hxx"
#include "CartAR.hxx"
#include "M6502Low.hxx"
#include "Random.hxx"
#include "System.hxx"
#include <iostream>

// We index purposely out of bounds (but correct it on the fetch) as a speed hack...
#pragma GCC diagnostic ignored "-Warray-bounds"

uInt8 myWriteEnabled        ;
uInt8 myDataHoldRegister    ;
uInt8 myWritePending        ;
uInt8 bPossibleLoad         ;    

// The 6K of RAM and 2K of ROM contained in the Supercharger
uInt8 *myImageAR            ;      // Pointer to the start of the 8K buffer for RAM + ROM    
uInt8 *myImageAR0           ;      // Pointer to the lower bank of 2K
uInt8 *myImageAR1           ;      // Pointer to the upper bank of 2K

uInt8 bWriteOrLoadPossibleAR  = 0;

CartridgeAR *myAR;   

uInt8 LastConfigurationAR = 255;

// The 256 byte header for the current 8448 byte load
uInt8 myHeader[256];

// All of the 8448 byte loads associated with the game 
uInt8* myLoadImages; 

// Indicates how many 8448 loads there are
uInt8 myNumberOfLoadImages;
  
extern uInt8 bWriteOrLoadPossibleAR;

static uInt8 dummyROMCode[] = {
  0xa5, 0xfa, 0x85, 0x80, 0x4c, 0x18, 0xf8, 0xff,
  0xff, 0xff, 0x78, 0xd8, 0xa0, 0x00, 0xa2, 0x00,
  0x94, 0x00, 0xe8, 0xd0, 0xfb, 0x4c, 0x50, 0xf8,
  0xa2, 0x00, 0xbd, 0x06, 0xf0, 0xad, 0xf8, 0xff,
  0xa2, 0x00, 0xad, 0x00, 0xf0, 0xea, 0xbd, 0x00,
  0xf7, 0xca, 0xd0, 0xf6, 0x4c, 0x50, 0xf8, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xa2, 0x03, 0xbc, 0x22, 0xf9, 0x94, 0xfa, 0xca,
  0x10, 0xf8, 0xa0, 0x00, 0xa2, 0x28, 0x94, 0x04,
  0xca, 0x10, 0xfb, 0xa2, 0x1c, 0x94, 0x81, 0xca,
  0x10, 0xfb, 0xa9, 0xff, 0xc9, 0x00, 0xd0, 0x03,
  0x4c, 0x13, 0xf9, 0xa9, 0x00, 0x85, 0x1b, 0x85,
  0x1c, 0x85, 0x1d, 0x85, 0x1e, 0x85, 0x1f, 0x85,
  0x19, 0x85, 0x1a, 0x85, 0x08, 0x85, 0x01, 0xa9,
  0x10, 0x85, 0x21, 0x85, 0x02, 0xa2, 0x07, 0xca,
  0xca, 0xd0, 0xfd, 0xa9, 0x00, 0x85, 0x20, 0x85,
  0x10, 0x85, 0x11, 0x85, 0x02, 0x85, 0x2a, 0xa9,
  0x05, 0x85, 0x0a, 0xa9, 0xff, 0x85, 0x0d, 0x85,
  0x0e, 0x85, 0x0f, 0x85, 0x84, 0x85, 0x85, 0xa9,
  0xf0, 0x85, 0x83, 0xa9, 0x74, 0x85, 0x09, 0xa9,
  0x0c, 0x85, 0x15, 0xa9, 0x1f, 0x85, 0x17, 0x85,
  0x82, 0xa9, 0x07, 0x85, 0x19, 0xa2, 0x08, 0xa0,
  0x00, 0x85, 0x02, 0x88, 0xd0, 0xfb, 0x85, 0x02,
  0x85, 0x02, 0xa9, 0x02, 0x85, 0x02, 0x85, 0x00,
  0x85, 0x02, 0x85, 0x02, 0x85, 0x02, 0xa9, 0x00,
  0x85, 0x00, 0xca, 0x10, 0xe4, 0x06, 0x83, 0x66,
  0x84, 0x26, 0x85, 0xa5, 0x83, 0x85, 0x0d, 0xa5,
  0x84, 0x85, 0x0e, 0xa5, 0x85, 0x85, 0x0f, 0xa6,
  0x82, 0xca, 0x86, 0x82, 0x86, 0x17, 0xe0, 0x0a,
  0xd0, 0xc3, 0xa9, 0x02, 0x85, 0x01, 0xa2, 0x1c,
  0xa0, 0x00, 0x84, 0x19, 0x84, 0x09, 0x94, 0x81,
  0xca, 0x10, 0xfb, 0xa6, 0x80, 0xdd, 0x00, 0xf0,
  0xa9, 0x9a, 0xa2, 0xff, 0xa0, 0x00, 0x9a, 0x4c,
  0xfa, 0x00, 0xcd, 0xf8, 0xff, 0x4c
  };
  
static const uInt8 defaultHeader[256] = {
  0xac, 0xfa, 0x0f, 0x18, 0x62, 0x00, 0x24, 0x02,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
  0x01, 0x05, 0x09, 0x0d, 0x11, 0x15, 0x19, 0x1d,
  0x02, 0x06, 0x0a, 0x0e, 0x12, 0x16, 0x1a, 0x1e,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeAR::CartridgeAR(const uInt8* image, uInt32 size)
{
  // Add header if image doesn't include it
  if(size < 8448)
  {
      size = 8448;
      memcpy((uInt8*)image+8192, defaultHeader, 256);
  }
    
  // Reuse the image buffer
  myLoadImages = (uInt8 *)image;
  myNumberOfLoadImages = size / 8448;

  myImageAR = (uInt8*)fast_cart_buffer; // Set this to the fast internal RAM... that buffer is otherwise unused at this point... Enough to handle 6k of Supercharger RAM
  myImageAR0 = myImageAR;
  myImageAR1 = myImageAR - 2048;
  bPossibleLoad=1;
  bWriteOrLoadPossibleAR = 1;
        
  // Initialize SC BIOS ROM
  initializeROM();
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeAR::~CartridgeAR()
{
    LastConfigurationAR = 255;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeAR::name() const
{
  return "AR";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::reset()
{
  // Initialize RAM to Zeros
  memset(myImageAR, 0x00, 6*1024);

  myWriteEnabled = false;

  myDataHoldRegister = 0;
  myWritePending = false;
  bPossibleLoad = 1;
  bWriteOrLoadPossibleAR=1;
  NumberOfDistinctAccesses=0;

  // Set bank configuration upon reset so ROM is selected and powered up
  bankConfiguration(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::systemCyclesReset()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  for(uInt32 i = 0x1000; i < 0x2000; i += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(i >> shift, page_access);
  }
    
  bankConfiguration(0);
  myAR = this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::peek(uInt16 addr)
{
    // Not used... all AR peek handling done directly in 6502-Low for speed
    return 0x00;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::poke(uInt16 addr, uInt8)
{
    // Not used... all AR poke handling done directly in 6502-Low for speed
}

void SetConfigurationAR(uInt8 configuration)
{
  // D7-D5 of this byte: Write Pulse Delay (n/a for emulator)
  //
  // D4-D0: RAM/ROM configuration:
  //       $F000-F7FF    $F800-FFFF Address range that banks map into
  //  000wp     2            ROM
  //  001wp     0            ROM
  //  010wp     2            0      as used in Commie Mutants and many others
  //  011wp     0            2      as used in Suicide Mission
  //  100wp     2            ROM
  //  101wp     1            ROM
  //  110wp     2            1      as used in Killer Satellites
  //  111wp     1            2      as we use for 2k/4k ROM cloning
  // 
  //  w = Write Enable (1 = enabled; accesses to $F000-$F0FF cause writes
  //    to happen.  0 = disabled, and the cart acts like ROM.)
  //  p = ROM Power (0 = enabled, 1 = off.)  Only power the ROM if you're
  //    wanting to access the ROM for multiloads.  Otherwise set to 1.
  
  LastConfigurationAR = configuration;
   
  myWriteEnabled = configuration & 0x02;
  
  switch((configuration >> 2) & 0x07)
  {
    case 0:
    {
      bPossibleLoad = 1;
      bWriteOrLoadPossibleAR = 1;
      myImageAR0 = fast_cart_buffer + (2 * 2048);
      myImageAR1 = fast_cart_buffer + (2 * 2048);  // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 1:
    {
      bPossibleLoad = 1;
      bWriteOrLoadPossibleAR = 1;
      myImageAR0 = fast_cart_buffer + (0 * 2048);
      myImageAR1 = fast_cart_buffer + (2 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 2:
    {
      bPossibleLoad = 0;
      bWriteOrLoadPossibleAR = myWritePending;
      myImageAR0 = fast_cart_buffer + (2 * 2048);
      myImageAR1 = fast_cart_buffer + (-1 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 3:
    {
      bPossibleLoad = 0;
      bWriteOrLoadPossibleAR = myWritePending;
      myImageAR0 = fast_cart_buffer + (0 * 2048);
      myImageAR1 = fast_cart_buffer + (1 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 4:
    {
      bPossibleLoad = 1;
      bWriteOrLoadPossibleAR = 1;
      myImageAR0 = fast_cart_buffer + (2 * 2048);
      myImageAR1 = fast_cart_buffer + (2 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 5:
    {
      bPossibleLoad = 1;
      bWriteOrLoadPossibleAR = 1;
      myImageAR0 = fast_cart_buffer + (1 * 2048);
      myImageAR1 = fast_cart_buffer + (2 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 6:
    {
      bPossibleLoad = 0;
      bWriteOrLoadPossibleAR = myWritePending;
      myImageAR0 = fast_cart_buffer + (2 * 2048);
      myImageAR1 = fast_cart_buffer + (0 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }

    case 7:
    {
      bPossibleLoad = 0;
      bWriteOrLoadPossibleAR = myWritePending;
      myImageAR0 = fast_cart_buffer + (1 * 2048);
      myImageAR1 = fast_cart_buffer + (1 * 2048); // This is 2048 lower so we can index faster in M6502Low::peek_AR()
      //myImageAR1 -= 2048;
      break;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::bankConfiguration(uInt8 configuration)
{
    SetConfigurationAR(configuration);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::initializeROM(void)
{
  //   0xFF -> do a complete jump over the SC BIOS progress bars code
  //   0x00 -> show SC BIOS progress bars as normal
  dummyROMCode[109] = 0x00;
  
  // The accumulator should contain a random value after exiting the
  // SC BIOS code - a value placed in offset 281 will be stored in A
  Random random;
  dummyROMCode[281] = random.next();

  uInt32 size = sizeof(dummyROMCode);

  // Initialize ROM with illegal 6502 opcode that causes a real 6502 to jam
  for(uInt32 i = 0; i < 2048; ++i)
  {
    myImageAR[3 * 2048 + i] = 0x02; 
  }

  // Copy the "dummy" Supercharger BIOS code into the ROM area
  for(uInt32 j = 0; j < size; ++j)
  {
    myImageAR[3 * 2048 + j] = dummyROMCode[j];
  }

  // Finally set 6502 vectors to point to initial load code at 0xF80A of BIOS
  myImageAR[3 * 2048 + 2044] = 0x0A;
  myImageAR[3 * 2048 + 2045] = 0xF8;
  myImageAR[3 * 2048 + 2046] = 0x0A;
  myImageAR[3 * 2048 + 2047] = 0xF8;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::checksum(uInt8* s, uInt16 length)
{
  uInt8 sum = 0;

  for(uInt32 i = 0; i < length; ++i)
  {
    sum += s[i];
  }

  return sum;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::loadIntoRAM(uInt8 load)
{
  uInt8 image;

  // Scan through all of the loads to see if we find the one we're looking for
  for(image = 0; image < myNumberOfLoadImages; ++image)
  {
    // Is this the correct load?
    if(myLoadImages[(image * 8448) + 8192 + 5] == load)
    {
      // Copy the load's header
        uInt32 *dest = (uInt32 *) (myHeader);
        uInt32 *src2 = (uInt32 *) (myLoadImages + (image * 8448) + 8192);
        for (int i=0; i<64; i++)
        {
           *dest++ = *src2++;   
        }     

      // Load all of the pages from the load
      for(uInt32 j = 0; j < myHeader[3]; ++j)
      {
        uInt32 bank = myHeader[16 + j] & 0x03;
        uInt32 page = (myHeader[16 + j] >> 2) & 0x07;
        uInt8* src = myLoadImages + (image * 8448) + (j * 256);
        // Copy page to Supercharger RAM (don't allow a copy into ROM area)
        if(bank < 3)
        {
            uInt32 *dest = (uInt32 *) (myImageAR + (bank * 2048) + (page * 256));
            uInt32 *src2 = (uInt32 *) src;
            for (int i=0; i<64; i++)
            {
               *dest++ = *src2++;   
            }
        }
      }

      // Copy the bank switching byte and starting address into the 2600's
      // RAM for the "dummy" SC BIOS to access it
      mySystem->poke(0xfe, myHeader[0]);
      mySystem->poke(0xff, myHeader[1]);
      mySystem->poke(0x80, myHeader[2]);

      return;
    }
  }
}
