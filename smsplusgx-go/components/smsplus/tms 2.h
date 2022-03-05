/******************************************************************************
 *  Sega Master System / GameGear Emulator
 *  Copyright (C) 1998-2007  Charles MacDonald
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
 *   TMS9918 and legacy video mode support.
 *
 ******************************************************************************/

#ifndef _TMS_H_
#define _TMS_H_

/* Global variables */
extern int text_counter;

/* Function prototypes */
extern void make_tms_tables(void);
extern void render_bg_tms(int line);
extern void render_obj_tms(int line);
extern void parse_line(int line);

#endif /* _TMS_H_ */
