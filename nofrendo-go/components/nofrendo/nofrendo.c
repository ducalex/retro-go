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
** nofrendo.c: Entry point of program
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nofrendo.h>
#include <nes.h>


void nofrendo_stop(void)
{
    nes_poweroff();
    nes_shutdown();
    osd_shutdown();
}

int nofrendo_start(const char *filename, int region, int sample_rate, bool stereo)
{
    if (osd_init())
        return -1;

    if (!nes_init(region, sample_rate, stereo))
    {
        MESSAGE_ERROR("Failed to create NES instance.\n");
        return -1;
    }

    if (!nes_insertcart(filename))
    {
        MESSAGE_ERROR("Failed to insert NES cart.\n");
        return -2;
    }

    nes_emulate();

    return 0;
}
