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

#ifndef CARTRIDGE_DPC_PLUS_HXX
#define CARTRIDGE_DPC_PLUS_HXX


extern bool   myFastFetch;
extern uInt32 myFractionalCounters[8];
extern uInt32 myFractionalIncrements[8];
extern uInt32 myTops[8];
extern uInt32 myTopsMinusBottoms[8];
extern uInt32 myBottoms[8];
extern uInt32 myCounters[8];
extern uInt32 myMusicCounters[3];
extern uInt32 myMusicFrequencies[3];
extern uInt32 myMusicWaveforms[3];
extern uInt32 myMusicCountersShifted[3];
extern uInt32 myDPCPRandomNumber;
extern Int32 myDPCPCycles;
extern uInt8 myParameter[8];
extern uInt8 myParameterPointer;


class System;
#ifdef THUMB_SUPPORT
class Thumbulator;
#endif

#include "bspf.hxx"
#include "Cart.hxx"

/**
  Cartridge class used for DPC+.  There are six 4K program banks, a 4K
  display bank, 1K frequency table and the DPC chip.  For complete details on
  the DPC chip see David P. Crane's United States Patent Number 4,644,495.

  @author  Darrell Spice Jr, Fred Quimby, Stephen Anthony
  @version $Id$
*/
class CartridgeDPCPlus : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
    */
    CartridgeDPCPlus(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~CartridgeDPCPlus();

  public:
    /**
      Reset device to its power-on state
    */
    void reset();

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary
      to override this method for devices that remember cycle counts.
    */
    void systemCyclesReset();

    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system);

    /**
      Install pages for the specified bank in the system.

      @param bank The bank that should be installed in the system
    */
    void bank(uInt16 bank);


    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    virtual const char* name() const;

  public:
    /**
      Get the byte at the specified address.

      @return The byte at the specified address
    */
    virtual uInt8 peek(uInt16 address);
    
    uInt8 peekFetch(uInt8 address);

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
    */
    virtual void poke(uInt16 address, uInt8 value);

  private:
    /** 
      Clocks the random number generator to move it to its next state
    */
    void clockRandomNumberGenerator();
  
    /** 
      Clocks the random number generator to move it to its prior state
    */
    void priorClockRandomNumberGenerator();
    
    /** 
      Call Special Functions
    */
    void callFunction(uInt8 value);

  private:
    // The ROM image and size
    uInt32 mySize;

    // Pointer to the 24K program ROM image of the cartridge
    uInt8* myProgramImage;

    // The DPC 8k RAM image will use fast_cart_buffer[]
    
    // Pointer to the 1K frequency table
    uInt8* myFrequencyImage;

    // Older DPC+ driver code had different behaviour wrt the mask used
    // to retrieve 'DFxFRACLOW' (fractional data pointer low byte)
    // ROMs built with an old DPC+ driver and using the newer mask can
    // result in 'jittering' in the playfield display
    // For current versions, this is 0x0F00FF; older versions need 0x0F0000
    uInt32 myFractionalLowMask{0x0F00FF};
    //uInt32 myFractionalLowMask{0x0F0000};
};

#endif
