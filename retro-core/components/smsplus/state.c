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
 *   Nintendo Gamecube State Management
 *
 ******************************************************************************/

#include "shared.h"

/*
state: sizeof sms=8216
state: sizeof vdp=16524
state: sizeof Z80=72
state: sizeof SN76489_Context=92
state: sizeof coleco=8
*/

int system_save_state(void *mem)
{
  uint8 padding[16] = {0};
  int i;

  /*** Save SMS Context ***/
  fwrite(sms.wram, 0x2000, 1, mem);
  fwrite(&sms.paused, 1, 1, mem);
  fwrite(&sms.save, 1, 1, mem);
  fwrite(&sms.territory, 1, 1, mem);
  fwrite(&sms.console, 1, 1, mem);
  fwrite(&sms.display, 1, 1, mem);
  fwrite(&sms.fm_detect, 1, 1, mem);
  fwrite(&sms.glasses_3d, 1, 1, mem);
  fwrite(&sms.hlatch, 1, 1, mem);
  fwrite(&sms.use_fm, 1, 1, mem);
  fwrite(&sms.memctrl, 1, 1, mem);
  fwrite(&sms.ioctrl, 1, 1, mem);
  fwrite(&padding, 1, 1, mem);
  fwrite(&sms.sio, 8, 1, mem);
  fwrite(&sms.device, 2, 1, mem);
  fwrite(&sms.gun_offset, 1, 1, mem);
  fwrite(&padding, 1, 1, mem);

  /*** Save VDP state ***/
  fwrite(vdp.vram, 0x4000, 1, mem);
  fwrite(vdp.cram, 0x40, 1, mem);
  fwrite(vdp.reg, 0x10, 1, mem);
  fwrite(&vdp.vscroll, 1, 1, mem);
  fwrite(&vdp.status, 1, 1, mem);
  fwrite(&vdp.latch, 1, 1, mem);
  fwrite(&vdp.pending, 1, 1, mem);
  fwrite(&vdp.addr, 2, 1, mem);
  fwrite(&vdp.code, 1, 1, mem);
  fwrite(&vdp.buffer, 1, 1, mem);
  fwrite(&vdp.pn, 4, 1, mem);
  fwrite(&vdp.ct, 4, 1, mem);
  fwrite(&vdp.pg, 4, 1, mem);
  fwrite(&vdp.sa, 4, 1, mem);
  fwrite(&vdp.sg, 4, 1, mem);
  fwrite(&vdp.ntab, 4, 1, mem);
  fwrite(&vdp.satb, 4, 1, mem);
  fwrite(&vdp.line, 4, 1, mem);
  fwrite(&vdp.left, 4, 1, mem);
  fwrite(&vdp.lpf, 2, 1, mem);
  fwrite(&vdp.height, 1, 1, mem);
  fwrite(&vdp.extended, 1, 1, mem);
  fwrite(&vdp.mode, 1, 1, mem);
  fwrite(&vdp.irq, 1, 1, mem);
  fwrite(&vdp.vint_pending, 1, 1, mem);
  fwrite(&vdp.hint_pending, 1, 1, mem);
  fwrite(&vdp.cram_latch, 2, 1, mem);
  fwrite(&vdp.spr_col, 2, 1, mem);
  fwrite(&vdp.spr_ovr, 1, 1, mem);
  fwrite(&vdp.bd, 1, 1, mem);
  fwrite(&padding, 2, 1, mem);

  /*** Save cart info ***/
  for (i = 0; i < 4; i++)
  {
    fwrite(&cart.fcr[i], 1, 1, mem);
  }

  /*** Save SRAM ***/
  fwrite(cart.sram, 0x8000, 1, mem);

  /*** Save Z80 Context ***/
  fwrite(&Z80.pc, 4, 1, mem);
  fwrite(&Z80.sp, 4, 1, mem);
  fwrite(&Z80.af, 4, 1, mem);
  fwrite(&Z80.bc, 4, 1, mem);
  fwrite(&Z80.de, 4, 1, mem);
  fwrite(&Z80.hl, 4, 1, mem);
  fwrite(&Z80.ix, 4, 1, mem);
  fwrite(&Z80.iy, 4, 1, mem);
  fwrite(&Z80.wz, 4, 1, mem);
  fwrite(&Z80.af2, 4, 1, mem);
  fwrite(&Z80.bc2, 4, 1, mem);
  fwrite(&Z80.de2, 4, 1, mem);
  fwrite(&Z80.hl2, 4, 1, mem);
  fwrite(&Z80.r, 1, 1, mem);
  fwrite(&Z80.r2, 1, 1, mem);
  fwrite(&Z80.iff1, 1, 1, mem);
  fwrite(&Z80.iff2, 1, 1, mem);
  fwrite(&Z80.halt, 1, 1, mem);
  fwrite(&Z80.im, 1, 1, mem);
  fwrite(&Z80.i, 1, 1, mem);
  fwrite(&Z80.nmi_state, 1, 1, mem);
  fwrite(&Z80.nmi_pending, 1, 1, mem);
  fwrite(&Z80.irq_state, 1, 1, mem);
  fwrite(&Z80.after_ei, 1, 1, mem);
  fwrite(&padding, 9, 1, mem);

#if 0
  /*** Save YM2413 ***/
  memcpy (&state[bufferptr], FM_GetContextPtr (), FM_GetContextSize ());
  bufferptr += FM_GetContextSize ();
#endif

  /*** Save SN76489 ***/
  fwrite(SN76489_GetContextPtr(0), SN76489_GetContextSize(), 1, mem);

  fwrite(&coleco.pio_mode, 1, 1, mem);
  fwrite(&coleco.port53, 1, 1, mem);
  fwrite(&coleco.port7F, 1, 1, mem);
  fwrite(&padding, 5, 1, mem);

  return 0;
}


void system_load_state(void *mem)
{
  uint8 padding[16] = {0};
  int i;

  /* Initialize everything */
  system_reset();

  /*** Set SMS Context ***/
  int current_console = sms.console;
  sms.console = 0xFF;

  fread(sms.wram, 0x2000, 1, mem);
  fread(&sms.paused, 1, 1, mem);
  fread(&sms.save, 1, 1, mem);
  fread(&sms.territory, 1, 1, mem);
  fread(&sms.console, 1, 1, mem);
  fread(&sms.display, 1, 1, mem);
  fread(&sms.fm_detect, 1, 1, mem);
  fread(&sms.glasses_3d, 1, 1, mem);
  fread(&sms.hlatch, 1, 1, mem);
  fread(&sms.use_fm, 1, 1, mem);
  fread(&sms.memctrl, 1, 1, mem);
  fread(&sms.ioctrl, 1, 1, mem);
  fread(&padding, 1, 1, mem);
  fread(&sms.sio, 8, 1, mem);
  fread(&sms.device, 2, 1, mem);
  fread(&sms.gun_offset, 1, 1, mem);
  fread(&padding, 1, 1, mem);

  if(sms.console != current_console)
  {
      MESSAGE_ERROR("Bad save data\n");
      set_rom_config();
      system_reset();
      return;
  }

  /*** Set vdp state ***/
  fread(vdp.vram, 0x4000, 1, mem);
  fread(vdp.cram, 0x40, 1, mem);
  fread(vdp.reg, 0x10, 1, mem);
  fread(&vdp.vscroll, 1, 1, mem);
  fread(&vdp.status, 1, 1, mem);
  fread(&vdp.latch, 1, 1, mem);
  fread(&vdp.pending, 1, 1, mem);
  fread(&vdp.addr, 2, 1, mem);
  fread(&vdp.code, 1, 1, mem);
  fread(&vdp.buffer, 1, 1, mem);
  fread(&vdp.pn, 4, 1, mem);
  fread(&vdp.ct, 4, 1, mem);
  fread(&vdp.pg, 4, 1, mem);
  fread(&vdp.sa, 4, 1, mem);
  fread(&vdp.sg, 4, 1, mem);
  fread(&vdp.ntab, 4, 1, mem);
  fread(&vdp.satb, 4, 1, mem);
  fread(&vdp.line, 4, 1, mem);
  fread(&vdp.left, 4, 1, mem);
  fread(&vdp.lpf, 2, 1, mem);
  fread(&vdp.height, 1, 1, mem);
  fread(&vdp.extended, 1, 1, mem);
  fread(&vdp.mode, 1, 1, mem);
  fread(&vdp.irq, 1, 1, mem);
  fread(&vdp.vint_pending, 1, 1, mem);
  fread(&vdp.hint_pending, 1, 1, mem);
  fread(&vdp.cram_latch, 2, 1, mem);
  fread(&vdp.spr_col, 2, 1, mem);
  fread(&vdp.spr_ovr, 1, 1, mem);
  fread(&vdp.bd, 1, 1, mem);
  fread(&padding, 2, 1, mem);

  /** restore video & audio settings (needed if timing changed) ***/
  vdp_init();
  sound_init();

  /*** Set cart info ***/
  for (i = 0; i < 4; i++)
  {
    fread(&cart.fcr[i], 1, 1, mem);
  }

  /*** Set SRAM ***/
  fread(cart.sram, 0x8000, 1, mem);

  /*** Set Z80 Context ***/
  fread(&Z80.pc, 4, 1, mem);
  fread(&Z80.sp, 4, 1, mem);
  fread(&Z80.af, 4, 1, mem);
  fread(&Z80.bc, 4, 1, mem);
  fread(&Z80.de, 4, 1, mem);
  fread(&Z80.hl, 4, 1, mem);
  fread(&Z80.ix, 4, 1, mem);
  fread(&Z80.iy, 4, 1, mem);
  fread(&Z80.wz, 4, 1, mem);
  fread(&Z80.af2, 4, 1, mem);
  fread(&Z80.bc2, 4, 1, mem);
  fread(&Z80.de2, 4, 1, mem);
  fread(&Z80.hl2, 4, 1, mem);
  fread(&Z80.r, 1, 1, mem);
  fread(&Z80.r2, 1, 1, mem);
  fread(&Z80.iff1, 1, 1, mem);
  fread(&Z80.iff2, 1, 1, mem);
  fread(&Z80.halt, 1, 1, mem);
  fread(&Z80.im, 1, 1, mem);
  fread(&Z80.i, 1, 1, mem);
  fread(&Z80.nmi_state, 1, 1, mem);
  fread(&Z80.nmi_pending, 1, 1, mem);
  fread(&Z80.irq_state, 1, 1, mem);
  fread(&Z80.after_ei, 1, 1, mem);
  fread(&padding, 9, 1, mem);

#if 0
  /*** Set YM2413 ***/
  buf = malloc(FM_GetContextSize());
  memcpy (buf, &state[bufferptr], FM_GetContextSize ());
  FM_SetContext(buf);
  free(buf);
  bufferptr += FM_GetContextSize ();
#endif

  // Preserve clock rate
  SN76489_Context* psg = (SN76489_Context*)SN76489_GetContextPtr(0);
  float psg_Clock = psg->Clock;
  float psg_dClock = psg->dClock;

  /*** Set SN76489 ***/
  fread(SN76489_GetContextPtr(0), SN76489_GetContextSize(), 1, mem);

  // Restore clock rate
  psg->Clock = psg_Clock;
  psg->dClock = psg_dClock;

  fread(&coleco.pio_mode, 1, 1, mem);
  fread(&coleco.port53, 1, 1, mem);
  fread(&coleco.port7F, 1, 1, mem);
  fread(&padding, 5, 1, mem);

  if (sms.console == CONSOLE_COLECO)
  {
    coleco_port_w(0x53, coleco.port53);
    coleco_port_w(0x7F, coleco.port7F);
  }
  else if (sms.console != CONSOLE_SG1000)
  {
    /* Cartridge by default */
    slot.rom    = cart.rom;
    slot.pages  = cart.pages;
    slot.mapper = cart.mapper;
    slot.fcr = &cart.fcr[0];

    /* Restore mapping */
    mapper_reset();
    cpu_readmap[0]  = &slot.rom[0];
    if (slot.mapper != MAPPER_KOREA_MSX)
    {
      mapper_16k_w(0,slot.fcr[0]);
      mapper_16k_w(1,slot.fcr[1]);
      mapper_16k_w(2,slot.fcr[2]);
      mapper_16k_w(3,slot.fcr[3]);
    }
    else
    {
      mapper_8k_w(0,slot.fcr[0]);
      mapper_8k_w(1,slot.fcr[1]);
      mapper_8k_w(2,slot.fcr[2]);
      mapper_8k_w(3,slot.fcr[3]);
    }
  }

  // /* Force full pattern cache update */
  // bg_list_index = 0x200;
  // for(i = 0; i < 0x200; i++)
  // {
  //   bg_name_list[i] = i;
  //   bg_name_dirty[i] = -1;
  // }

  /* Restore palette */
  for(i = 0; i < PALETTE_SIZE; i++)
    palette_sync(i);
}
