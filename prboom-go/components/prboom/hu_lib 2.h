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
 * DESCRIPTION:  none
 *
 *-----------------------------------------------------------------------------*/

#ifndef __HULIB__
#define __HULIB__

// We are referring to patches.
#include "r_defs.h"
#include "v_video.h"  //jff 2/16/52 include color range defs


/* background and foreground screen numbers
 * different from other modules. */
#define BG      1
#define FG      0

/* font stuff
 * #define HU_CHARERASE    KEYD_BACKSPACE / not used               / phares
 */

#define HU_MAXLINES   4
#define HU_MAXLINELENGTH  80
#define HU_REFRESHSPACING 8 /*jff 2/26/98 space lines in text refresh widget*/
/*jff 2/26/98 maximum number of messages allowed in refresh list */
#define HU_MAXMESSAGES 4

/*
 * Typedefs of widgets
 */

/* Text Line widget
 *  (parent of Scrolling Text and Input Text widgets) */
typedef struct
{
  // left-justified position of scrolling text window
  int   x;
  int   y;

  const patchnum_t* f;                    // font
  int   sc;                             // start character
  //const char *cr;                       //jff 2/16/52 output color range
  // Proff - Made this an int again. Needed for OpenGL
  int   cm;                         //jff 2/16/52 output color range

  // killough 1/23/98: Support multiple lines:
  #define MAXLINES 2

  int   linelen;
  char  l[HU_MAXLINELENGTH*MAXLINES+1]; // line of text
  int   len;                            // current line length

  // whether this line needs to be udpated
  int   needsupdate;

} hu_textline_t;



// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
  hu_textline_t l[HU_MAXLINES]; // text lines to draw
  int     h;                    // height in lines
  int     cl;                   // current line number

  // pointer to boolean stating whether to update window
  boolean*    on;
  boolean   laston;             // last value of *->on.

} hu_stext_t;

//jff 2/26/98 new widget to display last hud_msg_lines of messages
// Message refresh window widget
typedef struct
{
  hu_textline_t l[HU_MAXMESSAGES]; // text lines to draw
  int     nl;                          // height in lines
  int     nr;                          // total height in rows
  int     cl;                          // current line number

  int x,y,w,h;                         // window position and size
  const patchnum_t *bg;                  // patches for background

  // pointer to boolean stating whether to update window
  boolean*    on;
  boolean   laston;             // last value of *->on.

} hu_mtext_t;



// Input Text Line widget
//  (child of Text Line widget)
typedef struct
{
  hu_textline_t l;    // text line to input on

  // left margin past which I am not to delete characters
  int     lm;

  // pointer to boolean stating whether to update window
  boolean*    on;
  boolean   laston;   // last value of *->on;

} hu_itext_t;


//
// Widget creation, access, and update routines
//

//
// textline code
//

// clear a line of text
void HUlib_clearTextLine(hu_textline_t *t);

void HUlib_initTextLine
(
  hu_textline_t *t,
  int x,
  int y,
  const patchnum_t *f,
  int sc,
  int cm    //jff 2/16/98 add color range parameter
);

// returns success
boolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);

// draws tline
void HUlib_drawTextLine(hu_textline_t *l, boolean drawcursor);

// erases text line
void HUlib_eraseTextLine(hu_textline_t *l);


//
// Scrolling Text window widget routines
//

// initialize an stext widget
void HUlib_initSText
( hu_stext_t* s,
  int   x,
  int   y,
  int   h,
  const patchnum_t* font,
  int   startchar,
  int cm,   //jff 2/16/98 add color range parameter
  boolean*  on );

// add a text message to an stext widget
void HUlib_addMessageToSText(hu_stext_t* s, const char* prefix, const char* msg);

// draws stext
void HUlib_drawSText(hu_stext_t* s);

// erases all stext lines
void HUlib_eraseSText(hu_stext_t* s);

//jff 2/26/98 message refresh widget
// initialize refresh text widget
void HUlib_initMText(hu_mtext_t *m, int x, int y, int w, int h, const patchnum_t* font,
         int startchar, int cm, const patchnum_t* bgfont, boolean *on);

//jff 2/26/98 message refresh widget
// add a text message to refresh text widget
void HUlib_addMessageToMText(hu_mtext_t* m, const char* prefix, const char* msg);

//jff 2/26/98 new routine to display a background on which
// the list of last hud_msg_lines are displayed
void HUlib_drawMBg
( int x,
  int y,
  int w,
  int h,
  const patchnum_t* bgp
);

//jff 2/26/98 message refresh widget
// draws mtext
void HUlib_drawMText(hu_mtext_t* m);

//jff 4/28/98 erases behind message list
void HUlib_eraseMText(hu_mtext_t* m);

// Input Text Line widget routines
void HUlib_initIText
( hu_itext_t* it,
  int   x,
  int   y,
  const patchnum_t* font,
  int   startchar,
  int cm,   //jff 2/16/98 add color range parameter
  boolean*  on );

// resets line and left margin
void HUlib_resetIText(hu_itext_t* it);

// left of left-margin
void HUlib_addPrefixToIText
( hu_itext_t* it,
  char*   str );

// whether eaten
boolean HUlib_keyInIText
( hu_itext_t* it,
  unsigned char ch );

void HUlib_drawIText(hu_itext_t* it);

// erases all itext lines
void HUlib_eraseIText(hu_itext_t* it);

#endif
