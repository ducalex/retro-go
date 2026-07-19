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
#include <string.h>
#include "CartCTY.hxx"
#include "Random.hxx"
#include "System.hxx"
#include "../StellaDS.h"
#include <iostream>

#define CTY_RAM_SIZE    256
#define CTY_EE_SIZE     256

extern char my_filename[MAX_FILE_NAME_LEN+1];
extern char flash_filename[MAX_FILE_NAME_LEN+5];
uInt32 myRandomNumberCTY = 0x2B435044;
uInt16 myTunePosition   ;
uInt32 myAudioCycles     = 0;
uInt32 deltaCyclesX10    = 0;
const uInt8 *myFrequencyImage = NULL;
uInt8 myOperationType;
extern uInt32 myMusicCounters[3];
extern uInt32 myMusicFrequencies[3];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCTY::CartridgeCTY(const uInt8* image, uInt32 size)
{
  // Copy the ROM image into my buffer - just reuse the existing buffer
  myImage = (uInt8 *)image;

  // If we are the older non-music ROM, we just blank that stuff out... 
  if (size <= (32 * 1024)) 
  {
      memset(myImage+(32*1024), 0x00, 32*1024);
  }  
  memset(myMusicCounters, 0x00, sizeof(myMusicCounters));
  memset(myMusicFrequencies, 0x00, sizeof(myMusicFrequencies));  
  
  myFrequencyImage = fast_cart_buffer + 1024;
  ctyRAM = fast_cart_buffer + 0;
  ctyEE  = fast_cart_buffer + 256;

  // Initialize RAM with random values
  Random random;
  for(uInt32 i = 0; i < 256; ++i)
  {
    ctyRAM[i] = random.next();
  }
    
  // Save the Flash backing filename for the extra RAM
  strncpy(flash_filename,my_filename, MAX_FILE_NAME_LEN);
  flash_filename[MAX_FILE_NAME_LEN] = 0;
  strcat(flash_filename, ".fla");
}
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCTY::~CartridgeCTY()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeCTY::name() const
{
  return "CTY";
}

const uInt32 ourFrequencyTable[] =
{
  // this should really be referenced from within the ROM, but its part of
  // the Harmony/Melody CTY Driver, which does not appear to be in the ROM.

     0,           // CONT   0   Continue Note
     0,           // REPEAT 1   Repeat Song
     0,           // REST   2   Note Rest
     11811160,    // A2
     12513387,    // A2s
     13257490,    // B2
     14045832,    // C2
     14881203,    // C2s
     15765966,    // D2
     16703557,    // D2s
     17696768,    // E2
     18749035,    // F2
     19864009,    // F2s
     21045125,    // G2
     22296464,    // G2s

     23622320,    // A3
     25026989,    // A3s
     26515195,    // B3
     28091878,    // C3
     29762191,    // C3s
     31531932,    // D3
     33406900,    // D3s
     35393537,    // E3
     37498071,    // F3
     39727803,    // F3s
     42090250,    // G3
     44592927,    // G3s

     47244640,    // A4
     50053978,    // A4s
     53030391,    // B4
     56183756,    // C4   (Middle C)
     59524596,    // C4s
     63064079,    // D4
     66814014,    // D4s
     70787074,    // E4
     74996142,    // F4
     79455606,    // F4s
     84180285,    // G4
     89186069,    // G4s

     94489281,    // A5
     100107957,   // A5s
     106060567,   // B5
     112367297,   // C5
     119048977,   // C5s
     126128157,   // D5
     133628029,   // D5s
     141573933,   // E5
     149992288,   // F5
     158911428,   // F5s
     168360785,   // G5
     178371925,   // G5s

     188978561,   // A6
     200215913,   // A6s
     212121348,   // B6
     224734593,   // C6
     238098169,   // C6s
     252256099,   // D6
     267256058,   // D6s
     283147866,   // E6
     299984783,   // F6
     317822855,   // F6s
     336721571,   // G6
     356744064    // G6s
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCTY::reset()
{
  ctyRAM[0] = ctyRAM[1] = ctyRAM[2] = ctyRAM[3] = 0xFF;

  myRandomNumberCTY = 0x2B435044;
  myAudioCycles = gSystemCycles;
  deltaCyclesX10 = 0;
  
  // Upon reset we switch to bank 1
  myCurrentBank = 99;
  bank(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCTY::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  
  // Everything calls into this handler until bank() is called below
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
  {
    page_access.directPeekBase = 0;
    page_access.directPokeBase = 0;
    page_access.device = this;
    mySystem->setPageAccess(j >> shift, page_access);
  }
 
  // Install pages for bank 1 
  myCurrentBank = 99;
  bank(1);
}

void CartridgeCTY::systemCyclesReset()
{
  // Adjust the cycle counter so that it reflects the new value
  myAudioCycles = 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CTY_updateMusicModeDataFetchers(void)
{
  // Calculate the number of cycles since the last update
  uInt32 cycles = gSystemCycles - myAudioCycles;
  myAudioCycles = gSystemCycles;

  deltaCyclesX10 += (cycles*10);
  // Calculate the number of CTY OSC clocks since the last update
  uInt32 clocks   = (deltaCyclesX10 / 597);
  deltaCyclesX10  = (deltaCyclesX10 % 597);

  // Let's update counters and flags of the music mode data fetchers
  if(clocks)
  {
    for(int x = 0; x <= 2; ++x)
    {
      myMusicCounters[x] += myMusicFrequencies[x] * clocks;
    }
  }
  
  return (((myMusicCounters[0] >> 31) + (myMusicCounters[1] >> 31) + (myMusicCounters[2] >> 31)) << 2);  
}

void CartridgeCTY::updateTune(void)
{
  myTunePosition += 1;
  const uInt16 songPosition = (myTunePosition - 1) *3;

  uInt8 note = myFrequencyImage[songPosition + 0];
  if (note)
    myMusicFrequencies[0] = ourFrequencyTable[note];

  note = myFrequencyImage[songPosition + 1];
  if (note)
    myMusicFrequencies[1] = ourFrequencyTable[note];

  note = myFrequencyImage[songPosition + 2];
  if (note == 1)
    myTunePosition = 0;
  else
    myMusicFrequencies[2] = ourFrequencyTable[note];
}



void CartridgeCTY::handle_cty_flash_backing(void)
{
    u8 op = myOperationType & 0x0F;
    u8 index = (myOperationType >> 4) & 0x0F;

    switch (op)
    {
        case 2: // Load Score Table
        {
            FILE *fp = fopen(flash_filename, "rb");
            if (fp == NULL)
                memset(ctyEE, 0x00, CTY_EE_SIZE);
            else
                fread(ctyEE, CTY_EE_SIZE, 1, fp);
            
            // Grab 60B slice @ given index (first 4 bytes are ignored)
            memcpy(ctyRAM+4, ctyEE + (index << 6) + 4, 60);
            
            fclose(fp);
        }
        break;
        
        case 3: // Write Score Table
        {
            // Add 60B RAM to score table @ given index (first 4 bytes are ignored)
            memcpy(ctyEE + (index << 6) + 4, ctyRAM+4, 60);

            FILE *fp = fopen(flash_filename, "wb+");
            fwrite(ctyEE, CTY_EE_SIZE, 1, fp);
            fclose(fp);
        }
        break;
            
        case 4: // Wipe Score Table
        {
            memset(ctyEE, 0x00, CTY_EE_SIZE);
            FILE *fp = fopen(flash_filename, "wb+");
            fwrite(ctyEE, CTY_EE_SIZE, 1, fp);
            fclose(fp);
        }
        break;
    }
    
    ctyRAM[0] = 0;   // Mark this operation as GOOD
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeCTY::peek(uInt16 address)
{
  address = address & 0x0FFF;
  
  if (address & 0xFF80)
  {
      if (address >= 0x0FF5 && address <= 0x0FFB)  // Switch banks if necessary
      {
          bank(address - 0x0FF4);
      }
      else if (address == 0x0FF4)   // Read or Write the 256 bytes of extra RAM to flash
      {
          return ramReadWrite();
      }
      return cart_buffer[myCurrentOffset + address];
  }  
  else
  {
    address &= 0x3F;
    switch(address)
    {
      case 0x00:  // Error code after operation
        return ctyRAM[0];
      case 0x01:  // Get next Random Number (8-bit LFSR)
        myRandomNumberCTY = ((myRandomNumberCTY & (1<<10)) ? 0x10adab1e: 0x00) ^ ((myRandomNumberCTY >> 11) | (myRandomNumberCTY << 21));
        return myRandomNumberCTY & 0xFF;
      case 0x02:  // Get Tune position (low byte)
        return myTunePosition & 0xFF;
      case 0x03:  // Get Tune position (high byte)
        return (myTunePosition >> 8) & 0xFF;
      default:
        return ctyRAM[address];
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCTY::poke(uInt16 address, uInt8 value)
{
  address = address & 0x03F;
  switch(address)
  {
      case 0x00:  // Operation type for $1FF4
        myOperationType = value;
        break;
      case 0x01:  // Set Random seed value (reset)
        myRandomNumberCTY = 0x2B435044;
        break;
      case 0x02:  // Reset fetcher to beginning of tune
        myTunePosition = 0;
        myMusicCounters[0] = 0;
        myMusicCounters[1] = 0;
        myMusicCounters[2] = 0;
        myMusicFrequencies[0] = 0;
        myMusicFrequencies[1] = 0;
        myMusicFrequencies[2] = 0;
        break;
      case 0x03:  // Advance fetcher to next tune position
        updateTune();
        break;
      default:
        ctyRAM[address] = value;
        break;
  }
  return;
}

uInt8 CartridgeCTY::ramReadWrite(void)
{
    // Opcode and value in form of XXXXYYYY (from myOperationType), where:
    //    XXXX = index and YYYY = operation
    const uInt8 index = myOperationType >> 4;
    switch(myOperationType & 0xf)
    {
      case 1:  // Load tune (index = tune)
        if(index < 7)
        {
          memcpy ((void *)myFrequencyImage, (((uInt8 *)cart_buffer + (32*1024)) + (index << 12)), 4096);
          // Reset to beginning of tune
          myTunePosition = 0;
        }
        break;
      case 2:  // Load score table (index = table)
      case 3:  // Save score table (index = table)
      case 4:  // Wipe all score tables
        handle_cty_flash_backing();
        break;

      default:  // satisfy compiler
        break;
    }
    
    ctyRAM[0] = 0; // Successful operation
    return myImage[myCurrentOffset + 0xFF4] & ~0x40; // Bit 6 is 0, ready/success
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCTY::bank(uInt16 bank)
{
  myCurrentOffset = bank * 4096;
  
  // Setup the page access methods for the current bank
  page_access.device = this;
  page_access.directPokeBase = 0;

  // Setup the page access methods for the current bank
  uInt32 access_num = 0x1080 >> MY_PAGE_SHIFT;

  // Map ROM image into the system
  for(uInt32 address = 0x0080; address < 0x0F80; address += (1 << MY_PAGE_SHIFT))
  {
      page_access.directPeekBase = &cart_buffer[myCurrentOffset + address];
      mySystem->setPageAccess(access_num++, page_access);
  }  
}
