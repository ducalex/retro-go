/* This file is part of Snes9x. See LICENSE file. */

#include "memmap.h"
#include "spc7110dec.h"

#define SPC7110_DECOMP_BUFFER_SIZE 64 /* must be >= 64, and must be a power of two */

static const uint8_t evolution_table[53][4] = /* { prob, nextlps, nextmps, toggle invert } */
{
   {0x5a, 1,  1,  1}, {0x25, 6,  2,  0}, {0x11, 8,  3,  0},
   {0x08, 10, 4,  0}, {0x03, 12, 5,  0}, {0x01, 15, 5,  0},
   {0x5a, 7,  7,  1}, {0x3f, 19, 8,  0}, {0x2c, 21, 9,  0},
   {0x20, 22, 10, 0}, {0x17, 23, 11, 0}, {0x11, 25, 12, 0},
   {0x0c, 26, 13, 0}, {0x09, 28, 14, 0}, {0x07, 29, 15, 0},
   {0x05, 31, 16, 0}, {0x04, 32, 17, 0}, {0x03, 34, 18, 0},
   {0x02, 35, 5,  0}, {0x5a, 20, 20, 1}, {0x48, 39, 21, 0},
   {0x3a, 40, 22, 0}, {0x2e, 42, 23, 0}, {0x26, 44, 24, 0},
   {0x1f, 45, 25, 0}, {0x19, 46, 26, 0}, {0x15, 25, 27, 0},
   {0x11, 26, 28, 0}, {0x0e, 26, 29, 0}, {0x0b, 27, 30, 0},
   {0x09, 28, 31, 0}, {0x08, 29, 32, 0}, {0x07, 30, 33, 0},
   {0x05, 31, 34, 0}, {0x04, 33, 35, 0}, {0x04, 33, 36, 0},
   {0x03, 34, 37, 0}, {0x02, 35, 38, 0}, {0x02, 36, 5,  0},
   {0x58, 39, 40, 1}, {0x4d, 47, 41, 0}, {0x43, 48, 42, 0},
   {0x3b, 49, 43, 0}, {0x34, 50, 44, 0}, {0x2e, 51, 45, 0},
   {0x29, 44, 46, 0}, {0x25, 45, 24, 0}, {0x56, 47, 48, 1},
   {0x4f, 47, 49, 0}, {0x47, 48, 50, 0}, {0x41, 49, 51, 0},
   {0x3c, 50, 52, 0}, {0x37, 51, 43, 0},
};

static const uint8_t mode2_context_table[32][2] = /* { next 0, next 1 } */
{
   {1,  2},  {3,  8},  {13, 14}, {15, 16},
   {17, 18}, {19, 20}, {21, 22}, {23, 24},
   {25, 26}, {25, 26}, {25, 26}, {25, 26},
   {25, 26}, {27, 28}, {29, 30}, {31, 31},
   {31, 31}, {31, 31}, {31, 31}, {31, 31},
   {31, 31}, {31, 31}, {31, 31}, {31, 31},
   {31, 31}, {31, 31}, {31, 31}, {31, 31},
   {31, 31}, {31, 31}, {31, 31}, {31, 31},
};

typedef struct
{
   uint32_t mode;
   uint32_t offset;
   uint32_t original_mode;
   uint32_t original_offset;
   uint32_t original_index;
   uint32_t read_counter;
   uint8_t *buffer;
   uint32_t buffer_rdoffset;
   uint32_t buffer_wroffset;
   uint32_t buffer_length;

   struct ContextState
   {
      uint8_t index;
      uint8_t invert;
   } context[32];

   uint32_t morton16[2][256];
   uint32_t morton32[4][256];
} SPC7110Decomp;

SPC7110Decomp decomp;

uint8_t spc7110dec_read(void)
{
   uint8_t data;

   decomp.read_counter++;

   if(decomp.buffer_length == 0)
   {
      switch(decomp.mode)
      {
         case 0:
            spc7110dec_mode0(false);
            break;
         case 1:
            spc7110dec_mode1(false);
            break;
         case 2:
            spc7110dec_mode2(false);
            break;
         default:
            return 0x00;
      }
   }

   data = decomp.buffer[decomp.buffer_rdoffset++];
   decomp.buffer_rdoffset &= SPC7110_DECOMP_BUFFER_SIZE - 1;
   decomp.buffer_length--;
   return data;
}

void spc7110dec_write(uint8_t data)
{
   decomp.buffer[decomp.buffer_wroffset++] = data;
   decomp.buffer_wroffset &= SPC7110_DECOMP_BUFFER_SIZE - 1;
   decomp.buffer_length++;
}

uint8_t spc7110dec_dataread(void)
{
   uint32_t size = Memory.CalculatedSize - 0x100000;
   while(decomp.offset >= size)
      decomp.offset -= size;
   return Memory.ROM[0x100000 + decomp.offset++];
}

void spc7110dec_clear(uint32_t mode, uint32_t offset, uint32_t index)
{
   uint32_t i;

   decomp.original_mode = mode;
   decomp.original_offset = offset;
   decomp.original_index = index;
   decomp.mode = mode;
   decomp.offset = offset;
   decomp.buffer_rdoffset = 0;
   decomp.buffer_wroffset = 0;
   decomp.buffer_length = 0;

   for(i = 0; i < 32; i++) /* reset decomp.context states */
   {
      decomp.context[i].index = 0;
      decomp.context[i].invert = 0;
   }

   switch(decomp.mode)
   {
      case 0:
         spc7110dec_mode0(true);
         break;
      case 1:
         spc7110dec_mode1(true);
         break;
      case 2:
         spc7110dec_mode2(true);
         break;
   }

   while(index--) /* decompress up to requested output data index */
      spc7110dec_read();

   decomp.read_counter = 0;
}

void spc7110dec_mode0(bool init)
{
   static uint8_t val, in, span;
   static int32_t out, inverts, lps, in_count;

   if(init)
   {
      out = inverts = lps = 0;
      span = 0xff;
      val = spc7110dec_dataread();
      in = spc7110dec_dataread();
      in_count = 8;
      return;
   }

   while(decomp.buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
   {
      uint32_t bit;
      for(bit = 0; bit < 8; bit++)
      {
         uint32_t shift = 0;
         uint32_t flag_lps;
         uint32_t prob, mps;
         /* Get decomp.context */
         uint8_t mask = (1 << (bit & 3)) - 1;
         uint8_t con = mask + ((inverts & mask) ^ (lps & mask));

         if(bit > 3)
            con += 15;

         /* Get prob and mps */
         prob = spc7110dec_probability(con);
         mps = (((out >> 15) & 1) ^ decomp.context[con].invert);

         /* Get bit */

         if(val <= span - prob) /* mps */
         {
            span = span - prob;
            out = (out << 1) + mps;
            flag_lps = 0;
         }
         else /* lps */
         {
            val = val - (span - (prob - 1));
            span = prob - 1;
            out = (out << 1) + 1 - mps;
            flag_lps = 1;
         }

         /* Renormalize */

         while(span < 0x7f)
         {
            shift++;
            span = (span << 1) + 1;
            val = (val << 1) + (in >> 7);
            in <<= 1;

            if(--in_count == 0)
            {
               in = spc7110dec_dataread();
               in_count = 8;
            }
         }

         /* Update processing info */
         lps = (lps << 1) + flag_lps;
         inverts = (inverts << 1) + decomp.context[con].invert;

         /* Update context state */
         if(flag_lps & spc7110dec_toggle_invert(con))
            decomp.context[con].invert ^= 1;

         if(flag_lps)
            decomp.context[con].index = spc7110dec_next_lps(con);
         else if(shift)
            decomp.context[con].index = spc7110dec_next_mps(con);
      }

      /* Save byte */
      spc7110dec_write(out);
   }
}

void spc7110dec_mode1(bool init)
{
   static uint32_t pixelorder[4], realorder[4];
   static uint8_t in, val, span;
   static int32_t out, inverts, lps, in_count;

   if(init)
   {
      uint32_t i;
      for(i = 0; i < 4; i++)
         pixelorder[i] = i;
      out = inverts = lps = 0;
      span = 0xff;
      val = spc7110dec_dataread();
      in = spc7110dec_dataread();
      in_count = 8;
      return;
   }

   while(decomp.buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
   {
      uint32_t data;
      uint32_t pixel;

      for(pixel = 0; pixel < 8; pixel++)
      {
         uint32_t bit;
         /* Get first symbol decomp.context */
         uint32_t a = ((out >> (1 * 2)) & 3);
         uint32_t b = ((out >> (7 * 2)) & 3);
         uint32_t c = ((out >> (8 * 2)) & 3);
         uint32_t con = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);

         /* Update pixel order */
         uint32_t m, n;

         for(m = 0; m < 4; m++)
            if(pixelorder[m] == a)
               break;

         for(n = m; n > 0; n--)
            pixelorder[n] = pixelorder[n - 1];

         pixelorder[0] = a;

         /* Calculate the real pixel order */
         for(m = 0; m < 4; m++)
            realorder[m] = pixelorder[m];

         /* Rotate reference pixel c value to top */
         for(m = 0; m < 4; m++)
            if(realorder[m] == c)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = c;

         /* Rotate reference pixel b value to top */
         for(m = 0; m < 4; m++)
            if(realorder[m] == b)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = b;

         /* Rotate reference pixel a value to top */
         for(m = 0; m < 4; m++)
            if(realorder[m] == a)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = a;

         /* Get 2 symbols */

         for(bit = 0; bit < 2; bit++)
         {
            uint32_t shift = 0;
            /* Get prob */
            uint32_t prob = spc7110dec_probability(con);

            /* Get symbol */
            uint32_t flag_lps;

            if(val <= span - prob) /* mps */
            {
               span = span - prob;
               flag_lps = 0;
            }
            else /* lps */
            {
               val = val - (span - (prob - 1));
               span = prob - 1;
               flag_lps = 1;
            }

            /* Renormalize */

            while(span < 0x7f)
            {
               shift++;
               span = (span << 1) + 1;
               val = (val << 1) + (in >> 7);
               in <<= 1;
               if(--in_count == 0)
               {
                  in = spc7110dec_dataread();
                  in_count = 8;
               }
            }

            /* Update processing info */
            lps = (lps << 1) + flag_lps;
            inverts = (inverts << 1) + decomp.context[con].invert;

            /* Update context state */
            if(flag_lps & spc7110dec_toggle_invert(con))
               decomp.context[con].invert ^= 1;

            if(flag_lps)
               decomp.context[con].index = spc7110dec_next_lps(con);
            else if(shift)
               decomp.context[con].index = spc7110dec_next_mps(con);

            /* Get next decomp.context */
            con = 5 + (con << 1) + ((lps ^ inverts) & 1);
         }

         /* Get pixel */
         b = realorder[(lps ^ inverts) & 3];
         out = (out << 2) + b;
      }

      /* Turn pixel data into bitplanes */
      data = spc7110dec_morton_2x8(out);

      spc7110dec_write(data >> 8);
      spc7110dec_write(data >> 0);
   }
}

void spc7110dec_mode2(bool init)
{
   static uint32_t pixelorder[16], realorder[16];
   static uint8_t bitplanebuffer[16], buffer_index;
   static uint8_t in, val, span;
   static int32_t out0, out1, inverts, lps, in_count;

   if(init)
   {
      uint32_t i;

      for(i = 0; i < 16; i++)
         pixelorder[i] = i;
      buffer_index = 0;
      out0 = out1 = inverts = lps = 0;
      span = 0xff;
      val = spc7110dec_dataread();
      in = spc7110dec_dataread();
      in_count = 8;
      return;
   }

   while(decomp.buffer_length < (SPC7110_DECOMP_BUFFER_SIZE >> 1))
   {
      uint32_t pixel, data;

      for(pixel = 0; pixel < 8; pixel++)
      {
         uint32_t bit;
         /* Get first symbol context */
         uint32_t a = ((out0 >> (0 * 4)) & 15);
         uint32_t b = ((out0 >> (7 * 4)) & 15);
         uint32_t c = ((out1 >> (0 * 4)) & 15);
         uint32_t con = 0;
         uint32_t refcon = (a == b) ? (b != c) : (b == c) ? 2 : 4 - (a == c);

         /* Update pixel order */
         uint32_t m, n;

         for(m = 0; m < 16; m++)
            if(pixelorder[m] == a)
               break;

         for(n = m; n > 0; n--)
            pixelorder[n] = pixelorder[n - 1];

         pixelorder[0] = a;

         /* Calculate the real pixel order */
         for(m = 0; m < 16; m++)
            realorder[m] = pixelorder[m];

         /* Rotate reference pixel c value to top */
         for(m = 0; m < 16; m++)
            if(realorder[m] == c)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = c;

         /* Rotate reference pixel b value to top */
         for(m = 0; m < 16; m++)
            if(realorder[m] == b)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = b;

         /* Rotate reference pixel a value to top */
         for(m = 0; m < 16; m++)
            if(realorder[m] == a)
               break;

         for(n = m; n > 0; n--)
            realorder[n] = realorder[n - 1];

         realorder[0] = a;

         /* Get 4 symbols */

         for(bit = 0; bit < 4; bit++)
         {
            uint32_t invertbit;
            uint32_t shift = 0;
            /* Get prob */
            uint32_t prob = spc7110dec_probability(con);

            /* Get symbol */
            uint32_t flag_lps;

            if(val <= span - prob) /* mps */
            {
               span = span - prob;
               flag_lps = 0;
            }
            else /* lps */
            {
               val = val - (span - (prob - 1));
               span = prob - 1;
               flag_lps = 1;
            }

            /* Renormalize */

            while(span < 0x7f)
            {
               shift++;
               span = (span << 1) + 1;
               val = (val << 1) + (in >> 7);
               in <<= 1;
               if(--in_count == 0)
               {
                  in = spc7110dec_dataread();
                  in_count = 8;
               }
            }


            /* Update processing info */
            lps       = (lps << 1) + flag_lps;
            invertbit = decomp.context[con].invert;
            inverts   = (inverts << 1) + invertbit;

            /* Update decomp.context state */
            if(flag_lps & spc7110dec_toggle_invert(con))
               decomp.context[con].invert ^= 1;

            if(flag_lps)
               decomp.context[con].index = spc7110dec_next_lps(con);
            else if(shift)
               decomp.context[con].index = spc7110dec_next_mps(con);

            /* Get next decomp.context */
            con = mode2_context_table[con][flag_lps ^ invertbit] + (con == 1 ? refcon : 0);
         }

         /* Get pixel */
         b = realorder[(lps ^ inverts) & 0x0f];
         out1 = (out1 << 4) + ((out0 >> 28) & 0x0f);
         out0 = (out0 << 4) + b;
      }

      /* Convert pixel data into bitplanes */
      data = spc7110dec_morton_4x8(out0);
      spc7110dec_write(data >> 24);
      spc7110dec_write(data >> 16);
      bitplanebuffer[buffer_index++] = data >> 8;
      bitplanebuffer[buffer_index++] = data >> 0;

      if(buffer_index == 16)
      {
         uint32_t i;
         for(i = 0; i < 16; i++)
            spc7110dec_write(bitplanebuffer[i]);
         buffer_index = 0;
      }
   }
}

uint8_t spc7110dec_probability(uint32_t n)
{
   return evolution_table[decomp.context[n].index][0];
}

uint8_t spc7110dec_next_lps(uint32_t n)
{
   return evolution_table[decomp.context[n].index][1];
}

uint8_t spc7110dec_next_mps(uint32_t n)
{
   return evolution_table[decomp.context[n].index][2];
}

bool spc7110dec_toggle_invert(uint32_t n)
{
   return evolution_table[decomp.context[n].index][3];
}

uint32_t spc7110dec_morton_2x8(uint32_t data)
{
   /* Reverse morton lookup: de-interleave two 8-bit values
    * 15, 13, 11,  9,  7,  5,  3,  1 -> 15-8
    * 14, 12, 10,  8,  6,  4,  2,  0 -> 7 -0 */
   return decomp.morton16[0][(data >> 0) & 255] + decomp.morton16[1][(data >> 8) & 255];
}

uint32_t spc7110dec_morton_4x8(uint32_t data)
{
   /* Reverse morton lookup: de-interleave four 8-bit values
    * 31, 27, 23, 19, 15, 11,  7,  3 -> 31-24
    * 30, 26, 22, 18, 14, 10,  6,  2 -> 23-16
    * 29, 25, 21, 17, 13,  9,  5,  1 -> 15-8
    * 28, 24, 20, 16, 12,  8,  4,  0 -> 7 -0 */
   return decomp.morton32[0][(data >> 0) & 255] + decomp.morton32[1][(data >> 8) & 255] + decomp.morton32[2][(data >> 16) & 255] + decomp.morton32[3][(data >> 24) & 255];
}

void spc7110dec_reset(void)
{
   /* Mode 3 is invalid; this is treated as a special case to always return 0x00
    * set to mode 3 so that reading decomp port before starting first decomp will return 0x00 */
   decomp.mode = 3;
   decomp.buffer_rdoffset = 0;
   decomp.buffer_wroffset = 0;
   decomp.buffer_length = 0;
}

void spc7110dec_init(void)
{
   uint32_t i;

   decomp.buffer = malloc(SPC7110_DECOMP_BUFFER_SIZE);
   spc7110dec_reset();

   /* Initialize reverse morton lookup tables */
   for(i = 0; i < 256; i++)
   {
      #define map(x, y) (((i >> x) & 1) << y)
      /* 2x8-bit */
      decomp.morton16[1][i] = map(7, 15) + map(6, 7) + map(5, 14) + map(4, 6) + map(3, 13) + map(2, 5) + map(1, 12) + map(0, 4);
      decomp.morton16[0][i] = map(7, 11) + map(6, 3) + map(5, 10) + map(4, 2) + map(3, 9) + map(2, 1) + map(1, 8) + map(0, 0);
      /* 4x8-bit */
      decomp.morton32[3][i] = map(7, 31) + map(6, 23) + map(5, 15) + map(4, 7) + map(3, 30) + map(2, 22) + map(1, 14) + map(0, 6);
      decomp.morton32[2][i] = map(7, 29) + map(6, 21) + map(5, 13) + map(4, 5) + map(3, 28) + map(2, 20) + map(1, 12) + map(0, 4);
      decomp.morton32[1][i] = map(7, 27) + map(6, 19) + map(5, 11) + map(4, 3) + map(3, 26) + map(2, 18) + map(1, 10) + map(0, 2);
      decomp.morton32[0][i] = map(7, 25) + map(6, 17) + map(5, 9) + map(4, 1) + map(3, 24) + map(2, 16) + map(1, 8) + map(0, 0);
      #undef map
   }
}

void spc7110dec_deinit(void)
{
   free(decomp.buffer);
}
