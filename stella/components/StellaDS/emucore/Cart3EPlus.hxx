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
#ifndef CARTRIDGE3EPLUS_HXX
#define CARTRIDGE3EPLUS_HXX

class Cartridge3EPlus;

extern uInt16 myCurrentBanks[4];

#include "bspf.hxx"
#include "Cart.hxx"

/**
 Cartridge class for new tiling engine "Boulder Dash" format games with RAM.
  Kind of a combination of 3F and 3E, with better switchability.
  B.Watson's Cart3E was used as a template for building this implementation.
  The destination bank (0-3) is held in the top bits of the value written to
  $3E (for RAM switching) or $3F (for ROM switching). The low 6 bits give
  the actual bank number (0-63) corresponding to 512 byte blocks for RAM and
  1024 byte blocks for ROM. The maximum size is therefore 32K RAM and 64K ROM.
  D7D6         indicate the bank number (0-3)
  D5D4D3D2D1D0 indicate the actual # (0-63) from the image/ram
  
  ROM:
  Note: In descriptions $F000 is equivalent to $1000 -- that is, we only deal
  with the low 13 bits of addressing. Stella code uses $1000, I'm used to $F000
  So, mask with top bits clear :) when reading this document.
  In this scheme, the 4K address space is broken into four 1K ROM/512b RAM segments
  living at 0x1000, 0x1400, 0x1800, 0x1C00 (or, same thing, 0xF000... etc.),
  The last 1K ROM ($FC00-$FFFF) segment in the 6502 address space (ie: $1C00-$1FFF)
  is initialised to point to the FIRST 1K of the ROM image, so the reset vectors
  must be placed at the end of the first 1K in the ROM image.
  Note: This is DIFFERENT to 3E which switches in the UPPER bank and this bank is
  fixed.  This allows variable sized ROM without having to detect size.
  ROM switching (write of block+bank number to $3F) D7D6 upper 2 bits of bank #
  indicates the destination segment (0-3, corresponding to $F000, $F400, $F800,
  $FC00), and lower 6 bits indicate the 1K bank to switch in.  Can handle 64
  x 1K ROM banks (64K total).
  
  D7 D6 D5D4D3D2D1D0
  0  0   x x x x x x   switch a 1K ROM bank xxxxxx to $F000
  0  1                 switch a 1K ROM bank xxxxxx to $F400
  1  0                 switch a 1K ROM bank xxxxxx to $F800
  1  1                 switch a 1K ROM bank xxxxxx to $FC00
  
  RAM switching (write of segment+bank number to $3E) with D7D6 upper 2 bits of
  bank # indicates the destination RAM segment (0-3, corresponding to $F000,
  $F400, $F800, $FC00).
  Can handle 64 x 512 byte RAM banks (32K total)
  
  D7 D6 D5D4D3D2D1D0
  0  0   x x x x x x   switch a 512 byte RAM bank xxxxxx to $F000 with write @ $F200
  0  1                 switch a 512 byte RAM bank xxxxxx to $F400 with write @ $F600
  1  0                 switch a 512 byte RAM bank xxxxxx to $F800 with write @ $FA00
  1  1                 switch a 512 byte RAM bank xxxxxx to $FC00 with write @ $FE00
  @author  Thomas Jentzsch and Stephen Anthony
*/

class Cartridge3EPlus : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image and size

      @param image Pointer to the ROM image
      @param size The size of the ROM image
    */
    Cartridge3EPlus(const uInt8* image, uInt32 size);
 
    /**
      Destructor
    */
    virtual ~Cartridge3EPlus();

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
    void segment(uInt8 seg, uInt8 bank);

  private:
    // ROM contents ... up to 64K
    uInt8 *myImage;
    
    uInt8 statingBank;
    uInt8 myBankMod;
    
    // Size of the ROM image
    uInt32 mySize;
};
#endif

