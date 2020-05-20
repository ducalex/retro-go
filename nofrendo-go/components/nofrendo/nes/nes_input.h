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
** nes_input.h
**
** Platform independent input definitions
** $Id: nes_input.h,v 1.1.1.1 2001/04/27 07:03:54 neil Exp $
*/

#ifndef _NESINPUT_H_
#define _NESINPUT_H_

/* Registers */
#define  INP_JOY0          0x4016
#define  INP_JOY1          0x4017

/* NES control pad bitmasks */
#define  INP_PAD_A         0x01
#define  INP_PAD_B         0x02
#define  INP_PAD_SELECT    0x04
#define  INP_PAD_START     0x08
#define  INP_PAD_UP        0x10
#define  INP_PAD_DOWN      0x20
#define  INP_PAD_LEFT      0x40
#define  INP_PAD_RIGHT     0x80

#define  INP_ZAPPER_HIT    0x00
#define  INP_ZAPPER_MISS   0x08
#define  INP_ZAPPER_TRIG   0x10

#define  INP_JOYPAD0       0x0001
#define  INP_JOYPAD1       0x0002
#define  INP_ZAPPER        0x0004

/* upper byte is what's returned in D4, lower is D3 */
#define  INP_PPAD_1        0x0002
#define  INP_PPAD_2        0x0001
#define  INP_PPAD_3        0x0200
#define  INP_PPAD_4        0x0100
#define  INP_PPAD_5        0x0004
#define  INP_PPAD_6        0x0010
#define  INP_PPAD_7        0x0080
#define  INP_PPAD_8        0x0800
#define  INP_PPAD_9        0x0008
#define  INP_PPAD_10       0x0020
#define  INP_PPAD_11       0x0040
#define  INP_PPAD_12       0x0400

enum
{
   INP_STATE_BREAK,
   INP_STATE_MAKE
};

typedef struct nesinput_s
{
   int type;
   int data;
} nesinput_t;

#define  MAX_CONTROLLERS   8

extern void input_register(nesinput_t *input);
extern void input_event(nesinput_t *input, int state, int value);
extern void input_strobe(void);

uint8 input_read(uint32 address);
void input_write(uint32 address, uint8 value);

#endif /* _NESINPUT_H_ */
