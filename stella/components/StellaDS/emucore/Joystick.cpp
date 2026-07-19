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
#include <nds.h>
#include <assert.h>
#include "Event.hxx"
#include "Joystick.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Joystick::Joystick(Jack jack, const Event& event)
    : Controller(jack, event)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Joystick::~Joystick()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 bool Joystick::read(DigitalPin pin)
{
  switch(pin)
  {
    case One:
      return (myJack == Left) ? (myStellaEvent.get(Event::JoystickZeroUp) == 0) : 
          (myStellaEvent.get(Event::JoystickOneUp) == 0);

    case Two:
      return (myJack == Left) ? (myStellaEvent.get(Event::JoystickZeroDown) == 0) : 
          (myStellaEvent.get(Event::JoystickOneDown) == 0);

    case Three:
      return (myJack == Left) ? (myStellaEvent.get(Event::JoystickZeroLeft) == 0) : 
          (myStellaEvent.get(Event::JoystickOneLeft) == 0);

    case Four:
      return (myJack == Left) ? (myStellaEvent.get(Event::JoystickZeroRight) == 0) : 
          (myStellaEvent.get(Event::JoystickOneRight) == 0);

    case Six:
      return (myJack == Left) ? (myStellaEvent.get(Event::JoystickZeroFire) == 0) : 
          (myStellaEvent.get(Event::JoystickOneFire) == 0);

    default:
      return true;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 Joystick::read(AnalogPin)
{
  // Analog pins are not connect in joystick so we have infinite resistance 
  return maximumResistance;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Joystick::write(DigitalPin, bool)
{
  // Writing doesn't do anything to the joystick...
}

