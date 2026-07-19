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
// Copyright (c) 1995-2012 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================
#include <nds.h>
#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartCDF.hxx"
#include "Thumbulator.hxx"

#define FAST_FETCH_ON ((myMode & 0x0F) == 0)
#define DIGITAL_AUDIO_ON ((myMode & 0xF0) == 0)
#define DIGITAL_AUDIO_OFF (myMode & 0xF0)

static uInt16 myStartBank = 6;

#define MEM_2KB  (1024 * 2)
#define MEM_3KB  (1024 * 3)
#define MEM_4KB  (1024 * 4)
#define MEM_5KB  (1024 * 5)
#define MEM_8KB  (1024 * 8)
#define MEM_24KB (1024 * 24)
#define MEM_28KB (1024 * 28)
#define MEM_32KB (1024 * 32)

extern Thumbulator *myThumbEmulator;
uInt8 myDataStreamFetch      = 0x00;

// The counter registers for the data fetchers
uInt8* myDisplayImageCDF    ;  // Pointer to the 4K display image of the cartridge

uInt8* myFetcherOffsetPtr   ;  // Pointer to the RAM cell that contains the offset

uInt16 myMusicWaveformSize[3]  = {0,0,0};

uInt32 fastDataStreamBase  ;
uInt32 *commPtr32          ;
uInt32 fastIncStreamBase   ;
uInt32 *myWaveformBasePtr  ;

CartridgeCDF *myCartCDF    ;

// System cycle count when the last update to music data fetchers occurred

uInt16 myAmplitudeStream   = 0x22;

uInt8 peekvalue  = 0;

// These two don't need to be in fast memory as we will use a pointer to each anyway...
static uInt16 myDatastreamBase          = 0x00a0;
static uInt16 myDatastreamIncrementBase = 0x0128;

// Pointer to the 32K program ROM image of the cartridge
extern uInt8 myARM6502[];
extern uInt8 *myDPCptr;
extern Int32 myDPCPCycles;

extern uInt32 myMusicCounters[3];
extern uInt32 myMusicFrequencies[3];

extern uInt16 f8_bankbit;

// Controls mode, lower nybble sets Fast Fetch, upper nybble sets audio
// -0 = Fast Fetch ON
// -F = Fast Fetch OFF
// 0- = Packed Digital Sample
// F- = 3 Voice Music
uInt8 myMode                 = 0xFF;

bool  isCDFJPlus             = 0;

u8 myLDXenabled              = 0;
u8 myLDYenabled              = 0;
uInt16 myFastFetcherOffset   = 0;
uInt8 subversion = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// params:
//  - searchValue: uInt32 value to search for; assumes it is on a DWORD boundary
//
// returns:
//  - offset in image where value was found
//  - 0xFFFFFFFF if not found
uInt32 CartridgeCDF::scanCDFDriver(uInt32 searchValue)
{
  for (uInt32 i = 0; i < 2048; i += 4)
    if (getUInt32(cart_buffer, i) == searchValue)
      return i;

  return 0xFFFFFFFF;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCDF::CartridgeCDF(const uInt8* image, uInt32 size)
{
  uInt16 myWaveformBase;
    
  // isCDFJPlus is already set by the Cart auto-detect
  subversion = 0;

  // get offset of CDFJPlus ID
  uInt32 cdfjOffset = 0;

  // These need to be proven below...
  myFastFetcherOffset = 0;
  myLDXenabled = false;
  myLDYenabled = false;
    
  if ((cdfjOffset = scanCDFDriver(0x53554c50)) != 0xFFFFFFFF && // Plus
      getUInt32(image, cdfjOffset+4) == 0x4a464443 &&   // CDFJ
      getUInt32(image, cdfjOffset+8) == 0x00000001)     // V1
  {
    myAmplitudeStream = 0x23;
    myDatastreamBase = 0x0098;
    myDatastreamIncrementBase = 0x0124;
    myWaveformBase = 0x01b0;

    for (int i = 0; i < 2048; i += 4)
    {
      const uInt32 cdfjValue = getUInt32(image, i);
      if (cdfjValue == 0x135200A2)
        myLDXenabled = true;
      if (cdfjValue == 0x135200A0)
        myLDYenabled = true;
        
      // search for Fast Fetcher Offset (default is 0)
      if ((cdfjValue & 0xFFFFFF00) == 0xE2422000)
      {
        myFastFetcherOffset = i;
      }
      
      cBase = getUInt32(image, 0x17F8) & 0xFFFFFFFE;    // C Base Address
      cStart = cBase;                                   // C Start Address
      cStack = getUInt32(image, 0x17F4);                // C Stack        
    }
  }
  else
  {
      cStack = 0x40001fb4;
      cBase  = 0x00000800;
      cStart = 0x00000808;
      
      for(uInt32 i = 0; i < 2048; i += 4)
      {
        // CDF signature occurs 3 times in a row, i+3 (+7 or +11) is version
        if (    image[i+0] == 0x43 && image[i + 4] == 0x43 && image[i + 8] == 0x43) // C
          if (  image[i+1] == 0x44 && image[i + 5] == 0x44 && image[i + 9] == 0x44) // D
            if (image[i+2] == 0x46 && image[i + 6] == 0x46 && image[i +10] == 0x46) // F
            {
              subversion = image[i+3];
              break;
            }
      }

      switch (subversion) {
        case 0x4a: //CDFJ
          myAmplitudeStream = 0x23;
          myDatastreamBase = 0x0098;
          myDatastreamIncrementBase = 0x0124;
          myWaveformBase = 0x01b0;
          break;

        case 0: //V0
          myAmplitudeStream = 0x22;
          myDatastreamBase = 0x06e0;
          myDatastreamIncrementBase = 0x0768;
          myWaveformBase = 0x07f0;
          break;

        default: //V1
          myAmplitudeStream = 0x22;
          myDatastreamBase = 0x00a0;
          myDatastreamIncrementBase = 0x0128;
          myWaveformBase = 0x01b0;
          break;
     }
  }
    
  // ------------------------------------------------------------------------
  // Determine which RAM to use... for 32K/8K CDFJ+ or lower we can use the
  // 8K of Fast RAM as our ARM RAM. Otherwise we have to use the slower 32K
  // but we will repurpose the fast_cart_buffer[] as 6502 Assembly code 
  // assuming that those games won't need more than 2 banks of 6502 assembly.
  // ------------------------------------------------------------------------
  if ((isCDFJPlus && (size > MEM_32KB)) || (cartDriver == 11))
  {
      bSaveStateXL = true;
      myARMRAM = xl_ram_buffer;                   // Set to the slow but larger 32K RAM buffer
      memset(myARMRAM, 0, MEM_32KB);              // Clear all of the "ARM Thumb" RAM
  }
  else
  {
      bSaveStateXL = false;
      myARMRAM = (uInt8*)fast_cart_buffer;      // Set to the fast RAM buffer
      memset(myARMRAM, 0, MEM_8KB);             // Clear all of the "ARM Thumb" RAM
  }
    
  // Pointer to where the offset is located for DPCJ+ games utilizing a fast-fetcher offset
  extern uInt32 myDPCPRandomNumber;
  myDPCPRandomNumber = 0x00;
  if (myFastFetcherOffset)
    myFetcherOffsetPtr = &myARMRAM[myFastFetcherOffset];    
  else
    myFetcherOffsetPtr = (uInt8*)&myDPCPRandomNumber;   // Some fast memory from another driver we repurpose
   
  // Pointer to the waveform base in RAM
  myWaveformBasePtr = (uInt32*)&myARMRAM[myWaveformBase];
    
  // Pointer to the program ROM (28K @ 4K or 2K offset)
  myDPCptr = (uInt8 *)image + (isCDFJPlus ? MEM_2KB : MEM_4KB);
  memset(myARM6502, 0xFF, MEM_32KB);
  memcpy(myARM6502, myDPCptr, MEM_28KB); // For the 6502, we only need to copy 28K max
 
  // Repurpose the fast cart buffer memory for 6502 code. This is potentially
  // unsafe as it only supports 2 banks... but there are only a couple of 
  // games in this stratosphere and none use more than 2 banks of 6502 code.
  if ((isCDFJPlus && (size > MEM_32KB)) || (cartDriver == 11))
  {
      memcpy(fast_cart_buffer, myARM6502, MEM_8KB);
  }
    
  // Copy intial driver into the CDF Harmony RAM
  memcpy(myARMRAM, image, MEM_2KB);

  // Pointer to the display RAM
  myDisplayImageCDF = myARMRAM + MEM_2KB;
    
  // Set the initial Music Waveform sizes and counters
  for (int i=0; i < 3; ++i)
  {
    myMusicWaveformSize[i] = 27;
    myMusicCounters[i] = 0;
    myMusicFrequencies[i] = 0;
  }
    
  // CDF/CDFJ always starts in bank 6, CDFJ+ in bank 0
  myStartBank = (isCDFJPlus ? 0:6);
  bank(myStartBank);
    
  fastDataStreamBase = (uInt32)&myARMRAM[myDatastreamBase];
  fastIncStreamBase = (uInt32)&myARMRAM[myDatastreamIncrementBase];
    
  commPtr32 = (uInt32*) ((uInt32)fastDataStreamBase + (COMMSTREAM << 2));
    
  myMode = 0xFF;
  myDataStreamFetch = 0x00;
    
  // Create Thumbulator ARM emulator
  myThumbEmulator = new Thumbulator((uInt16*)image); // The Thumbulator gets the full image no matter how big...
  
  myCartCDF = this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCDF::~CartridgeCDF()
{
   if (myThumbEmulator) delete myThumbEmulator;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeCDF::name() const
{
  return (isCDFJPlus ? "CDFJ+": (subversion==0x4a ? "CDFJ":"CDF"));
}    

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCDF::reset()
{
  // Upon reset we switch to the startup bank
  myDPCPCycles = mySystem->cycles();
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCDF::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = mySystem->cycles();

  // Adjust the cycle counter so that it reflects the new value
  myDPCPCycles -= cycles;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCDF::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  // Set the page accessing methods for the hot spots
  PageAccess access;
  access.directPeekBase = 0;
  access.directPokeBase = 0;
  access.device = this;

  // Set page accessing method for the DPC reading & writing pages
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
  {
    mySystem->setPageAccess(j >> shift, access);
  }

  // Install pages for the startup bank
  bank(myStartBank);
    
  f8_bankbit = 0x0FFF;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeCDF::callFunction(uInt8 value)
{
  // myParameter
  switch (value)
  {
    case 254:
    case 255:
      // Call user written ARM code (most likely be C compiled for ARM)
      myThumbEmulator->run();
      break;
  }
}


 uInt32 CDFCallback(uInt8 function, uInt32 value1, uInt32 value2)
{
  switch (function)
  {
    case 0:
      // _SetNote - set the note/frequency
      myMusicFrequencies[value1] = value2;
      break;

      // _ResetWave - reset counter,
      // used to make sure digital samples start from the beginning
    case 1:
      myMusicCounters[value1] = 0;
      break;

      // _GetWavePtr - return the counter
    case 2:
        return myMusicCounters[value1];
          
      // _SetWaveSize - set size of waveform buffer
    case 3:
      myMusicWaveformSize[value1] = value2;
      break;
  }

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt32 CartridgeCDF::getWaveform(uInt8 index) const
{
  uInt32 result = myWaveformBasePtr[index];
  result = (result - MEM_2KB) & 0x3FFF;
  return result;
}

 uInt8 CartridgeCDF::peekMusic(void)
{
  if (DIGITAL_AUDIO_OFF)
  {
      uInt32 cyclesPassed = (gSystemCycles - myDPCPCycles);
      if (cyclesPassed >= 60)
      {
          myMusicCounters[0] += myMusicFrequencies[0] * (cyclesPassed/60);
          myMusicCounters[1] += myMusicFrequencies[1] * (cyclesPassed/60);
          myMusicCounters[2] += myMusicFrequencies[2] * (cyclesPassed/60);
          myDPCPCycles = (gSystemCycles - (cyclesPassed % 60));
          peekvalue = myDisplayImageCDF[getWaveform(0) + (myMusicCounters[0] >> myMusicWaveformSize[0])]
                    + myDisplayImageCDF[getWaveform(1) + (myMusicCounters[1] >> myMusicWaveformSize[1])]
                    + myDisplayImageCDF[getWaveform(2) + (myMusicCounters[2] >> myMusicWaveformSize[2])];
      }
  }
  else
  {
      uInt32 cyclesPassed = (gSystemCycles - myDPCPCycles);
      if (cyclesPassed >= 60)
      {
          myMusicCounters[0] += myMusicFrequencies[0] * (cyclesPassed/60);
          myDPCPCycles = (gSystemCycles - (cyclesPassed % 60));
          
          // retrieve packed sample (max size is 2K, or 4K of unpacked data)
          const uInt32 sampleaddress = *myWaveformBasePtr + (myMusicCounters[0] >> (isCDFJPlus ? 13:21));

          if (sampleaddress & 0xF0000000) // check for RAM
            peekvalue = myARMRAM[sampleaddress & RAMADDMASK];
          else 
            peekvalue = cart_buffer[sampleaddress];

          // make sure current volume value is in the lower nybble
          if ((myMusicCounters[0] & (1<<(isCDFJPlus ? 12:20))) == 0)
          {
            peekvalue >>= 4;
          }
          else peekvalue &= 0x0f;
      }
  }
  return peekvalue;   
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeCDF::peek(uInt16 address)  
{
   address &= 0xFFF;
   switch (address)
   {
        case 0x0FF4:  myDPCptr = &myARM6502[isCDFJPlus ? 0x0000:0x6000]; f8_bankbit = 0x0FFF; break;
        case 0x0FF5:  myDPCptr = &myARM6502[isCDFJPlus ? 0x1000:0x0000]; f8_bankbit = 0x1FFF;break;
        case 0x0FF6:  myDPCptr = &myARM6502[isCDFJPlus ? 0x2000:0x1000]; break;
        case 0x0FF7:  myDPCptr = &myARM6502[isCDFJPlus ? 0x3000:0x2000]; break;
        case 0x0FF8:  myDPCptr = &myARM6502[isCDFJPlus ? 0x4000:0x3000]; break;
        case 0x0FF9:  myDPCptr = &myARM6502[isCDFJPlus ? 0x5000:0x4000]; break;
        case 0x0FFA:  myDPCptr = &myARM6502[isCDFJPlus ? 0x6000:0x5000]; break;
        case 0x0FFB:  myDPCptr = &myARM6502[isCDFJPlus ? 0x0000:0x6000]; break;
    }
    return myDPCptr[address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCDF::bank(uInt16 bank)
{
  // Remember what bank we're in
  myDPCptr = &myARM6502[bank << 12];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 void CartridgeCDF::poke(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  switch(address)
  {
    case 0xFF0:   // DSWRITE
      if (isCDFJPlus)
      {
          myDisplayImageCDF[ *commPtr32 >> 16 ] = value;
          *commPtr32  += 0x10000;  // always increment by 1 when writing
      }
      else          
      {
          myDisplayImageCDF[ *commPtr32 >> 20 ] = value;
          *commPtr32  += 0x100000;  // always increment by 1 when writing
      }
      break;

    case 0xFF1:   // DSPTR
      *commPtr32 <<=8;
      if (isCDFJPlus)
      {
          *commPtr32 &= 0xff000000;
          *commPtr32 |= (value << 16);
      }
      else
      {
          *commPtr32 &= 0xf0000000;
          *commPtr32 |= (value << 20);
      }
      break;

    case 0xFF2:   // SETMODE
      myMode = value;
      if (myMode & 0x0F) myDataStreamFetch = 0x00; else myDataStreamFetch = (myAmplitudeStream+1);
      break;

    case 0xFF3:   // CALLFN
      callFunction(value);
      break;
          
    // Check for bankswitch write...
    case 0xFF4:  myDPCptr = &myARM6502[isCDFJPlus ? 0x0000:0x6000]; f8_bankbit = 0x0FFF; break;
    case 0xFF5:  myDPCptr = &myARM6502[isCDFJPlus ? 0x1000:0x0000]; f8_bankbit = 0x1FFF; break;
    case 0xFF6:  myDPCptr = &myARM6502[isCDFJPlus ? 0x2000:0x1000]; break;
    case 0xFF7:  myDPCptr = &myARM6502[isCDFJPlus ? 0x3000:0x2000]; break;
    case 0xFF8:  myDPCptr = &myARM6502[isCDFJPlus ? 0x4000:0x3000]; break;
    case 0xFF9:  myDPCptr = &myARM6502[isCDFJPlus ? 0x5000:0x4000]; break;
    case 0xFFA:  myDPCptr = &myARM6502[isCDFJPlus ? 0x6000:0x5000]; break;
    case 0xFFB:  myDPCptr = &myARM6502[isCDFJPlus ? 0x0000:0x6000]; break;

    default:
      break;
  }
    
  return;
}

