/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
 *
 *  additionnal code by Eke-Eke (SMS Plus GX)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   H/V counter
 *
 ******************************************************************************/

#ifndef _HVC_H_
#define _HVC_H_

/* fixed hc table (thanks to FluBBa) */
extern uint8 hc_256[228];

extern uint8 vc_ntsc_192[];

extern uint8 vc_ntsc_224[];

extern uint8 vc_ntsc_240[];

extern uint8 vc_pal_192[];

extern uint8 vc_pal_224[] ;

extern uint8 vc_pal_240[];

extern const uint8 * vc_table[2][3];

#endif /* _HVC_H_ */
