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

#ifndef M6532_HXX
#define M6532_HXX

class Console;
class System;

#include "bspf.hxx"
#include "Device.hxx"

// An amazing 128 bytes of RAM plus 128 more for the SARA carts
extern uInt8 myRAM[256];

extern uInt32 myTimer;
extern uInt8  myIntervalShift;
extern Int32  myCyclesWhenTimerSet;
extern uInt8  myInterruptEnabled;
extern uInt8  myInterruptTriggered;
extern uInt8  myDDRA;
extern uInt8  myDDRB; 
extern uInt8  myOutA;
extern uInt8  myOutTimer[4];


/**
  RIOT

  @author  Bradford W. Mott
  @version $Id: M6532.hxx,v 1.2 2002/05/13 19:17:32 stephena Exp $
*/
class M6532 : public Device
{
  public:
    /**
      Create a new 6532 for the specified console

      @param console The console the 6532 is associated with
    */
    M6532();
 
    /**
      Destructor
    */
    virtual ~M6532();

   public:
    /**
      Get a null terminated string which is the device's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const;

    /**
      Reset cartridge to its power-on state
    */
    virtual void reset();

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary
      to override this method for devices that remember cycle counts.
    */
    virtual void systemCyclesReset();

    /**
      Install 6532 in the specified system.  Invoked by the system
      when the 6532 is attached to it.

      @param system The system the device should install itself in
    */
    virtual void install(System& system);

   public:
   
     void setConsole(Console *console) {myConsole = console;}
   
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
    // Reference to the console
    Console *myConsole;
   
  private:
    // Copy constructor isn't supported by this class so make it private
    M6532(const M6532&);
 
    // Assignment operator isn't supported by this class so make it private
    M6532& operator = (const M6532&);
    
    inline Int32 timerClocks()
      { return myTimer - (gSystemCycles - myCyclesWhenTimerSet); }

    void setTimerRegister(uInt8 data, uInt8 interval);
    void setPinState(void);
};

extern M6532 theM6532;

#endif

