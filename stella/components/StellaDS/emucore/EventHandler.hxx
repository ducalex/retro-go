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

#ifndef EVENTHANDLER_HXX
#define EVENTHANDLER_HXX

#include "bspf.hxx"
#include "Event.hxx"
#include "StellaEvent.hxx"

class Console;


/**
  This class takes care of event remapping and dispatching for the
  Stella core, as well as keeping track of the current 'mode'.

  The frontends will send translated events here, and the handler will
  check to see what the current 'mode' is.  For now, the modes can be
  normal and menu mode.

  If in normal mode, events received from the frontends are remapped and
  sent to the emulation core.  If in menu mode, the events are sent
  unchanged to the user interface, where (among other things) changing key
  mapping can take place.

  @author  Stephen Anthony
  @version $Id: EventHandler.hxx,v 1.14 2004/06/20 23:30:48 stephena Exp $
*/
class EventHandler
{
  public:
    /**
      Create a new event handler object
    */
    EventHandler(Console* console);
 
    /**
      Destructor
    */
    virtual ~EventHandler();

    /**
      Returns the event object associated with this handler class.

      @return The event object
    */
    Event* event();
};

#endif
