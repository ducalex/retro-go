/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Globally defined strings.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef __GNUG__
#pragma implementation "dstrings.h"
#endif
#include "dstrings.h"


// killough 1/18/98: remove hardcoded limit, add const:
const char *const endmsg[]=
{
  // DOOM1
  QUITMSG,
  "please don't leave, there's more\ndemons to toast!",
  "let's beat it -- this is turning\ninto a bloodbath!",
  "i wouldn't leave if i were you.\ndos is much worse.",
  "you're trying to say you like dos\nbetter than me, right?",
  "don't leave yet -- there's a\ndemon around that corner!",
  "ya know, next time you come in here\ni'm gonna toast ya.",
  "go ahead and leave. see if i care.",  // 1/15/98 killough

  // QuitDOOM II messages
  "you want to quit?\nthen, thou hast lost an eighth!",
  "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
  "get outta here and go back\nto your boring programs.",
  "if i were your boss, i'd \n deathmatch ya in a minute!",
  "look, bud. you leave now\nand you forfeit your body count!",
  "just leave. when you come\nback, i'll be waiting with a bat.",
  "you're lucky i don't smack\nyou for thinking about leaving.",  // 1/15/98 killough

  // FinalDOOM?

// Note that these ending "bad taste" strings were commented out
// in the original id code as the #else case of an #if 1
// Obviously they were internal playthings before the release of
// DOOM2 and were not intended for public use.
//
// Following messages commented out for now. Bad taste.   // phares

//  "fuck you, pussy!\nget the fuck out!",
//  "you quit and i'll jizz\nin your cystholes!",
//  "if you leave, i'll make\nthe lord drink my jizz.",
//  "hey, ron! can we say\n'fuck' in the game?",
//  "i'd leave: this is just\nmore monsters and levels.\nwhat a load.",
//  "suck it down, asshole!\nyou're a fucking wimp!",
//  "don't quit now! we're \nstill spending your money!",

  // Internal debug. Different style, too.
  "THIS IS NO MESSAGE!\nPage intentionally left blank.",  // 1/15/98 killough
};

// killough 1/18/98: remove hardcoded limit and replace with var (silly hack):
const size_t NUM_QUITMESSAGES = sizeof(endmsg)/sizeof(*endmsg) - 1;
