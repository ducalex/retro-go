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
** nes/ppu.h: Graphics emulation header
**
*/

#pragma once

/* PPU Memory defines */
#define PPU_ADDRSPACE 0x4000
#define PPU_PAGESIZE  0x400
#define PPU_PAGEMASK  0x3FF
#define PPU_PAGESHIFT (10)
#define PPU_PAGECOUNT (PPU_ADDRSPACE / PPU_PAGESIZE)

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

/* Some mappers need to hook into the PPU's internals */
typedef void (*ppu_latchfunc_t)(uint32 address, uint8 value);
typedef uint8 (*ppu_vreadfunc_t)(uint32 address, uint8 value);

typedef enum
{
   PPU_DRAW_SPRITES,
   PPU_DRAW_BACKGROUND,
   PPU_LIMIT_SPRITES,
   PPU_OPTIONS_COUNT,
} ppu_option_t;

typedef enum
{
   PPU_MIRROR_SCR0 = 0,
   PPU_MIRROR_SCR1 = 1,
   PPU_MIRROR_HORI = 2,
   PPU_MIRROR_VERT = 3,
   PPU_MIRROR_FOUR = 4,
} ppu_mirror_t;

typedef struct
{
   uint8 y_loc;
   uint8 tile;
   uint8 attr;
   uint8 x_loc;
} ppu_obj_t;

typedef struct
{
   /* The NES has only 2 nametables, but we allocate 4 for mappers to use */
   uint8 *nametab; // [PPU_PAGESIZE * 4];

   /* Sprite memory */
   uint8 oam[256];

   /* Internal palette */
   uint8 palette[32];

   /* VRAM (CHR RAM/ROM) paging */
   uint8 *page[PPU_PAGECOUNT];

   /* Hardware registers */
   uint8 ctrl0, ctrl1, stat, oam_addr, nametab_base;
   uint8 latch, vdata_latch, tile_xofs, flipflop;
   int   vaddr, vaddr_latch, vaddr_inc;
   uint8 nt_map[4];

   int  obj_height, obj_base, bg_base;
   bool left_bg_on, left_obj_on;
   bool bg_on, obj_on;

   bool strikeflag;
   uint32 strike_cycle;

   int scanlines;
   int scanline;

   /* Determines if left column can be cropped/blanked */
   int left_bg_counter;

   /* Callbacks for naughty mappers */
   ppu_latchfunc_t latchfunc; // For MMC2 and MMC4
   ppu_vreadfunc_t vreadfunc; // For MMC5

   bool vram_accessible;
   bool vram_present;

   /* Misc runtime options */
   int options[16];
} ppu_t;

/* Mirroring / Paging */
void ppu_setpage(uint32 page_num, uint8 *location);
void ppu_setnametable(uint8 index, uint8 table);
void ppu_setmirroring(ppu_mirror_t type);
uint8 *ppu_getpage(uint32 page_num);
uint8 *ppu_getnametable(uint8 table);

/* Control */
ppu_t *ppu_init(void);
void ppu_reset(void);
void ppu_shutdown(void);
bool ppu_enabled(void);
void ppu_setopt(ppu_option_t n, int val);
int ppu_getopt(ppu_option_t n);

void ppu_setlatchfunc(ppu_latchfunc_t func);
void ppu_setvreadfunc(ppu_vreadfunc_t func);

void ppu_setcontext(const ppu_t *src);
void ppu_getcontext(ppu_t *dst);

/* IO */
uint8 ppu_read(uint32 address);
void ppu_write(uint32 address, uint8 value);

/* Rendering */
void ppu_renderline(uint8 *bmp, int scanline, bool draw_flag);
void ppu_endline(void);

/* Debugging */
void ppu_dumppattern(uint8 *bmp, int table_num, int x_loc, int y_loc, int col);
void ppu_dumpoam(uint8 *bmp, int x_loc, int y_loc);
