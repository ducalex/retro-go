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
 *  Video Display Processor (VDP) emulation.
 *
 ******************************************************************************/

#include "shared.h"

/* VDP context */
vdp_t vdp;

static void viewport_check(void)
{
  int i;
  int m1 = (vdp.reg[1] >> 4) & 1;
  int m3 = (vdp.reg[1] >> 3) & 1;
  int m2 = (vdp.reg[0] >> 1) & 1;
  int m4 = (vdp.reg[0] >> 2) & 1;

  vdp.mode = (m4 << 3 | m3 << 2 | m2 << 1 | m1 << 0);

  /* Check for extended modes */
  if (sms.console >= CONSOLE_SMS2)
  {
    switch (vdp.mode)
    {
      case 0x0B:  /* Mode 4 extended (224 lines) */
        vdp.height = 224;
        vdp.extended = 1;
        vdp.ntab = ((vdp.reg[2] << 10) & 0x3000) | 0x0700;
        break;

      case 0x0E:  /* Mode 4 extended (240 lines) */
        vdp.height = 240;
        vdp.extended = 2;
        vdp.ntab = ((vdp.reg[2] << 10) & 0x3000) | 0x0700;
        break;

      default:  /* Mode 4 (192 lines) */
        vdp.height = 192;
        vdp.extended = 0;
        vdp.ntab = (vdp.reg[2] << 10) & 0x3800;

        /* invalid text mode (Mode 4) */
        if ((vdp.mode & 0x0B) == 0x09) vdp.mode = 1;
        break;
    }
  }
  else
  {
    /* always use Mode 4 (192 lines) */
    vdp.height = 192;
    vdp.extended = 0;
    vdp.ntab = (vdp.reg[2] << 10) & 0x3800;

    /* invalid text mode (Mode 4) */
    if ((vdp.mode & 0x09) == 0x09) vdp.mode = 1;
  }

  /* update display area */
  if ((sms.console != CONSOLE_GG) || option.extra_gg)
  {
    if(bitmap.viewport.h != vdp.height)
    {
      bitmap.viewport.oh = bitmap.viewport.h;
      bitmap.viewport.h = vdp.height;
      bitmap.viewport.changed = 1;
    }
  }
  else
  {
    /* GG display area is fixed */
    bitmap.viewport.h = 144;
  }

  /* update border area */
  bitmap.viewport.y = 0;
  if (option.overscan)
  {
    int max = sms.display ? 288 : 240;
    bitmap.viewport.y = (max - bitmap.viewport.h) / 2;
  }

  /* check if this is switching in/out of tms */
  if (IS_SMS || IS_GG)
  {
    /* Restore palette */
    for(i = 0; i < PALETTE_SIZE; i++)
    {
      palette_sync(i);
    }
  }

  vdp.pn = (vdp.reg[2] << 10) & 0x3C00;

  if (vdp.mode & 8)
  {
    render_bg  = render_bg_sms;
    render_obj = render_obj_sms;
  }
  else
  {
    render_bg  = render_bg_tms;
    render_obj = render_obj_tms;
  }
}

/* Initialize VDP emulation */
void vdp_init(void)
{
  /* display area */
  if ((sms.console == CONSOLE_GG) && (!option.extra_gg))
  {
    bitmap.viewport.w = 160;
    bitmap.viewport.x = 48;
  }
  else
  {
    bitmap.viewport.w = 256;
    bitmap.viewport.x = 0;
  }

  /* overscan area */
  if (option.overscan)
  {
    bitmap.viewport.x += 14;
  }

  /* number of scanlines */
  vdp.lpf = sms.display ? 313 : 262;

  /* reset viewport */
  viewport_check();
  bitmap.viewport.changed = 1;
}

void vdp_shutdown(void)
{
  //
}

/* Reset VDP emulation */
void vdp_reset(void)
{
  /* reset VDP structure */
  memset(&vdp, 0, sizeof(vdp_t));

  /* number of scanlines */
  vdp.lpf = sms.display ? 313 : 262;

  /* VDP registers default values (usually set by BIOS) */
  if (IS_SMS && (bios.enabled != 3))
  {
    vdp.reg[0]  = 0x36;
    vdp.reg[1]  = 0x80;
    vdp.reg[2]  = 0xFF;
    vdp.reg[3]  = 0xFF;
    vdp.reg[4]  = 0xFF;
    vdp.reg[5]  = 0xFF;
    vdp.reg[6]  = 0xFB;
    vdp.reg[10] = 0xFF;
  }

  /* VDP interrupt */
  if (sms.console == CONSOLE_COLECO)
    vdp.irq = INPUT_LINE_NMI;
  else
    vdp.irq = INPUT_LINE_IRQ0;

  /* reset VDP viewport */
  viewport_check();

  /* reset VDP internals */
  vdp.ct    = (vdp.reg[3] <<  6) & 0x3FC0;
  vdp.pg    = (vdp.reg[4] << 11) & 0x3800;
  vdp.satb  = (vdp.reg[5] << 7) & 0x3F00;
  vdp.sa    = (vdp.reg[5] <<  7) & 0x3F80;
  vdp.sg    = (vdp.reg[6] << 11) & 0x3800;
  vdp.bd    = (vdp.reg[7] & 0x0F);

  bitmap.viewport.changed = 1;
}

void vdp_reg_w(uint8 r, uint8 d)
{
  r &= 0xF;

  /* Store register data */
  vdp.reg[r] = d;

  switch(r)
  {
    case 0x0: /* Mode Control No. 1 */
      if(vdp.hint_pending)
      {
        if(d & 0x10) z80_set_irq_line(0, ASSERT_LINE);
        else z80_set_irq_line(0, CLEAR_LINE);
      }
      viewport_check();
      break;

    case 0x1: /* Mode Control No. 2 */
      if(vdp.vint_pending)
      {
        if(d & 0x20) z80_set_irq_line(vdp.irq, ASSERT_LINE);
        else z80_set_irq_line(vdp.irq, CLEAR_LINE);
      }
      viewport_check();
      break;

    case 0x2: /* Name Table A Base Address */
      viewport_check();
      break;

    case 0x3:
      vdp.ct = (d <<  6) & 0x3FC0;
      break;

    case 0x4:
      vdp.pg = (d << 11) & 0x3800;
      break;

    case 0x5: /* Sprite Attribute Table Base Address */
      vdp.satb = (d << 7) & 0x3F00;
      vdp.sa = (d <<  7) & 0x3F80;
      break;

    case 0x6:
      vdp.sg = (d << 11) & 0x3800;
      break;

    case 0x7:
      vdp.bd = (d & 0x0F);
      break;
  }
}

void vdp_write(int offset, uint8 data)
{
  int index;

  if (((z80_get_elapsed_cycles() + 1) / CYCLES_PER_LINE) > vdp.line)
  {
    /* render next line now BEFORE updating register */
    render_line((vdp.line+1)%vdp.lpf);
  }

  if (offset & 1) /* Control port */
  {
    if(vdp.pending == 0)
    {
      vdp.addr = (vdp.addr & 0x3F00) | (data & 0xFF);
      vdp.latch = data;
      vdp.pending = 1;
    }
    else
    {
      vdp.pending = 0;
      vdp.code = (data >> 6) & 3;
      vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

      if(vdp.code == 0)
      {
        vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
        vdp.addr = (vdp.addr + 1) & 0x3FFF;
      }
      else if(vdp.code == 2)
      {
        vdp_reg_w(data & 0xF, vdp.latch);
      }
    }
  }
  else /* Data port */
  {
    vdp.pending = 0;

    switch(vdp.code)
    {
      case 0: /* VRAM write */
      case 1: /* VRAM write */
      case 2: /* VRAM write */
        vdp.vram[(vdp.addr & 0x3FFF)] = data;
        vdp.buffer = data;
        break;

      case 3: /* CRAM write */
        index = (vdp.addr & 0x1F);
        if(data != vdp.cram[index])
        {
          vdp.cram[index] = data;
          palette_sync(index);
        }
        vdp.buffer = data;
        break;
    }
    vdp.addr = (vdp.addr + 1) & 0x3FFF;
  }
}

uint8 vdp_read(int offset)
{
  uint8 temp;

  if (offset & 1) /* Status flags */
  {
    /* cycle-accurate SPR_OVR and INT flags */
    int cyc   = z80_get_elapsed_cycles();
    int line  = vdp.line;
    if ((cyc / CYCLES_PER_LINE) > line)
    {
      if (line == vdp.height) vdp.status |= 0x80;
      line = (line + 1)%vdp.lpf;
      render_line(line);
    }

    /* low 5 bits return non-zero data (fixes PGA Tour Golf course map introduction) */
    temp = vdp.status | 0x1f;

    /* clear flags */
    vdp.status = 0;
    vdp.pending = 0;
    vdp.vint_pending = 0;
    vdp.hint_pending = 0;
    z80_set_irq_line(vdp.irq, CLEAR_LINE);

    /* cycle-accurate SPR_COL flag */
    if (temp & 0x20)
    {
      uint8 hc = hc_256[(cyc + 1) % CYCLES_PER_LINE];
      if ((line == (vdp.spr_col >> 8)) && ((hc < (vdp.spr_col & 0xff)) || (hc > 0xf3)))
      {
        vdp.status |= 0x20;
        temp &= ~0x20;
      }
    }
  }
  else /* CPU <-> VDP data buffer */
  {
    vdp.pending = 0;
    temp = vdp.buffer;
    vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
    vdp.addr = (vdp.addr + 1) & 0x3FFF;
  }

  return temp;
}

/*--------------------------------------------------------------------------*/
/* Game Gear VDP handlers                           */
/*--------------------------------------------------------------------------*/

void gg_vdp_write(int offset, uint8 data)
{
  if (((z80_get_elapsed_cycles() + 1) / CYCLES_PER_LINE) > vdp.line)
  {
    /* render next line now BEFORE updating register */
    render_line((vdp.line+1)%vdp.lpf);
  }

  if (offset & 1) /* Control port */
  {
    if(vdp.pending == 0)
    {
      vdp.addr = (vdp.addr & 0x3F00) | (data & 0xFF);
      vdp.latch = data;
      vdp.pending = 1;
    }
    else
    {
      vdp.pending = 0;
      vdp.code = (data >> 6) & 3;
      vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

      if(vdp.code == 0)
      {
        vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
        vdp.addr = (vdp.addr + 1) & 0x3FFF;
      }
      else if(vdp.code == 2)
      {
        vdp_reg_w(data & 0xF, vdp.latch);
      }
    }
  }
  else /* Data port */
  {
    vdp.pending = 0;
    switch(vdp.code)
    {
      case 0: /* VRAM write */
      case 1: /* VRAM write */
      case 2: /* VRAM write */
        vdp.vram[(vdp.addr & 0x3FFF)] = data;
        vdp.buffer = data;
        break;

      case 3: /* CRAM write */
        if(vdp.addr & 1)
        {
          vdp.cram_latch = (vdp.cram_latch & 0x00FF) | ((data & 0xFF) << 8);
          vdp.cram[(vdp.addr & 0x3E) | (0)] = (vdp.cram_latch >> 0) & 0xFF;
          vdp.cram[(vdp.addr & 0x3E) | (1)] = (vdp.cram_latch >> 8) & 0xFF;
          palette_sync((vdp.addr >> 1) & 0x1F);
        }
        else
        {
          vdp.cram_latch = (vdp.cram_latch & 0xFF00) | ((data & 0xFF) << 0);
        }
        vdp.buffer = data;
        break;
    }
    vdp.addr = (vdp.addr + 1) & 0x3FFF;
  }
}

/*--------------------------------------------------------------------------*/
/* TMS9918 VDP handlers                           */
/*--------------------------------------------------------------------------*/

void tms_write(int offset, int data)
{
  if (offset & 1) /* Control port */
  {
    if(vdp.pending == 0)
    {
      vdp.latch = data;
      vdp.pending = 1;
    }
    else
    {
      vdp.pending = 0;
      vdp.code = (data >> 6) & 3;
      vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

      if(vdp.code == 0)
      {
        vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
        vdp.addr = (vdp.addr + 1) & 0x3FFF;
      }
      else if(vdp.code == 2)
      {
        vdp_reg_w(data & 0x07, vdp.latch);
      }
    }
  }
  else /* Data port */
  {
    vdp.pending = 0;

    switch(vdp.code)
    {
      case 0: /* VRAM write */
      case 1: /* VRAM write */
      case 2: /* VRAM write */
      case 3: /* VRAM write */
        vdp.vram[(vdp.addr & 0x3FFF)] = data;
        break;
    }
    vdp.addr = (vdp.addr + 1) & 0x3FFF;
  }
}
