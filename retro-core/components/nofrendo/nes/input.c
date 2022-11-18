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
** nes/input.c: Input emulation (joypads)
**
*/

#include "nes.h"

static input_t ports[2];
static int strobe = 0;


void input_write(uint32 address, uint8 value)
{
    value &= 1;

    if (!value && strobe)
    {
        ports[0].reads = 0;
        ports[1].reads = 0;
    }

    strobe = value;
}

uint8 input_read(uint32 address)
{
    /* 0x4016 = JOY0, 0x4017 = JOY1 */
    input_t *port = &ports[address & 1];
    unsigned retval = 0;
    unsigned value = 0;

    if (port->type == NES_JOYPAD)
    {
        value = port->state;
    }

    /* mask out left/right simultaneous keypresses */
    if ((value & NES_PAD_UP) && (value & NES_PAD_DOWN))
        value &= ~(NES_PAD_UP | NES_PAD_DOWN);

    if ((value & NES_PAD_LEFT) && (value & NES_PAD_RIGHT))
        value &= ~(NES_PAD_LEFT | NES_PAD_RIGHT);

    /* return (0x40 | value) due to bus conflicts */
    retval |= (0x40 | ((value >> port->reads++) & 1));

    if (address == 0x4017 && port->type == NES_ZAPPER) // JOY1 also manages the zapper
    {
        retval |= port->state;
    }

    return retval;
}

void input_connect(int port, nes_dev_t type)
{
    ports[port & 1].type = type;
    ports[port & 1].reads = 0;
    ports[port & 1].state = 0;
}

void input_update(int port, int state)
{
    ports[port & 1].state = state;
}

input_t *input_init(void)
{
    input_connect(0, NES_JOYPAD);
    input_connect(1, NES_NOTHING);
    return (input_t *)ports;
}

void input_reset(void)
{
    ports[0].reads = ports[1].reads = 0;
    ports[0].state = ports[1].state = 0;
}
