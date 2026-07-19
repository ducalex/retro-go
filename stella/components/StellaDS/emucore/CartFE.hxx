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

#ifndef CARTRIDGEFE_HXX
#define CARTRIDGEFE_HXX

class CartridgeFE;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  Bankswitching method used by Activison's Robot Tank and Decathlon.

  Kevin Horton describes FE as follows:

    Used only on two carts (Robot Tank and Decathlon).  These 
    carts are very weird.  It does not use accesses to the stack 
    like was previously thought.  Instead, if you watch the called 
    addresses very carefully, you can see that they are either Dxxx 
    or Fxxx.  This determines the bank to use.  Just monitor A13 of 
    the processor and use it to determine your bank! :-)  Of course 
    the 6507 in the 2600 does not have an A13, so the cart must have 
    an extra bit in the ROM matrix to tell when to switch banks.  
    There is *no* way to determine which bank you want to be in from
    monitoring the bus.

  @author  Bradford W. Mott
  @version $Id: CartFE.hxx,v 1.2 2002/05/13 19:17:32 stephena Exp $
*/
class CartridgeFE : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    CartridgeFE(const uInt8* image);
 
    /**
      Destructor
    */
    virtual ~CartridgeFE();

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
    // The 8K ROM image of the cartridge
    uInt8 *myImage;
};
#endif

