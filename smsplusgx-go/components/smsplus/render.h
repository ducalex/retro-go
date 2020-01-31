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
 *   VDP rendering core
 *
 ******************************************************************************/

#ifndef _RENDER_H_
#define _RENDER_H_

/* Pack RGB data into a 16-bit RGB 5:6:5 format */
#define MAKE_PIXEL(r,g,b)   (((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F))

/* Used for blanking a line in whole or in part */
#define BACKDROP_COLOR      (0x10 | (vdp.reg[7] & 0x0F))

extern void (*render_bg)(int line);
extern void (*render_obj)(int line);
extern uint8 *linebuf;
extern uint8 sms_cram_expand_table[4];
extern uint8 gg_cram_expand_table[16];
//extern uint8 bg_name_dirty[0x200];
//extern uint16 bg_name_list[0x200];
//extern uint16 bg_list_index;

extern void render_shutdown(void);
extern void render_init(void);
extern void render_reset(void);
extern void render_mode(int skip_render, int interlace);
extern void render_line(int line);
extern void render_bg_sms(int line);
extern void render_obj_sms(int line);
extern void palette_sync(int index);
extern void render_copy_palette(uint16* palette);

#endif /* _RENDER_H_ */
