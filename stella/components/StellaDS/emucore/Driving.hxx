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

#ifndef DRIVING_HXX
#define DRIVING_HXX

class Driving;
class System;

#include "bspf.hxx"
#include "Control.hxx"

/**
  The standard Atari 2600 Indy 500 driving controller.

  @author  Bradford W. Mott
  @version $Id: Driving.hxx,v 1.1.1.1 2001/12/27 19:54:21 bwmott Exp $
*/
class Driving : public Controller
{
  public:
    /**
      Create a new Indy 500 driving controller plugged into 
      the specified jack

      @param jack The jack the controller is plugged into
      @param event The event object to use for events
    */
    Driving(Jack jack, const Event& event);

    /**
      Destructor
    */
    virtual ~Driving();

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    virtual bool read(DigitalPin pin);

    /**
      Read the resistance at the specified analog pin for this controller.
      The returned value is the resistance measured in ohms.

      @param pin The pin of the controller jack to read
      @return The resistance at the specified pin
    */
    virtual Int32 read(AnalogPin pin);

    /**
      Write the given value to the specified digital pin for this
      controller.  Writing is only allowed to the pins associated
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    virtual void write(DigitalPin pin, bool value);

  private:
    // Counter to iterate through the gray codes
    uInt32 myCounter;
};
#endif

