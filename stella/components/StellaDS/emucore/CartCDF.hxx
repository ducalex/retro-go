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

#ifndef CARTRIDGE_CDF_HXX
#define CARTRIDGE_CDF_HXX

class System;
#ifdef THUMB_SUPPORT
class Thumbulator;
#endif

#include "bspf.hxx"
#include "Cart.hxx"

#define COMMSTREAM        0x20
#define JUMPSTREAM_BASE   0x21

#define DSRAM             0x800

extern bool   isCDFJPlus;
extern uInt16 myAmplitudeStream;
extern uInt8 myDataStreamFetch;
extern uInt8 peekvalue;
extern Int32 myDPCPCycles;
extern uInt8 myMode;
extern u8 myLDXenabled;
extern u8 myLDYenabled;
extern uInt16 myFastFetcherOffset;
extern uInt16 myMusicWaveformSize[3];

/**
  Cartridge class used for CDF/CDFJ.
*/
class CartridgeCDF : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
    */
    CartridgeCDF(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~CartridgeCDF();

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
    
    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
    */
    virtual void poke(uInt16 address, uInt8 value);

    uInt8 peekMusic(void);

  private:
    /** 
      Call Special Functions
    */
    void callFunction(uInt8 value);
    uInt32 scanCDFDriver(uInt32 searchValue);
    
    uInt32 getWaveform(uInt8 index) const;

  private:
};


#endif
