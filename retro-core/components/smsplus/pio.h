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

#ifndef _PIO_H_
#define _PIO_H_

#define SIO_TXFL    (1 << 0)    /* 1= Transmit buffer full */
#define SIO_RXRD    (1 << 1)    /* 1= Receive buffer full */
#define SIO_FRER    (1 << 2)    /* 1= Framing error occured */

#define MAX_DEVICE  2
#define DEVICE_D0   (1 << 0)
#define DEVICE_D1   (1 << 1)
#define DEVICE_D2   (1 << 2)
#define DEVICE_D3   (1 << 3)
#define DEVICE_TL   (1 << 4)
#define DEVICE_TR   (1 << 5)
#define DEVICE_TH   (1 << 6)
#define DEVICE_ALL  (DEVICE_D0 | DEVICE_D1 | DEVICE_D2 | DEVICE_D3 | DEVICE_TL | DEVICE_TR | DEVICE_TH)

#define PIN_LVL_LO  0       /* Pin outputs 0V */
#define PIN_LVL_HI  1       /* Pin outputs +5V */
#define PIN_DIR_OUT 0       /* Pin is is an active driving output */
#define PIN_DIR_IN  1       /* Pin is in active-low input mode */

enum {
  PORT_A      = 0,  /* I/O port A */
  PORT_B      = 1   /* I/O port B */
};

enum {
  DEVICE_NONE       = 0,  /* No peripheral */
  DEVICE_PAD2B      = 1,  /* Standard 2-button digital joystick/gamepad */
  DEVICE_PADDLE     = 2,  /* Paddle controller; rotary dial with fire button */
  DEVICE_LIGHTGUN   = 3,  /* Sega Light Phaser; Light Gun with trigger button */
  DEVICE_SPORTSPAD  = 4,  /* Sports Pad controller; analog stick with 2 buttons */
};

/* Function prototypes */
extern void pio_init(void);
extern void pio_reset(void);
extern void pio_shutdown(void);
extern void pio_ctrl_w(uint8 data);
extern uint8 pio_port_r(int offset);
extern void sio_w(int offset, int data);
extern uint8 sio_r(int offset);
extern uint8 coleco_pio_r(int port);

#endif /* _PIO_H_ */
