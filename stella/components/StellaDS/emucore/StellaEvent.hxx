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

#ifndef STELLAEVENT_HXX
#define STELLAEVENT_HXX

/**
  This file defines the global STELLA events that the frontends
  will use to communicate with the Event Handler.

  Only the standard keys are defined here.  Function and
  navigation (HOME, END, etc) keys are special and must be handled
  by the frontends directly.

  @author Stephen Anthony
  @version $Id: StellaEvent.hxx,v 1.9 2004/05/06 00:06:19 stephena Exp $
*/
class StellaEvent
{
  public:
    /**
      Enumeration of keyboard keycodes
    */
    enum KeyCode
    {
      KCODE_a, KCODE_b, KCODE_c, KCODE_d, KCODE_e, KCODE_f, KCODE_g, KCODE_h,
      KCODE_i, KCODE_j, KCODE_k, KCODE_l, KCODE_m, KCODE_n, KCODE_o, KCODE_p,
      KCODE_q, KCODE_r, KCODE_s, KCODE_t, KCODE_u, KCODE_v, KCODE_w, KCODE_x,
      KCODE_y, KCODE_z,

      KCODE_0, KCODE_1, KCODE_2, KCODE_3, KCODE_4, KCODE_5, KCODE_6, KCODE_7,
      KCODE_8, KCODE_9,

      KCODE_BACKSPACE, KCODE_TAB, KCODE_CLEAR, KCODE_RETURN, 
      KCODE_ESCAPE, KCODE_SPACE, KCODE_COMMA, KCODE_MINUS, KCODE_PERIOD,
      KCODE_SLASH, KCODE_BACKSLASH, KCODE_SEMICOLON, KCODE_EQUALS,

      KCODE_INSERT,    KCODE_HOME,    KCODE_PAGEUP,
      KCODE_DELETE,    KCODE_END,     KCODE_PAGEDOWN,

      KCODE_UP, KCODE_DOWN, KCODE_LEFT, KCODE_RIGHT,

      KCODE_F1, KCODE_F2, KCODE_F3, KCODE_F4, KCODE_F5, KCODE_F6, KCODE_F7,
      KCODE_F8, KCODE_F9, KCODE_F10, KCODE_F11, KCODE_F12, KCODE_F13,
      KCODE_F14, KCODE_F15,

      LastKCODE
    };
};

#endif
