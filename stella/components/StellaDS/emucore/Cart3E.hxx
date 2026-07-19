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
#ifndef CARTRIDGE3E_HXX
#define CARTRIDGE3E_HXX

class Cartridge3E;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for Tigervision's bankswitched
  games with RAM (basically, 3F plus up to 32K of RAM). This
  code is basically Brad's Cart3F code plus 32K RAM support.

  In this bankswitching scheme the 2600's 4K cartridge
  address space is broken into two 2K segments.  The last 2K
  segment always points to the last 2K of the ROM image.

  The lower 2K of address space maps to either one of the 2K ROM banks
  (up to 256 of them, though only 240 are supposed to be used for
  compatibility with the Kroko Cart and Cuttle Cart 2), or else one
  of the 1K RAM banks (up to 32 of them). Like other carts with RAM,
  this takes up twice the address space that it should: The lower 1K
  is the read port, and the upper 1K is the write port (maps to the
  same memory).

  To map ROM, the desired bank number of the first 2K segment is selected
  by storing its value into $3F. To map RAM in the first 2K segment
  instead, store the RAM bank number into $3E.

  This implementation of 3E bankswitching numbers the ROM banks 0 to
  255, and the RAM banks 256 to 287. This is done because the public
  bankswitching interface requires us to use one bank number, not one
  bank number plus the knowledge of whether it's RAM or ROM.

  All 32K of potential RAM is available to a game using this class, even
  though real cartridges might not have the full 32K: We have no way to
  tell how much RAM the game expects. This may change in the future (we
  may add a stella.pro property for this), but for now it shouldn't cause
  any problems. (Famous last words...)

  @author  B. Watson
  @version $Id: Cart3E.hxx,v 1.5 2007/01/14 16:17:52 stephena Exp $
*/
class Cartridge3E : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image and size

      @param image Pointer to the ROM image
      @param size The size of the ROM image
    */
    Cartridge3E(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~Cartridge3E();

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

  public:
    /**
      Get the byte at the specified address

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
      Map the specified bank into the first segment

      @param bank The bank that should be mapped
    */
    void bank(uInt16 bank);

  private:
    // ROM contents ... up to 512K
    uInt8 *myImage;
    
    // Size of the ROM image
    uInt32 mySize;
};
#endif

