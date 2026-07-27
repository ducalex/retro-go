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


#ifndef OPL_OPL_H
#define OPL_OPL_H

#include <stdint.h>

typedef enum
{
    OPL_REGISTER_PORT = 0,
    OPL_DATA_PORT = 1
} opl_port_t;

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES      9

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER2            0x03
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08

// Operator registers (21 of each):

#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80
#define OPL_REGS_WAVEFORM         0xE0

// Voice registers (9 of each):

#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0
#define OPL_REGS_FEEDBACK         0xC0

//
// Low-level functions.
//

// Initialize the OPL subsystem.
int OPL_Init(unsigned int samplerate);

// Shut down the OPL subsystem.
void OPL_Shutdown(void);

// Write to an OPL register.
void OPL_WriteRegister(int reg, int value);

// Initialize all registers, performed on startup.
void OPL_InitRegisters(void);

// Pause the OPL rendering.
void OPL_SetPaused(int paused);

extern unsigned int opl_sample_rate;

// Render OPL samples into the destination buffer.
void OPL_Render_Samples(void *dest, unsigned nsamp);

#endif
