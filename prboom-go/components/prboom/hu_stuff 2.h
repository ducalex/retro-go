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
 * DESCRIPTION:  Head up display
 *
 *-----------------------------------------------------------------------------*/

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"

/*
 * Globally visible constants.
 */
#define HU_FONTSTART    '!'     /* the first font characters */
#define HU_FONTEND      (0x7f) /*jff 2/16/98 '_' the last font characters */

/* Calculate # of glyphs in font. */
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)

#define HU_BROADCAST    5

/*#define HU_MSGREFRESH   KEYD_ENTER                                phares */
#define HU_MSGX         0
#define HU_MSGY         0
#define HU_MSGWIDTH     64      /* in characters */
#define HU_MSGHEIGHT    1       /* in lines */

#define HU_MSGTIMEOUT   (4*TICRATE)

/*
 * Heads up text
 */
void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);
void HU_MoveHud(void); // jff 3/9/98 avoid glitch in HUD display

/* killough 5/2/98: moved from m_misc.c: */

/* jff 2/16/98 hud supported automap colors added */
extern int hudcolor_titl;   /* color range of automap level title   */
extern int hudcolor_xyco;   /* color range of new coords on automap */
/* jff 2/16/98 hud text colors, controls added */
extern int hudcolor_mesg;   /* color range of scrolling messages    */
extern int hudcolor_chat;   /* color range of chat lines            */
/* jff 2/26/98 hud message list color and background enable */
extern int hudcolor_list;   /* color of list of past messages                  */
extern int hud_list_bgon;   /* solid window background for list of messages    */
extern int hud_msg_lines;   /* number of message lines in window up to 16      */
extern int hud_distributed; /* whether hud is all in lower left or distributed */
/* jff 2/23/98 hud is currently displayed */
extern int hud_displayed;   /* hud is displayed */
/* jff 2/18/98 hud/status control */
extern int hud_active;      /* hud mode 0=off, 1=small, 2=full          */
extern int hud_nosecrets;   /* status does not list secrets/items/kills */

#endif
