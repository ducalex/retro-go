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

#ifndef SWITCHES_HXX
#define SWITCHES_HXX

class Event;
class Switches;

#include "bspf.hxx"

/**
  This class represents the console switches of the game console.

  @author  Bradford W. Mott
  @version $Id: Switches.hxx,v 1.1.1.1 2001/12/27 19:54:23 bwmott Exp $
*/
class Switches
{
  public:
    /**
      Create a new set of switches using the specified events and
      properties

      @param event The event object to use for events
    */
    Switches(const Event& event);
 
    /**
      Destructor
    */
    virtual ~Switches();

  public:
    /**
      Get the value of the console switches

      @return The 8 bits which represent the state of the console switches
    */
    uInt8 read();

  private:
    // Reference to the event object to use
    const Event& myEvent;

    // State of the console switches
    uInt8 mySwitches;
};
#endif

