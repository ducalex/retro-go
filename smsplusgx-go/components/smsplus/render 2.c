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
#include "hvc.h"

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
static uint8 sms_cram_expand_table[4];
static uint8 gg_cram_expand_table[16];

/* Internal buffer for drawing non 8-bit displays */
static uint8 internal_buffer[0x200];

/* Precalculated pixel table */
static uint16 pixel[PALETTE_SIZE];
static int pal_dirty = 1;

/* Region-specific drawing area */
static int active_scanlines;
static int active_border[3];
const uint8 *vc_table[3];

static uint8 object_index_count;

/* CRAM palette in TMS compatibility mode */
static const uint8 tms_crom[] =
{
  0x00, 0x00, 0x08, 0x0C,
  0x10, 0x30, 0x01, 0x3C,
  0x02, 0x03, 0x05, 0x0F,
  0x04, 0x33, 0x15, 0x3F
};

/* original TMS palette for SG-1000 & Colecovision */
static const uint8 tms_palette[16*3][3] =
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
static uint32 atex[4] =
{
  0x00000000,
  0x10101010,
  0x20202020,
  0x30303030,
};

/* Pixel look-up table */
static const uint8 *lut; // )[0x10000];

/* Bitplane to packed pixel LUT */
static const uint32 *bp_lut; // 0x10000

static inline void parse_satb(int line);



/* Macros to access memory 32-bits at a time (from MAME's drawgfx.c) */
// ESP32 supports unaligned access
// #define ALIGN_DWORD 0

#ifdef ALIGN_DWORD
static inline uint32 read_dword(void *address)
{
  if ((uint32)address & 3)
  {
#ifndef IS_BIG_ENDIAN  /* little endian version */
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

static inline void write_dword(void *address, uint32 data)
{
  if ((uint32)address & 3)
  {
#ifndef IS_BIG_ENDIAN
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

  /* Generate 64k of data for the look up table */
  uint8 *_lut = malloc(0x10000);

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
      _lut[(bx << 8) | (sx)] = c;
    }
  }
  lut = _lut;


  /* Make bitplane to pixel lookup table */
  uint32 *_bp_lut = malloc(0x10000 * 4);

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
#ifndef IS_BIG_ENDIAN
    _bp_lut[(j << 8) | (i)] = out;
#else
    _bp_lut[(i << 8) | (j)] = out;
#endif
  }
  bp_lut = _bp_lut;

  sms_cram_expand_table[0] =  0;
  sms_cram_expand_table[1] = (5 << 3)  + (1 << 2);
  sms_cram_expand_table[2] = (15 << 3) + (1 << 2);
  sms_cram_expand_table[3] = (27 << 3) + (1 << 2);

  for(i = 0; i < 16; i++)
  {
    gg_cram_expand_table[i] = (i << 4) | i;
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

  /* Pick default render routine */
  if (vdp.reg[0] & 4)
  {
    render_bg = render_bg_sms;
    render_obj = render_obj_sms;
  }
  else
  {
    render_bg = render_bg_tms;
    render_obj = render_obj_tms;
  }

  if (sms.display == 0) // NTSC
  {
    active_scanlines = 243;
    active_border[0] = 24;
    active_border[1] = 8;
    active_border[2] = 0;
    vc_table[0] = vc_ntsc_192;
    vc_table[1] = vc_ntsc_224;
    vc_table[2] = vc_ntsc_240;
  }
  else // PAL
  {
    active_scanlines = 293;
    active_border[0] = 48;
    active_border[1] = 32;
    active_border[2] = 24;
    vc_table[0] = vc_pal_192;
    vc_table[1] = vc_pal_224;
    vc_table[2] = vc_pal_240;
  }
}

static int prev_line = -1;
static int skip_render = 0;

void render_mode(int skip)
{
    skip_render = skip;
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
  int top_border = active_border[vdp.extended];
  int vline = (line + top_border) % vdp.lpf;
  if (vline >= active_scanlines) return;

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

      /* Draw background */
      render_bg(line);

      /* Draw sprites */
      render_obj(line);

      /* Blank leftmost column of display */
      if((vdp.reg[0] & 0x20) && IS_SMS)
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

#if 0
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
#endif

  /* Only draw lines within the video output range ! */
  if (view && !skip_render)
  {
    /* adjust output line */
    if (!overscan)
      vline -= top_border;

    memcpy(
      bitmap.data + (vline * bitmap.pitch),
      internal_buffer,
      bitmap.viewport.w + 2*bitmap.viewport.x
    );
  }
}

static uint8 data[8];

__attribute__((optimize("unroll-loops")))
static inline void* tile_get(int attr, int line)
{
    // ---p cvhn nnnn nnnn
    const uint16 name = attr & 0x1ff;
    const uint16 y = (attr & 0x400) ? (line ^ 7) : line;
    const uint16* ptr = (uint16*)&vdp.vram[(name << 5) | (y << 2) | (0)];
    const uint32 temp = (bp_lut[*ptr] >> 2) | (bp_lut[*(ptr+1)]);

    for (size_t x = 0; x < 8; x++)
        data[(attr & 0x200) ? (x ^ 7) : x] = (temp >> (x << 2)) & 0x0F;

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

#ifdef IS_BIG_ENDIAN
    attr = (attr << 8) | (attr >> 8);
#endif

    /* Expand priority and palette bits */
    atex_mask = atex[(attr >> 11) & 3];

    cache_ptr = tile_get(attr, v_row >> 3);

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

#ifdef IS_BIG_ENDIAN
    attr = (attr << 8) | (attr >> 8);
#endif

    a = (attr >> 7) & 0x30;

    uint8* ptr = (uint8*)tile_get(attr, v_row >> 3);
    for(x = 0; x < shift; x++)
    {
      c = *(ptr + x);
      p[x] = ((c) | (a));
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
      cache_ptr = tile_get(n, yp >> 1);

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
      cache_ptr = tile_get(n, yp);

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
  color = color << 8 | color >> 8;;

  if (pixel[index] != color)
  {
    pixel[index] = color;
    pal_dirty++;
  }
}


static inline void parse_satb(int line)
{
  /* Pointer to sprite attribute table */
  uint8 *st = (uint8 *)&vdp.vram[vdp.satb];

  /* Sprite counter (64 max.) */
  int i = 0;

  /* Line counter value */
  int vc = vc_table[vdp.extended][line];

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

bool render_copy_palette(uint16* palette)
{
  if (pal_dirty == 0 || palette == NULL)
    return false;

  for (int i = 0; i < 256; i += PALETTE_SIZE)
    memcpy(palette + i, pixel, PALETTE_SIZE * 2);

  pal_dirty = 0;
  return true;
}
