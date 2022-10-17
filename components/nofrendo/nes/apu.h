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
** nes/apu.h: Sound emulation header
**
*/

#pragma once

#define  APU_WRA0       0x4000
#define  APU_WRA1       0x4001
#define  APU_WRA2       0x4002
#define  APU_WRA3       0x4003
#define  APU_WRB0       0x4004
#define  APU_WRB1       0x4005
#define  APU_WRB2       0x4006
#define  APU_WRB3       0x4007
#define  APU_WRC0       0x4008
#define  APU_WRC2       0x400A
#define  APU_WRC3       0x400B
#define  APU_WRD0       0x400C
#define  APU_WRD2       0x400E
#define  APU_WRD3       0x400F
#define  APU_WRE0       0x4010
#define  APU_WRE1       0x4011
#define  APU_WRE2       0x4012
#define  APU_WRE3       0x4013

#define  APU_SMASK      0x4015
#define  APU_FRAME_IRQ  0x4017

/* length of generated noise */
#define  APU_NOISE_32K  0x7FFF
#define  APU_NOISE_93   93


/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/

typedef struct
{
   uint8 regs[4];

   bool enabled;

   float accum;
   int freq;
   int output_vol;
   int fixed_envelope;
   int holdnote;
   int volume;

   int sweep_phase;
   int sweep_delay;
   int sweep_on;
   int sweep_shifts;
   int sweep_inc;

   /* this may not be necessary in the future */
   int freq_limit;
   int env_phase;
   int env_delay;
   int env_vol;

   int vbl_length;
   int adder;
   int duty_flip;
} rectangle_t;

typedef struct
{
   uint8 regs[3];

   bool enabled;

   float accum;
   int freq;
   int output_vol;

   int holdnote;
   int counter_started;
   /* quasi-hack */
   int write_latency;

   int vbl_length;
   int adder;
   int linear_length;
} triangle_t;


typedef struct
{
   uint8 regs[3];

   bool enabled;

   float accum;
   int freq;
   int output_vol;

   int env_phase;
   int env_delay;
   int env_vol;
   int fixed_envelope;
   int holdnote;

   uint8 volume;
   uint8 xor_tap;
   uint16 shift_reg;

   int vbl_length;
} noise_t;

typedef struct
{
   uint8 regs[4];

   /* bodge for timestamp queue */
   bool enabled;
   bool looping;
   bool irq_gen;
   bool irq_occurred;

   float accum;
   int freq;
   int output_vol;

   int address;
   int cached_addr;
   int dma_length;
   int cached_dmalength;
   int cur_byte;
} dmc_t;

enum
{
   APU_FILTER_NONE,
   APU_FILTER_LOWPASS,
   APU_FILTER_WEIGHTED
};

/* external sound chip stuff */
typedef struct
{
   int   (*init)(void);
   void  (*shutdown)(void);
   void  (*reset)(void);
   int   (*process)(void);
} apuext_t;

typedef enum
{
   APU_FILTER_TYPE,
   APU_CHANNEL1_EN,
   APU_CHANNEL2_EN,
   APU_CHANNEL3_EN,
   APU_CHANNEL4_EN,
   APU_CHANNEL5_EN,
   APU_CHANNEL6_EN,
} apu_option_t;

typedef struct
{
   rectangle_t rectangle[2];
   triangle_t triangle;
   noise_t noise;
   dmc_t dmc;
   uint8 control_reg;

   int samples_per_frame;
   int sample_rate;
   bool stereo;

   short *buffer;

   int prev_sample;

   float cycle_rate;

   struct {
      unsigned state;
      unsigned step;
      unsigned cycles;
      bool irq_occurred;
      bool disable_irq;
   } fc;

   /* look up table madness */
   int decay_lut[16];
   int vbl_lut[32];
   int trilength_lut[128];

   /* external sound chip */
   apuext_t *ext;

   /* Misc runtime options */
   int options[16];
} apu_t;

/* Function prototypes */
apu_t *apu_init(int sample_rate, bool stereo);
void apu_reset(void);
void apu_shutdown(void);
void apu_setext(apuext_t *ext);

void apu_emulate(void);

void apu_setopt(apu_option_t n, int val);
int  apu_getopt(apu_option_t n);

void apu_setcontext(apu_t *src_apu);
void apu_getcontext(apu_t *dest_apu);

void apu_process(short *buffer, size_t num_samples, bool stereo);
void apu_fc_advance(int cycles);

uint8 apu_read(uint32 address);
void apu_write(uint32 address, uint8 value);
