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
 *   Menu widget stuff, episode selection and such.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.

bool M_Responder (event_t *ev);

// Called by main loop,
// only used for menu (skull cursor) animation.

void M_Ticker (void);

// Called by main loop,
// draws the menus directly into the screen buffer.

void M_Drawer (void);

// Called by D_DoomMain,
// loads the config file.

void M_Init (void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.

void M_StartControlPanel (void);

void M_ForcedLoadGame(const char *msg); // killough 5/15/98: forced loadgames

void M_DrawCredits(void);    // killough 11/98

/****************************
 *
 *  The following #defines are for the m_flags field of each item on every
 *  Setup Screen. They can be OR'ed together where appropriate
 */

#define S_HILITE     0x1 // Cursor is sitting on this item
#define S_SELECT     0x2 // We're changing this item
#define S_TITLE      0x4 // Title item
#define S_YESNO      0x8 // Yes or No item
#define S_CRITEM    0x10 // Message color
#define S_COLOR     0x20 // Automap color
#define S_CHAT      0x40 // Chat String
#define S_RESET     0x80 // Reset to Defaults Button
#define S_PREV     0x100 // Previous menu exists
#define S_NEXT     0x200 // Next menu exists
#define S_KEY      0x400 // Key Binding
#define S_WEAP     0x800 // Weapon #
#define S_NUM     0x1000 // Numerical item
#define S_SKIP    0x2000 // Cursor can't land here
#define S_KEEP    0x4000 // Don't swap key out
#define S_END     0x8000 // Last item in list (dummy)
#define S_LEVWARN 0x10000// killough 8/30/98: Always warn about pending change
#define S_PRGWARN 0x20000// killough 10/98: Warn about change until next run
#define S_BADVAL  0x40000// killough 10/98: Warn about bad value
#define S_FILE    0x80000// killough 10/98: Filenames
#define S_LEFTJUST 0x100000 // killough 10/98: items which are left-justified
#define S_CREDIT  0x200000  // killough 10/98: credit
#define S_BADVID  0x400000  // killough 12/98: video mode change error
#define S_CHOICE  0x800000  // this item has several values

/* S_SHOWDESC  = the set of items whose description should be displayed
 * S_SHOWSET   = the set of items whose setting should be displayed
 * S_STRING    = the set of items whose settings are strings -- killough 10/98:
 * S_HASDEFPTR = the set of items whose var field points to default array
 */

#define S_SHOWDESC (S_TITLE|S_YESNO|S_CRITEM|S_COLOR|S_CHAT|S_RESET|S_PREV|S_NEXT|S_KEY|S_WEAP|S_NUM|S_FILE|S_CREDIT|S_CHOICE)

#define S_SHOWSET  (S_YESNO|S_CRITEM|S_COLOR|S_CHAT|S_KEY|S_WEAP|S_NUM|S_FILE|S_CHOICE)

#define S_STRING (S_CHAT|S_FILE)

#define S_HASDEFPTR (S_STRING|S_YESNO|S_NUM|S_WEAP|S_COLOR|S_CRITEM|S_CHOICE)

/****************************
 *
 * The setup_group enum is used to show which 'groups' keys fall into so
 * that you can bind a key differently in each 'group'.
 */

typedef enum {
  m_null,       // Has no meaning; not applicable
  m_scrn,       // A key can not be assigned to more than one action
  m_map,        // in the same group. A key can be assigned to one
  m_menu,       // action in one group, and another action in another.
} setup_group;

/****************************
 *
 * phares 4/17/98:
 * State definition for each item.
 * This is the definition of the structure for each setup item. Not all
 * fields are used by all items.
 *
 * A setup screen is defined by an array of these items specific to
 * that screen.
 *
 * killough 11/98:
 *
 * Restructured to allow simpler table entries,
 * and to Xref with defaults[] array in m_misc.c.
 * Moved from m_menu.c to m_menu.h so that m_misc.c can use it.
 */

typedef struct setup_menu_s
{
  const char  *m_text;  /* text to display */
  int         m_flags;  /* phares 4/17/98: flag bits S_* (defined above) */
  setup_group m_group;  /* Group */
  short       m_x;      /* screen x position (left is 0) */
  short       m_y;      /* screen y position (top is 0) */

  union  /* killough 11/98: The first field is a union of several types */
  {
    const void          *var;   /* generic variable */
    int                 *m_key; /* key value, or 0 if not shown */
    const char          *name;  /* name */
    struct default_s    *def;   /* default[] table entry */
    struct setup_menu_s *menu;  /* next or prev menu */
  } var;

  void (*action)(void); /* killough 10/98: function to call after changing */
  const char **selectstrings; /* list of strings for choice value */
} setup_menu_t;

#endif
