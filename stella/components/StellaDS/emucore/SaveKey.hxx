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

#ifndef SAVEKEY_HXX
#define SAVEKEY_HXX

class MT24LC256;

#include "Control.hxx"
#include "MT24LC256.hxx"

extern MT24LC256 *gSaveKeyEEprom;

/**
  Richard Hutchinson's SaveKey "controller", consisting of a 32KB EEPROM
  accessible using the I2C protocol.

  This code owes a great debt to Alex Herbert's AtariVox documentation and
  driver code.

  @author  Stephen Anthony
  @version $Id$
*/
class SaveKey : public Controller
{
  public:
    /**
      Create a new SaveKey controller plugged into the specified jack

      @param jack       The jack the controller is plugged into
      @param event      The event object to use for events
    */
    SaveKey(Jack jack, const Event& event);

    /**
      Destructor
    */
    virtual ~SaveKey();

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    virtual bool read(DigitalPin pin);

    /**
      Write the given value to the specified digital pin for this
      controller.  Writing is only allowed to the pins associated
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    virtual void write(DigitalPin pin, bool value);
    
    virtual Int32 read(AnalogPin pin);

    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    virtual void update() { }

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary 
      to override this method for devices that remember cycle counts.
    */
    virtual void systemCyclesReset();

  private:
    // The EEPROM used in the SaveKey
    MT24LC256* myEEPROM;
    
    bool bUseSaveKey;
};

#endif
