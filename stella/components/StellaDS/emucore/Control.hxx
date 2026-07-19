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

#ifndef CONTROLLER_HXX
#define CONTROLLER_HXX

class Controller;
class Event;

#include "bspf.hxx"

/**
  A controller is a device that plugs into either the left or right 
  controller jack of the Video Computer System (VCS).  The pins of 
  the controller jacks are mapped as follows:

                           -------------
                           \ 1 2 3 4 5 /
                            \ 6 7 8 9 /
                             ---------

            Left Controller             Right Controller

    pin 1   D4  PIA SWCHA               D0  PIA SWCHA
    pin 2   D5  PIA SWCHA               D1  PIA SWCHA
    pin 3   D6  PIA SWCHA               D2  PIA SWCHA
    pin 4   D7  PIA SWCHA               D3  PIA SWCHA
    pin 5   D7  TIA INPT1 (Dumped)      D7  TIA INPT3 (Dumped)
    pin 6   D7  TIA INPT4 (Latched)     D7  TIA INPT5 (Latched)
    pin 7   +5                          +5
    pin 8   GND                         GND
    pin 9   D7  TIA INPT0 (Dumped)      D7  TIA INPT2 (Dumped)

  Each of the pins connected to the PIA can be configured as an
  input or output pin.  The "dumped" TIA pins are used to charge
  a capacitor.  A potentiometer is sometimes connected to these
  pins for analog input.

  This is a base class for all controllers.  It provides a view 
  of the controller from the prespective of the controller's jack.  

  @author  Bradford W. Mott
  @version $Id: Control.hxx,v 1.1.1.1 2001/12/27 19:54:21 bwmott Exp $
*/
class Controller
{
  public:
    /**
      Enumeration of the controller jacks
    */
    enum Jack
    {
      Left, Right
    };

  public:
    /**
      Create a new controller plugged into the specified jack

      @param jack The jack the controller is plugged into
      @param event The event object to use for events
    */
    Controller(Jack jack, const Event& event);
 
    /**
      Destructor
    */
    virtual ~Controller();

  public:
    /**
      Enumeration of the digital pins of a controller port
    */
    enum DigitalPin
    {
      One, Two, Three, Four, Six
    };

    /**
      Enumeration of the analog pins of a controller port
    */
    enum AnalogPin
    {
      Five, Nine
    };

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    virtual bool read(DigitalPin pin) = 0;

    /**
      Read the resistance at the specified analog pin for this controller.  
      The returned value is the resistance measured in ohms.

      @param pin The pin of the controller jack to read
      @return The resistance at the specified pin
    */
    virtual Int32 read(AnalogPin pin) = 0;

    /**
      Write the given value to the specified digital pin for this 
      controller.  Writing is only allowed to the pins associated 
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    virtual void write(DigitalPin pin, bool value) = 0;

  public:
    /// Constant which represents maximum resistance for analog pins
    static const Int32 maximumResistance;

    /// Constant which represents minimum resistance for analog pins
    static const Int32 minimumResistance;

  protected:
    /// Specifies which jack the controller is plugged in
    const Jack myJack;

    /// Reference to the event object this controller uses
    const Event& myEvent;

  protected:
    // Copy constructor isn't supported by controllers so make it private
    Controller(const Controller&);

    // Assignment operator isn't supported by controllers so make it private
    Controller& operator = (const Controller&);
};
#endif

