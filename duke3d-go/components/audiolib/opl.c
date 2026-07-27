// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//     OPL interface.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "opl.h"
#include "dbopl.h"

unsigned int opl_sample_rate = 11025;
int mus_opl_gain = 120; 

// OPL software emulator structure.
static Chip opl_chip;

// Temporary mixing buffer used by the emulator.
static int32_t *mix_buffer = NULL;

static int opl_paused = 0;

// Initialize the OPL library.
int OPL_Init (unsigned int rate)
{
    opl_sample_rate = rate;
    opl_paused = 0;

    mix_buffer = malloc(opl_sample_rate * sizeof(int32_t));

    DBOPL_InitTables();
    Chip__Chip(&opl_chip);
    Chip__Setup(&opl_chip, opl_sample_rate);

    OPL_InitRegisters();

    return 1;
}

// Shut down the OPL library.
void OPL_Shutdown(void)
{
    if (mix_buffer) {
        free(mix_buffer);
        mix_buffer = NULL;
    }
}

void OPL_Render_Samples (void *dest, unsigned nsamples)
{
    if (nsamples == 0) return;
    
    int16_t *buffer = (int16_t *) dest;
    int sampval;

    if (opl_paused) {
        memset(dest, 0, nsamples * 2 * sizeof(int16_t));
        return;
    }

    Chip__GenerateBlock2(&opl_chip, nsamples, mix_buffer);

    // Mix into the destination buffer, doubling up into stereo.
    for (unsigned int i=0; i<nsamples; ++i)
    {
        sampval = (mix_buffer[i] * mus_opl_gain) / 50;
        // clip
        if (sampval > 32767)
            sampval = 32767;
        else if (sampval < -32768)
            sampval = -32768;
        
        buffer[i * 2] = (int16_t) sampval;
        buffer[i * 2 + 1] = (int16_t) sampval;
    }
}

// Write an OPL register value
void OPL_WriteRegister(int reg, int value)
{
    Chip__WriteReg(&opl_chip, reg, (unsigned char) value);
}

// Initialize registers on startup
void OPL_InitRegisters(void)
{
    int r;

    // Initialize level registers
    for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x3f);
    }

    // Initialize other registers
    for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    for (r=1; r < OPL_REGS_LEVEL; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // "Allow FM chips to control the waveform of each operator":
    OPL_WriteRegister(OPL_REG_WAVEFORM_ENABLE, 0x20);

    // Keyboard split point on (?)
    OPL_WriteRegister(OPL_REG_FM_MODE,         0x40);
}

void OPL_SetPaused(int paused)
{
    opl_paused = paused;
}
