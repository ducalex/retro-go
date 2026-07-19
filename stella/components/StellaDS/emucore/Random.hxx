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

#ifndef RANDOM_HXX
#define RANDOM_HXX

#include "bspf.hxx"

/**
  This is a quick-and-dirty random number generator.  It is based on 
  information in Chapter 7 of "Numerical Recipes in C".  It's a simple 
  linear congruential generator.

  @author  Bradford W. Mott
  @version $Id: Random.hxx,v 1.1.1.1 2001/12/27 19:54:23 bwmott Exp $
*/
class Random
{
  public:
    /**
      Create a new random number generator
    */
    Random();
    
  public:
    /**
      Answer the next random number from the random number generator

      @return A random number
    */
    uInt32 next();
};
#endif

