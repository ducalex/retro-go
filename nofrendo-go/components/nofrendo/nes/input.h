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
** nes/input.h: Input emulation header
**
*/

#pragma once

/* NES control pad bitmasks */
#define NES_PAD_A 0x01
#define NES_PAD_B 0x02
#define NES_PAD_SELECT 0x04
#define NES_PAD_START 0x08
#define NES_PAD_UP 0x10
#define NES_PAD_DOWN 0x20
#define NES_PAD_LEFT 0x40
#define NES_PAD_RIGHT 0x80

#define NES_ZAPPER_HIT 0x01
#define NES_ZAPPER_MISS 0x02
#define NES_ZAPPER_TRIG 0x04

typedef enum
{
    NES_NOTHING,
    NES_JOYPAD,
    NES_ZAPPER,
} nes_dev_t;

typedef struct
{
    nes_dev_t type;
    uint8 state;
    uint8 reads;
} input_t;

input_t *input_init(void);
void input_reset(void);
void input_connect(int port, nes_dev_t type);
void input_update(int port, int state);
uint8 input_read(uint32 address);
void input_write(uint32 address, uint8 value);
