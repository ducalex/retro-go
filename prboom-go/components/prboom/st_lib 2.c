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
 *      The status bar widget code.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_main.h"
#include "lprintf.h"

int sts_always_red;      //jff 2/18/98 control to disable status color changes
int sts_pct_always_gray; // killough 2/21/98: always gray %'s? bug or feature?

//
// STlib_init()
//
void STlib_init(void)
{
  // cph - no longer hold STMINUS pointer
}

//
// STlib_initNum()
//
// Initializes an st_number_t widget
//
// Passed the widget, its position, the patches for the digits, a pointer
// to the value displayed, a pointer to the on/off control, and the width
// Returns nothing
//
void STlib_initNum
( st_number_t* n,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  boolean* on,
  int     width )
{
  n->x  = x;
  n->y  = y;
  n->oldnum = 0;
  n->width  = width;
  n->num  = num;
  n->on = on;
  n->p  = pl;
}

/*
 * STlib_drawNum()
 *
 * A fairly efficient way to draw a number based on differences from the
 * old number.
 *
 * Passed a st_number_t widget, a color range for output, and a flag
 * indicating whether refresh is needed.
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - const pointer to colour trans table, made function static
 */
static void STlib_drawNum
( st_number_t*  n,
  int cm,
  boolean refresh )
{

  int   numdigits = n->width;
  int   num = *n->num;

  int   w = n->p[0].width;
  int   h = n->p[0].height;
  int   x = n->x;

  int   neg;

  // leban 1/20/99:
  // strange that somebody went through all the work to draw only the
  // differences, and then went and constantly redrew all the numbers.
  // return without drawing if the number didn't change and the bar
  // isn't refreshing.
  if(n->oldnum == num && !refresh)
    return;

  // CPhipps - compact some code, use num instead of *n->num
  if ((neg = (n->oldnum = num) < 0))
  {
    if (numdigits == 2 && num < -9)
      num = -9;
    else if (numdigits == 3 && num < -99)
      num = -99;

    num = -num;
  }

  // clear the area
  x = n->x - numdigits*w;

#ifdef RANGECHECK
  if (n->y - ST_Y < 0)
    I_Error("STlib_drawNum: n->y - ST_Y < 0");
#endif

  V_CopyRect(x, n->y - ST_Y, BG, w*numdigits, h, x, n->y, FG, VPT_STRETCH);

  // if non-number, do not draw it
  if (num == 1994)
    return;

  x = n->x;

  //jff 2/16/98 add color translation to digit output
  // in the special case of 0, you draw 0
  if (!num)
    // CPhipps - patch drawing updated, reformatted
    V_DrawNumPatch(x - w, n->y, FG, n->p[0].lumpnum, cm,
       (((cm!=CR_DEFAULT) && !sts_always_red) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);

  // draw the new number
  //jff 2/16/98 add color translation to digit output
  while (num && numdigits--) {
    // CPhipps - patch drawing updated, reformatted
    x -= w;
    V_DrawNumPatch(x, n->y, FG, n->p[num % 10].lumpnum, cm,
       (((cm!=CR_DEFAULT) && !sts_always_red) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);
    num /= 10;
  }

  // draw a minus sign if necessary
  //jff 2/16/98 add color translation to digit output
  // cph - patch drawing updated, load by name instead of acquiring pointer earlier
  if (neg)
    V_DrawNamePatch(x - w, n->y, FG, "STTMINUS", cm,
       (((cm!=CR_DEFAULT) && !sts_always_red) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);
}

/*
 * STlib_updateNum()
 *
 * Draws a number conditionally based on the widget's enable
 *
 * Passed a number widget, the output color range, and a refresh flag
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - make that pointer const
 */
void STlib_updateNum
( st_number_t*    n,
  int cm,
  boolean   refresh )
{
  if (*n->on) STlib_drawNum(n, cm, refresh);
}

//
// STlib_initPercent()
//
// Initialize a st_percent_t number with percent sign widget
//
// Passed a st_percent_t widget, the position, the digit patches, a pointer
// to the number to display, a pointer to the enable flag, and patch
// for the percent sign.
// Returns nothing.
//
void STlib_initPercent
( st_percent_t* p,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  boolean* on,
  const patchnum_t* percent )
{
  STlib_initNum(&p->n, x, y, pl, num, on, 3);
  p->p = percent;
}

/*
 * STlib_updatePercent()
 *
 * Draws a number/percent conditionally based on the widget's enable
 *
 * Passed a precent widget, the output color range, and a refresh flag
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps - const for pointer to the colour translation table
 */

void STlib_updatePercent
( st_percent_t*   per,
  int cm,
  int refresh )
{
  if (*per->n.on && (refresh || (per->n.oldnum != *per->n.num))) {
    // killough 2/21/98: fix percents not updated;
    /* CPhipps - make %'s only be updated if number changed */
    // CPhipps - patch drawing updated
    V_DrawNumPatch(per->n.x, per->n.y, FG, per->p->lumpnum,
       sts_pct_always_gray ? CR_GRAY : cm,
       (sts_always_red ? VPT_NONE : VPT_TRANS) | VPT_STRETCH);
  }

  STlib_updateNum(&per->n, cm, refresh);
}

//
// STlib_initMultIcon()
//
// Initialize a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys.
//
// Passed a st_multicon_t widget, the position, the graphic patches, a pointer
// to the numbers representing what to display, and pointer to the enable flag
// Returns nothing.
//
void STlib_initMultIcon
( st_multicon_t* i,
  int x,
  int y,
  const patchnum_t* il,
  int* inum,
  boolean* on )
{
  i->x  = x;
  i->y  = y;
  i->oldinum  = -1;
  i->inum = inum;
  i->on = on;
  i->p  = il;
}

//
// STlib_updateMultIcon()
//
// Draw a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys. Displays each when the control
// numbers change or refresh is true
//
// Passed a st_multicon_t widget, and a refresh flag
// Returns nothing.
//
void STlib_updateMultIcon
( st_multicon_t*  mi,
  boolean   refresh )
{
  int w;
  int h;
  int x;
  int y;

  if (*mi->on && (mi->oldinum != *mi->inum || refresh))
  {
    if (mi->oldinum != -1)
    {
      x = mi->x - mi->p[mi->oldinum].leftoffset;
      y = mi->y - mi->p[mi->oldinum].topoffset;
      w = mi->p[mi->oldinum].width;
      h = mi->p[mi->oldinum].height;

#ifdef RANGECHECK
      if (y - ST_Y < 0)
        I_Error("STlib_updateMultIcon: y - ST_Y < 0");
#endif

      V_CopyRect(x, y-ST_Y, BG, w, h, x, y, FG, VPT_STRETCH);
    }
    if (*mi->inum != -1)  // killough 2/16/98: redraw only if != -1
      V_DrawNumPatch(mi->x, mi->y, FG, mi->p[*mi->inum].lumpnum, CR_DEFAULT, VPT_STRETCH);
    mi->oldinum = *mi->inum;
  }
}

//
// STlib_initBinIcon()
//
// Initialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Passed a st_binicon_t widget, the position, the digit patches, a pointer
// to the flags representing what is displayed, and pointer to the enable flag
// Returns nothing.
//
void STlib_initBinIcon
( st_binicon_t* b,
  int x,
  int y,
  const patchnum_t* i,
  boolean* val,
  boolean* on )
{
  b->x  = x;
  b->y  = y;
  b->oldval = 0;
  b->val  = val;
  b->on = on;
  b->p  = i;
}

//
// STlib_updateBinIcon()
//
// DInitialize a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons, that are present or not.
//
// Draw a st_binicon_t widget, used for a multinumber display
// like the status bar's weapons that are present or not. Displays each
// when the control flag changes or refresh is true
//
// Passed a st_binicon_t widget, and a refresh flag
// Returns nothing.
//
void STlib_updateBinIcon
( st_binicon_t*   bi,
  boolean   refresh )
{
  int     x;
  int     y;
  int     w;
  int     h;

  if (*bi->on && (bi->oldval != *bi->val || refresh))
  {
    x = bi->x - bi->p->leftoffset;
    y = bi->y - bi->p->topoffset;
    w = bi->p->width;
    h = bi->p->height;

#ifdef RANGECHECK
    if (y - ST_Y < 0)
      I_Error("STlib_updateBinIcon: y - ST_Y < 0");
#endif

    if (*bi->val)
      V_DrawNumPatch(bi->x, bi->y, FG, bi->p->lumpnum, CR_DEFAULT, VPT_STRETCH);
    else
      V_CopyRect(x, y-ST_Y, BG, w, h, x, y, FG, VPT_STRETCH);

    bi->oldval = *bi->val;
  }
}
