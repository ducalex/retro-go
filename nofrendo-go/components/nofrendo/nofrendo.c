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

#include <stdlib.h>
#include <string.h>
#include "nofrendo.h"
#include "palettes.h"


int nofrendo_init(int system, int sample_rate, bool stereo, void *blit, void *vsync, void *input)
{
    nes_t *nes = nes_init(system, sample_rate, stereo);
    if (!nes)
    {
        MESSAGE_ERROR("Failed to create NES instance.\n");
        return -1;
    }

    nes->blit_func = blit;
    nes->input_func = input;
    nes->vsync_func = vsync;

    return 0;
}

/* Builds a 256 color 8-bit palette based on a 64-color NES palette
** Note that we set it up 3 times so that we flip bits on the primary
** NES buffer for priorities
*/
void *nofrendo_buildpalette(nespal_t palette, int bitdepth)
{
    if (palette >= NES_PALETTE_COUNT || palette < 0)
    {
        MESSAGE_WARN("Invalid palette %d, will use palette 0 instead.\n", palette);
        palette = 0;
    }

    const uint8 *nes_palette = nes_palettes[palette];

    if (bitdepth == 15 || bitdepth == 16)
    {
        uint16 *colors = calloc(256, 2);
        uint16 color = 0;

        for (int i = 0; i < 64; i++)
        {
            const uint8 *rgb = &nes_palette[i * 3];

            if (bitdepth == 16)
                color = (rgb[2] >> 3) + ((rgb[1] >> 2) << 5) + ((rgb[0] >> 3) << 11);
            else
                color = (rgb[2] >> 3) + ((rgb[1] >> 3) << 5) + ((rgb[0] >> 3) << 10);
            /* Set it up 3 times, for sprite priority/BG transparency trickery */
            colors[i] = colors[i + 64] = colors[i + 128] = color;
        }

        for (int i = 0; i < 8; i++)
        {
            const uint8 *rgb = &gui_pal[i * 3];
            if (bitdepth == 16)
                color = (rgb[2] >> 3) + ((rgb[1] >> 2) << 5) + ((rgb[0] >> 3) << 11);
            else
                color = (rgb[2] >> 3) + ((rgb[1] >> 3) << 5) + ((rgb[0] >> 3) << 10);
            colors[192 + i] = color;
        }
        return colors;
    }
    else if (bitdepth == 24)
    {
        uint8 *colors = calloc(256, 3);
        memcpy(colors, nes_palette, 64 * 3);
        /* Set it up 3 times, for sprite priority/BG transparency trickery */
        memcpy(colors + (64 * 3), nes_palette, 64 * 3);
        memcpy(colors + (128 * 3), nes_palette, 64 * 3);
        memcpy(colors + (192 * 3), gui_pal, 8 * 3);
        return colors;
    }

    return NULL;
}

int nofrendo_run(const char *filename, const char *savefile)
{
    nes_t *nes = nes_getptr();

    if (nes_insertcart(filename, NULL) < 0)
    {
        MESSAGE_ERROR("Failed to insert NES cart.\n");
        return -2;
    }

    if (savefile && state_load(savefile) < 0)
    {
        nes_reset(true);
    }

    while (!nes->poweroff)
    {
        nes_emulate(true);
    }

    return 0;
}

void nofrendo_stop(void)
{
    nes_poweroff();
    nes_shutdown();
}
