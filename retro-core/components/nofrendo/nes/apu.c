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
** nes/apu.c: Sound emulation (APU)
**
*/

#include "nes.h"

#define APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))

/* Runtime settings */
#define OPT(n) (apu.options[(n)])

/* active APU */
static apu_t apu;

/* vblank length table used for rectangles, triangle, noise */
static const uint8 vbl_length[32] =
{
    5, 127,
   10,   1,
   19,   2,
   40,   3,
   80,   4,
   30,   5,
    7,   6,
   13,   7,
    6,   8,
   12,   9,
   24,  10,
   48,  11,
   96,  12,
   36,  13,
    8,  14,
   16,  15
};

/* frequency limit of rectangle channels */
static const int freq_limit[8] =
{
   0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */
static const int noise_freq[16] =
{
     4,    8,   16,   32,   64,   96,  128,  160,
   202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */
static const int dmc_clocks[16] =
{
   428, 380, 340, 320, 286, 254, 226, 214,
   190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */
static const int duty_flip[4] = { 2, 4, 8, 12 };


IRAM_ATTR void apu_fc_advance(int cycles)
{
   // https://wiki.nesdev.com/w/index.php/APU_Frame_Counter
   const int int_period = 4 * 7457;

   apu.fc.cycles += cycles;
   // apu.fc.step = cycles / 7457;

   if (apu.fc.cycles >= int_period)
   {
      apu.fc.cycles -= int_period;
      // apu.fc.cycles = 0;

      if ((apu.fc.state & 0xC0) == 0)
      {
         apu.fc.irq_occurred = true;

         if (!apu.fc.disable_irq)
            nes6502_irq();
      }
   }
}

void apu_setcontext(apu_t *src_apu)
{
   ASSERT(src_apu);
   apu = *src_apu;
}

void apu_getcontext(apu_t *dest_apu)
{
   ASSERT(dest_apu);
   *dest_apu = apu;
}

static void apu_build_luts(int num_samples)
{
   /* lut used for enveloping and frequency sweeps */
   for (int i = 0; i < 16; i++)
      apu.decay_lut[i] = num_samples * (i + 1);

   /* used for note length, based on vblank and size of audio buffer */
   for (int i = 0; i < 32; i++)
      apu.vbl_lut[i] = vbl_length[i] * num_samples;

   /* triangle wave channel's linear length table */
   for (int i = 0; i < 128; i++)
      apu.trilength_lut[i] = (int) (0.25 * i * num_samples);
}

/* RECTANGLE WAVE
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_MAKE_RECTANGLE(ch) \
static inline int apu_rectangle_##ch(void) \
{ \
   int output, total, num_times; \
\
   APU_VOLUME_DECAY(apu.rectangle[ch].output_vol); \
\
   if (!apu.rectangle[ch].enabled || apu.rectangle[ch].vbl_length == 0) \
      return apu.rectangle[ch].output_vol; \
\
   /* vbl length counter */ \
   if (!apu.rectangle[ch].holdnote) \
      apu.rectangle[ch].vbl_length--; \
\
   /* envelope decay at a rate of (env_delay + 1) / 240 secs */ \
   apu.rectangle[ch].env_phase -= 4; /* 240/60 */ \
   while (apu.rectangle[ch].env_phase < 0) \
   { \
      apu.rectangle[ch].env_phase += apu.rectangle[ch].env_delay; \
\
      if (apu.rectangle[ch].holdnote) \
         apu.rectangle[ch].env_vol = (apu.rectangle[ch].env_vol + 1) & 0x0F; \
      else if (apu.rectangle[ch].env_vol < 0x0F) \
         apu.rectangle[ch].env_vol++; \
   } \
\
   /* TODO: find true relation of freq_limit to register values */ \
   if (apu.rectangle[ch].freq < 8 \
       || (false == apu.rectangle[ch].sweep_inc \
           && apu.rectangle[ch].freq > apu.rectangle[ch].freq_limit)) \
      return apu.rectangle[ch].output_vol; \
\
   /* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */ \
   if (apu.rectangle[ch].sweep_on && apu.rectangle[ch].sweep_shifts) \
   { \
      apu.rectangle[ch].sweep_phase -= 2; /* 120/60 */ \
      while (apu.rectangle[ch].sweep_phase < 0) \
      { \
         apu.rectangle[ch].sweep_phase += apu.rectangle[ch].sweep_delay; \
\
         if (apu.rectangle[ch].sweep_inc) /* ramp up */ \
         { \
            if (ch == 0) \
               apu.rectangle[ch].freq += ~(apu.rectangle[ch].freq >> apu.rectangle[ch].sweep_shifts); \
            else \
               apu.rectangle[ch].freq -= (apu.rectangle[ch].freq >> apu.rectangle[ch].sweep_shifts); \
         } \
         else /* ramp down */ \
         { \
            apu.rectangle[ch].freq += (apu.rectangle[ch].freq >> apu.rectangle[ch].sweep_shifts); \
         } \
      } \
   } \
\
   apu.rectangle[ch].accum -= apu.cycle_rate; \
   if (apu.rectangle[ch].accum >= 0) \
      return apu.rectangle[ch].output_vol; \
\
   if (apu.rectangle[ch].fixed_envelope) \
      output = apu.rectangle[ch].volume << 8; /* fixed volume */ \
   else \
      output = (apu.rectangle[ch].env_vol ^ 0x0F) << 8; \
\
   num_times = total = 0; \
\
   while (apu.rectangle[ch].accum < 0) \
   { \
      apu.rectangle[ch].accum += apu.rectangle[ch].freq + 1; \
      apu.rectangle[ch].adder = (apu.rectangle[ch].adder + 1) & 0x0F; \
\
      if (apu.rectangle[ch].adder < apu.rectangle[ch].duty_flip) \
         total += output; \
      else \
         total -= output; \
\
      num_times++; \
   } \
\
   apu.rectangle[ch].output_vol = total / num_times; \
   return apu.rectangle[ch].output_vol; \
}

/* generate the functions */
APU_MAKE_RECTANGLE(0)
APU_MAKE_RECTANGLE(1)


/* TRIANGLE WAVE
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
static inline int apu_triangle(void)
{
   APU_VOLUME_DECAY(apu.triangle.output_vol);

   if (!apu.triangle.enabled || apu.triangle.vbl_length == 0)
      goto output;

   if (apu.triangle.counter_started)
   {
      if (apu.triangle.linear_length > 0)
         apu.triangle.linear_length--;
      if (apu.triangle.vbl_length && false == apu.triangle.holdnote)
         apu.triangle.vbl_length--;
   }
   else if (false == apu.triangle.holdnote && apu.triangle.write_latency)
   {
      if (--apu.triangle.write_latency == 0)
         apu.triangle.counter_started = true;
   }

   if (apu.triangle.linear_length == 0 || apu.triangle.freq < 4) /* inaudible */
      goto output;

   apu.triangle.accum -= apu.cycle_rate; \
   while (apu.triangle.accum < 0)
   {
      apu.triangle.accum += apu.triangle.freq;
      apu.triangle.adder = (apu.triangle.adder + 1) & 0x1F;

      if (apu.triangle.adder & 0x10)
         apu.triangle.output_vol -= (2 << 8);
      else
         apu.triangle.output_vol += (2 << 8);
   }

output:
   return (apu.triangle.output_vol + (apu.triangle.output_vol >> 2));
}


/* WHITE NOISE CHANNEL
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
static inline int apu_noise(void)
{
   int outvol = 0;
   int num_times = 0;
   int total = 0;

   APU_VOLUME_DECAY(apu.noise.output_vol);

   if (!apu.noise.enabled || apu.noise.vbl_length == 0)
      goto output;

   /* vbl length counter */
   if (!apu.noise.holdnote)
      apu.noise.vbl_length--;

   /* envelope decay at a rate of (env_delay + 1) / 240 secs */
   apu.noise.env_phase -= 4; /* 240/60 */
   while (apu.noise.env_phase < 0)
   {
      apu.noise.env_phase += apu.noise.env_delay;

      if (apu.noise.holdnote)
         apu.noise.env_vol = (apu.noise.env_vol + 1) & 0x0F;
      else if (apu.noise.env_vol < 0x0F)
         apu.noise.env_vol++;
   }

   apu.noise.accum -= apu.cycle_rate;

   if (apu.noise.accum >= 0)
      goto output;

   if (apu.noise.fixed_envelope)
      outvol = apu.noise.volume << 8; /* fixed volume */
   else
      outvol = (apu.noise.env_vol ^ 0x0F) << 8;

   /* emulation of the 15-bit shift register the
   ** NES uses to generate pseudo-random series
   ** for the white noise channel
   */
   while (apu.noise.accum < 0)
   {
      int sreg = apu.noise.shift_reg;
      int tap = (sreg & apu.noise.xor_tap) ? 1 : 0;
      int bit0 = sreg & 1;
      int bit14 = (bit0 ^ tap);

      total += (bit0 ^ 1) ? outvol : -outvol;
      num_times++;

      apu.noise.shift_reg = (bit14 << 14) | (sreg >> 1);
      apu.noise.accum += apu.noise.freq;
   }

   outvol = total / num_times;

   apu.noise.output_vol = outvol;

output:
   return ((apu.noise.output_vol + apu.noise.output_vol + apu.noise.output_vol) >> 2);
}


static inline void apu_dmcreload(void)
{
   apu.dmc.address = apu.dmc.cached_addr;
   apu.dmc.dma_length = apu.dmc.cached_dmalength;
   apu.dmc.irq_occurred = false;
}

/* DELTA MODULATION CHANNEL
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
static inline int apu_dmc(void)
{
   APU_VOLUME_DECAY(apu.dmc.output_vol);

   /* only process when channel is alive */
   if (apu.dmc.dma_length)
   {
      apu.dmc.accum -= apu.cycle_rate;

      while (apu.dmc.accum < 0)
      {
         apu.dmc.accum += apu.dmc.freq;

         int delta_bit = (apu.dmc.dma_length & 7) ^ 7;

         if (7 == delta_bit)
         {
            apu.dmc.cur_byte = mem_getbyte(apu.dmc.address);

            /* steal a cycle from CPU*/
            nes6502_burn(1);

            /* prevent wraparound */
            if (0xFFFF == apu.dmc.address)
               apu.dmc.address = 0x8000;
            else
               apu.dmc.address++;
         }

         if (--apu.dmc.dma_length == 0)
         {
            /* if loop bit set, we're cool to retrigger sample */
            if (apu.dmc.looping)
            {
               apu_dmcreload();
            }
            else
            {
               /* check to see if we should generate an irq */
               if (apu.dmc.irq_gen)
               {
                  apu.dmc.irq_occurred = true;
                  nes6502_irq();
               }

               /* bodge for timestamp queue */
               apu.dmc.enabled = false;
               break;
            }
         }

         /* positive delta */
         if (apu.dmc.cur_byte & (1 << delta_bit))
         {
            if (apu.dmc.regs[1] < 0x7D)
            {
               apu.dmc.regs[1] += 2;
               apu.dmc.output_vol += (2 << 8);
            }
         }
         /* negative delta */
         else
         {
            if (apu.dmc.regs[1] > 1)
            {
               apu.dmc.regs[1] -= 2;
               apu.dmc.output_vol -= (2 << 8);
            }
         }
      }
   }

   return ((apu.dmc.output_vol + apu.dmc.output_vol + apu.dmc.output_vol) >> 2);
}


IRAM_ATTR void apu_write(uint32 address, uint8 value)
{
   int chan;

   switch (address)
   {
   /* rectangles */
   case APU_WRA0:
   case APU_WRB0:
      chan = (address & 4) >> 2;
      apu.rectangle[chan].regs[0] = value;
      apu.rectangle[chan].volume = value & 0x0F;
      apu.rectangle[chan].env_delay = apu.decay_lut[value & 0x0F];
      apu.rectangle[chan].holdnote = (value >> 5) & 1;
      apu.rectangle[chan].fixed_envelope = (value >> 4) & 1;
      apu.rectangle[chan].duty_flip = duty_flip[value >> 6];
      break;

   case APU_WRA1:
   case APU_WRB1:
      chan = (address & 4) >> 2;
      apu.rectangle[chan].regs[1] = value;
      apu.rectangle[chan].sweep_on = (value >> 7) & 1;
      apu.rectangle[chan].sweep_shifts = value & 7;
      apu.rectangle[chan].sweep_delay = apu.decay_lut[(value >> 4) & 7];
      apu.rectangle[chan].sweep_inc = (value >> 3) & 1;
      apu.rectangle[chan].freq_limit = freq_limit[value & 7];
      break;

   case APU_WRA2:
   case APU_WRB2:
      chan = (address & 4) >> 2;
      apu.rectangle[chan].regs[2] = value;
      apu.rectangle[chan].freq = (apu.rectangle[chan].freq & ~0xFF) | value;
      break;

   case APU_WRA3:
   case APU_WRB3:
      chan = (address & 4) >> 2;
      apu.rectangle[chan].regs[3] = value;
      apu.rectangle[chan].vbl_length = apu.vbl_lut[value >> 3];
      apu.rectangle[chan].env_vol = 0;
      apu.rectangle[chan].freq = ((value & 7) << 8) | (apu.rectangle[chan].freq & 0xFF);
      apu.rectangle[chan].adder = 0;
      break;

   /* triangle */
   case APU_WRC0:
      apu.triangle.regs[0] = value;
      apu.triangle.holdnote = (value >> 7) & 1;

      if (false == apu.triangle.counter_started && apu.triangle.vbl_length)
         apu.triangle.linear_length = apu.trilength_lut[value & 0x7F];

      break;

   case APU_WRC2:
      apu.triangle.regs[1] = value;
      apu.triangle.freq = (((apu.triangle.regs[2] & 7) << 8) + value) + 1;
      break;

   case APU_WRC3:

      apu.triangle.regs[2] = value;

      /* this is somewhat of a hack.  there appears to be some latency on
      ** the Real Thing between when trireg0 is written to and when the
      ** linear length counter actually begins its countdown.  we want to
      ** prevent the case where the program writes to the freq regs first,
      ** then to reg 0, and the counter accidentally starts running because
      ** of the sound queue's timestamp processing.
      **
      ** set latency to a couple hundred cycles -- should be plenty of time
      ** for the 6502 code to do a couple of table dereferences and load up
      ** the other triregs
      */
      apu.triangle.write_latency = (int) (228 / apu.cycle_rate);
      apu.triangle.freq = (((value & 7) << 8) + apu.triangle.regs[1]) + 1;
      apu.triangle.vbl_length = apu.vbl_lut[value >> 3];
      apu.triangle.counter_started = false;
      apu.triangle.linear_length = apu.trilength_lut[apu.triangle.regs[0] & 0x7F];
      break;

   /* noise */
   case APU_WRD0:
      apu.noise.regs[0] = value;
      apu.noise.env_delay = apu.decay_lut[value & 0x0F];
      apu.noise.holdnote = (value >> 5) & 1;
      apu.noise.fixed_envelope = (value >> 4) & 1;
      apu.noise.volume = value & 0x0F;
      break;

   case APU_WRD2:
      apu.noise.regs[1] = value;
      apu.noise.freq = noise_freq[value & 0x0F];
      apu.noise.xor_tap = (value & 0x80) ? 0x40: 0x02;
      break;

   case APU_WRD3:
      apu.noise.regs[2] = value;
      apu.noise.vbl_length = apu.vbl_lut[value >> 3];
      apu.noise.env_vol = 0; /* reset envelope */
      break;

   /* DMC */
   case APU_WRE0:
      apu.dmc.regs[0] = value;
      apu.dmc.freq = dmc_clocks[value & 0x0F];
      apu.dmc.looping = (value >> 6) & 1;

      if (value & 0x80)
      {
         apu.dmc.irq_gen = true;
      }
      else
      {
         apu.dmc.irq_gen = false;
         apu.dmc.irq_occurred = false;
      }
      break;

   case APU_WRE1: /* 7-bit DAC */
      /* add the _delta_ between written value and
      ** current output level of the volume reg
      */
      value &= 0x7F; /* bit 7 ignored */
      apu.dmc.output_vol += ((value - apu.dmc.regs[1]) << 8);
      apu.dmc.regs[1] = value;
      break;

   case APU_WRE2:
      apu.dmc.regs[2] = value;
      apu.dmc.cached_addr = 0xC000 + (uint16) (value << 6);
      break;

   case APU_WRE3:
      apu.dmc.regs[3] = value;
      apu.dmc.cached_dmalength = ((value << 4) + 1) << 3;
      break;

   case APU_SMASK:
      /* bodge for timestamp queue */
      apu.dmc.enabled = (value >> 4) & 1;
      apu.control_reg = value;

      for (chan = 0; chan < 2; chan++)
      {
         if (value & (1 << chan))
         {
            apu.rectangle[chan].enabled = true;
         }
         else
         {
            apu.rectangle[chan].enabled = false;
            apu.rectangle[chan].vbl_length = 0;
         }
      }

      if (value & 0x04)
      {
         apu.triangle.enabled = true;
      }
      else
      {
         apu.triangle.enabled = false;
         apu.triangle.vbl_length = 0;
         apu.triangle.linear_length = 0;
         apu.triangle.counter_started = false;
         apu.triangle.write_latency = 0;
      }

      if (value & 0x08)
      {
         apu.noise.enabled = true;
      }
      else
      {
         apu.noise.enabled = false;
         apu.noise.vbl_length = 0;
      }

      if (value & 0x10)
      {
         if (apu.dmc.dma_length == 0)
            apu_dmcreload();
      }
      else
      {
         apu.dmc.dma_length = 0;
      }

      apu.dmc.irq_occurred = false;
      break;

   case APU_FRAME_IRQ: /* frame IRQ control */
      apu.fc.state = value;
      apu.fc.cycles = 0; // 3-4 cpu cycles before reset
      apu.fc.irq_occurred = false;
      break;

      /* unused, but they get hit in some mem-clear loops */
   case 0x4009:
   case 0x400D:
      break;

   default:
      break;
   }
}

/* Read from $4000-$4017 */
IRAM_ATTR uint8 apu_read(uint32 address)
{
   uint8 value;

   switch (address)
   {
   case APU_SMASK:
      value = 0;
      /* Return 1 in 0-5 bit pos if a channel is playing */
      if (apu.rectangle[0].enabled && apu.rectangle[0].vbl_length)
         value |= 0x01;
      if (apu.rectangle[1].enabled && apu.rectangle[1].vbl_length)
         value |= 0x02;
      if (apu.triangle.enabled && apu.triangle.vbl_length)
         value |= 0x04;
      if (apu.noise.enabled && apu.noise.vbl_length)
         value |= 0x08;

      /* bodge for timestamp queue */
      if (apu.dmc.enabled)
         value |= 0x10;

      if (apu.dmc.irq_occurred)
         value |= 0x80;

      if (apu.fc.irq_occurred) {
         value |= 0x40;
         apu.fc.irq_occurred = false;
      }

      break;

   default:
      value = (address >> 8); /* heavy capacitance on data bus */
      break;
   }

   return value;
}

void apu_process(short *buffer, size_t num_samples, bool stereo)
{
   int prev_sample = apu.prev_sample;

   if (!buffer)
      return;

   while (num_samples--)
   {
      int accum = 0;

      // if (OPT(APU_CHANNEL1_EN))
         accum += apu_rectangle_0();
      // if (OPT(APU_CHANNEL2_EN))
         accum += apu_rectangle_1();
      // if (OPT(APU_CHANNEL3_EN))
         accum += apu_triangle();
      // if (OPT(APU_CHANNEL4_EN))
         accum += apu_noise();
      // if (OPT(APU_CHANNEL5_EN))
         accum += apu_dmc();
      if (apu.ext) // && OPT(APU_CHANNEL6_EN))
         accum += apu.ext->process();

      /* do any filtering */
      if (OPT(APU_FILTER_TYPE) == APU_FILTER_WEIGHTED)
      {
         accum = (accum + accum + accum + prev_sample) >> 2;
      }
      else if (OPT(APU_FILTER_TYPE) == APU_FILTER_LOWPASS)
      {
         accum += prev_sample;
         accum >>= 1;
      }
      prev_sample = accum;

      /* do clipping */
      if (accum > 0x7FFF)
         accum = 0x7FFF;
      else if (accum < -0x8000)
         accum = -0x8000;

      /* signed 16-bit output */
      *buffer++ = (short) accum;

      if (stereo)
         *buffer++ = (short) accum;

      // Advance frame counter
      // apu_fc_advance(apu.cycle_rate);
   }

   apu.prev_sample = prev_sample;
}

void apu_emulate(void)
{
   // Run for one frame
   apu_process(apu.buffer, apu.samples_per_frame, apu.stereo);
}

void apu_setopt(apu_option_t n, int val)
{
   // Some options need special care
   switch (n)
   {
      default:
      break;
   }

   apu.options[n] = val;
}

int apu_getopt(apu_option_t n)
{
   return apu.options[n];
}

void apu_reset(void)
{
   /* Update region if needed */
   apu.samples_per_frame = apu.sample_rate / NES_REFRESH_RATE;
   apu.cycle_rate = (float)NES_CPU_CLOCK / apu.sample_rate;
   apu.noise.shift_reg = 0x4000;
   apu_build_luts(apu.samples_per_frame);

   /* initialize all channel members */
   for (uint32 addr = 0x4000; addr <= 0x4013; addr++)
      apu_write(addr, 0);

   apu_write(APU_SMASK, 0x00);
   apu_write(APU_FRAME_IRQ, 0x80); // nesdev wiki says this should be 0, but it seems to work better disabled

   if (apu.ext && apu.ext->reset)
      apu.ext->reset();
}

apu_t *apu_init(int sample_rate, bool stereo)
{
   memset(&apu, 0, sizeof(apu_t));

   apu.buffer = calloc(sample_rate / 50 + 2, stereo ? 4 : 2);
   apu.sample_rate = sample_rate;
   apu.stereo = stereo;
   apu.ext = NULL;

   apu_setopt(APU_FILTER_TYPE, APU_FILTER_WEIGHTED);
   apu_setopt(APU_CHANNEL1_EN, true);
   apu_setopt(APU_CHANNEL2_EN, true);
   apu_setopt(APU_CHANNEL3_EN, true);
   apu_setopt(APU_CHANNEL4_EN, true);
   apu_setopt(APU_CHANNEL5_EN, true);
   apu_setopt(APU_CHANNEL6_EN, true);

   return &apu;
}

void apu_shutdown()
{
   if (apu.ext && apu.ext->shutdown)
      apu.ext->shutdown();
   free(apu.buffer);
   apu.buffer = NULL;
}

void apu_setext(apuext_t *ext)
{
   apu.ext = ext;

   /* initialize it */
   if (apu.ext && NULL != apu.ext->init)
      apu.ext->init();
}
