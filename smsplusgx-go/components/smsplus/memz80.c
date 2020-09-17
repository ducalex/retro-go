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
 *   Z80 memory handlers
 *
 ******************************************************************************/

#include "shared.h"

/* Pull-up resistors on data bus */
uint8 data_bus_pullup   = 0x00;

/* Read unmapped memory */
uint8 z80_read_unmapped(void)
{
  uint16 pc = (Z80.pc.w.l - 1) & 0xFFFF;
  uint8 data = cpu_readmap[pc >> 13][pc & 0x03FF];
  return (data | data_bus_pullup);
}

/* Port $3E (Memory Control Port) */
void memctrl_w(uint8 data)
{
  /* detect CARTRIDGE/BIOS enabled/disabled */
  if (IS_SMS)
  {
    /* autodetect loaded BIOS ROM */
    if (!(bios.enabled & 2) && ((data & 0xE8) == 0xE8))
    {
      bios.enabled = 0; //option.use_bios | 2;
      memcpy(bios.rom, cart.rom, cart.size);
      memcpy(bios.fcr, cart.fcr, 4);
      bios.pages = cart.pages;
      cart.loaded = 0;
    }

    /* disables CART & BIOS by default */
    slot.rom = NULL;
    slot.mapper = MAPPER_NONE;

    switch (data & 0x48)
    {
      case 0x00:  /* BIOS & CART enabled */
      case 0x08:  /* BIOS disabled, CART enabled */
        if (cart.loaded)
        {
          slot.rom    = cart.rom;
          slot.pages  = cart.pages;
          slot.mapper = cart.mapper;
          slot.fcr    = &cart.fcr[0];
        }
        break;

      case 0x40:  /* BIOS enabled, CART disabled */
        slot.rom    = bios.rom;
        slot.pages  = bios.pages;
        slot.mapper = MAPPER_SEGA;
        slot.fcr    = &bios.fcr[0];
        break;

      default:
        break;
    }

    mapper_reset();

    /* reset SLOT mapping */
    if (slot.rom)
    {
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
    else
    {
      uint8 i;
      for(i = 0x00; i <= 0x2F; i++)
      {
        cpu_readmap[i]  = dummy_memory;
        cpu_writemap[i] = dummy_memory;
      }
    }
  }

  /* update register value */
  sms.memctrl = data;
}

/*--------------------------------------------------------------------------*/
/* Sega Master System port handlers                                         */
/*--------------------------------------------------------------------------*/
void sms_port_w(uint16 port, uint8 data)
{
  port &= 0xFF;

  /* access FM unit */
  if(port >= 0xF0)
  {
    switch(port)
    {
      case 0xF0:
      fmunit_write(0, data);
      return;

      case 0xF1:
      fmunit_write(1, data);
      return;

      case 0xF2:
      fmunit_detect_w(data);
      return;
    }
  }

  switch(port & 0xC1)
  {
    case 0x00:
      memctrl_w(data);
      return;

    case 0x01:
      pio_ctrl_w(data);
      return;

    case 0x40:
    case 0x41:
      psg_write(data);
      return;

    case 0x80:
    case 0x81:
      vdp_write(port, data);
      return;

    case 0xC0:
    case 0xC1:
      return;
  }
}

uint8 sms_port_r(uint16 port)
{
  port &= 0xFF;

  /* FM unit */
  if (port == 0xF2)
    return fmunit_detect_r() & pio_port_r(port);

  switch(port & 0xC0)
  {
    case 0x00:
      return z80_read_unmapped();

    case 0x40:
      return vdp_counter_read(port);

    case 0x80:
      return vdp_read(port);

    case 0xC0:
      return pio_port_r(port);
  }

  /* Just to please the compiler */
  return -1;
}

/*--------------------------------------------------------------------------*/
/* Game Gear port handlers                                                  */
/*--------------------------------------------------------------------------*/

void gg_port_w(uint16 port, uint8 data)
{
  port &= 0xFF;

  if(port <= 0x20) {
    sio_w(port, data);
    return;
  }

  switch(port & 0xC1)
  {
    case 0x00:
      memctrl_w(data);
      return;

    case 0x01:
      pio_ctrl_w(data);
      return;

    case 0x40:
    case 0x41:
      psg_write(data);
      return;

    case 0x80:
    case 0x81:
      gg_vdp_write(port, data);
      return;
  }
}


uint8 gg_port_r(uint16 port)
{
  port &= 0xFF;

  if(port <= 0x20)
    return sio_r(port);

  switch(port & 0xC0)
  {
    case 0x00:
      return z80_read_unmapped();

    case 0x40:
      return vdp_counter_read(port);

    case 0x80:
      return vdp_read(port);

    case 0xC0:
      switch(port)
      {
        case 0xC0:
        case 0xC1:
        case 0xDC:
        case 0xDD:
          return pio_port_r(port);
      }
      return z80_read_unmapped();
  }

  /* Just to please the compiler */
  return -1;
}

/*--------------------------------------------------------------------------*/
/* Game Gear (MS) port handlers                                             */
/*--------------------------------------------------------------------------*/

void ggms_port_w(uint16 port, uint8 data)
{
  port &= 0xFF;

  if(port <= 0x20) {
    sio_w(port, data);
    return;
  }

  switch(port & 0xC1)
  {
    case 0x00:
      memctrl_w(data);
      return;

    case 0x01:
      pio_ctrl_w(data);
      return;

    case 0x40:
    case 0x41:
      psg_write(data);
      return;

    case 0x80:
    case 0x81:
      vdp_write(port, data); /* fixed */
      return;
  }
}

uint8 ggms_port_r(uint16 port)
{
  port &= 0xFF;

  if(port <= 0x20)
    return sio_r(port);

  switch(port & 0xC0)
  {
    case 0x00:
      return z80_read_unmapped();

    case 0x40:
      return vdp_counter_read(port);

    case 0x80:
      return vdp_read(port);

    case 0xC0:
      switch(port)
      {
        case 0xC0:
        case 0xC1:
        case 0xDC:
        case 0xDD:
          return pio_port_r(port);
      }
      return z80_read_unmapped();
  }

  /* Just to please the compiler */
  return -1;
}

/*--------------------------------------------------------------------------*/
/* SG1000,SC3000,SF7000 port handlers                                       */
/*--------------------------------------------------------------------------*/

void tms_port_w(uint16 port, uint8 data)
{
  switch(port & 0xC0)
  {
    case 0x40:
      psg_write(data);
      return;

    case 0x80:
      tms_write(port, data);
      return;

    default:
      return;
  }
}

uint8 tms_port_r(uint16 port)
{
  switch(port & 0xC0)
  {
    case 0x80:
      return vdp_read(port);

    case 0xC0:
      return pio_port_r(port);

    default:
      return 0xff;
  }
}

/*--------------------------------------------------------------------------*/
/* Colecovision port handlers                                               */
/*--------------------------------------------------------------------------*/
void coleco_port_w(uint16 port, uint8 data)
{
  /* A7 is used as enable input */
  /* A6 & A5 are used to decode the address */
  switch(port & 0xE0)
  {
    case 0x80:
      coleco.pio_mode = 0;
      return;

    case 0xa0:
      tms_write(port,data);
      return;

    case 0xc0:
      coleco.pio_mode = 1;
      return;

    case 0xe0:
      psg_write(data);
      return;

    default:
      return;
  }
}

uint8 coleco_port_r(uint16 port)
{
  /* A7 is used as enable input */
  /* A6 & A5 are used to decode the address */
  switch(port & 0xE0)
  {
    case 0xa0:
      return vdp_read(port);

    case 0xe0:
      return coleco_pio_r((port>>1)&1);

    default:
      return 0xff;
  }
}
