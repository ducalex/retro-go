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

#include "Event.hxx"
#include "Genesis.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Genesis::Genesis(Jack jack, const Event& event)
    : Controller(jack, event)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Genesis::~Genesis()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 bool Genesis::read(DigitalPin pin)
{
  switch(pin)
  {
    case One:
      return (myJack == Left) ? (myEvent.get(Event::JoystickZeroUp) == 0) : 
          (myEvent.get(Event::JoystickOneUp) == 0);

    case Two:
      return (myJack == Left) ? (myEvent.get(Event::JoystickZeroDown) == 0) : 
          (myEvent.get(Event::JoystickOneDown) == 0);

    case Three:
      return (myJack == Left) ? (myEvent.get(Event::JoystickZeroLeft) == 0) : 
          (myEvent.get(Event::JoystickOneLeft) == 0);

    case Four:
      return (myJack == Left) ? (myEvent.get(Event::JoystickZeroRight) == 0) :
          (myEvent.get(Event::JoystickOneRight) == 0);

    case Six:
      return (myJack == Left) ? (myEvent.get(Event::JoystickZeroFire) == 0) : 
          (myEvent.get(Event::JoystickOneFire) == 0);

    default:
      return true;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 Genesis::read(AnalogPin pin)
{
  // The Genesis has one more button (C) that can be read by the 2600
  // However, it seems to work opposite to the BoosterGrip controller,
  // in that the logic is inverted. We still use the BoosterGrip events.

  switch(pin)
  {
    case Five:
      if(myJack == Left)
      {
        return (myEvent.get(Event::BoosterGripZeroBooster) == 0) ? 
            minimumResistance : maximumResistance;
      }
      else
      {
        return (myEvent.get(Event::BoosterGripOneBooster) == 0) ? 
            minimumResistance : maximumResistance;
      }

    case Nine:
      return minimumResistance;     // This is actually how games can tell if we've got this controller plugged in!

    default:
      return maximumResistance;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Genesis::write(DigitalPin, bool)
{
  // Writing doesn't do anything to the Genesis Controller...
}

