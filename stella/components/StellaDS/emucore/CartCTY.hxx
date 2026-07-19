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

#ifndef CartridgeCTY_HXX
#define CartridgeCTY_HXX

class CartridgeCTY;

#include "bspf.hxx"
#include "Cart.hxx"

extern uInt16 myTunePosition;
extern uInt32 myAudioCycles;
extern uInt32 deltaCyclesX10;
extern uInt8 myOperationType;

extern uInt8 CTY_updateMusicModeDataFetchers(void);

/**
  Cartridge class used for Chetiry - has extra RAM and an EE Chip
*/
class CartridgeCTY : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    CartridgeCTY(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~CartridgeCTY();

  public:
    /**
      Get a null terminated string which is the device's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const;

    /**
      Reset device to its power-on state
    */
    virtual void reset();

    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

      @param system The system the device should install itself in
    */
    virtual void install(System& system);

    virtual void systemCyclesReset();
    
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

  private:
    /**
      Install pages for the specified bank in the system

      @param bank The bank that should be installed in the system
    */
    void bank(uInt16 bank);
    void handle_cty_flash_backing(void);
    void updateTune(void);
    uInt8 ramReadWrite(void);

  private:

    // The ROM image of the cartridge
    uInt8 *myImage;

    // The 256 bytes of RAM on the cartridge
    uInt8 *ctyRAM;

    // The 256 bytes of EEPROM on the cartridge
    uInt8 *ctyEE;
};
#endif

