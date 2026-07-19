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
#ifndef CARTRIDGE03E0_HXX
#define CARTRIDGE03E0_HXX

class Cartridge03E0;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for Parker Brothers' 8K games with special
  Brazilian bankswitching.  In this bankswitching scheme the 2600's 4K
  cartridge address space is broken into four 1K segments.

  The desired 1K bank of the ROM is selected as follows:
    If A12 == 0, A9 == 1, A8 == 1, A7 == 1 ($0380..$03ff):
      A4 == 0 ($03e0) loads the bank number for segment #0
      A5 == 0 ($03d0) loads the bank number for segment #1
      A6 == 0 ($03b0) loads the bank number for segment #2
    Bits A0, A1, A2 determine the bank number (0..7)

  The last 1K segment always points to the last 1K of the ROM image.

  Because of the complexity of this scheme, the cart reports having
  only one actual bank, in which pieces of it can be swapped out in
  many different ways.

  @author  Thomas Jentzsch
*/

class Cartridge03E0 : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    Cartridge03E0(const uInt8* image);
 
    /**
      Destructor
    */
    virtual ~Cartridge03E0();

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
    /**
      Install pages for the specified bank/segment in the system

      @param bank The bank that should be installed in the system
      @param segment The segment that should be set for that bank
    */
    void bank(uInt16 bank, uInt16 segment);
    
    // Check if we should switch banks on this address access
    void checkSwitchBank(uInt16 address);

  private:
    // Previous Device's page access
    PageAccess myHotSpotPageAccess;

    // The 8K ROM image of the cartridge
    uInt8 *myImage;
};
#endif

