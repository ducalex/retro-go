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

#include <assert.h>
#include "Event.hxx"
#include "Paddles.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Paddles::Paddles(Jack jack, const Event& event)
    : Controller(jack, event)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Paddles::~Paddles()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Paddles::read(DigitalPin pin)
{
  switch(pin)
  {
    case Three:
      return (myJack == Left) ? (myEvent.get(Event::PaddleOneFire) == 0) : 
          (myEvent.get(Event::PaddleThreeFire) == 0);

    case Four:
      return (myJack == Left) ? (myEvent.get(Event::PaddleZeroFire) == 0) : 
          (myEvent.get(Event::PaddleTwoFire) == 0);

    default:
      // Other pins are not connected (floating high)
      return true;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 Paddles::read(AnalogPin pin)
{
  switch(pin)
  {
    case Five:
      return (myJack == Left) ? myEvent.get(Event::PaddleOneResistance) : 
          myEvent.get(Event::PaddleThreeResistance);

    case Nine:
      return (myJack == Left) ? myEvent.get(Event::PaddleZeroResistance) : 
          myEvent.get(Event::PaddleTwoResistance);

    default:
      return maximumResistance;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Paddles::write(DigitalPin, bool)
{
  // Writing doesn't do anything to the paddles...
}

