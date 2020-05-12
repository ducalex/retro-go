/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes_ppu.h
**
** NES Picture Processing Unit (PPU) emulation header file
** $Id: nes_ppu.h,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#ifndef _NES_PPU_H_
#define _NES_PPU_H_

#include <bitmap.h>

/* PPU register defines */
#define  PPU_CTRL0            0x2000
#define  PPU_CTRL1            0x2001
#define  PPU_STAT             0x2002
#define  PPU_OAMADDR          0x2003
#define  PPU_OAMDATA          0x2004
#define  PPU_SCROLL           0x2005
#define  PPU_VADDR            0x2006
#define  PPU_VDATA            0x2007

#define  PPU_OAMDMA           0x4014

/* $2000 */
#define  PPU_CTRL0F_NMI       0x80
#define  PPU_CTRL0F_OBJ16     0x20
#define  PPU_CTRL0F_BGADDR    0x10
#define  PPU_CTRL0F_OBJADDR   0x08
#define  PPU_CTRL0F_ADDRINC   0x04
#define  PPU_CTRL0F_NAMETAB   0x03

/* $2001 */
#define  PPU_CTRL1F_OBJON     0x10
#define  PPU_CTRL1F_BGON      0x08
#define  PPU_CTRL1F_OBJMASK   0x04
#define  PPU_CTRL1F_BGMASK    0x02

/* $2002 */
#define  PPU_STATF_VBLANK     0x80
#define  PPU_STATF_STRIKE     0x40
#define  PPU_STATF_MAXSPRITE  0x20

/* Sprite attribute byte bitmasks */
#define  OAMF_VFLIP           0x80
#define  OAMF_HFLIP           0x40
#define  OAMF_BEHIND          0x20

/* Maximum number of sprites per horizontal scanline */
#define  PPU_MAXSPRITE        8

/* Predefined input palette count */
#define  PPU_PAL_COUNT        6


/* some mappers do *dumb* things */
typedef void (*ppulatchfunc_t)(uint32 address, uint8 value);

typedef struct ppu_s
{
   /* big nasty memory chunks */
   uint8 nametab[0x1000];
   uint8 oam[256];
   uint8 palette[32];
   uint8 *page[16];

   /* hardware registers */
   uint8 ctrl0, ctrl1, stat, oam_addr;
   uint32 vaddr, vaddr_latch;
   int tile_xofs, flipflop;
   int vaddr_inc;
   uint32 tile_nametab;

   uint8 obj_height;
   uint32 obj_base, bg_base;

   bool bg_on, obj_on;
   bool obj_mask, bg_mask;

   uint8 latch, vdata_latch;

   bool strikeflag;
   uint32 strike_cycle;

   /* callbacks for naughty mappers */
   ppulatchfunc_t latchfunc;

   /* copy of our current palette */
   rgb_t curpal[256];

   bool vram_accessible;
   bool vram_present;
   bool drawsprites;
   bool limitsprites;
} ppu_t;

typedef struct
{
    char  name[16];
    uint8 data[192]; // rgb_t
} palette_t;

/* Mirroring / Paging */
extern void ppu_mirrorhipages(void);
extern void ppu_mirror(int nt1, int nt2, int nt3, int nt4);
extern void ppu_setpage(int size, int page_num, uint8 *location);
extern uint8 *ppu_getpage(int page);

/* Control */
extern void ppu_reset();
extern bool ppu_enabled(void);
extern void ppu_scanline(bitmap_t *bmp, int scanline, bool draw_flag);
extern void ppu_endscanline(int scanline);
extern void ppu_checknmi();

extern ppu_t *ppu_create(void);
extern void ppu_destroy(ppu_t *ppu);
extern void ppu_setlatchfunc(ppulatchfunc_t func);

extern void ppu_getcontext(ppu_t *dest_ppu);
extern void ppu_setcontext(ppu_t *src_ppu);

/* IO */
extern uint8 ppu_read(uint32 address);
extern void ppu_write(uint32 address, uint8 value);

/* rendering */
extern void ppu_setpal(ppu_t *ppu, rgb_t *pal);
extern void ppu_setnpal(int n);
extern const palette_t *ppu_getnpal(int n);

/* bleh */
extern void ppu_dumppattern(bitmap_t *bmp, int table_num, int x_loc, int y_loc, int col);
extern void ppu_dumpoam(bitmap_t *bmp, int x_loc, int y_loc);
extern void ppu_displaysprites(bool display);
extern void ppu_limitsprites(bool limit);

/* PPU debug drawing */
#define  GUI_FIRSTENTRY 192

enum
{
   GUI_BLACK = GUI_FIRSTENTRY,
   GUI_DKGRAY,
   GUI_GRAY,
   GUI_LTGRAY,
   GUI_WHITE,
   GUI_RED,
   GUI_GREEN,
   GUI_BLUE,
   GUI_LASTENTRY
};

#define  GUI_TOTALCOLORS   (GUI_LASTENTRY - GUI_FIRSTENTRY)

#endif /* _NES_PPU_H_ */
