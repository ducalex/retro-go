#pragma GCC optimize ("O3")

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

#include "shared.h"

//#include "sms_ntsc.h"

/*** NTSC Filters ***/
//extern sms_ntsc_t sms_ntsc;

/*** Vertical Counter Tables ***/
extern const uint8 * vc_table[2][3];

struct
{
  uint16 yrange;
  uint16 xpos;
  uint16 attr;
  uint16 _pad0;
} object_info[64];

/* Background drawing function */
void (*render_bg)(int line) = NULL;
void (*render_obj)(int line) = NULL;

/* Pointer to output buffer */
uint8 *linebuf;

/* Pixel 8-bit color tables */
uint8 sms_cram_expand_table[4];
uint8 gg_cram_expand_table[16];

/* Dirty pattern info */
//uint8 bg_name_dirty[0x200];     /* 1= This pattern is dirty */
//uint16 bg_name_list[0x200];     /* List of modified pattern indices */
//uint16 bg_list_index;           /* # of modified patterns in list */

/* Internal buffer for drawing non 8-bit displays */
static uint8 internal_buffer[0x200];

/* Precalculated pixel table */
static uint16 pixel[PALETTE_SIZE];

//static uint8* bg_pattern_cache = ESP32_PSRAM + 0x300000; //[0x20000];/* Cached and flipped patterns */


/* Pixel look-up table */
extern const uint8 lut[0x10000];

static uint8 object_index_count;

/* Top Border area height */
static DRAM_ATTR const uint8 active_border[2][3] =
{
  {24, 8,  0},  /* NTSC VDP */
  {48, 32, 24}  /*  PAL VDP */
};

/* Active Scan Area height */
static DRAM_ATTR  const uint16 active_range[2] =
{
  243, /* NTSC VDP */
  294  /*  PAL VDP */
};

/* CRAM palette in TMS compatibility mode */
static DRAM_ATTR const uint8 tms_crom[] =
{
  0x00, 0x00, 0x08, 0x0C,
  0x10, 0x30, 0x01, 0x3C,
  0x02, 0x03, 0x05, 0x0F,
  0x04, 0x33, 0x15, 0x3F
};

/* original TMS palette for SG-1000 & Colecovision */
static DRAM_ATTR  const uint8 tms_palette[16*3][3] =
{
  /* from Sean Young (http://www.smspower.org/dev/docs/tms9918a.txt) */
  {  0,  0,  0},
  {  0,  0,  0},
  { 33,200, 66},
  { 94,220,120},
  { 84, 85,237},
  {125,118,252},
  {212, 82, 77},
  { 66,235,245},
  {252, 85, 84},
  {255,121,120},
  {212,193, 84},
  {230,206,128},
  { 33,176, 59},
  {201, 91,186},
  {204,204,204},
  {255,255,255},

  /* from Omar Cornut (http://www.smspower.org/dev/docs/sg1000.txt) */
  {  0,  0,  0},
  {  0,  0,  0},
  { 32,192, 30},
  { 96,224, 96},
  { 32, 32,224},
  { 64, 96,224},
  {160, 32, 32},
  { 64,192,224},
  {224, 32, 32},
  {224, 64, 64},
  {192,192, 32},
  {192,192,128},
  { 32,128, 32},
  {192, 64,160},
  {160,160,160},
  {224,224,224},

  /* from Richard F. Drushel (http://users.stargate.net/~drushel/pub/coleco/twwmca/wk961118.html) */
  {  0,  0,  0},
  {  0,  0,  0},
  { 71,183, 59},
  {124,207,111},
  { 93, 78,255},
  {128,114,255},
  {182, 98, 71},
  { 93,200,237},
  {215,107, 72},
  {251,143,108},
  {195,205, 65},
  {211,218,118},
  { 62,159, 47},
  {182,100,199},
  {204,204,204},
  {255,255,255}
};

/* Attribute expansion table */
static DRAM_ATTR const uint32 atex[4] =
{
  0x00000000,
  0x10101010,
  0x20202020,
  0x30303030,
};

/* Bitplane to packed pixel LUT */
extern const uint32 bp_lut[0x10000];

static void parse_satb(int line);
static void update_bg_pattern_cache(void);
static inline void remap_8_to_16(int line);



/* Macros to access memory 32-bits at a time (from MAME's drawgfx.c) */
#define ALIGN_DWORD 0

#ifdef ALIGN_DWORD

static __inline__ uint32 read_dword(void *address)
{
  if ((uint32)address & 3)
  {
#ifdef LSB_FIRST  /* little endian version */
    return ( *((uint8 *)address) +
            (*((uint8 *)address+1) << 8)  +
            (*((uint8 *)address+2) << 16) +
            (*((uint8 *)address+3) << 24) );
#else       /* big endian version */
    return ( *((uint8 *)address+3) +
            (*((uint8 *)address+2) << 8)  +
            (*((uint8 *)address+1) << 16) +
            (*((uint8 *)address)   << 24) );
#endif
  }
  else
    return *(uint32 *)address;
}


static __inline__ void write_dword(void *address, uint32 data)
{
  if ((uint32)address & 3)
  {
#ifdef LSB_FIRST
    *((uint8 *)address) =  data;
    *((uint8 *)address+1) = (data >> 8);
    *((uint8 *)address+2) = (data >> 16);
    *((uint8 *)address+3) = (data >> 24);
#else
    *((uint8 *)address+3) =  data;
    *((uint8 *)address+2) = (data >> 8);
    *((uint8 *)address+1) = (data >> 16);
    *((uint8 *)address)   = (data >> 24);
#endif
    return;
  }
  else
    *(uint32 *)address = data;
}
#else
#define read_dword(address) *(uint32 *)address
#define write_dword(address,data) *(uint32 *)address=data
#endif


/****************************************************************************/


void render_shutdown(void)
{
}

/* Initialize the rendering data */
void render_init(void)
{
  int i, j;
  int bx, sx, b, s, bp, bf, sf, c;

  make_tms_tables();

#if 0
  /* Generate 64k of data for the look up table */
  for(bx = 0; bx < 0x100; bx++)
  {
    for(sx = 0; sx < 0x100; sx++)
    {
      /* Background pixel */
      b  = (bx & 0x0F);

      /* Background priority */
      bp = (bx & 0x20) ? 1 : 0;

      /* Full background pixel + priority + sprite marker */
      bf = (bx & 0x7F);

      /* Sprite pixel */
      s  = (sx & 0x0F);

      /* Full sprite pixel, w/ palette and marker bits added */
      sf = (sx & 0x0F) | 0x10 | 0x40;

      /* Overwriting a sprite pixel ? */
      if(bx & 0x40)
      {
        /* Return the input */
        c = bf;
      }
      else
      {
        /* Work out priority and transparency for both pixels */
        if(bp)
        {
          /* Underlying pixel is high priority */
          if(b)
          {
            c = bf | 0x40;
          }
          else
          {
            if(s)
            {
              c = sf;
            }
            else
            {
              c = bf;
            }
          }
        }
        else
        {
          /* Underlying pixel is low priority */
          if(s)
          {
            c = sf;
          }
          else
          {
            c = bf;
          }
        }
      }

      /* Store result */
      lut[(bx << 8) | (sx)] = c;
    }
  }
#endif

#if 0
  /* Make bitplane to pixel lookup table */
  for(i = 0; i < 0x100; i++)
  for(j = 0; j < 0x100; j++)
  {
    int x;
    uint32 out = 0;
    for(x = 0; x < 8; x++)
    {
      out |= (j & (0x80 >> x)) ? (uint32)(8 << (x << 2)) : 0;
      out |= (i & (0x80 >> x)) ? (uint32)(4 << (x << 2)) : 0;
    }
#if LSB_FIRST
    bp_lut[(j << 8) | (i)] = out;
#else
    bp_lut[(i << 8) | (j)] = out;
#endif
  }
 #endif

  sms_cram_expand_table[0] =  0;
  sms_cram_expand_table[1] = (5 << 3)  + (1 << 2);
  sms_cram_expand_table[2] = (15 << 3) + (1 << 2);
  sms_cram_expand_table[3] = (27 << 3) + (1 << 2);

  for(i = 0; i < 16; i++)
  {
    uint8 c = i << 4 | i;
    gg_cram_expand_table[i] = c;
  }
}


/* Reset the rendering data */
void render_reset(void)
{
  int i;

  /* Clear display bitmap */
  memset(bitmap.data, 0, bitmap.pitch * bitmap.height);

  /* Clear palette */
  for(i = 0; i < PALETTE_SIZE; i++)
  {
    palette_sync(i);
  }

  /* Invalidate pattern cache */
  //memset(bg_name_dirty, 0, sizeof(bg_name_dirty));
  //memset(bg_name_list, 0, sizeof(bg_name_list));
  //bg_list_index = 0;
  //memset(bg_pattern_cache, 0, 0x20000 /*sizeof(bg_pattern_cache)*/);

  /* Pick default render routine */
  if (vdp.reg[0] & 4)
  {
    render_bg = render_bg_sms;
    render_obj = render_obj_sms;
    //printf("%s: render_bg = render_bg_sms, render_obj = render_obj_sms\n", __func__);
  }
  else
  {
    render_bg = render_bg_tms;
    render_obj = render_obj_tms;
    //printf("%s: render_bg = render_bg_tms, render_obj = render_obj_tms\n", __func__);
  }
}

static int prev_line = -1;
static int skip_render = 0;
static int interlace_field = -1;

void render_mode(int skip, int field)
{
    skip_render = skip;
    interlace_field = field;
}

/* Draw a line of the display */
IRAM_ATTR void render_line(int line)
{
  int view = 1;
  int overscan = option.overscan;

  /* ensure we have not already rendered this line */
  if (prev_line == line) return;
  prev_line = line;

  /* Ensure we're within the VDP active area (incl. overscan) */
  int top_border = active_border[sms.display][vdp.extended];
  int vline = (line + top_border) % vdp.lpf;
  if (vline >= active_range[sms.display]) return;

  /* adjust for Game Gear screen */
  top_border = top_border + (vdp.height - bitmap.viewport.h) / 2;

  /* Point to current line in output buffer */
  linebuf = &internal_buffer[0];

  /* Sprite limit flag is set at the beginning of the line */
  if (vdp.spr_ovr)
  {
    vdp.spr_ovr = 0;
    vdp.status |= 0x40;
  }

  /* Vertical borders */
  if ((vline < top_border) || (vline >= (bitmap.viewport.h + top_border)))
  {
    /* Sprites are still processed offscreen */
    if ((vdp.mode > 7) && (vdp.reg[1] & 0x40))
      render_obj(line);

    /* Line is only displayed where overscan is emulated */
    view = 0;
    if (overscan && (vline < (bitmap.viewport.h + 2*bitmap.viewport.y)))
    {
      /* Background color */
      memset(linebuf, BACKDROP_COLOR, bitmap.viewport.w + 2*bitmap.viewport.x);
      view = 1;
    }
  }
  /* Active display */
  else
  {
    /* Display enabled ? */
    if (vdp.reg[1] & 0x40)
    {
      /* adjust line horizontal offset */
      if (overscan)
        linebuf += 14;

      /* Update pattern cache */
      update_bg_pattern_cache();

      /* Draw background */
      render_bg(line);

      /* Draw sprites */
      render_obj(line);

      /* Blank leftmost column of display */
      if((vdp.reg[0] & 0x20) && (IS_SMS || IS_MD))
        memset(linebuf, BACKDROP_COLOR, 8);

      /* Horizontal borders */
      if (overscan)
      {
        /* Display background color */
        memset(linebuf - 14, BACKDROP_COLOR, bitmap.viewport.x);
        memset(linebuf - 14 + bitmap.viewport.w + bitmap.viewport.x, BACKDROP_COLOR, bitmap.viewport.x);
      }
    }
    else
    {
      /* Background color */
      memset(linebuf, BACKDROP_COLOR, bitmap.viewport.w + 2*bitmap.viewport.x);
    }
  }

  /* Parse Sprites for next line */
  if (vdp.mode > 7)
    parse_satb(line);
  else
    parse_line(line);

  /* LightGun mark */
  if (sms.device[0] == DEVICE_LIGHTGUN)
  {
    int dy = vdp.line - input.analog[0][1];

    if (abs(dy) < 6)
    {
      int i;
      int start = input.analog[0][0] - 4;
      int end = input.analog[0][0] + 4;
      if (start < 0) start = 0;
      if (end > 255) end = 255;
      for (i=start; i<end+1; i++)
      {
        linebuf[i] = 0xFF;
      }
    }
  }

  /* Only draw lines within the video output range ! */
  if (view)
  {
    /* adjust output line */
    if (!overscan)
      vline -= top_border;

    if (!skip_render &&
        (interlace_field < 0 || ((vline % 2) ^ interlace_field))) {
    //if (option.ntsc)
    //  sms_ntsc_blit(&sms_ntsc, ( SMS_NTSC_IN_T const * )pixel, internal_buffer, bitmap.viewport.w + 2*bitmap.viewport.x, vline);
    //else
      remap_8_to_16(vline);
    }
  }
}

uint8 data[8];
static IRAM_ATTR void* tile_get(short attr, short line)
{
    // ---p cvhn nnnn nnnn
    const uint16 name = attr & 0x1ff;

    // uint16 y = line & 7;
    // if (attr & 0x400)
    // {
    //     y = (y ^ 7);
    // }

    const uint16 y = (attr & 0x400) ? (line ^ 7) : line;

    const uint16* ptr = &vdp.vram[(name << 5) | (y << 2) | (0)];
    const uint16 bp01 = *ptr++;
    const uint16 bp23 = *ptr;
    const uint32 temp = (bp_lut[bp01] >> 2) | (bp_lut[bp23]);

    for(short x = 0; x < 8; x++)
    {
        const uint8 c = (temp >> (x << 2)) & 0x0F;
        const short index = (attr & 0x200) ? (x ^ 7) : x;
        data[index] = (c);
    }

    return data;
}

/* Draw the Master System background */
IRAM_ATTR void render_bg_sms(int line)
{
  int locked = 0;
  int yscroll_mask = (vdp.extended) ? 256 : 224;
  int v_line = (line + vdp.vscroll) % yscroll_mask;
  int v_row  = (v_line & 7) << 3;
  int hscroll = ((vdp.reg[0] & 0x40) && (line < 0x10) && (sms.console != CONSOLE_GG)) ? 0 : (0x100 - vdp.reg[8]);
  int column = 0;
  uint16 attr;
  uint16 nt_addr = (vdp.ntab + ((v_line >> 3) << 6)) & (((sms.console == CONSOLE_SMS) && !(vdp.reg[2] & 1)) ? ~0x400 :0xFFFF);
  uint16 *nt = (uint16 *)&vdp.vram[nt_addr];
  int nt_scroll = (hscroll >> 3);
  int shift = (hscroll & 7);
  uint32 atex_mask;
  uint32 *cache_ptr;
  uint32 *linebuf_ptr = (uint32 *)&linebuf[0 - shift];
  uint8* ctp;

  /* Draw first column (clipped) */
  if(shift)
  {
    int x;

    for(x = shift; x < 8; x++)
      linebuf[(0 - shift) + (x)] = 0;

    column++;
  }

  /* Draw a line of the background */
  for(; column < 32; column++)
  {
    /* Stop vertical scrolling for leftmost eight columns */
    if((vdp.reg[0] & 0x80) && (!locked) && (column >= 24))
    {
      locked = 1;
      v_row = (line & 7) << 3;
      nt = (uint16 *)&vdp.vram[nt_addr];
    }

    /* Get name table attribute word */
    attr = nt[(column + nt_scroll) & 0x1F];

#ifndef LSB_FIRST
    attr = (((attr & 0xFF) << 8) | ((attr & 0xFF00) >> 8));
#endif
    /* Expand priority and palette bits */
    atex_mask = atex[(attr >> 11) & 3];

#if 0
    /* Point to a line of pattern data in cache */
    cache_ptr = (uint32 *)&bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row)];
#else
    // ---p cvhn nnnn nnnn

    // uint8 data[8];
    // uint16 name = attr & 0x1ff;
    //
    // uint16 y = line & 7;
    // if (attr & 0x400)
    // {
    //     y = (y ^ 7);
    // }
    //
    // uint16 bp01 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (0)];
    // uint16 bp23 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (2)];
    // uint32 temp = (bp_lut[bp01] >> 2) | (bp_lut[bp23]);
    //
    // uint8 rot = (attr >> (1 + 8)) & 0x03;
    // for(short x = 0; x < 8; x++)
    // {
    //     uint8 c = (temp >> (x << 2)) & 0x0F;
    //     short index = (attr & 0x2000) ? (x ^ 7) : x;
    //     data[index] = (c);
    //
    //   //dst[0x08000 | (y << 3) | (x ^ 7)] = (c);
    //   //dst[0x10000 | ((y ^ 7) << 3) | (x)] = (c);
    //   //dst[0x18000 | ((y ^ 7) << 3) | (x ^ 7)] = (c);
    // }

    cache_ptr = tile_get(attr, v_row >> 3);

#endif
    /* Copy the left half, adding the attribute bits in */
    write_dword( &linebuf_ptr[(column << 1)] , read_dword( &cache_ptr[0] ) | (atex_mask));

    /* Copy the right half, adding the attribute bits in */
    write_dword( &linebuf_ptr[(column << 1) | (1)], read_dword( &cache_ptr[1] ) | (atex_mask));


  }

  /* Draw last column (clipped) */
  if(shift)
  {
    int x, c, a;

    uint8 *p = &linebuf[(0 - shift)+(column << 3)];

    attr = nt[(column + nt_scroll) & 0x1F];

#ifndef LSB_FIRST
    attr = (((attr & 0xFF) << 8) | ((attr & 0xFF00) >> 8));
#endif
    a = (attr >> 7) & 0x30;

    uint8* ptr = (uint8*)tile_get(attr, v_row >> 3);
    for(x = 0; x < shift; x++)
    {
#if 0
      c = bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row) | (x)];
      p[x] = ((c) | (a));
#else
     c = *(ptr + x);
     p[x] = ((c) | (a));
#endif

    }
  }
}


/* Draw sprites */
IRAM_ATTR void render_obj_sms(int line)
{
  int i,x,start,end,xp,yp,n;
  uint8 sp,bg;
  uint8 *linebuf_ptr;
  uint8 *cache_ptr;

  int width = 8;

  /* Adjust dimensions for double size sprites */
  if(vdp.reg[1] & 0x01)
    width *= 2;

  /* Draw sprites in front-to-back order */
  for(i = 0; i < object_index_count; i++)
  {
    /* Width of sprite */
    start = 0;
    end = width;

    /* Sprite X position */
    xp = object_info[i].xpos;

    /* Sprite Y range */
    yp = object_info[i].yrange;

    /* Pattern name */
    n = object_info[i].attr;

    /* X position shift */
    if(vdp.reg[0] & 0x08) xp -= 8;

    /* Add MSB of pattern name */
    if(vdp.reg[6] & 0x04) n |= 0x0100;

    /* Mask LSB for 8x16 sprites */
    if(vdp.reg[1] & 0x02) n &= 0x01FE;

    /* Point to offset in line buffer */
    linebuf_ptr = (uint8 *)&linebuf[xp];

    /* Clip sprites on left edge */
    if(xp < 0)
      start = (0 - xp);

    /* Clip sprites on right edge */
    if((xp + width) > 256)
      end = (256 - xp);

    /* Draw double size sprite */
    if(vdp.reg[1] & 0x01)
    {
#if 0
      /* Retrieve tile data from cached nametable */
      cache_ptr = (uint8 *)&bg_pattern_cache[(n << 6) | ((yp >> 1) << 3)];
#else
      cache_ptr = tile_get(n, yp >> 1);
#endif

      /* Draw sprite line (at 1/2 dot rate) */
      for(x = start; x < end; x+=2)
      {
        /* Source pixel from cache */
        sp = cache_ptr[(x >> 1)];

        /* Only draw opaque sprite pixels */
        if(sp)
        {
          /* Background pixel from line buffer */
          bg = linebuf_ptr[x];

          /* Look up result */
          linebuf_ptr[x] = linebuf_ptr[x+1] = lut[(bg << 8) | (sp)];

          /* Check sprite collision */
          if ((bg & 0x40) && !(vdp.status & 0x20))
          {
            /* pixel-accurate SPR_COL flag */
            vdp.status |= 0x20;
            vdp.spr_col = (line << 8) | ((xp + x + 13) >> 1);
          }
        }
      }
    }
    else /* Regular size sprite (8x8 / 8x16) */
    {
#if 0
      /* Retrieve tile data from cached nametable */
      cache_ptr = (uint8 *)&bg_pattern_cache[(n << 6) | (yp << 3)];
#else
      cache_ptr = tile_get(n, yp);
#endif

      /* Draw sprite line */
      for(x = start; x < end; x++)
      {
        /* Source pixel from cache */
        sp = cache_ptr[x];

        /* Only draw opaque sprite pixels */
        if(sp)
        {
          /* Background pixel from line buffer */
          bg = linebuf_ptr[x];

          /* Look up result */
          linebuf_ptr[x] = lut[(bg << 8) | (sp)];

          /* Check sprite collision */
          if ((bg & 0x40) && !(vdp.status & 0x20))
          {
            /* pixel-accurate SPR_COL flag */
            vdp.status |= 0x20;
            vdp.spr_col = (line << 8) | ((xp + x + 13) >> 1);
          }
        }
      }
    }
  }
}

/* Update a palette entry */
void palette_sync(int index)
{
  int r, g, b;

  /* VDP Mode */
  if ((vdp.reg[0] & 4) || IS_GG)
  {
    /* Mode 4 or Game Gear TMS mode*/
    if(sms.console == CONSOLE_GG)
    {
      /* GG palette */
      /* ----BBBBGGGGRRRR */
      r = (vdp.cram[(index << 1) | (0)] >> 0) & 0x0F;
      g = (vdp.cram[(index << 1) | (0)] >> 4) & 0x0F;
      b = (vdp.cram[(index << 1) | (1)] >> 0) & 0x0F;

      r = gg_cram_expand_table[r];
      g = gg_cram_expand_table[g];
      b = gg_cram_expand_table[b];
    }
    else
    {
      /* SMS palette */
      /* --BBGGRR */
      r = (vdp.cram[index] >> 0) & 3;
      g = (vdp.cram[index] >> 2) & 3;
      b = (vdp.cram[index] >> 4) & 3;

      r = sms_cram_expand_table[r];
      g = sms_cram_expand_table[g];
      b = sms_cram_expand_table[b];
    }
  }
  else
  {
    /* TMS Mode (16 colors only) */
    int color = index & 0x0F;

    if (sms.console < CONSOLE_SMS)
    {
      /* pick one of the original TMS9918 palettes */
      color += option.tms_pal * 16;

      r = tms_palette[color][0];
      g = tms_palette[color][1];
      b = tms_palette[color][2];
    }
    else
    {
      /* fixed CRAM palette in TMS mode */
      r = (tms_crom[color] >> 0) & 3;
      g = (tms_crom[color] >> 2) & 3;
      b = (tms_crom[color] >> 4) & 3;

      r = sms_cram_expand_table[r];
      g = sms_cram_expand_table[g];
      b = sms_cram_expand_table[b];
    }
  }

  uint16 color = MAKE_PIXEL(r, g, b);
  pixel[index] = (color >> 8) | (color << 8);
}


static IRAM_ATTR void parse_satb(int line)
{
  /* Pointer to sprite attribute table */
  uint8 *st = (uint8 *)&vdp.vram[vdp.satb];

  /* Sprite counter (64 max.) */
  int i = 0;

  /* Line counter value */
  int vc = vc_table[sms.display][vdp.extended][line];

  /* Sprite height (8x8 by default) */
  int yp;
  int height = 8;

  /* Adjust height for 8x16 sprites */
  if(vdp.reg[1] & 0x02)
    height <<= 1;

  /* Adjust height for zoomed sprites */
  if(vdp.reg[1] & 0x01)
    height <<= 1;

  /* Sprite count for current line (8 max.) */
  object_index_count = 0;

  for(i = 0; i < 64; i++)
  {
    /* Sprite Y position */
    yp = st[i];

    /* Found end of sprite list marker for non-extended modes? */
    if(vdp.extended == 0 && yp == 208)
      return;

    /* Wrap Y coordinate for sprites > 240 */
    if(yp > 240) yp -= 256;

    /* Compare sprite position with current line counter */
    yp = vc - yp;

    /* Sprite is within vertical range? */
    if((yp >= 0) && (yp < height))
    {
      /* Sprite limit reached? */
      if (object_index_count == 8)
      {
        /* Flag is set only during active area */
        if (line < vdp.height)
          vdp.spr_ovr = 1;

        /* End of sprite parsing */
        if (option.spritelimit)
          return;
      }

      /* Store sprite attributes for later processing */
      object_info[object_index_count].yrange = yp;
      object_info[object_index_count].xpos = st[0x80 + (i << 1)];
      object_info[object_index_count].attr = st[0x81 + (i << 1)];

      /* Increment Sprite count for current line */
      ++object_index_count;
    }
  }
}

static IRAM_ATTR void update_bg_pattern_cache(void)
{
#if 1
    return;
#else
  int i;
  uint8 x, y;
  uint16 name;

  if(!bg_list_index) return;

  for(i = 0; i < bg_list_index; i++)
  {
    name = bg_name_list[i];
    bg_name_list[i] = 0;

    for(y = 0; y < 8; y++)
    {
      if(bg_name_dirty[name] & (1 << y))
      {
        uint8 *dst = &bg_pattern_cache[name << 6];

        uint16 bp01 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (0)];
        uint16 bp23 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (2)];
        uint32 temp = (bp_lut[bp01] >> 2) | (bp_lut[bp23]);

        for(x = 0; x < 8; x++)
        {
          uint8 c = (temp >> (x << 2)) & 0x0F;
          dst[0x00000 | (y << 3) | (x)] = (c);
          dst[0x08000 | (y << 3) | (x ^ 7)] = (c);
          dst[0x10000 | ((y ^ 7) << 3) | (x)] = (c);
          dst[0x18000 | ((y ^ 7) << 3) | (x ^ 7)] = (c);
        }
      }
    }
    bg_name_dirty[name] = 0;
  }
  bg_list_index = 0;
#endif
}

static inline void remap_8_to_16(int line)
{
    //printf("%s: line=%d\n", __func__, line);

  //int i;
  //uint16 *p = (uint16 *)&bitmap.data[(line * bitmap.pitch)];
#if 0
  //int width = bitmap.width;// + 2*bitmap.viewport.x;
  int width = bitmap.viewport.w + 2*bitmap.viewport.x;

  for(i = 0; i < width; i++)
  {
    p[i] = pixel[ internal_buffer[i] & PIXEL_MASK ];
  }
 #else
    // //uint16* dst = p + bitmap.viewport.x;
    // uint8* src = internal_buffer + bitmap.viewport.x;
    //  for(int x = 0; x < bitmap.viewport.w; ++x)
    //  {
    //    *(p++) = pixel[ *(src++) & PIXEL_MASK ];
    //  }

    uint8* dst = (uint8*)bitmap.data + (line * bitmap.pitch); // + bitmap.viewport.x;
    // uint8* src = internal_buffer + bitmap.viewport.x;
    // for(int x = 0; x < bitmap.viewport.w; ++x)
    // {
    //     *(dst++) = *(src++) & PIXEL_MASK;
    // }
    int width = bitmap.viewport.w + 2*bitmap.viewport.x;
    memcpy(dst, internal_buffer, width);
 #endif
}

void render_copy_palette(uint16* palette)
{
    memcpy(palette, pixel, sizeof(pixel));
}
