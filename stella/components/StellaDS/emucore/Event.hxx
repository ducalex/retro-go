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

#ifndef EVENT_HXX
#define EVENT_HXX

class Event;

#include "bspf.hxx"

/**
  @author  Bradford W. Mott
  @version $Id: Event.hxx,v 1.5 2004/06/04 12:22:12 stephena Exp $
*/
class Event
{
  public:
    /**
      Enumeration of console and controller event types
    */
    enum Type
    {
      NoType,
      ConsoleOn, ConsoleOff, ConsoleColor, ConsoleBlackWhite,
      ConsoleLeftDifficultyA, ConsoleLeftDifficultyB,
      ConsoleRightDifficultyA, ConsoleRightDifficultyB,
      ConsoleSelect, ConsoleReset,

      JoystickZeroUp, JoystickZeroDown, JoystickZeroLeft,
      JoystickZeroRight, JoystickZeroFire,
      JoystickOneUp, JoystickOneDown, JoystickOneLeft,
      JoystickOneRight, JoystickOneFire,

      BoosterGripZeroTrigger, BoosterGripZeroBooster,
      BoosterGripOneTrigger, BoosterGripOneBooster,

      PaddleZeroResistance, PaddleZeroFire,
      PaddleOneResistance, PaddleOneFire,
      PaddleTwoResistance, PaddleTwoFire,
      PaddleThreeResistance, PaddleThreeFire,

      KeyboardZero1, KeyboardZero2, KeyboardZero3,
      KeyboardZero4, KeyboardZero5, KeyboardZero6,
      KeyboardZero7, KeyboardZero8, KeyboardZero9,
      KeyboardZeroStar, KeyboardZero0, KeyboardZeroPound,

      KeyboardOne1, KeyboardOne2, KeyboardOne3,
      KeyboardOne4, KeyboardOne5, KeyboardOne6,
      KeyboardOne7, KeyboardOne8, KeyboardOne9,
      KeyboardOneStar, KeyboardOne0, KeyboardOnePound,

      DrivingZeroClockwise, DrivingZeroCounterClockwise, DrivingZeroValue, 
	  DrivingZeroFire,
      DrivingOneClockwise, DrivingOneCounterClockwise, DrivingOneValue,
	  DrivingOneFire,
	  
      LastType
    };

  public:
    /**
      Create a new event object
    */
    Event();
 
    /**
      Destructor
    */
    virtual ~Event();

  public:
    /**
      Get the value associated with the event of the specified type
    */
    virtual Int32 get(Type type) const;

    /**
      Set the value associated with the event of the specified type
    */
    virtual void set(Type type, Int32 value);
    
  protected:
    // Array of values associated with each event type
    Int32 myValues[LastType];
};

extern Event myStellaEvent;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline Int32 Event::get(Type type) const
{
  return myValues[type];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void Event::set(Type type, Int32 value)
{
  myValues[type] = value;
}



#endif

