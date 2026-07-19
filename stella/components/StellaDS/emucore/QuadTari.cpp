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

#include "Event.hxx"
#include "QuadTari.hxx"
#include "TIA.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QuadTari::QuadTari(Jack jack, const Event& event)
    : Controller(jack, event)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
QuadTari::~QuadTari()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool QuadTari::read(DigitalPin pin)
{
  if (myVBLANK & 0x80)    
  {
      switch(pin)
      {
        case One:
          return (myEvent.get(Event::JoystickOneUp) == 0);

        case Two:
          return (myEvent.get(Event::JoystickOneDown) == 0);

        case Three:
          return (myEvent.get(Event::JoystickOneLeft) == 0);

        case Four:
          return (myEvent.get(Event::JoystickOneRight) == 0);

        case Six:
          return (myEvent.get(Event::JoystickOneFire) == 0);

        default:
          return true;
      }
  }
  else
  {
      switch(pin)
      {
        case One:
          return (myEvent.get(Event::JoystickZeroUp) == 0);

        case Two:
          return (myEvent.get(Event::JoystickZeroDown) == 0);

        case Three:
          return (myEvent.get(Event::JoystickZeroLeft) == 0);

        case Four:
          return (myEvent.get(Event::JoystickZeroRight) == 0);

        case Six:
          return (myEvent.get(Event::JoystickZeroFire) == 0);

        default:
          return true;
      }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 QuadTari::read(AnalogPin pin)
{
  switch(pin)
  {
    case Five:
      return minimumResistance;     // This is actually how games can tell if we've got this controller plugged in!
    case Nine:
      return maximumResistance;     // This is actually how games can tell if we've got this controller plugged in!

    default:
      return maximumResistance;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void QuadTari::write(DigitalPin, bool)
{
  // Writing doesn't do anything to our limited QuadTari implementation
}

