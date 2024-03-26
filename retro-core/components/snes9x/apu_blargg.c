/* This file is part of Snes9x. See LICENSE file. */

#ifdef USE_BLARGG_APU

#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

#include "blargg_endian.h"
#include "apu_blargg.h"

#include "snes9x.h"
#include "display.h"


/***********************************************************************************
   SPC DSP
***********************************************************************************/

static dsp_state_t dsp_m;

/* Copyright (C) 2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#define CLAMP16(io) \
{ \
   if ((int16_t) io != io) \
      io = (io >> 31) ^ 0x7FFF; \
}

/* Access global DSP register */
#define REG(n) dsp_m.regs [R_##n]

/* Access voice DSP register */
#define VREG(r,n) r [V_##n]

#define WRITE_SAMPLES(l, r, out) \
{\
   out [0] = l; \
   out [1] = r; \
   out += 2; \
   if ( out >= dsp_m.out_end ) \
   { \
      out       = dsp_m.extra; \
      dsp_m.out_end = &dsp_m.extra [EXTRA_SIZE]; \
   } \
}

/* Volume registers and efb are signed! Easy to forget int8_t cast. */
/* Prefixes are to avoid accidental use of locals with same names. */

/* Gaussian interpolation */

static int16_t gauss [512] =
{
0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
2,   2,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,
6,   6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
11,  11,  11,  12,  12,  13,  13,  14,  14,  15,  15,  15,  16,  16,  17,  17,
18,  19,  19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  27,  27,
28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  36,  36,  37,  38,  39,  40,
41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
58,  59,  60,  61,  62,  64,  65,  66,  67,  69,  70,  71,  73,  74,  76,  77,
78,  80,  81,  83,  84,  86,  87,  89,  90,  92,  94,  95,  97,  99,  100, 102,
104, 106, 107, 109, 111, 113, 115, 117, 118, 120, 122, 124, 126, 128, 130, 132,
134, 137, 139, 141, 143, 145, 147, 150, 152, 154, 156, 159, 161, 163, 166, 168,
171, 173, 175, 178, 180, 183, 186, 188, 191, 193, 196, 199, 201, 204, 207, 210,
212, 215, 218, 221, 224, 227, 230, 233, 236, 239, 242, 245, 248, 251, 254, 257,
260, 263, 267, 270, 273, 276, 280, 283, 286, 290, 293, 297, 300, 304, 307, 311,
314, 318, 321, 325, 328, 332, 336, 339, 343, 347, 351, 354, 358, 362, 366, 370,
374, 378, 381, 385, 389, 393, 397, 401, 405, 410, 414, 418, 422, 426, 430, 434,
439, 443, 447, 451, 456, 460, 464, 469, 473, 477, 482, 486, 491, 495, 499, 504,
508, 513, 517, 522, 527, 531, 536, 540, 545, 550, 554, 559, 563, 568, 573, 577,
582, 587, 592, 596, 601, 606, 611, 615, 620, 625, 630, 635, 640, 644, 649, 654,
659, 664, 669, 674, 678, 683, 688, 693, 698, 703, 708, 713, 718, 723, 728, 732,
737, 742, 747, 752, 757, 762, 767, 772, 777, 782, 787, 792, 797, 802, 806, 811,
816, 821, 826, 831, 836, 841, 846, 851, 855, 860, 865, 870, 875, 880, 884, 889,
894, 899, 904, 908, 913, 918, 923, 927, 932, 937, 941, 946, 951, 955, 960, 965,
969, 974, 978, 983, 988, 992, 997, 1001,1005,1010,1014,1019,1023,1027,1032,1036,
1040,1045,1049,1053,1057,1061,1066,1070,1074,1078,1082,1086,1090,1094,1098,1102,
1106,1109,1113,1117,1121,1125,1128,1132,1136,1139,1143,1146,1150,1153,1157,1160,
1164,1167,1170,1174,1177,1180,1183,1186,1190,1193,1196,1199,1202,1205,1207,1210,
1213,1216,1219,1221,1224,1227,1229,1232,1234,1237,1239,1241,1244,1246,1248,1251,
1253,1255,1257,1259,1261,1263,1265,1267,1269,1270,1272,1274,1275,1277,1279,1280,
1282,1283,1284,1286,1287,1288,1290,1291,1292,1293,1294,1295,1296,1297,1297,1298,
1299,1300,1300,1301,1302,1302,1303,1303,1303,1304,1304,1304,1304,1304,1305,1305,
};

/* Gaussian interpolation */

static INLINE int32_t dsp_interpolate( dsp_voice_t *v )
{
   int32_t offset, out, *in;
   int16_t *fwd, *rev;

   /* Make pointers into gaussian based on fractional position between samples */
   offset = v->interp_pos >> 4 & 0xFF;
   fwd = gauss + 255 - offset;
   rev = gauss       + offset; /* mirror left half of gaussian */

   in = &v->buf [(v->interp_pos >> 12) + v->buf_pos];
   out  = (fwd [  0] * in [0]) >> 11;
   out += (fwd [256] * in [1]) >> 11;
   out += (rev [256] * in [2]) >> 11;
   out = (int16_t) out;
   out += (rev [  0] * in [3]) >> 11;

   CLAMP16( out );
   out &= ~1;
   return out;
}

/* Counters */

/* 30720 =  2048 * 5 * 3 */
#define SIMPLE_COUNTER_RANGE 30720

static uint32_t const counter_rates [32] =
{
   SIMPLE_COUNTER_RANGE + 1, /* never fires */
         2048, 1536,
   1280, 1024,  768,
    640,  512,  384,
    320,  256,  192,
    160,  128,   96,
     80,   64,   48,
     40,   32,   24,
     20,   16,   12,
     10,    8,    6,
      5,    4,    3,
            2,
            1
};

static uint32_t const counter_offsets [32] =
{
     1, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
   536, 0, 1040,
        0,
        0
};

#define RUN_COUNTERS() \
   if (--dsp_m.counter < 0) \
      dsp_m.counter = SIMPLE_COUNTER_RANGE - 1;

#define READ_COUNTER(rate) (((uint32_t) dsp_m.counter + counter_offsets [rate]) % counter_rates [rate])

/* Envelope */

static INLINE void dsp_run_envelope( dsp_voice_t* const v )
{
   int32_t env, rate, env_data;

   env = v->env;
   env_data = v->regs[V_ADSR1];

   if ( dsp_m.t_adsr0 & 0x80 ) /* 99% ADSR */
   {
      if ( v->env_mode >= ENV_DECAY ) /* 99% */
      {
         env--;
         env -= env >> 8;
         rate = env_data & 0x1F;
         if ( v->env_mode == ENV_DECAY ) /* 1% */
            rate = (dsp_m.t_adsr0 >> 3 & 0x0E) + 0x10;
      }
      else /* ENV_ATTACK */
      {
         rate = (dsp_m.t_adsr0 & 0x0F) * 2 + 1;
         env += rate < 31 ? 0x20 : 0x400;
      }
   }
   else /* GAIN */
   {
      int32_t mode;
      env_data = v->regs[V_GAIN];
      mode = env_data >> 5;
      if ( mode < 4 ) /* direct */
      {
         env = env_data * 0x10;
         rate = 31;
      }
      else
      {
         rate = env_data & 0x1F;
         if ( mode == 4 ) /* 4: linear decrease */
         {
            env -= 0x20;
         }
         else if ( mode < 6 ) /* 5: exponential decrease */
         {
            env--;
            env -= env >> 8;
         }
         else /* 6,7: linear increase */
         {
            env += 0x20;
            if ( mode > 6 && (uint32_t) v->hidden_env >= 0x600 )
               env += 0x8 - 0x20; /* 7: two-slope linear increase */
         }
      }
   }

   /* Sustain level */
   if ( (env >> 8) == (env_data >> 5) && v->env_mode == ENV_DECAY )
      v->env_mode = ENV_SUSTAIN;

   v->hidden_env = env;

   /* unsigned cast because linear decrease going negative also triggers this */
   if ( (uint32_t) env > 0x7FF )
   {
      env = (env < 0 ? 0 : 0x7FF);
      if ( v->env_mode == ENV_ATTACK )
         v->env_mode = ENV_DECAY;
   }

   if (!READ_COUNTER( rate ))
      v->env = env; /* nothing else is controlled by the counter */
}

/* BRR Decoding */

static INLINE void dsp_decode_brr( dsp_voice_t* v )
{
   int32_t nybbles, *pos, *end, header;

   /* Arrange the four input nybbles in 0xABCD order for easy decoding */
   nybbles = dsp_m.t_brr_byte * 0x100 + dsp_m.ram [(v->brr_addr + v->brr_offset + 1) & 0xFFFF];

   header = dsp_m.t_brr_header;

   /* Write to next four samples in circular buffer */
   pos = &v->buf [v->buf_pos];

   if ( (v->buf_pos += 4) >= BRR_BUF_SIZE )
      v->buf_pos = 0;

   /* Decode four samples */
   for ( end = pos + 4; pos < end; pos++, nybbles <<= 4 )
   {
      int32_t filter, p1, p2, s, shift;
      /* Extract nybble and sign-extend */
      s = (int16_t) nybbles >> 12;

      /* Shift sample based on header */
      shift = header >> 4;
      s = (s << shift) >> 1;
      if ( shift >= 0xD ) /* handle invalid range */
         s = (s >> 25) << 11; /* same as: s = (s < 0 ? -0x800 : 0) */

      /* Apply IIR filter (8 is the most commonly used) */
      filter = header & 0x0C;
      p1 = pos [BRR_BUF_SIZE - 1];
      p2 = pos [BRR_BUF_SIZE - 2] >> 1;
      if ( filter >= 8 )
      {
         s += p1;
         s -= p2;
         if ( filter == 8 ) /* s += p1 * 0.953125 - p2 * 0.46875 */
         {
            s += p2 >> 4;
            s += (p1 * -3) >> 6;
         }
         else /* s += p1 * 0.8984375 - p2 * 0.40625 */
         {
            s += (p1 * -13) >> 7;
            s += (p2 * 3) >> 4;
         }
      }
      else if ( filter ) /* s += p1 * 0.46875 */
      {
         s += p1 >> 1;
         s += (-p1) >> 5;
      }

      /* Adjust and write sample */
      CLAMP16( s );
      s = (int16_t) (s * 2);
      pos [BRR_BUF_SIZE] = pos [0] = s; /* second copy simplifies wrap-around */
   }
}

/* Misc */

/* voice 0 doesn't support PMON */

#define MISC_27() dsp_m.t_pmon = dsp_m.regs[R_PMON] & 0xFE;

#define MISC_28() \
   dsp_m.t_non = dsp_m.regs[R_NON]; \
   dsp_m.t_eon = dsp_m.regs[R_EON]; \
   dsp_m.t_dir = dsp_m.regs[R_DIR];

#define MISC_29() \
   if ( (dsp_m.every_other_sample ^= 1) != 0 ) \
      dsp_m.new_kon &= ~dsp_m.kon; /* clears KON 63 clocks after it was last read */

static INLINE void dsp_misc_30()
{
   if ( dsp_m.every_other_sample )
   {
      dsp_m.kon    = dsp_m.new_kon;
      dsp_m.t_koff = dsp_m.regs[R_KOFF];
   }

   RUN_COUNTERS();

   /* Noise */
   if ( !READ_COUNTER( dsp_m.regs[R_FLG] & 0x1F ) )
   {
      int32_t feedback = (dsp_m.noise << 13) ^ (dsp_m.noise << 14);
      dsp_m.noise = (feedback & 0x4000) ^ (dsp_m.noise >> 1);
   }
}

/* Voices */

static INLINE void dsp_voice_V1( dsp_voice_t* const v )
{
   dsp_m.t_dir_addr = dsp_m.t_dir * 0x100 + dsp_m.t_srcn * 4;
   dsp_m.t_srcn = v->regs[V_SRCN];
}

static INLINE void dsp_voice_V2( dsp_voice_t* const v )
{
   uint8_t *entry;

   entry = &dsp_m.ram [dsp_m.t_dir_addr];
   if ( !v->kon_delay )
      entry += 2;

   dsp_m.t_brr_next_addr = GET_LE16( entry );

   dsp_m.t_adsr0 = v->regs [V_ADSR0];


   dsp_m.t_pitch = v->regs [V_PITCHL];
}

static INLINE void dsp_voice_V3a( dsp_voice_t* const v )
{
   dsp_m.t_pitch += (v->regs [V_PITCHH] & 0x3F) << 8;
}

static INLINE void dsp_voice_V3b( dsp_voice_t* const v )
{
   dsp_m.t_brr_byte = dsp_m.ram [(v->brr_addr + v->brr_offset) & 0xffff];
   dsp_m.t_brr_header = dsp_m.ram [v->brr_addr];
}

static void dsp_voice_V3c( dsp_voice_t* const v )
{
   int32_t output;

   /* Pitch modulation using previous voice's output */
   if ( dsp_m.t_pmon & v->vbit )
      dsp_m.t_pitch += ((dsp_m.t_output >> 5) * dsp_m.t_pitch) >> 10;

   if ( v->kon_delay )
   {
      /* Get ready to start BRR decoding on next sample */
      if ( v->kon_delay == 5 )
      {
         v->brr_addr    = dsp_m.t_brr_next_addr;
         v->brr_offset  = 1;
         v->buf_pos     = 0;
         dsp_m.t_brr_header = 0; /* header is ignored on this sample */
      }

      /* Envelope is never run during KON */
      v->env        = 0;
      v->hidden_env = 0;

      /* Disable BRR decoding until last three samples */
      v->interp_pos = 0;
      if ( --v->kon_delay & 3 )
         v->interp_pos = 0x4000;

      /* Pitch is never added during KON */
      dsp_m.t_pitch = 0;
   }

   output = dsp_interpolate( v );

   /* Noise */
   if ( dsp_m.t_non & v->vbit )
      output = (int16_t) (dsp_m.noise * 2);

   /* Apply envelope */
   dsp_m.t_output = (output * v->env) >> 11 & ~1;
   v->t_envx_out = (uint8_t) (v->env >> 4);

   /* Immediate silence due to end of sample or soft reset */
   if ( dsp_m.regs[R_FLG] & 0x80 || (dsp_m.t_brr_header & 3) == 1 )
   {
      v->env_mode = ENV_RELEASE;
      v->env      = 0;
   }

   if ( dsp_m.every_other_sample )
   {
      /* KOFF */
      if ( dsp_m.t_koff & v->vbit )
         v->env_mode = ENV_RELEASE;

      /* KON */
      if ( dsp_m.kon & v->vbit )
      {
         v->kon_delay = 5;
         v->env_mode  = ENV_ATTACK;
      }
   }

   /* Run envelope for next sample */
   if ( !v->kon_delay )
   {
      int32_t env = v->env;
      if ( v->env_mode == ENV_RELEASE ) /* 60% */
      {
         if ( (env -= 0x8) < 0 )
            env = 0;
         v->env = env;
      }
      else
      {
         dsp_run_envelope( v );
      }
   }
}

static INLINE void dsp_voice_output( dsp_voice_t const* v, int32_t ch )
{
   int32_t amp;

   /* Apply left/right volume */
   amp = (dsp_m.t_output * (int8_t) VREG(v->regs,VOLL + ch)) >> 7;

   /* Add to output total */
   dsp_m.t_main_out [ch] += amp;
   CLAMP16( dsp_m.t_main_out [ch] );

   /* Optionally add to echo total */
   if ( dsp_m.t_eon & v->vbit )
   {
      dsp_m.t_echo_out [ch] += amp;
      CLAMP16( dsp_m.t_echo_out [ch] );
   }
}

static INLINE void dsp_voice_V4( dsp_voice_t* const v )
{
   /* Decode BRR */
   dsp_m.t_looped = 0;
   if ( v->interp_pos >= 0x4000 )
   {
      dsp_decode_brr( v );

      if ( (v->brr_offset += 2) >= BRR_BLOCK_SIZE )
      {
         /* Start decoding next BRR block */
         v->brr_addr = (v->brr_addr + BRR_BLOCK_SIZE) & 0xFFFF;
         if ( dsp_m.t_brr_header & 1 )
         {
            v->brr_addr = dsp_m.t_brr_next_addr;
            dsp_m.t_looped = v->vbit;
         }
         v->brr_offset = 1;
      }
   }

   /* Apply pitch */
   v->interp_pos = (v->interp_pos & 0x3FFF) + dsp_m.t_pitch;

   /* Keep from getting too far ahead (when using pitch modulation) */
   if ( v->interp_pos > 0x7FFF )
      v->interp_pos = 0x7FFF;

   /* Output left */
   dsp_voice_output( v, 0 );
}

static INLINE void dsp_voice_V5( dsp_voice_t* const v )
{
   int32_t endx_buf;
   /* Output right */
   dsp_voice_output( v, 1 );

   /* ENDX, OUTX, and ENVX won't update if you wrote to them 1-2 clocks earlier */
   endx_buf = dsp_m.regs[R_ENDX] | dsp_m.t_looped;

   /* Clear bit in ENDX if KON just began */
   if ( v->kon_delay == 5 )
      endx_buf &= ~v->vbit;
   dsp_m.endx_buf = (uint8_t) endx_buf;
}

static INLINE void dsp_voice_V6( dsp_voice_t* const v )
{
   (void) v; /* avoid compiler warning about unused v */
   dsp_m.outx_buf = (uint8_t) (dsp_m.t_output >> 8);
}

static INLINE void dsp_voice_V7( dsp_voice_t* const v )
{
   /* Update ENDX */
   dsp_m.regs[R_ENDX] = dsp_m.endx_buf;

   dsp_m.envx_buf = v->t_envx_out;
}

static INLINE void dsp_voice_V8( dsp_voice_t* const v )
{
   /* Update OUTX */
   v->regs [V_OUTX] = dsp_m.outx_buf;
}

static INLINE void dsp_voice_V9( dsp_voice_t* const v )
{
   v->regs [V_ENVX] = dsp_m.envx_buf;
}

/* Most voices do all these in one clock, so make a handy composite */

static INLINE void dsp_voice_V3( dsp_voice_t* const v )
{
   dsp_voice_V3a( v );
   dsp_voice_V3b( v );
   dsp_voice_V3c( v );
}

/* Common combinations of voice steps on different voices. This greatly reduces
   code size and allows everything to be INLINEd in these functions. */

static void dsp_voice_V7_V4_V1( dsp_voice_t* const v )
{
   dsp_voice_V7(v);
   dsp_voice_V1(v+3);
   dsp_voice_V4(v+1);
}

static void dsp_voice_V8_V5_V2( dsp_voice_t* const v )
{
   dsp_voice_V8(v);
   dsp_voice_V5(v+1);
   dsp_voice_V2(v+2);
}

static void dsp_voice_V9_V6_V3( dsp_voice_t* const v )
{
   dsp_voice_V9(v);
   dsp_voice_V6(v+1);
   dsp_voice_V3(v+2);
}

/* Echo */

/* Current echo buffer pointer for left/right channel */
#define ECHO_PTR( ch )      (&dsp_m.ram [dsp_m.t_echo_ptr + ch * 2])

/* Sample in echo history buffer, where 0 is the oldest */
#define ECHO_FIR( i )       (dsp_m.echo_hist_pos [i])

/* Calculate FIR point for left/right channel */
#define CALC_FIR( i, ch )   ((ECHO_FIR( i + 1 ) [ch] * (int8_t) REG(FIR + i * 0x10)) >> 6)

#define ECHO_READ(ch) \
{ \
   int32_t s; \
   if ( dsp_m.t_echo_ptr >= 0xffc0 && dsp_m.rom_enabled ) \
      s = GET_LE16SA( &dsp_m.hi_ram [dsp_m.t_echo_ptr + ch * 2 - 0xffc0] ); \
   else \
      s = GET_LE16SA( ECHO_PTR( ch ) ); \
   /* second copy simplifies wrap-around handling */ \
   ECHO_FIR( 0 ) [ch] = ECHO_FIR( 8 ) [ch] = s >> 1; \
}

static INLINE void dsp_echo_22()
{
   int32_t l, r;

   if (++dsp_m.echo_hist_pos >= &dsp_m.echo_hist [ECHO_HIST_SIZE])
      dsp_m.echo_hist_pos = dsp_m.echo_hist;

   dsp_m.t_echo_ptr = (dsp_m.t_esa * 0x100 + dsp_m.echo_offset) & 0xFFFF;

   ECHO_READ(0);

   l = (((dsp_m.echo_hist_pos [0 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 0 * 0x10]) >> 6);
   r = (((dsp_m.echo_hist_pos [0 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 0 * 0x10]) >> 6);

   dsp_m.t_echo_in [0] = l;
   dsp_m.t_echo_in [1] = r;
}

static INLINE void dsp_echo_23()
{
   int32_t l, r;

   l = (((dsp_m.echo_hist_pos [1 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 1 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [2 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 2 * 0x10]) >> 6);
   r = (((dsp_m.echo_hist_pos [1 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 1 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [2 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 2 * 0x10]) >> 6);

   dsp_m.t_echo_in [0] += l;
   dsp_m.t_echo_in [1] += r;

   ECHO_READ(1);
}

static INLINE void dsp_echo_24()
{
   int32_t l, r;

   l = (((dsp_m.echo_hist_pos [3 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 3 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [4 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 4 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [5 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 5 * 0x10]) >> 6);
   r = (((dsp_m.echo_hist_pos [3 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 3 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [4 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 4 * 0x10]) >> 6) + (((dsp_m.echo_hist_pos [5 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 5 * 0x10]) >> 6);

   dsp_m.t_echo_in [0] += l;
   dsp_m.t_echo_in [1] += r;
}

static INLINE void dsp_echo_25()
{
   int32_t l = dsp_m.t_echo_in [0] + (((dsp_m.echo_hist_pos [6 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 6 * 0x10]) >> 6);
   int32_t r = dsp_m.t_echo_in [1] + (((dsp_m.echo_hist_pos [6 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 6 * 0x10]) >> 6);

   l = (int16_t) l;
   r = (int16_t) r;

   l += (int16_t) (((dsp_m.echo_hist_pos [7 + 1]) [0] * (int8_t) dsp_m.regs [R_FIR + 7 * 0x10]) >> 6);
   r += (int16_t) (((dsp_m.echo_hist_pos [7 + 1]) [1] * (int8_t) dsp_m.regs [R_FIR + 7 * 0x10]) >> 6);

   if ( (int16_t) l != l )
      l = (l >> 31) ^ 0x7FFF;
   if ( (int16_t) r != r )
      r = (r >> 31) ^ 0x7FFF;

   dsp_m.t_echo_in [0] = l & ~1;
   dsp_m.t_echo_in [1] = r & ~1;
}

#define ECHO_OUTPUT(var, ch) \
{ \
   var = (int16_t) ((dsp_m.t_main_out [ch] * (int8_t) REG(MVOLL + ch * 0x10)) >> 7) + (int16_t) ((dsp_m.t_echo_in [ch] * (int8_t) REG(EVOLL + ch * 0x10)) >> 7); \
   CLAMP16( var ); \
}

static INLINE void dsp_echo_26()
{
   int32_t l, r;

   ECHO_OUTPUT(dsp_m.t_main_out[0], 0 );

   l = dsp_m.t_echo_out [0] + (int16_t) ((dsp_m.t_echo_in [0] * (int8_t) dsp_m.regs [R_EFB]) >> 7);
   r = dsp_m.t_echo_out [1] + (int16_t) ((dsp_m.t_echo_in [1] * (int8_t) dsp_m.regs [R_EFB]) >> 7);

   if ( (int16_t) l != l ) l = (l >> 31) ^ 0x7FFF;
   if ( (int16_t) r != r ) r = (r >> 31) ^ 0x7FFF;

   dsp_m.t_echo_out [0] = l & ~1;
   dsp_m.t_echo_out [1] = r & ~1;
}

static INLINE void dsp_echo_27()
{
   int32_t l, r;
   int16_t *out;

   l = dsp_m.t_main_out [0];
   ECHO_OUTPUT(r, 1);
   dsp_m.t_main_out [0] = 0;
   dsp_m.t_main_out [1] = 0;

   if ( dsp_m.regs [R_FLG] & 0x40 )
   {
      l = 0;
      r = 0;
   }

   out = dsp_m.out;
   out [0] = l;
   out [1] = r;
   out += 2;
   if ( out >= dsp_m.out_end )
   {
      out = dsp_m.extra;
      dsp_m.out_end = &dsp_m.extra [EXTRA_SIZE];
   }
   dsp_m.out = out;
}

#define ECHO_28() dsp_m.t_echo_enabled = dsp_m.regs [R_FLG];

#define ECHO_WRITE(ch) \
   if ( !(dsp_m.t_echo_enabled & 0x20) ) \
   { \
      SET_LE16A( ECHO_PTR( ch ), dsp_m.t_echo_out [ch] ); \
      if ( dsp_m.t_echo_ptr >= 0xffc0 ) \
      { \
         SET_LE16A( &dsp_m.hi_ram [dsp_m.t_echo_ptr + ch * 2 - 0xffc0], dsp_m.t_echo_out [ch] ); \
         if ( dsp_m.rom_enabled ) \
            SET_LE16A( ECHO_PTR( ch ), GET_LE16A( &dsp_m.rom [dsp_m.t_echo_ptr + ch * 2 - 0xffc0] ) ); \
      } \
   } \
   dsp_m.t_echo_out [ch] = 0;

static INLINE void dsp_echo_29()
{
   dsp_m.t_esa = dsp_m.regs [R_ESA];

   if ( !dsp_m.echo_offset )
      dsp_m.echo_length = (dsp_m.regs [R_EDL] & 0x0F) * 0x800;

   dsp_m.echo_offset += 4;
   if ( dsp_m.echo_offset >= dsp_m.echo_length )
      dsp_m.echo_offset = 0;


   ECHO_WRITE(0);

   dsp_m.t_echo_enabled = dsp_m.regs [R_FLG];
}

/* Timing */

/* Execute clock for a particular voice */

/* The most common sequence of clocks uses composite operations
for efficiency. For example, the following are equivalent to the
individual steps on the right:

V(V7_V4_V1,2) -> V(V7,2) V(V4,3) V(V1,5)
V(V8_V5_V2,2) -> V(V8,2) V(V5,3) V(V2,4)
V(V9_V6_V3,2) -> V(V9,2) V(V6,3) V(V3,4) */

/* Voice      0      1      2      3      4      5      6      7 */

/* Runs DSP for specified number of clocks (~1024000 per second). Every 32 clocks
   a pair of samples is be generated. */

static void dsp_run( int32_t clocks_remain )
{
   int32_t phase;
   if (Settings.HardDisableAudio)
	   return;
   phase = dsp_m.phase;
   dsp_m.phase = (phase + clocks_remain) & 31;

   switch ( phase )
   {
loop:
      if ( 0 && !--clocks_remain )
         break;
      case 0:
      dsp_voice_V5( &dsp_m.voices [0] );
      dsp_voice_V2( &dsp_m.voices [1] );
      if ( 1 && !--clocks_remain )
         break;
      case 1:
      dsp_voice_V6( &dsp_m.voices [0] );
      dsp_voice_V3( &dsp_m.voices [1] );
      if ( 2 && !--clocks_remain )
         break;
      case 2:
      dsp_voice_V7_V4_V1( &dsp_m.voices [0] );
      if ( 3 && !--clocks_remain )
         break;
      case 3:
      dsp_voice_V8_V5_V2( &dsp_m.voices [0] );
      if ( 4 && !--clocks_remain )
         break;
      case 4:
      dsp_voice_V9_V6_V3( &dsp_m.voices [0] );
      if ( 5 && !--clocks_remain )
         break;
      case 5:
      dsp_voice_V7_V4_V1( &dsp_m.voices [1] );
      if ( 6 && !--clocks_remain )
         break;
      case 6:
      dsp_voice_V8_V5_V2( &dsp_m.voices [1] );
      if ( 7 && !--clocks_remain )
         break;
      case 7:
      dsp_voice_V9_V6_V3( &dsp_m.voices [1] );
      if ( 8 && !--clocks_remain )
         break;
      case 8:
      dsp_voice_V7_V4_V1( &dsp_m.voices [2] );
      if ( 9 && !--clocks_remain )
         break;
      case 9:
      dsp_voice_V8_V5_V2( &dsp_m.voices [2] );
      if ( 10 && !--clocks_remain )
         break;
      case 10:
      dsp_voice_V9_V6_V3( &dsp_m.voices [2] );
      if ( 11 && !--clocks_remain )
         break;
      case 11:
      dsp_voice_V7_V4_V1( &dsp_m.voices [3] );
      if ( 12 && !--clocks_remain )
         break;
      case 12:
      dsp_voice_V8_V5_V2( &dsp_m.voices [3] );
      if ( 13 && !--clocks_remain )
         break;
      case 13:
      dsp_voice_V9_V6_V3( &dsp_m.voices [3] );
      if ( 14 && !--clocks_remain )
         break;
      case 14:
      dsp_voice_V7_V4_V1( &dsp_m.voices [4] );
      if ( 15 && !--clocks_remain )
         break;
      case 15:
      dsp_voice_V8_V5_V2( &dsp_m.voices [4] );
      if ( 16 && !--clocks_remain )
         break;
      case 16:
      dsp_voice_V9_V6_V3( &dsp_m.voices [4] );
      if ( 17 && !--clocks_remain )
         break;
      case 17:
      dsp_voice_V1( &dsp_m.voices [0] );
      dsp_voice_V7( &dsp_m.voices [5] );
      dsp_voice_V4( &dsp_m.voices [6] );
      if ( 18 && !--clocks_remain )
         break;
      case 18:
      dsp_voice_V8_V5_V2( &dsp_m.voices [5] );
      if ( 19 && !--clocks_remain )
         break;
      case 19:
      dsp_voice_V9_V6_V3( &dsp_m.voices [5] );
      if ( 20 && !--clocks_remain )
         break;
      case 20:
      dsp_voice_V1( &dsp_m.voices [1] );
      dsp_voice_V7( &dsp_m.voices [6] );
      dsp_voice_V4( &dsp_m.voices [7] );
      if ( 21 && !--clocks_remain )
         break;
      case 21:
      dsp_voice_V8( &dsp_m.voices [6] );
      dsp_voice_V5( &dsp_m.voices [7] );
      dsp_voice_V2( &dsp_m.voices [0] );
      if ( 22 && !--clocks_remain )
         break;
      case 22:
      dsp_voice_V3a( &dsp_m.voices [0] );
      dsp_voice_V9( &dsp_m.voices [6] );
      dsp_voice_V6( &dsp_m.voices [7] );
      dsp_echo_22();
      if ( 23 && !--clocks_remain )
         break;
      case 23:
      dsp_voice_V7( &dsp_m.voices [7] );
      dsp_echo_23();
      if ( 24 && !--clocks_remain )
         break;
      case 24:
      dsp_voice_V8( &dsp_m.voices [7] );
      dsp_echo_24();
      if ( 25 && !--clocks_remain )
         break;
      case 25:
      dsp_voice_V3b( &dsp_m.voices [0] );
      dsp_voice_V9( &dsp_m.voices [7] );
      dsp_echo_25();
      if ( 26 && !--clocks_remain )
         break;
      case 26:
      dsp_echo_26();
      if ( 27 && !--clocks_remain )
         break;
      case 27:
      MISC_27();
      dsp_echo_27();
      if ( 28 && !--clocks_remain )
         break;
      case 28:
      MISC_28();
      ECHO_28();
      if ( 29 && !--clocks_remain )
         break;
      case 29:
      MISC_29();
      dsp_echo_29();
      if ( 30 && !--clocks_remain )
         break;
      case 30:
      dsp_misc_30();
      dsp_voice_V3c( &dsp_m.voices [0] );
      ECHO_WRITE(1);
      if ( 31 && !--clocks_remain )
         break;
      case 31:
      dsp_voice_V4( &dsp_m.voices [0] );
      dsp_voice_V1( &dsp_m.voices [2] );

      if ( --clocks_remain )
         goto loop;
   }
}

/* Sets destination for output samples. If out is NULL or out_size is 0,
   doesn't generate any. */

static void dsp_set_output( int16_t * out, int32_t size )
{
   if ( !out )
   {
      out  = dsp_m.extra;
      size = EXTRA_SIZE;
   }
   dsp_m.out_begin = out;
   dsp_m.out       = out;
   dsp_m.out_end   = out + size;
}

/* Setup */

static void dsp_soft_reset_common()
{
   dsp_m.noise              = 0x4000;
   dsp_m.echo_hist_pos      = dsp_m.echo_hist;
   dsp_m.every_other_sample = 1;
   dsp_m.echo_offset        = 0;
   dsp_m.phase              = 0;

   dsp_m.counter = 0;
}

/* Resets DSP to power-on state */

static void dsp_reset()
{
   int32_t i;

   uint8_t const initial_regs [REGISTER_COUNT] =
   {
      0x45,0x8B,0x5A,0x9A,0xE4,0x82,0x1B,0x78,0x00,0x00,0xAA,0x96,0x89,0x0E,0xE0,0x80,
      0x2A,0x49,0x3D,0xBA,0x14,0xA0,0xAC,0xC5,0x00,0x00,0x51,0xBB,0x9C,0x4E,0x7B,0xFF,
      0xF4,0xFD,0x57,0x32,0x37,0xD9,0x42,0x22,0x00,0x00,0x5B,0x3C,0x9F,0x1B,0x87,0x9A,
      0x6F,0x27,0xAF,0x7B,0xE5,0x68,0x0A,0xD9,0x00,0x00,0x9A,0xC5,0x9C,0x4E,0x7B,0xFF,
      0xEA,0x21,0x78,0x4F,0xDD,0xED,0x24,0x14,0x00,0x00,0x77,0xB1,0xD1,0x36,0xC1,0x67,
      0x52,0x57,0x46,0x3D,0x59,0xF4,0x87,0xA4,0x00,0x00,0x7E,0x44,0x00,0x4E,0x7B,0xFF,
      0x75,0xF5,0x06,0x97,0x10,0xC3,0x24,0xBB,0x00,0x00,0x7B,0x7A,0xE0,0x60,0x12,0x0F,
      0xF7,0x74,0x1C,0xE5,0x39,0x3D,0x73,0xC1,0x00,0x00,0x7A,0xB3,0xFF,0x4E,0x7B,0xFF
   };

   /* Resets DSP and uses supplied values to initialize registers */

   for (i = 0; i < REGISTER_COUNT; i++)
      dsp_m.regs[i] = initial_regs[i];

   /* Internal state */
   for ( i = VOICE_COUNT; --i >= 0; )
   {
      dsp_voice_t* v = &dsp_m.voices [i];
      v->brr_offset = 1;
      v->vbit       = 1 << i;
      v->regs       = &dsp_m.regs [i * 0x10];
   }
   dsp_m.new_kon = dsp_m.regs[R_KON];
   dsp_m.t_dir   = dsp_m.regs[R_DIR];
   dsp_m.t_esa   = dsp_m.regs[R_ESA];

   dsp_soft_reset_common();
}

/* Initializes DSP and has it use the 64K RAM provided */

static void dsp_init( void* ram_64k )
{
   dsp_m.ram = (uint8_t*) ram_64k;
   dsp_set_output( 0, 0 );
   dsp_reset();
}

/* Emulates pressing reset switch on SNES */

static void dsp_soft_reset(void)
{
   dsp_m.regs[R_FLG] = 0xE0;
   dsp_soft_reset_common();
}


/* State save/load */

#if !SPC_NO_COPY_STATE_FUNCS

static void spc_copier_copy(spc_state_copy_t * copier, void* state, size_t size )
{
   copier->func(copier->buf, state, size );
}

static int32_t spc_copier_copy_int(spc_state_copy_t * copier, int32_t state, int32_t size )
{
   uint8_t s [2];
   SET_LE16( s, state );
   copier->func(copier->buf, &s, size );
   return GET_LE16( s );
}

static void spc_copier_extra(spc_state_copy_t * copier)
{
   int32_t n = 0;
   n = (uint8_t) spc_copier_copy_int(copier, n, sizeof (uint8_t) );

   if ( n > 0 )
   {
      int8_t temp [64];
      memset( temp, 0, sizeof(temp));
      do
      {
         int32_t size_n = sizeof(temp);
         if ( size_n > n )
            size_n = n;
         n -= size_n;
         copier->func(copier->buf, temp, size_n );
      }
      while ( n );
   }
}

/* Saves/loads exact emulator state */

static void dsp_copy_state( uint8_t ** io, dsp_copy_func_t copy )
{
   int32_t i, j;

   spc_state_copy_t copier;
   copier.func = copy;
   copier.buf = io;

   /* DSP registers */
   spc_copier_copy(&copier, dsp_m.regs, REGISTER_COUNT );

   /* Internal state */

   /* Voices */
   for ( i = 0; i < VOICE_COUNT; i++ )
   {
      dsp_voice_t* v;

      v = &dsp_m.voices [i];

      /* BRR buffer */
      for ( j = 0; j < BRR_BUF_SIZE; j++ )
      {
         int32_t s;

         s = v->buf [j];
         SPC_COPY(  int16_t, s );
         v->buf [j] = v->buf [j + BRR_BUF_SIZE] = s;
      }

      SPC_COPY( uint16_t, v->interp_pos );
      SPC_COPY( uint16_t, v->brr_addr );
      SPC_COPY( uint16_t, v->env );
      SPC_COPY(  int16_t, v->hidden_env );
      SPC_COPY(  uint8_t, v->buf_pos );
      SPC_COPY(  uint8_t, v->brr_offset );
      SPC_COPY(  uint8_t, v->kon_delay );
      {
         int32_t m;

         m = v->env_mode;
         SPC_COPY(  uint8_t, m );
         v->env_mode = m;
      }
      SPC_COPY(  uint8_t, v->t_envx_out );

      spc_copier_extra(&copier);
   }

   /* Echo history */
   for ( i = 0; i < ECHO_HIST_SIZE; i++ )
   {
      int32_t s, s2;

      s = dsp_m.echo_hist_pos [i] [0];
      s2 = dsp_m.echo_hist_pos [i] [1];

      SPC_COPY( int16_t, s );
      dsp_m.echo_hist [i] [0] = s; /* write back at offset 0 */

      SPC_COPY( int16_t, s2 );
      dsp_m.echo_hist [i] [1] = s2; /* write back at offset 0 */
   }
   dsp_m.echo_hist_pos = dsp_m.echo_hist;
   memcpy( &dsp_m.echo_hist [ECHO_HIST_SIZE], dsp_m.echo_hist, ECHO_HIST_SIZE * sizeof dsp_m.echo_hist [0] );

   /* Misc */
   SPC_COPY(  uint8_t, dsp_m.every_other_sample );
   SPC_COPY(  uint8_t, dsp_m.kon );

   SPC_COPY( uint16_t, dsp_m.noise );
   SPC_COPY( uint16_t, dsp_m.counter );
   SPC_COPY( uint16_t, dsp_m.echo_offset );
   SPC_COPY( uint16_t, dsp_m.echo_length );
   SPC_COPY(  uint8_t, dsp_m.phase );

   SPC_COPY(  uint8_t, dsp_m.new_kon );
   SPC_COPY(  uint8_t, dsp_m.endx_buf );
   SPC_COPY(  uint8_t, dsp_m.envx_buf );
   SPC_COPY(  uint8_t, dsp_m.outx_buf );

   SPC_COPY(  uint8_t, dsp_m.t_pmon );
   SPC_COPY(  uint8_t, dsp_m.t_non );
   SPC_COPY(  uint8_t, dsp_m.t_eon );
   SPC_COPY(  uint8_t, dsp_m.t_dir );
   SPC_COPY(  uint8_t, dsp_m.t_koff );

   SPC_COPY( uint16_t, dsp_m.t_brr_next_addr );
   SPC_COPY(  uint8_t, dsp_m.t_adsr0 );
   SPC_COPY(  uint8_t, dsp_m.t_brr_header );
   SPC_COPY(  uint8_t, dsp_m.t_brr_byte );
   SPC_COPY(  uint8_t, dsp_m.t_srcn );
   SPC_COPY(  uint8_t, dsp_m.t_esa );
   SPC_COPY(  uint8_t, dsp_m.t_echo_enabled );

   SPC_COPY(  int16_t, dsp_m.t_main_out [0] );
   SPC_COPY(  int16_t, dsp_m.t_main_out [1] );
   SPC_COPY(  int16_t, dsp_m.t_echo_out [0] );
   SPC_COPY(  int16_t, dsp_m.t_echo_out [1] );
   SPC_COPY(  int16_t, dsp_m.t_echo_in  [0] );
   SPC_COPY(  int16_t, dsp_m.t_echo_in  [1] );

   SPC_COPY( uint16_t, dsp_m.t_dir_addr );
   SPC_COPY( uint16_t, dsp_m.t_pitch );
   SPC_COPY(  int16_t, dsp_m.t_output );
   SPC_COPY( uint16_t, dsp_m.t_echo_ptr );
   SPC_COPY(  uint8_t, dsp_m.t_looped );

   spc_copier_extra(&copier);
}
#endif

/* Core SPC emulation: CPU, timers, SMP registers, memory */

/* snes_spc 0.9.0. http://www.slack.net/~ant/ */

/***********************************************************************************
   SNES SPC
***********************************************************************************/

static spc_state_t m;
static int8_t reg_times [256];
static bool allow_time_overflow;

/* Copyright (C) 2004-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

/* (n ? n : 256) */
#define IF_0_THEN_256( n ) ((uint8_t) ((n) - 1) + 1)

/* Timers */

#define TIMER_DIV( t, n ) ((n) >> t->prescaler)
#define TIMER_MUL( t, n ) ((n) << t->prescaler)

static Timer* spc_run_timer_( Timer* t, int32_t time )
{
   int32_t elapsed;

   elapsed = TIMER_DIV( t, time - t->next_time ) + 1;
   t->next_time += TIMER_MUL( t, elapsed );

   if ( t->enabled )
   {
      int32_t remain, divider, over, n;

      remain = IF_0_THEN_256( t->period - t->divider );
      divider = t->divider + elapsed;
      over = elapsed - remain;
      if ( over >= 0 )
      {
         n = over / t->period;
         t->counter = (t->counter + 1 + n) & 0x0F;
         divider = over - n * t->period;
      }
      t->divider = (uint8_t) divider;
   }
   return t;
}

/* ROM */

void spc_enable_rom( int32_t enable )
{
   if ( m.rom_enabled != enable )
   {
      m.rom_enabled = dsp_m.rom_enabled = enable;
      if ( enable )
         memcpy( m.hi_ram, &m.ram.ram[ROM_ADDR], sizeof m.hi_ram );
      memcpy( &m.ram.ram[ROM_ADDR], (enable ? m.rom : m.hi_ram), ROM_SIZE );
      /* TODO: ROM can still get overwritten when DSP writes to echo buffer */
   }
}


/* DSP */

#define MAX_REG_TIME 29

#define RUN_DSP( time, offset ) \
   int32_t count = (time) - (offset) - m.dsp_time; \
   if ( count >= 0 ) \
   { \
      int32_t clock_count; \
      clock_count = (count & ~(CLOCKS_PER_SAMPLE - 1)) + CLOCKS_PER_SAMPLE; \
      m.dsp_time += clock_count; \
      dsp_run( clock_count ); \
   }

static INLINE void spc_dsp_write( int32_t data, int32_t time )
{
   int32_t addr;
   (void) time;

   /* Writes DSP registers. */
   addr = m.smp_regs[0][R_DSPADDR];
   dsp_m.regs [addr] = (uint8_t) data;
   switch ( addr & 0x0F )
   {
      case V_ENVX:
         dsp_m.envx_buf = (uint8_t) data;
         break;

      case V_OUTX:
         dsp_m.outx_buf = (uint8_t) data;
         break;
      case 0x0C:
         if ( addr == R_KON )
            dsp_m.new_kon = (uint8_t) data;

         if ( addr == R_ENDX ) /* always cleared, regardless of data written */
         {
            dsp_m.endx_buf = 0;
            dsp_m.regs [R_ENDX] = 0;
         }
         break;
   }
}


/* Memory access extras */

/* CPU write */

/* divided into multiple functions to keep rarely-used functionality separate
   so often-used functionality can be optimized better by compiler */

/* If write isn't preceded by read, data has this added to it
   int32_t const no_read_before_write = 0x2000; */

#define NO_READ_BEFORE_WRITE			8192
#define NO_READ_BEFORE_WRITE_DIVIDED_BY_TWO	4096

static void spc_cpu_write_smp_reg_( int32_t data, int32_t time, int32_t addr )
{
   switch ( addr )
   {
      case R_T0TARGET:
      case R_T1TARGET:
      case R_T2TARGET:
         {
            int32_t period;
            Timer *t;

            t = &m.timers [addr - R_T0TARGET];
            period = IF_0_THEN_256( data );

            if ( t->period != period )
            {
               if ( time >= t->next_time )
                  t = spc_run_timer_( t, time );
               t->period = period;
            }
            break;
         }
      case R_T0OUT:
      case R_T1OUT:
      case R_T2OUT:
             if ( data < NO_READ_BEFORE_WRITE_DIVIDED_BY_TWO)
             {
                if ( (time - 1) >= m.timers[addr - R_T0OUT].next_time )
                   spc_run_timer_( &m.timers [addr - R_T0OUT], time - 1 )->counter = 0;
               else
                  m.timers[addr - R_T0OUT].counter = 0;
             }
             break;

             /* Registers that act like RAM */
      case 0x8:
      case 0x9:
             m.smp_regs[1][addr] = (uint8_t) data;
             break;

      case R_TEST:
             break;

      case R_CONTROL:
             {
               int32_t i;
                /* port clears */
                if ( data & 0x10 )
                {
                   m.smp_regs[1][R_CPUIO0] = 0;
                   m.smp_regs[1][R_CPUIO1] = 0;
                }
                if ( data & 0x20 )
                {
                   m.smp_regs[1][R_CPUIO2] = 0;
                   m.smp_regs[1][R_CPUIO3] = 0;
                }

                /* timers */
                {
                   for ( i = 0; i < TIMER_COUNT; i++ )
                   {
                      Timer* t = &m.timers [i];
                      int32_t enabled = data >> i & 1;
                      if ( t->enabled != enabled )
                      {
                         if ( time >= t->next_time )
                            t = spc_run_timer_( t, time );
                         t->enabled = enabled;
                         if ( enabled )
                         {
                            t->divider = 0;
                            t->counter = 0;
                         }
                      }
                   }
                }
                spc_enable_rom( data & 0x80 );
             }
             break;
   }
}

static void spc_cpu_write( int32_t data, uint16_t addr, int32_t time )
{
   int32_t reg;
   /* RAM */
   m.ram.ram[addr] = (uint8_t) data;
   reg = addr - 0xF0;
   if ( reg >= 0 ) /* 64% */
   {
      /* $F0-$FF */
      if ( reg < REG_COUNT ) /* 87% */
      {
         m.smp_regs[0][reg] = (uint8_t) data;

         /* Registers other than $F2 and $F4-$F7
            if ( reg != 2 && reg != 4 && reg != 5 && reg != 6 && reg != 7 )
            TODO: this is a bit on the fragile side */

         if ( ((~0x2F00 << 16) << reg) < 0 ) /* 36% */
         {
            if ( reg == R_DSPDATA ) /* 99% */
            {
               RUN_DSP(time, reg_times [m.smp_regs[0][R_DSPADDR]] );
               if (m.smp_regs[0][R_DSPADDR] <= 0x7F )
                  spc_dsp_write( data, time );
            }
            else
               spc_cpu_write_smp_reg_( data, time, reg);
         }
      }
      /* High mem/address wrap-around */
      else
      {
         reg -= ROM_ADDR - 0xF0;
         if ( reg >= 0 ) /* 1% in IPL ROM area or address wrapped around */
         {
            m.hi_ram [reg] = (uint8_t) data;
            if ( m.rom_enabled )
              m.ram.ram[reg + ROM_ADDR] = m.rom [reg]; /* restore overwritten ROM */
         }
      }
   }
}

/* CPU read */

static int32_t spc_cpu_read( uint16_t addr, int32_t time )
{
   int32_t result, reg;

   /* RAM */
   result = m.ram.ram[addr];
   reg = addr - 0xF0;

   if ( reg >= 0 ) /* 40% */
   {
      reg -= 0x10;
      if ( (uint32_t) reg >= 0xFF00 ) /* 21% */
      {
         reg += 0x10 - R_T0OUT;

         /* Timers */
         if ( (uint32_t) reg < TIMER_COUNT ) /* 90% */
         {
            Timer* t = &m.timers [reg];
            if ( time >= t->next_time )
               t = spc_run_timer_( t, time );
            result = t->counter;
            t->counter = 0;
         }
         /* Other registers */
         else /* 10% */
         {
            int32_t reg_tmp;

            reg_tmp = reg + R_T0OUT;
            result = m.smp_regs[1][reg_tmp];
            reg_tmp -= R_DSPADDR;
            /* DSP addr and data */
            if ( (uint32_t) reg_tmp <= 1 ) /* 4% 0xF2 and 0xF3 */
            {
               result = m.smp_regs[0][R_DSPADDR];
               if ( (uint32_t) reg_tmp == 1 )
               {
                  RUN_DSP( time, reg_times [m.smp_regs[0][R_DSPADDR] & 0x7F] );

                  result = dsp_m.regs[m.smp_regs[0][R_DSPADDR] & 0x7F]; /* 0xF3 */
               }
            }
         }
      }
   }

   return result;
}

/***********************************************************************************
 SPC CPU
***********************************************************************************/

/* Inclusion here allows static memory access functions and better optimization */

/* Timers are by far the most common thing read from dp */

#define CPU_READ_TIMER( time, offset, addr_, out )\
{\
   int32_t adj_time, dp_addr, ti; \
   adj_time = time + offset;\
   dp_addr = addr_;\
   ti = dp_addr - (R_T0OUT + 0xF0);\
   if ( (uint32_t) ti < TIMER_COUNT )\
   {\
      Timer* t = &m.timers [ti];\
      if ( adj_time >= t->next_time )\
      t = spc_run_timer_( t, adj_time );\
      out = t->counter;\
      t->counter = 0;\
   }\
   else\
   {\
      int32_t i, reg; \
      out = ram [dp_addr];\
      i = dp_addr - 0xF0;\
      if ( (uint32_t) i < 0x10 )\
      { \
         reg = i;  \
         out = m.smp_regs[1][reg]; \
         reg -= R_DSPADDR; \
         /* DSP addr and data */ \
         if ( (uint32_t) reg <= 1 ) /* 4% 0xF2 and 0xF3 */ \
         { \
            out = m.smp_regs[0][R_DSPADDR]; \
            if ( (uint32_t) reg == 1 ) \
            { \
               RUN_DSP( adj_time, reg_times [m.smp_regs[0][R_DSPADDR] & 0x7F] ); \
               out = dsp_m.regs[m.smp_regs[0][R_DSPADDR] & 0x7F ]; /* 0xF3 */ \
            } \
         } \
      } \
   }\
}

#define READ_TIMER( time, addr, out )	CPU_READ_TIMER( rel_time, time, (addr), out )
#define SPC_CPU_READ( time, addr )		spc_cpu_read((addr), rel_time + time )
#define SPC_CPU_WRITE( time, addr, data )	spc_cpu_write((data), (addr), rel_time + time )

static uint32_t spc_CPU_mem_bit( uint8_t const* pc, int32_t rel_time )
{
   uint32_t addr, t;

   addr = GET_LE16( pc );
   t = SPC_CPU_READ( 0, addr & 0x1FFF ) >> (addr >> 13);
   return t << 8 & 0x100;
}

#define DP_ADDR( addr )                     (dp + (addr))

#define READ_DP_TIMER(  time, addr, out )   CPU_READ_TIMER( rel_time, time, DP_ADDR( addr ), out )
#define READ_DP(  time, addr )              SPC_CPU_READ( time, DP_ADDR( addr ) )
#define WRITE_DP( time, addr, data )        SPC_CPU_WRITE( time, DP_ADDR( addr ), data )

#define READ_PROG16( addr )                 GET_LE16( ram + (addr) )

#define SET_PC( n )     (pc = ram + (n))
#define GET_PC()        (pc - ram)
#define READ_PC( pc )   (*(pc))

#define SET_SP( v )     (sp = ram + 0x101 + (v))
#define GET_SP()        (sp - 0x101 - ram)

#define PUSH16( v )     (sp -= 2, SET_LE16( sp, v ))
#define PUSH( v )       (void) (*--sp = (uint8_t) (v))
#define POP( out )      (void) ((out) = *sp++)

#define MEM_BIT( rel ) spc_CPU_mem_bit( pc, rel_time + rel )

#define GET_PSW( out )\
{\
   out = psw & ~(N80 | P20 | Z02 | C01);\
   out |= c  >> 8 & C01;\
   out |= dp >> 3 & P20;\
   out |= ((nz >> 4) | nz) & N80;\
   if ( !(uint8_t) nz ) out |= Z02;\
}

#define SET_PSW( in )\
{\
   psw = in;\
   c   = in << 8;\
   dp  = in << 3 & 0x100;\
   nz  = (in << 4 & 0x800) | (~in & Z02);\
}

static uint8_t* spc_run_until_( int32_t end_time )
{
   int32_t dp, nz, c, psw, a, x, y;
   uint8_t *ram, *pc, *sp;
   int32_t rel_time = m.spc_time - end_time;
   m.spc_time = end_time;
   m.dsp_time += rel_time;
   m.timers [0].next_time += rel_time;
   m.timers [1].next_time += rel_time;
   m.timers [2].next_time += rel_time;
   ram = m.ram.ram;
   a = m.cpu_regs.a;
   x = m.cpu_regs.x;
   y = m.cpu_regs.y;

   SET_PC( m.cpu_regs.pc );
   SET_SP( m.cpu_regs.sp );
   SET_PSW( m.cpu_regs.psw );

   goto loop;


   /* Main loop */

cbranch_taken_loop:
   pc += *(int8_t const*) pc;
inc_pc_loop:
   pc++;
loop:
   {
      uint32_t opcode, data;

      opcode = *pc;

      if (allow_time_overflow && rel_time >= 0 )
         goto stop;
      if ( (rel_time += m.cycle_table [opcode]) > 0 && !allow_time_overflow)
         goto out_of_time;

      /* TODO: if PC is at end of memory, this will get wrong operand (very obscure) */
      data = *++pc;
      switch ( opcode )
      {

         /* Common instructions */

#define BRANCH( cond )\
         {\
            pc++;\
            pc += (int8_t) data;\
            if ( cond )\
            goto loop;\
            pc -= (int8_t) data;\
            rel_time -= 2;\
            goto loop;\
         }

         case 0xF0: /* BEQ */
            BRANCH( !(uint8_t) nz ) /* 89% taken */

         case 0xD0: /* BNE */
               BRANCH( (uint8_t) nz )

         case 0x3F:
               { /* CALL */
                  int32_t old_addr;
                  old_addr = GET_PC() + 2;
                  SET_PC( GET_LE16( pc ) );
                  PUSH16( old_addr );
                  goto loop;
               }
         case 0x6F: /* RET */
               SET_PC( GET_LE16( sp ) );
               sp += 2;
               goto loop;

         case 0xE4: /* MOV a,dp */
               ++pc;
               /* 80% from timer */
               READ_DP_TIMER( 0, data, a = nz );
               goto loop;

         case 0xFA:{ /* MOV dp,dp */
                 int32_t temp;
                 READ_DP_TIMER( -2, data, temp );
                 data = temp + NO_READ_BEFORE_WRITE ;
              }
              /* fall through */
         case 0x8F:
              { /* MOV dp,#imm */
                 int32_t i, temp;
                 temp = READ_PC( pc + 1 );
                 pc += 2;

                 i = dp + temp;
                 ram [i] = (uint8_t) data;
                 i -= 0xF0;
                 if ( (uint32_t) i < 0x10 ) /* 76% */
                 {
                    m.smp_regs[0][i] = (uint8_t) data;

                    /* Registers other than $F2 and $F4-$F7 */
                    if ( ((~0x2F00 << 16) << i) < 0 ) /* 12% */
                    {
                       if ( i == R_DSPDATA ) /* 99% */
                       {
                          RUN_DSP(rel_time, reg_times [m.smp_regs[0][R_DSPADDR]] );
                          if (m.smp_regs[0][R_DSPADDR] <= 0x7F )
                             spc_dsp_write( data, rel_time );
                       }
                       else
                          spc_cpu_write_smp_reg_( data, rel_time, i);
                    }
                 }
                 goto loop;
              }

         case 0xC4: /* MOV dp,a */
              ++pc;
              {
                 int32_t i;
                 i = dp + data;
                 ram [i] = (uint8_t) a;
                 i -= 0xF0;
                 if ( (uint32_t) i < 0x10 ) /* 39% */
                 {
                    uint32_t sel;
                    sel = i - 2;
                    m.smp_regs[0][i] = (uint8_t) a;

                    if ( sel == 1 ) /* 51% $F3 */
                    {
                       RUN_DSP(rel_time, reg_times [m.smp_regs[0][R_DSPADDR]] );
                       if (m.smp_regs[0][R_DSPADDR] <= 0x7F )
                          spc_dsp_write( a, rel_time );
                    }
                    else if ( sel > 1 ) /* 1% not $F2 or $F3 */
                       spc_cpu_write_smp_reg_( a, rel_time, i );
                 }
              }
              goto loop;

#define CASE( n )   case n:

              /* Define common address modes based on opcode for immediate mode. Execution
                 ends with data set to the address of the operand. */
#define ADDR_MODES_( op )\
              CASE( op - 0x02 ) /* (X) */\
              data = x + dp;\
              pc--;\
              goto end_##op;\
              CASE( op + 0x0F ) /* (dp)+Y */\
              data = READ_PROG16( data + dp ) + y;\
              goto end_##op;\
              CASE( op - 0x01 ) /* (dp+X) */\
              data = READ_PROG16( ((uint8_t) (data + x)) + dp );\
              goto end_##op;\
              CASE( op + 0x0E ) /* abs+Y */\
              data += y;\
              goto abs_##op;\
              CASE( op + 0x0D ) /* abs+X */\
              data += x;\
              CASE( op - 0x03 ) /* abs */\
              abs_##op:\
              data += 0x100 * READ_PC( ++pc );\
              goto end_##op;\
              CASE( op + 0x0C ) /* dp+X */\
              data = (uint8_t) (data + x);

#define ADDR_MODES_NO_DP( op )\
              ADDR_MODES_( op )\
              data += dp;\
              end_##op:

#define ADDR_MODES( op )\
              ADDR_MODES_( op )\
              CASE( op - 0x04 ) /* dp */\
              data += dp;\
              end_##op:

              /* 1. 8-bit Data Transmission Commands. Group I */

              ADDR_MODES_NO_DP( 0xE8 ) /* MOV A,addr */
                 a = nz = SPC_CPU_READ( 0, data );
              goto inc_pc_loop;

         case 0xBF:
              {
                 /* MOV A,(X)+ */
                 int32_t temp;
                 temp = x + dp;
                 x = (uint8_t) (x + 1);
                 a = nz = SPC_CPU_READ( -1, temp );
                 goto loop;
              }

         case 0xE8: /* MOV A,imm */
              a  = data;
              nz = data;
              goto inc_pc_loop;

         case 0xF9: /* MOV X,dp+Y */
              data = (uint8_t) (data + y);
         case 0xF8: /* MOV X,dp */
              READ_DP_TIMER( 0, data, x = nz );
              goto inc_pc_loop;

         case 0xE9: /* MOV X,abs */
              data = GET_LE16( pc );
              ++pc;
              data = SPC_CPU_READ( 0, data );
         case 0xCD: /* MOV X,imm */
              x  = data;
              nz = data;
              goto inc_pc_loop;

         case 0xFB: /* MOV Y,dp+X */
              data = (uint8_t) (data + x);
         case 0xEB: /* MOV Y,dp */
              /* 70% from timer */
              pc++;
              READ_DP_TIMER( 0, data, y = nz );
              goto loop;

         case 0xEC:
              { /* MOV Y,abs */
                 int32_t temp;
                 temp = GET_LE16( pc );
                 pc += 2;
                 READ_TIMER( 0, temp, y = nz );
                 goto loop;
              }

         case 0x8D: /* MOV Y,imm */
              y  = data;
              nz = data;
              goto inc_pc_loop;

              /* 2. 8-BIT DATA TRANSMISSION COMMANDS, GROUP 2 */

              ADDR_MODES_NO_DP( 0xC8 ) /* MOV addr,A */
                 SPC_CPU_WRITE( 0, data, a );
              goto inc_pc_loop;

              {
                 int32_t temp;
                 case 0xCC: /* MOV abs,Y */
                 temp = y;
                 goto mov_abs_temp;
                 case 0xC9: /* MOV abs,X */
                 temp = x;
mov_abs_temp:
                 SPC_CPU_WRITE( 0, GET_LE16( pc ), temp );
                 pc += 2;
                 goto loop;
              }

         case 0xD9: /* MOV dp+Y,X */
              data = (uint8_t) (data + y);
         case 0xD8: /* MOV dp,X */
              SPC_CPU_WRITE( 0, data + dp, x );
              goto inc_pc_loop;

         case 0xDB: /* MOV dp+X,Y */
              data = (uint8_t) (data + x);
         case 0xCB: /* MOV dp,Y */
              SPC_CPU_WRITE( 0, data + dp, y );
              goto inc_pc_loop;

              /* 3. 8-BIT DATA TRANSMISSION COMMANDS, GROUP 3. */

         case 0x7D: /* MOV A,X */
              a  = x;
              nz = x;
              goto loop;

         case 0xDD: /* MOV A,Y */
              a  = y;
              nz = y;
              goto loop;

         case 0x5D: /* MOV X,A */
              x  = a;
              nz = a;
              goto loop;

         case 0xFD: /* MOV Y,A */
              y  = a;
              nz = a;
              goto loop;

         case 0x9D: /* MOV X,SP */
              x = nz = GET_SP();
              goto loop;

         case 0xBD: /* MOV SP,X */
              SET_SP( x );
              goto loop;

         case 0xAF: /* MOV (X)+,A */
              WRITE_DP( 0, x, a + NO_READ_BEFORE_WRITE  );
              x++;
              goto loop;

              /* 5. 8-BIT LOGIC OPERATION COMMANDS */

#define LOGICAL_OP( op, func )\
              ADDR_MODES( op ) /* addr */\
              data = SPC_CPU_READ( 0, data );\
         case op: /* imm */\
                 nz = a func##= data;\
              goto inc_pc_loop;\
              {   uint32_t addr;\
                 case op + 0x11: /* X,Y */\
                           data = READ_DP( -2, y );\
                 addr = x + dp;\
                 goto addr_##op;\
                 case op + 0x01: /* dp,dp */\
                             data = READ_DP( -3, data );\
                 case op + 0x10:{/*dp,imm*/\
                         uint8_t const* addr2 = pc + 1;\
                         pc += 2;\
                         addr = READ_PC( addr2 ) + dp;\
                      }\
                 addr_##op:\
                 nz = data func SPC_CPU_READ( -1, addr );\
                 SPC_CPU_WRITE( 0, addr, nz );\
                 goto loop;\
              }

              LOGICAL_OP( 0x28, & ); /* AND */

              LOGICAL_OP( 0x08, | ); /* OR */

              LOGICAL_OP( 0x48, ^ ); /* EOR */

              /* 4. 8-BIT ARITHMETIC OPERATION COMMANDS */

              ADDR_MODES( 0x68 ) /* CMP addr */
                 data = SPC_CPU_READ( 0, data );
         case 0x68: /* CMP imm */
              nz = a - data;
              c = ~nz;
              nz &= 0xFF;
              goto inc_pc_loop;

         case 0x79: /* CMP (X),(Y) */
              data = READ_DP( -2, y );
              nz = READ_DP( -1, x ) - data;
              c = ~nz;
              nz &= 0xFF;
              goto loop;

         case 0x69: /* CMP dp,dp */
              data = READ_DP( -3, data );
         case 0x78: /* CMP dp,imm */
              nz = READ_DP( -1, READ_PC( ++pc ) ) - data;
              c = ~nz;
              nz &= 0xFF;
              goto inc_pc_loop;

         case 0x3E: /* CMP X,dp */
              data += dp;
              goto cmp_x_addr;
         case 0x1E: /* CMP X,abs */
              data = GET_LE16( pc );
              pc++;
cmp_x_addr:
              data = SPC_CPU_READ( 0, data );
         case 0xC8: /* CMP X,imm */
              nz = x - data;
              c = ~nz;
              nz &= 0xFF;
              goto inc_pc_loop;

         case 0x7E: /* CMP Y,dp */
              data += dp;
              goto cmp_y_addr;
         case 0x5E: /* CMP Y,abs */
              data = GET_LE16( pc );
              pc++;
cmp_y_addr:
              data = SPC_CPU_READ( 0, data );
         case 0xAD: /* CMP Y,imm */
              nz = y - data;
              c = ~nz;
              nz &= 0xFF;
              goto inc_pc_loop;

              {
                 int32_t addr;
                 case 0xB9: /* SBC (x),(y) */
                 case 0x99: /* ADC (x),(y) */
                 pc--; /* compensate for inc later */
                 data = READ_DP( -2, y );
                 addr = x + dp;
                 goto adc_addr;
                 case 0xA9: /* SBC dp,dp */
                 case 0x89: /* ADC dp,dp */
                 data = READ_DP( -3, data );
                 case 0xB8: /* SBC dp,imm */
                 case 0x98: /* ADC dp,imm */
                 addr = READ_PC( ++pc ) + dp;
adc_addr:
                 nz = SPC_CPU_READ( -1, addr );
                 goto adc_data;

                 /* catch ADC and SBC together, then decode later based on operand */
#undef CASE
#define CASE( n ) case n: case (n) + 0x20:
                 ADDR_MODES( 0x88 ) /* ADC/SBC addr */
                    data = SPC_CPU_READ( 0, data );
                 case 0xA8: /* SBC imm */
                 case 0x88: /* ADC imm */
                 addr = -1; /* A */
                 nz = a;
adc_data: {
        int32_t flags;
        if ( opcode >= 0xA0 ) /* SBC */
           data ^= 0xFF;

        flags = data ^ nz;
        nz += data + (c >> 8 & 1);
        flags ^= nz;

        psw = (psw & ~(V40 | H08)) |
           (flags >> 1 & H08) |
           ((flags + 0x80) >> 2 & V40);
        c = nz;
        if ( addr < 0 )
        {
           a = (uint8_t) nz;
           goto inc_pc_loop;
        }
        SPC_CPU_WRITE( 0, addr, /*(uint8_t)*/ nz );
        goto inc_pc_loop;
     }

              }

              /* 6. ADDITION & SUBTRACTION COMMANDS */

#define INC_DEC_REG( reg, op )\
              nz  = reg op;\
              reg = (uint8_t) nz;\
              goto loop;

         case 0xBC: INC_DEC_REG( a, + 1 ) /* INC A */
         case 0x3D: INC_DEC_REG( x, + 1 ) /* INC X */
         case 0xFC: INC_DEC_REG( y, + 1 ) /* INC Y */

         case 0x9C: INC_DEC_REG( a, - 1 ) /* DEC A */
         case 0x1D: INC_DEC_REG( x, - 1 ) /* DEC X */
         case 0xDC: INC_DEC_REG( y, - 1 ) /* DEC Y */

         case 0x9B: /* DEC dp+X */
         case 0xBB: /* INC dp+X */
               data = (uint8_t) (data + x);
         case 0x8B: /* DEC dp */
         case 0xAB: /* INC dp */
               data += dp;
               goto inc_abs;
         case 0x8C: /* DEC abs */
         case 0xAC: /* INC abs */
               data = GET_LE16( pc );
               pc++;
inc_abs:
               nz = (opcode >> 4 & 2) - 1;
               nz += SPC_CPU_READ( -1, data );
               SPC_CPU_WRITE( 0, data, /*(uint8_t)*/ nz );
               goto inc_pc_loop;

               /* 7. SHIFT, ROTATION COMMANDS */

         case 0x5C: /* LSR A */
               c = 0;
         case 0x7C:{ /* ROR A */
                 nz = (c >> 1 & 0x80) | (a >> 1);
                 c = a << 8;
                 a = nz;
                 goto loop;
              }

         case 0x1C: /* ASL A */
              c = 0;
         case 0x3C:
              {/* ROL A */
                 int32_t temp;
                 temp = c >> 8 & 1;
                 c = a << 1;
                 nz = c | temp;
                 a = (uint8_t) nz;
                 goto loop;
              }

         case 0x0B: /* ASL dp */
              c = 0;
              data += dp;
              goto rol_mem;
         case 0x1B: /* ASL dp+X */
              c = 0;
         case 0x3B: /* ROL dp+X */
              data = (uint8_t) (data + x);
         case 0x2B: /* ROL dp */
              data += dp;
              goto rol_mem;
         case 0x0C: /* ASL abs */
              c = 0;
         case 0x2C: /* ROL abs */
              data = GET_LE16( pc );
              pc++;
rol_mem:
              nz = c >> 8 & 1;
              nz |= (c = SPC_CPU_READ( -1, data ) << 1);
              SPC_CPU_WRITE( 0, data, /*(uint8_t)*/ nz );
              goto inc_pc_loop;

         case 0x4B: /* LSR dp */
              c = 0;
              data += dp;
              goto ror_mem;
         case 0x5B: /* LSR dp+X */
              c = 0;
         case 0x7B: /* ROR dp+X */
              data = (uint8_t) (data + x);
         case 0x6B: /* ROR dp */
              data += dp;
              goto ror_mem;
         case 0x4C: /* LSR abs */
              c = 0;
         case 0x6C: /* ROR abs */
              data = GET_LE16( pc );
              pc++;
ror_mem: {
       int32_t temp = SPC_CPU_READ( -1, data );
       nz = (c >> 1 & 0x80) | (temp >> 1);
       c = temp << 8;
       SPC_CPU_WRITE( 0, data, nz );
       goto inc_pc_loop;
    }

         case 0x9F: /* XCN */
    nz = a = (a >> 4) | (uint8_t) (a << 4);
    goto loop;

    /* 8. 16-BIT TRANSMISION COMMANDS */

         case 0xBA: /* MOVW YA,dp */
    a = READ_DP( -2, data );
    nz = (a & 0x7F) | (a >> 1);
    y = READ_DP( 0, (uint8_t) (data + 1) );
    nz |= y;
    goto inc_pc_loop;

         case 0xDA: /* MOVW dp,YA */
    WRITE_DP( -1, data, a );
    WRITE_DP( 0, (uint8_t) (data + 1), y + NO_READ_BEFORE_WRITE  );
    goto inc_pc_loop;

    /* 9. 16-BIT OPERATION COMMANDS */

         case 0x3A: /* INCW dp */
         case 0x1A:{/* DECW dp */
                 int32_t temp;
                 /* low byte */
                 data += dp;
                 temp = SPC_CPU_READ( -3, data );
                 temp += (opcode >> 4 & 2) - 1; /* +1 for INCW, -1 for DECW */
                 nz = ((temp >> 1) | temp) & 0x7F;
                 SPC_CPU_WRITE( -2, data, /*(uint8_t)*/ temp );

                 /* high byte */
                 data = (uint8_t) (data + 1) + dp;
                 temp = (uint8_t) ((temp >> 8) + SPC_CPU_READ( -1, data ));
                 nz |= temp;
                 SPC_CPU_WRITE( 0, data, temp );

                 goto inc_pc_loop;
              }

         case 0x7A: /* ADDW YA,dp */
         case 0x9A:
              {/* SUBW YA,dp */
                 int32_t lo, hi, result, flags;
                 lo = READ_DP( -2, data );
                 hi = READ_DP( 0, (uint8_t) (data + 1) );

                 if ( opcode == 0x9A ) /* SUBW */
                 {
                    lo = (lo ^ 0xFF) + 1;
                    hi ^= 0xFF;
                 }

                 lo += a;
                 result = y + hi + (lo >> 8);
                 flags = hi ^ y ^ result;

                 psw = (psw & ~(V40 | H08)) |
                    (flags >> 1 & H08) |
                    ((flags + 0x80) >> 2 & V40);
                 c = result;
                 a = (uint8_t) lo;
                 result = (uint8_t) result;
                 y = result;
                 nz = (((lo >> 1) | lo) & 0x7F) | result;

                 goto inc_pc_loop;
              }

         case 0x5A:
              { /* CMPW YA,dp */
                 int32_t temp;
                 temp = a - READ_DP( -1, data );
                 nz = ((temp >> 1) | temp) & 0x7F;
                 temp = y + (temp >> 8);
                 temp -= READ_DP( 0, (uint8_t) (data + 1) );
                 nz |= temp;
                 c  = ~temp;
                 nz &= 0xFF;
                 goto inc_pc_loop;
              }

               /* 10. MULTIPLICATION & DIVISON COMMANDS */

         case 0xCF: { /* MUL YA */
                  uint32_t temp = y * a;
                  a = (uint8_t) temp;
                  nz = ((temp >> 1) | temp) & 0x7F;
                  y = temp >> 8;
                  nz |= y;
                  goto loop;
               }

         case 0x9E: /* DIV YA,X */
               {
                  uint32_t ya = y * 0x100 + a;

                  psw &= ~(H08 | V40);

                  if ( y >= x )
                     psw |= V40;

                  if ( (y & 15) >= (x & 15) )
                     psw |= H08;

                  if ( y < x * 2 )
                  {
                     a = ya / x;
                     y = ya - a * x;
                  }
                  else
                  {
                     a = 255 - (ya - x * 0x200) / (256 - x);
                     y = x   + (ya - x * 0x200) % (256 - x);
                  }

                  nz = (uint8_t) a;
                  a = (uint8_t) a;

                  goto loop;
               }

               /* 11. DECIMAL COMPENSATION COMMANDS */

         case 0xDF: /* DAA */
               if ( a > 0x99 || c & 0x100 )
               {
                  a += 0x60;
                  c = 0x100;
               }

               if ( (a & 0x0F) > 9 || psw & H08 )
                  a += 0x06;

               nz = a;
               a = (uint8_t) a;
               goto loop;

         case 0xBE: /* DAS */
               if ( a > 0x99 || !(c & 0x100) )
               {
                  a -= 0x60;
                  c = 0;
               }

               if ( (a & 0x0F) > 9 || !(psw & H08) )
                  a -= 0x06;

               nz = a;
               a = (uint8_t) a;
               goto loop;

               /* 12. BRANCHING COMMANDS */

         case 0x2F: /* BRA rel */
               pc += (int8_t) data;
               goto inc_pc_loop;

         case 0x30: /* BMI */
               BRANCH( (nz & NZ_NEG_MASK) )

         case 0x10: /* BPL */
                  BRANCH( !(nz & NZ_NEG_MASK) )

         case 0xB0: /* BCS */
                  BRANCH( c & 0x100 )

         case 0x90: /* BCC */
                  BRANCH( !(c & 0x100) )

         case 0x70: /* BVS */
                  BRANCH( psw & V40 )

         case 0x50: /* BVC */
                  BRANCH( !(psw & V40) )

#define CBRANCH( cond )\
                  {\
                     pc++;\
                     if ( cond )\
                        goto cbranch_taken_loop;\
                     rel_time -= 2;\
                     goto inc_pc_loop;\
                  }

         case 0x03: /* BBS dp.bit,rel */
         case 0x23:
         case 0x43:
         case 0x63:
         case 0x83:
         case 0xA3:
         case 0xC3:
         case 0xE3:
                  CBRANCH( READ_DP( -4, data ) >> (opcode >> 5) & 1 )

         case 0x13: /* BBC dp.bit,rel */
         case 0x33:
         case 0x53:
         case 0x73:
         case 0x93:
         case 0xB3:
         case 0xD3:
         case 0xF3:
               CBRANCH( !(READ_DP( -4, data ) >> (opcode >> 5) & 1) )

         case 0xDE: /* CBNE dp+X,rel */
                                       data = (uint8_t) (data + x);
                                       /* fall through */
         case 0x2E:{ /* CBNE dp,rel */
                 int32_t temp;
                 /* 61% from timer */
                 READ_DP_TIMER( -4, data, temp );
                 CBRANCH( temp != a )
              }

         case 0x6E: { /* DBNZ dp,rel */
                  uint32_t temp = READ_DP( -4, data ) - 1;
                  WRITE_DP( -3, (uint8_t) data, /*(uint8_t)*/ temp + NO_READ_BEFORE_WRITE  );
                  CBRANCH( temp )
               }

         case 0xFE: /* DBNZ Y,rel */
               y = (uint8_t) (y - 1);
               BRANCH( y )

         case 0x1F: /* JMP [abs+X] */
                  SET_PC( GET_LE16( pc ) + x );
                  /* fall through */
         case 0x5F: /* JMP abs */
                  SET_PC( GET_LE16( pc ) );
                  goto loop;

                  /* 13. SUB-ROUTINE CALL RETURN COMMANDS */

         case 0x0F:{/* BRK */
                 int32_t temp;
                 int32_t ret_addr = GET_PC();
                 SET_PC( READ_PROG16( 0xFFDE ) ); /* vector address verified */
                 PUSH16( ret_addr );
                 GET_PSW( temp );
                 psw = (psw | B10) & ~I04;
                 PUSH( temp );
                 goto loop;
              }

         case 0x4F:{/* PCALL offset */
                 int32_t ret_addr = GET_PC() + 1;
                 SET_PC( 0xFF00 | data );
                 PUSH16( ret_addr );
                 goto loop;
              }

         case 0x01: /* TCALL n */
         case 0x11:
         case 0x21:
         case 0x31:
         case 0x41:
         case 0x51:
         case 0x61:
         case 0x71:
         case 0x81:
         case 0x91:
         case 0xA1:
         case 0xB1:
         case 0xC1:
         case 0xD1:
         case 0xE1:
         case 0xF1: {
                  int32_t ret_addr = GET_PC();
                  SET_PC( READ_PROG16( 0xFFDE - (opcode >> 3) ) );
                  PUSH16( ret_addr );
                  goto loop;
               }

               /* 14. STACK OPERATION COMMANDS */

               {
                  int32_t temp;
                  case 0x7F: /* RET1 */
                  temp = *sp;
                  SET_PC( GET_LE16( sp + 1 ) );
                  sp += 3;
                  goto set_psw;
                  case 0x8E: /* POP PSW */
                  POP( temp );
set_psw:
                  SET_PSW( temp );
                  goto loop;
               }

         case 0x0D: { /* PUSH PSW */
                  int32_t temp;
                  GET_PSW( temp );
                  PUSH( temp );
                  goto loop;
               }

         case 0x2D: /* PUSH A */
               PUSH( a );
               goto loop;

         case 0x4D: /* PUSH X */
               PUSH( x );
               goto loop;

         case 0x6D: /* PUSH Y */
               PUSH( y );
               goto loop;

         case 0xAE: /* POP A */
               POP( a );
               goto loop;

         case 0xCE: /* POP X */
               POP( x );
               goto loop;

         case 0xEE: /* POP Y */
               POP( y );
               goto loop;

               /* 15. BIT OPERATION COMMANDS */

         case 0x02: /* SET1 */
         case 0x22:
         case 0x42:
         case 0x62:
         case 0x82:
         case 0xA2:
         case 0xC2:
         case 0xE2:
         case 0x12: /* CLR1 */
         case 0x32:
         case 0x52:
         case 0x72:
         case 0x92:
         case 0xB2:
         case 0xD2:
         case 0xF2:
               {
                  int32_t bit, mask;
                  bit = 1 << (opcode >> 5);
                  mask = ~bit;
                  if ( opcode & 0x10 )
                     bit = 0;
                  data += dp;
                  SPC_CPU_WRITE( 0, data, (SPC_CPU_READ( -1, data ) & mask) | bit );
                  goto inc_pc_loop;
               }

         case 0x0E: /* TSET1 abs */
         case 0x4E: /* TCLR1 abs */
               data = GET_LE16( pc );
               pc += 2;
               {
                  uint32_t temp = SPC_CPU_READ( -2, data );
                  nz = (uint8_t) (a - temp);
                  temp &= ~a;
                  if ( opcode == 0x0E )
                     temp |= a;
                  SPC_CPU_WRITE( 0, data, temp );
               }
               goto loop;

         case 0x4A: /* AND1 C,mem.bit */
               c &= MEM_BIT( 0 );
               pc += 2;
               goto loop;

         case 0x6A: /* AND1 C,/mem.bit */
               c &= ~MEM_BIT( 0 );
               pc += 2;
               goto loop;

         case 0x0A: /* OR1 C,mem.bit */
               c |= MEM_BIT( -1 );
               pc += 2;
               goto loop;

         case 0x2A: /* OR1 C,/mem.bit */
               c |= ~MEM_BIT( -1 );
               pc += 2;
               goto loop;

         case 0x8A: /* EOR1 C,mem.bit */
               c ^= MEM_BIT( -1 );
               pc += 2;
               goto loop;

         case 0xEA: /* NOT1 mem.bit */
               data = GET_LE16( pc );
               pc += 2;
               {
                  uint32_t temp = SPC_CPU_READ( -1, data & 0x1FFF );
                  temp ^= 1 << (data >> 13);
                  SPC_CPU_WRITE( 0, data & 0x1FFF, temp );
               }
               goto loop;

         case 0xCA: /* MOV1 mem.bit,C */
               data = GET_LE16( pc );
               pc += 2;
               {
                  uint32_t temp, bit;
                  temp = SPC_CPU_READ( -2, data & 0x1FFF );
                  bit = data >> 13;
                  temp = (temp & ~(1 << bit)) | ((c >> 8 & 1) << bit);
                  SPC_CPU_WRITE( 0, data & 0x1FFF, temp + NO_READ_BEFORE_WRITE  );
               }
               goto loop;

         case 0xAA: /* MOV1 C,mem.bit */
               c = MEM_BIT( 0 );
               pc += 2;
               goto loop;

               /* 16. PROGRAM PSW FLAG OPERATION COMMANDS */

         case 0x60: /* CLRC */
               c = 0;
               goto loop;

         case 0x80: /* SETC */
               c = ~0;
               goto loop;

         case 0xED: /* NOTC */
               c ^= 0x100;
               goto loop;

         case 0xE0: /* CLRV */
               psw &= ~(V40 | H08);
               goto loop;

         case 0x20: /* CLRP */
               dp = 0;
               goto loop;

         case 0x40: /* SETP */
               dp = 0x100;
               goto loop;

         case 0xA0: /* EI */
               psw |= I04;
               goto loop;

         case 0xC0: /* DI */
               psw &= ~I04;
               goto loop;

               /* 17. OTHER COMMANDS */

         case 0x00: /* NOP */
               goto loop;

         case 0xFF:
               { /* STOP */
                  /* handle PC wrap-around */
                  uint32_t addr = GET_PC() - 1;
                  if ( addr >= 0x10000 )
                  {
                     addr &= 0xFFFF;
                     SET_PC( addr );
                     goto loop;
                  }
               }
              /* fall through */
         case 0xEF: /* SLEEP */
              --pc;
              rel_time = 0;
              goto stop;
      } /* switch */
   }
out_of_time:
   rel_time -= m.cycle_table [*pc]; /* undo partial execution of opcode */
stop:

   /* Uncache registers */
   m.cpu_regs.pc = (uint16_t) GET_PC();
   m.cpu_regs.sp = ( uint8_t) GET_SP();
   m.cpu_regs.a  = ( uint8_t) a;
   m.cpu_regs.x  = ( uint8_t) x;
   m.cpu_regs.y  = ( uint8_t) y;
   {
      int32_t temp;
      GET_PSW( temp );
      m.cpu_regs.psw = (uint8_t) temp;
   }
   m.spc_time += rel_time;
   m.dsp_time -= rel_time;
   m.timers [0].next_time -= rel_time;
   m.timers [1].next_time -= rel_time;
   m.timers [2].next_time -= rel_time;
   return &m.smp_regs[0][R_CPUIO0];
}

/* Runs SPC to end_time and starts a new time frame at 0 */

static void spc_end_frame( int32_t end_time )
{
   int32_t i;
   /* Catch CPU up to as close to end as possible. If final instruction
      would exceed end, does NOT execute it and leaves m.spc_time < end. */

   if ( end_time > m.spc_time )
      spc_run_until_( end_time );

   m.spc_time     -= end_time;
   m.extra_clocks += end_time;

   /* Catch timers up to CPU */
   for ( i = 0; i < TIMER_COUNT; i++ )
   {
      if ( 0 >= m.timers[i].next_time )
         spc_run_timer_( &m.timers [i], 0 );
   }

   /* Catch DSP up to CPU */
   if ( m.dsp_time < 0 )
   {
      RUN_DSP( 0, MAX_REG_TIME );
   }

   /* Save any extra samples beyond what should be generated */
   if ( m.buf_begin )
   {
      int16_t *main_end, *dsp_end, *out, *in;
      /* Get end pointers */
      main_end = m.buf_end;	/* end of data written to buf */
      dsp_end  = dsp_m.out;	/* end of data written to dsp.extra() */
      if ( m.buf_begin <= dsp_end && dsp_end <= main_end )
      {
         main_end = dsp_end;
         dsp_end  = dsp_m.extra; /* nothing in DSP's extra */
      }

      /* Copy any extra samples at these ends into extra_buf */
      out = m.extra_buf;
      for ( in = m.buf_begin + SPC_SAMPLE_COUNT(); in < main_end; in++ )
         *out++ = *in;
      for ( in = dsp_m.extra; in < dsp_end ; in++ )
         *out++ = *in;

      m.extra_pos = out;
   }
}

/* Support SNES_MEMORY_APURAM */

uint8_t * spc_apuram()
{
   return m.ram.ram;
}

/* Init */

static void spc_reset_buffer()
{
   int16_t *out;
   /* Start with half extra buffer of silence */
   out = m.extra_buf;
   while ( out < &m.extra_buf [EXTRA_SIZE_DIV_2] )
      *out++ = 0;
   m.extra_pos = out;
   m.buf_begin = 0;
   dsp_set_output( 0, 0 );
}

/* Sets tempo, where tempo_unit = normal, tempo_unit / 2 = half speed, etc. */

static void spc_set_tempo( int32_t t )
{
   int32_t timer2_shift, other_shift;
   m.tempo = t;
   timer2_shift = 4; /* 64 kHz */
   other_shift  = 3; /*  8 kHz */

   m.timers [2].prescaler = timer2_shift;
   m.timers [1].prescaler = timer2_shift + other_shift;
   m.timers [0].prescaler = timer2_shift + other_shift;
}

static void spc_reset_common( int32_t timer_counter_init )
{
   int32_t i;
   for ( i = 0; i < TIMER_COUNT; i++ )
      m.smp_regs[1][R_T0OUT + i] = timer_counter_init;

   /* Run IPL ROM */
   memset( &m.cpu_regs, 0, sizeof(m.cpu_regs));
   m.cpu_regs.pc = ROM_ADDR;

   m.smp_regs[0][R_TEST   ] = 0x0A;
   m.smp_regs[0][R_CONTROL] = 0xB0; /* ROM enabled, clear ports */
   for ( i = 0; i < PORT_COUNT; i++ )
      m.smp_regs[1][R_CPUIO0 + i] = 0;

   /* reset time registers */
   m.spc_time      = 0;
   m.dsp_time      = 0;
   m.dsp_time = CLOCKS_PER_SAMPLE + 1;

   for ( i = 0; i < TIMER_COUNT; i++ )
   {
      Timer* t = &m.timers [i];
      t->next_time = 1;
      t->divider   = 0;
   }

   /* Registers were just loaded. Applies these new values. */
   spc_enable_rom( m.smp_regs[0][R_CONTROL] & 0x80 );

   /*	Timer registers have been loaded. Applies these to the timers. Does not
      reset timer prescalers or dividers. */
   for ( i = 0; i < TIMER_COUNT; i++ )
   {
      Timer* t = &m.timers [i];
      t->period  = IF_0_THEN_256( m.smp_regs[0][R_T0TARGET + i] );
      t->enabled = m.smp_regs[0][R_CONTROL] >> i & 1;
      t->counter = m.smp_regs[1][R_T0OUT + i] & 0x0F;
   }

   spc_set_tempo( m.tempo );

   m.extra_clocks = 0;
   spc_reset_buffer();
}

/*	Resets SPC to power-on state. This resets your output buffer, so you must
   call set_output() after this. */

static void spc_reset()
{
   m.cpu_regs.pc  = 0xFFC0;
   m.cpu_regs.a   = 0x00;
   m.cpu_regs.x   = 0x00;
   m.cpu_regs.y   = 0x00;
   m.cpu_regs.psw = 0x02;
   m.cpu_regs.sp  = 0xEF;
   memset( m.ram.ram, 0x00, 0x10000 );

   /*	RAM was just loaded from SPC, with $F0-$FF containing SMP registers
      and timer counts. Copies these to proper registers. */
   m.rom_enabled = dsp_m.rom_enabled = 0;

   /* Loads registers from unified 16-byte format */
   memcpy( m.smp_regs[0], &m.ram.ram[0xF0], REG_COUNT );
   memcpy( m.smp_regs[1], m.smp_regs[0], REG_COUNT );

   /* These always read back as 0 */
   m.smp_regs[1][R_TEST    ] = 0;
   m.smp_regs[1][R_CONTROL ] = 0;
   m.smp_regs[1][R_T0TARGET] = 0;
   m.smp_regs[1][R_T1TARGET] = 0;
   m.smp_regs[1][R_T2TARGET] = 0;

   /* Put STOP instruction around memory to catch PC underflow/overflow */
   memset( m.ram.padding1, CPU_PAD_FILL, sizeof m.ram.padding1 );
   memset( m.ram.padding2, CPU_PAD_FILL, sizeof m.ram.padding2 );

   spc_reset_common( 0x0F );
   dsp_reset();
}


/*	Emulates pressing reset switch on SNES. This resets your output buffer, so
   you must call set_output() after this. */

static void spc_soft_reset()
{
   spc_reset_common(0);
   dsp_soft_reset();
}

#if !SPC_NO_COPY_STATE_FUNCS
void spc_copy_state( uint8_t ** io, dsp_copy_func_t copy )
{
   int32_t i;
   spc_state_copy_t copier;
   copier.func = copy;
   copier.buf = io;

   /*	Make state data more readable by putting 64K RAM, 16 SMP registers,
      then DSP (with its 128 registers) first */

   /* RAM */
   spc_enable_rom( 0 ); /* will get re-enabled if necessary in regs_loaded() below */
   spc_copier_copy(&copier, m.ram.ram, 0x10000 );

   {
      /* SMP registers */
      uint8_t regs [REG_COUNT], regs_in [REG_COUNT];

      memcpy( regs, m.smp_regs[0], REG_COUNT );
      memcpy( regs_in, m.smp_regs[1], REG_COUNT );

      spc_copier_copy(&copier, regs, sizeof(regs));
      spc_copier_copy(&copier, regs_in, sizeof(regs_in));

      memcpy( m.smp_regs[0], regs, REG_COUNT);
      memcpy( m.smp_regs[1], regs_in, REG_COUNT );

      spc_enable_rom( m.smp_regs[0][R_CONTROL] & 0x80 );
   }

   /* CPU registers */
   SPC_COPY( uint16_t, m.cpu_regs.pc );
   SPC_COPY(  uint8_t, m.cpu_regs.a );
   SPC_COPY(  uint8_t, m.cpu_regs.x );
   SPC_COPY(  uint8_t, m.cpu_regs.y );
   SPC_COPY(  uint8_t, m.cpu_regs.psw );
   SPC_COPY(  uint8_t, m.cpu_regs.sp );
   spc_copier_extra(&copier);

   SPC_COPY( int16_t, m.spc_time );
   SPC_COPY( int16_t, m.dsp_time );

   /* DSP */
   dsp_copy_state( io, copy );

   /* Timers */
   for ( i = 0; i < TIMER_COUNT; i++ )
   {
      Timer *t;

      t = &m.timers [i];
      t->period  = IF_0_THEN_256( m.smp_regs[0][R_T0TARGET + i] );
      t->enabled = m.smp_regs[0][R_CONTROL] >> i & 1;
      SPC_COPY( int16_t, t->next_time );
      SPC_COPY( uint8_t, t->divider );
      SPC_COPY( uint8_t, t->counter );
      spc_copier_extra(&copier);
   }

   spc_set_tempo( m.tempo );

   spc_copier_extra(&copier);
}
#endif


/***********************************************************************************
 APU
***********************************************************************************/

#define APU_DEFAULT_INPUT_RATE		32040
#define APU_MINIMUM_SAMPLE_COUNT	(512*8)
#define APU_MINIMUM_SAMPLE_BLOCK	(128*8)
#define APU_NUMERATOR_NTSC		15664
#define APU_DENOMINATOR_NTSC		328125
#define APU_NUMERATOR_PAL		34176
#define APU_DENOMINATOR_PAL		709379

static apu_callback	sa_callback     = NULL;

static bool			sound_in_sync   = true;

static int32_t		buffer_size;
static int32_t		lag_master      = 0;
static int32_t		lag             = 0;

static int16_t*		landing_buffer = NULL;

static bool			resampler      = false;

static int32_t		reference_time;
static uint32_t		spc_remainder;

static int32_t		timing_hack_denominator = TEMPO_UNIT;
/* Set these to NTSC for now. Will change to PAL in S9xAPUTimingSetSpeedup
   if necessary on game load. */
static uint32_t		ratio_numerator = APU_NUMERATOR_NTSC;
static uint32_t		ratio_denominator = APU_DENOMINATOR_NTSC;

/***********************************************************************************
   RESAMPLER
************************************************************************************/
static int32_t  rb_size;
static int32_t  rb_buffer_size;
static int32_t  rb_start;
static uint8_t* rb_buffer;
static uint32_t r_step;
static uint32_t r_frac;
static int32_t  r_left[4], r_right[4];

#define SPACE_EMPTY() (rb_buffer_size - rb_size)
#define SPACE_FILLED() (rb_size)
#define MAX_WRITE() (SPACE_EMPTY() >> 1)

#define RESAMPLER_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define SHORT_CLAMP(n) ((int16_t) CLAMP((n), -32768, 32767))

static INLINE int32_t hermite (int32_t mu1, int32_t a, int32_t b, int32_t c, int32_t d)
{
   int32_t mu2, mu3, m0, m1, a0, a1, a2, a3;

   mu2 = ((mu1 * mu1) >> 15);
   mu3 = ((mu2 * mu1) >> 15);

   m0 = (c - a) << 14;
   m1 = (d - b) << 14;

   a0 = (((mu3 << 1) - (3 * mu2) + 32768) * b);
   a1 = ((mu3 - (mu2 << 1) + mu1) * m0) >> 15;
   a2 = ((mu3 -     mu2) * m1) >> 15;
   a3 = ((3 * mu2 - (mu3 << 1)) * c);

   return ((a0) + (a1) + (a2) + (a3)) >> 15;
}

static void resampler_clear()
{
   rb_start = 0;
   rb_size = 0;
   memset (rb_buffer,  0, rb_buffer_size);

   r_frac = 65536;
   r_left [0] = r_left [1] = r_left [2] = r_left [3] = 0;
   r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
}

static void resampler_time_ratio(double ratio)
{
   r_step = 65536 * ratio;
   resampler_clear();
}

static void resampler_read(int16_t *data, int32_t num_samples)
{
   int32_t i_position, o_position, consumed;
   int16_t *internal_buffer;

	if (r_step == 65536)
	{
		//direct copy if we are not resampling
		int bytesUntilBufferEnd = rb_buffer_size - rb_start;
		while (num_samples > 0)
		{
			int bytesToConsume = num_samples * sizeof(int16_t);
			if (bytesToConsume >= bytesUntilBufferEnd)
			{
				bytesToConsume = bytesUntilBufferEnd;
			}
			if (rb_start >= rb_buffer_size)
			{
				rb_start = 0;
			}
			memcpy(data, &rb_buffer[rb_start], bytesToConsume);
			data += bytesToConsume / sizeof(int16_t);
			rb_start += bytesToConsume;
			rb_size -= bytesToConsume;
			num_samples -= bytesToConsume / sizeof(int16_t);
			if (rb_start >= rb_buffer_size)
			{
				rb_start = 0;
			}
		}
		return;
	}

   i_position = rb_start >> 1;
   internal_buffer = (int16_t *)rb_buffer;
   o_position = 0;
   consumed = 0;

   while (o_position < num_samples && consumed < rb_buffer_size)
   {
      int32_t s_left  = internal_buffer[i_position];
      int32_t s_right = internal_buffer[i_position + 1];
      int32_t max_samples = rb_buffer_size >> 1;

      while (r_frac <= 65536 && o_position < num_samples)
      {
         int32_t hermite_val	= hermite(r_frac >> 1, r_left [0], r_left [1], r_left [2], r_left [3]);
         data[o_position]     = SHORT_CLAMP (hermite_val);
         hermite_val = hermite(r_frac >> 1, r_right[0], r_right[1], r_right[2], r_right[3]);
         data[o_position + 1] = SHORT_CLAMP (hermite_val);

         o_position += 2;

         r_frac += r_step;
      }

      if (r_frac > 65536)
      {
         r_left [0] = r_left [1];
         r_left [1] = r_left [2];
         r_left [2] = r_left [3];
         r_left [3] = s_left;

         r_right[0] = r_right[1];
         r_right[1] = r_right[2];
         r_right[2] = r_right[3];
         r_right[3] = s_right;

         r_frac -= 65536;

         i_position += 2;
         if (i_position >= max_samples)
            i_position -= max_samples;
         consumed += 2;
      }
   }

   rb_size -= consumed << 1;
   rb_start += consumed << 1;
   if (rb_start >= rb_buffer_size)
      rb_start -= rb_buffer_size;
}

static void resampler_new(int32_t num_samples)
{
   int32_t new_size = num_samples << 1;

   rb_buffer_size = new_size;
   rb_buffer = (uint8_t *)malloc(rb_buffer_size);
   memset (rb_buffer, 0, rb_buffer_size);

   rb_size = 0;
   rb_start = 0;
   resampler_clear();
}

static INLINE bool resampler_push(int16_t *src, int32_t num_samples)
{
   int32_t end, first_write_size;
   uint8_t *src_ring;
   int32_t bytes = num_samples << 1;
   if (MAX_WRITE() < num_samples || SPACE_EMPTY() < bytes)
      return false;

   /* Ring buffer push */
   src_ring = (uint8_t *)src;
   end = (rb_start + rb_size) % rb_buffer_size;
   first_write_size = RESAMPLER_MIN(bytes, rb_buffer_size - end);

   memcpy (rb_buffer + end, src_ring, first_write_size);

   if (bytes > first_write_size)
      memcpy (rb_buffer, src_ring + first_write_size, bytes - first_write_size);

   rb_size += bytes;

   return true;
}

static INLINE void resampler_resize (int32_t num_samples)
{
   (void) num_samples;
   free(rb_buffer);
   rb_buffer_size = rb_size;
   rb_buffer = (uint8_t *)malloc(rb_buffer_size);
   memset (rb_buffer, 0, rb_buffer_size);

   rb_size = 0;
   rb_start = 0;
}

/***********************************************************************************
   APU
 ***********************************************************************************/

bool S9xMixSamples (int16_t *buffer, uint32_t sample_count)
{
   if (S9xGetSampleCount() >= (sample_count + lag))
   {
      resampler_read(buffer, sample_count);
      if (lag == lag_master)
         lag = 0;
   }
   else
   {
      memset(buffer, 0, sample_count << 1);
      if (lag == 0)
         lag = lag_master;

      return (false);
   }

   return (true);
}

int32_t S9xGetSampleCount()
{
	if (r_step == 65536)
      return rb_size / sizeof(int16_t);
	return (((((uint32_t)rb_size) << 14) - r_frac) / r_step * 2);
}

/* Sets destination for output samples */

static void spc_set_output( int16_t* out, int32_t size )
{
   int16_t *in;
   int16_t *out_end = out + size;

   m.buf_begin = out;
   m.buf_end   = out_end;

   /* Copy extra to output */
   in = m.extra_buf;
   while ( in < m.extra_pos && out < out_end )
      *out++ = *in++;

   /* Handle output being full already */
   if ( out >= out_end )
   {
      /* Have DSP write to remaining extra space */
      out     = dsp_m.extra;
      out_end = &dsp_m.extra[EXTRA_SIZE];

      /* Copy any remaining extra samples as if DSP wrote them */
      while ( in < m.extra_pos )
         *out++ = *in++;
   }

   dsp_set_output( out, out_end - out );
}

void S9xFinalizeSamples(void)
{
   bool ret = resampler_push(landing_buffer, SPC_SAMPLE_COUNT());
   sound_in_sync = false;

   /* We weren't able to process the entire buffer. Potential overrun. */
   if (!ret && Settings.SoundSync)
      return;

   if (!Settings.SoundSync || (SPACE_EMPTY() >= SPACE_FILLED()))
      sound_in_sync = true;

   m.extra_clocks &= CLOCKS_PER_SAMPLE - 1;
   spc_set_output(landing_buffer, buffer_size);
}

void S9xClearSamples(void)
{
   resampler_clear();
   lag = lag_master;
}

bool S9xSyncSound()
{
   if (!Settings.SoundSync || sound_in_sync)
      return true;

   sa_callback();

   return sound_in_sync;
}

void S9xSetSamplesAvailableCallback (apu_callback callback)
{
   sa_callback = callback;
}

static void UpdatePlaybackRate()
{
   double time_ratio;
   if (Settings.SoundInputRate == 0)
      Settings.SoundInputRate = APU_DEFAULT_INPUT_RATE;

   time_ratio = (double) Settings.SoundInputRate * TEMPO_UNIT / (Settings.SoundPlaybackRate * timing_hack_denominator);
   resampler_time_ratio(time_ratio);
}

bool S9xInitSound (int32_t buffer_ms, int32_t lag_ms)
{
   /*	buffer_ms : buffer size given in millisecond
      lag_ms    : allowable time-lag given in millisecond */
   int32_t sample_count, lag_sample_count;

   sample_count     = buffer_ms * 32040 / 1000;
   lag_sample_count = lag_ms    * 32040 / 1000;

   lag_master = lag_sample_count;

   lag_master <<= 1;

   lag = lag_master;

   if (sample_count < APU_MINIMUM_SAMPLE_COUNT)
      sample_count = APU_MINIMUM_SAMPLE_COUNT;

   buffer_size = sample_count;
   buffer_size <<= 1;
   buffer_size <<= 1;

   if (landing_buffer)
      free(landing_buffer);
   landing_buffer = (int16_t*)malloc(buffer_size * 2);
   if (!landing_buffer)
      return (false);

   /* The resampler and spc unit use samples (16-bit int16_t) as
      arguments. Use 2x in the resampler for buffer leveling with SoundSync */

   if (!resampler)
   {
      resampler_new(buffer_size >> (Settings.SoundSync ? 0 : 1));
      resampler = true;
   }
   else
      resampler_resize(buffer_size >> (Settings.SoundSync ? 0 : 1));

   m.extra_clocks &= CLOCKS_PER_SAMPLE - 1;
   spc_set_output(landing_buffer, buffer_size >> 1);

   UpdatePlaybackRate();

   return true;
}

/* Must be called once before using */
static uint8_t  cycle_table [128] =
{/*   01   23   45   67   89   AB   CD   EF */
   0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x68, /* 0 */
   0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x46, /* 1 */
   0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x74, /* 2 */
   0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x38, /* 3 */
   0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x66, /* 4 */
   0x48,0x47,0x45,0x56,0x55,0x45,0x22,0x43, /* 5 */
   0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x75, /* 6 */
   0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x36, /* 7 */
   0x28,0x47,0x34,0x36,0x26,0x54,0x52,0x45, /* 8 */
   0x48,0x47,0x45,0x56,0x55,0x55,0x22,0xC5, /* 9 */
   0x38,0x47,0x34,0x36,0x26,0x44,0x52,0x44, /* A */
   0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x34, /* B */
   0x38,0x47,0x45,0x47,0x25,0x64,0x52,0x49, /* C */
   0x48,0x47,0x56,0x67,0x45,0x55,0x22,0x83, /* D */
   0x28,0x47,0x34,0x36,0x24,0x53,0x43,0x40, /* E */
   0x48,0x47,0x45,0x56,0x34,0x54,0x22,0x60, /* F */
};

static int8_t const reg_times_ [256] =
{
   -1,  0,-11,-10,-15,-11, -2, -2,  4,  3, 14, 14, 26, 26, 14, 22,
   2,  3,  0,  1,-12,  0,  1,  1,  7,  6, 14, 14, 27, 14, 14, 23,
   5,  6,  3,  4, -1,  3,  4,  4, 10,  9, 14, 14, 26, -5, 14, 23,
   8,  9,  6,  7,  2,  6,  7,  7, 13, 12, 14, 14, 27, -4, 14, 24,
   11, 12,  9, 10,  5,  9, 10, 10, 16, 15, 14, 14, -2, -4, 14, 24,
   14, 15, 12, 13,  8, 12, 13, 13, 19, 18, 14, 14, -2,-36, 14, 24,
   17, 18, 15, 16, 11, 15, 16, 16, 22, 21, 14, 14, 28, -3, 14, 25,
   20, 21, 18, 19, 14, 18, 19, 19, 25, 24, 14, 14, 14, 29, 14, 25,

   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
   29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
};

void S9xSetSoundMute(bool mute)
{
	Settings.Mute = mute;
}

bool S9xInitAPU()
{
   int32_t i;

   uint8_t APUROM[64] =
   {
      0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
      0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
      0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
      0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
      0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
      0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
      0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
      0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
   };

   memset( &m, 0, sizeof m );
   dsp_init( m.ram.ram );

   m.tempo = TEMPO_UNIT;

   /*	Most SPC music doesn't need ROM, and almost all the rest only
      rely on these two bytes */

   m.rom [0x3E] = 0xFF;
   m.rom [0x3F] = 0xC0;


   /* unpack cycle table */
   for ( i = 0; i < 128; i++ )
   {
      int32_t n = cycle_table [i];
      m.cycle_table [i * 2 + 0] = n >> 4;
      m.cycle_table [i * 2 + 1] = n & 0x0F;
   }

   allow_time_overflow = false;

   dsp_m.rom = m.rom;
   dsp_m.hi_ram = m.hi_ram;


   memcpy( reg_times, reg_times_, sizeof reg_times );

   spc_reset();


   memcpy( m.rom, APUROM, sizeof m.rom );

   landing_buffer = NULL;

   return true;
}

void S9xDeinitAPU()
{
   if (resampler)
   {
      free(rb_buffer);
      resampler = false;
   }

   if (landing_buffer)
   {
      free(landing_buffer);
      landing_buffer = NULL;
   }
}

#define S9X_APU_GET_CLOCK(cpucycles)		((ratio_numerator * (cpucycles - reference_time) + spc_remainder) / ratio_denominator)
#define S9X_APU_GET_CLOCK_REMAINDER(cpucycles)	((ratio_numerator * (cpucycles - reference_time) + spc_remainder) % ratio_denominator)

/* Emulated port read at specified time */

uint8_t S9xAPUReadPort (int32_t port)	{ return ((uint8_t) spc_run_until_(S9X_APU_GET_CLOCK(CPU.Cycles))[port]); }

/* Emulated port write at specified time */

void S9xAPUWritePort (int32_t port, uint8_t byte)
{
   spc_run_until_( S9X_APU_GET_CLOCK(CPU.Cycles) ) [0x10 + port] = byte;
   m.ram.ram [0xF4 + port] = byte;
}

void S9xAPUSetReferenceTime (int32_t cpucycles)
{
   reference_time = cpucycles;
}

void S9xAPUExecute()
{
   /* Accumulate partial APU cycles */
   spc_end_frame(S9X_APU_GET_CLOCK(CPU.Cycles));

   spc_remainder = S9X_APU_GET_CLOCK_REMAINDER(CPU.Cycles);
   reference_time = CPU.Cycles;

   if (SPC_SAMPLE_COUNT() >= APU_MINIMUM_SAMPLE_BLOCK || !sound_in_sync)
      sa_callback();
}

void S9xAPUTimingSetSpeedup (int32_t ticks)
{
   timing_hack_denominator = TEMPO_UNIT - ticks;
   spc_set_tempo(timing_hack_denominator);

   ratio_numerator = Settings.PAL ? APU_NUMERATOR_PAL : APU_NUMERATOR_NTSC;
   ratio_denominator = Settings.PAL ? APU_DENOMINATOR_PAL : APU_DENOMINATOR_NTSC;
   ratio_denominator = ratio_denominator * timing_hack_denominator / TEMPO_UNIT;

   UpdatePlaybackRate();
}

void S9xAPUAllowTimeOverflow (bool allow)
{
   allow_time_overflow = allow;
}

void S9xResetAPU()
{
   reference_time = 0;
   spc_remainder = 0;
   spc_reset();

   m.extra_clocks &= CLOCKS_PER_SAMPLE - 1;

   spc_set_output(landing_buffer, buffer_size >> 1);

   resampler_clear();
}

void S9xSoftResetAPU()
{
   reference_time = 0;
   spc_remainder = 0;
   spc_soft_reset();

   m.extra_clocks &= CLOCKS_PER_SAMPLE - 1;
   spc_set_output(landing_buffer, buffer_size >> 1);

   resampler_clear();
}

static void from_apu_to_state (uint8_t **buf, void *var, size_t size)
{
   memcpy(*buf, var, size);
   *buf += size;
}

static void to_apu_from_state (uint8_t **buf, void *var, size_t size)
{
   memcpy(var, *buf, size);
   *buf += size;
}

/* work around optimization bug in android GCC
   similar to this: http://jeffq.com/blog/over-aggressive-gcc-optimization-can-cause-sigbus-crash-when-using-memcpy-with-the-android-ndk/ */
#if defined(ANDROID) || defined(__QNX__)
void __attribute__((optimize(0))) S9xAPUSaveState (uint8_t *block)
#else
void S9xAPUSaveState (uint8_t *block)
#endif
{
   uint8_t *ptr;

   ptr = block;

   spc_copy_state(&ptr, from_apu_to_state);

   ptr = (uint8_t*)(((uintptr_t)ptr + 0x3)& ~0x3);
   SET_LE32(ptr, reference_time);
   ptr += sizeof(int32_t);
   SET_LE32(ptr, spc_remainder);
}

#if defined(ANDROID) || defined(__QNX__)
void __attribute__((optimize(0))) S9xAPULoadState (const uint8_t *block)
#else
void S9xAPULoadState (const uint8_t *block)
#endif
{
   uint8_t *ptr = (uint8_t*) block;

   S9xResetAPU();

   spc_copy_state(&ptr, to_apu_from_state);

   ptr = (uint8_t*)(((uintptr_t)ptr + 0x3)& ~0x3);
   reference_time = GET_LE32(ptr);
   ptr += sizeof(int32_t);
   spc_remainder = GET_LE32(ptr);
}

#endif
