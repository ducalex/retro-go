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
#ifndef CARTRIDGEE7_HXX
#define CARTRIDGEE7_HXX

class CartridgeE7;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for M-Network bankswitched games.  
  In this bankswitching scheme the 2600's 4K cartridge address 
  space is broken into two 2K segments.

  Kevin Horton describes E7 as follows:

    Only M-Network used this scheme. This has to be the 
    most complex method used in any cart! :-)  It allows 
    for the capability of 2K of RAM; although it doesn't 
    have to be used (in fact, only one cart used it).  
    There are now 8 2K banks, instead of 4.  The last 2K 
    in the cart always points to the last 2K of the ROM 
    image, while the first 2K is selectable.  You access 
    1FE0 to 1FE6 to select which 2K bank. Note that you
    cannot select the last 2K of the ROM image into the 
    lower 2K of the cart!  Accessing 1FE7 selects 1K of 
    RAM at 1000-17FF instead of ROM!  The 2K of RAM is
    broken up into two 1K sections.  One 1K section is 
    mapped in at 1000-17FF if 1FE7 has been accessed.  
    1000-13FF is the write port, while 1400-17FF is the 
    read port.  The second 1K of RAM appears at 1800-19FF.  
    1800-18FF is the write port while 1900-19FF is the 
    read port.  You select which 256 byte block appears 
    here by accessing 1FF8 to 1FFB.

  @author  Bradford W. Mott
  @version $Id: CartE7.hxx,v 1.3 2005/02/13 19:17:02 stephena Exp $
*/
class CartridgeE7 : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    CartridgeE7(const uInt8* image, uInt16 size);
 
    /**
      Destructor
    */
    virtual ~CartridgeE7();

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
      Map the specfied bank into the first segment

      @param bank The bank that should be installed in the system
    */
    void bank(uInt16 bank);

    /**
      Install pages for the specified 256 byte bank of RAM

      @param bank The bank that should be installed in the system
    */
    void bankRAM(uInt16 bank);

  private:
    // The 16K ROM image of the cartridge
    uInt8 *myImage;

    // The 2048 bytes of RAM will use fast_cart_buffer[]    
};
#endif

