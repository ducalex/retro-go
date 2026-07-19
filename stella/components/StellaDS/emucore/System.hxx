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

#ifndef SYSTEM_HXX
#define SYSTEM_HXX

class Device;
class M6502;
class NullDevice;

#include "bspf.hxx"
#include "Device.hxx"
#include "NullDev.hxx"

#define SOUND_SIZE (1024)

extern Int32 gSystemCycles;    // Number of system cycles executed since the last reset
extern uInt32 gTotalSystemCycles;
extern Int32 debug[];          // Array that can be output on screen in ds_main_menu.cpp if the DEBUG_ENABLE switch is defined
extern uInt8 myRAM[];          // The Atari 128 bytes of RAM plus another 128 for the SC (Super Carts)

/**
  Structure used to specify access methods for a page
*/
struct PageAccess
{
  /**
    Pointer to a block of memory or the null pointer.  The null pointer
    indicates that the device's peek method should be invoked for reads
    to this page, while other values are the base address of an array 
    to directly access for reads to this page.
  */
  uInt8* directPeekBase;

  /**
    Pointer to a block of memory or the null pointer.  The null pointer
    indicates that the device's poke method should be invoked for writes
    to this page, while other values are the base address of an array 
    to directly access for pokes to this page.
  */
  uInt8* directPokeBase;

  /**
    Pointer to the device associated with this page or to the system's 
    null device if the page hasn't been mapped to a device
  */
  Device* device;
};

extern PageAccess myPageAccessTable[64];

#define MY_PAGE_SHIFT   7
#define MY_PAGE_MASK    0x7F
#define MY_ADDR_SHIFT   13
#define MY_ADDR_MASK    0x1FFF


/**
  This class represents a system consisting of a 6502 microprocessor
  and a set of devices.  The devices are mapped into an addressing
  space of 2^n bytes (1 <= n <= 16).  The addressing space is broken
  into 2^m byte pages (1 <= m <= n), where a page is the smallest unit
  a device can use when installing itself in the system.

  In general the addressing space will be 8192 (2^13) bytes for a 
  6507 based system and 65536 (2^16) bytes for a 6502 based system.

  TODO: To allow for dynamic code generation we probably need to
        add a tag to each page that indicates if it is read only
        memory.  We also need to notify the processor anytime a
        page access method is changed so that it can clear the
        dynamic code for that page of memory.

  @author  Bradford W. Mott
  @version $Id: System.hxx,v 1.3 2002/05/13 19:10:25 stephena Exp $
*/
class System
{
  public:
    /**
      Create a new system with an addressing space of 2^n bytes and
      pages of 2^m bytes.

      @param n Log base 2 of the addressing space size
      @param m Log base 2 of the page size
    */
    System(uInt16 n, uInt16 m);

    /**
      Destructor
    */
    virtual ~System();

  public:
    /**
      Reset the system cycle counter, the attached devices, and the
      attached processor of the system.
    */
    void reset();

  public:
    /**
      Attach the specified device and claim ownership of it.  The device 
      will be asked to install itself.

      @param device The device to attach to the system
    */
    void attach(Device* device);

    /**
      Attach the specified processor and claim ownership of it.  The
      processor will be asked to install itself.

      @param m6502 The 6502 microprocessor to attach to the system
    */
    void attach(M6502* m6502);

  public:
    /**
      Answer the 6502 microprocessor attached to the system.  If a
      processor has not been attached calling this function will fail.

      @return The attached 6502 microprocessor
    */
    M6502& m6502()
    {
      return *myM6502;
    }

    /**
      Get the null device associated with the system.  Every system 
      has a null device associated with it that's used by pages which 
      aren't mapped to "real" devices.

      @return The null device associated with the system
    */
    NullDevice& nullDevice()
    {
      return myNullDevice;
    }

    /**
      Get the total number of pages available in the system.

      @return The total number of pages available
    */
    uInt16 numberOfPages() const
    {
      return myNumberOfPages;
    }

    /**
      Get the amount to right shift an address by to obtain its page.

      @return The amount to right shift an address by to get its page
    */
    inline uInt16 pageShift() const
    {
      return myPageShift;
    }

    /**
      Get the mask to apply to an address to obtain its page offset.

      @return The mask to apply to an address to obtain its page offset
    */
    inline uInt16 pageMask() const
    {
      return myPageMask;
    }
 
  public:
    /**
      Get the number of system cycles which have passed since the last
      time cycles were reset or the system was reset.

      @return The number of system cycles which have passed
    */
    inline Int32 cycles() const 
    { 
      return gSystemCycles; 
    }


    /**
      Reset the system cycle count to zero.  The first thing that
      happens is that all devices are notified of the reset by invoking 
      their systemCyclesReset method then the system cycle count is 
      reset to zero.
    */
    void resetCycles();

  public:

    /**
      Get the byte at the specified address.  No masking of the
      address occurs before it's sent to the device mapped at
      the address.

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address);

    /**
      Change the byte at the specified address to the given value.
      No masking of the address occurs before it's sent to the device
      mapped at the address.

      @param address The address where the value should be stored
      @param value The value to be stored at the address
    */
    void poke(uInt16 address, uInt8 value);

  public:
    /**
      Set the page accessing method for the specified page.

      @param page The page accessing methods should be set for
      @param access The accessing methods to be used by the page
    */
    void setPageAccess(uInt16 page, const PageAccess& access);

    /**
      Get the page accessing method for the specified page.

      @param page The page to get accessing methods for
      @return The accessing methods used by the page
    */
    const PageAccess& getPageAccess(uInt16 page);
 
  private:
    // Mask to apply to an address before accessing memory
    const uInt16 myAddressMask;

    // Amount to shift an address by to determine what page it's on
    const uInt16 myPageShift;

    // Mask to apply to an address to obtain its page offset
    const uInt16 myPageMask;
 
    // Number of pages in the system
    const uInt16 myNumberOfPages;

    // Array of all the devices attached to the system
    Device* myDevices[16];

    // Number of devices attached to the system
    uInt16 myNumberOfDevices;

    // 6502 processor attached to the system or the null pointer
    M6502* myM6502;

    // Null device to use for page which are not installed
    NullDevice myNullDevice; 

  private:
    // Copy constructor isn't supported by this class so make it private
    System(const System&);

    // Assignment operator isn't supported by this class so make it private
    System& operator = (const System&);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uInt8 System::peek(uInt16 addr)
{
  PageAccess& access = myPageAccessTable[(addr & MY_ADDR_MASK) >> MY_PAGE_SHIFT];

  // See if this page uses direct accessing or not 
  if(access.directPeekBase != 0)
  {
    return *(access.directPeekBase + (addr & MY_PAGE_MASK));
  }
  else
  {
    return access.device->peek(addr);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void System::poke(uInt16 addr, uInt8 value)
{
  PageAccess& access = myPageAccessTable[(addr & MY_ADDR_MASK) >> MY_PAGE_SHIFT];
  
  // See if this page uses direct accessing or not 
  if(access.directPokeBase != 0)
  {
    *(access.directPokeBase + (addr & MY_PAGE_MASK)) = value;
  }
  else
  {
    access.device->poke(addr, value);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void System::setPageAccess(uInt16 page, const PageAccess& access)
{
  myPageAccessTable[page] = access;
}

#endif
