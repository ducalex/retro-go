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
 *   Sega Master System manager
 *
 ******************************************************************************/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define APP_NAME            "SMS Plus"
#define APP_VERSION         "1.3"

#define PALETTE_SIZE        0x20

/* Mask for removing unused pixel data */
#define PIXEL_MASK          0x1F

/* These can be used for 'input.pad[]' */
#define INPUT_UP            0x00000001
#define INPUT_DOWN          0x00000002
#define INPUT_LEFT          0x00000004
#define INPUT_RIGHT         0x00000008
#define INPUT_BUTTON1       0x00000010
#define INPUT_BUTTON2       0x00000020

/* These can be used for 'input.system' */
#define INPUT_START         0x00000001  /* Game Gear only */
#define INPUT_PAUSE         0x00000002  /* Master System only */
#define INPUT_RESET         0x00000004  /* Master System only */

enum {
  SRAM_SAVE   = 0,
  SRAM_LOAD   = 1
};


/* User input structure */
typedef struct
{
  uint8 pad[2];
  int analog[2][2];
  uint8 system;
} input_t;

/* Game image structure */
typedef struct
{
  uint8 *rom;
  uint8 loaded;
  uint32 size;
  uint8 pages;
  uint32 crc;
  uint32 sram_crc;
  int8 mapper;
  uint8 *sram; // [0x8000];
  uint8 fcr[4];
} cart_t;

/* Bitmap structure */
typedef struct
{
  unsigned char *data;
  int width;
  int height;
  int pitch;
  //int depth;
  int granularity;
  struct {
    int x, y, w, h;
    int ox, oy, ow, oh;
    int changed;
  } viewport;
} bitmap_t;

typedef struct
{
  char version[15];
  int sndrate;
  int country;
  int console;
  int display;
  int fm;
  int codies;
  int16 xshift;
  int16 yshift;
  int16 xscale;
  int16 yscale;
  uint8 tv_mode;
  uint8 aspect;
  uint8 overscan;
  uint8 render;
  uint8 ntsc;
  uint8 sms_pal;
  uint8 tms_pal;
  uint8 use_bios;
  uint8 spritelimit;
  uint8 extra_gg;
} option_t;

/* Global variables */
extern bitmap_t bitmap;   /* Display bitmap */
extern cart_t cart;       /* Game cartridge data */
extern input_t input;     /* Controller input */
extern option_t option;   /* EMulator configuration */

/* Function prototypes */
extern void system_frame(int skip);
extern void system_reset_config();
extern void system_shutdown(void);
extern void system_reset(void);
extern void system_manage_sram(uint8 *sram, int slot, int mode);
extern void system_poweron(void);
extern void system_poweroff(void);

#endif /* _SYSTEM_H_ */
