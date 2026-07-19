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

#ifndef CARTRIDGEWD_HXX
#define CARTRIDGEWD_HXX

class CartridgeWD;

#include "bspf.hxx"
#include "Cart.hxx"

extern Int32 myCyclesAtBankswitchInit;
extern uInt8 myPendingBank;


/**
  This is the cartridge class for a "Wickstead Design" prototype cart.
  The ROM has 64 bytes of RAM.
  In this bankswitching scheme the 2600's 4K cartridge address space
  is broken into four 1K segments.  The desired arrangement of 1K banks
  is selected by accessing $30 - $3F of TIA address space.  The banks
  are mapped into all 4 segments at once as follows:

    $0030, $0038: 0,0,1,3
    $0031, $0039: 0,1,2,3
    $0032, $003A: 4,5,6,7
    $0033, $003B: 7,4,2,3

    $0034, $003C: 0,0,6,7
    $0035, $003D: 0,1,7,6
    $0036, $003E: 2,3,4,5
    $0037, $003F: 6,0,5,1


  (Removed: In the uppermost (third) segment, the byte at $3FC is overwritten by 0.)

  The 64 bytes of RAM are accessible at $1000 - $103F (read port) and
  $1040 - $107F (write port).  Because the RAM takes 128 bytes of address
  space, the range $1000 - $107F of segment 0 ROM will never be available.

  @author  Stephen Anthony, Thomas Jentzsch
*/

class CartridgeWD : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image and size

      @param image Pointer to the ROM image
      @param size The size of the ROM image
    */
    CartridgeWD(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~CartridgeWD();

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
    // Pointer to a dynamically allocated ROM image of the cartridge
    uInt8* myImage;

    // Size of the ROM image
    uInt32 mySize;
};
#endif

