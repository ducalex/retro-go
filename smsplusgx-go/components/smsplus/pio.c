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
 *   I/O chip and peripheral emulation
 *
 ******************************************************************************/

#include "shared.h"
#include <math.h>

typedef struct {
  uint8 tr_level[2];  /* TR pin output level */
  uint8 th_level[2];  /* TH pin output level */
  uint8 tr_dir[2];    /* TR pin direction */
  uint8 th_dir[2];    /* TH pin direction */
} io_state;

static io_state io_lut[2][256];
static io_state *io_current;


/*
  from smpower WIKI:

  Port $3F: I/O port control

  Bit  Function
  ------------
  7  Port B TH pin output level (1=high, 0=low)
  6  Port B TR pin output level (1=high, 0=low)
  5  Port A TH pin output level (1=high, 0=low)
  4  Port A TR pin output level (1=high, 0=low)
  3  Port B TH pin direction (1=input, 0=output)
  2  Port B TR pin direction (1=input, 0=output)
  1  Port A TH pin direction (1=input, 0=output)
  0  Port A TR pin direction (1=input, 0=output)

  This port is used to control the two input/output lines available on each controller port (referred to as ports A and B).
  Some control hardware needs to recieve input from the CPU to perform its task, and uses one or more of these lines.
  Passive hardware uses all of them as inputs.

  This functionality is not present on the Mark III and Japanese Master System.
  As a result, it is commonly used for region detection.
  It also means that for compatibility, it defaults to an input-only state (ie. with all the low bits set) on startup.
*/

void pio_init(void)
{
  int i, j;

  /* Make pin state LUT */
  for(j = 0; j < 2; j++)
  {
    for(i = 0; i < 0x100; i++)
    {
      /* Common control: pin direction */
      io_lut[j][i].tr_dir[0]   = (i & 0x01) ? PIN_DIR_IN : PIN_DIR_OUT;
      io_lut[j][i].th_dir[0]   = (i & 0x02) ? PIN_DIR_IN : PIN_DIR_OUT;
      io_lut[j][i].tr_dir[1]   = (i & 0x04) ? PIN_DIR_IN : PIN_DIR_OUT;
      io_lut[j][i].th_dir[1]   = (i & 0x08) ? PIN_DIR_IN : PIN_DIR_OUT;

      if(j == 1)
      {
        /* Programmable output state (Export machines only) */
        io_lut[j][i].tr_level[0] = (i & 0x01) ? PIN_LVL_HI : (i & 0x10) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].th_level[0] = (i & 0x02) ? PIN_LVL_HI : (i & 0x20) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].tr_level[1] = (i & 0x04) ? PIN_LVL_HI : (i & 0x40) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].th_level[1] = (i & 0x08) ? PIN_LVL_HI : (i & 0x80) ? PIN_LVL_HI : PIN_LVL_LO;
      }
      else
      {
        /* Fixed output state (Domestic machines only) */
        io_lut[j][i].tr_level[0] = (i & 0x01) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].th_level[0] = (i & 0x02) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].tr_level[1] = (i & 0x04) ? PIN_LVL_HI : PIN_LVL_LO;
        io_lut[j][i].th_level[1] = (i & 0x08) ? PIN_LVL_HI : PIN_LVL_LO;
      }
    }
  }
}

void pio_reset(void)
{
  /* GG SIO power-on defaults */
  sms.sio.pdr   = 0x7F;
  sms.sio.ddr   = 0xFF;
  sms.sio.txdata  = 0x00;
  sms.sio.rxdata  = 0xFF;
  sms.sio.sctrl   = 0x00;
  input.analog[0][0] = 128;
  input.analog[0][1] = 96;

  /* SMS I/O power-on defaults */
  io_current = &io_lut[sms.territory][0xFF];
  pio_ctrl_w(0xFF);
}

void pio_shutdown(void)
{
  /* Nothing to do */
}


/* I/O port control */
void pio_ctrl_w(uint8 data)
{
  uint8 th_level[2];

  /* save old TH values */
  th_level[0] = io_current->th_level[0];
  th_level[1] = io_current->th_level[1];

  /* HCounter is latched on TH Low->High transition */
  io_current = &io_lut[sms.territory][data];
  if ((io_current->th_dir[0]   == PIN_DIR_IN) &&
       (io_current->th_level[0] == PIN_LVL_HI) &&
       (th_level[0] == PIN_LVL_LO)
      )
  {
    sms.hlatch = hc_256[z80_get_elapsed_cycles() % CYCLES_PER_LINE];
  }

  /* update port value */
  sms.ioctrl = data;
}

/*
   Return peripheral ports Pin value for various devices:

  Bit  Function
  ------------
  7   Unused
  6   TH pin input
  5   TR pin input
  4   TL pin input
  3   Right pin input
  2   Left pin input
  1   Down pin input
  0   Up pin input
*/

static uint8 paddle_toggle[2] = {0,0};
static uint8 lightgun_latch =0;

static uint8 device_r(int port)
{
  uint8 temp = 0x7F;

  switch(sms.device[port])
  {
    case DEVICE_NONE:
      break;

    case DEVICE_PAD2B:
      if(input.pad[port] & INPUT_UP)    temp &= ~0x01;
      if(input.pad[port] & INPUT_DOWN)  temp &= ~0x02;
      if(input.pad[port] & INPUT_LEFT)  temp &= ~0x04;
      if(input.pad[port] & INPUT_RIGHT)   temp &= ~0x08;
      if(input.pad[port] & INPUT_BUTTON1) temp &= ~0x10;  /* TL */
      if(input.pad[port] & INPUT_BUTTON2) temp &= ~0x20;  /* TR */
      break;

    /* PADDLE emulation
      based on the documentation from smspower.org
      http://www.smspower.org/dev/docs/wiki/InputAndOutput/Paddle
      http://www.smspower.org/forums/viewtopic.php?t=6783
    */
    case DEVICE_PADDLE:
      if (sms.territory == TERRITORY_EXPORT)
      {
        /* non-japanese mode: TH output control */
        paddle_toggle[port] = (io_current->th_level[0] == PIN_LVL_LO);
      }
      else
      {
        /* japanese mode: automatic flip-flop */
        paddle_toggle[port] ^= 1;
      }

      if (paddle_toggle[port])
      {
        /* Low position bits */
        temp = (temp & 0xF0) | (input.analog[port][0] & 0x0F);

        /* set TR low */
        temp &= ~0x20;
      }
      else
      {
        /* High position bits */
        temp = (temp & 0xF0) | ((input.analog[port][0] >> 4) & 0x0F);
      }

      /* Fire Button */
      if(input.pad[port] & INPUT_BUTTON1) temp &= ~0x10;

      break;

    case DEVICE_SPORTSPAD: /* TODO */
      break;

    /* LIGHTGUN emulation:
      Basically, the light phaser set TH low when it detects the TV light beam
      We emulate this by comparing our current "LightGun" position with HV counter values
      The value must be set long enough as the software generally make several access to
      get an average value because the lightgun see a circular spot, not a single pixel
      The system latches the current HV counter values on TH high-to-low transition, this
      is used by software to enable/disable LightGuns and support 2 players simultaneously
    */
    case DEVICE_LIGHTGUN:

      /* check TH INPUT */
      if (io_current->th_dir[port] == PIN_DIR_IN)
      {
        int hc = hc_256[z80_get_elapsed_cycles() % CYCLES_PER_LINE];
        int dx = input.analog[port][0] - (hc*2);
        int dy = input.analog[port][1] - vdp.line;

        /* is current pixel is within lightgun spot ? */
        if ((abs(dy) <= 5) && (abs(dx) <= 60))
        {
          /* set TH low */
          temp &= ~0x40;

          /* prevents multiple latch at each port read */
          if (!lightgun_latch)
          {
            /* latch estimated HC value */
            sms.hlatch = sms.gun_offset + (input.analog[port][0])/2;
            lightgun_latch = 1;
          }
        }
        else
        {
          lightgun_latch = 0;
        }
      }

      /* Trigger Button */
      if(input.pad[port] & INPUT_BUTTON1) temp &= ~0x10;

      break;
  }

  return temp;
}


uint8 pio_port_r(int offset)
{
  uint8 temp = 0xFF;

  /*
    If I/O chip is disabled, reads return last byte of instruction
    that read the I/O port.
  */
  if(sms.memctrl & 0x04) return z80_read_unmapped();

  switch (offset & 1)
  {
    /*
      Port $DC & mirrors: I/O port A and B

      Bit  Function:
      ------------
      7  Port B Down pin input
      6  Port B Up pin input
      5  Port A TR pin input
      4  Port A TL pin input
      3  Port A Right pin input
      2  Port A Left pin input
      1  Port A Down pin input
      0  Port A Up pin input

    */
    case 0:

      /* read I/O port A pins */
      temp = device_r(0) & 0x3f;

      /* read I/O port B low pins (Game Gear is special case) */
      if (IS_GG) temp |= ((sio_r(1) & 0x03) << 6);
      else temp |= ((device_r(1) & 0x03) << 6);

      /* Adjust TR state if it is an output */
      if(io_current->tr_dir[0] == PIN_DIR_OUT)
      {
        temp &= ~0x20;
        temp |= (io_current->tr_level[0] == PIN_LVL_HI) ? 0x20 : 0x00;
      }

      break;

    /*
      Port $DD & mirrors: I/O port B and miscellaneous

      Bit  Function:
      ------------
      7  Port B TH pin input
      6  Port A TH pin input
      5  Cartridge slot CONT pin *
      4  Reset button (1= not pressed, 0= pressed) *
      3  Port B TR pin input
      2  Port B TL pin input
      1  Port B Right pin input
      0  Port B Left pin input

      These map to hardware which does not exist on some systems (eg. SMS2 has neither).
      Reset always returns 1 when hardware is not present;
      CONT generally returns 1 on 8-bit hardware and 0 on a Mega Drive.
    */
    case 1:

      /* read I/O port B low pins (Game Gear is special case) */
      if (IS_GG)
      {
        uint8 state = sio_r(0x01);
        temp = (state & 0x3C) >> 2;     /* Insert TR,TL,D3,D2       */
        temp |= ((state & 0x40) << 1);  /* Insert TH2               */
        temp |= 0x40;                   /* Insert TH1 (unconnected) */
      }
      else
      {
        uint8 state = device_r(1);
        temp = (state & 0x3C) >> 2;   /* Insert TR,TL,D3,D2 */
        temp |= ((state & 0x40) << 1);  /* Insert TH2 */
        temp |= (device_r(0) & 0x40);   /* Insert TH1 */
      }

      /* Adjust TR state if it is an output */
      if(io_current->tr_dir[1] == PIN_DIR_OUT)
      {
        temp &= ~0x08;
        temp |= (io_current->tr_level[1] == PIN_LVL_HI) ? 0x08 : 0x00;
      }

      /* Adjust TH1 state if it is an output */
      if(io_current->th_dir[0] == PIN_DIR_OUT)
      {
        temp &= ~0x40;
        temp |= (io_current->th_level[0] == PIN_LVL_HI) ? 0x40 : 0x00;
      }

      /* Adjust TH2 state if it is an output */
      if(io_current->th_dir[1] == PIN_DIR_OUT)
      {
        temp &= ~0x80;
        temp |= (io_current->th_level[1] == PIN_LVL_HI) ? 0x80 : 0x00;
      }

      /* RESET and /CONT */
      temp |= 0x30;
      if (input.system & INPUT_RESET) temp &= ~0x10;

      break;
  }
  return temp;
}

/* Game Gear specific IO ports */
uint8 sio_r(int offset)
{
  uint8 temp;

  switch(offset & 0xFF)
  {
    case 0: /* Input port #2 */
      temp = 0xE0;
      if(input.system & INPUT_START)          temp &= ~0x80;
      if(sms.territory == TERRITORY_DOMESTIC) temp &= ~0x40;
      if(sms.display == DISPLAY_NTSC)         temp &= ~0x20;
      return temp;

    case 1: /* Parallel data register */
      temp = 0x00;
      temp |= (sms.sio.ddr & 0x01) ? 0x01 : (sms.sio.pdr & 0x01);
      temp |= (sms.sio.ddr & 0x02) ? 0x02 : (sms.sio.pdr & 0x02);
      temp |= (sms.sio.ddr & 0x04) ? 0x04 : (sms.sio.pdr & 0x04);
      temp |= (sms.sio.ddr & 0x08) ? 0x08 : (sms.sio.pdr & 0x08);
      temp |= (sms.sio.ddr & 0x10) ? 0x10 : (sms.sio.pdr & 0x10);
      temp |= (sms.sio.ddr & 0x20) ? 0x20 : (sms.sio.pdr & 0x20);
      temp |= (sms.sio.ddr & 0x40) ? 0x40 : (sms.sio.pdr & 0x40);
      temp |= (sms.sio.pdr & 0x80);
      return temp;

    case 2: /* Data direction register and NMI enable */
      return sms.sio.ddr;

    case 3: /* Transmit data buffer */
      return sms.sio.txdata;

    case 4: /* Receive data buffer */
      return sms.sio.rxdata;

    case 5: /* Serial control */
      return sms.sio.sctrl;

    case 6: /* Stereo sound control */
      return 0xFF;
  }

  /* Just to please compiler */
  return -1;
}

void sio_w(int offset, int data)
{
  switch(offset & 0xFF)
  {
    case 0: /* Input port #2 (read-only) */
      return;

    case 1: /* Parallel data register */
      sms.sio.pdr = data;
      return;

    case 2: /* Data direction register and NMI enable */
      sms.sio.ddr = data;
      return;

    case 3: /* Transmit data buffer */
      sms.sio.txdata = data;
      return;

    case 4: /* Receive data buffer */
      return;

    case 5: /* Serial control */
      sms.sio.sctrl = data & 0xF8;
      return;

    case 6: /* Stereo output control */
      psg_stereo_w(data);
      return;
  }
}

/* Colecovision I/O chip support */
static uint8 keymask[12] =
{
  0x7a, /* 0 */
  0x7d, /* 1 */
  0x77, /* 2 */
  0x7c, /* 3 */
  0x72, /* 4 */
  0x73, /* 5 */
  0x7e, /* 6 */
  0x75, /* 7 */
  0x71, /* 8 */
  0x7b, /* 9 */
  0x79, /* * */
  0x76  /* # */
};

uint8 coleco_pio_r(int port)
{
  uint8 temp = 0x7f;

  if (coleco.pio_mode)
  {
    /* Joystick  */
    if(input.pad[port] & INPUT_UP) temp &= ~1;
    else if(input.pad[port] & INPUT_DOWN) temp &= ~4;
    if(input.pad[port] & INPUT_LEFT) temp &= ~8;
    else if(input.pad[port] & INPUT_RIGHT) temp &= ~2;

    /* Left Button */
    if(input.pad[port] & INPUT_BUTTON1) temp &= ~0x40;
  }
  else
  {
    /* KeyPad (0-9,*,#) */
    if (coleco.keypad[port] < 12)
     temp = keymask[coleco.keypad[port]];

    /* Right Button */
    if(input.pad[port] & INPUT_BUTTON2) temp &= ~0x40;
  }

  return temp;
}
